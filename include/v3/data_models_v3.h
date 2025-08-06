#ifndef DATA_MODELS_V3_H
#define DATA_MODELS_V3_H

#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>
#include "board_config_v3.h"

// 游戏会话数据结构
struct GameSessionV3 {
    String start_time;          // 开始时间 "HH:MM:SS"
    uint32_t duration;          // 持续时间(秒)
    game_difficulty_t difficulty; // 游戏难度
    uint32_t jump_count;        // 跳跃次数
    float calories;             // 消耗卡路里
    float max_height;           // 最大跳跃高度
    float avg_frequency;        // 平均跳跃频率
    uint16_t score;             // 游戏得分
    bool target_achieved;       // 是否达成目标
    
    // 构造函数
    GameSessionV3() : 
        start_time("00:00:00"),
        duration(0),
        difficulty(DIFFICULTY_NORMAL),
        jump_count(0),
        calories(0.0f),
        max_height(0.0f),
        avg_frequency(0.0f),
        score(0),
        target_achieved(false) {}
    
    // 序列化到JSON
    void toJson(JsonObject& obj) const;
    
    // 从JSON反序列化
    bool fromJson(const JsonObject& obj);
    
    // 计算得分
    uint16_t calculateScore() const;
    
    // 计算卡路里
    float calculateCalories() const;
};

// 每日数据汇总结构
struct DailyTotalV3 {
    uint32_t total_jumps;       // 总跳跃次数
    float total_calories;       // 总卡路里
    uint32_t total_duration;    // 总时长(秒)
    uint8_t session_count;      // 游戏次数
    uint16_t best_score;        // 最佳得分
    uint8_t targets_achieved;   // 达成目标次数
    
    // 构造函数
    DailyTotalV3() : 
        total_jumps(0),
        total_calories(0.0f),
        total_duration(0),
        session_count(0),
        best_score(0),
        targets_achieved(0) {}
    
    // 序列化到JSON
    void toJson(JsonObject& obj) const;
    
    // 从JSON反序列化
    bool fromJson(const JsonObject& obj);
    
    // 重置数据
    void reset();
    
    // 添加会话数据
    void addSession(const GameSessionV3& session);
};

// 每日数据结构
struct DailyDataV3 {
    String date;                        // 日期 "YYYY-MM-DD"
    std::vector<GameSessionV3> sessions; // 游戏会话列表
    DailyTotalV3 daily_total;           // 每日汇总
    
    // 构造函数
    DailyDataV3() : date("") {}
    
    DailyDataV3(const String& date_str) : date(date_str) {}
    
    // 序列化到JSON
    String toJsonString() const;
    
    // 从JSON字符串反序列化
    bool fromJsonString(const String& json_str);
    
    // 添加游戏会话
    void addSession(const GameSessionV3& session);
    
    // 更新每日汇总
    void updateDailyTotal();
    
    // 获取会话数量
    size_t getSessionCount() const { return sessions.size(); }
    
    // 检查是否为空
    bool isEmpty() const { return sessions.empty(); }
};

// 系统配置结构
struct SystemConfigV3 {
    uint8_t volume;             // 音量 0-100
    uint8_t brightness;         // 亮度 0-100 (预留)
    uint32_t sleep_timeout;     // 睡眠超时(秒) (预留)
    game_difficulty_t default_difficulty; // 默认难度
    bool auto_sleep;            // 自动睡眠 (预留)
    bool sound_enabled;         // 声音开关
    bool vibration_enabled;     // 震动开关 (预留)
    String language;            // 语言设置 (预留)
    
    // 用户自定义基准难度配置（对应普通难度基准）
    uint32_t base_target_jumps;     // 基准目标跳跃次数（普通难度基准）
    uint32_t base_target_time;      // 基准目标时间(秒)（普通难度基准）

    // 构造函数
    SystemConfigV3() :
        volume(80),
        brightness(70),
        sleep_timeout(300),
        default_difficulty(DIFFICULTY_NORMAL),
        auto_sleep(false),
        sound_enabled(true),
        vibration_enabled(false),
        language("en-US"),
        base_target_jumps(20),
        base_target_time(60) {}
    
    // 序列化到JSON
    String toJsonString() const;
    
    // 从JSON字符串反序列化
    bool fromJsonString(const String& json_str);
    
    // 验证配置有效性
    bool validate();
    
    // 重置为默认值
    void resetToDefault();
};

// 历史统计数据结构
struct HistoryStatsV3 {
    uint32_t total_games;       // 总游戏次数
    uint32_t total_jumps;       // 总跳跃次数
    uint32_t total_time;        // 总游戏时间(秒)
    float total_calories;       // 总卡路里
    uint16_t best_score;        // 历史最佳得分
    uint16_t best_jumps;        // 单次最多跳跃
    String best_date;           // 最佳记录日期
    uint8_t streak_days;        // 连续运动天数
    
    // 构造函数
    HistoryStatsV3() : 
        total_games(0),
        total_jumps(0),
        total_time(0),
        total_calories(0.0f),
        best_score(0),
        best_jumps(0),
        best_date(""),
        streak_days(0) {}
    
    // 序列化到JSON
    String toJsonString() const;
    
    // 从JSON字符串反序列化
    bool fromJsonString(const String& json_str);
    
    // 更新统计数据
    void updateWithDailyData(const DailyDataV3& daily_data);
    
    // 重置统计数据
    void reset();
};

// 目标设置结构
struct TargetSettingsV3 {
    bool enabled;               // 是否启用目标
    uint32_t target_jumps;      // 目标跳跃次数
    uint32_t target_time;       // 目标时间(秒)
    float target_calories;      // 目标卡路里
    bool daily_target;          // 是否为每日目标
    
    // 构造函数
    TargetSettingsV3() :
        enabled(false),
        target_jumps(50),
        target_time(30),
        target_calories(30.0f),
        daily_target(true) {}
    
    // 序列化到JSON
    void toJson(JsonObject& obj) const;
    
    // 从JSON反序列化
    bool fromJson(const JsonObject& obj);
    
    // 检查是否达成目标
    bool isTargetAchieved(const GameSessionV3& session) const;
    bool isTargetAchieved(const DailyDataV3& daily_data) const;
};

// 工具函数
namespace DataUtilsV3 {
    // 时间格式化
    String formatTime(uint32_t seconds);
    String getCurrentTimeString();
    String getCurrentDateString();
    String getDateString(int days_offset = 0);
    
    // 数据验证
    bool isValidDate(const String& date);
    bool isValidTime(const String& time);
    
    // 数据转换
    float jumpsToCalories(uint32_t jumps, game_difficulty_t difficulty);
    uint16_t calculateGameScore(uint32_t jumps, uint32_t time, game_difficulty_t difficulty);
    
    // 统计计算
    float calculateAvgFrequency(uint32_t jumps, uint32_t time);
    float calculateMaxHeight(float avg_frequency); // 估算最大高度
}

#endif // DATA_MODELS_V3_H
