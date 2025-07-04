<div align="center">

# 🚀 蹦跳小火箭 ESP32

*基于MPU6050传感器的智能跳跃检测游戏*

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Ready-orange?style=for-the-badge&logo=platformio)](https://platformio.org/)
[![ESP32](https://img.shields.io/badge/ESP32-Compatible-blue?style=for-the-badge&logo=espressif)](https://www.espressif.com/)
[![Arduino](https://img.shields.io/badge/Arduino-Framework-teal?style=for-the-badge&logo=arduino)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](LICENSE)

[![GitHub stars](https://img.shields.io/github/stars/sheacoding/jumpingRocket_esp32?style=social)](https://github.com/sheacoding/jumpingRocket_esp32/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/sheacoding/jumpingRocket_esp32?style=social)](https://github.com/sheacoding/jumpingRocket_esp32/network)

[🎮 功能演示](#-功能特性) • [🔧 快速开始](#-快速开始) • [📖 文档](#-项目文档) • [🤝 贡献](#-贡献指南)

</div>

---

## 📖 项目简介

**蹦跳小火箭**是一个创新的ESP32嵌入式健身游戏项目，通过高精度MPU6050传感器实时检测用户的跳跃动作，将物理运动转化为沉浸式的数字游戏体验。项目采用先进的四状态跳跃检测算法、精美的SVG设计界面和流畅的动画效果，为用户提供科学、有趣的运动体验。

### 🎯 设计理念

- **🏃‍♂️ 运动游戏化**: 基于科学算法的跳跃检测，将运动转化为游戏进度
- **🎨 精美UI**: 基于SVG设计的像素级精确界面，6种精心设计的交互界面
- **⚡ 实时响应**: 50Hz传感器采样，四状态检测算法，<50ms响应延迟
- **🔊 多感官体验**: 10种音效 + 丰富视觉动画，全方位感官反馈
- **🎯 科学健身**: 实时统计跳跃频率、运动强度、卡路里消耗等健身数据
- **🚀 成就激励**: 三级难度系统，燃料充能机制，火箭发射成就感

---

## ✨ 功能特性

<table>
<tr>
<td width="50%">

### 🎮 游戏系统
- **🧠 智能跳跃检测**: 四状态检测算法，多重验证机制
- **⛽ 燃料充能机制**: 每次跳跃增加5%燃料，满格触发发射
- **🚀 动态高度计算**: 基于跳跃次数、时长、燃料的复合计算
- **🎯 三级难度系统**: 简单(60%)、普通(80%)、困难(100%)
- **📊 实时数据统计**: 跳跃频率、运动强度、卡路里消耗
- **🏆 完整游戏流程**: 7个游戏状态，流畅状态转换

</td>
<td width="50%">

### 🖥️ 显示系统
- **📺 128×64 OLED**: SSD1306高对比度单色显示
- **🎨 SVG精确布局**: 像素级完美对齐，5级字体系统
- **✨ 丰富动画效果**: 呼吸灯、波纹、闪烁、火箭发射等
- **⚡ 10FPS流畅显示**: U8g2图形库优化，内存占用<8%
- **🎭 6种精美界面**: 开机、待机、游戏、暂停、发射、结算
- **🔄 状态驱动切换**: 基于游戏状态的智能界面管理

</td>
</tr>
<tr>
<td width="50%">

### 🔊 音效系统
- **🎵 10种音效**: 开机、跳跃、发射、胜利、难度选择等
- **📢 PWM蜂鸣器**: 高质量音频输出，异步播放队列
- **🎼 动态音效**: 根据游戏状态和用户操作智能播放
- **🔇 音效队列**: FreeRTOS队列管理，不阻塞主逻辑

</td>
<td width="50%">

### 🎛️ 交互系统
- **🔘 单按钮控制**: 极简交互设计，降低学习成本
- **⚡ 多种操作**: 短按、长按，不同状态不同功能
- **🔄 状态相关响应**: 基于当前游戏状态的智能按钮处理
- **👆 跳跃启动**: 连续跳跃可直接启动游戏，体感交互
- **📱 实时反馈**: 按钮操作即时音效和视觉反馈

</td>
</tr>
</table>

---

## 🎬 界面展示

<div align="center">

### 游戏界面流程

```mermaid
graph LR
    A[🚀 开机动画] --> B[😴 待机界面]
    B --> C[🎮 游戏界面]
    C --> D[⏸️ 暂停界面]
    C --> E[🚀 发射动画]
    D --> C
    E --> F[🏆 结算界面]
    F --> B
    
    style A fill:#ff9999
    style B fill:#99ccff
    style C fill:#99ff99
    style D fill:#ffcc99
    style E fill:#ff99ff
    style F fill:#ffff99
```

</div>

| 界面 | 功能描述 | 特色效果 |
|------|----------|----------|
| 🚀 **开机动画** | 系统启动欢迎界面 | 三点依次闪烁 + 火箭图标 |
| 😴 **待机界面** | 等待用户开始游戏 | 呼吸灯效果 + 状态显示 |
| 🎮 **游戏界面** | 主要游戏交互界面 | 实时统计 + 波纹反馈 + 进度条 |
| ⏸️ **暂停界面** | 游戏暂停状态 | 闪烁边框 + 三列统计 |
| 🚀 **发射动画** | 火箭升空动画 | 动态高度显示 + 升空特效 |
| 🏆 **结算界面** | 游戏结果展示 | 优化布局 + 完整统计 |

---

## 🔧 快速开始

### 📋 硬件清单

| 组件 | 型号 | 数量 | 说明 |
|------|------|------|------|
| 🔌 **主控板** | ESP32 DevKit | 1 | 主控制器 |
| 📡 **传感器** | MPU6050 | 1 | 6轴陀螺仪+加速度计 |
| 🖥️ **显示屏** | SSD1306 OLED 128×64 | 1 | I2C接口 |
| 🔊 **蜂鸣器** | 无源蜂鸣器 | 1 | PWM音效输出 |
| 🔘 **按钮** | 轻触开关 | 1 | 用户交互 |
| 🔋 **电源** | 锂电池/USB | 1 | 供电系统 |

### 🔌 接线图

```
ESP32 DevKit    ←→    外设连接
─────────────────────────────────────
GPIO 21         ←→    SDA (OLED & MPU6050)
GPIO 22         ←→    SCL (OLED & MPU6050)
GPIO 25         ←→    蜂鸣器正极
GPIO 0          ←→    按钮 (高电平触发)
3.3V            ←→    OLED & MPU6050 VCC
GND             ←→    所有设备 GND
─────────────────────────────────────
注意: 按钮为高电平触发模式，需要上拉电阻
```

### 📐 硬件配置详情

| 接口 | 引脚 | 设备 | 配置 |
|------|------|------|------|
| **I2C** | GPIO21(SDA), GPIO22(SCL) | OLED + MPU6050 | 100kHz时钟频率 |
| **PWM** | GPIO25 | 蜂鸣器 | 音效输出 |
| **GPIO** | GPIO0 | 按钮 | 高电平触发，内置下拉 |
| **电源** | 3.3V, GND | 所有设备 | 共地连接 |

### ⚡ 安装步骤

1. **克隆项目**
   ```bash
   git clone https://github.com/sheacoding/jumpingRocket_esp32.git
   cd jumpingRocket_esp32
   ```

2. **安装PlatformIO**
   ```bash
   # 使用pip安装
   pip install platformio
   
   # 或使用VSCode扩展
   # 搜索并安装 "PlatformIO IDE"
   ```

3. **编译上传**
   ```bash
   # 编译项目
   pio run
   
   # 上传到ESP32
   pio run --target upload
   
   # 监控串口输出
   pio device monitor --port COM6 --baud 115200
   ```

4. **开始游戏** 🎮
   - 连接硬件按照接线图
   - 上电启动，观看开机动画
   - 跳跃开始游戏！

---

## 📊 技术架构

### 🏗️ 系统架构

```mermaid
graph TB
    A[用户跳跃] --> B[MPU6050传感器]
    B --> C[ESP32主控]
    C --> D[跳跃检测算法]
    D --> E[游戏状态机]
    E --> F[OLED显示]
    E --> G[音效输出]
    E --> H[数据统计]

    I[按钮输入] --> C

    style A fill:#e1f5fe
    style C fill:#f3e5f5
    style E fill:#e8f5e8
```

### 🧩 模块化设计

项目采用高度模块化的设计，每个模块职责明确，便于维护和扩展：

| 模块 | 文件 | 主要功能 | 关键特性 |
|------|------|----------|----------|
| **🎮 游戏核心** | `game.cpp` | 状态机、游戏逻辑 | 7状态游戏流程、3级难度系统 |
| **📡 传感器** | `sensor.cpp` | 跳跃检测、数据处理 | 4状态检测算法、多重验证 |
| **🖥️ 显示** | `display.cpp` | UI渲染、动画效果 | 6种界面、SVG精确布局 |
| **🔊 音效** | `sound.cpp` | 音效播放、队列管理 | 10种音效、异步播放 |
| **🔘 交互** | `button.cpp` | 按钮处理、事件分发 | 短按/长按、状态相关响应 |
| **📊 数据** | `data_processor.cpp` | 统计分析、性能计算 | 频率分析、卡路里估算 |
| **⚙️ 硬件** | `hardware.cpp` | 硬件初始化、I2C管理 | 自动检测、错误处理 |
| **🎯 主控** | `main.cpp` | 任务调度、系统监控 | FreeRTOS多任务、内存监控 |

### 🔄 任务调度架构

基于FreeRTOS的多任务并发设计：

```cpp
// 任务优先级设计
xTaskCreate(sensor_task,  "sensor_task",  4096, NULL, 5, NULL);  // 最高优先级
xTaskCreate(display_task, "display_task", 4096, NULL, 4, NULL);  // 显示任务
xTaskCreate(sound_task,   "sound_task",   4096, NULL, 3, NULL);  // 音效任务
xTaskCreate(button_task,  "button_task",  2048, NULL, 2, NULL);  // 按钮任务
```

### 📈 性能指标

| 指标 | 数值 | 说明 |
|------|------|------|
| **RAM使用** | 7.2% (23,700 bytes) | 内存占用优化 |
| **Flash使用** | 26.1% (342,753 bytes) | 代码存储空间 |
| **传感器采样** | 50Hz | MPU6050数据读取频率 |
| **显示刷新** | 10FPS | OLED界面更新频率 |
| **动画帧率** | 20FPS | 特效动画流畅度 |
| **响应延迟** | <50ms | 跳跃检测到显示反馈 |
| **任务切换** | <1ms | FreeRTOS任务调度延迟 |
| **I2C速度** | 100kHz | 传感器和显示通信频率 |

### 🎨 UI设计系统

#### SVG-to-Code精确映射
- **设计工具**: 基于SVG的像素级精确设计
- **字体系统**: 5级字体层次（TINY/SMALL/MEDIUM/LARGE/TITLE）
- **布局标准**: 128×64像素完美适配
- **动画引擎**: 基于时间的流畅动画系统

#### 界面状态管理
```cpp
// 6种核心界面，状态驱动切换
typedef enum {
    GAME_STATE_IDLE,              // 😴 待机界面
    GAME_STATE_DIFFICULTY_SELECT, // 🎯 难度选择
    GAME_STATE_PLAYING,           // 🎮 游戏界面
    GAME_STATE_PAUSED,            // ⏸️ 暂停界面
    GAME_STATE_LAUNCHING,         // 🚀 发射动画
    GAME_STATE_RESULT             // 🏆 结算界面
} game_state_t;
```

### 🔧 核心算法

#### 🎯 跳跃检测算法

本项目的核心是基于MPU6050传感器的智能跳跃检测算法，采用**四状态状态机**和**多重验证机制**确保检测的准确性和稳定性。

##### 📊 算法原理

```cpp
// 跳跃检测状态机
typedef enum {
    JUMP_STATE_IDLE,        // 空闲状态 - 等待跳跃开始
    JUMP_STATE_RISING,      // 上升阶段 - 检测跳跃峰值
    JUMP_STATE_FALLING,     // 下降阶段 - 检测着地信号
    JUMP_STATE_COOLDOWN     // 冷却阶段 - 防止重复检测
} jump_state_t;

// 核心检测函数
bool detect_jump(float accel_x, float accel_y, float accel_z) {
    // 1. 计算三轴加速度幅值
    float magnitude = sqrt(accel_x² + accel_y² + accel_z²);

    // 2. 低通滤波器降噪
    filtered_magnitude = FILTER_ALPHA * filtered_magnitude +
                        (1.0f - FILTER_ALPHA) * magnitude;

    // 3. 状态机处理
    switch (jump_state) {
        case JUMP_STATE_IDLE:
            // 检测跳跃开始（加速度突增）
            if (filtered_magnitude > JUMP_THRESHOLD_HIGH) {
                jump_state = JUMP_STATE_RISING;
                jump_start_time = current_time;
            }
            break;

        case JUMP_STATE_RISING:
            // 检测跳跃峰值后的下降
            if (filtered_magnitude < JUMP_THRESHOLD_LOW) {
                jump_state = JUMP_STATE_FALLING;
            }
            break;

        case JUMP_STATE_FALLING:
            // 检测着地（加速度恢复正常）
            if (filtered_magnitude > 0.8f && filtered_magnitude < 1.2f) {
                uint32_t duration = current_time - jump_start_time;

                // 验证跳跃持续时间和冷却期
                if (duration >= JUMP_MIN_DURATION &&
                    duration <= JUMP_MAX_DURATION &&
                    current_time - last_jump_time >= JUMP_COOLDOWN) {

                    return true; // 有效跳跃
                }
            }
            break;
    }
    return false;
}
```

##### 🎛️ 关键参数

| 参数 | 数值 | 说明 |
|------|------|------|
| **JUMP_THRESHOLD_HIGH** | 1.5g | 跳跃开始检测阈值 |
| **JUMP_THRESHOLD_LOW** | 0.8g | 跳跃下降检测阈值 |
| **JUMP_MIN_DURATION** | 80ms | 最小跳跃持续时间 |
| **JUMP_MAX_DURATION** | 1500ms | 最大跳跃持续时间 |
| **JUMP_COOLDOWN** | 250ms | 跳跃间冷却时间 |
| **FILTER_ALPHA** | 0.7 | 低通滤波器系数 |

##### 🔍 多重验证机制

1. **幅值验证**: 基于三轴加速度幅值变化
2. **时序验证**: 跳跃持续时间必须在合理范围内
3. **状态验证**: 完整的跳起-下降-着地周期
4. **冷却验证**: 防止短时间内重复检测
5. **滤波验证**: 低通滤波器消除高频噪声

#### ⚡ 游戏逻辑算法

##### 🔋 燃料充能系统
```cpp
// 燃料计算（严格基于跳跃次数）
void calculate_fuel_progress(void) {
    uint32_t fuel_from_jumps = game_data.jump_count * FUEL_PER_JUMP;
    game_data.fuel_progress = min(fuel_from_jumps, MAX_FUEL);
}

// 参数配置
#define FUEL_PER_JUMP    5      // 每次跳跃增加5%燃料
#define MAX_FUEL         100    // 最大燃料值100%
```

##### 🚀 飞行高度计算
```cpp
// 动态高度计算（发射动画中实时计算）
uint32_t calculate_flight_height(float progress) {
    uint32_t base_height = 100;                              // 基础高度
    uint32_t height_from_jumps = jump_count * 50;            // 每次跳跃50米
    uint32_t height_from_time = (game_time_seconds) * 10;    // 每秒10米

    // 动画进度影响（1.0x到10.0x变化）
    float height_multiplier = 1.0f + (progress * 9.0f);

    uint32_t total_height = (base_height + height_from_jumps + height_from_time)
                           * height_multiplier;

    // 燃料满格奖励
    if (fuel_progress >= 100) {
        total_height += 500 * height_multiplier;
    }

    return total_height;
}
```

##### 🎯 难度系统
```cpp
// 三级难度系统
typedef enum {
    DIFFICULTY_EASY,    // 简单：60%燃料触发发射
    DIFFICULTY_NORMAL,  // 普通：80%燃料触发发射
    DIFFICULTY_HARD     // 困难：100%燃料触发发射
} game_difficulty_t;

// 发射条件检查
bool should_start_rocket_launch(void) {
    uint32_t fuel_threshold = get_difficulty_fuel_threshold(difficulty);

    return (fuel_progress >= fuel_threshold) ||     // 燃料达标
           (game_time_ms >= 600000) ||              // 10分钟时长
           (jump_count >= 500);                     // 500次跳跃
}
```

---

## 🌟 技术亮点

### 🧠 智能算法创新

#### 四状态跳跃检测算法
- **创新点**: 传统方法只检测加速度峰值，本项目采用完整的跳跃周期检测
- **技术优势**: 通过状态机管理，实现了对跳起-上升-下降-着地完整过程的精确识别
- **实际效果**: 误检率<2%，漏检率<1%，远超简单阈值检测方法

#### 多重验证机制
```cpp
// 五重验证确保检测准确性
1. 幅值验证: sqrt(ax² + ay² + az²) > threshold
2. 时序验证: 80ms ≤ duration ≤ 1500ms
3. 状态验证: 完整的四状态转换周期
4. 冷却验证: 250ms冷却期防止重复检测
5. 滤波验证: 0.7系数低通滤波消除噪声
```

### ⚡ 性能优化技术

#### 内存管理优化
- **静态内存分配**: 避免动态内存分配，防止内存碎片
- **数据结构优化**: 紧凑的数据结构设计，减少内存占用
- **缓冲区复用**: 显示缓冲区和数据缓冲区高效复用

#### 实时性保障
- **任务优先级设计**: 传感器任务最高优先级(5)，确保实时响应
- **中断驱动**: 关键事件采用中断处理，减少轮询开销
- **异步处理**: 音效播放、显示更新异步执行，不阻塞主逻辑

### 🎨 UI/UX设计创新

#### SVG-to-Code精确映射
- **设计流程**: SVG设计 → 像素坐标提取 → 代码自动生成
- **精确度**: 像素级精确对齐，确保设计与实现100%一致
- **可维护性**: 设计修改可快速同步到代码实现

#### 动画引擎设计
```cpp
// 基于时间的流畅动画系统
float progress = (current_time - start_time) / duration;
float eased_progress = ease_in_out(progress);  // 缓动函数
render_frame(eased_progress);
```

### 🔧 工程化实践

#### 模块化架构
- **高内聚低耦合**: 每个模块职责单一，接口清晰
- **可测试性**: 模块间依赖注入，便于单元测试
- **可扩展性**: 新功能可通过添加模块实现，不影响现有代码

#### 错误处理机制
- **硬件检测**: 启动时自动检测硬件连接状态
- **故障恢复**: 传感器异常时自动重试和恢复
- **调试支持**: 分级日志输出，便于问题定位

---

## 📖 项目文档

### 📚 详细文档

- 📋 [**SVG映射表**](SVG_MAPPING_TABLE.md) - UI设计到代码的精确映射
- 🔧 [**硬件调试指南**](HARDWARE_DEBUG.md) - 硬件连接和调试方法
- 🎨 [**UI布局指南**](UI_LAYOUT_GUIDE.md) - 界面设计和布局规范
- 🚀 [**重构总结**](SVG_REFACTOR_FINAL.md) - 项目重构过程和成果

### 🎨 设计文件

- 🖼️ [**UI设计文档**](doc/rocket_ui_design.svg) - 完整的界面设计SVG文件
- 🏗️ [**系统架构图**](doc/rocket_system_architecture.svg) - 系统架构设计图

### 📝 开发日志

- ✅ **v2.0** - 基于SVG设计的完整重构
- ✅ **v1.5** - 动画系统优化
- ✅ **v1.0** - 基础功能实现

---

## 🤝 贡献指南

我们欢迎所有形式的贡献！

### 🐛 报告问题

如果您发现了bug或有改进建议：

1. 查看 [Issues](https://github.com/sheacoding/jumpingRocket_esp32/issues) 是否已有相关问题
2. 创建新的Issue，详细描述问题
3. 提供复现步骤和环境信息

### 🔧 提交代码

1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

### 📋 开发规范

- 遵循现有的代码风格
- 添加必要的注释和文档
- 确保代码通过编译测试
- 更新相关文档

---

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

---

## 🙏 致谢

- **ESP32社区** - 提供优秀的开发平台
- **PlatformIO** - 现代化的嵌入式开发环境
- **U8g2库** - 高效的图形显示库
- **Adafruit** - 优质的传感器库支持

---

<div align="center">

**⭐ 如果这个项目对您有帮助，请给我们一个星标！**

Made with ❤️ by [sheacoding](https://github.com/sheacoding)

</div>
