#ifndef NTP_TIME_V3_H
#define NTP_TIME_V3_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

// NTP服务器配置
#define NTP_SERVER_PRIMARY   "pool.ntp.org"
#define NTP_SERVER_SECONDARY "time.nist.gov"
#define NTP_SERVER_BACKUP    "cn.pool.ntp.org"

// 时区配置（中国标准时间 UTC+8）
#define TIME_ZONE_OFFSET     8
#define DAYLIGHT_OFFSET      0

// NTP同步配置
#define NTP_SYNC_INTERVAL    (6 * 60 * 60 * 1000UL)  // 6小时同步一次
#define NTP_TIMEOUT          10000                    // 10秒超时
#define NTP_RETRY_COUNT      3                        // 重试次数

class NTPTimeV3 {
private:
    bool initialized;
    bool time_synced;
    unsigned long last_sync_time;
    unsigned long last_sync_attempt;
    String current_ntp_server;
    
    // 内部方法
    bool connectToNTP(const char* server);
    bool waitForTimeSync(unsigned long timeout_ms);
    void logTimeSync(bool success);

public:
    NTPTimeV3();
    ~NTPTimeV3();
    
    // 初始化和管理
    bool init();
    void deinit();
    bool isInitialized() const { return initialized; }
    bool isTimeSynced() const { return time_synced; }
    
    // 时间同步
    bool syncTime();
    bool syncTimeIfNeeded();
    void forceSync();
    unsigned long getLastSyncTime() const { return last_sync_time; }
    
    // 时间获取
    String getCurrentDateString();
    String getCurrentTimeString();
    String getCurrentDateTimeString();
    String getDateString(int days_offset = 0);
    
    // 时间信息
    struct tm getCurrentTime();
    time_t getCurrentTimestamp();
    bool isValidTime();
    
    // 格式化
    String formatDate(const struct tm& timeinfo);
    String formatTime(const struct tm& timeinfo);
    String formatDateTime(const struct tm& timeinfo);
    
    // 调试信息
    void printTimeInfo();
    String getTimeZoneInfo();
    String getNTPServerInfo();
    
    // 静态工具函数
    static bool isLeapYear(int year);
    static int getDaysInMonth(int year, int month);
    static String formatTimestamp(time_t timestamp);
};

// 全局NTP时间管理器实例
extern NTPTimeV3 ntpTimeV3;

#endif // NTP_TIME_V3_H
