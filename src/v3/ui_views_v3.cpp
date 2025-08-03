#include "v3/ui_views_v3.h"
#include "v3/data_manager_v3.h"
#include "jumping_rocket_simple.h"

// 中文字体定义 - 与V2.0系统保持一致
#define FONT_CHINESE_TINY     u8g2_font_wqy12_t_gb2312a    // 12像素中文
#define FONT_CHINESE_SMALL    u8g2_font_wqy13_t_gb2312a    // 13像素中文  
#define FONT_CHINESE_MEDIUM   u8g2_font_wqy14_t_gb2312a    // 14像素中文
#define FONT_CHINESE_LARGE    u8g2_font_wqy15_t_gb2312a    // 15像素中文

// 全局UI管理器实例
UIManagerV3* uiManagerV3 = nullptr;

// UIViewV3 基类实现
void UIViewV3::drawTitle(const String& title, int y) {
    display->setFont(FONT_CHINESE_SMALL);  // 使用中文字体
    int width = display->getUTF8Width(title.c_str());
    int x = (128 - width) / 2;
    display->drawUTF8(x, y, title.c_str());
    // 移除顶部横线：display->drawHLine(10, y + 2, 108);
}

void UIViewV3::drawMenuItem(const String& text, int y, bool selected) {
    display->setFont(FONT_CHINESE_SMALL);  // 使用中文字体

    if (selected) {
        // 文字闪烁效果：每500ms切换一次显示状态
        uint32_t current_time = millis();
        bool blink_state = (current_time / 500) % 2 == 0;

        if (blink_state) {
            // 显示文字
            display->drawUTF8(4, y, text.c_str());
        }
        // 不显示状态时什么都不画，实现闪烁效果
    } else {
        // 非选中项正常显示
        display->drawUTF8(4, y, text.c_str());
    }
}

void UIViewV3::drawProgressBar(int x, int y, int width, int height, float progress) {
    // 绘制边框
    display->drawFrame(x, y, width, height);
    
    // 绘制进度
    int fill_width = (int)(progress * (width - 2));
    if (fill_width > 0) {
        display->drawBox(x + 1, y + 1, fill_width, height - 2);
    }
}

void UIViewV3::drawValue(const String& label, const String& value, int y) {
    display->setFont(FONT_CHINESE_SMALL);  // 使用中文字体
    
    // 检查标签宽度，避免与数值重叠
    int label_width = display->getUTF8Width(label.c_str());
    int value_width = display->getUTF8Width(value.c_str());
    
    // 确保标签和数值之间至少有8像素间距
    int max_label_width = 128 - value_width - 8 - 4; // 4是左边距
    
    if (label_width > max_label_width) {
        // 标签太长，使用更小的字体
        display->setFont(FONT_CHINESE_TINY);
        label_width = display->getUTF8Width(label.c_str());
    }
    
    display->drawUTF8(4, y, label.c_str());
    
    // 重新设置字体显示数值
    display->setFont(FONT_CHINESE_SMALL);
    display->drawUTF8(124 - value_width, y, value.c_str());
}

void UIViewV3::drawCenteredText(const String& text, int y) {
    display->setFont(FONT_CHINESE_SMALL);  // 使用中文字体
    int width = display->getUTF8Width(text.c_str());
    int x = (128 - width) / 2;
    display->drawUTF8(x, y, text.c_str());
}

// MainMenuViewV3 实现
MainMenuViewV3::MainMenuViewV3(U8G2* disp) : 
    UIViewV3(disp), selected_index(0), animation_time(0) {
    initMenuItems();
}

void MainMenuViewV3::initMenuItems() {
    menu_items.clear();
    menu_items.push_back(MenuItemV3("开始运动", "", UI_VIEW_DIFFICULTY_SELECT));
    menu_items.push_back(MenuItemV3("目标计时", "", UI_VIEW_TARGET_TIMER));
    menu_items.push_back(MenuItemV3("系统设置", "", UI_VIEW_SETTINGS));
}

void MainMenuViewV3::enter() {
    active = true;
    selected_index = 0;
    animation_time = millis();
    Serial.println("Entering main menu");
}

void MainMenuViewV3::exit() {
    active = false;
    Serial.println("Exiting main menu");
}

void MainMenuViewV3::update() {
    if (!active) return;
    
    uint32_t current_time = millis();
    if (current_time - last_update_time >= 100) { // 10FPS更新
        last_update_time = current_time;
    }
}

void MainMenuViewV3::render() {
    if (!active) {
        Serial.println("❌ Main menu view not active");
        return;
    }

    Serial.println("🎨 Rendering V3.0 main menu with Chinese items");
    display->clearBuffer();

    // 绘制菜单项 (不再绘制标题)
    renderMenuItems();

    // 不再绘制状态栏，保持界面简洁
    // renderStatusBar();

    display->sendBuffer();
    Serial.println("✅ Main menu rendering completed");
}

void MainMenuViewV3::renderMenuItems() {
    // 调整布局：4个菜单项从屏幕上方开始，确保所有文字完整显示
    // 从顶部留出适当边距，确保"系统设置"文字完整显示
    int start_y = 2;  // 从顶部16px开始，留出足够上边距
    int item_spacing = 15;  // 减少间距以适应屏幕高度，每项占12px

    Serial.printf("🎨 Rendering menu items, start_y=%d, spacing=%d\n", start_y, item_spacing);

    for (int i = 0; i < menu_items.size() && i < 4; i++) {
        int y = start_y + i * item_spacing;
        bool selected = (i == selected_index);

        Serial.printf("📝 Rendering Chinese menu item %d: '%s' (y=%d, selected=%s)\n",
                     i, menu_items[i].title.c_str(), y, selected ? "yes" : "no");

        // 使用闪烁效果绘制选中项，并水平居中
        drawMenuItemWithBlinkCentered(menu_items[i].title, y, selected);
    }
}

void MainMenuViewV3::drawMenuItemWithBlinkCentered(const String& text, int y, bool selected) {
    display->setFont(FONT_CHINESE_SMALL);  // 使用中文字体
    
    // 计算文字宽度以实现水平居中
    int text_width = display->getUTF8Width(text.c_str());
    int x = (128 - text_width) / 2;  // 水平居中
    
    // 确保不超出屏幕边界
    if (x < 2) x = 2;
    if (x + text_width > 126) x = 126 - text_width;

    if (selected) {
        // 文字闪烁效果：每500ms切换一次显示状态
        uint32_t current_time = millis();
        bool blink_state = (current_time / 500) % 2 == 0;

        if (blink_state) {
            // 显示文字（居中）
            display->drawUTF8(x, y, text.c_str());
        }
        // 不显示状态时什么都不画，实现闪烁效果
    } else {
        // 非选中项正常显示（居中）
        display->drawUTF8(x, y, text.c_str());
    }
}

void MainMenuViewV3::renderStatusBar() {
    // 绘制底部状态信息
    display->setFont(u8g2_font_5x7_tf);
    
    // 今日统计
    if (dataManagerV3.isInitialized()) {
        String status = "Today: " + String(dataManagerV3.getTotalJumpsToday()) + " jumps";
        display->drawUTF8(4, 62, status.c_str());

        // 目标进度
        float progress = dataManagerV3.getTodayTargetProgress();
        String progress_text = String((int)(progress * 100)) + "%";
        int width = display->getUTF8Width(progress_text.c_str());
        display->drawUTF8(124 - width, 62, progress_text.c_str());
    }
}

bool MainMenuViewV3::handleButton(button_event_t event) {
    if (!active) return false;
    
    switch (event) {
        case BUTTON_EVENT_SHORT_PRESS:
            // 短按：选择下一项
            updateSelection(1);
            return true;
            
        case BUTTON_EVENT_LONG_PRESS:
            // 长按：确认选择
            Serial.printf("选择菜单项: %s\n", menu_items[selected_index].title.c_str());
            return false; // 返回false让UI管理器处理视图切换
            
        default:
            return false;
    }
}

ui_view_t MainMenuViewV3::getSelectedView() const {
    if (selected_index >= 0 && selected_index < menu_items.size()) {
        return menu_items[selected_index].target_view;
    }
    return UI_VIEW_MAIN_MENU;
}

void MainMenuViewV3::updateSelection(int direction) {
    selected_index += direction;
    if (selected_index < 0) {
        selected_index = menu_items.size() - 1;
    } else if (selected_index >= menu_items.size()) {
        selected_index = 0;
    }
    
    Serial.printf("菜单选择: %d - %s\n", selected_index, menu_items[selected_index].title.c_str());
}

// DifficultySelectViewV3 实现
DifficultySelectViewV3::DifficultySelectViewV3(U8G2* disp) : 
    UIViewV3(disp), 
    selected_difficulty(DIFFICULTY_NORMAL),
    confirmed_difficulty(DIFFICULTY_NORMAL),
    animation_time(0),
    selection_confirmed(false) {
}

void DifficultySelectViewV3::enter() {
    active = true;
    selected_difficulty = DIFFICULTY_NORMAL;
    selection_confirmed = false;
    animation_time = millis();
    Serial.println("🎯 进入难度选择");
}

void DifficultySelectViewV3::exit() {
    active = false;
    Serial.println("🎯 退出难度选择");
}

void DifficultySelectViewV3::update() {
    if (!active) return;
    
    uint32_t current_time = millis();
    if (current_time - last_update_time >= 100) {
        last_update_time = current_time;
    }
}

void DifficultySelectViewV3::render() {
    if (!active) return;
    
    display->clearBuffer();
    
    if (selection_confirmed) {
        renderConfirmation();
    } else {
        // 绘制标题（上移12个单位）
        drawTitle("选择难度", 12 - 12);  // 12 - 12 = 0

        // 绘制难度选项
        renderDifficultyOptions();

        // 绘制难度详情
        renderDifficultyDetails();
    }
    
    display->sendBuffer();
}

void DifficultySelectViewV3::renderDifficultyOptions() {
    int start_y = 25 - 12;  // 上移12个单位：25 - 12 = 13
    int item_height = 11;

    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        int y = start_y + i * item_height;
        bool selected = (i == (int)selected_difficulty);

        const difficulty_config_t* config = V3Config::getDifficultyConfig((game_difficulty_t)i);
        // 使用中文难度名称
        String difficulty_name;
        switch((game_difficulty_t)i) {
            case DIFFICULTY_EASY: difficulty_name = "简单"; break;
            case DIFFICULTY_NORMAL: difficulty_name = "普通"; break;
            case DIFFICULTY_HARD: difficulty_name = "困难"; break;
            default: difficulty_name = "普通"; break;
        }
        String text = difficulty_name + " (" + String((int)(config->multiplier * 100)) + "%)";

        // 使用居中显示的菜单项
        display->setFont(FONT_CHINESE_SMALL);  // 使用中文字体
        int text_width = display->getUTF8Width(text.c_str());
        int x = (128 - text_width) / 2;  // 计算居中位置
        
        if (selected) {
            // 选中项闪烁效果
            uint32_t current_time = millis();
            bool blink_state = (current_time / 500) % 2 == 0;
            if (blink_state) {
                display->drawUTF8(x, y, text.c_str());
            }
        } else {
            // 非选中项正常显示
            display->drawUTF8(x, y, text.c_str());
        }
    }
}

void DifficultySelectViewV3::renderDifficultyDetails() {
    const difficulty_config_t* config = V3Config::getDifficultyConfig(selected_difficulty);

    display->setFont(FONT_CHINESE_TINY);  // 使用中文字体

    int detail_y = 52;  // 调整位置确保文字完整显示：64 - 8 = 56px（预留足够空间）
    String target_text = "目标: " + String(config->target_jumps) + " 次/" + String(config->target_time) + " 秒";
    drawCenteredText(target_text, detail_y);
}

void DifficultySelectViewV3::renderConfirmation() {
    drawTitle("难度已选择", 12 - 12);  // 上移12个单位：12 - 12 = 0

    const difficulty_config_t* config = V3Config::getDifficultyConfig(confirmed_difficulty);

    // 使用中文难度名称
    String difficulty_name;
    switch(confirmed_difficulty) {
        case DIFFICULTY_EASY: difficulty_name = "简单"; break;
        case DIFFICULTY_NORMAL: difficulty_name = "普通"; break;
        case DIFFICULTY_HARD: difficulty_name = "困难"; break;
        default: difficulty_name = "普通"; break;
    }

    display->setFont(FONT_CHINESE_SMALL);  // 使用中文字体
    drawCenteredText(difficulty_name, 30 - 12);  // 上移12个单位：30 - 12 = 18

    display->setFont(FONT_CHINESE_TINY);  // 使用中文字体
    drawCenteredText("准备运动...", 45 - 12);  // 上移12个单位：45 - 12 = 33

    // 绘制动画效果
    // 移除按键提示，按照用户要求去掉页面上的按键提示信息
    // 原来的按键提示代码已注释掉
    /*
    uint32_t elapsed = millis() - animation_time;
    if ((elapsed / 500) % 2 == 0) {
        drawCenteredText("按键开始", 58 - 12);  // 上移12个单位：58 - 12 = 46
    }
    */
}

bool DifficultySelectViewV3::handleButton(button_event_t event) {
    if (!active) return false;
    
    if (selection_confirmed) {
        // 已确认选择，任意按键开始游戏
        return false; // 让UI管理器处理
    }
    
    switch (event) {
        case BUTTON_EVENT_SHORT_PRESS:
            // 短按：切换难度
            updateSelection(1);
            return true;
            
        case BUTTON_EVENT_LONG_PRESS:
            // 长按：确认选择
            confirmSelection();
            return true;
            
        default:
            return false;
    }
}

void DifficultySelectViewV3::updateSelection(int direction) {
    int new_difficulty = (int)selected_difficulty + direction;
    if (new_difficulty < 0) {
        new_difficulty = DIFFICULTY_COUNT - 1;
    } else if (new_difficulty >= DIFFICULTY_COUNT) {
        new_difficulty = 0;
    }
    
    selected_difficulty = (game_difficulty_t)new_difficulty;
    
    const difficulty_config_t* config = V3Config::getDifficultyConfig(selected_difficulty);
    Serial.printf("Selected difficulty: %s\n", config->name_en);
}

void DifficultySelectViewV3::confirmSelection() {
    confirmed_difficulty = selected_difficulty;
    selection_confirmed = true;
    animation_time = millis();
    
    const difficulty_config_t* config = V3Config::getDifficultyConfig(confirmed_difficulty);
    Serial.printf("Confirmed difficulty: %s\n", config->name_en);
    
    // 设置V3.0难度
    if (dataManagerV3.isInitialized()) {
        SystemConfigV3 config_v3 = dataManagerV3.getSystemConfig();
        config_v3.default_difficulty = confirmed_difficulty;
        dataManagerV3.saveSystemConfig(config_v3);
    }
}

// HistoryViewV3 类已移除 - 简化版本不包含历史统计功能

// SettingsViewV3 实现
SettingsViewV3::SettingsViewV3(U8G2* disp) :
    UIViewV3(disp), selected_item(0), editing_mode(false), edit_start_time(0) {
}

void SettingsViewV3::enter() {
    active = true;
    selected_item = 0;
    editing_mode = false;
    display_start_index = 0;  // 初始化轮播显示起始索引
    loadConfig();
    loadTargetSettings();
    Serial.println("⚙️ 进入系统设置");
}

void SettingsViewV3::exit() {
    active = false;
    saveConfig();
    saveTargetSettings();
    Serial.println("⚙️ 退出系统设置");
}

void SettingsViewV3::update() {
    if (!active) return;

    uint32_t current_time = millis();
    if (current_time - last_update_time >= 100) {
        last_update_time = current_time;
    }

    // 编辑模式超时检查
    if (editing_mode && current_time - edit_start_time >= 5000) {
        editing_mode = false;
        Serial.println("编辑模式超时，自动退出");
    }
}

void SettingsViewV3::render() {
    if (!active) return;

    display->clearBuffer();

    drawTitle("设置", 6);  // 进一步上移到Y=6，为设置项预留更多空间
    renderSettingItems();

    // 移除编辑提示，简化界面

    display->sendBuffer();
}

void SettingsViewV3::loadConfig() {
    if (dataManagerV3.isInitialized()) {
        config = dataManagerV3.getSystemConfig();
    } else {
        config.resetToDefault();
    }
}

void SettingsViewV3::saveConfig() {
    if (dataManagerV3.isInitialized()) {
        dataManagerV3.saveSystemConfig(config);
        Serial.println("✅ 设置已保存");
    }
}

void SettingsViewV3::loadTargetSettings() {
    if (dataManagerV3.isInitialized()) {
        target_settings = dataManagerV3.getTargetSettings();
    } else {
        target_settings = TargetSettingsV3(); // 使用默认值
    }
}

void SettingsViewV3::saveTargetSettings() {
    if (dataManagerV3.isInitialized()) {
        dataManagerV3.saveTargetSettings(target_settings);
        Serial.println("✅ 目标设置已保存");
    }
}

void SettingsViewV3::renderSettingItems() {
    // 轮播显示逻辑：
    // 标题: Y=6, 占用到Y=13
    // 可用空间: Y=16 到 Y=56 = 40px
    // 每项高度: 8px (u8g2_font_6x10_tf字体 + 间距)
    // 最多显示: 5项 (40px / 8px = 5)
    int start_y = 16;
    int item_height = 8;

    // 只渲染当前窗口内的设置项
    int end_index = min(display_start_index + MAX_VISIBLE_ITEMS, (int)SETTING_COUNT);

    for (int i = display_start_index; i < end_index; i++) {
        int display_index = i - display_start_index;  // 在屏幕上的相对位置
        int y = start_y + display_index * item_height;
        bool selected = (i == selected_item);

        String item_text = getSettingName(i) + ": " + getSettingValue(i);

        display->setFont(FONT_CHINESE_SMALL);  // 使用中文字体保证可读性

        if (selected) {
            if (editing_mode) {
                // 编辑模式：文字闪烁显示
                uint32_t elapsed = millis() - edit_start_time;
                if ((elapsed / 300) % 2 == 0) {
                    display->drawUTF8(4, y, item_text.c_str());
                }
                // 不显示状态时什么都不画，实现闪烁效果
            } else {
                // 选中但未编辑：文字闪烁显示
                uint32_t current_time = millis();
                bool blink_state = (current_time / 500) % 2 == 0;
                if (blink_state) {
                    display->drawUTF8(4, y, item_text.c_str());
                }
                // 不显示状态时什么都不画，实现闪烁效果
            }
        } else {
            // 非选中项正常显示
            display->drawUTF8(4, y, item_text.c_str());
        }
    }

    // 绘制滚动指示器
    renderScrollIndicator();
}

void SettingsViewV3::renderEditIndicator() {
    // 移除按键提示，按照用户要求去掉页面上的按键提示信息
    // 原来的编辑模式提示代码已注释掉
    /*
    display->setFont(FONT_CHINESE_TINY);  // 使用中文字体
    drawCenteredText("编辑模式 - 按键调整", 58);
    */
    
    // 简化的编辑模式指示器，只显示闪烁效果不显示文字
    uint32_t blink_cycle = millis() % 800;
    if ((blink_cycle / 400) % 2 == 0) {
        display->setFont(FONT_CHINESE_TINY);
        drawCenteredText("编辑中", 58);
    }
}

void SettingsViewV3::renderScrollIndicator() {
    // 只有当设置项总数超过可显示数量时才显示滚动指示器
    if ((int)SETTING_COUNT <= MAX_VISIBLE_ITEMS) {
        return;
    }

    // 在屏幕右侧绘制滚动条
    int scroll_x = 124;  // 滚动条X位置
    int scroll_y_start = 16;  // 滚动条起始Y位置
    int scroll_height = 40;   // 滚动条总高度

    // 计算滚动条位置和大小
    float scroll_ratio = (float)display_start_index / ((int)SETTING_COUNT - MAX_VISIBLE_ITEMS);
    int indicator_height = max(2, scroll_height * MAX_VISIBLE_ITEMS / (int)SETTING_COUNT);
    int indicator_y = scroll_y_start + (scroll_height - indicator_height) * scroll_ratio;

    // 绘制滚动条背景
    display->drawVLine(scroll_x, scroll_y_start, scroll_height);

    // 绘制滚动指示器
    for (int i = 0; i < indicator_height; i++) {
        display->drawPixel(scroll_x - 1, indicator_y + i);
        display->drawPixel(scroll_x + 1, indicator_y + i);
    }
}

String SettingsViewV3::getSettingName(int index) {
    switch (index) {
        case SETTING_VOLUME: return "音量";
        case SETTING_DIFFICULTY: return "难度";
        case SETTING_SOUND_ENABLED: return "声音";
        case SETTING_TARGET_ENABLED: return "目标";
        case SETTING_TARGET_JUMPS: return "跳跃数";
        case SETTING_TARGET_TIME: return "时间";
        case SETTING_TARGET_CALORIES: return "卡路里";
        case SETTING_RESET_DATA: return "重置";
        case SETTING_BACK: return "返回";
        default: return "未知";
    }
}

String SettingsViewV3::getSettingValue(int index) {
    switch (index) {
        case SETTING_VOLUME:
            return String(config.volume) + "%";
        case SETTING_DIFFICULTY:
            return V3Config::getDifficultyName(config.default_difficulty);
        case SETTING_SOUND_ENABLED:
            return config.sound_enabled ? "开" : "关";
        case SETTING_TARGET_ENABLED:
            return target_settings.enabled ? "开" : "关";
        case SETTING_TARGET_JUMPS:
            return String(target_settings.target_jumps);
        case SETTING_TARGET_TIME:
            return String(target_settings.target_time) + " 秒";
        case SETTING_TARGET_CALORIES:
            return String((int)target_settings.target_calories);
        case SETTING_RESET_DATA:
            return "执行";
        case SETTING_BACK:
            return "";
        default:
            return "";
    }
}

bool SettingsViewV3::handleButton(button_event_t event) {
    if (!active) return false;

    switch (event) {
        case BUTTON_EVENT_SHORT_PRESS:
            if (editing_mode) {
                // 编辑模式：调整数值
                adjustValue(1);
            } else {
                // 普通模式：移动选择
                updateSelection(1);
            }
            return true;

        case BUTTON_EVENT_LONG_PRESS:
            if (selected_item == SETTING_BACK) {
                // 返回主菜单
                return false;
            } else if (selected_item == SETTING_RESET_DATA) {
                // 重置数据
                Serial.println("⚠️ 重置数据功能暂未实现");
                return true;
            } else {
                // 切换编辑模式
                toggleEditMode();
            }
            return true;

        default:
            return false;
    }
}

void SettingsViewV3::updateSelection(int direction) {
    selected_item += direction;
    if (selected_item < 0) {
        selected_item = SETTING_COUNT - 1;
    } else if (selected_item >= SETTING_COUNT) {
        selected_item = 0;
    }

    // 更新轮播显示窗口
    updateDisplayWindow();

    Serial.printf("设置选择: %s\n", getSettingName(selected_item).c_str());
}

void SettingsViewV3::updateDisplayWindow() {
    // 确保选中项在可见范围内
    if (selected_item < display_start_index) {
        // 选中项在当前窗口上方，向上滚动
        display_start_index = selected_item;
    } else if (selected_item >= display_start_index + MAX_VISIBLE_ITEMS) {
        // 选中项在当前窗口下方，向下滚动
        display_start_index = selected_item - MAX_VISIBLE_ITEMS + 1;
    }

    // 确保显示窗口不超出范围
    if (display_start_index < 0) {
        display_start_index = 0;
    } else if (display_start_index > (int)SETTING_COUNT - MAX_VISIBLE_ITEMS) {
        display_start_index = max(0, (int)SETTING_COUNT - MAX_VISIBLE_ITEMS);
    }
}

void SettingsViewV3::toggleEditMode() {
    if (selected_item == SETTING_BACK || selected_item == SETTING_RESET_DATA) {
        return; // 这些项目不能编辑
    }

    editing_mode = !editing_mode;
    if (editing_mode) {
        edit_start_time = millis();
        Serial.printf("Start editing: %s\n", getSettingName(selected_item).c_str());
    } else {
        Serial.printf("End editing: %s\n", getSettingName(selected_item).c_str());
        saveConfig();
        saveTargetSettings();
    }
}

void SettingsViewV3::adjustValue(int direction) {
    switch (selected_item) {
        case SETTING_VOLUME:
            config.volume += direction * 10;
            if (config.volume > 100) config.volume = 100;
            if (config.volume < 0) config.volume = 0;
            Serial.printf("Volume adjusted: %d%%\n", config.volume);
            break;

        case SETTING_DIFFICULTY:
            {
                int new_diff = (int)config.default_difficulty + direction;
                if (new_diff < 0) new_diff = DIFFICULTY_COUNT - 1;
                if (new_diff >= DIFFICULTY_COUNT) new_diff = 0;
                config.default_difficulty = (game_difficulty_t)new_diff;
                Serial.printf("Default difficulty: %s\n", V3Config::getDifficultyName(config.default_difficulty));
            }
            break;

        case SETTING_SOUND_ENABLED:
            config.sound_enabled = !config.sound_enabled;
            Serial.printf("Sound: %s\n", config.sound_enabled ? "On" : "Off");
            break;

        // 删除了目标相关设置项 - 简化版本不包含历史统计功能
    }
}

// TargetTimerViewV3 简化实现（暂时）
TargetTimerViewV3::TargetTimerViewV3(U8G2* disp) :
    UIViewV3(disp), timer_start_time(0), target_duration(0), timer_active(false), target_achieved(false) {
}

void TargetTimerViewV3::enter() {
    active = true;
    loadTargetSettings();
    Serial.println("⏰ 进入目标计时");
}

void TargetTimerViewV3::exit() {
    active = false;
    Serial.println("⏰ 退出目标计时");
}

void TargetTimerViewV3::update() {
    if (!active) return;

    if (timer_active) {
        checkTargetAchievement();
    }
}

void TargetTimerViewV3::render() {
    if (!active) return;

    display->clearBuffer();

    // 检查是否需要屏幕闪烁效果
    if (is_target_flash_active() && !should_screen_flash_now()) {
        // 闪烁状态：显示空白屏幕
        display->sendBuffer();
        return;
    }

    drawTitle("目标计时", 12);

    renderTargetInfo();
    renderTimer();
    renderProgress();

    display->sendBuffer();
}

bool TargetTimerViewV3::handleButton(button_event_t event) {
    if (!active) return false;

    switch (event) {
        case BUTTON_EVENT_SHORT_PRESS:
            if (timer_active) {
                stopTimer();
            } else {
                startTimer();
            }
            return true;

        case BUTTON_EVENT_LONG_PRESS:
            return false; // 返回主菜单

        default:
            return false;
    }
}

void TargetTimerViewV3::loadTargetSettings() {
    if (dataManagerV3.isInitialized()) {
        target_settings = dataManagerV3.getTargetSettings();
    }
    target_duration = target_settings.target_time;
}

void TargetTimerViewV3::startTimer() {
    timer_start_time = millis();
    timer_active = true;
    target_achieved = false;
    Serial.println("⏰ 计时器启动");
}

void TargetTimerViewV3::stopTimer() {
    timer_active = false;
    Serial.println("⏰ 计时器停止");
}

void TargetTimerViewV3::checkTargetAchievement() {
    if (!timer_active) return;

    uint32_t elapsed = (millis() - timer_start_time) / 1000;
    if (elapsed >= target_duration) {
        target_achieved = true;
        timer_active = false;
        Serial.println("🎉 目标达成！");

        // 启动屏幕闪烁效果和音效
        start_target_achievement_flash();
        play_sound_effect(SOUND_TARGET_ACHIEVED);
    }
}

void TargetTimerViewV3::renderTargetInfo() {
    display->setFont(FONT_CHINESE_SMALL);  // 使用中文字体
    String target_text = "目标: " + String(target_duration) + " 秒";
    drawCenteredText(target_text, 25);
}

void TargetTimerViewV3::renderTimer() {
    if (timer_active) {
        uint32_t elapsed = (millis() - timer_start_time) / 1000;
        String time_text = DataUtilsV3::formatTime(elapsed);

        display->setFont(FONT_CHINESE_LARGE);  // 使用中文大字体
        drawCenteredText(time_text, 45);
    } else {
        display->setFont(FONT_CHINESE_SMALL);  // 使用中文字体
        // 移除按键提示，只保留目标达成信息
        if (target_achieved) {
            drawCenteredText("目标达成!", 45);
        }
        // 移除"按键开始"提示，按照用户要求去掉页面上的按键提示信息
    }
}

void TargetTimerViewV3::renderProgress() {
    if (timer_active && target_duration > 0) {
        uint32_t elapsed = (millis() - timer_start_time) / 1000;
        float progress = (float)elapsed / target_duration;
        if (progress > 1.0f) progress = 1.0f;

        drawProgressBar(14, 55, 100, 6, progress);
    }
}

// UIManagerV3 实现
UIManagerV3::UIManagerV3(U8G2* disp) :
    display(disp),
    current_view(UI_VIEW_MAIN_MENU),
    previous_view(UI_VIEW_MAIN_MENU),
    main_menu(nullptr),
    difficulty_select(nullptr),
    settings_view(nullptr),
    target_timer(nullptr),
    current_view_instance(nullptr) {
}

UIManagerV3::~UIManagerV3() {
    deinit();
}

bool UIManagerV3::init() {
    Serial.println("🎨 初始化V3.0 UI管理器...");

    if (!display) {
        Serial.println("❌ 显示器指针为空");
        return false;
    }

    // 创建视图实例
    main_menu = new MainMenuViewV3(display);
    difficulty_select = new DifficultySelectViewV3(display);
    settings_view = new SettingsViewV3(display);
    target_timer = new TargetTimerViewV3(display);

    if (!main_menu || !difficulty_select || !settings_view || !target_timer) {
        Serial.println("❌ UI视图创建失败");
        return false;
    }

    // 设置初始视图
    current_view_instance = main_menu;
    current_view_instance->enter();

    Serial.println("✅ V3.0 UI管理器初始化成功");
    return true;
}

void UIManagerV3::deinit() {
    if (current_view_instance) {
        current_view_instance->exit();
        current_view_instance = nullptr;
    }

    delete main_menu;
    delete difficulty_select;
    delete settings_view;
    delete target_timer;

    main_menu = nullptr;
    difficulty_select = nullptr;
    settings_view = nullptr;
    target_timer = nullptr;

    Serial.println("UI管理器已清理");
}

void UIManagerV3::update() {
    if (current_view_instance) {
        current_view_instance->update();
    }
}

void UIManagerV3::render() {
    if (current_view_instance) {
        current_view_instance->render();
    }
}

bool UIManagerV3::handleButton(button_event_t event) {
    if (!current_view_instance) return false;

    // 先让当前视图处理按钮事件
    if (current_view_instance->handleButton(event)) {
        return true; // 视图处理了事件
    }

    // 视图没有处理事件，UI管理器处理视图切换
    switch (current_view) {
        case UI_VIEW_MAIN_MENU:
            if (event == BUTTON_EVENT_LONG_PRESS) {
                ui_view_t target_view = main_menu->getSelectedView();
                switchToView(target_view);
                return true;
            }
            break;

        case UI_VIEW_DIFFICULTY_SELECT:
            if (difficulty_select->isSelectionConfirmed()) {
                // 难度已确认，开始游戏
                Serial.println("🎮 启动游戏...");
                // 这里应该切换到游戏状态，暂时返回主菜单
                switchToView(UI_VIEW_MAIN_MENU);
                return true;
            } else if (event == BUTTON_EVENT_LONG_PRESS) {
                // 取消选择，返回主菜单
                switchToView(UI_VIEW_MAIN_MENU);
                return true;
            }
            break;

        case UI_VIEW_SETTINGS:
        case UI_VIEW_TARGET_TIMER:
            if (event == BUTTON_EVENT_LONG_PRESS) {
                // 返回主菜单
                switchToView(UI_VIEW_MAIN_MENU);
                return true;
            }
            break;

        default:
            break;
    }

    return false;
}

void UIManagerV3::switchToView(ui_view_t view) {
    if (view == current_view) return;

    UIViewV3* new_view_instance = getViewInstance(view);
    if (!new_view_instance) {
        Serial.printf("❌ 无效的视图: %d\n", view);
        return;
    }

    // 退出当前视图
    if (current_view_instance) {
        current_view_instance->exit();
    }

    // 切换到新视图
    previous_view = current_view;
    current_view = view;
    current_view_instance = new_view_instance;
    current_view_instance->enter();

    Serial.printf("🎨 视图切换: %d -> %d\n", previous_view, current_view);
}

UIViewV3* UIManagerV3::getViewInstance(ui_view_t view) {
    switch (view) {
        case UI_VIEW_MAIN_MENU: return main_menu;
        case UI_VIEW_DIFFICULTY_SELECT: return difficulty_select;
        case UI_VIEW_SETTINGS: return settings_view;
        case UI_VIEW_TARGET_TIMER: return target_timer;
        default: return nullptr;
    }
}

game_difficulty_t UIManagerV3::getSelectedDifficulty() {
    if (difficulty_select) {
        return difficulty_select->getSelectedDifficulty();
    }
    return DIFFICULTY_NORMAL;
}

bool UIManagerV3::isDifficultyConfirmed() {
    if (difficulty_select) {
        return difficulty_select->isSelectionConfirmed();
    }
    return false;
}

void UIManagerV3::resetDifficultySelection() {
    if (difficulty_select) {
        difficulty_select->resetSelection();
    }
}

// 全局函数实现
bool initUIManagerV3(U8G2* display) {
    if (uiManagerV3) {
        Serial.println("⚠️ UI管理器已经初始化");
        return true;
    }

    uiManagerV3 = new UIManagerV3(display);
    if (!uiManagerV3) {
        Serial.println("❌ UI管理器创建失败");
        return false;
    }

    return uiManagerV3->init();
}

void deinitUIManagerV3() {
    if (uiManagerV3) {
        delete uiManagerV3;
        uiManagerV3 = nullptr;
    }
}

// 注意：updateUIV3, renderUIV3, handleUIButtonV3 函数已在 game_integration_v3.cpp 中定义
