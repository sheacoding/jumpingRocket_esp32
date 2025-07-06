# 🔧 蹦跳小火箭 - 多开发板配置指南

本项目支持多种ESP32开发板，通过自动配置系统可以轻松在不同开发板之间切换。

## 📋 支持的开发板

### 1. ESP32-C3 DevKit (`esp32c3dev`)
- **芯片**: ESP32-C3 (RISC-V架构)
- **特点**: USB串口、低功耗、21个GPIO
- **适用场景**: 小型项目、电池供电应用

**引脚配置**:
```
I2C SCL:  GPIO8
I2C SDA:  GPIO9
按钮:     GPIO3 (高电平触发，下拉电阻)
蜂鸣器:   GPIO4
UART RX:  GPIO20
UART TX:  GPIO21
```

### 2. ESP32 DevKit (`esp32dev`)
- **芯片**: ESP32 (Xtensa双核架构)
- **特点**: 经典开发板、39个GPIO、丰富外设
- **适用场景**: 复杂项目、需要更多GPIO的应用

**引脚配置**:
```
I2C SCL:  GPIO22
I2C SDA:  GPIO21
按钮:     GPIO2 (低电平触发，上拉电阻)
蜂鸣器:   GPIO25
UART RX:  GPIO3
UART TX:  GPIO1
```

## 🚀 快速开始

### 方法1: 使用切换脚本 (推荐)

1. **查看支持的开发板**:
   ```bash
   python switch_board.py --list
   ```

2. **切换到ESP32-C3**:
   ```bash
   python switch_board.py esp32c3
   ```

3. **切换到ESP32并编译上传**:
   ```bash
   python switch_board.py esp32 --build --upload
   ```

4. **切换并开始串口监控**:
   ```bash
   python switch_board.py esp32c3 --monitor
   ```

### 方法2: 使用PlatformIO命令

1. **编译ESP32-C3版本**:
   ```bash
   pio run -e esp32c3dev
   ```

2. **编译ESP32版本**:
   ```bash
   pio run -e esp32dev
   ```

3. **上传到ESP32-C3**:
   ```bash
   pio run -e esp32c3dev --target upload
   ```

4. **上传到ESP32**:
   ```bash
   pio run -e esp32dev --target upload
   ```

## 🔌 硬件连接

### ESP32-C3 连接图
```
OLED SSD1306:          MPU6050:           按钮:              蜂鸣器:
VCC -> 3.3V           VCC -> 3.3V        一端 -> GPIO3      正极 -> GPIO4
GND -> GND            GND -> GND         另一端 -> 3.3V     负极 -> GND
SCL -> GPIO8          SCL -> GPIO8
SDA -> GPIO9          SDA -> GPIO9
```

### ESP32 连接图
```
OLED SSD1306:          MPU6050:           按钮:              蜂鸣器:
VCC -> 3.3V           VCC -> 3.3V        一端 -> GPIO2      正极 -> GPIO25
GND -> GND            GND -> GND         另一端 -> GND      负极 -> GND
SCL -> GPIO22         SCL -> GPIO22
SDA -> GPIO21         SDA -> GPIO21
```

## ⚙️ 配置系统说明

### 自动配置机制

项目使用编译时宏定义来自动选择正确的引脚配置：

1. **platformio.ini** 中定义开发板类型标志
2. **board_config.h** 根据标志选择对应配置
3. **运行时验证** 确保配置正确性

### 配置文件结构

```
include/
├── board_config.h          # 开发板配置头文件
├── jumping_rocket_simple.h # 主要头文件
└── jumping_rocket.h        # 传统头文件(兼容)

src/
├── board_config.cpp        # 配置实现和验证
├── main.cpp               # 主程序
└── ...                    # 其他模块
```

## 🔍 调试和验证

### 启动时的配置信息

程序启动时会显示详细的配置信息：

```
========================================
🔧 开发板配置初始化
========================================
📋 开发板详细信息:
   开发板名称: ESP32-C3 DevKit
   芯片型号: ESP32-C3
   芯片版本: 3
   CPU频率: 160 MHz
   Flash大小: 4 MB
📋 开发板配置: ESP32-C3 DevKit
   I2C: SDA=GPIO9, SCL=GPIO8
   按钮: GPIO3 (高电平触发)
   蜂鸣器: GPIO4
   UART: RX=GPIO20, TX=GPIO21
   I2C频率: 100000 Hz
✅ 开发板配置验证通过
```

### 配置验证功能

系统会自动验证：
- ✅ 引脚冲突检查
- ✅ 引脚有效性验证
- ✅ 开发板特定限制检查
- ✅ I2C设备扫描

## 🛠️ 添加新开发板

要添加新的开发板支持：

1. **在 platformio.ini 中添加新环境**:
   ```ini
   [env:new_board]
   platform = espressif32
   board = your_board
   framework = arduino
   build_flags = 
       -DBOARD_NEW_BOARD=1
       -DI2C_SCL_PIN=your_scl_pin
       -DI2C_SDA_PIN=your_sda_pin
       # ... 其他引脚配置
   ```

2. **在 board_config.h 中添加配置**:
   ```c
   #elif defined(BOARD_NEW_BOARD)
       #define BOARD_NAME "Your Board Name"
       // ... 引脚定义
   ```

3. **更新切换脚本**:
   在 `switch_board.py` 的 `BOARD_CONFIGS` 中添加新配置。

## 🔧 故障排除

### 常见问题

1. **编译错误 "未定义的引脚"**:
   - 检查 platformio.ini 中的 build_flags
   - 确保开发板类型标志正确定义

2. **I2C设备未找到**:
   - 检查引脚连接
   - 验证开发板配置是否正确
   - 使用I2C扫描功能检查设备

3. **按钮不响应**:
   - 检查按钮触发电平配置
   - 验证上拉/下拉电阻设置
   - 查看串口输出的按钮状态信息

### 调试技巧

1. **查看详细启动信息**:
   ```bash
   pio device monitor -e esp32c3dev
   ```

2. **使用I2C扫描**:
   程序启动时会自动扫描I2C设备

3. **监控按钮状态**:
   按钮任务会输出详细的状态变化信息

## 📚 相关文档

- [PlatformIO环境配置](https://docs.platformio.org/en/latest/projectconf/section_env.html)
- [ESP32-C3引脚图](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html)
- [ESP32引脚图](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html)

## 🤝 贡献

欢迎提交新的开发板配置或改进建议！请确保：
- 添加完整的引脚配置
- 包含配置验证逻辑
- 更新相关文档
