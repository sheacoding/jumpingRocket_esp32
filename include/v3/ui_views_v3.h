#ifndef UI_VIEWS_V3_H
#define UI_VIEWS_V3_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "board_config_v3.h"
#include "data_models_v3.h"

// UI视图状态枚举
typedef enum {
    UI_VIEW_MAIN_MENU = 0,      // 主菜单
    UI_VIEW_DIFFICULTY_SELECT,  // 难度选择
    UI_VIEW_SETTINGS,          // 系统设置
    UI_VIEW_TARGET_TIMER,      // 目标计时
    UI_VIEW_COUNT
} ui_view_t;

// 菜单项结构
struct MenuItemV3 {
    String title;
    String description;
    ui_view_t target_view;
    bool enabled;
    
    MenuItemV3(const String& t, const String& d, ui_view_t tv, bool e = true) :
        title(t), description(d), target_view(tv), enabled(e) {}
};

// UI视图基类
class UIViewV3 {
protected:
    U8G2* display;
    bool active;
    uint32_t last_update_time;
    
public:
    UIViewV3(U8G2* disp) : display(disp), active(false), last_update_time(0) {}
    virtual ~UIViewV3() {}
    
    // 纯虚函数，子类必须实现
    virtual void enter() = 0;
    virtual void exit() = 0;
    virtual void update() = 0;
    virtual void render() = 0;
    virtual bool handleButton(button_event_t event) = 0;
    
    // 通用方法
    bool isActive() const { return active; }
    void setActive(bool state) { active = state; }
    
protected:
    // 通用绘制方法
    void drawTitle(const String& title, int y = 12);
    void drawMenuItem(const String& text, int y, bool selected = false);
    void drawProgressBar(int x, int y, int width, int height, float progress);
    void drawValue(const String& label, const String& value, int y);
    void drawCenteredText(const String& text, int y);
};

// 主菜单视图
class MainMenuViewV3 : public UIViewV3 {
private:
    std::vector<MenuItemV3> menu_items;
    int selected_index;
    uint32_t animation_time;
    
public:
    MainMenuViewV3(U8G2* disp);
    
    void enter() override;
    void exit() override;
    void update() override;
    void render() override;
    bool handleButton(button_event_t event) override;
    
    ui_view_t getSelectedView() const;
    
private:
    void initMenuItems();
    void updateSelection(int direction);
    void renderMenuItems();
    void renderStatusBar();
    void drawMenuItemWithBlinkCentered(const String& text, int y, bool selected);
};

// 难度选择视图
class DifficultySelectViewV3 : public UIViewV3 {
private:
    game_difficulty_t selected_difficulty;
    game_difficulty_t confirmed_difficulty;
    uint32_t animation_time;
    bool selection_confirmed;
    
public:
    DifficultySelectViewV3(U8G2* disp);
    
    void enter() override;
    void exit() override;
    void update() override;
    void render() override;
    bool handleButton(button_event_t event) override;
    
    game_difficulty_t getSelectedDifficulty() const { return confirmed_difficulty; }
    bool isSelectionConfirmed() const { return selection_confirmed; }
    void resetSelection() { selection_confirmed = false; }
    
private:
    void updateSelection(int direction);
    void confirmSelection();
    void renderDifficultyOptions();
    void renderDifficultyDetails();
    void renderConfirmation();
};

// 历史数据视图类已移除 - 简化版本不包含历史统计功能

// 设置视图
class SettingsViewV3 : public UIViewV3 {
private:
    SystemConfigV3 config;
    TargetSettingsV3 target_settings;
    int selected_item;
    bool editing_mode;
    uint32_t edit_start_time;

    // 轮播显示相关
    int display_start_index;  // 当前显示的第一个设置项索引
    static const int MAX_VISIBLE_ITEMS = 5;  // 屏幕最多显示5个设置项

    enum SettingItem {
        SETTING_VOLUME = 0,
        SETTING_DIFFICULTY,
        SETTING_SOUND_ENABLED,
        SETTING_BACK,
        SETTING_COUNT
    };
    
public:
    SettingsViewV3(U8G2* disp);
    
    void enter() override;
    void exit() override;
    void update() override;
    void render() override;
    bool handleButton(button_event_t event) override;
    
private:
    void loadConfig();
    void saveConfig();
    void loadTargetSettings();
    void saveTargetSettings();
    void updateSelection(int direction);
    void toggleEditMode();
    void adjustValue(int direction);
    void renderSettingItems();
    void renderEditIndicator();
    void renderScrollIndicator();  // 渲染滚动指示器
    void updateDisplayWindow();  // 更新轮播显示窗口
    String getSettingName(int index);
    String getSettingValue(int index);
};

// 目标计时视图
class TargetTimerViewV3 : public UIViewV3 {
private:
    TargetSettingsV3 target_settings;
    uint32_t timer_start_time;
    uint32_t target_duration;
    bool timer_active;
    bool target_achieved;
    
public:
    TargetTimerViewV3(U8G2* disp);
    
    void enter() override;
    void exit() override;
    void update() override;
    void render() override;
    bool handleButton(button_event_t event) override;
    
    bool isTimerActive() const { return timer_active; }
    bool isTargetAchieved() const { return target_achieved; }
    
private:
    void loadTargetSettings();
    void startTimer();
    void stopTimer();
    void checkTargetAchievement();
    void renderTimer();
    void renderProgress();
    void renderTargetInfo();
};

// UI管理器
class UIManagerV3 {
private:
    U8G2* display;
    ui_view_t current_view;
    ui_view_t previous_view;
    
    // 视图实例
    MainMenuViewV3* main_menu;
    DifficultySelectViewV3* difficulty_select;
    SettingsViewV3* settings_view;
    TargetTimerViewV3* target_timer;
    
    UIViewV3* current_view_instance;
    
public:
    UIManagerV3(U8G2* disp);
    ~UIManagerV3();
    
    bool init();
    void deinit();
    
    void update();
    void render();
    bool handleButton(button_event_t event);
    
    void switchToView(ui_view_t view);
    ui_view_t getCurrentView() const { return current_view; }
    ui_view_t getPreviousView() const { return previous_view; }
    
    // 获取特定视图的状态
    game_difficulty_t getSelectedDifficulty();
    bool isDifficultyConfirmed();
    void resetDifficultySelection();
    
private:
    UIViewV3* getViewInstance(ui_view_t view);
    void switchViewInstance(UIViewV3* new_view);
};

// 全局UI管理器实例声明
extern UIManagerV3* uiManagerV3;

// 初始化和管理函数
bool initUIManagerV3(U8G2* display);
void deinitUIManagerV3();
void updateUIV3();
void renderUIV3();
bool handleUIButtonV3(button_event_t event);

#endif // UI_VIEWS_V3_H
