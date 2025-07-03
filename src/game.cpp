#include "jumping_rocket_simple.h"

// 全局游戏状态和数据
game_state_t current_state = GAME_STATE_IDLE;
game_data_t game_data = {0};

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
    
    // 重置游戏数据
    memset(&game_data, 0, sizeof(game_data_t));
    
    // 重置计时器
    game_start_time = 0;
    pause_start_time = 0;
    total_pause_time = 0;
    
    // 切换到待机状态
    current_state = GAME_STATE_IDLE;
    
    Serial.println("游戏重置完成");
}

// 游戏开始
void game_start(void) {
    Serial.println("🎮 开始游戏");

    // 记录精确的开始时间
    uint32_t current_time = get_time_ms();

    // 重置游戏数据（但保留可能已有的跳跃计数）
    uint32_t existing_jumps = game_data.jump_count;
    memset(&game_data, 0, sizeof(game_data_t));
    game_data.jump_count = existing_jumps; // 恢复跳跃计数

    // 设置开始时间
    game_start_time = current_time;
    total_pause_time = 0;

    // 切换到游戏状态
    current_state = GAME_STATE_PLAYING;

    Serial.printf("   游戏开始时间: %lu ms\n", game_start_time);
    Serial.printf("   初始跳跃计数: %lu\n", game_data.jump_count);
    Serial.printf("   暂停时间重置: %lu ms\n", total_pause_time);
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
    // 条件1: 燃料满格（达到100%）
    if (game_data.fuel_progress >= MAX_FUEL) {
        Serial.println("燃料满格，启动火箭发射动画");
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

        case GAME_STATE_PLAYING:
            // 游戏进行中，更新数据
            game_update_data();

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
