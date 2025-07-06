#ifndef FILE_SYSTEM_V3_H
#define FILE_SYSTEM_V3_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <vector>
#include "board_config_v3.h"

// 文件路径定义 (SPIFFS不支持真正的目录，使用扁平结构)
#define V3_CONFIG_FILE          "/system.json"
#define V3_DAILY_DATA_PREFIX    "/daily_"
#define V3_STATS_FILE           "/summary.json"
#define V3_LOG_FILE             "/system.log"

// 文件系统配置
#define V3_FS_FORMAT_ON_FAIL    true
#define V3_MAX_FILE_SIZE        8192    // 8KB最大文件大小
#define V3_JSON_BUFFER_SIZE     2048    // JSON缓冲区大小

class FileSystemV3 {
private:
    bool fs_initialized;
    bool fs_available;
    
public:
    FileSystemV3();
    ~FileSystemV3();
    
    // 初始化和管理
    bool init();
    void deinit();
    bool isAvailable() const { return fs_available; }
    
    // 文件操作
    bool writeFile(const String& path, const String& content);
    String readFile(const String& path);
    bool fileExists(const String& path);
    bool deleteFile(const String& path);
    size_t getFileSize(const String& path);
    
    // 目录操作
    bool createDirectory(const String& path);
    std::vector<String> listFiles(const String& dir = "/");
    
    // JSON操作
    bool writeJson(const String& path, const JsonDocument& doc);
    bool readJson(const String& path, JsonDocument& doc);
    
    // 系统信息
    size_t getTotalBytes();
    size_t getUsedBytes();
    size_t getFreeBytes();
    float getUsagePercent();
    
    // 维护操作
    void formatFileSystem();
    void cleanupOldFiles(int keep_days = V3_HISTORY_DAYS);
    void printFileSystemInfo();
    
    // 数据文件路径生成
    String getDailyDataPath(const String& date);
    String getCurrentDailyDataPath();
    
private:
    void createDefaultDirectories();
    bool ensureDirectoryExists(const String& path);
    String getDateString(int days_offset = 0);
    void logOperation(const String& operation, const String& path, bool success);
};

// 全局文件系统实例声明
extern FileSystemV3 fileSystemV3;

#endif // FILE_SYSTEM_V3_H
