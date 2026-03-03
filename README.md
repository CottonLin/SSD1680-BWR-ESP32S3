# SSD1680 电子纸显示项目

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.5-blue.svg)](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-orange.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Display](https://img.shields.io/badge/Display-SSD1680_E--Paper-green.svg)](https://www.e-paper-display.com/products_detail/productId=397.html)
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)

基于 **ESP32-S3** 和 **SSD1680** 驱动 IC 的三色电子纸显示项目，支持黑/红/白三色显示，采用模块化组件架构，适用于 ESP-IDF v5.5 平台。

---

## 📋 目录

- [功能特性](#-功能特性)
- [硬件要求](#-硬件要求)
- [项目结构](#-项目结构)
- [快速开始](#-快速开始)
- [组件说明](#-组件说明)
- [使用示例](#-使用示例)
- [API 文档](#-api-文档)
- [字体数据导入](#-字体数据导入)
- [测试说明](#-测试说明)
- [常见问题](#-常见问题)
- [文档资源](#-文档资源)
- [许可证](#-许可证)

---

## ✨ 功能特性

### 核心功能
- ✅ **三色显示**: 支持黑/红/白三色显示
- ✅ **双缓冲区架构**: 独立控制黑白和红色显示通道
- ✅ **自动坐标转换**: 用户坐标 ↔ SSD1680 内部坐标自动映射
- ✅ **智能换行**: 按单词智能换行，保持单词完整性
- ✅ **多种字体**: 支持 4 种字体大小 (6x8, 6x12, 8x16, 12x24)
- ✅ **丰富的绘图**: 直线、矩形、圆形、三角形绘制

### 技术特性
- ✅ **模块化设计**: 高内聚低耦合的组件架构
- ✅ **设备句柄管理**: 支持多设备管理
- ✅ **完善错误处理**: 所有函数返回 `esp_err_t` 错误码
- ✅ **内存优化**: 动态内存管理，字模数据外置
- ✅ **单元测试**: 基于 ESP-IDF Unity 框架，15 个测试用例

---

## 🛠️ 硬件要求

### 必需硬件
- **开发板**: ESP32-S3 开发板
- **显示屏**: SSD1680 驱动 IC 的 2.66 英寸三色电子纸显示屏 (296×152 分辨率)

### 引脚连接

| SSD1680 | ESP32-S3 | 说明 |
|---------|----------|------|
| VCC | 3.3V | 电源正极 |
| GND | GND | 电源地 |
| CLK | GPIO12 | SPI 时钟 |
| DIN | GPIO11 | SPI 数据 |
| CS | GPIO21 | 片选信号 |
| DC | GPIO6 | 数据/命令选择 |
| RST | GPIO4 | 复位信号 |
| BUSY | GPIO5 | 忙信号输入 |

### 默认引脚配置

```c
#define EPD_SPI_SCLK_PIN_DEFAULT    GPIO_NUM_12   // SPI CLK
#define EPD_SPI_MOSI_PIN_DEFAULT    GPIO_NUM_11   // SPI MOSI
#define EPD_PIN_RST_DEFAULT         GPIO_NUM_4    // 复位
#define EPD_PIN_DC_DEFAULT          GPIO_NUM_6    // DC
#define EPD_PIN_CS_DEFAULT          GPIO_NUM_21   // CS
#define EPD_PIN_BUSY_DEFAULT        GPIO_NUM_5    // BUSY
```

---

## 📁 项目结构

```
SSD1680/
├── components/                  # ESP-IDF 组件目录
│   ├── epd_driver/             # 驱动层组件
│   │   ├── include/
│   │   │   └── epd_driver.h    # 驱动头文件
│   │   ├── src/
│   │   │   └── epd_driver.c    # 驱动实现
│   │   └── CMakeLists.txt
│   ├── epd_gui/                # GUI 层组件
│   │   ├── include/
│   │   │   └── epd_gui.h       # GUI 头文件
│   │   ├── src/
│   │   │   └── epd_gui.c       # GUI 实现
│   │   └── CMakeLists.txt
│   ├── epd_font/               # 字体层组件
│   │   ├── include/
│   │   │   └── epd_font.h      # 字体头文件
│   │   ├── src/
│   │   │   ├── epd_font.c      # 字体管理
│   │   │   └── epd_font_data.c # 字模数据（用户自行导入）
│   │   └── CMakeLists.txt
│   └── README.md               # 组件说明
├── main/
│   ├── main.c                  # 主程序入口
│   ├── CMakeLists.txt
│   └── Kconfig.projbuild
├── docs/                       # 文档目录
│   └── 问题修复记录.md         # 问题修复记录
├── CMakeLists.txt              # 项目构建配置
├── README.md                   # 本文件
└── .gitignore
```

---

## 🚀 快速开始

### 1. 环境准备

确保已安装以下工具：
- [ESP-IDF v5.5](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/get-started/)
- Python 3.8 或更高版本
- CMake 3.16 或更高版本

### 2. 克隆项目

```bash
git clone <repository-url>
cd SSD1680-BWR-ESP32S3
```

### 3. 配置项目

```bash
# 设置目标芯片为 ESP32-S3
idf.py set-target esp32s3

# 配置菜单（可选）
idf.py menuconfig
```

### 4. 导入字体数据

字体数据需要用户自行导入，详见 [字体数据导入](#-字体数据导入) 章节。

### 5. 编译和烧录

```bash
# 编译项目
idf.py build

# 烧录到开发板（替换 COM3 为实际端口）
idf.py -p COM3 flash

# 监视串口输出
idf.py -p COM3 monitor
```

---

## 📦 组件说明

### 1. epd_driver - 驱动层组件

**功能**: SSD1680 硬件驱动，提供底层设备控制接口。

**核心接口**:
```c
// 初始化设备
epd_handle_t* epd_init(const epd_config_t *config);

// 反初始化设备
esp_err_t epd_deinit(epd_handle_t *handle);

// 显示刷新
esp_err_t epd_display(epd_handle_t *handle, 
                      const uint8_t *buffer_bw, 
                      const uint8_t *buffer_red);

// 清屏
esp_err_t epd_clear(epd_handle_t *handle);

// 进入深度睡眠
esp_err_t epd_deep_sleep(epd_handle_t *handle);
```

**特性**:
- 设备句柄管理，支持多设备
- 可配置引脚和 SPI 参数
- 完整的错误处理和日志输出

### 2. epd_gui - GUI 层组件

**功能**: 提供画布管理、图形绘制、文本渲染功能。

**核心接口**:
```c
// 创建画布
epd_canvas_t* epd_canvas_create(uint8_t *buffer_bw, uint8_t *buffer_red, 
                                uint16_t width, uint16_t height);

// 绘制图形
esp_err_t epd_draw_line(epd_canvas_t *canvas, ...);
esp_err_t epd_draw_rectangle(epd_canvas_t *canvas, ...);
esp_err_t epd_draw_circle(epd_canvas_t *canvas, ...);
esp_err_t epd_draw_triangle(epd_canvas_t *canvas, ...);

// 文本渲染
esp_err_t epd_show_string(epd_canvas_t *canvas, ...);
esp_err_t epd_show_string_wrap(epd_canvas_t *canvas, ...);  // 智能换行
```

**特性**:
- 双缓冲区管理
- 自动坐标转换（用户坐标 ↔ SSD1680 内部坐标）
- Bresenham 直线算法、中点圆算法
- 单词级别智能换行

### 3. epd_font - 字体层组件

**功能**: 字体管理和字模数据组织。

**核心接口**:
```c
// 获取字体对象
const epd_font_t* epd_font_get(epd_font_size_t size);

// 计算字符串宽度
uint16_t epd_font_get_string_width(const epd_font_t *font, const char *str);
```

**支持字体**:
- 6×8 像素
- 6×12 像素
- 8×16 像素
- 12×24 像素

**特性**:
- 字模数据外置，用户自行导入
- 二维数组格式，与原 STM32 项目完全兼容
- 支持字体数据动态加载

---

## 💡 使用示例

### 基础显示示例

```c
#include "epd_driver.h"
#include "epd_gui.h"
#include "epd_font.h"

void app_main(void)
{
    epd_handle_t *epd;
    epd_canvas_t *canvas;
    uint8_t *buffer_bw;
    uint8_t *buffer_red;
    const epd_font_t *font;
    
    // 1. 初始化设备（使用默认配置）
    epd = epd_init(NULL);
    if (epd == NULL) {
        ESP_LOGE(TAG, "初始化失败");
        return;
    }
    
    // 2. 获取缓冲区
    buffer_bw = epd->buffer_bw;
    buffer_red = epd->buffer_red;
    
    // 3. 创建画布
    canvas = epd_canvas_create(buffer_bw, buffer_red, 296, 152);
    
    // 4. 清屏（白色）
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    epd_display(epd, buffer_bw, buffer_red);
    
    // 5. 绘制黑色边框
    epd_draw_rectangle(canvas, 0, 0, 295, 151, EPD_COLOR_BLACK, false);
    
    // 6. 绘制红色实心圆
    epd_draw_circle(canvas, 100, 50, 30, EPD_COLOR_RED, true);
    
    // 7. 显示文本
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    epd_show_string(canvas, 10, 10, "Hello World!", font, EPD_COLOR_BLACK);
    
    // 8. 刷新显示
    epd_display(epd, buffer_bw, buffer_red);
    
    // 9. 进入深度睡眠
    epd_deep_sleep(epd);
    
    // 注意：短生命周期应用可不释放资源
}
```

### 三色显示示例

```c
// 黑色文字（用于标题、边框等）
epd_show_string(canvas, 10, 10, "Title", font, EPD_COLOR_BLACK);

// 红色文字（用于强调、警告等）
epd_show_string(canvas, 10, 40, "Warning!", font, EPD_COLOR_RED);

// 混合使用
epd_draw_line(canvas, 0, 0, 295, 151, EPD_COLOR_BLACK);   // 黑色对角线
epd_draw_line(canvas, 295, 0, 0, 151, EPD_COLOR_RED);     // 红色对角线
```

### 智能换行示例

```c
// 自动换行文本（宽度 276 像素，高度 80 像素）
epd_show_string_wrap(canvas, 10, 60, 
                     "This is a long text that will wrap to multiple lines automatically.",
                     font, EPD_COLOR_BLACK, 276, 80);

// 强制换行
epd_show_string(canvas, 10, 100, "Line 1\nLine 2", font, EPD_COLOR_BLACK);
```

---

## 📖 API 文档

### 驱动层 API (epd_driver.h)

#### 设备管理
| 函数 | 说明 | 返回值 |
|-----|------|--------|
| `epd_init(config)` | 初始化设备 | `epd_handle_t*` 设备句柄 |
| `epd_deinit(handle)` | 反初始化设备 | `esp_err_t` |
| `epd_get_handle()` | 获取全局设备句柄 | `epd_handle_t*` |

#### 显示控制
| 函数 | 说明 | 返回值 |
|-----|------|--------|
| `epd_display(handle, bw, red)` | 写入显存并刷新 | `esp_err_t` |
| `epd_clear(handle)` | 清屏（白色） | `esp_err_t` |
| `epd_update(handle)` | 刷新显示 | `esp_err_t` |
| `epd_deep_sleep(handle)` | 进入深度睡眠 | `esp_err_t` |
| `epd_wake_up(handle)` | 唤醒设备 | `esp_err_t` |

### GUI 层 API (epd_gui.h)

#### 画布管理
| 函数 | 说明 | 返回值 |
|-----|------|--------|
| `epd_canvas_create(bw, red, w, h)` | 创建画布 | `epd_canvas_t*` |
| `epd_canvas_destroy(canvas)` | 销毁画布 | `void` |
| `epd_canvas_clear(canvas, color)` | 清空画布 | `esp_err_t` |

#### 绘图函数
| 函数 | 说明 | 返回值 |
|-----|------|--------|
| `epd_draw_line(canvas, x1, y1, x2, y2, color)` | 绘制直线 | `esp_err_t` |
| `epd_draw_rectangle(canvas, x1, y1, x2, y2, color, filled)` | 绘制矩形 | `esp_err_t` |
| `epd_draw_circle(canvas, cx, cy, r, color, filled)` | 绘制圆形 | `esp_err_t` |
| `epd_draw_triangle(canvas, x1, y1, x2, y2, x3, y3, color, filled)` | 绘制三角形 | `esp_err_t` |

#### 文本渲染
| 函数 | 说明 | 返回值 |
|-----|------|--------|
| `epd_show_char(canvas, x, y, ch, font, color)` | 显示字符 | `esp_err_t` |
| `epd_show_string(canvas, x, y, str, font, color)` | 显示字符串 | `esp_err_t` |
| `epd_show_string_wrap(canvas, x, y, str, font, color, max_w, max_h)` | 智能换行 | `esp_err_t` |
| `epd_show_num(canvas, x, y, num, font, color)` | 显示数字 | `esp_err_t` |
| `epd_get_string_width(str, font)` | 计算字符串宽度 | `uint16_t` |
| `epd_get_string_height(str, font, max_w)` | 计算字符串高度 | `uint16_t` |

### 字体层 API (epd_font.h)

| 函数 | 说明 | 返回值 |
|-----|------|--------|
| `epd_font_get(size)` | 获取字体对象 | `const epd_font_t*` |
| `epd_font_get_width(font)` | 获取字体宽度 | `uint8_t` |
| `epd_font_get_height(font)` | 获取字体高度 | `uint8_t` |
| `epd_font_get_string_width(font, str)` | 计算字符串宽度 | `uint16_t` |

---

## 🔤 字体数据导入

### 步骤说明

1. **打开字体数据模板文件**
   
   编辑 `components/epd_font/src/epd_font_data.c`

2. **从取模软件复制字模数据**

```c
// 示例：复制 8×16 字体数据
const unsigned char g_font_8x16_data[95][16] = {
    /* 从取模软件复制 */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 空格 (0x20)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // '!' (0x21)
    // ... 继续复制所有 95 个字符 (0x20-0x7E)
};
```

3. **字模格式要求**

- **点阵格式**: 阴码
- **取模方式**: 逐行式
- **取模走向**: 顺向（高位在前）
- **ASCII 码格式**: C51

4. **编译验证**

```bash
idf.py build
```

### 字体数据说明

| 字体大小 | 数组名称 | 数据大小 | 字符范围 |
|---------|---------|---------|---------|
| 6×8 | `g_font_6x8_data` | 766 字节 | 0x20-0x7E (95 字符) |
| 6×12 | `g_font_6x12_data` | 1146 字节 | 0x20-0x7E (95 字符) |
| 8×16 | `g_font_8x16_data` | 1526 字节 | 0x20-0x7E (95 字符) |
| 12×24 | `g_font_12x24_data` | 4566 字节 | 0x20-0x7E (95 字符) |

---

## 🧪 测试说明

### 运行所有测试

```bash
# 烧录并运行测试
idf.py -p COM3 flash monitor
```

### 运行特定测试（标签过滤）

```bash
# 只运行画布管理测试
idf.py -p COM3 flash monitor --test-filter "canvas"

# 只运行绘图算法测试
idf.py -p COM3 flash monitor --test-filter "geometry"

# 只运行文本渲染测试
idf.py -p COM3 flash monitor --test-filter "text"

# 只运行字体管理测试
idf.py -p COM3 flash monitor --test-filter "epd_font"
```

### 测试用例统计

| 测试类别 | 测试用例数 | 验证点 |
|---------|-----------|--------|
| 画布管理 | 4 | 15 |
| 绘图算法 | 4 | 16 |
| 文本渲染 | 6 | 30 |
| 字体管理 | 1 | 5 |
| **总计** | **15** | **66** |

### 配置测试模式

在 `main/main.c` 中配置测试模式：

```c
#define ENABLE_BASIC_TEST       0   // 基础测试
#define ENABLE_GEOMETRY_TEST    1   // 几何图形测试
#define ENABLE_FONT_TEST        0   // 字体测试
```

---

## ❓ 常见问题

### Q1: 编译时提示缺少字体数据符号

**A**: 需要导入字体数据。请参考 [字体数据导入](#-字体数据导入) 章节。

### Q2: 显示内容位置颠倒

**A**: 检查坐标转换逻辑。项目已实现自动坐标转换（用户坐标 ↔ SSD1680 内部坐标），确保使用正确的 API 函数。

### Q3: 字符之间出现黑线

**A**: 字符间距过小导致。已在 `epd_show_string()` 函数中增加像素间距，至少为 1 （`offset_x += font->width + 1`）。

### Q4: 红色显示异常

**A**: SSD1680 红色显存数据需要取反。驱动层已自动处理，用户无需手动取反。

### Q5: 如何只使用黑白显示？

**A**: 创建画布时红色缓冲区传入 NULL：

```c
canvas = epd_canvas_create(buffer_bw, NULL, 296, 152);
```

### Q6: 支持局部刷新吗？

**A**: 不支持。SSD1680 三色墨水屏硬件不支持局部刷新，每次刷新都需要全屏更新（约 30 秒）。

---

## 📚 文档资源

### 项目文档

- [问题修复记录](docs/问题修复记录.md) - 完整的问题修复历史和技术细节

### 官方文档

- [SSD1680 数据手册](https://www.e-paper-display.com/products_detail/productId=397.html)
- [ESP-IDF v5.5 文档](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/)
- [ESP32-S3 技术参考手册](https://www.espressif.com.cn/sites/default/files/documentation/esp32-s3_technical_reference_manual_cn.pdf)

### 相关资源

- [Unity 测试框架](https://github.com/ThrowTheSwitch/Unity)
- [ESP-IDF 组件系统](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/api-guides/build-system.html#components)

---

## 📄 许可证

本项目采用 Apache 2.0 许可证。详见 [LICENSE](LICENSE) 文件。

**许可证要点**：
- ✅ 商业使用：允许
- ✅ 修改：允许
- ✅ 分发：允许
- ✅ 专利使用：允许
- ✅ 私人使用：允许
- ⚠️ 许可证和版权声明：必须保留
- ⚠️ 状态变更说明：必须标注
- ⚠️ 商标使用：不允许

详见：[http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

---

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

### 贡献指南

1. Fork 本项目
2. 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建 Pull Request

---

## 📧 联系方式

如有问题或建议，请通过以下方式联系：

- 提交 Issue
- 发送邮件至项目维护者

---

## 📊 项目统计

- **代码行数**: ~3245 行（不含注释）
- **文档行数**: ~5000+ 行
- **测试用例**: 15 个
- **验证点**: 66 个
- **Git 提交**: 10+ 次

---

**最后更新**: 2026-03-03  
**项目版本**: v1.0  
**ESP-IDF 版本**: v5.5  
**硬件平台**: ESP32-S3 + SSD1680

---

<div align="center">

**如果这个项目对你有帮助，请给一个 ⭐ Star！**

Made with ❤️ by AI Assistant

</div>
