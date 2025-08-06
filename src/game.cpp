#include "jumping_rocket_simple.h"

// V3.0 集成
#ifdef JUMPING_ROCKET_V3
#include "v3/game_integration_v3.h"
#include "v3/data_manager_v3.h"
#include "v3/data_models_v3.h"
#include "v3/board_config_v3.h"
#endif

// 全局游戏状态和数据
game_state_t current_state = GAME_STATE_IDLE;
game_data_t game_data = {0};

// 难度选择相关变量
game_difficulty_t selected_difficulty = DIFFICULTY_NORMAL; // 默认普通难度

// 游戏数据初始化函数
void game_data_init(void) {
    memset(&game_data, 0, sizeof(game_data_t));
    game_data.difficulty = DIFFICULTY_NORMAL; // 设置默认难度

    // 初始化目标监控状态
    game_target_monitor_init();

    Serial.printf("游戏数据初始化，默认难度: %s\n", get_difficulty_name(game_data.difficulty));
}

// 游戏计时器
static uint32_t game_start_time = 0;
static uint32_t pause_start_time = 0;
static uint32_t total_pause_time = 0;

// 燃料计算参数
#define FUEL_PER_JUMP           5       // 每次跳跃增加的燃料
#define FUEL_TIME_BONUS         1       // 每秒时间奖励燃料
#define MAX_FUEL                100     // 最大燃料值

// 飞行高度计算参数
#define HEIGHT_BASE             100     // 基础高度
#define HEIGHT_PER_JUMP         50      // 每次跳跃增加高度
#define HEIGHT_TIME_MULTIPLIER  2       // 时间倍数

// 获取当前时间（毫秒）
uint32_t get_time_ms(void) {
    return millis();
}

// 游戏重置
void game_reset(void) {
    Serial.println("重置游戏");

    // 保存当前难度设置
    game_difficulty_t current_difficulty = game_data.difficulty;

    // 重置游戏数据
    memset(&game_data, 0, sizeof(game_data_t));

    // 恢复难度设置（重置后保持用户选择的难度）
    game_data.difficulty = current_difficulty;

    // 重置计时器
    game_start_time = 0;
    pause_start_time = 0;
    total_pause_time = 0;

    // 切换到待机状态
    current_state = GAME_STATE_IDLE;

    Serial.printf("游戏重置完成，保持难度: %s\n", get_difficulty_name(game_data.difficulty));

#ifdef JUMPING_ROCKET_V3
    // V3.0游戏重置事件
    V3_ON_GAME_RESET();
#endif
}

// 游戏开始
void game_start(void) {
    Serial.println("🎮 开始游戏");

    // 记录精确的开始时间
    uint32_t current_time = get_time_ms();

    // 重置游戏数据（完全重置，只保留难度设置）
    game_difficulty_t existing_difficulty = game_data.difficulty;
    memset(&game_data, 0, sizeof(game_data_t));
    game_data.difficulty = existing_difficulty; // 恢复难度设置

    Serial.printf("   游戏数据完全重置，难度: %s\n", get_difficulty_name(existing_difficulty));

    // 设置开始时间
    game_start_time = current_time;
    total_pause_time = 0;

    // 切换到游戏状态
    current_state = GAME_STATE_PLAYING;

    Serial.printf("   游戏开始时间: %lu ms\n", game_start_time);
    Serial.printf("   初始跳跃计数: %lu\n", game_data.jump_count);
    Serial.printf("   暂停时间重置: %lu ms\n", total_pause_time);

#ifdef JUMPING_ROCKET_V3
    // V3.0游戏开始事件
    V3_ON_GAME_START(existing_difficulty);
#endif
}

// 游戏暂停
void game_pause(void) {
    if (current_state != GAME_STATE_PLAYING) {
        Serial.println("游戏未在进行中，无法暂停");
        return;
    }
    
    Serial.println("暂停游戏");
    
    // 记录暂停时间
    pause_start_time = get_time_ms();
    
    // 更新游戏时长
    game_update_data();
    
    // 切换到暂停状态
    current_state = GAME_STATE_PAUSED;
    
    // 播放暂停音效
    play_sound_effect(SOUND_PAUSE);

    Serial.println("游戏已暂停");

#ifdef JUMPING_ROCKET_V3
    // V3.0游戏暂停事件
    V3_ON_GAME_PAUSE();
#endif
}

// 游戏继续
void game_resume(void) {
    if (current_state != GAME_STATE_PAUSED) {
        Serial.println("游戏未暂停，无法继续");
        return;
    }
    
    Serial.println("继续游戏");
    
    // 累计暂停时间
    if (pause_start_time > 0) {
        total_pause_time += get_time_ms() - pause_start_time;
        pause_start_time = 0;
    }
    
    // 切换到游戏状态
    current_state = GAME_STATE_PLAYING;
    
    // 播放继续音效
    play_sound_effect(SOUND_RESUME);

    Serial.println("游戏已继续");

#ifdef JUMPING_ROCKET_V3
    // V3.0游戏恢复事件
    V3_ON_GAME_RESUME();
#endif
}

// 计算燃料进度（严格按照跳跃充能）
static void calculate_fuel_progress(void) {
    // 燃料进度严格基于跳跃次数，不受时间影响
    uint32_t fuel_from_jumps = game_data.jump_count * FUEL_PER_JUMP;

    // 限制最大燃料值
    if (fuel_from_jumps > MAX_FUEL) {
        fuel_from_jumps = MAX_FUEL;
    }

    game_data.fuel_progress = fuel_from_jumps;

    // 调试输出燃料计算
    static uint32_t last_fuel_debug = 0;
    if (game_data.fuel_progress != last_fuel_debug) {
        Serial.printf("⛽ 燃料充能: %lu次跳跃 × %d = %lu%% (最大%d%%)\n",
                     game_data.jump_count, FUEL_PER_JUMP, game_data.fuel_progress, MAX_FUEL);
        last_fuel_debug = game_data.fuel_progress;
    }
}

// 计算游戏结果（高度已在发射动画中计算）
void game_calculate_result(void) {
    Serial.println("计算游戏结果");

    // 确保游戏时长是最新的
    game_update_data();

    // 飞行高度已经在火箭发射动画中动态计算并保存
    // 这里只需要显示最终结果

    Serial.printf("游戏结果 - 跳跃: %lu次, 时长: %lu秒, 飞行高度: %lu米\n",
             game_data.jump_count, game_data.game_time_ms / 1000, game_data.flight_height);

    // 切换到结算状态
    current_state = GAME_STATE_RESULT;

    // 如果成绩不错，播放胜利音效
    if (game_data.flight_height >= 5000) { // 提高胜利音效触发门槛
        play_sound_effect(SOUND_VICTORY);
    }

#ifdef JUMPING_ROCKET_V3
    // V3.0游戏完成事件
    V3_ON_GAME_COMPLETE();
#endif
}

// 更新游戏数据
void game_update_data(void) {
    if (current_state == GAME_STATE_PLAYING && game_start_time > 0) {
        // 计算实际游戏时长（排除暂停时间）
        uint32_t current_time = get_time_ms();
        uint32_t total_elapsed = current_time - game_start_time;
        game_data.game_time_ms = total_elapsed - total_pause_time;

        // 调试输出计时信息（每5秒输出一次）
        static uint32_t last_debug_time = 0;
        if (current_time - last_debug_time >= 5000) {
            Serial.printf("⏰ 计时调试 - 当前时间: %lu ms\n", current_time);
            Serial.printf("   游戏开始时间: %lu ms\n", game_start_time);
            Serial.printf("   原始游戏时长: %lu ms\n", total_elapsed);
            Serial.printf("   总暂停时间: %lu ms\n", total_pause_time);
            Serial.printf("   净游戏时长: %lu ms (%lu秒)\n",
                         game_data.game_time_ms, game_data.game_time_ms / 1000);
            last_debug_time = current_time;
        }

        // 计算燃料进度
        calculate_fuel_progress();
    }
}

// 检查是否应该启动火箭发射
bool should_start_rocket_launch(void) {
    // 获取当前难度的燃料阈值
    uint32_t fuel_threshold = get_difficulty_fuel_threshold(game_data.difficulty);

    // 调试输出当前难度信息（每次检查时输出）
    static uint32_t last_debug_time = 0;
    uint32_t current_time = millis();
    if (current_time - last_debug_time > 3000) { // 每3秒输出一次
        Serial.printf("🎯 难度检查: 当前难度=%s, 燃料阈值=%lu%%, 当前燃料=%lu%%\n",
                     get_difficulty_name(game_data.difficulty), fuel_threshold, game_data.fuel_progress);
        last_debug_time = current_time;
    }

    // 条件1: 燃料达到难度阈值
    if (game_data.fuel_progress >= fuel_threshold) {
        Serial.printf("🚀 燃料达到%s模式阈值(%lu%%)，启动火箭发射动画\n",
                     get_difficulty_name(game_data.difficulty), fuel_threshold);
        return true;
    }

    // 条件2: 游戏时长超过10分钟
    if (game_data.game_time_ms >= 600000) { // 10分钟
        Serial.println("游戏时长达到上限，启动火箭发射动画");
        return true;
    }

    // 条件3: 跳跃次数达到500次
    if (game_data.jump_count >= 500) {
        Serial.println("跳跃次数达到上限，启动火箭发射动画");
        return true;
    }

    return false;
}

// 游戏状态机
void game_state_machine(void) {
    static game_state_t last_state = GAME_STATE_IDLE;

    // 状态变化日志
    if (current_state != last_state) {
        Serial.printf("状态变化: %d -> %d\n", last_state, current_state);
        last_state = current_state;
    }

    // 根据当前状态执行相应逻辑
    switch (current_state) {
        case GAME_STATE_IDLE:
            // 待机状态，等待用户操作或跳跃启动
            break;

        case GAME_STATE_DIFFICULTY_SELECT:
            // 难度选择状态，等待用户选择难度
            break;

        case GAME_STATE_PLAYING:
            // 游戏进行中，更新数据
            game_update_data();

            // 检查目标达成情况
            game_target_monitor_check();

            // 检查是否应该启动火箭发射
            if (should_start_rocket_launch()) {
                Serial.println("满足发射条件，启动火箭发射动画");
                current_state = GAME_STATE_LAUNCHING;
                // 播放火箭发射音效
                play_sound_effect(SOUND_ROCKET_LAUNCH);
            }
            break;

        case GAME_STATE_PAUSED:
            // 暂停状态，保持数据不变
            break;

        case GAME_STATE_RESET_CONFIRM:
            // 重置确认状态，等待用户确认
            break;

        case GAME_STATE_LAUNCHING:
            // 火箭发射动画状态，等待动画完成
            // 动画完成后自动切换到结算状态
            // 这个逻辑在display.cpp中处理
            break;

        case GAME_STATE_RESULT:
            // 结算状态，显示结果
            // 可以添加自动返回待机的逻辑（如30秒后自动返回）
            static uint32_t result_start_time = 0;
            if (result_start_time == 0) {
                result_start_time = millis();
            }

            // 30秒后自动返回待机状态
            if (millis() - result_start_time >= 30000) {
                Serial.println("结算界面超时，自动返回待机");
                current_state = GAME_STATE_IDLE;
                result_start_time = 0;
            }
            break;

        default:
            Serial.printf("未知游戏状态: %d\n", current_state);
            current_state = GAME_STATE_IDLE;
            break;
    }
}

// 游戏主任务
void game_task(void* pvParameters) {
    Serial.println("游戏任务启动");
    
    // 初始化游戏状态
    game_reset();
    
    while (1) {
        // 执行状态机
        game_state_machine();
        
        // 处理按钮事件
        button_event_t button_event = button_get_event();
        if (button_event != BUTTON_EVENT_NONE) {
            handle_button_event(button_event);
        }
        
        // 任务延时
        delay(50); // 20Hz更新频率
    }
}

// ==================== 难度选择相关函数 ====================

// 初始化难度选择
void difficulty_select_init(void) {
    selected_difficulty = DIFFICULTY_NORMAL; // 默认选择普通难度
    Serial.println("🎯 进入难度选择界面");
    Serial.printf("   默认难度: %s\n", get_difficulty_name(selected_difficulty));
}

// 切换到下一个难度
void difficulty_select_next(void) {
    switch (selected_difficulty) {
        case DIFFICULTY_EASY:
            selected_difficulty = DIFFICULTY_NORMAL;
            break;
        case DIFFICULTY_NORMAL:
            selected_difficulty = DIFFICULTY_HARD;
            break;
        case DIFFICULTY_HARD:
            selected_difficulty = DIFFICULTY_EASY;
            break;
    }

    Serial.printf("🎯 难度切换: %s (燃料阈值: %lu%%)\n",
                 get_difficulty_name(selected_difficulty),
                 get_difficulty_fuel_threshold(selected_difficulty));

    // 播放选择音效
    play_sound_effect(SOUND_DIFFICULTY_SELECT);
}

// 确认难度选择并开始游戏
void difficulty_select_confirm(void) {
    Serial.printf("🎯 确认难度: %s\n", get_difficulty_name(selected_difficulty));
    Serial.printf("   燃料发射阈值: %lu%%\n", get_difficulty_fuel_threshold(selected_difficulty));

    // 保存难度到游戏数据
    game_data.difficulty = selected_difficulty;

    // 播放确认音效
    play_sound_effect(SOUND_DIFFICULTY_CONFIRM);

    // 开始游戏
    game_start();
}

// 获取难度对应的燃料阈值
uint32_t get_difficulty_fuel_threshold(game_difficulty_t difficulty) {
#ifdef JUMPING_ROCKET_V3
    // V3系统使用基于难度的燃料阈值计算
    return V3Config::getFuelThresholdForDifficulty(difficulty);
#else
    switch (difficulty) {
        case DIFFICULTY_EASY:
            return 60;  // 简单模式：60%燃料触发
        case DIFFICULTY_NORMAL:
            return 80;  // 普通模式：80%燃料触发
        case DIFFICULTY_HARD:
            return 100; // 困难模式：100%燃料触发
        default:
            return 80;  // 默认普通模式
    }
#endif
}

// 获取难度名称
const char* get_difficulty_name(game_difficulty_t difficulty) {
    switch (difficulty) {
        case DIFFICULTY_EASY:
            return "简单";
        case DIFFICULTY_NORMAL:
            return "普通";
        case DIFFICULTY_HARD:
            return "困难";
        default:
            return "普通";
    }
}

// ==================== 目标监控功能 ====================

// 目标监控初始化
void game_target_monitor_init(void) {
    game_data.target_jumps_achieved = false;
    game_data.target_time_achieved = false;
    game_data.target_calories_achieved = false;
    game_data.last_target_check_time = millis();

    // 初始化闪烁效果状态
    game_data.target_flash_active = false;
    game_data.target_flash_start_time = 0;
    game_data.target_flash_duration = 3000; // 3秒闪烁

    Serial.println("目标监控初始化完成");
}

// 检查目标是否启用
bool is_target_enabled(void) {
#ifdef JUMPING_ROCKET_V3
    // 从V3系统获取目标设置
    extern DataManagerV3 dataManagerV3;
    if (dataManagerV3.isInitialized()) {
        return dataManagerV3.getTargetSettings().enabled;
    }
#endif
    return false; // V3系统未启用或未初始化时禁用目标
}

// 计算当前卡路里消耗
float calculate_current_calories(void) {
    // 基于跳跃次数和时间计算卡路里
    // 这里使用简化的计算公式
    float calories_per_jump = 0.5f; // 每次跳跃消耗0.5卡路里
    float calories_per_minute = 5.0f; // 每分钟基础消耗5卡路里

    float jump_calories = game_data.jump_count * calories_per_jump;
    float time_calories = (game_data.game_time_ms / 1000.0f / 60.0f) * calories_per_minute;

    return jump_calories + time_calories;
}

// 目标监控检查
void game_target_monitor_check(void) {
    // 限制检查频率，每500ms检查一次
    uint32_t current_time = millis();
    if (current_time - game_data.last_target_check_time < 500) {
        return;
    }
    game_data.last_target_check_time = current_time;

    // 检查目标是否启用
    if (!is_target_enabled()) {
        return;
    }

    // 从V3系统获取目标设置
#ifdef JUMPING_ROCKET_V3
    // V3系统启用时使用基于当前游戏难度的目标设置
    uint32_t target_jumps = V3Config::getTargetJumpsForDifficulty(game_data.difficulty);
    uint32_t target_time = V3Config::getTargetTimeForDifficulty(game_data.difficulty);
    
    // 卡路里目标保持从TargetSettings获取，或使用默认值
    float target_calories = 30.0f;
    extern DataManagerV3 dataManagerV3;
    if (dataManagerV3.isInitialized()) {
        const TargetSettingsV3& target_settings = dataManagerV3.getTargetSettings();
        target_calories = target_settings.target_calories;
    }
#else
    // V3系统未启用时使用默认目标值
    uint32_t target_jumps = 50;      // 目标跳跃次数
    uint32_t target_time = 30;       // 目标时间(秒)
    float target_calories = 30.0f;   // 目标卡路里
#endif

    // 检查跳跃目标
    if (!game_data.target_jumps_achieved && game_data.jump_count >= target_jumps) {
        game_data.target_jumps_achieved = true;
        Serial.printf("🎯 跳跃目标达成! 当前: %d, 目标: %d\n", game_data.jump_count, target_jumps);
        start_target_achievement_flash();
        play_sound_effect(SOUND_TARGET_ACHIEVED);
    }

    // 检查时间目标
    uint32_t current_time_seconds = game_data.game_time_ms / 1000;
    if (!game_data.target_time_achieved && current_time_seconds >= target_time) {
        game_data.target_time_achieved = true;
        Serial.printf("🎯 时间目标达成! 当前: %d秒, 目标: %d秒\n", current_time_seconds, target_time);
        start_target_achievement_flash();
        play_sound_effect(SOUND_TARGET_ACHIEVED);
    }

    // 检查卡路里目标
    float current_calories = calculate_current_calories();
    if (!game_data.target_calories_achieved && current_calories >= target_calories) {
        game_data.target_calories_achieved = true;
        Serial.printf("🎯 卡路里目标达成! 当前: %.1f, 目标: %.1f\n", current_calories, target_calories);
        start_target_achievement_flash();
        play_sound_effect(SOUND_TARGET_ACHIEVED);
    }
}

// ==================== 目标达成提醒效果 ====================

// 启动目标达成屏幕闪烁效果
void start_target_achievement_flash(void) {
    game_data.target_flash_active = true;
    game_data.target_flash_start_time = millis();
    Serial.println("🌟 启动目标达成屏幕闪烁效果");
}

// 检查屏幕闪烁是否激活
bool is_target_flash_active(void) {
    if (!game_data.target_flash_active) {
        return false;
    }

    // 检查闪烁是否超时
    uint32_t elapsed = millis() - game_data.target_flash_start_time;
    if (elapsed >= game_data.target_flash_duration) {
        game_data.target_flash_active = false;
        Serial.println("🌟 目标达成闪烁效果结束");
        return false;
    }

    return true;
}

// 检查当前时刻是否应该显示闪烁
bool should_screen_flash_now(void) {
    if (!is_target_flash_active()) {
        return false;
    }

    // 快速闪烁：每200ms切换一次显示状态
    uint32_t elapsed = millis() - game_data.target_flash_start_time;
    return (elapsed / 200) % 2 == 0;
}
