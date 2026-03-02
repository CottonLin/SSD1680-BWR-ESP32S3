# SSD1680 项目组件目录

## 目录结构

```
components/
├── epd_driver/         # 硬件驱动层组件
│   ├── include/        # 公共头文件
│   │   └── epd_driver.h
│   └── src/            # 源文件
│       ├── epd_driver.c
│       ├── epd_spi.c
│       └── epd_gpio.c
│
├── epd_gui/            # GUI 层组件
│   ├── include/        # 公共头文件
│   │   └── epd_gui.h
│   └── src/            # 源文件
│       ├── epd_gui.c
│       ├── epd_draw.c
│       └── epd_text.c
│
├── epd_font/           # 字体系统组件
│   ├── include/        # 公共头文件
│   │   └── epd_font.h
│   └── src/            # 源文件
│       ├── epd_font.c
│       └── fonts/      # 字体数据
│
└── epd_utils/          # 工具函数组件
    ├── include/        # 公共头文件
    │   └── epd_utils.h
    └── src/            # 源文件
        └── epd_utils.c
```

## 组件说明

### epd_driver - 硬件驱动层

**职责**: 
- SSD1680 硬件驱动
- SPI 通信管理
- GPIO 控制
- 设备句柄管理

**主要功能**:
- 设备初始化和反初始化
- 显示刷新控制
- 电源管理（睡眠/唤醒）

### epd_gui - GUI 层

**职责**:
- 画布管理
- 图形绘制
- 文本渲染

**主要功能**:
- 画布创建/销毁
- 基本图形绘制（直线、矩形、圆形）
- 文本显示（支持自动换行）

### epd_font - 字体系统

**职责**:
- 字体数据管理
- 字体渲染

**主要功能**:
- 多种字体支持
- 字符串宽度计算
- 字体数据访问

### epd_utils - 工具函数

**职责**:
- 通用工具函数
- 辅助功能

**主要功能**:
- 内存管理辅助
- 调试辅助函数

## 使用指南

### 组件依赖关系

```
main
├── epd_gui
│   ├── epd_font
│   └── epd_driver
└── epd_utils
```

### 在代码中使用

```c
// 包含组件头文件
#include "epd_driver.h"
#include "epd_gui.h"
#include "epd_font.h"
#include "epd_utils.h"
```

## 注意事项

1. 所有组件的头文件应放在对应的 `include/` 目录
2. 源文件应放在对应的 `src/` 目录
3. 组件间通过公共头文件进行交互
4. 避免循环依赖
