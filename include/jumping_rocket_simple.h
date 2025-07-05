#ifndef JUMPING_ROCKET_SIMPLE_H
#define JUMPING_ROCKET_SIMPLE_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "board_config.h"

#define OLED_WIDTH                  128   // OLED宽度
#define OLED_HEIGHT                 64    // OLED高度
#define OLED_ADDRESS                0x3C  // OLED I2C地址

// MPU6050传感器定义
#define MPU6050_ADDR                0x68
#define MPU6050_PWR_MGMT_1          0x6B
#define MPU6050_ACCEL_XOUT_H        0x3B

// 游戏状态枚举
typedef enum {
    GAME_STATE_IDLE,        // 待机状态
    GAME_STATE_DIFFICULTY_SELECT, // 难度选择状态
    GAME_STATE_PLAYING,     // 游戏中
    GAME_STATE_PAUSED,      // 暂停状态
    GAME_STATE_RESET_CONFIRM, // 重置确认
    GAME_STATE_LAUNCHING,   // 火箭发射动画状态
    GAME_STATE_RESULT       // 结算状态
} game_state_t;

// 游戏难度枚举
typedef enum {
    DIFFICULTY_EASY,        // 简单模式 - 60%燃料触发
    DIFFICULTY_NORMAL,      // 普通模式 - 80%燃料触发
    DIFFICULTY_HARD         // 困难模式 - 100%燃料触发
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
    SOUND_VICTORY,          // 胜利音效
    SOUND_DIFFICULTY_SELECT, // 难度选择音效
    SOUND_DIFFICULTY_CONFIRM // 难度确认音效
} sound_type_t;

// 游戏数据结构
typedef struct {
    uint32_t jump_count;        // 跳跃次数
    uint32_t game_time_ms;      // 游戏时长(毫秒)
    uint32_t fuel_progress;     // 燃料进度(0-100)
    uint32_t flight_height;     // 飞行高度(分数)
    bool is_jumping;            // 是否正在跳跃
    uint32_t last_jump_time;    // 上次跳跃时间
    game_difficulty_t difficulty; // 当前游戏难度
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
extern game_difficulty_t selected_difficulty; // 当前选中的难度

// 函数声明 - 使用C++兼容的声明
#ifdef __cplusplus
extern "C" {
#endif

// 硬件初始化
bool hardware_init(void);
bool buzzer_init(void);
bool button_init(void);

// 这些函数在各自模块中声明和实现：
// - mpu6050_init_sensor, mpu6050_read_accel 在 sensor.cpp 中
// - oled_init 在 display.cpp 中

// I2C和调试相关函数
void i2c_scan(void);

// OLED相关函数（由U8g2库处理）

// 传感器相关
bool mpu6050_init_sensor(void);
bool mpu6050_read_accel(float* accel_x, float* accel_y, float* accel_z);
bool detect_jump(float accel_x, float accel_y, float accel_z);
void sensor_task(void* pvParameters);

// 显示相关
void oled_clear(void);
void oled_display_text(int x, int y, const char* text);
void oled_display_progress_bar(int x, int y, int width, int height, int progress);
void draw_icon(int x, int y, const uint8_t* icon_data);
void draw_large_icon(int x, int y, const uint8_t* icon_data);
void draw_labeled_value(int x, int y, const uint8_t* icon, const char* label, const char* value);
void draw_horizontal_icon_text(int x, int y, const uint8_t* icon, const char* text);

// SVG映射和动画相关函数
int svg_transform_x(int svg_x, int svg_translate_x);
int svg_transform_y(int svg_y, int svg_translate_y);
uint32_t svg_animate_progress(uint32_t start_time, uint32_t duration_ms);
bool svg_opacity_visible(float opacity, uint32_t time_offset);
float ease_out(float t);
void start_jump_animation(void);
void start_fuel_animation(uint32_t target_fuel);
void start_rocket_launch_animation(void);
void oled_display_boot_animation(void);
void oled_display_rocket_launch_animation(void);
void oled_display_idle_screen(void);
void oled_display_difficulty_select_screen(void);
void oled_display_game_screen(void);
void oled_display_pause_screen(void);
void oled_display_reset_confirm_screen(void);
void oled_display_result_screen(void);
void display_task(void* pvParameters);

// 音效相关
void buzzer_play_tone(int frequency, int duration_ms);
void play_sound_effect(sound_type_t sound_type);
void sound_task(void* pvParameters);

// 按钮相关
button_event_t button_get_event(void);
void button_task(void* pvParameters);

// 游戏逻辑
void game_data_init(void);
void game_state_machine(void);
void game_reset(void);
void game_start(void);
void game_pause(void);
void game_resume(void);
void game_calculate_result(void);
void game_update_data(void);
void game_task(void* pvParameters);

// 难度选择相关
void difficulty_select_init(void);
void difficulty_select_next(void);
void difficulty_select_confirm(void);
uint32_t get_difficulty_fuel_threshold(game_difficulty_t difficulty);
const char* get_difficulty_name(game_difficulty_t difficulty);

// 按钮事件处理
void handle_button_event(button_event_t event);

// 数据处理器
void data_processor_init(void);
void add_jump_record(uint32_t timestamp);
void update_game_statistics(void);

// 工具函数
uint32_t get_time_ms(void);

#ifdef __cplusplus
}
#endif

#endif // JUMPING_ROCKET_SIMPLE_H
