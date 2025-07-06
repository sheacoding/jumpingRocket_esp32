#include "v3/board_config_v3.h"
#include "v3/file_system_v3.h"
#include "v3/data_manager_v3.h"
#include "v3/ui_views_v3.h"
#include "v3/game_integration_v3.h"

// V3.0系统测试结果
struct V3TestResult {
    bool file_system_test;
    bool data_manager_test;
    bool ui_manager_test;
    bool config_test;
    bool integration_test;
    bool overall_success;
    
    V3TestResult() : 
        file_system_test(false),
        data_manager_test(false),
        ui_manager_test(false),
        config_test(false),
        integration_test(false),
        overall_success(false) {}
};

static V3TestResult test_result;

// 测试文件系统
bool testV3FileSystem() {
    Serial.println("🧪 测试V3.0文件系统...");
    
    // 测试初始化
    if (!fileSystemV3.init()) {
        Serial.println("❌ 文件系统初始化失败");
        return false;
    }
    
    // 测试文件写入
    String test_data = "{\"test\":\"data\",\"timestamp\":" + String(millis()) + "}";
    if (!fileSystemV3.writeFile("/test.json", test_data)) {
        Serial.println("❌ 文件写入测试失败");
        return false;
    }
    
    // 测试文件读取
    String read_data = fileSystemV3.readFile("/test.json");
    if (read_data.isEmpty()) {
        Serial.println("❌ 文件读取测试失败");
        return false;
    }
    
    // 测试文件删除
    if (!fileSystemV3.deleteFile("/test.json")) {
        Serial.println("❌ 文件删除测试失败");
        return false;
    }
    
    Serial.println("✅ 文件系统测试通过");
    return true;
}

// 测试数据管理器
bool testV3DataManager() {
    Serial.println("🧪 测试V3.0数据管理器...");
    
    // 测试初始化
    if (!dataManagerV3.init(&fileSystemV3)) {
        Serial.println("❌ 数据管理器初始化失败");
        return false;
    }
    
    // 测试系统配置
    SystemConfigV3 config = dataManagerV3.getSystemConfig();
    config.volume = 75;
    config.sound_enabled = true;
    config.default_difficulty = DIFFICULTY_NORMAL;
    
    if (!dataManagerV3.saveSystemConfig(config)) {
        Serial.println("❌ 系统配置保存失败");
        return false;
    }
    
    SystemConfigV3 loaded_config = dataManagerV3.getSystemConfig();
    if (loaded_config.volume != 75 || !loaded_config.sound_enabled) {
        Serial.println("❌ 系统配置加载验证失败");
        return false;
    }
    
    // 测试游戏会话
    GameSessionV3 session = dataManagerV3.createGameSession(DIFFICULTY_NORMAL, 50, 300);
    if (session.jump_count != 50 || session.duration != 300) {
        Serial.println("❌ 游戏会话创建失败");
        return false;
    }
    
    if (!dataManagerV3.saveGameSession(session)) {
        Serial.println("❌ 游戏会话保存失败");
        return false;
    }
    
    // 测试统计数据
    uint32_t total_jumps = dataManagerV3.getTotalJumpsToday();
    if (total_jumps < 50) {
        Serial.println("❌ 统计数据计算错误");
        return false;
    }
    
    Serial.println("✅ 数据管理器测试通过");
    return true;
}

// 测试UI管理器
bool testV3UIManager() {
    Serial.println("🧪 测试V3.0 UI管理器...");
    
    // 由于UI管理器需要显示器，这里只做基本的创建测试
    extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
    
    if (!initUIManagerV3(&u8g2)) {
        Serial.println("❌ UI管理器初始化失败");
        return false;
    }
    
    if (!uiManagerV3) {
        Serial.println("❌ UI管理器实例为空");
        return false;
    }
    
    // 测试视图切换
    uiManagerV3->switchToView(UI_VIEW_DIFFICULTY_SELECT);
    if (uiManagerV3->getCurrentView() != UI_VIEW_DIFFICULTY_SELECT) {
        Serial.println("❌ 视图切换失败");
        return false;
    }
    
    uiManagerV3->switchToView(UI_VIEW_MAIN_MENU);
    if (uiManagerV3->getCurrentView() != UI_VIEW_MAIN_MENU) {
        Serial.println("❌ 视图切换失败");
        return false;
    }
    
    Serial.println("✅ UI管理器测试通过");
    return true;
}

// 测试配置系统
bool testV3Config() {
    Serial.println("Testing V3.0 configuration system...");

    // 测试配置验证
    if (!V3Config::validateConfigs()) {
        Serial.println("ERROR: Configuration validation failed");
        return false;
    }

    // 测试难度配置
    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        game_difficulty_t difficulty = (game_difficulty_t)i;
        const difficulty_config_t* config = V3Config::getDifficultyConfig(difficulty);

        if (!config || !config->name_en) {
            Serial.printf("ERROR: Difficulty %d configuration invalid\n", i);
            return false;
        }

        if (config->multiplier <= 0 || config->target_jumps == 0 || config->target_time == 0) {
            Serial.printf("ERROR: Difficulty %d parameters invalid\n", i);
            return false;
        }
    }

    // 测试得分计算
    uint16_t score = V3Config::calculateScore(DIFFICULTY_NORMAL, 100, 600, 2.5f);
    if (score == 0) {
        Serial.println("ERROR: Score calculation failed");
        return false;
    }
    
    // 测试卡路里计算
    float calories = V3Config::calculateCalories(DIFFICULTY_NORMAL, 100, 600);
    if (calories <= 0) {
        Serial.println("❌ 卡路里计算失败");
        return false;
    }
    
    Serial.println("✅ 配置系统测试通过");
    return true;
}

// 测试游戏集成
bool testV3GameIntegration() {
    Serial.println("🧪 测试V3.0游戏集成...");
    
    // 测试集成初始化
    if (!initGameIntegrationV3()) {
        Serial.println("❌ 游戏集成初始化失败");
        return false;
    }
    
    // 测试兼容性检查
    if (!checkV3Compatibility()) {
        Serial.println("❌ 兼容性检查失败");
        return false;
    }
    
    // 测试状态获取
    V3GameState state = getV3GameState();
    if (state == V3_GAME_DISABLED) {
        Serial.println("❌ 游戏集成未激活");
        return false;
    }
    
    Serial.println("✅ 游戏集成测试通过");
    return true;
}

// 运行完整的V3.0系统测试
void runV3SystemTests() {
    Serial.println("🧪 开始V3.0系统完整测试...");
    Serial.println("================================================");
    
    // 重置测试结果
    test_result = V3TestResult();
    
    // 依次运行各项测试
    test_result.file_system_test = testV3FileSystem();
    delay(100);
    
    test_result.data_manager_test = testV3DataManager();
    delay(100);
    
    test_result.ui_manager_test = testV3UIManager();
    delay(100);
    
    test_result.config_test = testV3Config();
    delay(100);
    
    test_result.integration_test = testV3GameIntegration();
    delay(100);
    
    // 计算总体结果
    test_result.overall_success = test_result.file_system_test &&
                                  test_result.data_manager_test &&
                                  test_result.ui_manager_test &&
                                  test_result.config_test &&
                                  test_result.integration_test;
    
    // 输出测试报告
    Serial.println("================================================");
    Serial.println("📋 V3.0系统测试报告:");
    Serial.printf("   文件系统: %s\n", test_result.file_system_test ? "✅ 通过" : "❌ 失败");
    Serial.printf("   数据管理: %s\n", test_result.data_manager_test ? "✅ 通过" : "❌ 失败");
    Serial.printf("   UI管理器: %s\n", test_result.ui_manager_test ? "✅ 通过" : "❌ 失败");
    Serial.printf("   配置系统: %s\n", test_result.config_test ? "✅ 通过" : "❌ 失败");
    Serial.printf("   游戏集成: %s\n", test_result.integration_test ? "✅ 通过" : "❌ 失败");
    Serial.println("================================================");
    Serial.printf("🎯 总体结果: %s\n", test_result.overall_success ? "✅ 全部通过" : "❌ 存在失败");
    
    if (test_result.overall_success) {
        Serial.println("🎉 V3.0系统测试完全成功！");
    } else {
        Serial.println("⚠️ V3.0系统测试存在问题，请检查失败项目");
    }
}

// 获取测试结果
V3TestResult getV3TestResult() {
    return test_result;
}

// 打印V3.0系统信息
void printV3SystemDetails() {
    Serial.println("V3.0 System Information Report:");
    Serial.println("================================================");

    // 基本信息
    Serial.printf("Version: %s\n", V3Config::getConfigVersion());
    Serial.printf("Build Time: %s\n", V3Config::getConfigUpdateTime());
    Serial.printf("Board: %s\n", BOARD_NAME);

    // 功能状态
    Serial.println("\nModule Status:");
    Serial.printf("   File System: %s\n", fileSystemV3.isAvailable() ? "Available" : "Unavailable");
    Serial.printf("   Data Manager: %s\n", dataManagerV3.isInitialized() ? "Initialized" : "Not Initialized");
    Serial.printf("   UI Manager: %s\n", (uiManagerV3 != nullptr) ? "Created" : "Not Created");
    
    // 内存使用情况
    Serial.println("\n💾 内存使用情况:");
    Serial.printf("   剩余堆内存: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("   最大分配块: %d bytes\n", ESP.getMaxAllocHeap());
    Serial.printf("   堆碎片率: %.1f%%\n", 100.0 - (ESP.getMaxAllocHeap() * 100.0) / ESP.getFreeHeap());
    
    // 文件系统信息
    if (fileSystemV3.isAvailable()) {
        Serial.println("\n💾 文件系统信息:");
        Serial.printf("   总空间: %d bytes\n", SPIFFS.totalBytes());
        Serial.printf("   已使用: %d bytes\n", SPIFFS.usedBytes());
        Serial.printf("   使用率: %.1f%%\n", (SPIFFS.usedBytes() * 100.0) / SPIFFS.totalBytes());
    }
    
    // 配置信息
    Serial.println("\nConfiguration:");
    V3Config::printAllDifficultyConfigs();

    // 测试结果
    if (test_result.overall_success || test_result.file_system_test) {
        Serial.println("\nRecent Test Results:");
        Serial.printf("   File System: %s\n", test_result.file_system_test ? "PASS" : "FAIL");
        Serial.printf("   Data Manager: %s\n", test_result.data_manager_test ? "PASS" : "FAIL");
        Serial.printf("   UI Manager: %s\n", test_result.ui_manager_test ? "PASS" : "FAIL");
        Serial.printf("   Config System: %s\n", test_result.config_test ? "PASS" : "FAIL");
        Serial.printf("   Game Integration: %s\n", test_result.integration_test ? "PASS" : "FAIL");
    }
    
    Serial.println("================================================");
}

// 性能基准测试
void runV3PerformanceBenchmark() {
    Serial.println("⚡ 运行V3.0性能基准测试...");
    
    uint32_t start_time, end_time;
    
    // 文件系统性能测试
    if (fileSystemV3.isAvailable()) {
        Serial.println("📁 文件系统性能测试:");
        
        String test_data = "{\"benchmark\":true,\"data\":\"" + String(millis()) + "\"}";
        
        // 写入性能
        start_time = micros();
        fileSystemV3.writeFile("/benchmark.json", test_data);
        end_time = micros();
        Serial.printf("   写入耗时: %lu μs\n", end_time - start_time);
        
        // 读取性能
        start_time = micros();
        String read_data = fileSystemV3.readFile("/benchmark.json");
        end_time = micros();
        Serial.printf("   读取耗时: %lu μs\n", end_time - start_time);
        
        fileSystemV3.deleteFile("/benchmark.json");
    }
    
    // 数据处理性能测试
    if (dataManagerV3.isInitialized()) {
        Serial.println("Data Processing Performance Test:");

        // 会话创建性能
        start_time = micros();
        GameSessionV3 session = dataManagerV3.createGameSession(DIFFICULTY_NORMAL, 100, 600);
        end_time = micros();
        Serial.printf("   Session creation time: %lu μs\n", end_time - start_time);

        // 统计计算性能
        start_time = micros();
        uint32_t total_jumps = dataManagerV3.getTotalJumpsToday();
        end_time = micros();
        Serial.printf("   Statistics calculation time: %lu μs\n", end_time - start_time);
    }

    Serial.println("Performance benchmark test completed");
}
