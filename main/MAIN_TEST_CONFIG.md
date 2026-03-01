# main.c 测试配置说明

## 📋 概述

`main.c` 已通过宏定义配置为支持多种测试模式，无需使用独立的测试文件。

---

## 🔧 配置宏定义

在 `main.c` 文件开头定义了两个配置宏：

```c
#define ENABLE_GUI_TEST     0   // 1=启用 GUI 测试，0=禁用
#define ENABLE_BASIC_TEST   1   // 1=启用基础测试，0=禁用
```

---

## 🎯 测试模式

### 模式 1: GUI 功能测试

**配置**：
```c
#define ENABLE_GUI_TEST     1
#define ENABLE_BASIC_TEST   0
```

**测试内容**：
- ✅ 画布创建和销毁
- ✅ 清屏功能
- ✅ 绘制边框矩形
- ✅ 绘制对角线
- ✅ 绘制实心矩形
- ✅ 绘制空心圆
- ✅ 绘制实心圆
- ✅ 显示字符串（需要字模数据）

**执行流程**：
1. 初始化显示屏
2. 清屏并显示 2 秒
3. 绘制各种几何图形
4. 刷新显示并等待 3 秒
5. 显示字符串（8x6、16x8、24x12 三种字体）
6. 刷新显示并等待 5 秒
7. 清理资源

---

### 模式 2: 基础显示测试

**配置**：
```c
#define ENABLE_GUI_TEST     0
#define ENABLE_BASIC_TEST   1
```

**测试内容**：
- ✅ 清屏功能
- ✅ 显存分配
- ✅ 显示测试图案（黑白条纹）
- ✅ 再次清屏
- ✅ 深度睡眠模式

**执行流程**：
1. 初始化显示屏
2. 清屏并等待 2 秒
3. 创建黑白条纹图案
4. 显示图案并等待 5 秒
5. 再次清屏
6. 进入深度睡眠
7. 释放内存

---

### 模式 3: 禁用所有测试

**配置**：
```c
#define ENABLE_GUI_TEST     0
#define ENABLE_BASIC_TEST   0
```

**效果**：
- 仅初始化显示屏
- 输出警告信息
- 不执行任何测试

---

## 📊 测试对比

| 特性 | GUI 测试 | 基础测试 |
|------|---------|---------|
| 几何图形绘制 | ✅ | ❌ |
| 字符显示 | ✅ | ❌ |
| 画布管理 | ✅ | ❌ |
| 基础显示 | ✅ | ✅ |
| 清屏功能 | ✅ | ✅ |
| 需要字模 | ⚠️ 可选 | ❌ 不需要 |
| 内存占用 | 中等 | 中等 |
| 测试时间 | ~10 秒 | ~7 秒 |

---

## 🚀 使用方法

### 步骤 1: 选择测试模式

打开 `main.c` 文件，找到第 12-13 行：

```c
#define ENABLE_GUI_TEST     0   // 修改这里
#define ENABLE_BASIC_TEST   1   // 修改这里
```

### 步骤 2: 修改配置

根据需要的测试模式修改宏定义值。

### 步骤 3: 编译和烧录

```bash
# 编译
idf.py build

# 烧录（替换 COM3 为实际端口）
idf.py -p COM3 flash

# 查看串口输出
idf.py -p COM3 monitor
```

---

## 📝 日志输出示例

### GUI 测试日志

```
I (0) MAIN: SSD1680 测试程序启动
I (0) MAIN: 配置：GUI_TEST=1, BASIC_TEST=0
I (0) SSD1680: 初始化 SSD1680 显示屏
I (0) SSD1680: SSD1680 初始化完成
I (0) MAIN: 显示屏初始化成功
I (0) MAIN: === 开始 GUI 功能测试 ===
I (0) MAIN: 清屏...
I (0) MAIN: 绘制边框矩形...
I (0) MAIN: 绘制对角线...
I (0) MAIN: 绘制实心矩形...
I (0) MAIN: 绘制空心圆...
I (0) MAIN: 绘制实心圆...
I (0) MAIN: 显示字符串...
I (0) MAIN: === GUI 功能测试完成 ===
I (0) MAIN: 程序结束
```

### 基础测试日志

```
I (0) MAIN: SSD1680 测试程序启动
I (0) MAIN: 配置：GUI_TEST=0, BASIC_TEST=1
I (0) SSD1680: 初始化 SSD1680 显示屏
I (0) SSD1680: SSD1680 初始化完成
I (0) MAIN: 显示屏初始化成功
I (0) MAIN: === 开始基础显示测试 ===
I (0) MAIN: 清屏测试...
I (0) MAIN: 显示测试图案...
I (0) MAIN: 再次清屏...
I (0) MAIN: 进入深度睡眠模式
I (0) MAIN: === 基础显示测试完成 ===
I (0) MAIN: 程序结束
```

---

## ⚠️ 注意事项

### 1. 字模数据

GUI 测试中的字符显示功能需要字模数据：

- **如果字模已导入**：正常显示字符串
- **如果字模未导入**：字符显示函数会返回，不影响其他功能

### 2. 内存管理

两个测试都会动态分配内存：

- GUI 测试：分配一个显存缓冲区（5,624 字节）
- 基础测试：分配两个显存缓冲区（11,248 字节）

测试完成后会自动释放内存。

### 3. 刷新时间

SSD1680 刷新需要约 30 秒：

- GUI 测试：刷新 2 次，总等待时间约 10 秒
- 基础测试：刷新 1 次，总等待时间约 7 秒

实际刷新时间在后台进行，日志中的延时只是为了观察效果。

---

## 🔍 调试技巧

### 快速测试

如果只想快速验证基本功能，可以缩短延时时间：

```c
// 在 gui_test() 或 basic_test() 中修改
vTaskDelay(pdMS_TO_TICKS(3000));  // 改为 1000 或更短
```

### 单独测试几何图形

注释掉字符显示部分：

```c
// 注释掉这些行
// font = epd_font_get(EPD_FONT_SIZE_8X6);
// epd_show_string(canvas, 10, 10, "8x6 Font", font, EPD_COLOR_BLACK);
```

### 测试特定字体

只启用一种字体：

```c
// 只测试 16x8 字体
font = epd_font_get(EPD_FONT_SIZE_16X8);
if (font != NULL) {
    epd_show_string(canvas, 10, 25, "16x8 Font", font, EPD_COLOR_BLACK);
}
```

---

## 📚 相关文件

| 文件 | 说明 |
|------|------|
| [`main.c`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\main.c) | 主程序（包含测试代码） |
| [`epd_gui.h`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\epd_gui.h) | GUI 接口定义 |
| [`epd_gui.c`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\epd_gui.c) | GUI 功能实现 |
| [`epd_font.h`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\epd_font.h) | 字体接口定义 |
| [`epd_font.c`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\epd_font.c) | 字体功能实现 |

---

## ✅ 配置检查清单

切换测试模式时，请确认：

- [ ] 修改 `ENABLE_GUI_TEST` 的值（0 或 1）
- [ ] 修改 `ENABLE_BASIC_TEST` 的值（0 或 1）
- [ ] 两个宏不能同时为 1
- [ ] 保存 `main.c` 文件
- [ ] 重新编译项目 `idf.py build`
- [ ] 烧录到开发板 `idf.py -p COM3 flash`
- [ ] 查看串口输出确认测试运行

---

**更新日期**: 2026-02-28  
**测试模式**: 2 种（GUI 测试、基础测试）  
**配置方式**: 宏定义控制  
**状态**: ✅ 已完成
