#include "jumping_rocket_simple.h"

// I2C扫描函数
void i2c_scan(void) {
    Serial.println("🔍 开始I2C设备扫描...");
    Serial.println("   扫描地址范围: 0x08 - 0x77");
    Serial.printf("   I2C引脚配置: SDA=GPIO%d, SCL=GPIO%d\n", I2C_SDA_PIN, I2C_SCL_PIN);

    int device_count = 0;
    bool oled_found = false;
    bool mpu6050_found = false;

    for (uint8_t address = 0x08; address <= 0x77; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            Serial.printf("✅ 发现I2C设备，地址: 0x%02X", address);

            // 识别常见设备
            switch (address) {
                case 0x3C:
                case 0x3D:
                    Serial.print(" (OLED SSD1306) ⭐");
                    oled_found = true;
                    break;
                case 0x68:
                case 0x69:
                    Serial.print(" (MPU6050) ⭐");
                    mpu6050_found = true;
                    break;
                default:
                    Serial.print(" (未知设备)");
                    break;
            }
            Serial.println();
            device_count++;
        }
        delay(5);
    }

    Serial.println("----------------------------------------");
    if (device_count == 0) {
        Serial.println("❌ 未发现任何I2C设备！");
        Serial.println("🔧 请检查连接：");
        Serial.printf("   SCL -> GPIO%d\n", I2C_SCL_PIN);
        Serial.printf("   SDA -> GPIO%d\n", I2C_SDA_PIN);
        Serial.println("   VCC -> 3.3V");
        Serial.println("   GND -> GND");
        Serial.println("   确保设备供电正常");
    } else {
        Serial.printf("✅ 总共发现 %d 个I2C设备\n", device_count);

        // 检查关键设备
        if (!oled_found) {
            Serial.println("⚠️  警告: 未发现OLED显示屏 (地址0x3C)");
        }
        if (!mpu6050_found) {
            Serial.println("⚠️  警告: 未发现MPU6050传感器 (地址0x68)");
        }
        if (oled_found && mpu6050_found) {
            Serial.println("🎉 所有关键设备都已找到！");
        }
    }
    Serial.println("🔍 I2C扫描完成\n");
}

// 这些函数现在在各自的模块中实现：
// - mpu6050_init 和 mpu6050_read_accel 在 sensor.cpp 中
// - oled_init 在 display.cpp 中

// 蜂鸣器初始化
bool buzzer_init(void) {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("蜂鸣器初始化成功");
    return true;
}

// 按钮初始化（修正为高电平触发按钮）
bool button_init(void) {
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);  // 改为下拉电阻，默认低电平
    Serial.println("按钮初始化成功（高电平触发模式）");
    return true;
}

// 硬件总初始化
bool hardware_init(void) {
    Serial.println("========================================");
    Serial.println("蹦跳小火箭 V2.0 - 硬件初始化开始");
    Serial.println("========================================");

    // 显示引脚配置
    Serial.println("引脚配置:");
    Serial.printf("  I2C SCL: GPIO%d\n", I2C_SCL_PIN);
    Serial.printf("  I2C SDA: GPIO%d\n", I2C_SDA_PIN);
    Serial.printf("  按钮:    GPIO%d\n", BUTTON_PIN);
    Serial.printf("  蜂鸣器:  GPIO%d\n", BUZZER_PIN);
    Serial.println();

    // 初始化I2C
    Serial.println("初始化I2C总线...");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000); // 设置为100kHz，提高兼容性
    delay(100);
    Serial.println("I2C总线初始化完成");

    // 扫描I2C设备
    i2c_scan();

    // 初始化蜂鸣器（先初始化，用于状态指示）
    Serial.println("1. 初始化蜂鸣器...");
    if (!buzzer_init()) {
        Serial.println("❌ 蜂鸣器初始化失败");
        return false;
    }
    Serial.println("✅ 蜂鸣器初始化成功");

    // 播放初始化音效
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);

    // 初始化按钮
    Serial.println("2. 初始化按钮...");
    if (!button_init()) {
        Serial.println("❌ 按钮初始化失败");
        return false;
    }
    Serial.println("✅ 按钮初始化成功");

    // MPU6050和OLED初始化现在在各自的任务中进行
    Serial.println("3. MPU6050传感器将在传感器任务中初始化");
    Serial.println("✅ MPU6050初始化准备完成");

    Serial.println("4. OLED显示屏将在显示任务中初始化");
    Serial.println("✅ OLED初始化准备完成");

    // 播放成功音效
    Serial.println("播放初始化成功音效...");
    int success_melody[] = {262, 330, 392, 523}; // C-E-G-C
    for (int i = 0; i < 4; i++) {
        // 简单的PWM音调生成
        for (int j = 0; j < 100; j++) {
            digitalWrite(BUZZER_PIN, HIGH);
            delayMicroseconds(1000000 / success_melody[i] / 2);
            digitalWrite(BUZZER_PIN, LOW);
            delayMicroseconds(1000000 / success_melody[i] / 2);
        }
        delay(100);
    }

    Serial.println("========================================");
    Serial.println("✅ 所有硬件初始化完成！");
    Serial.println("系统准备就绪，开始游戏循环...");
    Serial.println("========================================");
    return true;
}
