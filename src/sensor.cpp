#include "jumping_rocket_simple.h"

// Adafruit MPU6050对象
Adafruit_MPU6050 mpu;

// 跳跃检测参数（平衡灵敏度和稳定性）
#define JUMP_THRESHOLD_HIGH     1.5f    // 跳跃检测高阈值(g) - 适中的灵敏度
#define JUMP_THRESHOLD_LOW      0.8f    // 跳跃检测低阈值(g) - 确保能检测到着地
#define JUMP_MIN_DURATION       80      // 最小跳跃持续时间(ms) - 过滤短暂噪声
#define JUMP_MAX_DURATION       1500    // 最大跳跃持续时间(ms)
#define JUMP_COOLDOWN           250     // 跳跃冷却时间(ms) - 适中的冷却时间

// 滤波器参数
#define FILTER_ALPHA            0.7f    // 低通滤波器系数 - 调整响应性

// 调试输出控制
#define DEBUG_SENSOR_DATA       false   // 是否输出详细传感器数据
#define DEBUG_JUMP_DETECTION    true    // 是否输出跳跃检测调试信息

// 传感器数据
sensor_data_t sensor_data = {0};

// 跳跃检测状态
typedef enum {
    JUMP_STATE_IDLE,        // 空闲状态
    JUMP_STATE_RISING,      // 上升阶段
    JUMP_STATE_FALLING,     // 下降阶段
    JUMP_STATE_COOLDOWN     // 冷却阶段
} jump_state_t;

static jump_state_t jump_state = JUMP_STATE_IDLE;
static uint32_t jump_start_time = 0;
static uint32_t last_jump_time = 0;
static float filtered_magnitude = 1.0f;

// 低通滤波器
static float low_pass_filter(float current_value, float previous_filtered) {
    return FILTER_ALPHA * previous_filtered + (1.0f - FILTER_ALPHA) * current_value;
}

// 跳跃检测算法
bool detect_jump(float accel_x, float accel_y, float accel_z) {
    uint32_t current_time = millis();

    // 计算加速度幅值
    float magnitude = sqrt(accel_x * accel_x + accel_y * accel_y + accel_z * accel_z);

    // 应用低通滤波器
    filtered_magnitude = low_pass_filter(magnitude, filtered_magnitude);

    // 更新传感器数据
    sensor_data.accel_x = accel_x;
    sensor_data.accel_y = accel_y;
    sensor_data.accel_z = accel_z;
    sensor_data.magnitude = filtered_magnitude;

    // 调试输出传感器数据
    if (DEBUG_SENSOR_DATA) {
        static uint32_t last_debug_time = 0;
        if (current_time - last_debug_time >= 1000) { // 每秒输出一次
            Serial.printf("传感器数据 - X:%.2f Y:%.2f Z:%.2f 幅值:%.2f 滤波:%.2f\n",
                         accel_x, accel_y, accel_z, magnitude, filtered_magnitude);
            last_debug_time = current_time;
        }
    }

    bool jump_detected = false;

    switch (jump_state) {
        case JUMP_STATE_IDLE:
            // 检测跳跃开始（加速度突然增大）
            if (filtered_magnitude > JUMP_THRESHOLD_HIGH) {
                jump_state = JUMP_STATE_RISING;
                jump_start_time = current_time;
                if (DEBUG_JUMP_DETECTION) {
                    Serial.printf("🚀 跳跃开始检测，幅值: %.2f (阈值: %.2f)\n",
                                 filtered_magnitude, JUMP_THRESHOLD_HIGH);
                }
            }
            break;

        case JUMP_STATE_RISING:
            // 检测跳跃峰值后的下降
            if (filtered_magnitude < JUMP_THRESHOLD_LOW) {
                jump_state = JUMP_STATE_FALLING;
                if (DEBUG_JUMP_DETECTION) {
                    Serial.printf("📉 跳跃下降阶段，幅值: %.2f (阈值: %.2f)\n",
                                 filtered_magnitude, JUMP_THRESHOLD_LOW);
                }
            } else if (current_time - jump_start_time > JUMP_MAX_DURATION) {
                // 超时，重置状态
                jump_state = JUMP_STATE_IDLE;
                if (DEBUG_JUMP_DETECTION) {
                    Serial.printf("⏰ 跳跃检测超时，重置状态 (持续时间: %lu ms)\n",
                                 current_time - jump_start_time);
                }
            }
            break;

        case JUMP_STATE_FALLING:
            // 检测着地（加速度恢复正常）
            if (filtered_magnitude > 0.8f && filtered_magnitude < 1.2f) {
                uint32_t jump_duration = current_time - jump_start_time;

                // 验证跳跃持续时间
                if (jump_duration >= JUMP_MIN_DURATION && jump_duration <= JUMP_MAX_DURATION) {
                    // 检查冷却时间
                    if (current_time - last_jump_time >= JUMP_COOLDOWN) {
                        jump_detected = true;
                        last_jump_time = current_time;
                        Serial.printf("✅ 跳跃检测成功！持续时间: %lu ms, 幅值: %.2f\n",
                                     jump_duration, filtered_magnitude);
                    } else {
                        if (DEBUG_JUMP_DETECTION) {
                            Serial.printf("❄️ 跳跃在冷却期内，忽略 (剩余: %lu ms)\n",
                                         JUMP_COOLDOWN - (current_time - last_jump_time));
                        }
                    }
                } else {
                    if (DEBUG_JUMP_DETECTION) {
                        Serial.printf("⚠️ 跳跃持续时间不符合要求: %lu ms (要求: %d-%d ms)\n",
                                     jump_duration, JUMP_MIN_DURATION, JUMP_MAX_DURATION);
                    }
                }

                jump_state = JUMP_STATE_COOLDOWN;
            } else if (current_time - jump_start_time > JUMP_MAX_DURATION) {
                // 超时，重置状态
                jump_state = JUMP_STATE_IDLE;
                if (DEBUG_JUMP_DETECTION) {
                    Serial.printf("⏰ 跳跃着地检测超时，重置状态 (持续时间: %lu ms)\n",
                                 current_time - jump_start_time);
                }
            }
            break;

        case JUMP_STATE_COOLDOWN:
            // 冷却期，等待状态稳定
            if (current_time - last_jump_time >= JUMP_COOLDOWN) {
                jump_state = JUMP_STATE_IDLE;
                if (DEBUG_JUMP_DETECTION) {
                    Serial.println("🔄 跳跃冷却完成，状态重置");
                }
            }
            break;
    }

    sensor_data.jump_detected = jump_detected;
    return jump_detected;
}

// MPU6050初始化（使用Adafruit库）
bool mpu6050_init_sensor(void) {
    Serial.println("🔧 开始Adafruit MPU6050初始化...");

    // 检查I2C连接
    Wire.beginTransmission(MPU6050_ADDR);
    uint8_t error = Wire.endTransmission();

    if (error != 0) {
        Serial.printf("❌ MPU6050 I2C连接失败，错误代码: %d\n", error);
        Serial.printf("   请检查连接: SDA->GPIO%d, SCL->GPIO%d\n", I2C_SDA_PIN, I2C_SCL_PIN);
        return false;
    }
    Serial.println("✅ MPU6050 I2C连接成功");

    // 初始化Adafruit库
    Serial.println("   正在初始化Adafruit MPU6050库...");
    if (!mpu.begin()) {
        Serial.println("❌ MPU6050库初始化失败！请检查连接");
        return false;
    }
    Serial.println("✅ MPU6050库初始化成功");

    // 设置加速度计范围为±2g
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    Serial.println("✅ 加速度计范围设置为±2g");

    // 设置陀螺仪范围为±250度/秒
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    Serial.println("✅ 陀螺仪范围设置为±250°/s");

    // 设置滤波器带宽
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("✅ 滤波器带宽设置为21Hz");

    // 测试读取数据
    Serial.println("   正在测试传感器数据读取...");
    sensors_event_t a, g, temp;
    if (mpu.getEvent(&a, &g, &temp)) {
        Serial.printf("✅ 传感器测试成功 - 加速度: X=%.2f, Y=%.2f, Z=%.2f m/s²\n",
                     a.acceleration.x, a.acceleration.y, a.acceleration.z);
        Serial.printf("   温度: %.1f°C\n", temp.temperature);
    } else {
        Serial.println("❌ 传感器数据读取测试失败");
        return false;
    }

    Serial.println("🎉 Adafruit MPU6050初始化完全成功");
    return true;
}

// 读取MPU6050加速度数据（使用Adafruit库）
bool mpu6050_read_accel(float* accel_x, float* accel_y, float* accel_z) {
    sensors_event_t a, g, temp;

    if (!mpu.getEvent(&a, &g, &temp)) {
        return false;
    }

    *accel_x = a.acceleration.x / 9.8f; // 转换为g单位
    *accel_y = a.acceleration.y / 9.8f;
    *accel_z = a.acceleration.z / 9.8f;

    return true;
}

// 传感器任务
void sensor_task(void* pvParameters) {
    Serial.println("传感器任务启动");

    // 初始化MPU6050
    if (!mpu6050_init_sensor()) {
        Serial.println("传感器初始化失败，任务退出");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        float accel_x, accel_y, accel_z;

        // 读取加速度数据
        if (mpu6050_read_accel(&accel_x, &accel_y, &accel_z)) {
            // 执行跳跃检测
            bool jump_detected = detect_jump(accel_x, accel_y, accel_z);

            // 如果检测到跳跃
            if (jump_detected) {
                // 如果在待机状态，需要更严格的条件才能启动游戏
                if (current_state == GAME_STATE_IDLE) {
                    // 增加额外验证：需要连续的明显跳跃动作
                    static uint32_t idle_jump_count = 0;
                    static uint32_t last_idle_jump_time = 0;

                    uint32_t current_time = millis();

                    // 如果距离上次跳跃超过2秒，重置计数
                    if (current_time - last_idle_jump_time > 2000) {
                        idle_jump_count = 0;
                    }

                    idle_jump_count++;
                    last_idle_jump_time = current_time;

                    Serial.printf("🔍 待机状态跳跃检测: 第%lu次，需要连续2次明确跳跃才能启动游戏\n", idle_jump_count);

                    // 需要连续2次明确的跳跃才能进入难度选择
                    if (idle_jump_count >= 2) {
                        Serial.println("🚀 连续跳跃确认，进入难度选择界面！");
                        Serial.printf("   触发时间: %lu ms\n", millis());

                        // 切换到难度选择状态
                        current_state = GAME_STATE_DIFFICULTY_SELECT;
                        difficulty_select_init();

                        // 重置待机跳跃计数
                        idle_jump_count = 0;
                    } else {
                        Serial.printf("   等待更多跳跃确认 (%lu/2)\n", idle_jump_count);
                    }
                }
                // 如果游戏正在进行，更新跳跃计数
                else if (current_state == GAME_STATE_PLAYING) {
                    game_data.jump_count++;
                    game_data.is_jumping = true;
                    game_data.last_jump_time = millis();

                    // 播放跳跃音效
                    play_sound_effect(SOUND_JUMP);

                    Serial.printf("⬆️ 跳跃计数: %lu，时间: %lu ms\n",
                                 game_data.jump_count, millis());
                }
            }
        } else {
            Serial.println("读取传感器数据失败");
        }

        // 重置跳跃标志
        if (game_data.is_jumping && (millis() - game_data.last_jump_time > 500)) {
            game_data.is_jumping = false;
        }

        // 等待下次采样
        delay(20); // 50Hz采样率
    }
}
