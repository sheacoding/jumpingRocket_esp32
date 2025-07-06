#include "v3/data_manager_v3.h"
#include <time.h>

// 全局数据管理器实例
DataManagerV3 dataManagerV3;

DataManagerV3::DataManagerV3() : 
    fs(nullptr), 
    initialized(false),
    current_date("") {
}

DataManagerV3::~DataManagerV3() {
    deinit();
}

bool DataManagerV3::init(FileSystemV3* filesystem) {
    Serial.println("📊 初始化V3.0数据管理器...");

    if (initialized) {
        Serial.println("数据管理器已经初始化");
        return true;
    }

    if (!filesystem || !filesystem->isAvailable()) {
        Serial.println("❌ 文件系统不可用");
        return false;
    }

    fs = filesystem;

    // 初始化NTP时间同步
    initNTPTime();
    
    // 加载系统配置
    if (!loadSystemConfig()) {
        Serial.println("⚠️ 使用默认系统配置");
        system_config.resetToDefault();
        saveSystemConfig(system_config);
    }
    
    // 加载目标设置
    if (!loadTargetSettings()) {
        Serial.println("⚠️ 使用默认目标设置");
        resetTargetSettings();
    } else {
        // 临时方案：强制重置目标设置以使用新的默认值（30秒）
        Serial.println("🔄 重置目标设置以使用新的默认值");
        resetTargetSettings();
    }
    
    // 加载历史统计
    if (!loadHistoryStats()) {
        Serial.println("⚠️ 初始化历史统计数据");
        resetHistoryStats();
    }
    
    // 更新当前日期并加载当日数据
    updateCurrentDate();
    if (!loadCurrentDayData()) {
        Serial.println("📅 创建新的每日数据");
        current_day_data = DailyDataV3(current_date);
        saveCurrentDayData();
    }
    
    initialized = true;
    Serial.println("✅ V3.0数据管理器初始化成功");
    
    printDataSummary();
    return true;
}

void DataManagerV3::deinit() {
    if (initialized) {
        // 保存当前数据
        saveCurrentDayData();
        saveSystemConfig(system_config);
        saveHistoryStats();
        saveTargetSettings(target_settings);
        
        initialized = false;
        fs = nullptr;
        Serial.println("数据管理器已关闭");
    }
}

bool DataManagerV3::saveSystemConfig(const SystemConfigV3& config) {
    if (!fs || !fs->isAvailable()) return false;
    
    system_config = config;
    String json_data = config.toJsonString();
    
    bool success = fs->writeFile(V3_CONFIG_FILE, json_data);
    if (success) {
        Serial.println("✅ 系统配置保存成功");
    } else {
        Serial.println("❌ 系统配置保存失败");
    }
    
    return success;
}

bool DataManagerV3::loadSystemConfig() {
    if (!fs || !fs->isAvailable()) return false;
    
    if (!fs->fileExists(V3_CONFIG_FILE)) {
        Serial.println("⚠️ 系统配置文件不存在");
        return false;
    }
    
    String json_data = fs->readFile(V3_CONFIG_FILE);
    if (json_data.isEmpty()) {
        Serial.println("❌ 系统配置文件为空");
        return false;
    }
    
    bool success = system_config.fromJsonString(json_data);
    if (success) {
        Serial.println("✅ 系统配置加载成功");
    } else {
        Serial.println("❌ 系统配置解析失败");
    }
    
    return success;
}

void DataManagerV3::resetSystemConfig() {
    system_config.resetToDefault();
    saveSystemConfig(system_config);
    Serial.println("🔄 系统配置已重置为默认值");
}

bool DataManagerV3::saveGameSession(const GameSessionV3& session) {
    if (!initialized) return false;

    // 检查并更新当前日期
    updateCurrentDate();

    // 添加到当日数据
    current_day_data.addSession(session);

    // 保存当日数据
    bool success = saveCurrentDayData();
    
    if (success) {
        // 更新历史统计
        updateHistoryStats();
        
        Serial.printf("✅ 游戏会话保存成功: %d次跳跃, %.1f卡路里, %d分\n",
                     session.jump_count, session.calories, session.score);
    } else {
        Serial.println("❌ 游戏会话保存失败");
    }
    
    return success;
}

GameSessionV3 DataManagerV3::createGameSession(game_difficulty_t difficulty, 
                                              uint32_t jump_count, 
                                              uint32_t duration) {
    GameSessionV3 session;
    
    session.start_time = DataUtilsV3::getCurrentTimeString();
    session.duration = duration;
    session.difficulty = difficulty;
    session.jump_count = jump_count;
    session.calories = DataUtilsV3::jumpsToCalories(jump_count, difficulty);
    session.avg_frequency = DataUtilsV3::calculateAvgFrequency(jump_count, duration);
    session.max_height = DataUtilsV3::calculateMaxHeight(session.avg_frequency);
    session.score = DataUtilsV3::calculateGameScore(jump_count, duration, difficulty);
    session.target_achieved = isSessionTargetAchieved(session);
    
    return session;
}

bool DataManagerV3::loadDailyData(const String& date, DailyDataV3& data) {
    if (!fs || !fs->isAvailable()) return false;
    
    String file_path = fs->getDailyDataPath(date);
    
    if (!fs->fileExists(file_path)) {
        Serial.printf("⚠️ 日期 %s 的数据文件不存在\n", date.c_str());
        return false;
    }
    
    String json_data = fs->readFile(file_path);
    if (json_data.isEmpty()) {
        Serial.printf("❌ 日期 %s 的数据文件为空\n", date.c_str());
        return false;
    }
    
    bool success = data.fromJsonString(json_data);
    if (success) {
        Serial.printf("✅ 日期 %s 的数据加载成功\n", date.c_str());
    } else {
        Serial.printf("❌ 日期 %s 的数据解析失败\n", date.c_str());
    }
    
    return success;
}

bool DataManagerV3::saveDailyData(const DailyDataV3& data) {
    if (!fs || !fs->isAvailable()) return false;
    
    String file_path = fs->getDailyDataPath(data.date);
    String json_data = data.toJsonString();
    
    bool success = fs->writeFile(file_path, json_data);
    if (success) {
        Serial.printf("✅ 日期 %s 的数据保存成功\n", data.date.c_str());
    } else {
        Serial.printf("❌ 日期 %s 的数据保存失败\n", data.date.c_str());
    }
    
    return success;
}

bool DataManagerV3::loadCurrentDayData() {
    return loadDailyData(current_date, current_day_data);
}

bool DataManagerV3::saveCurrentDayData() {
    return saveDailyData(current_day_data);
}

std::vector<DailyDataV3> DataManagerV3::getHistoryData(uint8_t days) {
    std::vector<DailyDataV3> history;

    if (!fs || !fs->isAvailable()) return history;

    for (int i = 0; i < days; i++) {
        // 生成日期字符串（从今天往前推）
        String date = DataUtilsV3::getDateString(-i); // 使用新的日期计算函数

        DailyDataV3 daily_data;
        if (loadDailyData(date, daily_data)) {
            history.push_back(daily_data);
            Serial.printf("📊 加载历史数据: %s (%d次游戏)\n",
                         date.c_str(), daily_data.daily_total.session_count);
        } else {
            // 如果没有数据，创建空的日期记录
            daily_data.date = date;
            history.push_back(daily_data);
            Serial.printf("📊 创建空历史记录: %s\n", date.c_str());
        }
    }

    Serial.printf("📊 加载了 %d 天的历史数据\n", history.size());
    return history;
}

bool DataManagerV3::updateHistoryStats() {
    // 重新计算历史统计
    history_stats.reset();
    
    std::vector<DailyDataV3> history = getHistoryData(30); // 最近30天
    for (const auto& daily_data : history) {
        history_stats.updateWithDailyData(daily_data);
    }
    
    return saveHistoryStats();
}

bool DataManagerV3::saveHistoryStats() {
    if (!fs || !fs->isAvailable()) return false;
    
    String json_data = history_stats.toJsonString();
    return fs->writeFile(V3_STATS_FILE, json_data);
}

bool DataManagerV3::loadHistoryStats() {
    if (!fs || !fs->isAvailable()) return false;
    
    if (!fs->fileExists(V3_STATS_FILE)) {
        return false;
    }
    
    String json_data = fs->readFile(V3_STATS_FILE);
    return history_stats.fromJsonString(json_data);
}

void DataManagerV3::resetHistoryStats() {
    history_stats.reset();
    saveHistoryStats();
}

bool DataManagerV3::saveTargetSettings(const TargetSettingsV3& settings) {
    target_settings = settings;
    
    // 将目标设置保存到系统配置中
    JsonDocument doc;
    JsonObject target_obj = doc["target"].to<JsonObject>();
    target_settings.toJson(target_obj);
    
    String json_data;
    serializeJson(doc, json_data);
    
    return fs->writeFile("/targets.json", json_data);
}

bool DataManagerV3::loadTargetSettings() {
    if (!fs || !fs->fileExists("/targets.json")) {
        return false;
    }

    String json_data = fs->readFile("/targets.json");
    JsonDocument doc;
    
    if (deserializeJson(doc, json_data) != DeserializationError::Ok) {
        return false;
    }
    
    JsonObject target_obj = doc["target"];
    return target_settings.fromJson(target_obj);
}

void DataManagerV3::resetTargetSettings() {
    target_settings = TargetSettingsV3(); // 使用默认构造函数
    saveTargetSettings(target_settings);
}

uint32_t DataManagerV3::getTotalGamesToday() const {
    return current_day_data.daily_total.session_count;
}

uint32_t DataManagerV3::getTotalJumpsToday() const {
    return current_day_data.daily_total.total_jumps;
}

float DataManagerV3::getTotalCaloriesToday() const {
    return current_day_data.daily_total.total_calories;
}

uint32_t DataManagerV3::getTotalTimeToday() const {
    return current_day_data.daily_total.total_duration;
}

uint16_t DataManagerV3::getBestScoreToday() const {
    return current_day_data.daily_total.best_score;
}

// 周统计数据实现
uint32_t DataManagerV3::getWeeklyWorkouts() const {
    std::vector<DailyDataV3> week_data = const_cast<DataManagerV3*>(this)->getHistoryData(7);
    uint32_t total = 0;
    for (const auto& day : week_data) {
        total += day.daily_total.session_count;
    }
    return total;
}

uint32_t DataManagerV3::getWeeklyJumps() const {
    std::vector<DailyDataV3> week_data = const_cast<DataManagerV3*>(this)->getHistoryData(7);
    uint32_t total = 0;
    for (const auto& day : week_data) {
        total += day.daily_total.total_jumps;
    }
    return total;
}

float DataManagerV3::getWeeklyCalories() const {
    std::vector<DailyDataV3> week_data = const_cast<DataManagerV3*>(this)->getHistoryData(7);
    float total = 0.0f;
    for (const auto& day : week_data) {
        total += day.daily_total.total_calories;
    }
    return total;
}

uint32_t DataManagerV3::getWeeklyTime() const {
    std::vector<DailyDataV3> week_data = const_cast<DataManagerV3*>(this)->getHistoryData(7);
    uint32_t total = 0;
    for (const auto& day : week_data) {
        total += day.daily_total.total_duration;
    }
    return total;
}

uint8_t DataManagerV3::getWeeklyGoalsAchieved() const {
    std::vector<DailyDataV3> week_data = const_cast<DataManagerV3*>(this)->getHistoryData(7);
    uint8_t total = 0;
    for (const auto& day : week_data) {
        total += day.daily_total.targets_achieved;
    }
    return total;
}

bool DataManagerV3::isTodayTargetAchieved() const {
    return target_settings.isTargetAchieved(current_day_data);
}

bool DataManagerV3::isSessionTargetAchieved(const GameSessionV3& session) const {
    return target_settings.isTargetAchieved(session);
}

float DataManagerV3::getTodayTargetProgress() const {
    if (!target_settings.enabled) return 0.0f;
    
    float progress = 0.0f;
    
    if (target_settings.target_jumps > 0) {
        progress = max(progress, (float)getTotalJumpsToday() / target_settings.target_jumps);
    }
    
    if (target_settings.target_time > 0) {
        progress = max(progress, (float)getTotalTimeToday() / target_settings.target_time);
    }
    
    if (target_settings.target_calories > 0) {
        progress = max(progress, getTotalCaloriesToday() / target_settings.target_calories);
    }
    
    return min(progress, 1.0f);
}

void DataManagerV3::printDataSummary() {
    Serial.println("📊 V3.0数据管理器状态:");
    Serial.printf("   当前日期: %s\n", current_date.c_str());
    Serial.printf("   今日游戏: %d 次\n", getTotalGamesToday());
    Serial.printf("   今日跳跃: %d 次\n", getTotalJumpsToday());
    Serial.printf("   今日卡路里: %.1f\n", getTotalCaloriesToday());
    Serial.printf("   今日时长: %s\n", DataUtilsV3::formatTime(getTotalTimeToday()).c_str());
    Serial.printf("   今日最佳: %d 分\n", getBestScoreToday());
    Serial.printf("   目标进度: %.1f%%\n", getTodayTargetProgress() * 100);
    
    if (fs) {
        Serial.printf("   存储使用: %.1f%%\n", fs->getUsagePercent());
    }
}

void DataManagerV3::updateCurrentDate() {
    // 使用基于millis()的日期计算
    String new_date = DataUtilsV3::getCurrentDateString();

    // 如果日期变化，保存昨天的数据并创建新的当日数据
    if (current_day_data.date != new_date && !current_day_data.date.isEmpty()) {
        Serial.printf("📅 日期变化: %s -> %s\n", current_day_data.date.c_str(), new_date.c_str());
        saveCurrentDayData();
        current_day_data = DailyDataV3(new_date);
        saveCurrentDayData(); // 创建新日期的空数据文件
    } else if (current_day_data.date.isEmpty()) {
        current_day_data.date = new_date;
    }

    current_date = new_date;
}

bool DataManagerV3::generateDemoData(uint8_t days) {
    if (!initialized || !fs || !fs->isAvailable()) {
        Serial.println("❌ 数据管理器未初始化，无法生成演示数据");
        return false;
    }

    Serial.printf("🎯 开始生成 %d 天的演示数据...\n", days);

    // 为过去几天生成演示数据
    for (int i = 1; i < days; i++) { // 从昨天开始，不覆盖今天的数据
        String date = DataUtilsV3::getDateString(-i);

        // 检查是否已有数据，如果有就跳过
        DailyDataV3 existing_data;
        if (loadDailyData(date, existing_data) && !existing_data.isEmpty()) {
            Serial.printf("⏭️ 跳过 %s，已有数据\n", date.c_str());
            continue;
        }

        // 创建演示数据
        DailyDataV3 demo_data(date);

        // 根据天数生成不同的数据模式
        int base_sessions = 2 + (i % 3); // 2-4次游戏
        int base_jumps = 80 + (i * 15) + (rand() % 40); // 80-200次跳跃

        for (int session = 0; session < base_sessions; session++) {
            GameSessionV3 demo_session;

            // 生成会话时间
            int hour = 8 + session * 4 + (rand() % 2); // 分散在一天中
            int minute = rand() % 60;
            int second = rand() % 60;
            char time_str[9];
            snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hour, minute, second);
            demo_session.start_time = String(time_str);

            // 生成游戏数据
            demo_session.difficulty = (game_difficulty_t)(rand() % DIFFICULTY_COUNT);
            demo_session.jump_count = base_jumps + (rand() % 30) - 15; // 添加随机变化
            demo_session.duration = 180 + (rand() % 120); // 3-5分钟
            demo_session.calories = DataUtilsV3::jumpsToCalories(demo_session.jump_count, demo_session.difficulty);
            demo_session.avg_frequency = DataUtilsV3::calculateAvgFrequency(demo_session.jump_count, demo_session.duration);
            demo_session.max_height = DataUtilsV3::calculateMaxHeight(demo_session.avg_frequency);
            demo_session.score = DataUtilsV3::calculateGameScore(demo_session.jump_count, demo_session.duration, demo_session.difficulty);
            demo_session.target_achieved = (rand() % 100) < 70; // 70%概率达成目标

            demo_data.addSession(demo_session);
        }

        // 更新每日汇总
        demo_data.updateDailyTotal();

        // 保存演示数据
        if (saveDailyData(demo_data)) {
            Serial.printf("✅ %s: %d次游戏, %d次跳跃, %.1f卡路里\n",
                         date.c_str(),
                         demo_data.daily_total.session_count,
                         demo_data.daily_total.total_jumps,
                         demo_data.daily_total.total_calories);
        } else {
            Serial.printf("❌ %s: 数据保存失败\n", date.c_str());
        }
    }

    // 更新历史统计
    updateHistoryStats();

    Serial.println("🎉 演示数据生成完成！");
    return true;
}

void DataManagerV3::initNTPTime() {
    Serial.println("🕐 初始化NTP时间同步...");

    // 配置时区为中国标准时间 (UTC+8)
    configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov", "cn.pool.ntp.org");

    // 等待时间同步（最多等待10秒）
    Serial.print("⏳ 等待NTP时间同步");
    int retry_count = 0;
    time_t now = 0;

    while (now < 946684800 && retry_count < 20) { // 2000-01-01 00:00:00 UTC
        delay(500);
        Serial.print(".");
        time(&now);
        retry_count++;
    }
    Serial.println();

    if (now > 946684800) {
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);

        Serial.printf("✅ NTP时间同步成功: %04d-%02d-%02d %02d:%02d:%02d\n",
                     timeinfo.tm_year + 1900,
                     timeinfo.tm_mon + 1,
                     timeinfo.tm_mday,
                     timeinfo.tm_hour,
                     timeinfo.tm_min,
                     timeinfo.tm_sec);
    } else {
        Serial.println("⚠️ NTP时间同步失败，将使用备用时间计算");
    }
}
