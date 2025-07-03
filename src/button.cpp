#include "jumping_rocket_simple.h"

// 按钮状态定义
#define BUTTON_PRESSED      LOW
#define BUTTON_RELEASED     HIGH

// 按钮参数
#define DEBOUNCE_TIME_MS    50      // 防抖时间
#define LONG_PRESS_TIME_MS  1000    // 长按时间阈值

// 按钮状态变量
static bool button_last_state = BUTTON_RELEASED;
static uint32_t button_press_time = 0;
static uint32_t button_release_time = 0;
static bool button_long_press_triggered = false;

// 按钮事件队列
static QueueHandle_t button_event_queue = NULL;

// 获取按钮当前状态
static bool get_button_state(void) {
    return digitalRead(BUTTON_PIN);
}

// 按钮事件检测
static button_event_t detect_button_event(void) {
    bool current_state = get_button_state();
    uint32_t current_time = millis();
    button_event_t event = BUTTON_EVENT_NONE;
    
    // 检测按钮按下
    if (current_state == BUTTON_PRESSED && button_last_state == BUTTON_RELEASED) {
        // 防抖检查
        if (current_time - button_release_time >= DEBOUNCE_TIME_MS) {
            button_press_time = current_time;
            button_long_press_triggered = false;
            Serial.println("按钮按下");
        }
    }
    // 检测按钮释放
    else if (current_state == BUTTON_RELEASED && button_last_state == BUTTON_PRESSED) {
        // 防抖检查
        if (current_time - button_press_time >= DEBOUNCE_TIME_MS) {
            button_release_time = current_time;
            uint32_t press_duration = button_release_time - button_press_time;
            
            if (!button_long_press_triggered) {
                if (press_duration >= LONG_PRESS_TIME_MS) {
                    event = BUTTON_EVENT_LONG_PRESS;
                    Serial.printf("检测到长按事件，持续时间: %lu ms\n", press_duration);
                } else {
                    event = BUTTON_EVENT_SHORT_PRESS;
                    Serial.printf("检测到短按事件，持续时间: %lu ms\n", press_duration);
                }
            }
        }
    }
    // 检测长按（按钮仍然按下）
    else if (current_state == BUTTON_PRESSED && button_last_state == BUTTON_PRESSED) {
        if (!button_long_press_triggered && 
            (current_time - button_press_time >= LONG_PRESS_TIME_MS)) {
            button_long_press_triggered = true;
            event = BUTTON_EVENT_LONG_PRESS;
            Serial.println("检测到长按事件（持续按下）");
        }
    }
    
    button_last_state = current_state;
    return event;
}

// 获取按钮事件
button_event_t button_get_event(void) {
    button_event_t event = BUTTON_EVENT_NONE;
    
    if (button_event_queue) {
        xQueueReceive(button_event_queue, &event, 0);
    }
    
    return event;
}

// 按钮任务
void button_task(void* pvParameters) {
    Serial.println("按钮任务启动");
    
    // 创建按钮事件队列
    button_event_queue = xQueueCreate(5, sizeof(button_event_t));
    if (!button_event_queue) {
        Serial.println("按钮事件队列创建失败");
        vTaskDelete(NULL);
        return;
    }
    
    // 初始化按钮状态
    button_last_state = get_button_state();
    button_release_time = millis();
    
    while (1) {
        // 检测按钮事件
        button_event_t event = detect_button_event();
        
        // 如果有事件，发送到队列
        if (event != BUTTON_EVENT_NONE) {
            if (xQueueSend(button_event_queue, &event, 0) != pdTRUE) {
                Serial.println("按钮事件队列已满，丢弃事件");
            }
        }
        
        // 等待下次检测
        delay(10); // 100Hz检测频率
    }
}

// 处理按钮事件的游戏逻辑
void handle_button_event(button_event_t event) {
    if (event == BUTTON_EVENT_NONE) return;
    
    Serial.printf("处理按钮事件: %d，当前状态: %d\n", event, current_state);
    
    switch (current_state) {
        case GAME_STATE_IDLE:
            // 待机状态下，任何按键都开始游戏
            if (event == BUTTON_EVENT_SHORT_PRESS || event == BUTTON_EVENT_LONG_PRESS) {
                game_start();
            }
            break;
            
        case GAME_STATE_PLAYING:
            if (event == BUTTON_EVENT_SHORT_PRESS) {
                // 短按暂停游戏
                game_pause();
            } else if (event == BUTTON_EVENT_LONG_PRESS) {
                // 长按进入重置确认
                current_state = GAME_STATE_RESET_CONFIRM;
                play_sound_effect(SOUND_RESET_WARNING);
                Serial.println("进入重置确认状态");
            }
            break;
            
        case GAME_STATE_PAUSED:
            if (event == BUTTON_EVENT_SHORT_PRESS) {
                // 短按继续游戏
                game_resume();
            } else if (event == BUTTON_EVENT_LONG_PRESS) {
                // 长按进入重置确认
                current_state = GAME_STATE_RESET_CONFIRM;
                play_sound_effect(SOUND_RESET_WARNING);
                Serial.println("进入重置确认状态");
            }
            break;
            
        case GAME_STATE_RESET_CONFIRM:
            if (event == BUTTON_EVENT_SHORT_PRESS) {
                // 短按取消重置，返回暂停状态
                current_state = GAME_STATE_PAUSED;
                Serial.println("取消重置，返回暂停状态");
            } else if (event == BUTTON_EVENT_LONG_PRESS) {
                // 长按确认重置
                game_reset();
                Serial.println("确认重置游戏");
            }
            break;

        case GAME_STATE_LAUNCHING:
            // 火箭发射动画状态，忽略按键输入
            // 让动画自然播放完成
            Serial.println("火箭发射中，请等待动画完成");
            break;

        case GAME_STATE_RESULT:
            // 结算状态下，任何按键都返回待机
            if (event == BUTTON_EVENT_SHORT_PRESS || event == BUTTON_EVENT_LONG_PRESS) {
                current_state = GAME_STATE_IDLE;
                Serial.println("从结算状态返回待机");
            }
            break;
            
        default:
            Serial.printf("未处理的游戏状态: %d\n", current_state);
            break;
    }
}
