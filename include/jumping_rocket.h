#ifndef JUMPING_ROCKET_H
#define JUMPING_ROCKET_H

#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sdkconfig.h"

// 硬件引脚定义
#define I2C_MASTER_SCL_IO           22    // I2C SCL引脚
#define I2C_MASTER_SDA_IO           21    // I2C SDA引脚
#define I2C_MASTER_NUM              0     // I2C端口号
#define I2C_MASTER_FREQ_HZ          400000 // I2C频率

#define BUTTON_GPIO                 2     // 按钮引脚
#define BUZZER_GPIO                 25    // 蜂鸣器引脚

#define OLED_WIDTH                  128   // OLED宽度
#define OLED_HEIGHT                 64    // OLED高度

// MPU6050传感器地址和寄存器
#define MPU6050_ADDR                0x68
#define MPU6050_ACCEL_XOUT_H        0x3B
#define MPU6050_PWR_MGMT_1          0x6B

// 游戏状态枚举
typedef enum {
    GAME_STATE_IDLE,              // 待机状态
    GAME_STATE_DIFFICULTY_SELECT, // 难度选择
    GAME_STATE_PLAYING,           // 游戏中
    GAME_STATE_PAUSED,            // 暂停状态
    GAME_STATE_RESET_CONFIRM,     // 重置确认
    GAME_STATE_LAUNCHING,         // 火箭发射
    GAME_STATE_RESULT             // 结算状态
} game_state_t;

// 游戏难度枚举
typedef enum {
    DIFFICULTY_EASY = 0,    // 简单：60%燃料发射
    DIFFICULTY_NORMAL = 1,  // 普通：80%燃料发射
    DIFFICULTY_HARD = 2     // 困难：100%燃料发射
} game_difficulty_t;

// 按钮事件枚举
typedef enum {
    BUTTON_EVENT_NONE,      // 无事件
    BUTTON_EVENT_SHORT_PRESS, // 短按
    BUTTON_EVENT_LONG_PRESS   // 长按
} button_event_t;

// 音效类型枚举
typedef enum {
    SOUND_BOOT,             // 开机音效
    SOUND_GAME_START,       // 游戏开始
    SOUND_JUMP,             // 跳跃音效
    SOUND_PAUSE,            // 暂停音效
    SOUND_RESUME,           // 继续音效
    SOUND_RESET_WARNING,    // 重置警告
    SOUND_ROCKET_LAUNCH,    // 火箭发射
    SOUND_VICTORY           // 胜利音效
} sound_type_t;

// 游戏数据结构
typedef struct {
    uint32_t jump_count;        // 跳跃次数
    uint32_t game_time_ms;      // 游戏时长(毫秒)
    uint32_t fuel_progress;     // 燃料进度(0-100)
    uint32_t flight_height;     // 飞行高度(分数)
    bool is_jumping;            // 是否正在跳跃
    uint32_t last_jump_time;    // 上次跳跃时间
} game_data_t;

// 传感器数据结构
typedef struct {
    float accel_x;
    float accel_y;
    float accel_z;
    float magnitude;
    bool jump_detected;
} sensor_data_t;

// 全局变量声明
extern game_state_t current_state;
extern game_data_t game_data;
extern sensor_data_t sensor_data;
extern game_difficulty_t selected_difficulty;

// 函数声明

// 硬件初始化
bool hardware_init(void);
bool i2c_master_init(void);
bool mpu6050_init(void);
bool oled_init(void);
bool buzzer_init(void);
bool button_init(void);

// 传感器相关
bool mpu6050_read_accel(float* accel_x, float* accel_y, float* accel_z);
bool detect_jump(float accel_x, float accel_y, float accel_z);
void sensor_task(void* pvParameters);

// 显示相关
void oled_clear(void);
void oled_display_text(int x, int y, const char* text, int size);
void oled_display_progress_bar(int x, int y, int width, int height, int progress);
void oled_display_boot_animation(void);
void oled_display_rocket_launch_animation(void);
void oled_display_game_screen(void);
void oled_display_pause_screen(void);
void oled_display_reset_confirm_screen(void);
void oled_display_result_screen(void);
void display_task(void* pvParameters);

// 中文显示相关
void oled_display_chinese_text(int x, int y, const char* text, const uint8_t* font);
void oled_display_mixed_text(int x, int y, const char* text, const uint8_t* font);
void oled_display_chinese_tiny(int x, int y, const char* text);
void oled_display_chinese_small(int x, int y, const char* text);
void oled_display_chinese_medium(int x, int y, const char* text);
void oled_display_chinese_large(int x, int y, const char* text);

// 音效相关
void buzzer_play_tone(int frequency, int duration_ms);
void buzzer_play_melody(const int* frequencies, const int* durations, int length);
void play_sound_effect(sound_type_t sound_type);
void sound_task(void* pvParameters);

// 按钮相关
button_event_t button_get_event(void);
void button_task(void* pvParameters);

// 游戏逻辑
void game_state_machine(void);
void game_reset(void);
void game_start(void);
void game_pause(void);
void game_resume(void);
void game_calculate_result(void);
void game_update_data(void);
void game_task(void* pvParameters);

// 工具函数
uint32_t get_time_ms(void);
void delay_ms(uint32_t ms);

// 按钮事件处理
void handle_button_event(button_event_t event);

// 数据处理器
void data_processor_init(void);
void add_jump_record(uint32_t timestamp);
void update_game_statistics(void);
float calculate_jump_frequency(void);
float calculate_exercise_intensity(void);
float calculate_calories_burned(void);
void generate_exercise_report(char* report_buffer, size_t buffer_size);

#endif // JUMPING_ROCKET_H
