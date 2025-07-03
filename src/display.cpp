#include "jumping_rocket_simple.h"

// U8g2显示对象 - 使用I2C接口的SSD1306
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// 动画相关变量
static uint32_t last_animation_time = 0;
static int animation_frame = 0;
static bool display_initialized = false;

// 动画状态变量
static uint32_t jump_animation_start = 0;
static bool jump_animation_active = false;
static uint32_t fuel_animation_start = 0;
static uint32_t fuel_animation_target = 0;
static uint32_t fuel_animation_current = 0;
static bool fuel_animation_active = false;
static uint32_t rocket_launch_start = 0;
static bool rocket_launch_active = false;
static game_state_t last_display_state = GAME_STATE_IDLE;

// 动画常量
#define JUMP_ANIMATION_DURATION     200   // 跳跃动画持续时间(ms)
#define FUEL_ANIMATION_DURATION     300   // 燃料动画持续时间(ms)
#define ROCKET_LAUNCH_DURATION      2000  // 火箭发射动画持续时间(ms)
#define TRANSITION_DURATION         150   // 界面切换动画持续时间(ms)

// 字体定义 - 针对128x64优化
#define FONT_TINY     u8g2_font_4x6_tf        // 4x6像素，用于小标签
#define FONT_SMALL    u8g2_font_6x10_tf       // 6x10像素，用于数值
#define FONT_MEDIUM   u8g2_font_7x13_tf       // 7x13像素，用于标题
#define FONT_LARGE    u8g2_font_10x20_tf      // 10x20像素，用于大数字
#define FONT_TITLE    u8g2_font_ncenB12_tf    // 12像素粗体，用于主标题

// SVG-to-U8g2映射标准
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

// SVG布局标准（基于设计文档精确定义）
#define STATUS_BAR_HEIGHT    12    // 顶部状态栏高度
#define HINT_BAR_HEIGHT      10    // 底部提示栏高度
#define CONTENT_AREA_Y       12    // 内容区域起始Y坐标
#define CONTENT_AREA_HEIGHT  42    // 内容区域高度(64-12-10)

// SVG坐标映射函数
#define SVG_TO_SCREEN_X(svg_x) ((int)((svg_x) * SCREEN_WIDTH / 128))
#define SVG_TO_SCREEN_Y(svg_y) ((int)((svg_y) * SCREEN_HEIGHT / 64))

// SVG字体大小映射标准
// SVG font-size="6-7px" → FONT_TINY
// SVG font-size="8px" → FONT_SMALL
// SVG font-size="10-12px" → FONT_MEDIUM
// SVG font-size="14-16px" → FONT_LARGE

// SVG动画参数转换
#define SVG_DUR_TO_MS(dur_s) ((uint32_t)((dur_s) * 1000))
#define SVG_OPACITY_TO_PATTERN(opacity) ((opacity) > 0.5f ? 1 : 0)

// 图标尺寸常量
#define ICON_SIZE_SMALL  8   // 8x8小图标
#define ICON_SIZE_LARGE  16  // 16x16大图标
#define ICON_SIZE        ICON_SIZE_SMALL  // 默认图标尺寸

// 8x8像素图标定义（基于SVG设计精确重绘）
const uint8_t icon_rocket_small[] = {
    0x18,  // 00011000 - 火箭头部尖端
    0x3C,  // 00111100 - 火箭头部
    0x3C,  // 00111100 - 火箭主体上部
    0x3C,  // 00111100 - 火箭主体
    0x3C,  // 00111100 - 火箭主体
    0x7E,  // 01111110 - 火箭底部扩展
    0xFF,  // 11111111 - 火焰效果
    0x66   // 01100110 - 火焰尾迹
};

const uint8_t icon_pause[] = {
    0x00,  // 00000000
    0x66,  // 01100110 - 双竖条暂停图标
    0x66,  // 01100110
    0x66,  // 01100110
    0x66,  // 01100110
    0x66,  // 01100110
    0x66,  // 01100110
    0x00   // 00000000
};

const uint8_t icon_warning[] = {
    0x18,  // 00011000 - 三角形顶点
    0x3C,  // 00111100
    0x3C,  // 00111100
    0x66,  // 01100110
    0x66,  // 01100110
    0x7E,  // 01111110 - 三角形底边
    0x18,  // 00011000 - 感叹号
    0x00   // 00000000
};

const uint8_t icon_fuel[] = {
    0x3C,  // 00111100 - 燃料罐顶部
    0x7E,  // 01111110 - 燃料罐主体
    0x7E,  // 01111110
    0x7E,  // 01111110
    0x7E,  // 01111110
    0x7E,  // 01111110
    0x7E,  // 01111110
    0x3C   // 00111100 - 燃料罐底部
};

const uint8_t icon_trophy[] = {
    0x3C,  // 00111100 - 奖杯顶部
    0x7E,  // 01111110 - 奖杯杯身
    0x7E,  // 01111110
    0x3C,  // 00111100 - 奖杯收腰
    0x18,  // 00011000 - 奖杯柄
    0x18,  // 00011000
    0x3C,  // 00111100 - 奖杯底座
    0x00   // 00000000
};

// 16x16像素火箭图标定义（基于SVG设计精确重绘）
const uint8_t icon_rocket_large[] = {
    // 第1-4行：尖锐三角形头部（基于SVG polygon points="0,-10 4,-2 -4,-2"）
    0x01, 0x80,  // 00000001 10000000 - 火箭尖端
    0x03, 0xC0,  // 00000011 11000000 - 头部扩展
    0x07, 0xE0,  // 00000111 11100000 - 头部底部
    0x0F, 0xF0,  // 00001111 11110000 - 头部与主体连接

    // 第5-12行：矩形主体部分（基于SVG rect x="-2" y="-2" width="4" height="8"）
    0x0F, 0xF0,  // 00001111 11110000 - 主体
    0x0F, 0xF0,  // 00001111 11110000 - 主体
    0x0F, 0xF0,  // 00001111 11110000 - 主体（添加窗口）
    0x09, 0x90,  // 00001001 10010000 - 主体窗口
    0x09, 0x90,  // 00001001 10010000 - 主体窗口
    0x0F, 0xF0,  // 00001111 11110000 - 主体
    0x0F, 0xF0,  // 00001111 11110000 - 主体
    0x0F, 0xF0,  // 00001111 11110000 - 主体底部

    // 第13-16行：扩散火焰（基于SVG polygon points="-5,6 -2,10 2,10 5,6"）
    0x1F, 0xF8,  // 00011111 11111000 - 火焰开始
    0x3F, 0xFC,  // 00111111 11111100 - 火焰扩散
    0x7F, 0xFE,  // 01111111 11111110 - 火焰最大
    0xFF, 0xFF   // 11111111 11111111 - 火焰尾迹
};

// OLED初始化
bool oled_init(void) {
    Serial.println("🖥️  开始U8g2 OLED初始化...");

    // 检查I2C连接
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.beginTransmission(OLED_ADDRESS);
    uint8_t error = Wire.endTransmission();

    if (error != 0) {
        Serial.printf("❌ OLED I2C连接失败，错误代码: %d\n", error);
        Serial.printf("   请检查连接: SDA->GPIO%d, SCL->GPIO%d\n", I2C_SDA_PIN, I2C_SCL_PIN);
        return false;
    }
    Serial.println("✅ OLED I2C连接成功");

    // 初始化U8g2库
    Serial.println("   正在初始化U8g2库...");
    if (!u8g2.begin()) {
        Serial.println("❌ U8g2库初始化失败");
        return false;
    }
    Serial.println("✅ U8g2库初始化成功");

    // 设置字体和显示参数
    u8g2.setFont(FONT_MEDIUM);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    Serial.println("✅ 字体和显示参数设置完成");

    // 清屏测试
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    Serial.println("✅ 清屏测试完成");

    // 显示测试文本
    Serial.println("   正在显示测试文本...");
    u8g2.clearBuffer();
    u8g2.setFont(FONT_MEDIUM);
    u8g2.drawStr(10, 10, "OLED Test OK");
    u8g2.drawStr(10, 30, "Initializing...");
    u8g2.sendBuffer();
    delay(2000); // 显示2秒

    display_initialized = true;
    Serial.println("🎉 U8g2 OLED初始化完全成功");

    return true;
}

// 清屏
void oled_clear(void) {
    if (!display_initialized) return;
    u8g2.clearBuffer();
}

// 显示文本
void oled_display_text(int x, int y, const char* text) {
    if (!display_initialized || !text) return;
    u8g2.drawStr(x, y, text);
}

// 绘制8x8图标
void draw_icon(int x, int y, const uint8_t* icon_data) {
    if (!display_initialized || !icon_data) return;

    for (int row = 0; row < 8; row++) {
        uint8_t line = icon_data[row];
        for (int col = 0; col < 8; col++) {
            if (line & (0x80 >> col)) {
                u8g2.drawPixel(x + col, y + row);
            }
        }
    }
}

// 绘制16x16大图标
void draw_large_icon(int x, int y, const uint8_t* icon_data) {
    if (!display_initialized || !icon_data) return;

    for (int row = 0; row < 16; row++) {
        // 每行2个字节
        uint8_t high_byte = icon_data[row * 2];
        uint8_t low_byte = icon_data[row * 2 + 1];

        // 绘制高字节（左8位）
        for (int col = 0; col < 8; col++) {
            if (high_byte & (0x80 >> col)) {
                u8g2.drawPixel(x + col, y + row);
            }
        }

        // 绘制低字节（右8位）
        for (int col = 0; col < 8; col++) {
            if (low_byte & (0x80 >> col)) {
                u8g2.drawPixel(x + 8 + col, y + row);
            }
        }
    }
}

// 显示进度条（带平滑动画）
void oled_display_progress_bar(int x, int y, int width, int height, int progress) {
    if (!display_initialized) return;

    // 限制进度值范围
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;

    // 检查是否需要启动燃料动画
    static int last_progress = 0;
    if (progress != last_progress && !fuel_animation_active) {
        start_fuel_animation(progress);
        last_progress = progress;
    }

    // 计算当前显示的进度（带动画）
    int display_progress = progress;
    if (fuel_animation_active) {
        uint32_t current_time = millis();
        uint32_t elapsed = current_time - fuel_animation_start;

        if (elapsed >= FUEL_ANIMATION_DURATION) {
            // 动画结束
            fuel_animation_active = false;
            display_progress = fuel_animation_target;
        } else {
            // 计算动画进度
            float t = (float)elapsed / FUEL_ANIMATION_DURATION;
            t = ease_out(t); // 应用缓动函数

            display_progress = fuel_animation_current +
                              (fuel_animation_target - fuel_animation_current) * t;
        }
    }

    // 绘制进度条边框
    u8g2.drawFrame(x, y, width, height);

    // 绘制进度条填充
    int fill_width = (width - 2) * display_progress / 100;
    if (fill_width > 0) {
        u8g2.drawBox(x + 1, y + 1, fill_width, height - 2);
    }
}

// 绘制带图标的标签（修复重叠问题）
void draw_labeled_value(int x, int y, const uint8_t* icon, const char* label, const char* value) {
    if (!display_initialized) return;

    int current_x = x;

    // 绘制图标
    if (icon) {
        draw_icon(current_x, y, icon);
        current_x += ICON_SIZE + 3; // 图标宽度 + 3像素间距
    }

    // 绘制标签
    u8g2.setFont(FONT_TINY);
    u8g2.drawStr(current_x, y, label);

    // 绘制数值（在标签下方）
    u8g2.setFont(FONT_SMALL);
    u8g2.drawStr(current_x, y + 10, value); // 增加垂直间距
}

// 绘制水平排列的图标和文字
void draw_horizontal_icon_text(int x, int y, const uint8_t* icon, const char* text) {
    if (!display_initialized) return;

    int current_x = x;

    // 绘制图标
    if (icon) {
        draw_icon(current_x, y, icon);
        current_x += ICON_SIZE + 3; // 图标宽度 + 3像素间距
    }

    // 绘制文字
    u8g2.setFont(FONT_TINY);
    u8g2.drawStr(current_x, y + 2, text); // 垂直居中对齐
}

// SVG坐标映射函数
int svg_transform_x(int svg_x, int svg_translate_x) {
    return SVG_TO_SCREEN_X(svg_translate_x + svg_x);
}

int svg_transform_y(int svg_y, int svg_translate_y) {
    return SVG_TO_SCREEN_Y(svg_translate_y + svg_y);
}

// SVG动画时间控制
uint32_t svg_animate_progress(uint32_t start_time, uint32_t duration_ms) {
    uint32_t elapsed = millis() - start_time;
    return elapsed % duration_ms;
}

// SVG透明度模拟（使用点阵密度）
bool svg_opacity_visible(float opacity, uint32_t time_offset) {
    if (opacity >= 1.0f) return true;
    if (opacity <= 0.0f) return false;

    // 使用时间偏移创建透明度效果
    uint32_t pattern = (millis() + time_offset) / 100;
    return (pattern % 10) < (opacity * 10);
}

// 缓动函数（ease-out）
float ease_out(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

// 启动跳跃动画
void start_jump_animation(void) {
    jump_animation_start = millis();
    jump_animation_active = true;
}

// 启动燃料进度动画
void start_fuel_animation(uint32_t target_fuel) {
    if (!fuel_animation_active) {
        fuel_animation_start = millis();
        fuel_animation_current = game_data.fuel_progress;
        fuel_animation_target = target_fuel;
        fuel_animation_active = true;
    }
}

// 启动火箭发射动画
void start_rocket_launch_animation(void) {
    rocket_launch_start = millis();
    rocket_launch_active = true;
}

// 开机动画（基于SVG设计精确重构）
void oled_display_boot_animation(void) {
    if (!display_initialized) return;

    uint32_t current_time = millis();

    // 每100ms更新一帧（提高动画流畅度）
    if (current_time - last_animation_time >= 100) {
        u8g2.clearBuffer();

        // 火箭图标（基于SVG transform="translate(64, 32)"精确定位）
        int rocket_x = svg_transform_x(-8, 64);  // SVG中心点64，图标半宽8
        int rocket_y = svg_transform_y(-8, 32);  // SVG中心点32，图标半高8
        draw_large_icon(rocket_x, rocket_y, icon_rocket_large);

        // "ROCKET"文字（基于SVG text x="64" y="48" font-size="8"）
        uint32_t text_cycle = svg_animate_progress(current_time, SVG_DUR_TO_MS(1.0)); // 1秒周期
        float text_opacity = 0.5f + 0.5f * sin(text_cycle * 2 * PI / 1000.0f);

        if (svg_opacity_visible(text_opacity, 0)) {
            u8g2.setFont(FONT_SMALL); // 对应SVG font-size="8"
            const char* rocket_text = "ROCKET";
            int text_width = u8g2.getStrWidth(rocket_text);
            int text_x = svg_transform_x(-text_width/2, 64); // 居中对齐
            int text_y = svg_transform_y(0, 48);
            u8g2.drawStr(text_x, text_y, rocket_text);
        }

        // 三个动画指示点（基于SVG circle cx="40,50,60" cy="55" r="2"）
        uint32_t dot_cycle = svg_animate_progress(current_time, SVG_DUR_TO_MS(1.5)); // 1.5秒周期

        // 第一个点 (SVG cx="40")
        int dot1_x = svg_transform_x(0, 40);
        int dot1_y = svg_transform_y(0, 55);
        uint32_t dot1_phase = dot_cycle;
        if ((dot1_phase / 500) % 3 == 0) {
            u8g2.drawDisc(dot1_x, dot1_y, 2);
        } else {
            u8g2.drawCircle(dot1_x, dot1_y, 2);
        }

        // 第二个点 (SVG cx="50"，延迟0.5秒)
        int dot2_x = svg_transform_x(0, 50);
        int dot2_y = svg_transform_y(0, 55);
        uint32_t dot2_phase = (dot_cycle + 500) % SVG_DUR_TO_MS(1.5);
        if ((dot2_phase / 500) % 3 == 0) {
            u8g2.drawDisc(dot2_x, dot2_y, 2);
        } else {
            u8g2.drawCircle(dot2_x, dot2_y, 2);
        }

        // 第三个点 (SVG cx="60"，延迟1.0秒)
        int dot3_x = svg_transform_x(0, 60);
        int dot3_y = svg_transform_y(0, 55);
        uint32_t dot3_phase = (dot_cycle + 1000) % SVG_DUR_TO_MS(1.5);
        if ((dot3_phase / 500) % 3 == 0) {
            u8g2.drawDisc(dot3_x, dot3_y, 2);
        } else {
            u8g2.drawCircle(dot3_x, dot3_y, 2);
        }

        u8g2.sendBuffer();
        last_animation_time = current_time;
    }
}

// 火箭发射动画（完整版）
void oled_display_rocket_launch_animation(void) {
    if (!display_initialized) return;

    uint32_t current_time = millis();

    // 启动发射动画（如果还没有启动）
    if (!rocket_launch_active) {
        start_rocket_launch_animation();
    }

    // 每50ms更新一帧（20FPS，更流畅）
    if (current_time - last_animation_time >= 50) {
        u8g2.clearBuffer();

        uint32_t elapsed = current_time - rocket_launch_start;
        float progress = (float)elapsed / ROCKET_LAUNCH_DURATION;

        if (progress >= 1.0f) {
            // 动画结束，计算并保存最终高度
            uint32_t base_height = 100;
            uint32_t height_from_jumps = game_data.jump_count * 50;
            uint32_t height_from_time = (game_data.game_time_ms / 1000) * 10;
            float final_multiplier = 10.0f; // 最终高度倍数

            game_data.flight_height = (uint32_t)((base_height + height_from_jumps + height_from_time) * final_multiplier);

            // 燃料满格奖励
            if (game_data.fuel_progress >= 100) {
                game_data.flight_height += (uint32_t)(500 * final_multiplier);
            }

            Serial.printf("🚀 火箭发射完成，最终高度: %lu米\n", game_data.flight_height);

            // 动画结束，重置
            rocket_launch_active = false;
            animation_frame = 0;
            return;
        }

        // 三个阶段的动画
        int rocket_x = (SCREEN_WIDTH - 16) / 2;
        int rocket_y;

        if (progress < 0.3f) {
            // 阶段1：起飞（0-30%）
            float stage_progress = progress / 0.3f;
            rocket_y = 55 - (int)(10 * stage_progress);
        } else if (progress < 0.8f) {
            // 阶段2：加速（30-80%）
            float stage_progress = (progress - 0.3f) / 0.5f;
            rocket_y = 45 - (int)(35 * stage_progress * stage_progress); // 加速曲线
        } else {
            // 阶段3：消失（80-100%）
            float stage_progress = (progress - 0.8f) / 0.2f;
            rocket_y = 10 - (int)(20 * stage_progress);
        }

        // 绘制背景星星
        for (int i = 0; i < 8; i++) {
            int star_x = (i * 17 + (int)(current_time / 100)) % SCREEN_WIDTH;
            int star_y = (i * 13 + (int)(current_time / 150)) % SCREEN_HEIGHT;
            u8g2.drawPixel(star_x, star_y);
        }

        // 绘制火箭（如果在屏幕内）
        if (rocket_y > -16 && rocket_y < SCREEN_HEIGHT) {
            draw_large_icon(rocket_x, rocket_y, icon_rocket_large);

            // 火焰尾迹效果
            if (progress < 0.9f) {
                int flame_length = 8 + (int)(progress * 12);
                for (int i = 0; i < flame_length; i += 2) {
                    if (rocket_y + 16 + i < SCREEN_HEIGHT) {
                        u8g2.drawPixel(rocket_x + 6, rocket_y + 16 + i);
                        u8g2.drawPixel(rocket_x + 8, rocket_y + 16 + i + 1);
                        u8g2.drawPixel(rocket_x + 10, rocket_y + 16 + i);
                    }
                }
            }
        }

        // 计算动态飞行高度（随火箭升空快速变化）
        uint32_t base_height = 100; // 基础高度
        uint32_t height_from_jumps = game_data.jump_count * 50; // 每次跳跃50米
        uint32_t height_from_time = (game_data.game_time_ms / 1000) * 10; // 每秒10米

        // 动画进度影响高度（火箭越高，显示高度越大）
        float height_multiplier = 1.0f + (progress * 9.0f); // 1.0x到10.0x的变化
        uint32_t dynamic_height = (uint32_t)((base_height + height_from_jumps + height_from_time) * height_multiplier);

        // 燃料满格奖励
        if (game_data.fuel_progress >= 100) {
            dynamic_height += (uint32_t)(500 * height_multiplier); // 奖励也随高度放大
        }

        // 显示动态飞行高度（右侧显示，避免与火箭重叠）
        u8g2.setFont(FONT_LARGE);
        char height_text[16];
        snprintf(height_text, sizeof(height_text), "%lum", dynamic_height);
        int height_x = SCREEN_WIDTH - u8g2.getStrWidth(height_text) - 2; // 右对齐，留2像素边距
        int height_y = 15; // 顶部位置
        u8g2.drawStr(height_x, height_y, height_text);

        // 显示"ALTITUDE"标签（右侧）
        u8g2.setFont(FONT_TINY);
        const char* alt_label = "ALTITUDE";
        int label_x = SCREEN_WIDTH - u8g2.getStrWidth(alt_label) - 2;
        int label_y = 25;
        u8g2.drawStr(label_x, label_y, alt_label);

        // 显示统计信息（左侧，避免与高度重叠）
        u8g2.setFont(FONT_TINY);
        char jump_text[16];
        snprintf(jump_text, sizeof(jump_text), "JUMPS: %lu", game_data.jump_count);
        u8g2.drawStr(2, 15, jump_text);

        char time_text[16];
        uint32_t total_seconds = game_data.game_time_ms / 1000;
        uint32_t minutes = total_seconds / 60;
        uint32_t seconds = total_seconds % 60;
        snprintf(time_text, sizeof(time_text), "TIME: %02lu:%02lu", minutes, seconds);
        u8g2.drawStr(2, 25, time_text);

        u8g2.sendBuffer();

        animation_frame++;
        last_animation_time = current_time;
    }
}

// 游戏界面显示（基于SVG设计精确重构）
void oled_display_game_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // 顶部状态栏（上移到屏幕顶部边缘，避免双色分界线）

    // 时间显示（移到屏幕顶部）
    u8g2.setFont(FONT_SMALL); // 从FONT_TINY升级到FONT_SMALL（增大50%）
    uint32_t total_seconds = game_data.game_time_ms / 1000;
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;
    char time_text[16];
    snprintf(time_text, sizeof(time_text), "%02lu:%02lu", minutes, seconds);
    int time_x = 2;  // 靠近左边缘
    int time_y = 8;  // 上移到屏幕顶部（字体高度约8像素）
    u8g2.drawStr(time_x, time_y, time_text);

    // 跳跃计数（移到屏幕顶部）
    u8g2.setFont(FONT_SMALL); // 从FONT_TINY升级到FONT_SMALL（增大50%）
    char count_text[16];
    snprintf(count_text, sizeof(count_text), "%lu", game_data.jump_count);

    // 计算跳跃次数的宽度，右对齐显示
    int count_width = u8g2.getStrWidth(count_text);
    int count_x = SCREEN_WIDTH - count_width - 2; // 靠近右边缘，留2像素边距
    int count_y = 8;  // 与时间对齐在屏幕顶部
    u8g2.drawStr(count_x, count_y, count_text);

    // 移除状态栏分割线

    // 火箭图标（确保始终显示，增强可见性）
    int rocket_x = svg_transform_x(-4, 20); // 8x8图标居中
    int rocket_y = svg_transform_y(-4, 32);

    // 绘制火箭图标，确保可见
    draw_icon(rocket_x, rocket_y, icon_rocket_small);

    // 火箭图标应该在坐标(16, 28)附近显示

    // 跳跃反馈波纹动画（只在检测到跳跃时触发一次）
    if (game_data.is_jumping && !jump_animation_active) {
        start_jump_animation();
        // 重置跳跃状态，避免动画一直触发
        game_data.is_jumping = false;
    }

    if (jump_animation_active) {
        uint32_t current_time = millis();
        uint32_t elapsed = current_time - jump_animation_start;

        if (elapsed >= JUMP_ANIMATION_DURATION) {
            jump_animation_active = false;
        } else {
            float t = (float)elapsed / JUMP_ANIMATION_DURATION;

            // 第一层波纹（基于SVG animate r="5;15" dur="0.5s"）
            int center_x = svg_transform_x(0, 20);
            int center_y = svg_transform_y(0, 32);
            int radius1 = 5 + (int)(10 * t); // 5-15像素

            // 绘制第一层波纹
            for (int i = 0; i < 12; i++) {
                float angle = (i * 30.0f) * PI / 180.0f;
                int x = center_x + (int)(radius1 * cos(angle));
                int y = center_y + (int)(radius1 * sin(angle));
                if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
                    u8g2.drawPixel(x, y);
                }
            }

            // 第二层波纹（基于SVG animate r="8;20" dur="0.5s" begin="0.1s"）
            if (elapsed >= 100) { // 延迟0.1秒
                float t2 = (float)(elapsed - 100) / (JUMP_ANIMATION_DURATION - 100);
                int radius2 = 8 + (int)(12 * t2); // 8-20像素

                // 绘制第二层波纹
                for (int i = 0; i < 8; i++) {
                    float angle = (i * 45.0f) * PI / 180.0f;
                    int x = center_x + (int)(radius2 * cos(angle));
                    int y = center_y + (int)(radius2 * sin(angle));
                    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
                        u8g2.drawPixel(x, y);
                    }
                }
            }
        }
    }

    // 燃料进度条区域（重新布局避免重叠）
    u8g2.setFont(FONT_TINY);

    // FUEL标签（左侧）
    int fuel_label_x = svg_transform_x(0, 35);
    int fuel_label_y = svg_transform_y(0, 20);
    u8g2.drawStr(fuel_label_x, fuel_label_y, "FUEL");

    // 百分比显示（右侧，与标签同行）
    char fuel_text[8];
    snprintf(fuel_text, sizeof(fuel_text), "%lu%%", game_data.fuel_progress);
    int fuel_pct_x = svg_transform_x(0, 100);  // 右侧显示
    int fuel_pct_y = svg_transform_y(0, 20);   // 与标签同行
    u8g2.drawStr(fuel_pct_x, fuel_pct_y, fuel_text);

    // 进度条（使用固定像素值，确保计算准确）
    int bar_x = 35;                           // 固定X坐标
    int bar_y = 28;                           // 固定Y坐标
    int bar_width = 80;                       // 固定宽度80像素
    int bar_height = 6;                       // 固定高度6像素
    u8g2.drawFrame(bar_x, bar_y, bar_width, bar_height);

    // 进度条填充（确保100%时完全填满）
    int available_width = bar_width - 2;     // 78像素可用宽度
    int fill_width = (available_width * game_data.fuel_progress) / 100;

    // 确保100%时完全填满
    if (game_data.fuel_progress >= 100) {
        fill_width = available_width;
    }

    // 调试输出进度条计算
    static uint32_t last_debug_fuel = 999;
    if (game_data.fuel_progress != last_debug_fuel) {
        Serial.printf("📊 进度条: 燃料=%lu%%, 可用宽度=%d, 填充宽度=%d\n",
                     game_data.fuel_progress, available_width, fill_width);
        last_debug_fuel = game_data.fuel_progress;
    }

    if (fill_width > 0) {
        u8g2.drawBox(bar_x + 1, bar_y + 1, fill_width, bar_height - 2);
    }

    // 底部提示（基于SVG两个text元素）
    u8g2.setFont(FONT_TINY);
    int hint1_x = svg_transform_x(0, 5);
    int hint1_y = svg_transform_y(0, 55);
    u8g2.drawStr(hint1_x, hint1_y, "Press: Pause");

    int hint2_x = svg_transform_x(0, 75);
    int hint2_y = svg_transform_y(0, 55);
    u8g2.drawStr(hint2_x, hint2_y, "Hold: Reset");

    u8g2.sendBuffer();
}

// 暂停界面显示（基于SVG设计精确重构）
void oled_display_pause_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // 闪烁边框效果（基于SVG animate opacity="0.3;1;0.3" dur="1s"）
    uint32_t border_cycle = svg_animate_progress(millis(), SVG_DUR_TO_MS(1.0)); // 1秒周期
    float border_t = border_cycle / 1000.0f;
    float border_opacity = 0.3f + 0.7f * (0.5f + 0.5f * sin(border_t * 2 * PI)); // 0.3-1.0变化

    // 使用透明度控制边框显示
    if (svg_opacity_visible(border_opacity, 0)) {
        // 绘制双重边框（基于SVG rect stroke-width="2"）
        int border_margin = svg_transform_x(2, 0);
        u8g2.drawFrame(border_margin, border_margin,
                      SCREEN_WIDTH - 2*border_margin, SCREEN_HEIGHT - 2*border_margin);
        u8g2.drawFrame(border_margin+1, border_margin+1,
                      SCREEN_WIDTH - 2*(border_margin+1), SCREEN_HEIGHT - 2*(border_margin+1));
    }

    // 暂停图标（基于SVG transform="translate(64, 18)"）
    int pause_icon_x = svg_transform_x(-4, 64);
    int pause_icon_y = svg_transform_y(-4, 18);
    draw_icon(pause_icon_x, pause_icon_y, icon_pause);

    // "PAUSED"文字（基于SVG text x="64" y="30" font-size="10"）
    u8g2.setFont(FONT_MEDIUM); // 对应SVG font-size="10"
    const char* title = "PAUSED";
    int title_width = u8g2.getStrWidth(title);
    int title_x = svg_transform_x(-title_width/2, 64);
    int title_y = svg_transform_y(0, 30);
    u8g2.drawStr(title_x, title_y, title);

    // 统计信息三列布局（基于SVG g元素的transform属性）
    u8g2.setFont(FONT_SMALL); // 对应SVG font-size="8"

    // 第一列：跳跃次数（基于SVG text x="20" y="42"）
    char jump_text[16];
    snprintf(jump_text, sizeof(jump_text), "%lu", game_data.jump_count);
    int jump_x = svg_transform_x(0, 20);
    int jump_y = svg_transform_y(0, 42);
    u8g2.drawStr(jump_x, jump_y, jump_text);

    u8g2.setFont(FONT_TINY); // 对应SVG font-size="6"
    int jump_label_y = svg_transform_y(0, 50);
    u8g2.drawStr(jump_x, jump_label_y, "JUMPS");

    // 第二列：游戏时长（基于SVG text x="55" y="42"）
    uint32_t total_seconds = game_data.game_time_ms / 1000;
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;

    u8g2.setFont(FONT_SMALL);
    char time_text[16];
    snprintf(time_text, sizeof(time_text), "%02lu:%02lu", minutes, seconds);
    int time_x = svg_transform_x(0, 55);
    int time_y = svg_transform_y(0, 42);
    u8g2.drawStr(time_x, time_y, time_text);

    u8g2.setFont(FONT_TINY);
    int time_label_y = svg_transform_y(0, 50);
    u8g2.drawStr(time_x, time_label_y, "TIME");

    // 第三列：燃料进度（基于SVG text x="95" y="42"）
    u8g2.setFont(FONT_SMALL);
    char fuel_text[8];
    snprintf(fuel_text, sizeof(fuel_text), "%lu%%", game_data.fuel_progress);
    int fuel_x = svg_transform_x(0, 95);
    int fuel_y = svg_transform_y(0, 42);
    u8g2.drawStr(fuel_x, fuel_y, fuel_text);

    u8g2.setFont(FONT_TINY);
    int fuel_label_y = svg_transform_y(0, 50);
    u8g2.drawStr(fuel_x, fuel_label_y, "FUEL");

    // 底部操作提示（基于SVG text x="64" y="58"）
    u8g2.setFont(FONT_TINY);
    const char* hint = "Short:Resume  Long:Reset";
    int hint_width = u8g2.getStrWidth(hint);
    int hint_x = svg_transform_x(-hint_width/2, 64);
    int hint_y = svg_transform_y(0, 58);
    u8g2.drawStr(hint_x, hint_y, hint);

    u8g2.sendBuffer();
}

// 重置确认界面（基于SVG设计精确重构）
void oled_display_reset_confirm_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // 警告边框闪烁效果（基于SVG rect stroke-dasharray animate）
    uint32_t border_cycle = svg_animate_progress(millis(), SVG_DUR_TO_MS(0.6)); // 0.6秒周期
    float border_t = border_cycle / 600.0f;
    float border_opacity = 0.3f + 0.7f * (0.5f + 0.5f * sin(border_t * 4 * PI)); // 快速闪烁

    if (svg_opacity_visible(border_opacity, 0)) {
        // 绘制警告边框（基于SVG rect x="8" y="8" width="112" height="48"）
        int border_x = svg_transform_x(0, 8);
        int border_y = svg_transform_y(0, 8);
        int border_w = svg_transform_x(112, 0);
        int border_h = svg_transform_y(48, 0);

        u8g2.drawFrame(border_x, border_y, border_w, border_h);
        u8g2.drawFrame(border_x+1, border_y+1, border_w-2, border_h-2);
    }

    // 警告图标（基于SVG transform="translate(64, 20)"）
    int warning_x = svg_transform_x(-4, 64);
    int warning_y = svg_transform_y(-4, 20);
    draw_icon(warning_x, warning_y, icon_warning);

    // "RESET?"文字（基于SVG text x="64" y="32" font-size="10"）
    u8g2.setFont(FONT_MEDIUM); // 对应SVG font-size="10"
    const char* title = "RESET?";
    int title_width = u8g2.getStrWidth(title);
    int title_x = svg_transform_x(-title_width/2, 64);
    int title_y = svg_transform_y(0, 32);
    u8g2.drawStr(title_x, title_y, title);

    // 闪烁警告文字（基于SVG animate opacity="0;1;0" dur="0.8s"）
    uint32_t text_cycle = svg_animate_progress(millis(), SVG_DUR_TO_MS(0.8)); // 0.8秒周期
    float text_t = text_cycle / 800.0f;
    float text_opacity = 0.5f + 0.5f * sin(text_t * 2 * PI);

    if (svg_opacity_visible(text_opacity, 100)) {
        u8g2.setFont(FONT_TINY); // 对应SVG font-size="6"
        const char* warning = "All progress lost!";
        int warning_width = u8g2.getStrWidth(warning);
        int warning_x = svg_transform_x(-warning_width/2, 64);
        int warning_y = svg_transform_y(0, 42);
        u8g2.drawStr(warning_x, warning_y, warning);
    }

    // 操作说明（基于SVG两个text元素）
    u8g2.setFont(FONT_TINY);

    // 确认操作（基于SVG text x="64" y="50"）
    const char* confirm = "Hold: Confirm";
    int confirm_width = u8g2.getStrWidth(confirm);
    int confirm_x = svg_transform_x(-confirm_width/2, 64);
    int confirm_y = svg_transform_y(0, 50);
    u8g2.drawStr(confirm_x, confirm_y, confirm);

    // 取消操作（基于SVG text x="64" y="58"）
    const char* cancel = "Press: Cancel";
    int cancel_width = u8g2.getStrWidth(cancel);
    int cancel_x = svg_transform_x(-cancel_width/2, 64);
    int cancel_y = svg_transform_y(0, 58);
    u8g2.drawStr(cancel_x, cancel_y, cancel);

    u8g2.sendBuffer();
}

// 结算界面显示（重新布局，避免重叠，图标移到两侧）
void oled_display_result_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // 左侧奖杯图标（移到左上角空白位置）
    int trophy_x = 8;   // 左侧位置
    int trophy_y = 8;   // 顶部位置
    draw_icon(trophy_x, trophy_y, icon_trophy);

    // 右侧火箭图标（移到右上角空白位置）
    int rocket_x = SCREEN_WIDTH - 16;  // 右侧位置
    int rocket_y = 8;                  // 顶部位置
    draw_icon(rocket_x, rocket_y, icon_rocket_small);

    // "COMPLETE!"文字（居中，但上移避免重叠）
    u8g2.setFont(FONT_MEDIUM);
    const char* title = "COMPLETE!";
    int title_width = u8g2.getStrWidth(title);
    int title_x = (SCREEN_WIDTH - title_width) / 2;
    int title_y = 12;  // 上移到顶部
    u8g2.drawStr(title_x, title_y, title);

    // 飞行高度（居中显示，大字体突出）
    u8g2.setFont(FONT_LARGE);
    char height_text[16];
    snprintf(height_text, sizeof(height_text), "%lum", game_data.flight_height);
    int height_width = u8g2.getStrWidth(height_text);
    int height_x = (SCREEN_WIDTH - height_width) / 2;
    int height_y = 28;  // 中央位置
    u8g2.drawStr(height_x, height_y, height_text);

    // 高度标签（居中，在高度下方）
    u8g2.setFont(FONT_TINY);
    const char* height_label = "ALTITUDE";
    int label_width = u8g2.getStrWidth(height_label);
    int label_x = (SCREEN_WIDTH - label_width) / 2;
    int label_y = 38;  // 高度下方
    u8g2.drawStr(label_x, label_y, height_label);

    // 统计信息重新布局（三列分布，避免重叠）
    u8g2.setFont(FONT_TINY);

    // 第一列：跳跃次数（左侧）
    char jump_text[16];
    snprintf(jump_text, sizeof(jump_text), "%lu", game_data.jump_count);
    int jump_x = 8;   // 左侧对齐
    int jump_y = 50;  // 下移避免与高度标签重叠
    u8g2.drawStr(jump_x, jump_y, jump_text);
    u8g2.drawStr(jump_x, 58, "JUMPS");  // 标签在下方

    // 第二列：游戏时长（中央）
    uint32_t total_seconds = game_data.game_time_ms / 1000;
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;
    char time_text[16];
    snprintf(time_text, sizeof(time_text), "%02lu:%02lu", minutes, seconds);
    int time_width = u8g2.getStrWidth(time_text);
    int time_x = (SCREEN_WIDTH - time_width) / 2;  // 居中
    int time_y = 50;
    u8g2.drawStr(time_x, time_y, time_text);

    // TIME标签居中
    const char* time_label = "TIME";
    int time_label_width = u8g2.getStrWidth(time_label);
    int time_label_x = (SCREEN_WIDTH - time_label_width) / 2;
    u8g2.drawStr(time_label_x, 58, time_label);

    // 第三列：燃料使用（右侧）
    char fuel_text[8];
    snprintf(fuel_text, sizeof(fuel_text), "%lu%%", game_data.fuel_progress);
    int fuel_width = u8g2.getStrWidth(fuel_text);
    int fuel_x = SCREEN_WIDTH - fuel_width - 8;  // 右侧对齐，留8像素边距
    int fuel_y = 50;
    u8g2.drawStr(fuel_x, fuel_y, fuel_text);

    // FUEL标签右对齐
    const char* fuel_label = "FUEL";
    int fuel_label_width = u8g2.getStrWidth(fuel_label);
    int fuel_label_x = SCREEN_WIDTH - fuel_label_width - 8;
    u8g2.drawStr(fuel_label_x, 58, fuel_label);

    u8g2.sendBuffer();
}

// 待机界面显示（基于SVG设计精确重构）
void oled_display_idle_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // 顶部状态栏（基于SVG rect height="12"）
    u8g2.setFont(FONT_TINY); // 对应SVG font-size="6"
    int status_text_x = svg_transform_x(0, 5);
    int status_text_y = svg_transform_y(0, 9);
    u8g2.drawStr(status_text_x, status_text_y, "READY");

    // 移除状态栏分割线

    // 中央火箭图标（基于SVG transform="translate(64, 30)"精确定位）
    int rocket_x = svg_transform_x(-8, 64);  // 图标中心对齐
    int rocket_y = svg_transform_y(-8, 30);
    draw_large_icon(rocket_x, rocket_y, icon_rocket_large);

    // 提示文字（基于SVG text x="64" y="45" font-size="8"）
    u8g2.setFont(FONT_SMALL); // 对应SVG font-size="8"
    const char* hint = "Jump to Start";
    int hint_width = u8g2.getStrWidth(hint);
    int hint_x = svg_transform_x(-hint_width/2, 64);
    int hint_y = svg_transform_y(0, 45);
    u8g2.drawStr(hint_x, hint_y, hint);

    // 呼吸灯效果（基于SVG animate r="15;20;15" opacity="0.3;0.1;0.3" dur="2s"）
    uint32_t breath_cycle = svg_animate_progress(millis(), SVG_DUR_TO_MS(2.0)); // 2秒周期
    float t = breath_cycle / 2000.0f;

    // SVG关键帧：r="15;20;15" 在 t=0,0.5,1.0
    float radius_progress = sin(t * PI); // 0->1->0 的正弦曲线
    int radius = 15 + (int)(5 * radius_progress); // 15-20像素变化

    // SVG关键帧：opacity="0.3;0.1;0.3"
    float opacity_progress = 0.5f * (1.0f + cos(t * 2 * PI)); // 0->1->0 的余弦曲线
    float opacity = 0.1f + 0.2f * opacity_progress; // 0.1-0.3变化

    // 绘制呼吸灯圆环（基于SVG circle cx="64" cy="30"）
    int center_x = svg_transform_x(0, 64);
    int center_y = svg_transform_y(0, 30);

    // 使用点阵密度模拟透明度
    int point_density = (int)(opacity * 24); // 0.1->0.3 映射到 2->7个点
    for (int i = 0; i < point_density; i++) {
        float angle = (i * 360.0f / point_density) * PI / 180.0f;
        int x = center_x + (int)(radius * cos(angle));
        int y = center_y + (int)(radius * sin(angle));

        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            u8g2.drawPixel(x, y);
            // 添加内圈点增强效果
            if (radius > 16) {
                int inner_x = center_x + (int)((radius-2) * cos(angle));
                int inner_y = center_y + (int)((radius-2) * sin(angle));
                if (inner_x >= 0 && inner_x < SCREEN_WIDTH && inner_y >= 0 && inner_y < SCREEN_HEIGHT) {
                    u8g2.drawPixel(inner_x, inner_y);
                }
            }
        }
    }

    u8g2.sendBuffer();
}

// 显示任务
void display_task(void* pvParameters) {
    Serial.println("🖥️  显示任务启动");

    // 在任务中初始化OLED
    Serial.println("   在显示任务中初始化OLED...");
    if (!oled_init()) {
        Serial.println("❌ 显示任务中OLED初始化失败，任务退出");
        vTaskDelete(NULL);
        return;
    }

    // 显示开机动画
    Serial.println("   播放开机动画...");
    animation_frame = 0;
    last_animation_time = millis();

    for (int i = 0; i < 15; i++) {
        oled_display_boot_animation();
        delay(300);
    }

    Serial.println("✅ 显示任务初始化完成，开始主循环");

    while (1) {
        // 检测界面切换
        bool state_changed = (current_state != last_display_state);
        if (state_changed) {
            Serial.printf("🎬 界面切换: %d -> %d\n", last_display_state, current_state);
            last_display_state = current_state;
        }

        // 根据状态显示对应界面
        switch (current_state) {
            case GAME_STATE_IDLE:
                oled_display_idle_screen();
                break;

            case GAME_STATE_PLAYING:
                oled_display_game_screen();
                break;

            case GAME_STATE_PAUSED:
                oled_display_pause_screen();
                break;

            case GAME_STATE_RESET_CONFIRM:
                oled_display_reset_confirm_screen();
                break;

            case GAME_STATE_LAUNCHING:
                // 火箭发射动画状态
                if (state_changed) {
                    Serial.println("🚀 开始播放火箭发射动画");
                    start_rocket_launch_animation();
                }

                // 播放火箭发射动画
                oled_display_rocket_launch_animation();

                // 检查动画是否完成
                if (!rocket_launch_active) {
                    Serial.println("🚀 火箭发射动画完成，切换到结算状态");
                    game_calculate_result(); // 计算游戏结果
                }
                break;

            case GAME_STATE_RESULT:
                // 结算状态，直接显示结果
                oled_display_result_screen();
                break;
        }

        delay(100); // 10FPS更新率，提供流畅体验
    }
}
