#include <Arduino.h>

// V3.0 核心头文件
#include "v3/board_config_v3.h"
#include "v3/file_system_v3.h"
#include "v3/data_manager_v3.h"
#include "v3/data_models_v3.h"
#include "v3/ui_views_v3.h"
#include "v3/game_integration_v3.h"

// 外部测试函数声明
extern void runV3SystemTests();
extern void printV3SystemDetails();
extern void runV3PerformanceBenchmark();

// 保持与V2.0的兼容性
#include "jumping_rocket_simple.h"

// V3.0 全局变量
bool v3_system_initialized = false;
v3_system_state_t v3_current_state = V3_STATE_BOOT;
game_difficulty_t v3_selected_difficulty = DIFFICULTY_NORMAL;

// V3.0 初始化函数
bool initializeV3System() {
    Serial.println("========================================");
    Serial.println("🚀 蹦跳小火箭 V3.0 系统初始化");
    Serial.println("========================================");
    
    // 显示V3.0配置信息
    BOARD_DEBUG_INFO_V3();
    
    // 初始化文件系统
    if (!fileSystemV3.init()) {
        Serial.println("❌ V3.0文件系统初始化失败");
        return false;
    }
    
    // 初始化数据管理器
    if (!dataManagerV3.init(&fileSystemV3)) {
        Serial.println("❌ V3.0数据管理器初始化失败");
        return false;
    }

    // 生成演示数据（如果需要）
    dataManagerV3.generateDemoData(7);

    // 初始化UI管理器
    Serial.println("🎨 初始化V3.0 UI管理器...");
    extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
    if (!initUIManagerV3(&u8g2)) {
        Serial.println("❌ V3.0 UI管理器初始化失败");
        return false;
    }
    Serial.println("✅ V3.0 UI管理器初始化成功");

    // 初始化游戏集成
    Serial.println("🎮 初始化V3.0游戏集成...");
    if (!initGameIntegrationV3()) {
        Serial.println("❌ V3.0游戏集成初始化失败");
        return false;
    }
    Serial.println("✅ V3.0游戏集成初始化成功");

    // 检查是否需要从V2.0迁移数据
    Serial.println("🔄 检查V2.0数据迁移...");
    migrateFromV2ToV3();

    // 验证配置完整性
    Serial.println("🔍 验证V3.0配置...");
    if (V3Config::validateConfigs()) {
        Serial.println("✅ V3.0配置验证通过");
    } else {
        Serial.println("⚠️ V3.0配置验证有警告");
    }

    v3_system_initialized = true;
    v3_current_state = V3_STATE_MAIN_MENU;

    Serial.println("✅ V3.0系统初始化完成");
    Serial.println("========================================");
    
    return true;
}

// V3.0 游戏会话处理
void handleV3GameSession(uint32_t jump_count, uint32_t duration) {
    if (!v3_system_initialized) return;
    
    Serial.printf("🎮 V3.0游戏会话结束: %d次跳跃, %d秒\n", jump_count, duration);
    
    // 创建游戏会话数据
    GameSessionV3 session = dataManagerV3.createGameSession(
        v3_selected_difficulty, 
        jump_count, 
        duration
    );
    
    // 保存游戏会话
    if (dataManagerV3.saveGameSession(session)) {
        Serial.println("✅ V3.0游戏数据保存成功");
        
        // 显示会话结果
        Serial.printf("   得分: %d 分\n", session.score);
        Serial.printf("   卡路里: %.1f\n", session.calories);
        Serial.printf("   平均频率: %.2f 次/秒\n", session.avg_frequency);
        Serial.printf("   目标达成: %s\n", session.target_achieved ? "是" : "否");
        
        // 显示今日统计
        Serial.printf("📊 今日统计:\n");
        Serial.printf("   总游戏: %d 次\n", dataManagerV3.getTotalGamesToday());
        Serial.printf("   总跳跃: %d 次\n", dataManagerV3.getTotalJumpsToday());
        Serial.printf("   总卡路里: %.1f\n", dataManagerV3.getTotalCaloriesToday());
        Serial.printf("   目标进度: %.1f%%\n", dataManagerV3.getTodayTargetProgress() * 100);
    } else {
        Serial.println("❌ V3.0游戏数据保存失败");
    }
}

// V3.0 状态管理
void updateV3State() {
    if (!v3_system_initialized) return;
    
    // 根据当前状态执行相应逻辑
    switch (v3_current_state) {
        case V3_STATE_BOOT:
            // 启动状态，等待初始化完成
            break;
            
        case V3_STATE_MAIN_MENU:
            // 主菜单状态
            break;
            
        case V3_STATE_DIFFICULTY_SELECT:
            // 难度选择状态
            break;
            
        case V3_STATE_GAME_READY:
            // 游戏准备状态
            break;
            
        case V3_STATE_GAME_PLAYING:
            // 游戏进行状态
            break;
            
        case V3_STATE_GAME_RESULT:
            // 游戏结果状态
            break;
            
        case V3_STATE_HISTORY_VIEW:
            // 历史数据查看状态
            break;
            
        case V3_STATE_SETTINGS:
            // 设置状态
            break;
            
        case V3_STATE_TARGET_TIMER:
            // 目标计时状态
            break;
            
        default:
            v3_current_state = V3_STATE_MAIN_MENU;
            break;
    }
}

// V3.0 难度选择
void setV3Difficulty(game_difficulty_t difficulty) {
    if (difficulty < DIFFICULTY_COUNT) {
        v3_selected_difficulty = difficulty;
        
        const difficulty_config_t* config = V3Config::getDifficultyConfig(difficulty);
        Serial.printf("🎯 V3.0难度设置: %s (%.0f%%强度)\n",
                     config->name_en, config->multiplier * 100);
        Serial.printf("   目标跳跃: %d 次\n", config->target_jumps);
        Serial.printf("   目标时间: %d 秒\n", config->target_time);
    }
}

// V3.0 系统信息显示（调用外部完整信息显示）
void printV3SystemInfo() {
    if (!v3_system_initialized) {
        Serial.println("⚠️ V3.0系统未初始化");
        return;
    }

    // 调用外部完整系统信息显示
    extern void printV3SystemDetails();
    printV3SystemDetails();

    // 显示本地状态信息
    Serial.println("📊 V3.0本地状态:");
    Serial.printf("   版本: %s\n", JUMPING_ROCKET_VERSION_STRING);
    Serial.printf("   当前状态: %d\n", v3_current_state);
    Serial.printf("   当前难度: %s\n", V3Config::getDifficultyName(v3_selected_difficulty));

    // 显示数据管理器状态
    dataManagerV3.printDataSummary();

    // 显示文件系统状态
    fileSystemV3.printFileSystemInfo();
}

// V3.0 测试函数（调用外部完整测试）
void testV3System() {
    Serial.println("🧪 V3.0系统测试开始...");

    // 调用外部完整测试套件
    extern void runV3SystemTests();
    runV3SystemTests();

    // 运行性能基准测试
    Serial.println("⚡ 运行性能基准测试...");
    runV3PerformanceBenchmark();

    // 测试不同难度的游戏会话
    Serial.println("🎮 测试游戏会话处理...");
    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        game_difficulty_t test_difficulty = (game_difficulty_t)i;
        setV3Difficulty(test_difficulty);

        // 模拟游戏会话
        uint32_t test_jumps = 30 + i * 20;  // 30, 50, 70
        uint32_t test_duration = 180 + i * 60; // 3, 4, 5分钟

        handleV3GameSession(test_jumps, test_duration);

        delay(100); // 短暂延迟
    }

    Serial.println("✅ V3.0系统测试完成");
    printV3SystemInfo();
}

// V3.0 主循环处理
void loopV3() {
    if (!v3_system_initialized) return;

    // 更新状态机
    updateV3State();

    // 更新UI（如果在UI模式）
    if (V3_IS_IN_UI()) {
        V3_UPDATE_UI();
    }

    // 定期保存数据（每分钟）
    static uint32_t last_save_time = 0;
    uint32_t current_time = millis();
    if (current_time - last_save_time > 60000) { // 60秒
        dataManagerV3.saveCurrentDayData();
        last_save_time = current_time;
    }

    // 定期系统状态报告（每10分钟）
    static uint32_t last_report_time = 0;
    if (current_time - last_report_time > 600000) { // 10分钟
        reportV3SystemStatus();
        last_report_time = current_time;
    }
}

// V3.0 关闭处理
void shutdownV3System() {
    if (v3_system_initialized) {
        Serial.println("🔄 V3.0系统关闭中...");

        // 保存所有数据
        dataManagerV3.deinit();
        fileSystemV3.deinit();

        // 清理UI管理器
        deinitUIManagerV3();

        v3_system_initialized = false;
        v3_current_state = V3_STATE_BOOT;

        Serial.println("✅ V3.0系统已关闭");
    }
}

// 兼容性函数：从V2.0游戏逻辑调用V3.0功能
extern "C" {
    void v3_on_game_complete(uint32_t jumps, uint32_t duration) {
        handleV3GameSession(jumps, duration);
    }
    
    void v3_set_difficulty(int difficulty) {
        if (difficulty >= 0 && difficulty < DIFFICULTY_COUNT) {
            setV3Difficulty((game_difficulty_t)difficulty);
        }
    }
    
    int v3_get_total_jumps_today() {
        return v3_system_initialized ? dataManagerV3.getTotalJumpsToday() : 0;
    }
    
    float v3_get_total_calories_today() {
        return v3_system_initialized ? dataManagerV3.getTotalCaloriesToday() : 0.0f;
    }
    
    bool v3_is_target_achieved() {
        return v3_system_initialized ? dataManagerV3.isTodayTargetAchieved() : false;
    }
}
