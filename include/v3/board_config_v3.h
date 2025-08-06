#ifndef BOARD_CONFIG_V3_H
#define BOARD_CONFIG_V3_H

#include <Arduino.h>

// 确保包含V2.0的类型定义
#ifndef JUMPING_ROCKET_SIMPLE_H
#include "jumping_rocket_simple.h"
#endif

// V3.0版本标识
#define JUMPING_ROCKET_VERSION_MAJOR 3
#define JUMPING_ROCKET_VERSION_MINOR 0
#define JUMPING_ROCKET_VERSION_PATCH 0
#define JUMPING_ROCKET_VERSION_STRING "3.0.0"

// ESP32-C3 Mini V3.0 专用配置
#ifdef BOARD_ESP32_C3
    #define BOARD_NAME_V3 "ESP32-C3 Mini V3.0"
    #define BOARD_TYPE_ESP32_C3_V3 1
    
    // 使用esp32c3dev环境的引脚配置
    #ifndef I2C_SCL_PIN
    #define I2C_SCL_PIN                 8     // I2C SCL引脚
    #endif
    
    #ifndef I2C_SDA_PIN
    #define I2C_SDA_PIN                 9     // I2C SDA引脚
    #endif
    
    #ifndef BUTTON_PIN
    #define BUTTON_PIN                  3     // 按钮引脚
    #endif
    
    #ifndef BUZZER_PIN
    #define BUZZER_PIN                  4     // 蜂鸣器引脚
    #endif
    
    // V3.0新增引脚定义
    #define STATUS_LED_PIN              2     // 状态指示LED
    #define BATTERY_ADC_PIN             0     // 电池电压检测(预留)
    
    // ESP32-C3 V3.0 特定配置
    #define I2C_FREQUENCY               100000  // 100kHz，稳定性优先
    #define BUTTON_PULL_MODE            INPUT_PULLDOWN
    #define BUTTON_ACTIVE_LEVEL         HIGH
    #ifndef LED_BUILTIN_PIN
    #define LED_BUILTIN_PIN             STATUS_LED_PIN
    #endif
    
    // V3.0功能特性开关
    #define V3_FILE_SYSTEM_ENABLED      1     // 文件系统功能
    #define V3_DIFFICULTY_SYSTEM_ENABLED 1    // 难度选择系统
    #define V3_HISTORY_DATA_ENABLED     1     // 历史数据功能
    #define V3_TARGET_TIMER_ENABLED     1     // 目标计时功能
    #define V3_ENHANCED_STATS_ENABLED   1     // 增强统计功能
    #define V3_SETTINGS_MENU_ENABLED    1     // 设置菜单功能
    
    // 暂时禁用的功能（按要求）
    #define V3_POWER_MANAGEMENT_ENABLED 0     // 电源管理（暂不实现）
    #define V3_DEEP_SLEEP_ENABLED       0     // 深度睡眠（暂不实现）
    #define V3_BATTERY_MONITOR_ENABLED  0     // 电池监测（暂不实现）
    
    // V3.0数据配置
    #define V3_MAX_DAILY_SESSIONS       20    // 每日最大游戏次数
    #define V3_HISTORY_DAYS             7     // 历史记录天数
    #define V3_MAX_FILENAME_LENGTH      32    // 最大文件名长度
    
    // 调试信息
    #define BOARD_DEBUG_INFO_V3() do { \
        Serial.println("Jumping Rocket V3.0 Configuration:"); \
        Serial.printf("   Version: %s\n", JUMPING_ROCKET_VERSION_STRING); \
        Serial.printf("   Board: %s\n", BOARD_NAME_V3); \
        Serial.printf("   I2C: SDA=GPIO%d, SCL=GPIO%d, Freq=%dHz\n", \
                     I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY); \
        Serial.printf("   Button: GPIO%d (%s trigger)\n", \
                     BUTTON_PIN, (BUTTON_ACTIVE_LEVEL == HIGH) ? "HIGH" : "LOW"); \
        Serial.printf("   Buzzer: GPIO%d\n", BUZZER_PIN); \
        Serial.printf("   Status LED: GPIO%d\n", STATUS_LED_PIN); \
        Serial.println("   Features:"); \
        Serial.printf("     File System: %s\n", V3_FILE_SYSTEM_ENABLED ? "Enabled" : "Disabled"); \
        Serial.printf("     Difficulty: %s\n", V3_DIFFICULTY_SYSTEM_ENABLED ? "Enabled" : "Disabled"); \
        Serial.printf("     History: %s\n", V3_HISTORY_DATA_ENABLED ? "Enabled" : "Disabled"); \
        Serial.printf("     Target Timer: %s\n", V3_TARGET_TIMER_ENABLED ? "Enabled" : "Disabled"); \
        Serial.printf("     Enhanced Stats: %s\n", V3_ENHANCED_STATS_ENABLED ? "Enabled" : "Disabled"); \
        Serial.printf("     Settings Menu: %s\n", V3_SETTINGS_MENU_ENABLED ? "Enabled" : "Disabled"); \
        Serial.printf("     Power Mgmt: %s\n", V3_POWER_MANAGEMENT_ENABLED ? "Enabled" : "Disabled(V3.1)"); \
    } while(0)

#else
    #error "V3.0 requires ESP32-C3 board configuration"
#endif

// V3.0游戏难度配置 (使用V2.0已有的定义)
// 注意：game_difficulty_t, DIFFICULTY_EASY, DIFFICULTY_NORMAL, DIFFICULTY_HARD 已在jumping_rocket_simple.h中定义
// 这里只定义V3.0特有的常量
#ifndef DIFFICULTY_COUNT
#define DIFFICULTY_COUNT 3
#endif

// 难度配置结构
typedef struct {
    const char* name_en;        // 英文名称
    float multiplier;           // 难度倍数
    uint32_t target_jumps;      // 目标跳跃次数
    uint32_t target_time;       // 目标时间(秒)
    uint32_t fuel_threshold;    // 燃料发射阈值(%)
    float score_multiplier;     // 得分倍数
} difficulty_config_t;

// 难度配置表
static const difficulty_config_t DIFFICULTY_CONFIGS[DIFFICULTY_COUNT] = {
    {"Easy",   0.8f, 10,  30, 60, 0.8f},   // Easy: 10 jumps, 30 seconds, 60% fuel
    {"Normal", 1.0f, 20,  60, 80, 1.0f},   // Normal: 20 jumps, 60 seconds, 80% fuel
    {"Hard",   1.2f, 30,  90, 100, 1.2f}   // Hard: 30 jumps, 90 seconds, 100% fuel
};

// V3.0状态定义
typedef enum {
    V3_STATE_BOOT = 0,          // 启动状态
    V3_STATE_MAIN_MENU,         // 主菜单
    V3_STATE_DIFFICULTY_SELECT, // 难度选择
    V3_STATE_GAME_READY,        // 游戏准备
    V3_STATE_GAME_PLAYING,      // 游戏进行中
    V3_STATE_GAME_RESULT,       // 游戏结果
    V3_STATE_HISTORY_VIEW,      // 历史数据查看
    V3_STATE_SETTINGS,          // 系统设置
    V3_STATE_TARGET_TIMER,      // 目标计时
    V3_STATE_COUNT
} v3_system_state_t;

// V3.0专用函数，避免与V2.0冲突
namespace V3Config {
    // 获取难度配置
    inline const difficulty_config_t* getDifficultyConfig(game_difficulty_t difficulty) {
        if (difficulty >= DIFFICULTY_COUNT) {
            return &DIFFICULTY_CONFIGS[DIFFICULTY_NORMAL];
        }
        return &DIFFICULTY_CONFIGS[difficulty];
    }

    // 获取难度名称
    inline const char* getDifficultyName(game_difficulty_t difficulty) {
        return getDifficultyConfig(difficulty)->name_en;
    }

    // 获取难度倍数
    inline float getDifficultyMultiplier(game_difficulty_t difficulty) {
        return getDifficultyConfig(difficulty)->multiplier;
    }

    // 其他方法声明（在config_v3.cpp中实现）
    const char* getDifficultyNameEn(game_difficulty_t difficulty);
    uint32_t getTargetJumps(game_difficulty_t difficulty);
    uint32_t getTargetTime(game_difficulty_t difficulty);
    uint32_t getFuelThreshold(game_difficulty_t difficulty);
    float getScoreMultiplier(game_difficulty_t difficulty);
    bool isValidDifficulty(game_difficulty_t difficulty);
    game_difficulty_t getNextDifficulty(game_difficulty_t current);
    game_difficulty_t getPreviousDifficulty(game_difficulty_t current);
    float calculateTargetProgress(game_difficulty_t difficulty, uint32_t jumps, uint32_t time_seconds);
    bool isTargetAchieved(game_difficulty_t difficulty, uint32_t jumps, uint32_t time_seconds);
    uint16_t calculateScore(game_difficulty_t difficulty, uint32_t jumps, uint32_t time_seconds, float avg_frequency);
    float calculateCalories(game_difficulty_t difficulty, uint32_t jumps, uint32_t time_seconds);
    String formatDifficultyInfo(game_difficulty_t difficulty);
    void printAllDifficultyConfigs();
    game_difficulty_t getRecommendedDifficulty(uint32_t total_games, uint32_t best_score);
    bool validateConfigs();
    const char* getConfigVersion();
    const char* getConfigUpdateTime();
    
    // 基于用户基准设置和难度的函数
    uint32_t getTargetJumpsForDifficulty(game_difficulty_t difficulty);
    uint32_t getTargetTimeForDifficulty(game_difficulty_t difficulty);
    uint32_t getFuelThresholdForDifficulty(game_difficulty_t difficulty);
    bool isTargetAchievedForDifficulty(game_difficulty_t difficulty, uint32_t jumps, uint32_t time_seconds);
    float calculateTargetProgressForDifficulty(game_difficulty_t difficulty, uint32_t jumps, uint32_t time_seconds);
}

#endif // BOARD_CONFIG_V3_H
