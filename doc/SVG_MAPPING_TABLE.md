# SVG-to-U8g2 精确映射对照表

## 📐 坐标系统映射

### SVG坐标到U8g2坐标转换
```cpp
// 基础映射函数
#define SVG_TO_SCREEN_X(svg_x) ((int)((svg_x) * SCREEN_WIDTH / 128))
#define SVG_TO_SCREEN_Y(svg_y) ((int)((svg_y) * SCREEN_HEIGHT / 64))

// 变换映射函数
int svg_transform_x(int svg_x, int svg_translate_x);
int svg_transform_y(int svg_y, int svg_translate_y);
```

### 布局区域映射
| SVG区域 | 高度(px) | U8g2常量 | 用途 |
|---------|----------|----------|------|
| 状态栏 | 0-12 | STATUS_BAR_HEIGHT | 顶部信息显示 |
| 内容区 | 12-54 | CONTENT_AREA_HEIGHT | 主要内容 |
| 提示区 | 54-64 | HINT_BAR_HEIGHT | 底部操作提示 |

## 🎨 字体大小映射标准

### SVG font-size 到 U8g2字体对应表
| SVG font-size | U8g2字体常量 | 实际尺寸 | 用途 |
|---------------|--------------|----------|------|
| 6-7px | FONT_TINY | 4x6像素 | 小标签、提示文字 |
| 8px | FONT_SMALL | 6x10像素 | 普通文字、数值 |
| 10-12px | FONT_MEDIUM | 7x13像素 | 标题、重要文字 |
| 14-16px | FONT_LARGE | 10x20像素 | 大数字、主要数据 |

### 字体选择示例
```cpp
// SVG: font-size="6" → U8g2: FONT_TINY
u8g2.setFont(FONT_TINY);

// SVG: font-size="8" → U8g2: FONT_SMALL  
u8g2.setFont(FONT_SMALL);

// SVG: font-size="10" → U8g2: FONT_MEDIUM
u8g2.setFont(FONT_MEDIUM);

// SVG: font-size="14" → U8g2: FONT_LARGE
u8g2.setFont(FONT_LARGE);
```

## 🎬 动画参数映射

### SVG animate 到 millis() 时间控制
| SVG属性 | 转换函数 | 示例 |
|---------|----------|------|
| dur="2s" | SVG_DUR_TO_MS(2.0) | 2000ms |
| dur="0.5s" | SVG_DUR_TO_MS(0.5) | 500ms |
| dur="1.5s" | SVG_DUR_TO_MS(1.5) | 1500ms |

### 动画周期控制
```cpp
// SVG: dur="2s" 的动画周期控制
uint32_t cycle = svg_animate_progress(millis(), SVG_DUR_TO_MS(2.0));
float t = cycle / 2000.0f; // 0.0-1.0 的周期进度
```

### 透明度效果模拟
| SVG opacity | 模拟方法 | 实现函数 |
|-------------|----------|----------|
| 0.0-1.0 | 点阵密度控制 | svg_opacity_visible() |
| 闪烁效果 | 时间偏移模式 | 时间取模判断 |

## 📍 界面元素精确定位

### 1. 开机动画界面
| SVG元素 | SVG坐标 | U8g2实现 |
|---------|---------|----------|
| 火箭图标 | transform="translate(64,32)" | svg_transform_x(-8,64), svg_transform_y(-8,32) |
| "ROCKET"文字 | x="64" y="48" | 居中对齐，y=48 |
| 指示点1 | cx="40" cy="55" | x=40, y=55, r=2 |
| 指示点2 | cx="50" cy="55" | x=50, y=55, r=2 |
| 指示点3 | cx="60" cy="55" | x=60, y=55, r=2 |

### 2. 待机界面
| SVG元素 | SVG坐标 | U8g2实现 |
|---------|---------|----------|
| 状态文字 | x="5" y="9" | svg_transform_x(0,5), svg_transform_y(0,9) |
| 分割线 | y1="12" | y=STATUS_BAR_HEIGHT |
| 火箭图标 | transform="translate(64,30)" | svg_transform_x(-8,64), svg_transform_y(-8,30) |
| 提示文字 | x="64" y="45" | 居中对齐，y=45 |
| 呼吸灯 | cx="64" cy="30" r="15;20;15" | 中心(64,30)，半径15-20变化 |

### 3. 游戏界面
| SVG元素 | SVG坐标 | U8g2实现 |
|---------|---------|----------|
| 时间显示 | x="5" y="9" | svg_transform_x(0,5), svg_transform_y(0,9) |
| 跳跃计数 | x="90" y="9" | svg_transform_x(0,90), svg_transform_y(0,9) |
| 分割线 | y1="12" | y=STATUS_BAR_HEIGHT |
| 火箭图标 | transform="translate(20,32)" | svg_transform_x(-4,20), svg_transform_y(-4,32) |
| 波纹中心 | cx="20" cy="32" | 中心(20,32) |
| 燃料条 | x="35" y="25" width="85" height="6" | 精确尺寸映射 |

### 4. 暂停界面
| SVG元素 | SVG坐标 | U8g2实现 |
|---------|---------|----------|
| 边框 | x="2" y="2" width="124" height="60" | 双重边框，闪烁效果 |
| 暂停图标 | transform="translate(64,18)" | svg_transform_x(-4,64), svg_transform_y(-4,18) |
| "PAUSED"文字 | x="64" y="30" | 居中对齐，y=30 |
| 统计列1 | x="20" y="42" | svg_transform_x(0,20), svg_transform_y(0,42) |
| 统计列2 | x="55" y="42" | svg_transform_x(0,55), svg_transform_y(0,42) |
| 统计列3 | x="95" y="42" | svg_transform_x(0,95), svg_transform_y(0,42) |

### 5. 重置确认界面
| SVG元素 | SVG坐标 | U8g2实现 |
|---------|---------|----------|
| 警告边框 | x="8" y="8" width="112" height="48" | 闪烁双重边框 |
| 警告图标 | transform="translate(64,20)" | svg_transform_x(-4,64), svg_transform_y(-4,20) |
| "RESET?"文字 | x="64" y="32" | 居中对齐，y=32 |
| 警告文字 | x="64" y="42" | 居中对齐，闪烁效果 |
| 操作说明1 | x="64" y="50" | 居中对齐，y=50 |
| 操作说明2 | x="64" y="58" | 居中对齐，y=58 |

### 6. 结算界面
| SVG元素 | SVG坐标 | U8g2实现 |
|---------|---------|----------|
| 奖杯图标 | transform="translate(64,12)" | svg_transform_x(-4,64), svg_transform_y(-4,12) |
| "COMPLETE!"文字 | x="64" y="24" | 居中对齐，y=24 |
| 飞行高度 | x="64" y="38" | 居中对齐，大字体 |
| 高度标签 | x="64" y="48" | 居中对齐，小字体 |
| 统计列1 | x="15" y="56" | svg_transform_x(0,15), svg_transform_y(0,56) |
| 统计列2 | x="55" y="56" | svg_transform_x(0,55), svg_transform_y(0,56) |
| 统计列3 | x="95" y="56" | svg_transform_x(0,95), svg_transform_y(0,56) |

## 🎭 动画效果精确实现

### 呼吸灯效果（待机界面）
```cpp
// SVG: animate r="15;20;15" opacity="0.3;0.1;0.3" dur="2s"
uint32_t breath_cycle = svg_animate_progress(millis(), SVG_DUR_TO_MS(2.0));
float t = breath_cycle / 2000.0f;
float radius_progress = sin(t * PI); // 0->1->0
int radius = 15 + (int)(5 * radius_progress); // 15-20像素
float opacity = 0.1f + 0.2f * (0.5f + 0.5f * cos(t * 2 * PI)); // 0.1-0.3
```

### 波纹扩散效果（游戏界面）
```cpp
// SVG: 双层circle，r="5;15" 和 r="8;20"，dur="0.5s"
float t = (float)elapsed / JUMP_ANIMATION_DURATION;
int radius1 = 5 + (int)(10 * t); // 第一层：5-15像素
int radius2 = 8 + (int)(12 * t); // 第二层：8-20像素（延迟0.1秒）
```

### 闪烁边框效果（暂停/重置界面）
```cpp
// SVG: animate opacity="0.3;1;0.3" dur="1s"
uint32_t border_cycle = svg_animate_progress(millis(), SVG_DUR_TO_MS(1.0));
float border_t = border_cycle / 1000.0f;
float border_opacity = 0.3f + 0.7f * (0.5f + 0.5f * sin(border_t * 2 * PI));
```

## 🔧 实现验证标准

### 位置精度验证
- 所有元素位置误差 ≤ 1像素
- 文字居中对齐使用u8g2.getStrWidth()精确计算
- 图标中心点严格按照SVG transform坐标

### 动画时间精度验证
- 动画周期误差 ≤ 50ms
- 使用millis()确保时间精确性
- 所有dur属性严格按照SVG定义

### 视觉效果验证
- 字体大小符合SVG font-size映射
- 透明度效果通过点阵密度模拟
- 闪烁频率与SVG animate周期一致

这个映射表确保了ESP32 OLED显示系统与SVG设计文档的完全一致性！
