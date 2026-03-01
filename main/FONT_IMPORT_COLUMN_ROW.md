# 列行式字体导入指南

## 📋 新增字体规格

已支持以下 4 种列行式字体（宽×高）：

| 字体名称 | 规格 | 每字符字节数 | 总大小 | 数组名称 |
|---------|------|------------|--------|---------|
| asc2_0806 | 8×6 | 6 字节 | 570 字节 | `asc2_0806[]` |
| asc2_1206 | 12×6 | 9 字节 | 855 字节 | `asc2_1206[]` |
| asc2_1608 | 16×8 | 16 字节 | 1,520 字节 | `asc2_1608[]` |
| asc2_2412 | 24×12 | 36 字节 | 3,420 字节 | `asc2_2412[]` |

**总计**：4 种字体共 6,365 字节（约 6.2KB）

---

## 🔍 字体格式说明

### 列行式 vs 行列式

**列行式（Column-Row）**：
- 字模数据按列组织
- 每列包含若干行数据
- 适合某些特定的 LCD 控制器

**行列式（Row-Column）**：
- 字模数据按行组织
- 每行包含若干列数据
- 常见的字模格式

### 字节计算方式

```
每字符字节数 = (宽度 × 高度) / 8

asc2_0806:  (8 × 6) / 8 = 6 字节
asc2_1206:  (12 × 6) / 8 = 9 字节（向上取整）
asc2_1608:  (16 × 8) / 8 = 16 字节
asc2_2412:  (24 × 12) / 8 = 36 字节
```

---

## 📥 导入步骤

### 步骤 1: 定位源字模数据

在 STM32 项目中查找字模文件：

```
EPD_STM32F103ZE_SPI/
└── HANDWARE/
    └── EPD/
        └── EPD_Font.h    ← 查找以下数组
```

搜索关键字：
- `asc2_0806`
- `asc2_1206`
- `asc2_1608`
- `asc2_2412`

### 步骤 2: 复制字模数据

#### 2.1 复制 asc2_0806

在 STM32 项目中找到：

```c
const unsigned char asc2_0806[] = {
    // 95 个字符 × 6 字节 = 570 字节
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 空格
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // !
    // ... 更多字符
};
```

**复制整个数组定义**。

#### 2.2 复制其他字体

同样复制：
- `asc2_1206[]` - 95 字符 × 9 字节
- `asc2_1608[]` - 95 字符 × 16 字节
- `asc2_2412[]` - 95 字符 × 36 字节

### 步骤 3: 粘贴到 ESP-IDF 项目

打开文件：`epd_font.c`

找到对应位置（约第 24 行之后）：

```c
/**
 * @brief 8×6 列行式 ASCII 字模数据 (asc2_0806)
 * ...
 */
extern const uint8_t asc2_0806[];  // ← 在这里添加实际数据
```

**替换 `extern` 声明为实际数据**：

```c
/**
 * @brief 8×6 列行式 ASCII 字模数据 (asc2_0806)
 */
const uint8_t asc2_0806[] = {
    // 在这里粘贴从 STM32 项目复制的 570 字节数据
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 空格
    // ... 粘贴所有数据
};
```

### 步骤 4: 重复操作

对其余 3 种字体重复步骤 3：

1. **asc2_1206** - 约第 40 行
2. **asc2_1608** - 约第 56 行
3. **asc2_2412** - 约第 72 行

### 步骤 5: 验证数据

确保：
- ✅ 数组名称正确
- ✅ 数据完整性（无遗漏）
- ✅ 字符顺序正确（从 0x20 开始）

---

## 🔧 使用方法

### 在代码中使用新字体

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
    
    // 使用 8×6 字体
    font = epd_font_get(EPD_FONT_SIZE_8X6);
    epd_show_string(canvas, 10, 10, "8x6 Font", font, EPD_COLOR_BLACK);
    
    // 使用 12×6 字体
    font = epd_font_get(EPD_FONT_SIZE_12X6);
    epd_show_string(canvas, 10, 20, "12x6 Font", font, EPD_COLOR_BLACK);
    
    // 使用 16×8 字体
    font = epd_font_get(EPD_FONT_SIZE_16X8);
    epd_show_string(canvas, 10, 30, "16x8 Font", font, EPD_COLOR_BLACK);
    
    // 使用 24×12 字体
    font = epd_font_get(EPD_FONT_SIZE_24X12);
    epd_show_string(canvas, 10, 45, "24x12 Font", font, EPD_COLOR_BLACK);
    
    // 刷新显示
    epd_display(buffer, buffer);
    
    epd_canvas_destroy(canvas);
}
```

### 字体大小对比

| 字体 | 宽度 | 高度 | 适用场景 |
|------|------|------|---------|
| 8×6 | 8 像素 | 6 像素 | 超小空间，数据显示 |
| 12×6 | 12 像素 | 6 像素 | 窄高空间 |
| 16×8 | 16 像素 | 8 像素 | 标准大小，通用 |
| 24×12 | 24 像素 | 12 像素 | 大标题，醒目文字 |

---

## ⚠️ 注意事项

### 1. 字模格式匹配

**重要**：确保 STM32 项目中的字模格式与 ESP-IDF 项目兼容。

**检查项**：
- 位顺序：高位在前（MSB First）
- 扫描方式：列行式
- 字符范围：0x20-0x7E（95 个字符）

### 2. 字节对齐

某些字体的宽度不是 8 的倍数（如 12 像素）：

```
asc2_1206: 宽度 12 像素
- 每行需要 2 字节（12/8 = 1.5，向上取整）
- 6 行 × 2 字节 = 12 字节？❌
- 实际：9 字节（特殊排列）
```

**解决**：直接使用 STM32 项目的原始数据，不要手动转换。

### 3. 内存占用

所有 4 种字体总大小：

```
asc2_0806:   570 字节
asc2_1206:   855 字节
asc2_1608: 1,520 字节
asc2_2412: 3,420 字节
-------------------
总计：    6,365 字节（约 6.2KB）
```

**影响**：
- Flash 占用：6.2KB
- 不影响 SRAM（字模存放在 Flash）
- ESP32-S3 完全可承受

---

## 🐛 常见问题

### Q1: 编译报错 "undefined reference to asc2_0806"

**原因**：只有 `extern` 声明，没有实际数据

**解决**：
```c
// ❌ 错误：只有声明
extern const uint8_t asc2_0806[];

// ✅ 正确：有实际数据
const uint8_t asc2_0806[] = {
    // ... 实际数据
};
```

### Q2: 显示的字符是乱码

**可能原因**：
1. 字模数据格式不匹配
2. 字符顺序错误
3. 字节数计算错误

**解决**：
- 确认字模是**列行式**格式
- 检查第一个字符（空格 0x20）的数据是否正确
- 验证总字节数

### Q3: 某些字体不显示

**原因**：未导入该字体的字模数据

**解决**：
- 检查 `epd_font.c` 中是否有所需字体的数组定义
- 确保数据完整（无遗漏）

### Q4: 字体大小与预期不符

**原因**：字体对象的 width/height 设置错误

**解决**：
```c
// 检查字体对象定义
static const epd_font_t g_font_12x6 = {
    .data = asc2_1206,
    .width = 12,    // ← 确认宽度
    .height = 6,    // ← 确认高度
    // ...
};
```

---

## 📊 字体数据验证

### 验证方法 1: 检查数组大小

```c
// 在 epd_font.c 中添加验证代码
#include <stdio.h>

void verify_fonts(void)
{
    printf("asc2_0806 size: %zu bytes (expected 570)\n", sizeof(asc2_0806));
    printf("asc2_1206 size: %zu bytes (expected 855)\n", sizeof(asc2_1206));
    printf("asc2_1608 size: %zu bytes (expected 1520)\n", sizeof(asc2_1608));
    printf("asc2_2412 size: %zu bytes (expected 3420)\n", sizeof(asc2_2412));
}
```

### 验证方法 2: 测试显示

```c
void test_fonts(void)
{
    epd_canvas_t *canvas = epd_canvas_create(buffer, 296, 152);
    const epd_font_t *font;
    
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    
    // 测试所有字体
    font = epd_font_get(EPD_FONT_SIZE_8X6);
    epd_show_string(canvas, 0, 0, "0123456789", font, EPD_COLOR_BLACK);
    
    font = epd_font_get(EPD_FONT_SIZE_12X6);
    epd_show_string(canvas, 0, 10, "0123456789", font, EPD_COLOR_BLACK);
    
    font = epd_font_get(EPD_FONT_SIZE_16X8);
    epd_show_string(canvas, 0, 20, "0123456789", font, EPD_COLOR_BLACK);
    
    font = epd_font_get(EPD_FONT_SIZE_24X12);
    epd_show_string(canvas, 0, 35, "0123456789", font, EPD_COLOR_BLACK);
    
    epd_display(buffer, buffer);
    epd_canvas_destroy(canvas);
}
```

---

## 📚 相关文件

- `epd_font.h` - 字体头文件（已添加枚举定义）
- `epd_font.c` - 字体实现（需导入字模数据）
- `epd_gui.c` - GUI 模块（支持所有字体）
- `EPD_STM32F103ZE_SPI/HANDWARE/EPD/EPD_Font.h` - STM32 原字模文件

---

## ✅ 导入检查清单

- [ ] 从 STM32 项目复制 `asc2_0806[]` 数据
- [ ] 从 STM32 项目复制 `asc2_1206[]` 数据
- [ ] 从 STM32 项目复制 `asc2_1608[]` 数据
- [ ] 从 STM32 项目复制 `asc2_2412[]` 数据
- [ ] 粘贴到 `epd_font.c` 替换 `extern` 声明
- [ ] 验证数组大小正确
- [ ] 编译测试 `idf.py build`
- [ ] 功能测试（显示字符）

---

**更新日期**: 2026-02-28  
**字体规格**: 8×6, 12×6, 16×8, 24×12  
**总大小**: 6,365 字节  
**状态**: ✅ 接口完成，待导入字模数据
