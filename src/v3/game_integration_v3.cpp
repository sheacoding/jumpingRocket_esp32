#include "v3/game_integration_v3.h"
#include "v3/data_manager_v3.h"
#include "v3/ui_views_v3.h"
#include "jumping_rocket_simple.h"

// V3.0游戏集成状态
static bool v3_game_integration_active = false;
static bool v3_ui_mode_active = false;
static uint32_t v3_game_start_time = 0;
static game_difficulty_t v3_current_difficulty = DIFFICULTY_NORMAL;

// V3.0游戏集成初始化
bool initGameIntegrationV3() {
    Serial.println("🎮 初始化V3.0游戏集成...");
    
    if (!dataManagerV3.isInitialized()) {
        Serial.println("❌ V3.0数据管理器未初始化");
        return false;
    }
    
    v3_game_integration_active = true;
    v3_ui_mode_active = false;
    
    Serial.println("✅ V3.0游戏集成初始化成功");
    return true;
}

// 检查是否应该进入V3.0 UI模式
bool shouldEnterV3UIMode() {
    // 在待机状态且V3.0功能启用时进入UI模式
    return (current_state == GAME_STATE_IDLE && 
            v3_game_integration_active && 
            !v3_ui_mode_active);
}

// 进入V3.0 UI模式
void enterV3UIMode() {
    if (!v3_game_integration_active) {
        Serial.println("❌ V3.0游戏集成未激活，无法进入UI模式");
        return;
    }

    Serial.println("🎨 进入V3.0 UI模式");
    v3_ui_mode_active = true;

    // 初始化UI管理器（如果还没有初始化）
    extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
    if (!uiManagerV3) {
        Serial.println("🎨 初始化UI管理器...");
        if (!initUIManagerV3(&u8g2)) {
            Serial.println("❌ UI管理器初始化失败");
            v3_ui_mode_active = false;
            return;
        }
        Serial.println("✅ UI管理器初始化成功");
    }

    // 切换到主菜单
    if (uiManagerV3) {
        Serial.println("🎨 切换到主菜单视图");
        uiManagerV3->switchToView(UI_VIEW_MAIN_MENU);
        Serial.printf("🎨 当前视图: %d\n", uiManagerV3->getCurrentView());
    } else {
        Serial.println("❌ UI管理器为空");
        v3_ui_mode_active = false;
    }
}

// 退出V3.0 UI模式
void exitV3UIMode() {
    if (!v3_ui_mode_active) return;

    Serial.println("Exiting V3.0 UI mode");
    v3_ui_mode_active = false;

    // 重置难度选择
    if (uiManagerV3) {
        uiManagerV3->resetDifficultySelection();
    }
}

// 检查是否在V3.0 UI模式
bool isInV3UIMode() {
    return v3_ui_mode_active;
}

// V3.0 UI核心函数实现
void updateUIV3() {
    if (uiManagerV3) {
        uiManagerV3->update();
    }
}

void renderUIV3() {
    if (uiManagerV3) {
        // 添加调试信息
        static uint32_t last_debug_time = 0;
        uint32_t current_time = millis();
        if (current_time - last_debug_time > 2000) { // 每2秒输出一次调试信息
            Serial.printf("V3.0 UI rendering, current view: %d\n", uiManagerV3->getCurrentView());
            last_debug_time = current_time;
        }

        uiManagerV3->render();
    } else {
        Serial.println("ERROR: renderUIV3: UI manager is null");
    }
}

bool handleUIButtonV3(button_event_t event) {
    if (uiManagerV3) {
        return uiManagerV3->handleButton(event);
    }
    return false;
}

// V3.0 UI模式更新
void updateV3UIMode() {
    if (!v3_ui_mode_active || !uiManagerV3) return;

    // 更新UI
    updateUIV3();

    // 检查是否需要开始游戏
    if (uiManagerV3->getCurrentView() == UI_VIEW_DIFFICULTY_SELECT &&
        uiManagerV3->isDifficultyConfirmed()) {

        // 获取选择的难度
        v3_current_difficulty = uiManagerV3->getSelectedDifficulty();

        // 开始游戏
        startV3Game();
    }
}

// V3.0 UI模式渲染
void renderV3UIMode() {
    if (!v3_ui_mode_active || !uiManagerV3) return;

    renderUIV3();
}

// V3.0 UI模式按钮处理
bool handleV3UIButton(button_event_t event) {
    if (!v3_ui_mode_active || !uiManagerV3) return false;

    return handleUIButtonV3(event);
}

// 开始V3.0游戏
void startV3Game() {
    if (!v3_game_integration_active) return;
    
    Serial.printf("🎮 开始V3.0游戏，难度: %s\n", 
                 V3Config::getDifficultyName(v3_current_difficulty));
    
    // 退出UI模式
    exitV3UIMode();
    
    // 设置游戏难度
    game_data.difficulty = v3_current_difficulty;
    
    // 记录游戏开始时间
    v3_game_start_time = millis();
    
    // 启动V2.0游戏逻辑
    game_start();
    
    Serial.println("✅ V3.0游戏启动完成");
}

// V3.0游戏结束处理
void onV3GameComplete() {
    if (!v3_game_integration_active) return;
    
    uint32_t game_duration = (millis() - v3_game_start_time) / 1000;
    uint32_t jump_count = game_data.jump_count;
    
    Serial.printf("🏁 V3.0游戏结束: %d次跳跃, %d秒\n", jump_count, game_duration);
    
    // 保存游戏会话到V3.0数据系统
    if (dataManagerV3.isInitialized()) {
        GameSessionV3 session = dataManagerV3.createGameSession(
            v3_current_difficulty, 
            jump_count, 
            game_duration
        );
        
        if (dataManagerV3.saveGameSession(session)) {
            Serial.println("✅ V3.0游戏数据保存成功");
            
            // 显示会话结果
            Serial.printf("   得分: %d 分\n", session.score);
            Serial.printf("   卡路里: %.1f\n", session.calories);
            Serial.printf("   平均频率: %.2f 次/秒\n", session.avg_frequency);
            Serial.printf("   目标达成: %s\n", session.target_achieved ? "是" : "否");
        } else {
            Serial.println("❌ V3.0游戏数据保存失败");
        }
    }
    
    // 重置游戏开始时间
    v3_game_start_time = 0;
}

// 获取当前V3.0游戏状态
V3GameState getV3GameState() {
    if (!v3_game_integration_active) {
        return V3_GAME_DISABLED;
    }
    
    if (v3_ui_mode_active) {
        return V3_GAME_UI_MODE;
    }
    
    if (current_state == GAME_STATE_PLAYING) {
        return V3_GAME_PLAYING;
    }
    
    if (current_state == GAME_STATE_IDLE) {
        return V3_GAME_IDLE;
    }
    
    return V3_GAME_OTHER;
}

// V3.0游戏统计信息
void printV3GameStats() {
    if (!dataManagerV3.isInitialized()) {
        Serial.println("ERROR: V3.0 data manager not initialized");
        return;
    }

    Serial.println("V3.0 Game Statistics:");
    Serial.printf("   Today Games: %d\n", dataManagerV3.getTotalGamesToday());
    Serial.printf("   Today Jumps: %d\n", dataManagerV3.getTotalJumpsToday());
    Serial.printf("   Today Calories: %.1f\n", dataManagerV3.getTotalCaloriesToday());
    Serial.printf("   Today Duration: %s\n", DataUtilsV3::formatTime(dataManagerV3.getTotalTimeToday()).c_str());
    Serial.printf("   Target Progress: %.1f%%\n", dataManagerV3.getTodayTargetProgress() * 100);

    const HistoryStatsV3& stats = dataManagerV3.getHistoryStats();
    Serial.printf("   Best Score: %d\n", stats.best_score);
    Serial.printf("   Total Jumps: %d\n", stats.total_jumps);
}

// V3.0兼容性检查
bool checkV3Compatibility() {
    // 检查V3.0功能是否可用
    bool file_system_ok = fileSystemV3.isAvailable();
    bool data_manager_ok = dataManagerV3.isInitialized();
    bool ui_manager_ok = (uiManagerV3 != nullptr);
    
    Serial.println("🔍 V3.0兼容性检查:");
    Serial.printf("   文件系统: %s\n", file_system_ok ? "✅" : "❌");
    Serial.printf("   数据管理: %s\n", data_manager_ok ? "✅" : "❌");
    Serial.printf("   UI管理器: %s\n", ui_manager_ok ? "✅" : "❌");
    
    bool compatible = file_system_ok && data_manager_ok;
    Serial.printf("   整体状态: %s\n", compatible ? "✅ 兼容" : "❌ 不兼容");
    
    return compatible;
}

// V3.0数据迁移（从V2.0）
bool migrateFromV2ToV3() {
    Serial.println("🔄 开始V2.0到V3.0数据迁移...");
    
    if (!dataManagerV3.isInitialized()) {
        Serial.println("❌ V3.0数据管理器未初始化");
        return false;
    }
    
    // 这里可以实现从V2.0内存数据到V3.0文件系统的迁移
    // 目前V2.0没有持久化数据，所以主要是迁移配置
    
    SystemConfigV3 config = dataManagerV3.getSystemConfig();
    
    // 从V2.0游戏数据迁移当前会话（如果有）
    if (current_state == GAME_STATE_PLAYING || current_state == GAME_STATE_RESULT) {
        uint32_t duration = game_data.game_time_ms / 1000;
        if (duration > 0 && game_data.jump_count > 0) {
            GameSessionV3 session = dataManagerV3.createGameSession(
                game_data.difficulty,
                game_data.jump_count,
                duration
            );
            
            if (dataManagerV3.saveGameSession(session)) {
                Serial.println("✅ 当前游戏会话已迁移到V3.0");
            }
        }
    }
    
    Serial.println("✅ V2.0到V3.0数据迁移完成");
    return true;
}

// V3.0系统状态报告
void reportV3SystemStatus() {
    Serial.println("V3.0 System Status Report:");
    Serial.printf("   Integration: %s\n", v3_game_integration_active ? "Active" : "Disabled");
    Serial.printf("   UI Mode: %s\n", v3_ui_mode_active ? "Active" : "Disabled");
    Serial.printf("   Game State: %d\n", getV3GameState());
    Serial.printf("   Current Difficulty: %s\n", V3Config::getDifficultyName(v3_current_difficulty));

    if (v3_game_start_time > 0) {
        uint32_t elapsed = (millis() - v3_game_start_time) / 1000;
        Serial.printf("   Game Duration: %s\n", DataUtilsV3::formatTime(elapsed).c_str());
    }

    // 检查兼容性
    checkV3Compatibility();

    // 显示统计信息
    printV3GameStats();
}

// V2.0游戏事件回调函数实现

/**
 * V2.0游戏开始时的V3.0处理
 */
void onV2GameStart(game_difficulty_t difficulty) {
    if (!v3_game_integration_active) return;

    v3_current_difficulty = difficulty;
    v3_game_start_time = millis();

    Serial.printf("🎮 V2.0游戏开始，V3.0记录难度: %s\n",
                 V3Config::getDifficultyName(difficulty));
}

/**
 * V2.0游戏暂停时的V3.0处理
 */
void onV2GamePause() {
    if (!v3_game_integration_active) return;

    Serial.println("⏸️ V2.0游戏暂停，V3.0记录暂停事件");

    // 可以在这里记录暂停时间等信息
}

/**
 * V2.0游戏恢复时的V3.0处理
 */
void onV2GameResume() {
    if (!v3_game_integration_active) return;

    Serial.println("▶️ V2.0游戏恢复，V3.0记录恢复事件");

    // 可以在这里记录恢复时间等信息
}

/**
 * V2.0游戏重置时的V3.0处理
 */
void onV2GameReset() {
    if (!v3_game_integration_active) return;

    Serial.println("🔄 V2.0游戏重置，V3.0清理状态");

    // 重置V3.0游戏状态
    v3_game_start_time = 0;
    v3_current_difficulty = DIFFICULTY_NORMAL;
}

/**
 * V2.0跳跃检测时的V3.0处理
 */
void onV2JumpDetected(uint32_t jump_count, uint32_t game_time) {
    if (!v3_game_integration_active) return;

    // 这里可以实时更新V3.0的跳跃统计
    // 暂时只记录日志，避免过多输出
    static uint32_t last_log_time = 0;
    uint32_t current_time = millis();

    // 每5秒记录一次日志
    if (current_time - last_log_time >= 5000) {
        Serial.printf("📊 V3.0记录跳跃: %d次，游戏时间: %d秒\n",
                     jump_count, game_time / 1000);
        last_log_time = current_time;
    }
}
