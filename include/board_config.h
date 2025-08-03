#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <Arduino.h>

// 开发板类型检测和配置
#ifdef BOARD_ESP32_C3
    #define BOARD_NAME "ESP32-C3 DevKit"
    #define BOARD_TYPE_ESP32_C3 1
    
    // ESP32-C3 引脚配置
    #ifndef I2C_SCL_PIN
    #define I2C_SCL_PIN                 9     // I2C SCL引脚
    #endif
    
    #ifndef I2C_SDA_PIN
    #define I2C_SDA_PIN                 8     // I2C SDA引脚
    #endif
    
    #ifndef BUTTON_PIN
    #define BUTTON_PIN                  3     // 按钮引脚
    #endif
    
    #ifndef BUZZER_PIN
    #define BUZZER_PIN                  4     // 蜂鸣器引脚
    #endif
    
    #ifndef UART_RX_PIN
    #define UART_RX_PIN                 20    // UART RX引脚
    #endif
    
    #ifndef UART_TX_PIN
    #define UART_TX_PIN                 21    // UART TX引脚
    #endif
    
    // ESP32-C3 特定配置
    #define I2C_FREQUENCY               100000  // 100kHz，更稳定
    #define BUTTON_PULL_MODE            INPUT_PULLDOWN  // 下拉模式
    #define BUTTON_ACTIVE_LEVEL         HIGH    // 高电平触发
    #define LED_BUILTIN_PIN             8       // 内置LED（如果有）
    
    // 调试信息
    #define BOARD_DEBUG_INFO() do { \
        Serial.println("📋 开发板配置: ESP32-C3 DevKit"); \
        Serial.printf("   I2C: SDA=GPIO%d, SCL=GPIO%d\n", I2C_SDA_PIN, I2C_SCL_PIN); \
        Serial.printf("   按钮: GPIO%d (高电平触发)\n", BUTTON_PIN); \
        Serial.printf("   蜂鸣器: GPIO%d\n", BUZZER_PIN); \
        Serial.printf("   UART: RX=GPIO%d, TX=GPIO%d\n", UART_RX_PIN, UART_TX_PIN); \
        Serial.printf("   I2C频率: %d Hz\n", I2C_FREQUENCY); \
    } while(0)

#elif defined(BOARD_ESP32_DEV)
    #define BOARD_NAME "ESP32 DevKit"
    #define BOARD_TYPE_ESP32_DEV 1
    
    // ESP32 标准开发板引脚配置
    #ifndef I2C_SCL_PIN
    #define I2C_SCL_PIN                 22    // I2C SCL引脚
    #endif
    
    #ifndef I2C_SDA_PIN
    #define I2C_SDA_PIN                 21    // I2C SDA引脚
    #endif
    
    #ifndef BUTTON_PIN
    #define BUTTON_PIN                  2     // 按钮引脚
    #endif
    
    #ifndef BUZZER_PIN
    #define BUZZER_PIN                  25    // 蜂鸣器引脚
    #endif
    
    #ifndef UART_RX_PIN
    #define UART_RX_PIN                 3     // UART RX引脚
    #endif
    
    #ifndef UART_TX_PIN
    #define UART_TX_PIN                 1     // UART TX引脚
    #endif
    
    // ESP32 标准开发板特定配置
    #define I2C_FREQUENCY               400000  // 400kHz，标准速度
    #define BUTTON_PULL_MODE            INPUT_PULLUP    // 上拉模式
    #define BUTTON_ACTIVE_LEVEL         LOW     // 低电平触发
    #define LED_BUILTIN_PIN             2       // 内置LED
    
    // 调试信息
    #define BOARD_DEBUG_INFO() do { \
        Serial.println("📋 开发板配置: ESP32 DevKit"); \
        Serial.printf("   I2C: SDA=GPIO%d, SCL=GPIO%d\n", I2C_SDA_PIN, I2C_SCL_PIN); \
        Serial.printf("   按钮: GPIO%d (低电平触发)\n", BUTTON_PIN); \
        Serial.printf("   蜂鸣器: GPIO%d\n", BUZZER_PIN); \
        Serial.printf("   UART: RX=GPIO%d, TX=GPIO%d\n", UART_RX_PIN, UART_TX_PIN); \
        Serial.printf("   I2C频率: %d Hz\n", I2C_FREQUENCY); \
    } while(0)

#else
    // 默认配置（向后兼容）
    #define BOARD_NAME "Unknown Board"
    #define BOARD_TYPE_UNKNOWN 1
    
    #warning "未检测到开发板类型，使用默认配置"
    
    // 默认引脚配置（ESP32-C3）
    #ifndef I2C_SCL_PIN
    #define I2C_SCL_PIN                 8
    #endif
    
    #ifndef I2C_SDA_PIN
    #define I2C_SDA_PIN                 9
    #endif
    
    #ifndef BUTTON_PIN
    #define BUTTON_PIN                  3
    #endif
    
    #ifndef BUZZER_PIN
    #define BUZZER_PIN                  4
    #endif
    
    #ifndef UART_RX_PIN
    #define UART_RX_PIN                 20
    #endif
    
    #ifndef UART_TX_PIN
    #define UART_TX_PIN                 21
    #endif
    
    // 默认配置
    #define I2C_FREQUENCY               100000
    #define BUTTON_PULL_MODE            INPUT_PULLDOWN
    #define BUTTON_ACTIVE_LEVEL         HIGH
    #define LED_BUILTIN_PIN             8
    
    // 调试信息
    #define BOARD_DEBUG_INFO() do { \
        Serial.println("⚠️  开发板配置: 未知开发板 (使用默认配置)"); \
        Serial.printf("   I2C: SDA=GPIO%d, SCL=GPIO%d\n", I2C_SDA_PIN, I2C_SCL_PIN); \
        Serial.printf("   按钮: GPIO%d\n", BUTTON_PIN); \
        Serial.printf("   蜂鸣器: GPIO%d\n", BUZZER_PIN); \
        Serial.printf("   UART: RX=GPIO%d, TX=GPIO%d\n", UART_RX_PIN, UART_TX_PIN); \
        Serial.println("   请在platformio.ini中正确配置开发板类型"); \
    } while(0)

#endif

// 通用配置验证宏
#define VALIDATE_PIN_CONFIG() do { \
    Serial.println("🔍 验证引脚配置..."); \
    if (I2C_SDA_PIN == I2C_SCL_PIN) { \
        Serial.println("❌ 错误: I2C SDA和SCL引脚不能相同!"); \
    } \
    if (BUTTON_PIN == BUZZER_PIN) { \
        Serial.println("❌ 错误: 按钮和蜂鸣器引脚不能相同!"); \
    } \
    Serial.println("✅ 引脚配置验证完成"); \
} while(0)

// 开发板能力检测
#ifdef BOARD_ESP32_C3
    #define HAS_WIFI 1
    #define HAS_BLUETOOTH 1
    #define HAS_USB_SERIAL 1
    #ifndef BOARD_MAX_GPIO_NUM
    #define BOARD_MAX_GPIO_NUM 21
    #endif
#elif defined(BOARD_ESP32_DEV)
    #define HAS_WIFI 1
    #define HAS_BLUETOOTH 1
    #define HAS_USB_SERIAL 0
    #ifndef BOARD_MAX_GPIO_NUM
    #define BOARD_MAX_GPIO_NUM 39
    #endif
#else
    #define HAS_WIFI 1
    #define HAS_BLUETOOTH 1
    #define HAS_USB_SERIAL 0
    #ifndef BOARD_MAX_GPIO_NUM
    #define BOARD_MAX_GPIO_NUM 39
    #endif
#endif

// 引脚有效性检查宏
#define IS_VALID_GPIO(pin) ((pin) >= 0 && (pin) <= BOARD_MAX_GPIO_NUM)

// 初始化开发板配置的函数声明
void board_config_init(void);
void board_config_print_info(void);
bool board_config_validate(void);

// 配置获取函数声明
const char* board_get_type_string(void);
bool board_is_esp32_c3(void);
bool board_is_esp32_dev(void);
uint32_t board_get_i2c_frequency(void);
void board_get_button_config(uint8_t* pin, uint8_t* pull_mode, uint8_t* active_level);

#endif // BOARD_CONFIG_H
