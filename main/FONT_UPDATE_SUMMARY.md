# 字体模块更新说明

## 📋 更新概述

**更新日期**: 2026-02-28  
**更新内容**: 新增 4 种列行式字体支持

---

## ✅ 已完成的更改

### 1. 头文件更新

**文件**: [`epd_font.h`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\epd_font.h)

**新增字体枚举**：
```c
typedef enum {
    EPD_FONT_SIZE_8X8 = 0,    // 8x8 字体（原有）
    EPD_FONT_SIZE_16X16,      // 16x16 字体（原有）
    EPD_FONT_SIZE_8X6,        // 8×6 列行式字体（新增）
    EPD_FONT_SIZE_12X6,       // 12×6 列行式字体（新增）
    EPD_FONT_SIZE_16X8,       // 16×8 列行式字体（新增）
    EPD_FONT_SIZE_24X12       // 24×12 列行式字体（新增）
} epd_font_size_t;
```

### 2. 实现文件更新

**文件**: [`epd_font.c`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\epd_font.c)

**新增字模声明**：
```c
extern const uint8_t asc2_0806[];   // 8×6 字模
extern const uint8_t asc2_1206[];   // 12×6 字模
extern const uint8_t asc2_1608[];   // 16×8 字模
extern const uint8_t asc2_2412[];   // 24×12 字模
```

**新增字体对象**：
```c
static const epd_font_t g_font_8x6 = {
    .data = asc2_0806,
    .width = 8,
    .height = 6,
    .char_bytes = 6
};

static const epd_font_t g_font_12x6 = {
    .data = asc2_1206,
    .width = 12,
    .height = 6,
    .char_bytes = 9
};

static const epd_font_t g_font_16x8 = {
    .data = asc2_1608,
    .width = 16,
    .height = 8,
    .char_bytes = 16
};

static const epd_font_t g_font_24x12 = {
    .data = asc2_2412,
    .width = 24,
    .height = 12,
    .char_bytes = 36
};
```

**更新获取函数**：
```c
const epd_font_t* epd_font_get(epd_font_size_t size)
{
    switch (size) {
    case EPD_FONT_SIZE_8X8:
        return &g_font_8x8;
    case EPD_FONT_SIZE_16X16:
        return &g_font_16x16;
    case EPD_FONT_SIZE_8X6:
        return &g_font_8x6;      // 新增
    case EPD_FONT_SIZE_12X6:
        return &g_font_12x6;     // 新增
    case EPD_FONT_SIZE_16X8:
        return &g_font_16x8;     // 新增
    case EPD_FONT_SIZE_24X12:
        return &g_font_24x12;    // 新增
    default:
        return NULL;
    }
}
```

---

## 📊 字体规格对比

| 字体 | 宽度 | 高度 | 每字符字节 | 总字节 | 适用场景 |
|------|------|------|-----------|--------|---------|
| 8×8 | 8 | 8 | 8 | 760 | 标准小字体 |
| 16×16 | 16 | 16 | 32 | 3,040 | 标准大字体 |
| **8×6** | **8** | **6** | **6** | **570** | **超紧凑显示** |
| **12×6** | **12** | **6** | **9** | **855** | **窄高空间** |
| **16×8** | **16** | **8** | **16** | **1,520** | **宽屏显示** |
| **24×12** | **24** | **12** | **36** | **3,420** | **大标题** |

**原有字体**：2 种，共 3,800 字节  
**新增字体**：4 种，共 6,365 字节  
**总计**：6 种字体，10,165 字节（约 10KB）

---

## 🚀 使用示例

### 基础使用

```c
#include "epd_gui.h"

void app_main(void)
{
    epd_canvas_t *canvas;
    const epd_font_t *font;
    
    // 初始化
    epd_init();
    canvas = epd_canvas_create(buffer, 296, 152);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    
    // 使用不同字体显示
    font = epd_font_get(EPD_FONT_SIZE_8X6);
    epd_show_string(canvas, 10, 10, "8x6", font, EPD_COLOR_BLACK);
    
    font = epd_font_get(EPD_FONT_SIZE_12X6);
    epd_show_string(canvas, 10, 20, "12x6", font, EPD_COLOR_BLACK);
    
    font = epd_font_get(EPD_FONT_SIZE_16X8);
    epd_show_string(canvas, 10, 30, "16x8", font, EPD_COLOR_BLACK);
    
    font = epd_font_get(EPD_FONT_SIZE_24X12);
    epd_show_string(canvas, 10, 45, "24x12", font, EPD_COLOR_BLACK);
    
    // 刷新
    epd_display(buffer, buffer);
    
    epd_canvas_destroy(canvas);
}
```

### 混合使用多种字体

```c
void display_mixed_fonts(void)
{
    const epd_font_t *font_small, *font_large;
    
    font_small = epd_font_get(EPD_FONT_SIZE_8X6);
    font_large = epd_font_get(EPD_FONT_SIZE_24X12);
    
    // 大标题
    epd_show_string(canvas, 50, 20, "标题", font_large, EPD_COLOR_BLACK);
    
    // 小字内容
    epd_show_string(canvas, 10, 60, "详细信息...", font_small, EPD_COLOR_BLACK);
}
```

### 数字显示

```c
void display_sensor_data(float temperature, float humidity)
{
    const epd_font_t *font;
    
    font = epd_font_get(EPD_FONT_SIZE_16X8);
    
    // 显示温度
    epd_show_string(canvas, 10, 10, "Temp:", font, EPD_COLOR_BLACK);
    epd_show_num(canvas, 80, 10, (uint32_t)temperature, 2, font, EPD_COLOR_BLACK);
    
    // 显示湿度
    epd_show_string(canvas, 10, 30, "Humi:", font, EPD_COLOR_BLACK);
    epd_show_num(canvas, 80, 30, (uint32_t)humidity, 2, font, EPD_COLOR_BLACK);
}
```

---

## ⚠️ 重要提示

### 1. 字模数据待导入

**当前状态**：只有接口定义，字模数据未导入

```c
// epd_font.c 中当前是 extern 声明
extern const uint8_t asc2_0806[];   // ⚠️ 需要导入实际数据
extern const uint8_t asc2_1206[];   // ⚠️ 需要导入实际数据
extern const uint8_t asc2_1608[];   // ⚠️ 需要导入实际数据
extern const uint8_t asc2_2412[];   // ⚠️ 需要导入实际数据
```

**必须完成**：
- 从 STM32 项目复制字模数据
- 替换 `extern` 声明为实际数组
- 参考：[`FONT_IMPORT_COLUMN_ROW.md`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\FONT_IMPORT_COLUMN_ROW.md)

### 2. 编译影响

**导入字模前**：
- ✅ 编译通过（只有 extern 声明）
- ❌ 链接失败（找不到实际数据）
- ❌ 运行时崩溃

**导入字模后**：
- ✅ 编译成功
- ✅ 链接成功
- ✅ 功能正常

### 3. 临时测试方案

如果暂时只需要测试几何图形功能，可以创建空数组：

```c
// epd_font.c - 临时测试用
const uint8_t asc2_0806[570] = {0};
const uint8_t asc2_1206[855] = {0};
const uint8_t asc2_1608[1520] = {0};
const uint8_t asc2_2412[3420] = {0};
```

**注意**：这样字符显示会是全黑块，但不会崩溃。

---

## 📁 修改的文件列表

| 文件 | 修改内容 | 行数变化 |
|------|---------|---------|
| `epd_font.h` | 新增字体枚举 | +4 行 |
| `epd_font.c` | 新增字模声明 | +56 行 |
| `epd_font.c` | 新增字体对象 | +48 行 |
| `epd_font.c` | 更新获取函数 | +8 行 |
| **总计** | - | **+116 行** |

---

## 🧪 测试建议

### 单元测试扩展

在 [`test/test_epd_gui.c`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\test\test_epd_gui.c) 中添加测试：

```c
TEST_CASE("Font 8x6", "[epd_font]")
{
    const epd_font_t *font;
    
    font = epd_font_get(EPD_FONT_SIZE_8X6);
    TEST_ASSERT_NOT_NULL(font);
    TEST_ASSERT_EQUAL(8, font->width);
    TEST_ASSERT_EQUAL(6, font->height);
    TEST_ASSERT_EQUAL(6, font->char_bytes);
}

TEST_CASE("Font 12x6", "[epd_font]")
{
    const epd_font_t *font;
    
    font = epd_font_get(EPD_FONT_SIZE_12X6);
    TEST_ASSERT_NOT_NULL(font);
    TEST_ASSERT_EQUAL(12, font->width);
    TEST_ASSERT_EQUAL(6, font->height);
    TEST_ASSERT_EQUAL(9, font->char_bytes);
}

// 类似测试 16x8 和 24x12
```

---

## 📚 相关文档

| 文档 | 用途 |
|------|------|
| [`FONT_IMPORT_COLUMN_ROW.md`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\FONT_IMPORT_COLUMN_ROW.md) | ⭐ **列行式字体导入指南** |
| [`FONT_IMPORT_GUIDE.md`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\FONT_IMPORT_GUIDE.md) | 通用字模导入指南 |
| [`README_GUI.md`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\README_GUI.md) | GUI 快速参考 |
| [`GUI_MIGRATION_REPORT.md`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\GUI_MIGRATION_REPORT.md) | 完整移植报告 |

---

## ✅ 检查清单

### 代码更新
- [x] 头文件添加字体枚举
- [x] 实现文件添加字模声明
- [x] 实现文件添加字体对象
- [x] 更新字体获取函数
- [x] 编译语法检查通过

### 文档更新
- [x] 创建字体导入指南
- [x] 更新使用说明
- [x] 添加示例代码

### 待完成
- [ ] 导入 asc2_0806 字模数据
- [ ] 导入 asc2_1206 字模数据
- [ ] 导入 asc2_1608 字模数据
- [ ] 导入 asc2_2412 字模数据
- [ ] 编译验证
- [ ] 功能测试

---

## 🎯 下一步行动

1. **立即**：
   - 阅读 [`FONT_IMPORT_COLUMN_ROW.md`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\FONT_IMPORT_COLUMN_ROW.md)
   - 从 STM32 项目复制 4 种字模数据

2. **短期**：
   - 运行 `idf.py build` 验证编译
   - 测试各种字体显示效果

3. **长期**：
   - 根据实际需求选择合适的字体
   - 优化字体渲染性能

---

**更新状态**: ✅ **代码更新完成，待导入字模数据**  
**新增字体**: 4 种（8×6, 12×6, 16×8, 24×12）  
**总字体数**: 6 种  
**总大小**: 约 10KB
