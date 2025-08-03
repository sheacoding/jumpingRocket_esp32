#include "jumping_rocket_simple.h"
#include "u8g2_wqy.h"  // 添加中文字体支持

// V3.0 UI集成
#ifdef JUMPING_ROCKET_V3
#include "v3/game_integration_v3.h"
#endif

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

// 字体定义 - 针对128x64优化，添加中文字体支持
// 英文字体（保持兼容性）
#define FONT_TINY     u8g2_font_4x6_tf        // 4x6像素，用于小标签
#define FONT_SMALL    u8g2_font_6x10_tf       // 6x10像素，用于数值
#define FONT_MEDIUM   u8g2_font_7x13_tf       // 7x13像素，用于标题
#define FONT_LARGE    u8g2_font_10x20_tf      // 10x20像素，用于大数字
#define FONT_TITLE    u8g2_font_ncenB12_tf    // 12像素粗体，用于主标题
#define FONT_XLARGE   u8g2_font_ncenB14_tf    // 14像素超大字体，用于结算界面时间
#define FONT_XXLARGE  u8g2_font_ncenB18_tf    // 18像素巨大字体，用于结算界面重要数据

// 中文字体定义 - 根据显示区域优化尺寸
#define FONT_CHINESE_TINY     u8g2_font_wqy12_t_gb2312a    // 12像素中文，用于小标签和提示
#define FONT_CHINESE_SMALL    u8g2_font_wqy13_t_gb2312a    // 13像素中文，用于数值标签
#define FONT_CHINESE_MEDIUM   u8g2_font_wqy14_t_gb2312a    // 14像素中文，用于主要文本
#define FONT_CHINESE_LARGE    u8g2_font_wqy15_t_gb2312a    // 15像素中文，用于标题

// 中英文混合显示的字体选择宏
#define FONT_MIXED_TINY       FONT_CHINESE_TINY     // 中英文混合小字体
#define FONT_MIXED_SMALL      FONT_CHINESE_SMALL    // 中英文混合小字体
#define FONT_MIXED_MEDIUM     FONT_CHINESE_MEDIUM   // 中英文混合中字体
#define FONT_MIXED_LARGE      FONT_CHINESE_LARGE    // 中英文混合大字体

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

const uint8_t icon_gear[] = {
    0x3C,  // 00111100 - 齿轮外圈
    0x7E,  // 01111110 - 齿轮齿
    0xFF,  // 11111111 - 齿轮齿
    0xE7,  // 11100111 - 齿轮主体
    0xE7,  // 11100111 - 齿轮中心
    0xFF,  // 11111111 - 齿轮齿
    0x7E,  // 01111110 - 齿轮齿
    0x3C   // 00111100 - 齿轮外圈
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

    // 测试文本（仅串口输出，不在屏幕显示）
    Serial.println("   OLED测试文本检查...");
    Serial.println("   ✅ OLED Test OK");
    Serial.println("   ✅ Initializing...");
    Serial.println("   跳过屏幕测试显示，直接进入开机动画");

    display_initialized = true;
    Serial.println("🎉 U8g2 OLED初始化完全成功");

    return true;
}

// 清屏
void oled_clear(void) {
    if (!display_initialized) return;
    u8g2.clearBuffer();
}

// 显示文本（自动选择合适字体）
void oled_display_text(int x, int y, const char* text) {
    if (!display_initialized || !text) return;
    u8g2.drawStr(x, y, text);
}

// 显示中文文本（指定字体）
void oled_display_chinese_text(int x, int y, const char* text, const uint8_t* font) {
    if (!display_initialized || !text || !font) return;
    u8g2.setFont(font);
    u8g2.drawUTF8(x, y, text);
}

// 显示中英文混合文本（使用中文字体）
void oled_display_mixed_text(int x, int y, const char* text, const uint8_t* font) {
    if (!display_initialized || !text || !font) return;
    u8g2.setFont(font);
    u8g2.drawUTF8(x, y, text);
}

// 便捷的中文文本显示函数
void oled_display_chinese_tiny(int x, int y, const char* text) {
    oled_display_chinese_text(x, y, text, FONT_CHINESE_TINY);
}

void oled_display_chinese_small(int x, int y, const char* text) {
    oled_display_chinese_text(x, y, text, FONT_CHINESE_SMALL);
}

void oled_display_chinese_medium(int x, int y, const char* text) {
    oled_display_chinese_text(x, y, text, FONT_CHINESE_MEDIUM);
}

void oled_display_chinese_large(int x, int y, const char* text) {
    oled_display_chinese_text(x, y, text, FONT_CHINESE_LARGE);
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

// 开机动画（重新设计布局，解决ROCKET文字与三个点的重叠问题）
void oled_display_boot_animation(void) {
    if (!display_initialized) return;

    uint32_t current_time = millis();

    // 每100ms更新一帧（提高动画流畅度）
    if (current_time - last_animation_time >= 100) {
        u8g2.clearBuffer();

        // 火箭图标（上移，为下方元素预留更多空间）
        int rocket_x = (SCREEN_WIDTH - 16) / 2;  // 精确居中
        int rocket_y = 8;  // 上移到18px位置
        draw_large_icon(rocket_x, rocket_y, icon_rocket_large);

        // "蹦跳火箭"中文文字（重新计算布局，确保不超出边界且与其他元素不重叠）
        uint32_t text_cycle = millis() % 1000; // 直接使用millis()，1秒周期
        float text_opacity = 0.5f + 0.5f * sin(text_cycle * 2 * PI / 1000.0f);

        if (svg_opacity_visible(text_opacity, 0)) {
            u8g2.setFont(FONT_CHINESE_SMALL); // 使用中文小字体 (13px)
            const char* rocket_text = "蹦跳火箭";
            int text_width = u8g2.getUTF8Width(rocket_text);
            
            // 检查文字是否会超出屏幕边界
            if (text_width > SCREEN_WIDTH - 4) {
                // 如果太宽，使用更小的字体
                u8g2.setFont(FONT_CHINESE_TINY);
                text_width = u8g2.getUTF8Width(rocket_text);
            }
            
            int text_x = (SCREEN_WIDTH - text_width) / 2; // 精确居中
            int text_y = 32;  // 调整到40px，为三个点预留更多空间
            
            // 确保不超出边界
            if (text_x < 2) text_x = 2;
            if (text_x + text_width > SCREEN_WIDTH - 2) text_x = SCREEN_WIDTH - text_width - 2;
            
            u8g2.drawUTF8(text_x, text_y, rocket_text);
        }

        // 三个进度指示点（实心点依次移动的波浪式动画）
        static uint32_t animation_start_time = 0;
        if (animation_start_time == 0) {
            animation_start_time = millis(); // 记录动画开始时间
        }

        uint32_t current_millis = millis();
        uint32_t elapsed_time = current_millis - animation_start_time;
        uint32_t dot_cycle = elapsed_time % 1500; // 1.5秒周期

        // 计算三个点的Y位置，确保与中文文字有足够间距
        int dots_y = 54;  // 距离中文文字底部足够像素 (40+13+5=58)

        // 计算当前活跃点的位置（0=第1个点，1=第2个点，2=第3个点）
        // 每个点持续500ms，总周期1.5秒
        int active_dot = (dot_cycle / 500) % 3;

        // 三个点的X坐标
        int dot1_x = 48;  // 左侧
        int dot2_x = 64;  // 中央
        int dot3_x = 80;  // 右侧

        // 绘制三个点，只有活跃的点是实心圆
        // 第一个点
        if (active_dot == 0) {
            u8g2.drawDisc(dot1_x, dots_y, 2);  // 实心圆
        } else {
            u8g2.drawCircle(dot1_x, dots_y, 2); // 空心圆
        }

        // 第二个点
        if (active_dot == 1) {
            u8g2.drawDisc(dot2_x, dots_y, 2);  // 实心圆
        } else {
            u8g2.drawCircle(dot2_x, dots_y, 2); // 空心圆
        }

        // 第三个点
        if (active_dot == 2) {
            u8g2.drawDisc(dot3_x, dots_y, 2);  // 实心圆
        } else {
            u8g2.drawCircle(dot3_x, dots_y, 2); // 空心圆
        }

        // 添加布局和动画调试信息
        static bool boot_debug_printed = false;
        static uint32_t last_debug_time = 0;
        if (!boot_debug_printed) {
            Serial.printf("🚀 开机动画布局修复: 火箭(%d,%d) ROCKET文字(居中,42) 三个点(48,64,80,%d)\n",
                         rocket_x, rocket_y, dots_y);
            Serial.printf("📐 垂直分布: 火箭18-34px, ROCKET文字42-52px, 三个点54-58px, 完全分离\n");
            Serial.printf("🎬 波浪式动画: 1.5秒周期，实心点依次移动 ●○○→○●○→○○●\n");
            boot_debug_printed = true;
        }

        // 每500ms输出一次动画状态（更频繁的调试）
        if (current_time - last_debug_time > 500) {
            Serial.printf("🔄 开机动画状态: 活跃点%d/3, 周期%lums, 经过时间%lums\n", active_dot, dot_cycle, elapsed_time);
            Serial.printf("📍 点状态: 点1(%s) 点2(%s) 点3(%s)\n",
                         (active_dot == 0) ? "●实心" : "○空心",
                         (active_dot == 1) ? "●实心" : "○空心",
                         (active_dot == 2) ? "●实心" : "○空心");
            Serial.printf("🧮 计算详情: 开始时间=%lu, 当前时间=%lu, 经过=%lu, 周期=%lu, 活跃点=%d\n",
                         animation_start_time, current_millis, elapsed_time, dot_cycle, active_dot);
            last_debug_time = current_time;
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

        // 中央火箭动画区域（保护区域：X=40-88，确保左右侧元素不侵入）
        int rocket_x = (SCREEN_WIDTH - 16) / 2;  // X=56，16像素宽度的火箭居中
        int rocket_y;

        // 中央保护区域边界：左边界40像素，右边界88像素
        const int CENTRAL_AREA_LEFT = 40;
        const int CENTRAL_AREA_RIGHT = 88;

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

        // 右侧高度显示区域（修复重叠问题，基于FONT_LARGE=20px高度重新计算）
        u8g2.setFont(FONT_LARGE);  // 10x20像素字体，高度20像素
        char height_text[16];
        snprintf(height_text, sizeof(height_text), "%lum", dynamic_height);
        int height_width = u8g2.getStrWidth(height_text);
        int height_x = SCREEN_WIDTH - height_width - 3; // 右对齐，留3像素边距
        int height_y = 15; // 重新定位，为20像素高度字体预留空间
        u8g2.drawStr(height_x, height_y, height_text);

        // ALTITUDE标签（右侧，确保与高度数字完全分离）
        u8g2.setFont(FONT_TINY);  // 4x6像素字体，高度6像素
        const char* alt_label = "高度";
        int label_width = u8g2.getStrWidth(alt_label);
        int label_x = SCREEN_WIDTH - label_width - 3; // 与高度数字对齐
        int label_y = 37; // 距离高度字体底部2像素 (15+20+2=37)
        u8g2.drawStr(label_x, label_y, alt_label);

        // 左侧统计信息区域（确保不侵入中央保护区域40px边界）
        u8g2.setFont(FONT_TINY);  // 4x6像素字体

        // 跳跃统计（左上，调整位置避免与右侧高度显示冲突）
        char jump_text[8]; // 进一步缩短避免超出边界
        snprintf(jump_text, sizeof(jump_text), "J:%lu", game_data.jump_count);
        int jump_width = u8g2.getStrWidth(jump_text);
        // 确保文字不超过中央保护区域左边界(40px)
        int jump_x = (jump_width < 37) ? 3 : (40 - jump_width);
        u8g2.drawStr(jump_x, 15, jump_text); // 与右侧高度对齐

        // 时间统计（左下，确保不超出边界）
        char time_text[8]; // 进一步缩短
        uint32_t total_seconds = game_data.game_time_ms / 1000;
        uint32_t minutes = total_seconds / 60;
        uint32_t seconds = total_seconds % 60;
        snprintf(time_text, sizeof(time_text), "T:%02lu:%02lu", minutes, seconds);
        int time_width = u8g2.getStrWidth(time_text);
        // 确保文字不超过中央保护区域左边界(40px)
        int time_x = (time_width < 37) ? 3 : (40 - time_width);
        u8g2.drawStr(time_x, 25, time_text); // 与跳跃统计间距10像素

        // 添加布局调试信息
        static uint32_t last_debug_time = 0;
        if (current_time - last_debug_time > 1000) { // 每秒输出一次
            Serial.printf("🚀 发射动画布局修复: 高度(%d,%d) 标签(%d,%d) 跳跃(%d,15) 时间(%d,25)\n",
                         height_x, height_y, label_x, label_y, jump_x, time_x);
            Serial.printf("📐 字体高度: FONT_LARGE=20px, 高度占用15-35px, ALTITUDE在37px, 中央保护区40-88px\n");
            last_debug_time = current_time;
        }

        u8g2.sendBuffer();

        animation_frame++;
        last_animation_time = current_time;
    }
}

// 游戏界面显示（基于SVG设计精确重构）
void oled_display_game_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // 检查是否需要屏幕闪烁效果
    if (is_target_flash_active() && !should_screen_flash_now()) {
        // 闪烁状态：不显示内容，只显示空白屏幕
        u8g2.sendBuffer();
        return;
    }

    // 顶部状态栏（上移到屏幕顶部边缘，避免双色分界线）

    // 时间显示（移到屏幕顶部）
    u8g2.setFont(FONT_MEDIUM); // 从FONT_SMALL升级到FONT_MEDIUM（放大0.8倍）
    uint32_t total_seconds = game_data.game_time_ms / 1000;
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;
    char time_text[16];
    snprintf(time_text, sizeof(time_text), "%02lu:%02lu", minutes, seconds);
    int time_x = 2;  // 靠近左边缘
    int time_y = 10;  // 下移到10px，为FONT_MEDIUM字体预留更多空间
    u8g2.drawStr(time_x, time_y, time_text);

    // 跳跃计数（移到屏幕顶部）
    u8g2.setFont(FONT_MEDIUM); // 从FONT_SMALL升级到FONT_MEDIUM（放大0.8倍）
    char count_text[16];
    snprintf(count_text, sizeof(count_text), "%lu", game_data.jump_count);

    // 计算跳跃次数的宽度，右对齐显示
    int count_width = u8g2.getStrWidth(count_text);
    int count_x = SCREEN_WIDTH - count_width - 2; // 靠近右边缘，留2像素边距
    int count_y = 10;  // 与时间对齐，下移到10px适应更大字体
    u8g2.drawStr(count_x, count_y, count_text);

    // 移除状态栏分割线

    // 火箭图标（重新定位到屏幕正中心）
    int rocket_x = (SCREEN_WIDTH - 8) / 2;  // 水平居中：(128-8)/2 = 60
    int rocket_y = (SCREEN_HEIGHT - 8) / 2; // 垂直居中：(64-8)/2 = 28

    // 绘制火箭图标，位于屏幕正中心
    draw_icon(rocket_x, rocket_y, icon_rocket_small);

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

            // 第一层波纹（波纹中心与小火箭图标位置同步）
            int center_x = rocket_x + 4;  // 火箭图标中心：rocket_x + 8/2 = rocket_x + 4
            int center_y = rocket_y + 4;  // 火箭图标中心：rocket_y + 8/2 = rocket_y + 4
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

            // 第二层波纹（中心位置同步）
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

    // 燃料区域（整体下移到距离屏幕下边缘5px位置）
    int fuel_area_y = SCREEN_HEIGHT - 5 - 12; // 64 - 5 - 12 = 47px，为燃料区域预留12px高度
    
    // 计算整体宽度（文本+间距+进度条）
    u8g2.setFont(FONT_CHINESE_TINY);
    const char* fuel_label = "燃料";
    int fuel_label_width = u8g2.getUTF8Width(fuel_label);
    int bar_width = 98;  // 进度条宽度
    int spacing = 6;     // 文本与进度条间距
    int total_width = fuel_label_width + spacing + bar_width;
    
    // 整体水平居中
    int start_x = (SCREEN_WIDTH - total_width) / 2;
    
    // "燃料"标签（水平对齐）
    int fuel_label_x = start_x;
    int fuel_label_y = fuel_area_y+6;  // 上移4像素
    u8g2.drawUTF8(fuel_label_x, fuel_label_y, fuel_label);

    // 进度条（与文本水平对齐）
    int bar_x = start_x + fuel_label_width + spacing;
    int bar_y = fuel_area_y + 8;              // 在文字下方8px
    int bar_height = 8;                       // 高度4px，更紧凑
    u8g2.drawFrame(bar_x, bar_y, bar_width, bar_height);

    // 进度条填充
    int available_width = bar_width - 2;     // 58像素可用宽度
    int fill_width = (available_width * game_data.fuel_progress) / 100;

    // 确保100%时完全填满
    if (game_data.fuel_progress >= 100) {
        fill_width = available_width;
    }

    if (fill_width > 0) {
        u8g2.drawBox(bar_x + 1, bar_y + 1, fill_width, bar_height - 2);
    }

    // 调试输出燃料区域布局
    static bool fuel_debug_printed = false;
    if (!fuel_debug_printed) {
        Serial.printf("⛽ 燃料区域重新布局: 标签(%d,%d) 进度条(%d,%d,%dx%d)\n",
                     fuel_label_x, fuel_label_y, bar_x, bar_y, bar_width, bar_height);
        Serial.printf("📐 燃料区域: 距离屏幕底部5px，总高度12px (47-59px)\n");
        fuel_debug_printed = true;
    }

    // 移除底部按键提示，按照用户要求去掉页面上的按键提示信息
    // 原来的按键提示代码已注释掉
    /*
    u8g2.setFont(FONT_CHINESE_TINY);
    
    const char* hint1 = "按键:暂停";
    const char* hint2 = "长按:重置";
    
    int hint1_width = u8g2.getUTF8Width(hint1);
    int hint2_width = u8g2.getUTF8Width(hint2);
    
    int hint_y = svg_transform_y(0, 55);
    
    // 计算两个提示的位置，确保不重叠
    int total_width = hint1_width + hint2_width + 6; // 加6px间距
    
    if (total_width <= SCREEN_WIDTH - 4) {
        // 如果能放在一行，左右分布
        int hint1_x = 2;
        int hint2_x = SCREEN_WIDTH - hint2_width - 2;
        
        // 检查是否重叠
        if (hint1_x + hint1_width + 6 > hint2_x) {
            // 重叠时，调整为上下分布
            hint1_x = (SCREEN_WIDTH - hint1_width) / 2;
            hint_y -= 8;
            u8g2.drawUTF8(hint1_x, hint_y, hint1);
            
            hint2_x = (SCREEN_WIDTH - hint2_width) / 2;
            hint_y += 8;
            u8g2.drawUTF8(hint2_x, hint_y, hint2);
        } else {
            // 不重叠，正常显示
            u8g2.drawUTF8(hint1_x, hint_y, hint1);
            u8g2.drawUTF8(hint2_x, hint_y, hint2);
        }
    } else {
        // 太宽，必须上下分布
        int hint1_x = (SCREEN_WIDTH - hint1_width) / 2;
        int hint2_x = (SCREEN_WIDTH - hint2_width) / 2;
        
        hint_y -= 6;
        u8g2.drawUTF8(hint1_x, hint_y, hint1);
        hint_y += 10;
        u8g2.drawUTF8(hint2_x, hint_y, hint2);
    }
    */

    u8g2.sendBuffer();
}

// 暂停界面显示（基于SVG设计精确重构）
void oled_display_pause_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // 闪烁边框效果（移到屏幕最边缘，避免与文字重叠）
    uint32_t border_cycle = millis() % 1000; // 1秒周期
    float border_t = border_cycle / 1000.0f;
    float border_opacity = 0.3f + 0.7f * (0.5f + 0.5f * sin(border_t * 2 * PI)); // 0.3-1.0变化

    // 使用透明度控制边框显示
    if (svg_opacity_visible(border_opacity, 0)) {
        // 绘制双重边框（放到屏幕最边缘）
        u8g2.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        u8g2.drawFrame(1, 1, SCREEN_WIDTH-2, SCREEN_HEIGHT-2);
    }

    // 暂停图标（上移，为底部文字预留空间）
    int pause_icon_x = (SCREEN_WIDTH - 8) / 2;  // 8px图标居中
    int pause_icon_y = 12;  // 上移到12px
    draw_icon(pause_icon_x, pause_icon_y, icon_pause);

    // "已暂停"中文文字（上移，确保与边框和底部文字有足够间距）
    u8g2.setFont(FONT_CHINESE_MEDIUM);
    const char* title = "已暂停";
    int title_width = u8g2.getUTF8Width(title);
    int title_x = (SCREEN_WIDTH - title_width) / 2;  // 精确居中
    int title_y = 24;  // 上移到24px
    u8g2.drawUTF8(title_x, title_y, title);

    // 统计信息三列布局（上移，确保与边框有足够间距）
    u8g2.setFont(FONT_SMALL);

    // 第一列：跳跃次数（左侧）
    char jump_text[16];
    snprintf(jump_text, sizeof(jump_text), "%lu", game_data.jump_count);
    int jump_x = 15;  // 左侧位置，距离边框15px
    int jump_y = 36;  // 上移到36px
    u8g2.drawStr(jump_x, jump_y, jump_text);

    u8g2.setFont(FONT_CHINESE_TINY);
    int jump_label_y = 44;  // 标签位置44px
    u8g2.drawUTF8(jump_x, jump_label_y, "跳跃");

    // 第二列：游戏时长（中央）
    uint32_t total_seconds = game_data.game_time_ms / 1000;
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;

    u8g2.setFont(FONT_SMALL);
    char time_text[16];
    snprintf(time_text, sizeof(time_text), "%02lu:%02lu", minutes, seconds);
    int time_width = u8g2.getStrWidth(time_text);
    int time_x = (SCREEN_WIDTH - time_width) / 2;  // 居中
    int time_y = 36;  // 与跳跃次数对齐
    u8g2.drawStr(time_x, time_y, time_text);

    u8g2.setFont(FONT_CHINESE_TINY);
    const char* time_label = "时间";
    int time_label_width = u8g2.getUTF8Width(time_label);
    int time_label_x = (SCREEN_WIDTH - time_label_width) / 2;  // 标签居中
    u8g2.drawUTF8(time_label_x, jump_label_y, time_label);

    // 第三列：燃料进度（右侧）
    u8g2.setFont(FONT_SMALL);
    char fuel_text[8];
    snprintf(fuel_text, sizeof(fuel_text), "%lu%%", game_data.fuel_progress);
    int fuel_width = u8g2.getStrWidth(fuel_text);
    int fuel_x = SCREEN_WIDTH - fuel_width - 15;  // 右侧位置，距离边框15px
    int fuel_y = 36;  // 与其他数据对齐
    u8g2.drawStr(fuel_x, fuel_y, fuel_text);

    u8g2.setFont(FONT_CHINESE_TINY);
    const char* fuel_label = "燃料";
    int fuel_label_width = u8g2.getUTF8Width(fuel_label);
    int fuel_label_x = SCREEN_WIDTH - fuel_label_width - 15;  // 与数据对齐
    u8g2.drawUTF8(fuel_label_x, jump_label_y, fuel_label);

    // 移除按键提示，按照用户要求去掉页面上的按键提示信息
    // 注释掉原来的按键提示代码
    /*
    u8g2.setFont(FONT_CHINESE_TINY);
    const char* hint = "按键:继续  长按:重置";
    int hint_width = u8g2.getUTF8Width(hint);
    int hint_x = (SCREEN_WIDTH - hint_width) / 2;  // 精确居中
    int hint_y = 54;  // 上移到54px，距离底部边框10px
    u8g2.drawUTF8(hint_x, hint_y, hint);
    */

    u8g2.sendBuffer();
}

// 重置确认界面（基于SVG设计精确重构）
void oled_display_reset_confirm_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // 警告边框闪烁效果（单层边框，避免遮挡文字）
    uint32_t border_cycle = millis() % 600; // 0.6秒周期
    float border_t = border_cycle / 600.0f;
    float border_opacity = 0.3f + 0.7f * (0.5f + 0.5f * sin(border_t * 4 * PI)); // 快速闪烁

    if (svg_opacity_visible(border_opacity, 0)) {
        // 绘制警告边框（仅外层边框，不遮挡内容）
        u8g2.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        u8g2.drawFrame(1, 1, SCREEN_WIDTH-2, SCREEN_HEIGHT-2);
    }

    // 警告图标（上移，为底部文字预留空间）
    int warning_x = (SCREEN_WIDTH - 8) / 2;  // 8px图标居中
    int warning_y = 14;  // 上移到14px
    draw_icon(warning_x, warning_y, icon_warning);

    // "重置?"中文文字（上移，确保与边框和底部文字有足够间距）
    u8g2.setFont(FONT_CHINESE_MEDIUM);
    const char* title = "重置?";
    int title_width = u8g2.getUTF8Width(title);
    
    // 检查是否超出边界
    if (title_width > SCREEN_WIDTH - 4) {
        u8g2.setFont(FONT_CHINESE_SMALL);
        title_width = u8g2.getUTF8Width(title);
    }
    
    int title_x = (SCREEN_WIDTH - title_width) / 2;  // 精确居中
    if (title_x < 2) title_x = 2;
    if (title_x + title_width > SCREEN_WIDTH - 2) title_x = SCREEN_WIDTH - title_width - 2;
    
    int title_y = 26;  // 上移到26px
    u8g2.drawUTF8(title_x, title_y, title);

    // 闪烁警告文字（改为中文，确保与其他元素有足够间距）
    uint32_t text_cycle = millis() % 800; // 0.8秒周期
    float text_t = text_cycle / 800.0f;
    float text_opacity = 0.5f + 0.5f * sin(text_t * 2 * PI);

    if (svg_opacity_visible(text_opacity, 100)) {
        u8g2.setFont(FONT_CHINESE_TINY);
        const char* warning = "所有进度将丢失!";
        int warning_width = u8g2.getUTF8Width(warning);
        int warning_x = (SCREEN_WIDTH - warning_width) / 2;  // 精确居中
        
        // 确保不超出边界
        if (warning_x < 2) warning_x = 2;
        if (warning_x + warning_width > SCREEN_WIDTH - 2) warning_x = SCREEN_WIDTH - warning_width - 2;
        
        int warning_y = 36;  // 上移到36px
        u8g2.drawUTF8(warning_x, warning_y, warning);
    }

    // 移除操作说明，按照用户要求去掉页面上的按键提示信息
    // 注释掉原来的操作说明代码
    /*
    u8g2.setFont(FONT_TINY);

    // 确认操作（上移）
    const char* confirm = "Hold: Confirm";
    int confirm_width = u8g2.getStrWidth(confirm);
    int confirm_x = (SCREEN_WIDTH - confirm_width) / 2;  // 精确居中
    int confirm_y = 46;  // 上移到46px
    u8g2.drawStr(confirm_x, confirm_y, confirm);

    // 取消操作（上移，确保距离底部边框有足够间距）
    const char* cancel = "Press: Cancel";
    int cancel_width = u8g2.getStrWidth(cancel);
    int cancel_x = (SCREEN_WIDTH - cancel_width) / 2;  // 精确居中
    int cancel_y = 54;  // 上移到54px，距离底部边框10px
    u8g2.drawStr(cancel_x, cancel_y, cancel);
    */

    u8g2.sendBuffer();
}

// 结算界面显示（上移内容，分离时间与标签，保留底部图标）
void oled_display_result_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // === 顶部图标区域 ===
    // 左侧奖杯图标
    int trophy_x = 5;   // 左侧边距5像素
    int trophy_y = 2;   // 顶部边距2像素
    draw_icon(trophy_x, trophy_y, icon_trophy);

    // 右侧火箭图标
    int rocket_x = SCREEN_WIDTH - 8 - 5;  // 右侧边距5像素
    int rocket_y = 2;                     // 与奖杯图标对齐
    draw_icon(rocket_x, rocket_y, icon_rocket_small);

    // === 时间标签区域（移到原标题位置） ===
    // "运动时间"标签（上移4px以保证底部显示完整）
    u8g2.setFont(FONT_CHINESE_SMALL);  // 使用中文字体
    const char* time_label = "运动时间";
    int time_label_width = u8g2.getUTF8Width(time_label);
    int time_label_x = (SCREEN_WIDTH - time_label_width) / 2;
    int time_label_y = 11;  // 从15px上移4px到11px
    u8g2.drawUTF8(time_label_x, time_label_y, time_label);

    // === 时间显示区域（重新调整） ===
    // 健身时长（主要功能，使用超大字体突出显示 - 放大一倍）
    uint32_t total_seconds = game_data.game_time_ms / 1000;
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;

    u8g2.setFont(FONT_XXLARGE);  // 从FONT_LARGE升级到FONT_XXLARGE（放大一倍）
    char time_text[16];
    snprintf(time_text, sizeof(time_text), "%02lu:%02lu", minutes, seconds);
    int time_width = u8g2.getStrWidth(time_text);
    int time_x = (SCREEN_WIDTH - time_width) / 2;
    int time_y = 21;  // 从25px上移4px到21px
    u8g2.drawStr(time_x, time_y, time_text);

    // === 底部统计区域（移除按键提示，重新调整布局） ===
    
    // 第一列：跳跃次数（左侧） - 放大一倍
    u8g2.setFont(FONT_MEDIUM);  // 从FONT_TINY升级到FONT_MEDIUM（放大一倍）
    char jump_text[16];
    snprintf(jump_text, sizeof(jump_text), "%lu", game_data.jump_count);
    int jump_x = 8;   // 左侧边距8px
    int jump_y = 38;  // 从42px上移4px到38px
    u8g2.drawStr(jump_x, jump_y, jump_text);
    
    u8g2.setFont(FONT_CHINESE_TINY);  // 使用中文小字体
    u8g2.drawUTF8(jump_x, 51, "跳跃");  // 标签位置从55px上移4px到51px

    // 第二列：飞行高度（中央）
    u8g2.setFont(FONT_MEDIUM);  // 与跳跃次数保持一致的字体大小
    char height_text[16];
    snprintf(height_text, sizeof(height_text), "%lum", game_data.flight_height);
    int height_width = u8g2.getStrWidth(height_text);
    int height_x = (SCREEN_WIDTH - height_width) / 2;  // 居中
    int height_y = 38;  // 从42px上移4px到38px，与跳跃次数对齐
    u8g2.drawStr(height_x, height_y, height_text);

    // 高度标签居中对齐
    u8g2.setFont(FONT_CHINESE_TINY);  // 使用中文小字体
    const char* height_label = "高度";
    int height_label_width = u8g2.getUTF8Width(height_label);
    int height_label_x = (SCREEN_WIDTH - height_label_width) / 2;
    u8g2.drawUTF8(height_label_x, 51, height_label);  // 从55px上移4px到51px

    // 第三列：燃料使用（右侧）
    u8g2.setFont(FONT_MEDIUM);  // 与跳跃次数保持一致的字体大小
    char fuel_text[8];
    snprintf(fuel_text, sizeof(fuel_text), "%lu%%", game_data.fuel_progress);
    int fuel_width = u8g2.getStrWidth(fuel_text);
    int fuel_x = SCREEN_WIDTH - fuel_width - 8;  // 右侧边距8px
    int fuel_y = 38;  // 从42px上移4px到38px，与其他统计数据对齐
    u8g2.drawStr(fuel_x, fuel_y, fuel_text);

    // 燃料标签右对齐
    u8g2.setFont(FONT_CHINESE_TINY);  // 使用中文小字体
    const char* fuel_label = "燃料";
    int fuel_label_width = u8g2.getUTF8Width(fuel_label);
    int fuel_label_x = SCREEN_WIDTH - fuel_label_width - 8;
    u8g2.drawUTF8(fuel_label_x, 51, fuel_label);  // 从55px上移4px到51px

    // 添加布局调试信息
    static bool layout_debug_printed = false;
    if (!layout_debug_printed) {
        Serial.printf("🏆 结算界面布局优化: 整体上移4px，保证底部显示完整\n");
        Serial.printf("   图标区域: 奖杯(5,2) 火箭(115,2) - 保持不变\n");
        Serial.printf("   时间标签: 运动时间 (%d,11) - 上移4px\n", time_label_x);
        Serial.printf("   时间: %s (%d,21) - 上移4px\n", time_text, time_x);
        Serial.printf("   统计数据: (38px) 上移4px\n");
        Serial.printf("   中文标签: (51px) 上移4px\n");
        Serial.printf("   垂直分布: 图标2px, 时间标签11px, 时间21px, 统计38px, 标签51px\n");
        layout_debug_printed = true;
    }

    u8g2.sendBuffer();
}

// 待机界面显示（完全重新设计，移除所有可能的重叠元素）
void oled_display_idle_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // 顶部状态栏 - 检查中文文字布局
    u8g2.setFont(FONT_CHINESE_TINY);
    const char* status_text = "准备就绪";
    int status_width = u8g2.getUTF8Width(status_text);
    
    // 确保不超出屏幕边界
    int status_text_x = 5;
    if (status_text_x + status_width > SCREEN_WIDTH - 5) {
        status_text_x = SCREEN_WIDTH - status_width - 5;
    }
    int status_text_y = 9;
    u8g2.drawUTF8(status_text_x, status_text_y, status_text);

    // 中央火箭图标（重新定位，避免与任何其他元素重叠）
    int rocket_x = (SCREEN_WIDTH - 16) / 2;  // 水平居中
    int rocket_y = 24;  // 垂直居中位置
    draw_large_icon(rocket_x, rocket_y, icon_rocket_large);

    // 提示文字（确保在底部有足够空间且不超出边界）
    u8g2.setFont(FONT_CHINESE_SMALL);
    const char* hint = "跳跃开始";
    int hint_width = u8g2.getUTF8Width(hint);
    int hint_x = (SCREEN_WIDTH - hint_width) / 2;
    
    // 确保不超出屏幕边界
    if (hint_x < 2) hint_x = 2;
    if (hint_x + hint_width > SCREEN_WIDTH - 2) hint_x = SCREEN_WIDTH - hint_width - 2;
    
    int hint_y = 50;  // 底部位置，确保不与其他元素重叠
    // 检查是否会超出屏幕底部
    if (hint_y + 4 > SCREEN_HEIGHT) hint_y = SCREEN_HEIGHT - 6;
    
    u8g2.drawUTF8(hint_x, hint_y, hint);

    // 注意：移除所有可能导致重叠的元素
    // 不绘制"ROCKET"文字
    // 不绘制三个动画点
    // 不绘制开机动画的任何元素

    // 呼吸灯效果（重新定位到火箭图标和提示文字之间）
    uint32_t breath_cycle = svg_animate_progress(millis(), SVG_DUR_TO_MS(2.0)); // 2秒周期
    float t = breath_cycle / 2000.0f;

    // 调整半径范围，避免与其他元素重叠
    float radius_progress = sin(t * PI); // 0->1->0 的正弦曲线
    int radius = 8 + (int)(4 * radius_progress); // 8-12像素变化（缩小范围）

    // 透明度变化保持不变
    float opacity_progress = 0.5f * (1.0f + cos(t * 2 * PI)); // 0->1->0 的余弦曲线
    float opacity = 0.1f + 0.2f * opacity_progress; // 0.1-0.3变化

    // 呼吸灯圆环重新定位到中央空白区域
    int center_x = SCREEN_WIDTH / 2;  // 屏幕水平中心
    int center_y = 38;  // 火箭图标(20-36)和提示文字(52)之间的中央位置

    // 使用点阵密度模拟透明度
    int point_density = (int)(opacity * 16); // 减少点数，避免过于密集
    for (int i = 0; i < point_density; i++) {
        float angle = (i * 360.0f / point_density) * PI / 180.0f;
        int x = center_x + (int)(radius * cos(angle));
        int y = center_y + (int)(radius * sin(angle));

        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            u8g2.drawPixel(x, y);
            // 添加内圈点增强效果（调整内圈半径）
            if (radius > 9) {
                int inner_x = center_x + (int)((radius-1) * cos(angle));
                int inner_y = center_y + (int)((radius-1) * sin(angle));
                if (inner_x >= 0 && inner_x < SCREEN_WIDTH && inner_y >= 0 && inner_y < SCREEN_HEIGHT) {
                    u8g2.drawPixel(inner_x, inner_y);
                }
            }
        }
    }

    // 添加布局调试信息
    static bool idle_debug_printed = false;
    if (!idle_debug_printed) {
        Serial.printf("😴 待机界面布局优化: 火箭(%d,%d) 呼吸灯(%d,%d,r=%d-%d) 提示(%d,%d)\n",
                     rocket_x, rocket_y, center_x, center_y, 8, 12, hint_x, hint_y);
        Serial.printf("📐 垂直分布: 火箭20-36px, 呼吸灯34-42px, 提示52px, 完全分离\n");
        idle_debug_printed = true;
    }

    u8g2.sendBuffer();
}

// 难度选择界面显示（基于现有界面风格设计）
void oled_display_difficulty_select_screen(void) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // 移除顶部横线和边框效果
    // 注释掉原来的闪烁边框代码
    /*
    uint32_t border_cycle = millis() % 1000; // 1秒周期
    float border_t = border_cycle / 1000.0f;
    float border_opacity = 0.3f + 0.7f * (0.5f + 0.5f * sin(border_t * 2 * PI)); // 0.3-1.0变化

    if (svg_opacity_visible(border_opacity, 0)) {
        // 绘制双重边框（放到屏幕最边缘）
        u8g2.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        u8g2.drawFrame(1, 1, SCREEN_WIDTH-2, SCREEN_HEIGHT-2);
    }
    */

    // 标题图标和文字（上移12个单位）
    int gear_x = (SCREEN_WIDTH - 8) / 2;  // 8px图标居中
    int gear_y = 10 - 12;  // 上移12个单位：10 - 12 = -2px（如果为负数则设为0）
    if (gear_y < 0) gear_y = 0;
    draw_icon(gear_x, gear_y, icon_gear);

    u8g2.setFont(FONT_CHINESE_MEDIUM);
    const char* title = "选择难度";
    int title_width = u8g2.getUTF8Width(title);
    
    // 检查标题是否会超出屏幕边界
    if (title_width > SCREEN_WIDTH - 4) {
        // 如果太宽，使用更小的字体
        u8g2.setFont(FONT_CHINESE_SMALL);
        title_width = u8g2.getUTF8Width(title);
    }
    
    int title_x = (SCREEN_WIDTH - title_width) / 2;  // 精确居中
    // 确保不超出边界
    if (title_x < 2) title_x = 2;
    if (title_x + title_width > SCREEN_WIDTH - 2) title_x = SCREEN_WIDTH - title_width - 2;
    
    int title_y = 22 - 12;  // 上移12个单位：22 - 12 = 10px
    u8g2.drawUTF8(title_x, title_y, title);

    // 三个难度选项的横向布局（选中项闪烁效果）
    const char* difficulties[] = {"简单", "普通", "困难"};
    int option_y = 40 - 12;  // 选项统一Y位置，上移12个单位：40 - 12 = 28px
    int total_width = SCREEN_WIDTH - 20;  // 可用宽度（左右各留10px边距）
    int option_width = total_width / 3;   // 每个选项的宽度
    int start_x = 10;  // 起始X位置

    // 闪烁效果计算（与边框类似的闪烁周期）
    uint32_t blink_cycle = millis() % 800; // 0.8秒周期
    float blink_t = blink_cycle / 800.0f;
    float blink_opacity = 0.3f + 0.7f * (0.5f + 0.5f * sin(blink_t * 2 * PI)); // 0.3-1.0变化

    for (int i = 0; i < 3; i++) {
        bool is_selected = (selected_difficulty == i);
        const char* diff_name = difficulties[i];

        // 计算每个选项的中心位置
        int option_center_x = start_x + i * option_width + option_width / 2;

        // 计算文字宽度以居中显示
        u8g2.setFont(FONT_CHINESE_SMALL);
        int text_width = u8g2.getUTF8Width(diff_name);
        int text_x = option_center_x - text_width / 2;
        
        // 确保不超出分配的区域边界
        int min_x = start_x + i * option_width + 2;
        int max_x = start_x + (i + 1) * option_width - text_width - 2;
        if (text_x < min_x) text_x = min_x;
        if (text_x > max_x) text_x = max_x;

        // 选中项的文字闪烁效果
        if (is_selected) {
            // 使用透明度控制文字闪烁显示
            if (svg_opacity_visible(blink_opacity, 0)) {
                // 绘制难度名称（选中时闪烁显示）
                u8g2.drawUTF8(text_x, option_y, diff_name);
            }
        } else {
            // 非选中项正常显示
            u8g2.drawUTF8(text_x, option_y, diff_name);
        }
    }

    // 添加选中难度的详细信息显示（放在屏幕底部）
    if (selected_difficulty >= 0 && selected_difficulty < 3) {
        u8g2.setFont(FONT_CHINESE_TINY);  // 使用最小字体

        // 根据选中的难度显示相应信息
        const char* detail_info = "";
        switch (selected_difficulty) {
            case DIFFICULTY_EASY:
                detail_info = "燃料60%发射";
                break;
            case DIFFICULTY_NORMAL:
                detail_info = "燃料80%发射";
                break;
            case DIFFICULTY_HARD:
                detail_info = "燃料100%发射";
                break;
        }

        // 计算文字宽度并居中显示，确保不超出边界
        int detail_width = u8g2.getUTF8Width(detail_info);
        int detail_x = (SCREEN_WIDTH - detail_width) / 2;
        
        // 确保不超出屏幕边界
        if (detail_x < 2) detail_x = 2;
        if (detail_x + detail_width > SCREEN_WIDTH - 2) detail_x = SCREEN_WIDTH - detail_width - 2;
        
        int detail_y = 57;  // 调整位置确保文字完整显示：64 - 7 = 57px（预留足够空间）
        // 检查是否会超出屏幕底部
        if (detail_y + 4 > SCREEN_HEIGHT) detail_y = SCREEN_HEIGHT - 6;
        
        u8g2.drawUTF8(detail_x, detail_y, detail_info);
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

        // V3.0 UI模式检查
#ifdef JUMPING_ROCKET_V3
        if (V3_IS_IN_UI()) {
            // V3.0 UI模式渲染
            V3_RENDER_UI();
            delay(100); // 10FPS更新
            continue;
        }
#endif

        // 根据状态显示对应界面
        switch (current_state) {
            case GAME_STATE_IDLE:
#ifdef JUMPING_ROCKET_V3
                // 在待机状态检查是否应该进入V3.0 UI模式
                if (V3_SHOULD_ENTER_UI()) {
                    Serial.println("🎨 从待机状态进入V3.0 UI模式");
                    V3_ENTER_UI();
                    continue;
                }
#endif
                oled_display_idle_screen();
                break;

            case GAME_STATE_DIFFICULTY_SELECT:
                oled_display_difficulty_select_screen();
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
