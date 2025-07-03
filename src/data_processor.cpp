#include "jumping_rocket_simple.h"

// 数据统计结构
typedef struct {
    uint32_t total_games;           // 总游戏次数
    uint32_t total_jumps;           // 总跳跃次数
    uint32_t total_time_ms;         // 总游戏时长
    uint32_t best_height;           // 最佳飞行高度
    uint32_t best_jumps;            // 最佳跳跃次数
    uint32_t best_time_ms;          // 最佳游戏时长
} game_statistics_t;

static game_statistics_t game_stats = {0};

// 跳跃频率分析
#define JUMP_FREQUENCY_WINDOW   5000    // 5秒窗口
#define MAX_JUMP_RECORDS        50      // 最大跳跃记录数

typedef struct {
    uint32_t timestamp;
    bool valid;
} jump_record_t;

static jump_record_t jump_records[MAX_JUMP_RECORDS];
static int jump_record_index = 0;

// 添加跳跃记录
void add_jump_record(uint32_t timestamp) {
    jump_records[jump_record_index].timestamp = timestamp;
    jump_records[jump_record_index].valid = true;
    
    jump_record_index = (jump_record_index + 1) % MAX_JUMP_RECORDS;
    
    Serial.printf("添加跳跃记录: %lu\n", timestamp);
}

// 计算跳跃频率（每分钟跳跃次数）
float calculate_jump_frequency(void) {
    uint32_t current_time = millis();
    uint32_t valid_jumps = 0;
    
    // 统计窗口内的有效跳跃
    for (int i = 0; i < MAX_JUMP_RECORDS; i++) {
        if (jump_records[i].valid && 
            (current_time - jump_records[i].timestamp) <= JUMP_FREQUENCY_WINDOW) {
            valid_jumps++;
        }
    }
    
    // 计算每分钟跳跃频率
    float frequency = (float)valid_jumps * 60000.0f / JUMP_FREQUENCY_WINDOW;
    
    Serial.printf("跳跃频率: %.1f 次/分钟\n", frequency);
    return frequency;
}

// 清理过期的跳跃记录
void cleanup_jump_records(void) {
    uint32_t current_time = millis();
    
    for (int i = 0; i < MAX_JUMP_RECORDS; i++) {
        if (jump_records[i].valid && 
            (current_time - jump_records[i].timestamp) > JUMP_FREQUENCY_WINDOW * 2) {
            jump_records[i].valid = false;
        }
    }
}

// 计算运动强度
float calculate_exercise_intensity(void) {
    float frequency = calculate_jump_frequency();
    uint32_t duration_seconds = game_data.game_time_ms / 1000;
    
    if (duration_seconds == 0) return 0.0f;
    
    // 基于跳跃频率和持续时间计算强度
    float intensity = frequency * (duration_seconds / 60.0f) / 10.0f;
    
    // 限制强度范围 0-10
    if (intensity > 10.0f) intensity = 10.0f;
    
    Serial.printf("运动强度: %.1f\n", intensity);
    return intensity;
}

// 计算卡路里消耗（估算）
float calculate_calories_burned(void) {
    // 基于跳跃次数和时长的简单卡路里计算
    // 假设每次跳跃消耗0.5卡路里，每分钟基础消耗3卡路里
    
    float calories_from_jumps = game_data.jump_count * 0.5f;
    float calories_from_time = (game_data.game_time_ms / 60000.0f) * 3.0f;
    
    float total_calories = calories_from_jumps + calories_from_time;
    
    Serial.printf("估算卡路里消耗: %.1f\n", total_calories);
    return total_calories;
}

// 更新游戏统计
void update_game_statistics(void) {
    game_stats.total_games++;
    game_stats.total_jumps += game_data.jump_count;
    game_stats.total_time_ms += game_data.game_time_ms;
    
    // 更新最佳记录
    if (game_data.flight_height > game_stats.best_height) {
        game_stats.best_height = game_data.flight_height;
    }
    
    if (game_data.jump_count > game_stats.best_jumps) {
        game_stats.best_jumps = game_data.jump_count;
    }
    
    if (game_data.game_time_ms > game_stats.best_time_ms) {
        game_stats.best_time_ms = game_data.game_time_ms;
    }
    
    Serial.printf("统计更新 - 总游戏: %lu, 总跳跃: %lu, 最佳高度: %lu\n", 
             game_stats.total_games, game_stats.total_jumps, game_stats.best_height);
}

// 生成运动报告
void generate_exercise_report(char* report_buffer, size_t buffer_size) {
    if (!report_buffer || buffer_size == 0) return;
    
    float frequency = calculate_jump_frequency();
    float intensity = calculate_exercise_intensity();
    float calories = calculate_calories_burned();
    
    snprintf(report_buffer, buffer_size,
        "=== 运动报告 ===\n"
        "跳跃次数: %lu\n"
        "运动时长: %lu分%lu秒\n"
        "跳跃频率: %.1f次/分钟\n"
        "运动强度: %.1f/10\n"
        "卡路里: %.1f千卡\n"
        "飞行高度: %lu米\n",
        game_data.jump_count,
        game_data.game_time_ms / 60000,
        (game_data.game_time_ms % 60000) / 1000,
        frequency,
        intensity,
        calories,
        game_data.flight_height
    );
}

// 数据处理任务初始化
void data_processor_init(void) {
    // 清空跳跃记录
    memset(jump_records, 0, sizeof(jump_records));
    jump_record_index = 0;
    
    Serial.println("数据处理器初始化完成");
}
