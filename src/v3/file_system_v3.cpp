#include "v3/file_system_v3.h"
#include <time.h>

// 全局文件系统实例
FileSystemV3 fileSystemV3;

FileSystemV3::FileSystemV3() : fs_initialized(false), fs_available(false) {
}

FileSystemV3::~FileSystemV3() {
    deinit();
}

bool FileSystemV3::init() {
    Serial.println("🗂️ 初始化V3.0文件系统...");
    
    if (fs_initialized) {
        Serial.println("文件系统已经初始化");
        return fs_available;
    }
    
    // 初始化SPIFFS
    Serial.println("正在初始化SPIFFS...");
    if (!SPIFFS.begin(false)) {
        Serial.println("⚠️ SPIFFS挂载失败，尝试格式化...");
        if (!SPIFFS.begin(true)) {
            Serial.println("❌ SPIFFS初始化和格式化都失败");
            fs_available = false;
            return false;
        }
        Serial.println("✅ SPIFFS格式化成功");
    } else {
        Serial.println("✅ SPIFFS挂载成功");
    }
    
    fs_initialized = true;
    fs_available = true;
    
    // 创建默认目录结构
    createDefaultDirectories();
    
    // 打印文件系统信息
    printFileSystemInfo();
    
    Serial.println("✅ V3.0文件系统初始化成功");
    return true;
}

void FileSystemV3::deinit() {
    if (fs_initialized) {
        SPIFFS.end();
        fs_initialized = false;
        fs_available = false;
        Serial.println("文件系统已关闭");
    }
}

bool FileSystemV3::writeFile(const String& path, const String& content) {
    if (!fs_available) {
        Serial.println("❌ 文件系统不可用");
        return false;
    }
    
    if (content.length() > V3_MAX_FILE_SIZE) {
        Serial.printf("❌ 文件内容过大: %d > %d\n", content.length(), V3_MAX_FILE_SIZE);
        return false;
    }
    
    File file = SPIFFS.open(path, "w");
    if (!file) {
        Serial.printf("❌ 无法创建文件: %s\n", path.c_str());
        logOperation("WRITE_FAIL", path, false);
        return false;
    }
    
    size_t written = file.print(content);
    file.close();
    
    bool success = (written == content.length());
    if (success) {
        Serial.printf("✅ 文件写入成功: %s (%d bytes)\n", path.c_str(), written);
    } else {
        Serial.printf("❌ 文件写入不完整: %s (%d/%d bytes)\n", 
                     path.c_str(), written, content.length());
    }
    
    logOperation("WRITE", path, success);
    return success;
}

String FileSystemV3::readFile(const String& path) {
    if (!fs_available) {
        Serial.println("❌ 文件系统不可用");
        return "";
    }
    
    if (!fileExists(path)) {
        Serial.printf("❌ 文件不存在: %s\n", path.c_str());
        return "";
    }
    
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.printf("❌ 无法打开文件: %s\n", path.c_str());
        logOperation("READ_FAIL", path, false);
        return "";
    }
    
    String content = file.readString();
    file.close();
    
    Serial.printf("✅ 文件读取成功: %s (%d bytes)\n", path.c_str(), content.length());
    logOperation("READ", path, true);
    return content;
}

bool FileSystemV3::fileExists(const String& path) {
    if (!fs_available) return false;
    return SPIFFS.exists(path);
}

bool FileSystemV3::deleteFile(const String& path) {
    if (!fs_available) return false;
    
    if (!fileExists(path)) {
        Serial.printf("⚠️ 要删除的文件不存在: %s\n", path.c_str());
        return true; // 文件不存在也算删除成功
    }
    
    bool success = SPIFFS.remove(path);
    if (success) {
        Serial.printf("✅ 文件删除成功: %s\n", path.c_str());
    } else {
        Serial.printf("❌ 文件删除失败: %s\n", path.c_str());
    }
    
    logOperation("DELETE", path, success);
    return success;
}

size_t FileSystemV3::getFileSize(const String& path) {
    if (!fs_available || !fileExists(path)) return 0;
    
    File file = SPIFFS.open(path, "r");
    if (!file) return 0;
    
    size_t size = file.size();
    file.close();
    return size;
}

std::vector<String> FileSystemV3::listFiles(const String& dir) {
    std::vector<String> files;
    
    if (!fs_available) return files;
    
    File root = SPIFFS.open(dir);
    if (!root || !root.isDirectory()) {
        Serial.printf("❌ 无法打开目录: %s\n", dir.c_str());
        return files;
    }
    
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            files.push_back(String(file.name()));
        }
        file = root.openNextFile();
    }
    
    Serial.printf("📁 目录 %s 包含 %d 个文件\n", dir.c_str(), files.size());
    return files;
}

bool FileSystemV3::writeJson(const String& path, const JsonDocument& doc) {
    String jsonString;
    serializeJson(doc, jsonString);
    return writeFile(path, jsonString);
}

bool FileSystemV3::readJson(const String& path, JsonDocument& doc) {
    String content = readFile(path);
    if (content.isEmpty()) return false;
    
    DeserializationError error = deserializeJson(doc, content);
    if (error) {
        Serial.printf("❌ JSON解析失败: %s, 错误: %s\n", 
                     path.c_str(), error.c_str());
        return false;
    }
    
    return true;
}

size_t FileSystemV3::getTotalBytes() {
    if (!fs_available) return 0;
    return SPIFFS.totalBytes();
}

size_t FileSystemV3::getUsedBytes() {
    if (!fs_available) return 0;
    return SPIFFS.usedBytes();
}

size_t FileSystemV3::getFreeBytes() {
    if (!fs_available) return 0;
    return getTotalBytes() - getUsedBytes();
}

float FileSystemV3::getUsagePercent() {
    size_t total = getTotalBytes();
    if (total == 0) return 0.0f;
    return (float)getUsedBytes() / total * 100.0f;
}

void FileSystemV3::formatFileSystem() {
    Serial.println("⚠️ 格式化文件系统...");
    SPIFFS.format();
    Serial.println("✅ 文件系统格式化完成");
}

void FileSystemV3::cleanupOldFiles(int keep_days) {
    Serial.printf("🧹 清理 %d 天前的旧文件...\n", keep_days);
    
    // 获取当前时间戳
    time_t now = time(nullptr);
    time_t cutoff_time = now - (keep_days * 24 * 60 * 60);
    
    // 列出所有每日数据文件
    std::vector<String> files = listFiles("/");
    int deleted_count = 0;
    
    for (const String& filename : files) {
        if (filename.startsWith("/daily_") && filename.endsWith(".json")) {
            // 从文件名提取日期并检查是否过期
            // 文件名格式: /daily_YYYY-MM-DD.json
            String date_part = filename.substring(7, 17); // 提取YYYY-MM-DD

            // 简单的日期比较（这里可以改进为更精确的日期解析）
            if (date_part.length() == 10) {
                // 暂时保留所有文件，不删除
                Serial.printf("保留历史文件: %s\n", filename.c_str());
            }
        }
    }
    
    Serial.printf("✅ 清理完成，删除了 %d 个旧文件\n", deleted_count);
}

void FileSystemV3::printFileSystemInfo() {
    if (!fs_available) {
        Serial.println("❌ 文件系统不可用");
        return;
    }
    
    Serial.println("📊 文件系统信息:");
    Serial.printf("   总容量: %d bytes (%.1f KB)\n", 
                 getTotalBytes(), getTotalBytes() / 1024.0f);
    Serial.printf("   已使用: %d bytes (%.1f KB)\n", 
                 getUsedBytes(), getUsedBytes() / 1024.0f);
    Serial.printf("   可用空间: %d bytes (%.1f KB)\n", 
                 getFreeBytes(), getFreeBytes() / 1024.0f);
    Serial.printf("   使用率: %.1f%%\n", getUsagePercent());
    
    // 列出根目录文件
    std::vector<String> files = listFiles("/");
    Serial.printf("   文件数量: %d\n", files.size());
    
    if (files.size() > 0 && files.size() <= 10) {
        Serial.println("   文件列表:");
        for (const String& file : files) {
            size_t size = getFileSize(file);
            Serial.printf("     %s (%d bytes)\n", file.c_str(), size);
        }
    }
}

String FileSystemV3::getDailyDataPath(const String& date) {
    return V3_DAILY_DATA_PREFIX + date + ".json";
}

String FileSystemV3::getCurrentDailyDataPath() {
    return getDailyDataPath(getDateString());
}

void FileSystemV3::createDefaultDirectories() {
    Serial.println("📁 创建默认目录结构...");
    
    // SPIFFS不支持真正的目录，但我们可以创建一些默认文件来模拟目录结构
    
    // 创建默认配置文件
    if (!fileExists(V3_CONFIG_FILE)) {
        JsonDocument config;
        config["version"] = JUMPING_ROCKET_VERSION_STRING;
        config["volume"] = 80;
        config["brightness"] = 70;
        config["difficulty"] = "normal";
        config["auto_sleep"] = false; // V3.0暂不支持
        config["created_time"] = getDateString();
        
        if (writeJson(V3_CONFIG_FILE, config)) {
            Serial.println("✅ 创建默认配置文件");
        } else {
            Serial.println("❌ 创建默认配置文件失败");
        }
    }
    
    // 创建统计文件
    if (!fileExists(V3_STATS_FILE)) {
        JsonDocument stats;
        stats["total_games"] = 0;
        stats["total_jumps"] = 0;
        stats["total_time"] = 0;
        stats["best_score"] = 0;
        stats["created_time"] = getDateString();
        
        if (writeJson(V3_STATS_FILE, stats)) {
            Serial.println("✅ 创建默认统计文件");
        } else {
            Serial.println("❌ 创建默认统计文件失败");
        }
    }
}

String FileSystemV3::getDateString(int days_offset) {
    // 简单的日期字符串生成（实际项目中应该使用RTC或NTP）
    // 暂时使用启动后的天数计算
    static uint32_t start_day = 0;
    if (start_day == 0) {
        start_day = millis() / (24 * 60 * 60 * 1000UL); // 启动时的天数
    }

    uint32_t current_day = start_day + days_offset;

    // 生成简单的日期格式 (从2025-01-01开始)
    uint32_t base_year = 2025;
    uint32_t base_month = 1;
    uint32_t base_day = 1;

    uint32_t total_days = current_day;
    uint32_t year = base_year + (total_days / 365);
    uint32_t month = base_month + ((total_days % 365) / 30);
    uint32_t day = base_day + ((total_days % 365) % 30);

    if (month > 12) {
        year++;
        month -= 12;
    }
    if (day > 31) {
        month++;
        day -= 31;
    }

    char date_str[11];
    snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", year, month, day);

    return String(date_str);
}

void FileSystemV3::logOperation(const String& operation, const String& path, bool success) {
    // 简单的操作日志（可选实现）
    if (operation == "WRITE" || operation == "DELETE") {
        Serial.printf("📝 文件操作: %s %s %s\n", 
                     operation.c_str(), 
                     path.c_str(), 
                     success ? "成功" : "失败");
    }
}
