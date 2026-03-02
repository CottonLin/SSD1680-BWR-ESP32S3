# EPD 测试组件

## 概述

本组件提供 SSD1680 GUI 和字体系统的单元测试，基于 ESP-IDF Unity 测试框架。

## 测试覆盖范围

### 画布管理测试 (`[epd_gui][canvas]`)
- 画布创建（双缓冲区、单缓冲区）
- 画布销毁（正常销毁、重复销毁、NULL 处理）
- 画布清屏（黑白/红色、全白/全黑/全红/无红）
- 像素设置（黑白/红色、边界像素、参数验证）

### 绘图算法测试 (`[epd_gui][geometry]`)
- 直线绘制（水平、垂直、对角、任意角度）
- 矩形绘制（空心、实心、小矩形、大矩形）
- 圆形绘制（空心、实心、小圆、大圆）
- 三角形绘制（空心、实心、等边、直角）

### 文本渲染测试 (`[epd_gui][text]`)
- 字符显示（基本字符、特殊字符、边界字符、不同字体）
- 字符串显示（短字符串、长字符串、空字符串）
- 智能换行（短文本、长文本、单词换行、强制换行）
- 数字显示（正数、负数、零、大数）
- 字符串宽度/高度计算

### 字体管理测试 (`[epd_font]`)
- 字体获取（6x8、6x12、8x16、12x24、无效字体）
- 字体数据结构验证

## 编译和运行测试

### 前提条件

1. ESP-IDF 环境已安装（v4.4 或更高版本）
2. 项目依赖已正确配置
3. 目标硬件为 ESP32-S3

### 编译测试

```bash
# 进入项目根目录
cd c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680

# 设置目标芯片
idf.py set-target esp32s3

# 构建项目（包含测试组件）
idf.py build
```

### 运行测试

#### 方法 1：通过串口监视器运行所有测试

```bash
# 烧录并运行
idf.py -p COM3 flash monitor
```

测试将自动运行，输出类似：

```
I (0) EPD_TESTS: === 开始 EPD GUI 和字体系统测试 ===
...
TEST_CASE: Canvas create [epd_gui][canvas]
  PASS: Test 1: Create dual-buffer canvas
  PASS: Test 2: Create single BW buffer canvas
  PASS: Test 3: Create single RED buffer canvas
  PASS: Test 4: NULL buffer pointer should fail
...
```

#### 方法 2：运行特定测试标签

在 `main/main.c` 中配置测试标签过滤（如果支持）：

```bash
# 只运行画布管理测试
idf.py -p COM3 flash monitor --test-filter "canvas"

# 只运行文本渲染测试
idf.py -p COM3 flash monitor --test-filter "text"

# 只运行字体管理测试
idf.py -p COM3 flash monitor --test-filter "epd_font"
```

#### 方法 3：通过菜单选择测试

如果启用了 Unity 测试菜单，将在串口控制台显示测试菜单：

```
Unity test runner menu
======================
u) Run all tests
g) Run GUI tests
f) Run font tests
c) Run canvas tests
t) Run text tests
q) Quit
```

## 测试用例列表

### 画布管理测试

| 测试用例 | 标签 | 验证点 |
|---------|------|--------|
| Canvas create | `[epd_gui][canvas]` | 双缓冲区、单缓冲区创建 |
| Canvas destroy | `[epd_gui][canvas]` | 正常销毁、重复销毁、NULL 处理 |
| Canvas clear | `[epd_gui][canvas]` | 黑白/红色清屏、参数验证 |
| Canvas set pixel | `[epd_gui][canvas]` | 像素设置、边界像素、参数验证 |

### 绘图算法测试

| 测试用例 | 标签 | 验证点 |
|---------|------|--------|
| Draw line | `[epd_gui][geometry]` | 水平、垂直、对角直线 |
| Draw rectangle | `[epd_gui][geometry]` | 空心、实心矩形 |
| Draw circle | `[epd_gui][geometry]` | 空心、实心圆形 |
| Draw triangle | `[epd_gui][geometry]` | 空心、实心三角形 |

### 文本渲染测试

| 测试用例 | 标签 | 验证点 |
|---------|------|--------|
| Show character | `[epd_gui][text]` | 基本字符、特殊字符、边界字符 |
| Show string | `[epd_gui][text]` | 短字符串、长字符串、空字符串 |
| Show string with wrap | `[epd_gui][text]` | 智能换行、强制换行 |
| Show number | `[epd_gui][text]` | 正数、负数、零、大数 |
| Get string width | `[epd_gui][text]` | 空字符串、单字符、多字符 |
| Get string height | `[epd_gui][text]` | 单行、多行、不同字体 |

### 字体管理测试

| 测试用例 | 标签 | 验证点 |
|---------|------|--------|
| Get font | `[epd_font]` | 4 种字体大小、无效字体 |

## 测试验证标准

### 通过标准

- 所有测试用例 PASS
- 无内存泄漏
- 无崩溃或异常
- 参数验证正确（返回 `ESP_ERR_INVALID_ARG`）
- 功能与预期一致

### 失败处理

如果测试失败：

1. 检查失败输出日志
2. 定位失败的测试用例
3. 分析失败原因（参数错误、逻辑错误、硬件问题）
4. 修复代码后重新运行测试

## 添加新测试用例

### 测试用例模板

```c
TEST_CASE("Test name", "[tag1][tag2]")
{
    // 准备工作
    epd_canvas_t *canvas;
    esp_err_t ret;
    
    // 测试 1: 描述
    ESP_LOGI(TAG, "Test 1: Description");
    canvas = epd_canvas_create(EPD_BUFFER_DUAL);
    TEST_ASSERT_NOT_NULL(canvas);
    
    // 执行操作
    ret = some_function(canvas, ...);
    
    // 验证结果
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    // 清理资源
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 参数验证
    ESP_LOGI(TAG, "Test 2: Parameter validation");
    ret = some_function(NULL, ...);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
}
```

### 常用断言宏

```c
TEST_ASSERT(condition)              // 验证条件为真
TEST_ASSERT_TRUE(condition)         // 验证条件为真
TEST_ASSERT_FALSE(condition)        // 验证条件为假
TEST_ASSERT_NULL(ptr)               // 验证指针为 NULL
TEST_ASSERT_NOT_NULL(ptr)           // 验证指针不为 NULL
TEST_ASSERT_EQUAL(expected, actual) // 验证相等（整数）
TEST_ASSERT_EQUAL_HEX8(exp, act)    // 验证相等（16 进制）
TEST_ASSERT_EQUAL_STRING(exp, act)  // 验证字符串相等
```

## 性能基准

### 内存使用

- 画布创建：动态分配 EPD_BUFFER_SIZE (5624 字节) × 缓冲区数量
- 测试执行：额外约 2KB 栈空间

### 执行时间

单个测试用例执行时间：< 10ms
完整测试套件执行时间：< 500ms

## 故障排查

### 常见问题

**Q1: 测试编译失败**
- 检查组件依赖是否正确配置
- 检查 `CMakeLists.txt` 中的 `REQUIRES` 是否包含所有依赖

**Q2: 测试运行时崩溃**
- 检查是否正确初始化了 EPD 驱动
- 检查内存分配是否成功
- 检查硬件连接是否正常

**Q3: 测试失败但功能正常**
- 检查测试逻辑是否正确
- 检查断言条件是否准确
- 检查是否存在硬件差异

## 参考资料

- [ESP-IDF 单元测试框架](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-tests.html)
- [Unity 测试框架文档](https://www.throwtheswitch.org/unity)
- [SSD1680 数据手册](https://www.e-paper-display.com/products_detail/productId=399.html)

## 维护者

- 创建日期：2026-03-03
- 最后更新：2026-03-03
- 版本：1.0
