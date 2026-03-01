# 字体配置最终说明

## 📋 配置更新

**更新日期**: 2026-02-28  
**更新内容**: 移除 8×8 和 16×16 字体，仅保留 4 种列行式字体

---

## ✅ 当前支持的字体

### 字体列表（4 种）

| 枚举值 | 字体名称 | 规格 | 每字符字节 | 总大小 | 数组名称 |
|--------|---------|------|-----------|--------|---------|
| `EPD_FONT_SIZE_8X6` | asc2_0806 | 8×6 | 6 字节 | 570 字节 | `asc2_0806[]` |
| `EPD_FONT_SIZE_12X6` | asc2_1206 | 12×6 | 9 字节 | 855 字节 | `asc2_1206[]` |
| `EPD_FONT_SIZE_16X8` | asc2_1608 | 16×8 | 16 字节 | 1,520 字节 | `asc2_1608[]` |
| `EPD_FONT_SIZE_24X12` | asc2_2412 | 24×12 | 36 字节 | 3,420 字节 | `asc2_2412[]` |

**总计**: 4 种字体，6,365 字节（约 6.2KB）

---

## 🗑️ 已删除的字体

以下字体已移除：

| 字体名称 | 规格 | 原因 |
|---------|------|------|
| ~~8×8~~ | 8×8 | 用户要求删除 |
| ~~16×16~~ | 16×16 | 用户要求删除 |

---

## 🔧 使用方法

### 基础示例

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

---

## 📊 字体特性对比

### 8×6 字体 (asc2_0806)

**特点**:
- 超紧凑设计
- 宽度 8 像素，高度 6 像素
- 每字符仅 6 字节
- 适合显示密集数据

**适用场景**:
- 传感器数据显示
- 状态信息
- 多列数据表格

### 12×6 字体 (asc2_1206)

**特点**:
- 窄高设计
- 宽度 12 像素，高度 6 像素
- 每字符 9 字节
- 横向空间充足，纵向受限

**适用场景**:
- 宽屏布局
- 多列显示
- 横向滚动文本

### 16×8 字体 (asc2_1608)

**特点**:
- 标准宽度字体
- 宽度 16 像素，高度 8 像素
- 每字符 16 字节
- 可读性好，通用性强

**适用场景**:
- 通用文本显示
- 菜单界面
- 提示信息

### 24×12 字体 (asc2_2412)

**特点**:
- 大标题字体
- 宽度 24 像素，高度 12 像素
- 每字符 36 字节
- 醒目清晰，远距离可读

**适用场景**:
- 页面标题
- 重要警告
- 大字号显示

---

## 📁 修改的文件

### 1. epd_font.h

**修改内容**:
- ✅ 删除 `EPD_FONT_SIZE_8X8` 枚举
- ✅ 删除 `EPD_FONT_SIZE_16X16` 枚举
- ✅ 更新枚举起始值（`EPD_FONT_SIZE_8X6 = 0`）
- ✅ 添加详细注释说明

**代码变化**:
```c
typedef enum {
    EPD_FONT_SIZE_8X6 = 0,        // ✅ 新起始值
    EPD_FONT_SIZE_12X6,
    EPD_FONT_SIZE_16X8,
    EPD_FONT_SIZE_24X12
} epd_font_size_t;
```

### 2. epd_font.c

**修改内容**:
- ✅ 删除 `ascii_font_8x8[]` 声明
- ✅ 删除 `ascii_font_16x16[]` 声明
- ✅ 删除 `g_font_8x8` 对象
- ✅ 删除 `g_font_16x16` 对象
- ✅ 更新 `epd_font_get()` 函数

**代码变化**:
```c
// ✅ 删除的内容
extern const uint8_t ascii_font_8x8[];
extern const uint8_t ascii_font_16x16[];
static const epd_font_t g_font_8x8;
static const epd_font_t g_font_16x16;

// ✅ 保留的内容
extern const uint8_t asc2_0806[];
extern const uint8_t asc2_1206[];
extern const uint8_t asc2_1608[];
extern const uint8_t asc2_2412[];

const epd_font_t* epd_font_get(epd_font_size_t size)
{
    switch (size) {
    case EPD_FONT_SIZE_8X6:      // ✅ 起始 case
        return &g_font_8x6;
    case EPD_FONT_SIZE_12X6:
        return &g_font_12x6;
    case EPD_FONT_SIZE_16X8:
        return &g_font_16x8;
    case EPD_FONT_SIZE_24X12:
        return &g_font_24x12;
    default:
        return NULL;
    }
}
```

---

## ⚠️ 重要提示

### 1. 字模数据仍需导入

**当前状态**: 
- ✅ 接口定义完成
- ✅ 字体对象定义完成
- ⚠️ **字模数据未导入**（只有 `extern` 声明）

**必须完成**:
```c
// epd_font.c 中需要导入实际数据
const uint8_t asc2_0806[] = {
    // 从 STM32 项目复制 570 字节数据
};

const uint8_t asc2_1206[] = {
    // 从 STM32 项目复制 855 字节数据
};

const uint8_t asc2_1608[] = {
    // 从 STM32 项目复制 1520 字节数据
};

const uint8_t asc2_2412[] = {
    // 从 STM32 项目复制 3420 字节数据
};
```

### 2. 兼容性影响

**破坏性变更**:
- ❌ 原有使用 `EPD_FONT_SIZE_8X8` 的代码将编译失败
- ❌ 原有使用 `EPD_FONT_SIZE_16X16` 的代码将编译失败

**迁移方案**:
```c
// ❌ 旧代码
font = epd_font_get(EPD_FONT_SIZE_8X8);

// ✅ 新代码（选择最接近的字体）
font = epd_font_get(EPD_FONT_SIZE_16X8);  // 或其他字体
```

### 3. 临时测试

如需临时测试（不导入字模），可创建空数组：

```c
// epd_font.c - 仅用于测试
const uint8_t asc2_0806[570] = {0};
const uint8_t asc2_1206[855] = {0};
const uint8_t asc2_1608[1520] = {0};
const uint8_t asc2_2412[3420] = {0};
```

---

## 📚 相关文档

| 文档 | 用途 |
|------|------|
| [`FONT_IMPORT_COLUMN_ROW.md`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\FONT_IMPORT_COLUMN_ROW.md) | 列行式字体导入指南 |
| [`epd_font.h`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\epd_font.h) | 字体头文件（已更新） |
| [`epd_font.c`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\epd_font.c) | 字体实现（已更新） |
| [`README_GUI.md`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\README_GUI.md) | GUI 快速参考 |

---

## ✅ 检查清单

### 代码更新
- [x] 删除 `EPD_FONT_SIZE_8X8` 枚举
- [x] 删除 `EPD_FONT_SIZE_16X16` 枚举
- [x] 删除 `ascii_font_8x8[]` 声明
- [x] 删除 `ascii_font_16x16[]` 声明
- [x] 删除 `g_font_8x8` 对象
- [x] 删除 `g_font_16x16` 对象
- [x] 更新 `epd_font_get()` 函数
- [x] 更新枚举起始值

### 文档更新
- [x] 创建配置说明文档
- [x] 更新使用示例
- [x] 添加迁移指南

### 待完成
- [ ] 导入 `asc2_0806[]` 字模数据
- [ ] 导入 `asc2_1206[]` 字模数据
- [ ] 导入 `asc2_1608[]` 字模数据
- [ ] 导入 `asc2_2412[]` 字模数据
- [ ] 编译验证 `idf.py build`
- [ ] 功能测试

---

## 🎯 下一步行动

1. **立即**：
   - 从 STM32 项目复制 4 种字模数据
   - 参考 [`FONT_IMPORT_COLUMN_ROW.md`](file://c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1\main\FONT_IMPORT_COLUMN_ROW.md)

2. **验证**：
   ```bash
   idf.py build
   idf.py test
   ```

3. **测试**：
   - 烧录到硬件
   - 测试各种字体显示效果

---

## 📊 资源统计

### 代码行数变化

| 文件 | 删除行数 | 保留行数 | 净变化 |
|------|---------|---------|--------|
| `epd_font.h` | -2 行 | 4 行 | -2 行 |
| `epd_font.c` | -60 行 | 4 行 | -56 行 |
| **总计** | **-62 行** | **8 行** | **-58 行** |

### 内存占用

**字库总大小**: 6,365 字节（约 6.2KB）
- Flash 占用：6.2KB
- SRAM 占用：0（字模存放在 Flash）

**对比原配置**：
- 原配置：10,165 字节（6 种字体）
- 现配置：6,365 字节（4 种字体）
- **节省**: 3,800 字节（约 3.7KB）

---

**更新状态**: ✅ **代码更新完成，待导入字模数据**  
**支持字体**: 4 种列行式（8×6, 12×6, 16×8, 24×12）  
**总大小**: 约 6.2KB  
**节省空间**: 约 3.7KB
