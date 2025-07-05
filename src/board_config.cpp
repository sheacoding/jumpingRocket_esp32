#include "board_config.h"

// 初始化开发板配置
void board_config_init(void) {
    Serial.println("========================================");
    Serial.println("🔧 开发板配置初始化");
    Serial.println("========================================");
    
    // 显示开发板信息
    board_config_print_info();
    
    // 验证配置
    if (!board_config_validate()) {
        Serial.println("❌ 开发板配置验证失败!");
        return;
    }
    
    Serial.println("✅ 开发板配置初始化完成");
    Serial.println("========================================");
}

// 打印开发板配置信息
void board_config_print_info(void) {
    Serial.println("📋 开发板详细信息:");
    Serial.printf("   开发板名称: %s\n", BOARD_NAME);
    
    // 显示芯片信息
    Serial.printf("   芯片型号: %s\n", ESP.getChipModel());
    Serial.printf("   芯片版本: %d\n", ESP.getChipRevision());
    Serial.printf("   CPU频率: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("   Flash大小: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    
    // 显示引脚配置
    BOARD_DEBUG_INFO();
    
    // 显示开发板能力
    Serial.println("📡 开发板能力:");
    Serial.printf("   WiFi: %s\n", HAS_WIFI ? "支持" : "不支持");
    Serial.printf("   蓝牙: %s\n", HAS_BLUETOOTH ? "支持" : "不支持");
    Serial.printf("   USB串口: %s\n", HAS_USB_SERIAL ? "支持" : "不支持");
    Serial.printf("   最大GPIO编号: %d\n", BOARD_MAX_GPIO_NUM);
    
    // 显示编译时配置
    Serial.println("🔨 编译配置:");
    #ifdef BOARD_ESP32_C3
    Serial.println("   开发板类型: ESP32-C3 (BOARD_ESP32_C3)");
    #elif defined(BOARD_ESP32_DEV)
    Serial.println("   开发板类型: ESP32 DevKit (BOARD_ESP32_DEV)");
    #else
    Serial.println("   开发板类型: 未知 (使用默认配置)");
    #endif
    
    #ifdef CORE_DEBUG_LEVEL
    Serial.printf("   调试级别: %d\n", CORE_DEBUG_LEVEL);
    #endif
    
    Serial.printf("   编译时间: %s %s\n", __DATE__, __TIME__);
}

// 验证开发板配置
bool board_config_validate(void) {
    Serial.println("🔍 验证开发板配置...");
    bool is_valid = true;
    
    // 检查引脚冲突
    if (I2C_SDA_PIN == I2C_SCL_PIN) {
        Serial.println("❌ 错误: I2C SDA和SCL引脚不能相同!");
        is_valid = false;
    }
    
    if (BUTTON_PIN == BUZZER_PIN) {
        Serial.println("❌ 错误: 按钮和蜂鸣器引脚不能相同!");
        is_valid = false;
    }
    
    if (I2C_SDA_PIN == BUTTON_PIN || I2C_SDA_PIN == BUZZER_PIN) {
        Serial.println("❌ 错误: I2C SDA引脚与其他功能引脚冲突!");
        is_valid = false;
    }
    
    if (I2C_SCL_PIN == BUTTON_PIN || I2C_SCL_PIN == BUZZER_PIN) {
        Serial.println("❌ 错误: I2C SCL引脚与其他功能引脚冲突!");
        is_valid = false;
    }
    
    // 检查引脚有效性
    if (!IS_VALID_GPIO(I2C_SDA_PIN)) {
        Serial.printf("❌ 错误: I2C SDA引脚 %d 无效! (有效范围: 0-%d)\n", I2C_SDA_PIN, BOARD_MAX_GPIO_NUM);
        is_valid = false;
    }

    if (!IS_VALID_GPIO(I2C_SCL_PIN)) {
        Serial.printf("❌ 错误: I2C SCL引脚 %d 无效! (有效范围: 0-%d)\n", I2C_SCL_PIN, BOARD_MAX_GPIO_NUM);
        is_valid = false;
    }

    if (!IS_VALID_GPIO(BUTTON_PIN)) {
        Serial.printf("❌ 错误: 按钮引脚 %d 无效! (有效范围: 0-%d)\n", BUTTON_PIN, BOARD_MAX_GPIO_NUM);
        is_valid = false;
    }

    if (!IS_VALID_GPIO(BUZZER_PIN)) {
        Serial.printf("❌ 错误: 蜂鸣器引脚 %d 无效! (有效范围: 0-%d)\n", BUZZER_PIN, BOARD_MAX_GPIO_NUM);
        is_valid = false;
    }
    
    // ESP32-C3 特定检查
    #ifdef BOARD_ESP32_C3
    // ESP32-C3 的一些引脚有特殊用途
    if (I2C_SDA_PIN >= 18 && I2C_SDA_PIN <= 19) {
        Serial.printf("⚠️  警告: GPIO%d 在ESP32-C3上用于USB，可能影响调试\n", I2C_SDA_PIN);
    }
    if (I2C_SCL_PIN >= 18 && I2C_SCL_PIN <= 19) {
        Serial.printf("⚠️  警告: GPIO%d 在ESP32-C3上用于USB，可能影响调试\n", I2C_SCL_PIN);
    }
    #endif
    
    // ESP32 标准开发板特定检查
    #ifdef BOARD_ESP32_DEV
    // ESP32 的一些引脚有特殊用途
    if (I2C_SDA_PIN == 0 || I2C_SCL_PIN == 0) {
        Serial.println("⚠️  警告: GPIO0 用于启动模式选择，使用时需注意");
    }
    if (I2C_SDA_PIN == 2 || I2C_SCL_PIN == 2) {
        Serial.println("⚠️  警告: GPIO2 连接到内置LED，可能影响I2C通信");
    }
    #endif
    
    if (is_valid) {
        Serial.println("✅ 开发板配置验证通过");
        
        // 显示配置摘要
        Serial.println("📋 配置摘要:");
        Serial.printf("   I2C: SDA=GPIO%d, SCL=GPIO%d, 频率=%dHz\n", 
                     I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);
        Serial.printf("   按钮: GPIO%d (%s触发)\n", 
                     BUTTON_PIN, (BUTTON_ACTIVE_LEVEL == HIGH) ? "高电平" : "低电平");
        Serial.printf("   蜂鸣器: GPIO%d\n", BUZZER_PIN);
        Serial.printf("   UART: RX=GPIO%d, TX=GPIO%d\n", UART_RX_PIN, UART_TX_PIN);
    } else {
        Serial.println("❌ 开发板配置验证失败，请检查platformio.ini配置");
    }
    
    return is_valid;
}

// 获取开发板类型字符串
const char* board_get_type_string(void) {
    #ifdef BOARD_ESP32_C3
    return "ESP32-C3";
    #elif defined(BOARD_ESP32_DEV)
    return "ESP32-DEV";
    #else
    return "UNKNOWN";
    #endif
}

// 检查是否为特定开发板类型
bool board_is_esp32_c3(void) {
    #ifdef BOARD_ESP32_C3
    return true;
    #else
    return false;
    #endif
}

bool board_is_esp32_dev(void) {
    #ifdef BOARD_ESP32_DEV
    return true;
    #else
    return false;
    #endif
}

// 获取推荐的I2C频率
uint32_t board_get_i2c_frequency(void) {
    return I2C_FREQUENCY;
}

// 获取按钮配置
void board_get_button_config(uint8_t* pin, uint8_t* pull_mode, uint8_t* active_level) {
    if (pin) *pin = BUTTON_PIN;
    if (pull_mode) *pull_mode = BUTTON_PULL_MODE;
    if (active_level) *active_level = BUTTON_ACTIVE_LEVEL;
}
