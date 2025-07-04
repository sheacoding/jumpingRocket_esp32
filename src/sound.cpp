#include "jumping_rocket_simple.h"

// 音效队列
QueueHandle_t sound_queue = NULL;

// 播放单个音调
void buzzer_play_tone(int frequency, int duration_ms) {
    if (frequency <= 0) {
        // 静音
        digitalWrite(BUZZER_PIN, LOW);
    } else {
        // 简单的方波生成
        int period = 1000000 / frequency; // 微秒
        int half_period = period / 2;
        int cycles = (duration_ms * 1000) / period;
        
        for (int i = 0; i < cycles; i++) {
            digitalWrite(BUZZER_PIN, HIGH);
            delayMicroseconds(half_period);
            digitalWrite(BUZZER_PIN, LOW);
            delayMicroseconds(half_period);
        }
    }
    
    digitalWrite(BUZZER_PIN, LOW);
}

// 播放音效
void play_sound_effect(sound_type_t sound_type) {
    if (sound_queue) {
        xQueueSend(sound_queue, &sound_type, 0);
    }
}

// 开机音效
static void play_boot_sound(void) {
    Serial.println("音效: 开机音效");
    buzzer_play_tone(262, 200); // C4
    buzzer_play_tone(330, 200); // E4
    buzzer_play_tone(392, 200); // G4
    buzzer_play_tone(523, 400); // C5
}

// 游戏开始音效
static void play_game_start_sound(void) {
    Serial.println("音效: 游戏开始");
    buzzer_play_tone(392, 150); // G4
    buzzer_play_tone(440, 150); // A4
    buzzer_play_tone(494, 150); // B4
    buzzer_play_tone(523, 150); // C5
    buzzer_play_tone(587, 300); // D5
}

// 跳跃音效
static void play_jump_sound(void) {
    Serial.println("音效: 跳跃");
    buzzer_play_tone(523, 100); // C5
    buzzer_play_tone(659, 150); // E5
}

// 暂停音效
static void play_pause_sound(void) {
    Serial.println("音效: 暂停");
    buzzer_play_tone(440, 200); // A4
    buzzer_play_tone(349, 300); // F4
}

// 继续音效
static void play_resume_sound(void) {
    Serial.println("音效: 继续");
    buzzer_play_tone(349, 150); // F4
    buzzer_play_tone(440, 150); // A4
    buzzer_play_tone(523, 200); // C5
}

// 重置警告音效
static void play_reset_warning_sound(void) {
    Serial.println("音效: 重置警告");
    for (int i = 0; i < 3; i++) {
        buzzer_play_tone(880, 200); // A5
        delay(100);
    }
}

// 火箭发射音效
static void play_rocket_launch_sound(void) {
    Serial.println("音效: 火箭发射");
    // 倒计时音效
    for (int i = 0; i < 3; i++) {
        buzzer_play_tone(392, 300); // G4
        delay(200);
    }
    
    // 发射音效 - 上升音调
    int frequencies[] = {262, 294, 330, 349, 392, 440, 494, 523, 587, 659};
    for (int i = 0; i < 10; i++) {
        buzzer_play_tone(frequencies[i], 100);
    }
}

// 胜利音效
static void play_victory_sound(void) {
    Serial.println("音效: 胜利");
    int frequencies[] = {523, 523, 784, 784, 880, 880, 784, 698, 698, 659, 659, 587, 587, 523};
    int durations[] = {200, 200, 200, 200, 200, 200, 400, 200, 200, 200, 200, 200, 200, 400};

    for (int i = 0; i < 14; i++) {
        buzzer_play_tone(frequencies[i], durations[i]);
        delay(50);
    }
}

// 难度选择音效
static void play_difficulty_select_sound(void) {
    Serial.println("音效: 难度选择");
    buzzer_play_tone(440, 100); // A4
    buzzer_play_tone(523, 100); // C5
}

// 难度确认音效
static void play_difficulty_confirm_sound(void) {
    Serial.println("音效: 难度确认");
    buzzer_play_tone(523, 150); // C5
    buzzer_play_tone(659, 150); // E5
    buzzer_play_tone(784, 200); // G5
}

// 音效任务
void sound_task(void* pvParameters) {
    Serial.println("音效任务启动");
    
    // 创建音效队列
    sound_queue = xQueueCreate(10, sizeof(sound_type_t));
    if (!sound_queue) {
        Serial.println("音效队列创建失败");
        vTaskDelete(NULL);
        return;
    }
    
    sound_type_t sound_type;
    
    while (1) {
        // 等待音效请求
        if (xQueueReceive(sound_queue, &sound_type, portMAX_DELAY) == pdTRUE) {
            Serial.printf("播放音效: %d\n", sound_type);
            
            switch (sound_type) {
                case SOUND_BOOT:
                    play_boot_sound();
                    break;
                    
                case SOUND_GAME_START:
                    play_game_start_sound();
                    break;
                    
                case SOUND_JUMP:
                    play_jump_sound();
                    break;
                    
                case SOUND_PAUSE:
                    play_pause_sound();
                    break;
                    
                case SOUND_RESUME:
                    play_resume_sound();
                    break;
                    
                case SOUND_RESET_WARNING:
                    play_reset_warning_sound();
                    break;
                    
                case SOUND_ROCKET_LAUNCH:
                    play_rocket_launch_sound();
                    break;
                    
                case SOUND_VICTORY:
                    play_victory_sound();
                    break;

                case SOUND_DIFFICULTY_SELECT:
                    play_difficulty_select_sound();
                    break;

                case SOUND_DIFFICULTY_CONFIRM:
                    play_difficulty_confirm_sound();
                    break;

                default:
                    Serial.printf("未知音效类型: %d\n", sound_type);
                    break;
            }
        }
    }
}
