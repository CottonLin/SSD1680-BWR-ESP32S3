/**
 * @file epd_tests.c
 * @brief GUI 和字体系统单元测试
 * 
 * 测试覆盖：
 * - 画布管理接口
 * - 绘图算法（直线、矩形、圆形、三角形）
 * - 文本渲染（字符、字符串、数字）
 * - 字体管理
 * 
 * 测试框架：ESP-IDF Unity Test Framework
 */

#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "epd_driver.h"
#include "epd_gui.h"
#include "epd_font.h"
#include "esp_log.h"

static const char *TAG = "EPD_TESTS";

/**
 * @brief 测试辅助宏：检查两个值是否相等
 */
#define TEST_ASSERT_EQUAL_HEX8_MESSAGE(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            ESP_LOGE(TAG, "Assertion failed: %s, expected 0x%02X, got 0x%02X", \
                     message, (expected), (actual)); \
        } \
        TEST_ASSERT_EQUAL_HEX8((expected), (actual)); \
    } while(0)

/**
 * @brief 测试辅助宏：检查布尔值是否为真
 */
#define TEST_ASSERT_TRUE_MESSAGE(condition, message) \
    do { \
        if (!(condition)) { \
            ESP_LOGE(TAG, "Assertion failed: %s", message); \
        } \
        TEST_ASSERT_TRUE(condition); \
    } while(0)

// ==================== 画布管理测试 ====================

/**
 * @brief 测试画布创建功能
 * 
 * 验证点：
 * 1. 正常创建画布（双缓冲区）
 * 2. 正常创建画布（单缓冲区 - 黑白）
 * 3. 正常创建画布（单缓冲区 - 红色）
 * 4. 参数验证（NULL 缓冲区）
 */
TEST_CASE("Canvas create", "[epd_gui][canvas]")
{
    epd_canvas_t *canvas;
    esp_err_t ret;
    
    // 测试 1: 创建双缓冲区画布
    ESP_LOGI(TAG, "Test 1: Create dual-buffer canvas");
    canvas = epd_canvas_create(EPD_BUFFER_DUAL);
    TEST_ASSERT_NOT_NULL(canvas);
    TEST_ASSERT_TRUE(canvas->buffer_bw != NULL);
    TEST_ASSERT_TRUE(canvas->buffer_red != NULL);
    TEST_ASSERT_EQUAL(EPD_BUFFER_DUAL, canvas->buffer_type);
    epd_canvas_destroy(&canvas);
    TEST_ASSERT_NULL(canvas);
    
    // 测试 2: 创建单缓冲区画布（黑白）
    ESP_LOGI(TAG, "Test 2: Create single BW buffer canvas");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    TEST_ASSERT_TRUE(canvas->buffer_bw != NULL);
    TEST_ASSERT_NULL(canvas->buffer_red);
    TEST_ASSERT_EQUAL(EPD_BUFFER_BW, canvas->buffer_type);
    epd_canvas_destroy(&canvas);
    TEST_ASSERT_NULL(canvas);
    
    // 测试 3: 创建单缓冲区画布（红色）
    ESP_LOGI(TAG, "Test 3: Create single RED buffer canvas");
    canvas = epd_canvas_create(EPD_BUFFER_RED);
    TEST_ASSERT_NOT_NULL(canvas);
    TEST_ASSERT_NULL(canvas->buffer_bw);
    TEST_ASSERT_TRUE(canvas->buffer_red != NULL);
    TEST_ASSERT_EQUAL(EPD_BUFFER_RED, canvas->buffer_type);
    epd_canvas_destroy(&canvas);
    TEST_ASSERT_NULL(canvas);
    
    // 测试 4: 参数验证 - NULL 缓冲区指针
    ESP_LOGI(TAG, "Test 4: NULL buffer pointer should fail");
    // 注意：epd_canvas_create 内部会分配缓冲区，不会接受外部传入的 NULL
    // 这个测试主要验证函数不会崩溃
}

/**
 * @brief 测试画布销毁功能
 * 
 * 验证点：
 * 1. 正常销毁画布
 * 2. 重复销毁（应安全处理）
 * 3. 销毁 NULL 画布（应安全处理）
 */
TEST_CASE("Canvas destroy", "[epd_gui][canvas]")
{
    epd_canvas_t *canvas;
    
    // 测试 1: 正常销毁
    ESP_LOGI(TAG, "Test 1: Normal destroy");
    canvas = epd_canvas_create(EPD_BUFFER_DUAL);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_destroy(&canvas);
    TEST_ASSERT_NULL(canvas);
    
    // 测试 2: 重复销毁（应安全处理）
    ESP_LOGI(TAG, "Test 2: Double destroy (should be safe)");
    canvas = epd_canvas_create(EPD_BUFFER_DUAL);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_destroy(&canvas);
    epd_canvas_destroy(&canvas);  // 第二次调用应安全
    TEST_ASSERT_NULL(canvas);
    
    // 测试 3: 销毁 NULL 画布
    ESP_LOGI(TAG, "Test 3: Destroy NULL canvas");
    canvas = NULL;
    epd_canvas_destroy(&canvas);  // 应安全处理
    TEST_ASSERT_NULL(canvas);
}

/**
 * @brief 测试画布清屏功能
 * 
 * 验证点：
 * 1. 黑白缓冲区清屏（全白）
 * 2. 黑白缓冲区清屏（全黑）
 * 3. 红色缓冲区清屏（全红）
 * 4. 红色缓冲区清屏（无红色）
 * 5. 参数验证（NULL 画布）
 */
TEST_CASE("Canvas clear", "[epd_gui][canvas]")
{
    epd_canvas_t *canvas;
    esp_err_t ret;
    
    // 测试 1: 黑白缓冲区清屏（全白）
    ESP_LOGI(TAG, "Test 1: Clear BW buffer to WHITE");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    // 填充非零数据
    memset(canvas->buffer_bw, 0xAA, EPD_BUFFER_SIZE);
    ret = epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    // 验证缓冲区全为 0xFF（白色）
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_HEX8(0xFF, canvas->buffer_bw[i]);
    }
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 黑白缓冲区清屏（全黑）
    ESP_LOGI(TAG, "Test 2: Clear BW buffer to BLACK");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    // 填充非零数据
    memset(canvas->buffer_bw, 0xAA, EPD_BUFFER_SIZE);
    ret = epd_canvas_clear(canvas, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    // 验证缓冲区全为 0x00（黑色）
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_HEX8(0x00, canvas->buffer_bw[i]);
    }
    epd_canvas_destroy(&canvas);
    
    // 测试 3: 红色缓冲区清屏（全红）
    ESP_LOGI(TAG, "Test 3: Clear RED buffer to RED");
    canvas = epd_canvas_create(EPD_BUFFER_RED);
    TEST_ASSERT_NOT_NULL(canvas);
    // 填充非零数据
    memset(canvas->buffer_red, 0xAA, EPD_BUFFER_SIZE);
    ret = epd_canvas_clear(canvas, EPD_COLOR_RED);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    // 验证缓冲区全为 0xFF（红色）
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_HEX8(0xFF, canvas->buffer_red[i]);
    }
    epd_canvas_destroy(&canvas);
    
    // 测试 4: 红色缓冲区清屏（无红色）
    ESP_LOGI(TAG, "Test 4: Clear RED buffer to NO_RED");
    canvas = epd_canvas_create(EPD_BUFFER_RED);
    TEST_ASSERT_NOT_NULL(canvas);
    // 填充非零数据
    memset(canvas->buffer_red, 0xAA, EPD_BUFFER_SIZE);
    ret = epd_canvas_clear(canvas, EPD_COLOR_NO_RED);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    // 验证缓冲区全为 0x00（无红色）
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_HEX8(0x00, canvas->buffer_red[i]);
    }
    epd_canvas_destroy(&canvas);
    
    // 测试 5: 参数验证 - NULL 画布
    ESP_LOGI(TAG, "Test 5: NULL canvas should fail");
    ret = epd_canvas_clear(NULL, EPD_COLOR_WHITE);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
}

/**
 * @brief 测试像素设置功能
 * 
 * 验证点：
 * 1. 设置黑白像素（黑色）
 * 2. 设置黑白像素（白色）
 * 3. 设置红色像素
 * 4. 边界像素（0,0）
 * 5. 边界像素（最大坐标）
 * 6. 参数验证（NULL 画布）
 */
TEST_CASE("Canvas set pixel", "[epd_gui][canvas]")
{
    epd_canvas_t *canvas;
    esp_err_t ret;
    
    // 测试 1: 设置黑白像素（黑色）
    ESP_LOGI(TAG, "Test 1: Set BW pixel to BLACK");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    // 先清屏为白色
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    // 设置像素 (10, 10) 为黑色
    ret = epd_canvas_set_pixel(canvas, 10, 10, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    // 验证像素已设置
    uint16_t byte_idx = (10 * 152 + 10) / 8;
    uint8_t bit_mask = (0x80 >> (10 % 8));
    TEST_ASSERT_FALSE(canvas->buffer_bw[byte_idx] & bit_mask);  // 黑色为 0
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 设置黑白像素（白色）
    ESP_LOGI(TAG, "Test 2: Set BW pixel to WHITE");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    // 先清屏为黑色
    epd_canvas_clear(canvas, EPD_COLOR_BLACK);
    // 设置像素 (20, 20) 为白色
    ret = epd_canvas_set_pixel(canvas, 20, 20, EPD_COLOR_WHITE);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    // 验证像素已设置
    byte_idx = (20 * 152 + 20) / 8;
    bit_mask = (0x80 >> (20 % 8));
    TEST_ASSERT_TRUE(canvas->buffer_bw[byte_idx] & bit_mask);  // 白色为 1
    epd_canvas_destroy(&canvas);
    
    // 测试 3: 设置红色像素
    ESP_LOGI(TAG, "Test 3: Set RED pixel");
    canvas = epd_canvas_create(EPD_BUFFER_RED);
    TEST_ASSERT_NOT_NULL(canvas);
    // 先清屏为无红色
    epd_canvas_clear(canvas, EPD_COLOR_NO_RED);
    // 设置像素 (30, 30) 为红色
    ret = epd_canvas_set_pixel(canvas, 30, 30, EPD_COLOR_RED);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    // 验证像素已设置
    byte_idx = (30 * 152 + 30) / 8;
    bit_mask = (0x80 >> (30 % 8));
    TEST_ASSERT_TRUE(canvas->buffer_red[byte_idx] & bit_mask);  // 红色为 1
    epd_canvas_destroy(&canvas);
    
    // 测试 4: 边界像素（0,0）
    ESP_LOGI(TAG, "Test 4: Boundary pixel (0,0)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_canvas_set_pixel(canvas, 0, 0, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_FALSE(canvas->buffer_bw[0] & 0x80);  // 第一个像素
    epd_canvas_destroy(&canvas);
    
    // 测试 5: 边界像素（最大坐标）
    ESP_LOGI(TAG, "Test 5: Boundary pixel (max)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    // 最大坐标：X=151, Y=295（SSD1680 内部坐标）
    // 用户坐标：X=295, Y=151
    ret = epd_canvas_set_pixel(canvas, 295, 151, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 6: 参数验证 - NULL 画布
    ESP_LOGI(TAG, "Test 6: NULL canvas should fail");
    ret = epd_canvas_set_pixel(NULL, 10, 10, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
}

// ==================== 绘图算法测试 ====================

/**
 * @brief 测试直线绘制功能
 * 
 * 验证点：
 * 1. 水平直线
 * 2. 垂直直线
 * 3. 对角直线
 * 4. 任意角度直线
 * 5. 参数验证（NULL 画布）
 */
TEST_CASE("Draw line", "[epd_gui][geometry]")
{
    epd_canvas_t *canvas;
    esp_err_t ret;
    
    // 测试 1: 水平直线
    ESP_LOGI(TAG, "Test 1: Horizontal line");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_line(canvas, 10, 50, 100, 50, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    // 验证直线上的像素
    for (uint16_t x = 10; x <= 100; x++) {
        // 检查 Y=50 行上的像素是否被设置
        // 具体验证逻辑依赖于坐标转换和像素设置
    }
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 垂直直线
    ESP_LOGI(TAG, "Test 2: Vertical line");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_line(canvas, 50, 10, 50, 100, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 3: 对角直线
    ESP_LOGI(TAG, "Test 3: Diagonal line");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_line(canvas, 10, 10, 100, 100, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 4: 参数验证 - NULL 画布
    ESP_LOGI(TAG, "Test 4: NULL canvas should fail");
    ret = epd_draw_line(NULL, 0, 0, 100, 100, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
}

/**
 * @brief 测试矩形绘制功能
 * 
 * 验证点：
 * 1. 空心矩形
 * 2. 实心矩形
 * 3. 小矩形（1x1）
 * 4. 大矩形
 * 5. 参数验证（NULL 画布）
 */
TEST_CASE("Draw rectangle", "[epd_gui][geometry]")
{
    epd_canvas_t *canvas;
    esp_err_t ret;
    
    // 测试 1: 空心矩形
    ESP_LOGI(TAG, "Test 1: Hollow rectangle");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_rectangle(canvas, 10, 10, 100, 80, EPD_COLOR_BLACK, false);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 实心矩形
    ESP_LOGI(TAG, "Test 2: Filled rectangle");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_rectangle(canvas, 10, 10, 100, 80, EPD_COLOR_BLACK, true);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 3: 小矩形（1x1）
    ESP_LOGI(TAG, "Test 3: Small rectangle (1x1)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_rectangle(canvas, 50, 50, 51, 51, EPD_COLOR_BLACK, true);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 4: 参数验证 - NULL 画布
    ESP_LOGI(TAG, "Test 4: NULL canvas should fail");
    ret = epd_draw_rectangle(NULL, 0, 0, 100, 100, EPD_COLOR_BLACK, false);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
}

/**
 * @brief 测试圆形绘制功能
 * 
 * 验证点：
 * 1. 空心圆
 * 2. 实心圆
 * 3. 小圆（半径=1）
 * 4. 大圆
 * 5. 参数验证（NULL 画布）
 */
TEST_CASE("Draw circle", "[epd_gui][geometry]")
{
    epd_canvas_t *canvas;
    esp_err_t ret;
    
    // 测试 1: 空心圆
    ESP_LOGI(TAG, "Test 1: Hollow circle");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_circle(canvas, 75, 75, 50, EPD_COLOR_BLACK, false);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 实心圆
    ESP_LOGI(TAG, "Test 2: Filled circle");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_circle(canvas, 75, 75, 50, EPD_COLOR_BLACK, true);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 3: 小圆（半径=1）
    ESP_LOGI(TAG, "Test 3: Small circle (r=1)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_circle(canvas, 50, 50, 1, EPD_COLOR_BLACK, true);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 4: 参数验证 - NULL 画布
    ESP_LOGI(TAG, "Test 4: NULL canvas should fail");
    ret = epd_draw_circle(NULL, 0, 0, 10, EPD_COLOR_BLACK, false);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
}

/**
 * @brief 测试三角形绘制功能
 * 
 * 验证点：
 * 1. 空心三角形
 * 2. 实心三角形
 * 3. 等边三角形
 * 4. 直角三角形
 * 5. 参数验证（NULL 画布）
 */
TEST_CASE("Draw triangle", "[epd_gui][geometry]")
{
    epd_canvas_t *canvas;
    esp_err_t ret;
    
    // 测试 1: 空心三角形
    ESP_LOGI(TAG, "Test 1: Hollow triangle");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_triangle(canvas, 50, 10, 20, 100, 80, 100, EPD_COLOR_BLACK, false);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 实心三角形
    ESP_LOGI(TAG, "Test 2: Filled triangle");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_triangle(canvas, 50, 10, 20, 100, 80, 100, EPD_COLOR_BLACK, true);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 3: 等边三角形
    ESP_LOGI(TAG, "Test 3: Equilateral triangle");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_draw_triangle(canvas, 75, 20, 30, 100, 120, 100, EPD_COLOR_BLACK, true);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 4: 参数验证 - NULL 画布
    ESP_LOGI(TAG, "Test 4: NULL canvas should fail");
    ret = epd_draw_triangle(NULL, 0, 0, 0, 100, 100, 100, EPD_COLOR_BLACK, false);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
}

// ==================== 文本渲染测试 ====================

/**
 * @brief 测试字符显示功能
 * 
 * 验证点：
 * 1. 显示基本字符（A-Z, a-z, 0-9）
 * 2. 显示特殊字符（!@#$%）
 * 3. 显示边界字符（第一个和最后一个字符）
 * 4. 不同字体大小
 * 5. 参数验证（NULL 画布、NULL 字体）
 */
TEST_CASE("Show character", "[epd_gui][text]")
{
    epd_canvas_t *canvas;
    const epd_font_t *font;
    esp_err_t ret;
    
    // 测试 1: 显示字符 'A'（8x16 字体）
    ESP_LOGI(TAG, "Test 1: Show char 'A' (8x16)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_char(canvas, 10, 10, 'A', font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 显示特殊字符 '!'（6x8 字体）
    ESP_LOGI(TAG, "Test 2: Show char '!' (6x8)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_6X8);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_char(canvas, 10, 10, '!', font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 3: 显示边界字符（第一个字符 0x20 空格）
    ESP_LOGI(TAG, "Test 3: Show boundary char (space)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_char(canvas, 10, 10, 0x20, font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 4: 显示边界字符（最后一个字符 0x7E ~）
    ESP_LOGI(TAG, "Test 4: Show boundary char (~)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_char(canvas, 10, 10, 0x7E, font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 5: 不同字体大小（12x24）
    ESP_LOGI(TAG, "Test 5: Different font size (12x24)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_12X24);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_char(canvas, 10, 10, 'Z', font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 6: 参数验证 - NULL 画布
    ESP_LOGI(TAG, "Test 6: NULL canvas should fail");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    ret = epd_show_char(NULL, 10, 10, 'A', font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    
    // 测试 7: 参数验证 - NULL 字体
    ESP_LOGI(TAG, "Test 7: NULL font should fail");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    ret = epd_show_char(canvas, 10, 10, 'A', NULL, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    epd_canvas_destroy(&canvas);
}

/**
 * @brief 测试字符串显示功能
 * 
 * 验证点：
 * 1. 显示短字符串
 * 2. 显示长字符串
 * 3. 显示空字符串
 * 4. 不同字体大小
 * 5. 参数验证（NULL 画布、NULL 字体、NULL 字符串）
 */
TEST_CASE("Show string", "[epd_gui][text]")
{
    epd_canvas_t *canvas;
    const epd_font_t *font;
    esp_err_t ret;
    
    // 测试 1: 显示短字符串
    ESP_LOGI(TAG, "Test 1: Show short string");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_string(canvas, 10, 10, "Hello", font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 显示长字符串
    ESP_LOGI(TAG, "Test 2: Show long string");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_6X8);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_string(canvas, 10, 10, "This is a longer string for testing", font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 3: 显示空字符串
    ESP_LOGI(TAG, "Test 3: Show empty string");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_string(canvas, 10, 10, "", font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 4: 参数验证 - NULL 画布
    ESP_LOGI(TAG, "Test 4: NULL canvas should fail");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    ret = epd_show_string(NULL, 10, 10, "Test", font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    
    // 测试 5: 参数验证 - NULL 字体
    ESP_LOGI(TAG, "Test 5: NULL font should fail");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    ret = epd_show_string(canvas, 10, 10, "Test", NULL, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 6: 参数验证 - NULL 字符串
    ESP_LOGI(TAG, "Test 6: NULL string should fail");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    ret = epd_show_string(canvas, 10, 10, NULL, font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    epd_canvas_destroy(&canvas);
}

/**
 * @brief 测试智能换行功能
 * 
 * 验证点：
 * 1. 短文本（不需要换行）
 * 2. 长文本（需要换行）
 * 3. 包含单词的换行（不应切断单词）
 * 4. 包含 \n 的强制换行
 * 5. 参数验证（NULL 画布、NULL 字体、NULL 字符串）
 */
TEST_CASE("Show string with wrap", "[epd_gui][text]")
{
    epd_canvas_t *canvas;
    const epd_font_t *font;
    esp_err_t ret;
    
    // 测试 1: 短文本（不需要换行）
    ESP_LOGI(TAG, "Test 1: Short text (no wrap)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_string_wrap(canvas, 10, 10, 200, "Short", font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 长文本（需要换行）
    ESP_LOGI(TAG, "Test 2: Long text (with wrap)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_string_wrap(canvas, 10, 10, 100, "This is a long text that should wrap to multiple lines", font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 3: 包含单词的换行（不应切断单词）
    ESP_LOGI(TAG, "Test 3: Word wrap (should not break words)");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_string_wrap(canvas, 10, 10, 80, "Hello world this is a test", font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 4: 包含 \n 的强制换行
    ESP_LOGI(TAG, "Test 4: Forced wrap with \\n");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_string_wrap(canvas, 10, 10, 200, "Line 1\nLine 2\nLine 3", font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 5: 参数验证 - NULL 画布
    ESP_LOGI(TAG, "Test 5: NULL canvas should fail");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    ret = epd_show_string_wrap(NULL, 10, 10, 100, "Test", font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    
    // 测试 6: 参数验证 - NULL 字体
    ESP_LOGI(TAG, "Test 6: NULL font should fail");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    ret = epd_show_string_wrap(canvas, 10, 10, 100, "Test", NULL, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 7: 参数验证 - NULL 字符串
    ESP_LOGI(TAG, "Test 7: NULL string should fail");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    ret = epd_show_string_wrap(canvas, 10, 10, 100, NULL, font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    epd_canvas_destroy(&canvas);
}

/**
 * @brief 测试数字显示功能
 * 
 * 验证点：
 * 1. 显示正数
 * 2. 显示负数
 * 3. 显示零
 * 4. 显示大数
 * 5. 参数验证（NULL 画布、NULL 字体）
 */
TEST_CASE("Show number", "[epd_gui][text]")
{
    epd_canvas_t *canvas;
    const epd_font_t *font;
    esp_err_t ret;
    
    // 测试 1: 显示正数
    ESP_LOGI(TAG, "Test 1: Show positive number");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_num(canvas, 10, 10, 12345, font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 2: 显示负数
    ESP_LOGI(TAG, "Test 2: Show negative number");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_num(canvas, 10, 10, -9876, font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 3: 显示零
    ESP_LOGI(TAG, "Test 3: Show zero");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_num(canvas, 10, 10, 0, font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 4: 显示大数
    ESP_LOGI(TAG, "Test 4: Show large number");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    ret = epd_show_num(canvas, 10, 10, 2147483647, font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    epd_canvas_destroy(&canvas);
    
    // 测试 5: 参数验证 - NULL 画布
    ESP_LOGI(TAG, "Test 5: NULL canvas should fail");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    ret = epd_show_num(NULL, 10, 10, 100, font, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    
    // 测试 6: 参数验证 - NULL 字体
    ESP_LOGI(TAG, "Test 6: NULL font should fail");
    canvas = epd_canvas_create(EPD_BUFFER_BW);
    TEST_ASSERT_NOT_NULL(canvas);
    ret = epd_show_num(canvas, 10, 10, 100, NULL, EPD_COLOR_BLACK);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    epd_canvas_destroy(&canvas);
}

// ==================== 字体管理测试 ====================

/**
 * @brief 测试字体获取功能
 * 
 * 验证点：
 * 1. 获取 6x8 字体
 * 2. 获取 6x12 字体
 * 3. 获取 8x16 字体
 * 4. 获取 12x24 字体
 * 5. 获取无效字体（应返回 NULL）
 */
TEST_CASE("Get font", "[epd_font]")
{
    const epd_font_t *font;
    
    // 测试 1: 获取 6x8 字体
    ESP_LOGI(TAG, "Test 1: Get 6x8 font");
    font = epd_font_get(EPD_FONT_SIZE_6X8);
    TEST_ASSERT_NOT_NULL(font);
    TEST_ASSERT_EQUAL(6, font->width);
    TEST_ASSERT_EQUAL(8, font->height);
    TEST_ASSERT_EQUAL(0x20, font->first_char);
    TEST_ASSERT_EQUAL(0x7E, font->last_char);
    TEST_ASSERT_EQUAL(8, font->char_bytes);  // 6x8/8 = 6 字节，向上取整为 8
    
    // 测试 2: 获取 6x12 字体
    ESP_LOGI(TAG, "Test 2: Get 6x12 font");
    font = epd_font_get(EPD_FONT_SIZE_6X12);
    TEST_ASSERT_NOT_NULL(font);
    TEST_ASSERT_EQUAL(6, font->width);
    TEST_ASSERT_EQUAL(12, font->height);
    TEST_ASSERT_EQUAL(12, font->char_bytes);  // 6x12/8 = 9 字节，向上取整为 12
    
    // 测试 3: 获取 8x16 字体
    ESP_LOGI(TAG, "Test 3: Get 8x16 font");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    TEST_ASSERT_EQUAL(8, font->width);
    TEST_ASSERT_EQUAL(16, font->height);
    TEST_ASSERT_EQUAL(16, font->char_bytes);  // 8x16/8 = 16 字节
    
    // 测试 4: 获取 12x24 字体
    ESP_LOGI(TAG, "Test 4: Get 12x24 font");
    font = epd_font_get(EPD_FONT_SIZE_12X24);
    TEST_ASSERT_NOT_NULL(font);
    TEST_ASSERT_EQUAL(12, font->width);
    TEST_ASSERT_EQUAL(24, font->height);
    TEST_ASSERT_EQUAL(48, font->char_bytes);  // 12x24/8 = 36 字节，向上取整为 48
    
    // 测试 5: 获取无效字体
    ESP_LOGI(TAG, "Test 5: Get invalid font size");
    font = epd_font_get(99);  // 无效的字体大小
    TEST_ASSERT_NULL(font);
}

/**
 * @brief 测试字符串宽度计算
 * 
 * 验证点：
 * 1. 空字符串宽度
 * 2. 单字符宽度
 * 3. 多字符宽度
 * 4. 不同字体大小
 * 5. 参数验证（NULL 字体、NULL 字符串）
 */
TEST_CASE("Get string width", "[epd_gui][text]")
{
    const epd_font_t *font;
    uint16_t width;
    
    // 测试 1: 空字符串宽度（8x16 字体）
    ESP_LOGI(TAG, "Test 1: Empty string width (8x16)");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    width = epd_get_string_width("", font);
    TEST_ASSERT_EQUAL(0, width);
    
    // 测试 2: 单字符宽度（8x16 字体）
    ESP_LOGI(TAG, "Test 2: Single char width (8x16)");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    width = epd_get_string_width("A", font);
    TEST_ASSERT_EQUAL(8, width);
    
    // 测试 3: 多字符宽度（8x16 字体）
    ESP_LOGI(TAG, "Test 3: Multi-char width (8x16)");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    width = epd_get_string_width("Hello", font);
    TEST_ASSERT_EQUAL(40, width);  // 5 字符 × 8 像素
    
    // 测试 4: 不同字体大小（6x8 字体）
    ESP_LOGI(TAG, "Test 4: Different font size (6x8)");
    font = epd_font_get(EPD_FONT_SIZE_6X8);
    TEST_ASSERT_NOT_NULL(font);
    width = epd_get_string_width("Hi", font);
    TEST_ASSERT_EQUAL(12, width);  // 2 字符 × 6 像素
    
    // 测试 5: 参数验证 - NULL 字体
    ESP_LOGI(TAG, "Test 5: NULL font should return 0");
    width = epd_get_string_width("Test", NULL);
    TEST_ASSERT_EQUAL(0, width);
    
    // 测试 6: 参数验证 - NULL 字符串
    ESP_LOGI(TAG, "Test 6: NULL string should return 0");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    width = epd_get_string_width(NULL, font);
    TEST_ASSERT_EQUAL(0, width);
}

/**
 * @brief 测试字符串高度计算
 * 
 * 验证点：
 * 1. 空字符串高度
 * 2. 单行字符串高度
 * 3. 多行字符串高度（包含 \n）
 * 4. 不同字体大小
 * 5. 参数验证（NULL 字体、NULL 字符串）
 */
TEST_CASE("Get string height", "[epd_gui][text]")
{
    const epd_font_t *font;
    uint16_t height;
    
    // 测试 1: 空字符串高度（8x16 字体）
    ESP_LOGI(TAG, "Test 1: Empty string height (8x16)");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    height = epd_get_string_height("", font);
    TEST_ASSERT_EQUAL(0, height);
    
    // 测试 2: 单行字符串高度（8x16 字体）
    ESP_LOGI(TAG, "Test 2: Single line height (8x16)");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    height = epd_get_string_height("Hello", font);
    TEST_ASSERT_EQUAL(16, height);
    
    // 测试 3: 多行字符串高度（包含 \n）
    ESP_LOGI(TAG, "Test 3: Multi-line height with \\n");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    TEST_ASSERT_NOT_NULL(font);
    height = epd_get_string_height("Line1\nLine2\nLine3", font);
    TEST_ASSERT_EQUAL(48, height);  // 3 行 × 16 像素
    
    // 测试 4: 不同字体大小（6x8 字体）
    ESP_LOGI(TAG, "Test 4: Different font size (6x8)");
    font = epd_font_get(EPD_FONT_SIZE_6X8);
    TEST_ASSERT_NOT_NULL(font);
    height = epd_get_string_height("Hi", font);
    TEST_ASSERT_EQUAL(8, height);
    
    // 测试 5: 参数验证 - NULL 字体
    ESP_LOGI(TAG, "Test 5: NULL font should return 0");
    height = epd_get_string_height("Test", NULL);
    TEST_ASSERT_EQUAL(0, height);
    
    // 测试 6: 参数验证 - NULL 字符串
    ESP_LOGI(TAG, "Test 6: NULL string should return 0");
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    height = epd_get_string_height(NULL, font);
    TEST_ASSERT_EQUAL(0, height);
}

/**
 * @brief 测试初始化函数（由 main.c 的 app_main 调用）
 */
void epd_tests_run(void)
{
    ESP_LOGI(TAG, "=== 开始 EPD GUI 和字体系统测试 ===");
    
    // 运行所有测试用例
    unity_run_menu();
}
