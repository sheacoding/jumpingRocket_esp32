#ifndef DATA_MANAGER_V3_H
#define DATA_MANAGER_V3_H

#include <Arduino.h>
#include <vector>
#include "data_models_v3.h"
#include "file_system_v3.h"

class DataManagerV3 {
private:
    FileSystemV3* fs;
    SystemConfigV3 system_config;
    DailyDataV3 current_day_data;
    HistoryStatsV3 history_stats;
    TargetSettingsV3 target_settings;
    
    bool initialized;
    String current_date;
    
public:
    DataManagerV3();
    ~DataManagerV3();
    
    // 初始化和管理
    bool init(FileSystemV3* filesystem);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // 系统配置管理
    const SystemConfigV3& getSystemConfig() const { return system_config; }
    bool saveSystemConfig(const SystemConfigV3& config);
    bool loadSystemConfig();
    void resetSystemConfig();
    
    // 游戏会话管理
    bool saveGameSession(const GameSessionV3& session);
    GameSessionV3 createGameSession(game_difficulty_t difficulty, 
                                   uint32_t jump_count, 
                                   uint32_t duration);
    
    // 每日数据管理
    const DailyDataV3& getCurrentDayData() const { return current_day_data; }
    bool loadDailyData(const String& date, DailyDataV3& data);
    bool saveDailyData(const DailyDataV3& data);
    bool loadCurrentDayData();
    bool saveCurrentDayData();
    
    // 历史数据管理
    std::vector<DailyDataV3> getHistoryData(uint8_t days = V3_HISTORY_DAYS);
    bool deleteHistoryData(const String& date);
    void cleanupOldData(uint8_t keep_days = V3_HISTORY_DAYS);
    
    // 统计数据管理
    const HistoryStatsV3& getHistoryStats() const { return history_stats; }
    bool updateHistoryStats();
    bool saveHistoryStats();
    bool loadHistoryStats();
    void resetHistoryStats();
    
    // 目标设置管理
    const TargetSettingsV3& getTargetSettings() const { return target_settings; }
    bool saveTargetSettings(const TargetSettingsV3& settings);
    bool loadTargetSettings();
    void resetTargetSettings();
    
    // 数据查询和分析
    uint32_t getTotalGamesToday() const;
    uint32_t getTotalJumpsToday() const;
    float getTotalCaloriesToday() const;
    uint32_t getTotalTimeToday() const;
    uint16_t getBestScoreToday() const;

    // 周统计数据
    uint32_t getWeeklyWorkouts() const;
    uint32_t getWeeklyJumps() const;
    float getWeeklyCalories() const;
    uint32_t getWeeklyTime() const;
    uint8_t getWeeklyGoalsAchieved() const;
    
    // 趋势分析
    std::vector<uint32_t> getJumpsTrend(uint8_t days = 7);
    std::vector<float> getCaloriesTrend(uint8_t days = 7);
    std::vector<uint16_t> getScoresTrend(uint8_t days = 7);
    
    // 目标检查
    bool isTodayTargetAchieved() const;
    bool isSessionTargetAchieved(const GameSessionV3& session) const;
    float getTodayTargetProgress() const; // 返回0.0-1.0的进度
    
    // 数据导出和备份
    String exportDailyDataAsJson(const String& date);
    String exportAllDataAsJson();
    bool importDailyDataFromJson(const String& json_data);
    
    // 数据验证和修复
    bool validateData();
    bool repairCorruptedData();
    
    // 调试和信息
    void printDataSummary();
    void printSystemInfo();
    size_t getDataStorageUsage();

    // 演示数据生成
    bool generateDemoData(uint8_t days = 7);
    
private:
    // 内部辅助函数
    void updateCurrentDate();
    void initNTPTime();
    bool ensureDailyDataExists(const String& date);
    void updateDailyTotals();
    String generateDailyDataPath(const String& date);
    
    // 数据迁移和兼容性
    bool migrateFromV2();
    bool checkDataVersion();
    void upgradeDataFormat();
};

// 全局数据管理器实例声明
extern DataManagerV3 dataManagerV3;

#endif // DATA_MANAGER_V3_H
