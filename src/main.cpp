#include "jumping_rocket_simple.h"

// V3.0 功能集成
#ifdef JUMPING_ROCKET_V3
#include "v3/board_config_v3.h"
#include "v3/game_integration_v3.h"
#include "v3/ui_views_v3.h"

// V3.0 外部函数声明
extern bool initializeV3System();
extern void loopV3();
extern void testV3System();
extern void printV3SystemInfo();
extern void shutdownV3System();
#endif

// 任务句柄
TaskHandle_t sensor_task_handle = NULL;
TaskHandle_t display_task_handle = NULL;
TaskHandle_t sound_task_handle = NULL;
TaskHandle_t button_task_handle = NULL;
TaskHandle_t game_task_handle = NULL;

// 外部函数声明
extern "C" {
    void handle_button_event(button_event_t event);
    void data_processor_init(void);
    void add_jump_record(uint32_t timestamp);
    void update_game_statistics(void);
}

void setup() {
    #ifdef UART_RX_PIN
    Serial.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    #else
    Serial.begin(115200);
    #endif
    
    delay(2000); // 等待串口稳定
    Serial.println("\n\n========================================");
    Serial.println("🚀 蹦跳小火箭 V2.0 启动 - 调试模式");
    Serial.println("========================================");
    Serial.println("检查串口通信是否正常...");

    // 显示系统信息
    Serial.printf("ESP32 芯片型号: %s\n", ESP.getChipModel());
    Serial.printf("芯片版本: %d\n", ESP.getChipRevision());
    Serial.printf("CPU频率: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("空闲堆内存: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("开发板类型: %s\n", BOARD_NAME);

#ifdef JUMPING_ROCKET_V3
    Serial.printf("🚀 V3.0功能: 启用\n");
    Serial.printf("   版本: %s\n", JUMPING_ROCKET_VERSION_STRING);
#else
    Serial.printf("🚀 V3.0功能: 禁用 (V2.0模式)\n");
#endif
    Serial.println();
    
    // 初始化硬件
    Serial.println("🔧 开始硬件初始化...");
    if (!hardware_init()) {
        Serial.println("❌ 硬件初始化失败，系统停止");
        while(1) {
            delay(1000);
            Serial.println("系统已停止，请检查硬件连接");
        }
    }

#ifdef JUMPING_ROCKET_V3
    // 初始化V3.0系统
    Serial.println("🔧 初始化V3.0系统...");
    if (!initializeV3System()) {
        Serial.println("❌ V3.0系统初始化失败，继续使用V2.0模式");
    } else {
        Serial.println("✅ V3.0系统初始化成功");

        // 运行V3.0系统测试
        Serial.println("🧪 运行V3.0系统测试...");
        testV3System();

        // 显示V3.0系统信息
        printV3SystemInfo();

        // 初始化V3.0游戏集成
        Serial.println(" 初始化V3.0游戏集成...");
        if (initGameIntegrationV3()) {
            Serial.println("✅ V3.0游戏集成初始化成功");
        } else {
            Serial.println("❌ V3.0游戏集成初始化失败");
        }

        // 检查V3.0兼容性
        checkV3Compatibility();
    }
#endif

    // 初始化数据处理器
    Serial.println("📊 初始化数据处理器...");
    data_processor_init();

    // 初始化游戏数据
    Serial.println("🎯 初始化游戏数据...");
    game_data_init();

    Serial.println("🎮 准备启动游戏任务...");
    
    Serial.println("创建任务...");
    
    // 创建传感器任务
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, &sensor_task_handle);
    if (sensor_task_handle == NULL) {
        Serial.println("传感器任务创建失败");
        return;
    }

    // 创建显示任务
    xTaskCreate(display_task, "display_task", 4096, NULL, 4, &display_task_handle);
    if (display_task_handle == NULL) {
        Serial.println("显示任务创建失败");
        return;
    }

    // 创建音效任务
    xTaskCreate(sound_task, "sound_task", 4096, NULL, 3, &sound_task_handle);
    if (sound_task_handle == NULL) {
        Serial.println("音效任务创建失败");
        return;
    }

    // 创建按钮任务
    xTaskCreate(button_task, "button_task", 2048, NULL, 6, &button_task_handle);
    if (button_task_handle == NULL) {
        Serial.println("按钮任务创建失败");
        return;
    }

    // 创建游戏任务
    xTaskCreate(game_task, "game_task", 4096, NULL, 5, &game_task_handle);
    if (game_task_handle == NULL) {
        Serial.println("游戏任务创建失败");
        return;
    }
    
    Serial.println("所有任务创建成功");
    Serial.println("=== 蹦跳小火箭 V2.0 启动完成 ===");
}

void loop() {
    // 检查任务状态
    if (sensor_task_handle && eTaskGetState(sensor_task_handle) == eDeleted) {
        Serial.println("传感器任务异常退出");
    }
    
    if (display_task_handle && eTaskGetState(display_task_handle) == eDeleted) {
        Serial.println("显示任务异常退出");
    }
    
    if (sound_task_handle && eTaskGetState(sound_task_handle) == eDeleted) {
        Serial.println("音效任务异常退出");
    }
    
    if (button_task_handle && eTaskGetState(button_task_handle) == eDeleted) {
        Serial.println("按钮任务异常退出");
    }
    
    if (game_task_handle && eTaskGetState(game_task_handle) == eDeleted) {
        Serial.println("游戏任务异常退出");
    }
    
    // 定期打印系统信息和计时调试
    static uint32_t last_info_time = 0;
    uint32_t current_time = millis();
    if (current_time - last_info_time >= 15000) { // 每15秒
        Serial.println("📊 系统状态报告:");
        Serial.printf("   当前游戏状态: %d\n", current_state);
        Serial.printf("   系统运行时间: %lu 秒\n", current_time / 1000);
        Serial.printf("   跳跃次数: %lu\n", game_data.jump_count);

        if (current_state == GAME_STATE_PLAYING) {
            Serial.printf("   🎮 游戏进行中:\n");
            Serial.printf("      游戏时长: %lu 秒\n", game_data.game_time_ms / 1000);
            Serial.printf("      燃料进度: %lu%%\n", game_data.fuel_progress);
            Serial.printf("      跳跃状态: %s\n", game_data.is_jumping ? "跳跃中" : "正常");
        }

        // 打印内存使用情况
        Serial.printf("   空闲堆内存: %lu bytes\n", ESP.getFreeHeap());
        
        last_info_time = current_time;
    }
    
    delay(1000); // 1秒检查一次

#ifdef JUMPING_ROCKET_V3
    // V3.0主循环处理
    loopV3();

    // V3.0 UI模式检查和处理
    if (V3_SHOULD_ENTER_UI()) {
        V3_ENTER_UI();
    }

    // V3.0 UI模式更新
    if (V3_IS_IN_UI()) {
        V3_UPDATE_UI();
    }
#endif
}
