#include "v3/data_models_v3.h"
#include <math.h>

// GameSessionV3 实现
void GameSessionV3::toJson(JsonObject& obj) const {
    obj["start_time"] = start_time;
    obj["duration"] = duration;
    obj["difficulty"] = (int)difficulty;
    obj["jump_count"] = jump_count;
    obj["calories"] = calories;
    obj["max_height"] = max_height;
    obj["avg_frequency"] = avg_frequency;
    obj["score"] = score;
    obj["target_achieved"] = target_achieved;
}

bool GameSessionV3::fromJson(const JsonObject& obj) {
    if (!obj["start_time"].is<String>() || !obj["duration"].is<uint32_t>()) {
        return false;
    }
    
    start_time = obj["start_time"].as<String>();
    duration = obj["duration"];
    difficulty = (game_difficulty_t)obj["difficulty"].as<int>();
    jump_count = obj["jump_count"];
    calories = obj["calories"];
    max_height = obj["max_height"];
    avg_frequency = obj["avg_frequency"];
    score = obj["score"];
    target_achieved = obj["target_achieved"];
    
    return true;
}

uint16_t GameSessionV3::calculateScore() const {
    return DataUtilsV3::calculateGameScore(jump_count, duration, difficulty);
}

float GameSessionV3::calculateCalories() const {
    return DataUtilsV3::jumpsToCalories(jump_count, difficulty);
}

// DailyTotalV3 实现
void DailyTotalV3::toJson(JsonObject& obj) const {
    obj["total_jumps"] = total_jumps;
    obj["total_calories"] = total_calories;
    obj["total_duration"] = total_duration;
    obj["session_count"] = session_count;
    obj["best_score"] = best_score;
    obj["targets_achieved"] = targets_achieved;
}

bool DailyTotalV3::fromJson(const JsonObject& obj) {
    total_jumps = obj["total_jumps"];
    total_calories = obj["total_calories"];
    total_duration = obj["total_duration"];
    session_count = obj["session_count"];
    best_score = obj["best_score"];
    targets_achieved = obj["targets_achieved"];
    return true;
}

void DailyTotalV3::reset() {
    total_jumps = 0;
    total_calories = 0.0f;
    total_duration = 0;
    session_count = 0;
    best_score = 0;
    targets_achieved = 0;
}

void DailyTotalV3::addSession(const GameSessionV3& session) {
    total_jumps += session.jump_count;
    total_calories += session.calories;
    total_duration += session.duration;
    session_count++;
    
    if (session.score > best_score) {
        best_score = session.score;
    }
    
    if (session.target_achieved) {
        targets_achieved++;
    }
}

// DailyDataV3 实现
String DailyDataV3::toJsonString() const {
    JsonDocument doc;

    doc["date"] = date;

    JsonArray sessions_array = doc["sessions"].to<JsonArray>();
    for (const auto& session : sessions) {
        JsonObject session_obj = sessions_array.add<JsonObject>();
        session.toJson(session_obj);
    }

    JsonObject total_obj = doc["daily_total"].to<JsonObject>();
    daily_total.toJson(total_obj);
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool DailyDataV3::fromJsonString(const String& json_str) {
    JsonDocument doc;
    
    DeserializationError error = deserializeJson(doc, json_str);
    if (error) {
        Serial.printf("❌ DailyDataV3 JSON解析失败: %s\n", error.c_str());
        return false;
    }
    
    date = doc["date"].as<String>();
    
    // 解析会话数据
    sessions.clear();
    JsonArray sessions_array = doc["sessions"];
    for (JsonObject session_obj : sessions_array) {
        GameSessionV3 session;
        if (session.fromJson(session_obj)) {
            sessions.push_back(session);
        }
    }
    
    // 解析每日汇总
    JsonObject total_obj = doc["daily_total"];
    daily_total.fromJson(total_obj);
    
    return true;
}

void DailyDataV3::addSession(const GameSessionV3& session) {
    sessions.push_back(session);
    daily_total.addSession(session);
}

void DailyDataV3::updateDailyTotal() {
    daily_total.reset();
    for (const auto& session : sessions) {
        daily_total.addSession(session);
    }
}

// SystemConfigV3 实现
String SystemConfigV3::toJsonString() const {
    JsonDocument doc;
    
    doc["volume"] = volume;
    doc["brightness"] = brightness;
    doc["sleep_timeout"] = sleep_timeout;
    doc["default_difficulty"] = (int)default_difficulty;
    doc["auto_sleep"] = auto_sleep;
    doc["sound_enabled"] = sound_enabled;
    doc["vibration_enabled"] = vibration_enabled;
    doc["language"] = language;
    doc["version"] = JUMPING_ROCKET_VERSION_STRING;
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool SystemConfigV3::fromJsonString(const String& json_str) {
    JsonDocument doc;
    
    DeserializationError error = deserializeJson(doc, json_str);
    if (error) {
        Serial.printf("❌ SystemConfigV3 JSON解析失败: %s\n", error.c_str());
        return false;
    }
    
    volume = doc["volume"];
    brightness = doc["brightness"];
    sleep_timeout = doc["sleep_timeout"];
    default_difficulty = (game_difficulty_t)doc["default_difficulty"].as<int>();
    auto_sleep = doc["auto_sleep"];
    sound_enabled = doc["sound_enabled"];
    vibration_enabled = doc["vibration_enabled"];
    language = doc["language"].as<String>();
    
    return validate();
}

bool SystemConfigV3::validate() {
    bool valid = true;
    
    if (volume > 100) {
        volume = 100;
        valid = false;
    }
    
    if (brightness > 100) {
        brightness = 100;
        valid = false;
    }
    
    if (default_difficulty >= DIFFICULTY_COUNT) {
        default_difficulty = DIFFICULTY_NORMAL;
        valid = false;
    }
    
    if (sleep_timeout < 60 || sleep_timeout > 3600) {
        sleep_timeout = 300;
        valid = false;
    }
    
    return valid;
}

void SystemConfigV3::resetToDefault() {
    volume = 80;
    brightness = 70;
    sleep_timeout = 300;
    default_difficulty = DIFFICULTY_NORMAL;
    auto_sleep = false;
    sound_enabled = true;
    vibration_enabled = false;
    language = "zh-CN";
}

// HistoryStatsV3 实现
String HistoryStatsV3::toJsonString() const {
    JsonDocument doc;
    
    doc["total_games"] = total_games;
    doc["total_jumps"] = total_jumps;
    doc["total_time"] = total_time;
    doc["total_calories"] = total_calories;
    doc["best_score"] = best_score;
    doc["best_jumps"] = best_jumps;
    doc["best_date"] = best_date;
    doc["streak_days"] = streak_days;
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool HistoryStatsV3::fromJsonString(const String& json_str) {
    JsonDocument doc;
    
    DeserializationError error = deserializeJson(doc, json_str);
    if (error) {
        return false;
    }
    
    total_games = doc["total_games"];
    total_jumps = doc["total_jumps"];
    total_time = doc["total_time"];
    total_calories = doc["total_calories"];
    best_score = doc["best_score"];
    best_jumps = doc["best_jumps"];
    best_date = doc["best_date"].as<String>();
    streak_days = doc["streak_days"];
    
    return true;
}

void HistoryStatsV3::updateWithDailyData(const DailyDataV3& daily_data) {
    total_games += daily_data.daily_total.session_count;
    total_jumps += daily_data.daily_total.total_jumps;
    total_time += daily_data.daily_total.total_duration;
    total_calories += daily_data.daily_total.total_calories;
    
    if (daily_data.daily_total.best_score > best_score) {
        best_score = daily_data.daily_total.best_score;
        best_date = daily_data.date;
    }
    
    if (daily_data.daily_total.total_jumps > best_jumps) {
        best_jumps = daily_data.daily_total.total_jumps;
    }
}

void HistoryStatsV3::reset() {
    total_games = 0;
    total_jumps = 0;
    total_time = 0;
    total_calories = 0.0f;
    best_score = 0;
    best_jumps = 0;
    best_date = "";
    streak_days = 0;
}

// TargetSettingsV3 实现
void TargetSettingsV3::toJson(JsonObject& obj) const {
    obj["enabled"] = enabled;
    obj["target_jumps"] = target_jumps;
    obj["target_time"] = target_time;
    obj["target_calories"] = target_calories;
    obj["daily_target"] = daily_target;
}

bool TargetSettingsV3::fromJson(const JsonObject& obj) {
    enabled = obj["enabled"];
    target_jumps = obj["target_jumps"];
    target_time = obj["target_time"];
    target_calories = obj["target_calories"];
    daily_target = obj["daily_target"];
    return true;
}

bool TargetSettingsV3::isTargetAchieved(const GameSessionV3& session) const {
    if (!enabled) return false;
    
    return (session.jump_count >= target_jumps) ||
           (session.duration >= target_time) ||
           (session.calories >= target_calories);
}

bool TargetSettingsV3::isTargetAchieved(const DailyDataV3& daily_data) const {
    if (!enabled || !daily_target) return false;
    
    return (daily_data.daily_total.total_jumps >= target_jumps) ||
           (daily_data.daily_total.total_duration >= target_time) ||
           (daily_data.daily_total.total_calories >= target_calories);
}

// DataUtilsV3 命名空间实现
namespace DataUtilsV3 {
    
String formatTime(uint32_t seconds) {
    uint32_t hours = seconds / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;
    
    if (hours > 0) {
        return String(hours) + ":" + 
               (minutes < 10 ? "0" : "") + String(minutes) + ":" +
               (secs < 10 ? "0" : "") + String(secs);
    } else {
        return (minutes < 10 ? "0" : "") + String(minutes) + ":" +
               (secs < 10 ? "0" : "") + String(secs);
    }
}

String getCurrentTimeString() {
    // 尝试使用NTP同步的时间
    time_t now;
    struct tm timeinfo;

    time(&now);

    // 检查时间是否已同步
    if (now > 946684800) { // 2000-01-01 00:00:00 UTC
        localtime_r(&now, &timeinfo);

        char time_str[9];
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d",
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec);
        return String(time_str);
    } else {
        // 备用方案：基于millis()
        uint32_t ms = millis();
        uint32_t seconds = (ms / 1000) % 86400; // 一天的秒数

        uint32_t hours = seconds / 3600;
        uint32_t minutes = (seconds % 3600) / 60;
        uint32_t secs = seconds % 60;

        char time_str[9];
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, secs);
        return String(time_str);
    }
}

String getCurrentDateString() {
    return getDateString(0); // 获取今天的日期
}

String getDateString(int days_offset) {
    // 尝试使用NTP同步的时间
    time_t now;
    struct tm timeinfo;

    time(&now);

    // 检查时间是否已同步（时间戳大于2000年1月1日）
    if (now > 946684800) { // 2000-01-01 00:00:00 UTC
        // 添加天数偏移
        now += (days_offset * 24 * 60 * 60);

        // 转换为本地时间
        localtime_r(&now, &timeinfo);

        char date_str[11];
        snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d",
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday);

        return String(date_str);
    } else {
        // 如果NTP时间未同步，使用备用的基于millis()的计算
        Serial.println("⚠️ NTP时间未同步，使用备用日期计算");

        static uint32_t start_day_millis = 0;
        static bool initialized = false;

        if (!initialized) {
            start_day_millis = millis();
            initialized = true;
            Serial.printf("📅 备用日期系统初始化，启动时间: %lu ms\n", start_day_millis);
        }

        // 计算从启动到现在经过的天数
        uint32_t current_millis = millis();
        uint32_t elapsed_days = (current_millis - start_day_millis) / (24UL * 60UL * 60UL * 1000UL);

        // 基准日期：2025-07-06
        uint32_t base_year = 2025;
        uint32_t base_month = 7;
        uint32_t base_day = 6;

        // 计算目标日期
        int32_t target_day = base_day + elapsed_days + days_offset;
        uint32_t year = base_year;
        uint32_t month = base_month;

        // 简单的日期计算
        while (target_day > 30) {
            target_day -= 30;
            month++;
            if (month > 12) {
                month = 1;
                year++;
            }
        }

        while (target_day <= 0) {
            target_day += 30;
            month--;
            if (month == 0) {
                month = 12;
                year--;
            }
        }

        char date_str[11];
        snprintf(date_str, sizeof(date_str), "%04u-%02u-%02u", year, month, target_day);

        return String(date_str);
    }
}

bool isValidDate(const String& date) {
    return date.length() == 10 && 
           date.charAt(4) == '-' && 
           date.charAt(7) == '-';
}

bool isValidTime(const String& time) {
    return time.length() == 8 && 
           time.charAt(2) == ':' && 
           time.charAt(5) == ':';
}

float jumpsToCalories(uint32_t jumps, game_difficulty_t difficulty) {
    // 基础卡路里计算：每次跳跃约0.5卡路里
    float base_calories = jumps * 0.5f;
    float multiplier = V3Config::getDifficultyMultiplier(difficulty);
    return base_calories * multiplier;
}

uint16_t calculateGameScore(uint32_t jumps, uint32_t time, game_difficulty_t difficulty) {
    if (time == 0) return 0;

    // 基础得分：跳跃次数 * 难度倍数
    float base_score = jumps * V3Config::getDifficultyMultiplier(difficulty);
    
    // 时间奖励：时间越短得分越高
    float time_bonus = 1.0f + (300.0f - min((uint32_t)time, (uint32_t)300)) / 300.0f;
    
    return (uint16_t)(base_score * time_bonus);
}

float calculateAvgFrequency(uint32_t jumps, uint32_t time) {
    if (time == 0) return 0.0f;
    return (float)jumps / time;
}

float calculateMaxHeight(float avg_frequency) {
    // 根据频率估算最大跳跃高度（简单物理模型）
    // 假设跳跃频率与高度有正相关关系
    return avg_frequency * 0.3f + 0.2f; // 基础高度0.2m + 频率影响
}

} // namespace DataUtilsV3
