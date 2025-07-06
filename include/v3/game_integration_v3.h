#ifndef GAME_INTEGRATION_V3_H
#define GAME_INTEGRATION_V3_H

#include <Arduino.h>
#include "board_config_v3.h"
#include "data_models_v3.h"

// 确保包含V2.0的类型定义
#ifndef JUMPING_ROCKET_SIMPLE_H
#include "jumping_rocket_simple.h"
#endif

// V3.0游戏状态枚举
typedef enum {
    V3_GAME_DISABLED = 0,   // V3.0功能禁用
    V3_GAME_IDLE,           // 空闲状态
    V3_GAME_UI_MODE,        // UI模式（菜单导航）
    V3_GAME_PLAYING,        // 游戏进行中
    V3_GAME_PAUSED,         // 游戏暂停
    V3_GAME_RESULT,         // 游戏结果
    V3_GAME_OTHER           // 其他状态
} V3GameState;

// V3.0游戏集成功能

/**
 * 初始化V3.0游戏集成系统
 * @return 成功返回true，失败返回false
 */
bool initGameIntegrationV3();

/**
 * 检查是否应该进入V3.0 UI模式
 * @return 如果应该进入UI模式返回true
 */
bool shouldEnterV3UIMode();

/**
 * 进入V3.0 UI模式（菜单导航）
 */
void enterV3UIMode();

/**
 * 退出V3.0 UI模式
 */
void exitV3UIMode();

/**
 * 检查是否在V3.0 UI模式
 * @return 如果在UI模式返回true
 */
bool isInV3UIMode();

/**
 * 更新V3.0 UI模式
 * 在主循环中调用，处理UI逻辑和状态转换
 */
void updateV3UIMode();

/**
 * 渲染V3.0 UI模式
 * 在显示任务中调用，绘制UI界面
 */
void renderV3UIMode();

/**
 * 处理V3.0 UI模式的按钮事件
 * @param event 按钮事件
 * @return 如果事件被处理返回true
 */
bool handleV3UIButton(button_event_t event);

/**
 * 开始V3.0游戏
 * 从UI模式切换到游戏模式
 */
void startV3Game();

/**
 * V3.0游戏结束处理
 * 保存游戏数据，更新统计信息
 */
void onV3GameComplete();

/**
 * 获取当前V3.0游戏状态
 * @return V3.0游戏状态
 */
V3GameState getV3GameState();

/**
 * 打印V3.0游戏统计信息
 */
void printV3GameStats();

/**
 * 检查V3.0兼容性
 * @return 如果系统兼容V3.0功能返回true
 */
bool checkV3Compatibility();

/**
 * 从V2.0迁移数据到V3.0
 * @return 迁移成功返回true
 */
bool migrateFromV2ToV3();

/**
 * 生成V3.0系统状态报告
 */
void reportV3SystemStatus();

// V3.0与V2.0集成的回调函数

/**
 * V2.0游戏开始时的V3.0处理
 * @param difficulty 游戏难度
 */
void onV2GameStart(game_difficulty_t difficulty);

/**
 * V2.0游戏暂停时的V3.0处理
 */
void onV2GamePause();

/**
 * V2.0游戏恢复时的V3.0处理
 */
void onV2GameResume();

/**
 * V2.0游戏重置时的V3.0处理
 */
void onV2GameReset();

/**
 * V2.0跳跃检测时的V3.0处理
 * @param jump_count 当前跳跃次数
 * @param game_time 游戏时间（毫秒）
 */
void onV2JumpDetected(uint32_t jump_count, uint32_t game_time);

// V3.0数据查询接口

/**
 * 获取今日跳跃总数
 * @return 今日跳跃次数
 */
uint32_t getV3TotalJumpsToday();

/**
 * 获取今日卡路里总数
 * @return 今日消耗卡路里
 */
float getV3TotalCaloriesToday();

/**
 * 获取今日游戏次数
 * @return 今日游戏次数
 */
uint32_t getV3TotalGamesToday();

/**
 * 获取今日目标完成进度
 * @return 进度百分比（0.0-1.0）
 */
float getV3TodayTargetProgress();

/**
 * 检查今日目标是否达成
 * @return 如果目标达成返回true
 */
bool isV3TodayTargetAchieved();

/**
 * 获取历史最佳得分
 * @return 历史最佳得分
 */
uint16_t getV3BestScore();

/**
 * 获取历史最多跳跃次数
 * @return 历史最多跳跃次数
 */
uint32_t getV3BestJumps();

// V3.0配置管理接口

/**
 * 获取V3.0系统配置
 * @return 系统配置结构
 */
SystemConfigV3 getV3SystemConfig();

/**
 * 保存V3.0系统配置
 * @param config 系统配置
 * @return 保存成功返回true
 */
bool saveV3SystemConfig(const SystemConfigV3& config);

/**
 * 获取V3.0目标设置
 * @return 目标设置结构
 */
TargetSettingsV3 getV3TargetSettings();

/**
 * 保存V3.0目标设置
 * @param settings 目标设置
 * @return 保存成功返回true
 */
bool saveV3TargetSettings(const TargetSettingsV3& settings);

// V3.0调试和诊断接口

/**
 * 启用V3.0调试模式
 * @param enable 是否启用
 */
void enableV3Debug(bool enable);

/**
 * 获取V3.0内存使用情况
 * @return 内存使用字节数
 */
size_t getV3MemoryUsage();

/**
 * 获取V3.0文件系统使用情况
 * @return 文件系统使用百分比
 */
float getV3FileSystemUsage();

/**
 * 执行V3.0系统自检
 * @return 自检通过返回true
 */
bool runV3SystemCheck();

/**
 * 重置V3.0所有数据
 * @param confirm 确认重置（防止误操作）
 * @return 重置成功返回true
 */
bool resetV3AllData(bool confirm);

/**
 * 导出V3.0数据为JSON格式
 * @return JSON格式的数据字符串
 */
String exportV3DataAsJson();

/**
 * 从JSON格式导入V3.0数据
 * @param json_data JSON格式的数据字符串
 * @return 导入成功返回true
 */
bool importV3DataFromJson(const String& json_data);

// V3.0事件处理宏定义

#ifdef JUMPING_ROCKET_V3
    // V3.0功能启用时的事件处理
    #define V3_ON_GAME_START(difficulty) onV2GameStart(difficulty)
    #define V3_ON_GAME_COMPLETE() onV3GameComplete()
    #define V3_ON_JUMP_DETECTED(count, time) onV2JumpDetected(count, time)
    #define V3_ON_GAME_PAUSE() onV2GamePause()
    #define V3_ON_GAME_RESUME() onV2GameResume()
    #define V3_ON_GAME_RESET() onV2GameReset()
    
    #define V3_UPDATE_UI() updateV3UIMode()
    #define V3_RENDER_UI() renderV3UIMode()
    #define V3_HANDLE_BUTTON(event) handleV3UIButton(event)
    
    #define V3_SHOULD_ENTER_UI() shouldEnterV3UIMode()
    #define V3_ENTER_UI() enterV3UIMode()
    #define V3_EXIT_UI() exitV3UIMode()
    #define V3_IS_IN_UI() isInV3UIMode()
#else
    // V3.0功能禁用时的空宏
    #define V3_ON_GAME_START(difficulty) do {} while(0)
    #define V3_ON_GAME_COMPLETE() do {} while(0)
    #define V3_ON_JUMP_DETECTED(count, time) do {} while(0)
    #define V3_ON_GAME_PAUSE() do {} while(0)
    #define V3_ON_GAME_RESUME() do {} while(0)
    #define V3_ON_GAME_RESET() do {} while(0)
    
    #define V3_UPDATE_UI() do {} while(0)
    #define V3_RENDER_UI() do {} while(0)
    #define V3_HANDLE_BUTTON(event) false
    
    #define V3_SHOULD_ENTER_UI() false
    #define V3_ENTER_UI() do {} while(0)
    #define V3_EXIT_UI() do {} while(0)
    #define V3_IS_IN_UI() false
#endif

#endif // GAME_INTEGRATION_V3_H
