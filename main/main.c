#include <stdio.h>
#include "ssd1680.h"
#include "epd_gui.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN";

/**
 * @brief 编译配置宏
 * 
 * 设置为 1: 启用对应测试
 * 设置为 0: 禁用对应测试
 */
#define ENABLE_BASIC_TEST       0   // 1=启用基础测试，0=禁用
#define ENABLE_GEOMETRY_TEST    0   // 1=启用几何图形测试，0=禁用
#define ENABLE_FONT_TEST        1   // 1=启用字体测试，0=禁用

/**
 * @brief 基础显示测试（清屏、图案显示、睡眠）
 */
#if ENABLE_BASIC_TEST
static void basic_test(void)
{
    uint8_t *test_buffer_bw;
    uint8_t *test_buffer_red;
    
    ESP_LOGI(TAG, "=== 开始基础显示测试 ===");
    
    // 1. 清屏测试
    ESP_LOGI(TAG, "清屏测试...");
    epd_clear();
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 2. 创建测试图案
    test_buffer_bw = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    test_buffer_red = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    
    if (test_buffer_bw == NULL || test_buffer_red == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return;
    }
    
    // 3. 填充测试图案：黑白条纹
    for (int i = 0; i < EPD_BUFFER_SIZE; i++) {
        test_buffer_bw[i] = (i % 2 == 0) ? 0xAA : 0x55;
        test_buffer_red[i] = 0xFF;
    }
    
    // 4. 显示测试图案
    ESP_LOGI(TAG, "显示测试图案...");
    epd_display(test_buffer_bw, test_buffer_red);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // 5. 再次清屏
    ESP_LOGI(TAG, "再次清屏...");
    epd_clear();
    
    // 6. 进入深度睡眠
    ESP_LOGI(TAG, "进入深度睡眠模式");
    epd_deep_sleep();
    
    // 释放内存
    free(test_buffer_bw);
    free(test_buffer_red);
    
    ESP_LOGI(TAG, "=== 基础显示测试完成 ===");
}
#endif

/**
 * @brief 几何图形绘制测试（矩形、线、圆）
 */
#if ENABLE_GEOMETRY_TEST
static void geometry_test(void)
{
    epd_canvas_t *canvas;
    uint8_t *test_buffer_bw;
    uint8_t *test_buffer_red;
    
    ESP_LOGI(TAG, "=== 开始几何图形绘制测试 ===");
    
    // 创建两个显存缓冲区
    test_buffer_bw = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    test_buffer_red = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    if (test_buffer_bw == NULL || test_buffer_red == NULL) {
        ESP_LOGE(TAG, "显存分配失败");
        return;
    }
    
    // 创建画布（传入两个缓冲区）
    canvas = epd_canvas_create(test_buffer_bw, test_buffer_red, 296, 152);
    if (canvas == NULL) {
        ESP_LOGE(TAG, "画布创建失败");
        free(test_buffer_bw);
        free(test_buffer_red);
        return;
    }
    
    // 1. 清屏
    ESP_LOGI(TAG, "清屏...");
    memset(test_buffer_bw, 0xFF, EPD_BUFFER_SIZE);
    memset(test_buffer_red, 0xFF, EPD_BUFFER_SIZE);
    epd_display(test_buffer_bw, test_buffer_red);
    
    // 2. 绘制边框矩形（黑色）
    ESP_LOGI(TAG, "绘制边框矩形（黑色）...");
    epd_draw_rectangle(canvas, 0, 0, 295, 151, EPD_COLOR_BLACK, 0);
    
    // 3. 绘制对角线（黑色）
    ESP_LOGI(TAG, "绘制对角线（黑色）...");
    epd_draw_line(canvas, 0, 0, 295, 151, EPD_COLOR_BLACK);
    epd_draw_line(canvas, 295, 0, 0, 151, EPD_COLOR_BLACK);
    
    // 4. 绘制实心矩形（红色）
    ESP_LOGI(TAG, "绘制实心矩形（红色）...");
    epd_draw_rectangle(canvas, 50, 30, 100, 60, EPD_COLOR_RED, 1);
    
    // 5. 绘制空心圆（黑色）
    ESP_LOGI(TAG, "绘制空心圆（黑色）...");
    epd_draw_circle(canvas, 200, 50, 30, EPD_COLOR_BLACK, 0);
    
    // 6. 绘制实心圆（红色）
    ESP_LOGI(TAG, "绘制实心圆（红色）...");
    epd_draw_circle(canvas, 250, 50, 20, EPD_COLOR_RED, 1);
    
    // 刷新显示
    epd_display(test_buffer_bw, test_buffer_red);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // 清理
    epd_canvas_destroy(canvas);
    free(test_buffer_bw);
    free(test_buffer_red);
    
    ESP_LOGI(TAG, "=== 几何图形绘制测试完成 ===");
}
#endif

/**
 * @brief 字体显示测试（多种字号）
 */
#if ENABLE_FONT_TEST
static void font_test(void)
{
    epd_canvas_t *canvas;
    const epd_font_t *font;
    uint8_t *test_buffer_bw;
    uint8_t *test_buffer_red;
    
    ESP_LOGI(TAG, "=== 开始字体显示测试 ===");
    
    // 创建两个显存缓冲区
    test_buffer_bw = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    test_buffer_red = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    if (test_buffer_bw == NULL || test_buffer_red == NULL) {
        ESP_LOGE(TAG, "显存分配失败");
        return;
    }
    
    // 创建画布（传入两个缓冲区）
    canvas = epd_canvas_create(test_buffer_bw, test_buffer_red, 296, 152);
    if (canvas == NULL) {
        ESP_LOGE(TAG, "画布创建失败");
        free(test_buffer_bw);
        free(test_buffer_red);
        return;
    }
    
    // 1. 清屏
    ESP_LOGI(TAG, "清屏...");
    memset(test_buffer_bw, 0xFF, EPD_BUFFER_SIZE);
    memset(test_buffer_red, 0xFF, EPD_BUFFER_SIZE);
    epd_display(test_buffer_bw, test_buffer_red);
    
    // 2. 显示不同字号的字体
    ESP_LOGI(TAG, "显示不同字号字体...");
    
    // 坐标规划（避免显存重叠）：
    // 6x8:   Y=10  → 内部 X=10,  占用 X[10-17]
    // 6x12:  Y=30  → 内部 X=30,  占用 X[30-41]
    // 8x16:  Y=55  → 内部 X=55,  占用 X[55-70]
    // 12x24: Y=85  → 内部 X=85,  占用 X[85-108]
    
    font = epd_font_get(EPD_FONT_SIZE_6X8);
    if (font != NULL) {
        ESP_LOGI(TAG, "绘制 6x8 字体在 (10,10): 6x8 Font (黑色)");
        epd_show_string(canvas, 10, 10, "6x8 Font", font, EPD_COLOR_BLACK);
    }
    
    font = epd_font_get(EPD_FONT_SIZE_6X12);
    if (font != NULL) {
        ESP_LOGI(TAG, "绘制 6x12 字体在 (10,30): 6x12 Font (红色)");
        epd_show_string(canvas, 10, 30, "6x12 Font", font, EPD_COLOR_RED);
    }

    font = epd_font_get(EPD_FONT_SIZE_8X16);
    if (font != NULL) {
        ESP_LOGI(TAG, "绘制 8x16 字体在 (10,55): 8x16 Font (黑色)");
        epd_show_string(canvas, 10, 55, "8x16 Font", font, EPD_COLOR_BLACK);
    }
    
    font = epd_font_get(EPD_FONT_SIZE_12X24);
    if (font != NULL) {
        ESP_LOGI(TAG, "绘制 12x24 字体在 (10,85): 12x24 Font (红色)");
        epd_show_string(canvas, 10, 85, "12x24 Font", font, EPD_COLOR_RED);
    }
    
    // 刷新显示
    epd_display(test_buffer_bw, test_buffer_red);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // 测试自动换行功能
    ESP_LOGI(TAG, "=== 测试智能换行功能（按单词换行）===");
    memset(test_buffer_bw, 0xFF, EPD_BUFFER_SIZE);
    memset(test_buffer_red, 0xFF, EPD_BUFFER_SIZE);
    
    // 绘制边框（黑色）
    epd_draw_rectangle(canvas, 5, 5, 290, 146, EPD_COLOR_BLACK, 0);
    
    // 测试 1: 使用 8x16 字体显示长文本（智能换行，不会切断单词）
    font = epd_font_get(EPD_FONT_SIZE_8X16);
    if (font != NULL) {
        const char *long_text = "Smart Wrap: This intelligent word wrapper will never break a word in the middle. It always keeps complete words together on each line.";
        ESP_LOGI(TAG, "测试智能换行：8x16 字体 (红色)");
        epd_show_string_wrap(canvas, 10, 10, long_text, font, EPD_COLOR_RED, 280, 140);
    }
    
    // 刷新显示
    epd_display(test_buffer_bw, test_buffer_red);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // 测试 2: 使用 6x8 字体显示多段落文本
    ESP_LOGI(TAG, "=== 测试多段落文本 ===");
    memset(test_buffer_bw, 0xFF, EPD_BUFFER_SIZE);
    memset(test_buffer_red, 0xFF, EPD_BUFFER_SIZE);
    
    font = epd_font_get(EPD_FONT_SIZE_6X8);
    if (font != NULL) {
        const char *multi_para_text = "Para1:\nThis first paragraph shows smart word wrapping.\n\nPara2:\nVeryLongWordThatShouldNotBreak will stay together.";
        ESP_LOGI(TAG, "测试多段落文本：6x8 字体 (黑色)");
        epd_show_string_wrap(canvas, 10, 10, multi_para_text, font, EPD_COLOR_BLACK, 276, 132);
    }
    
    // 刷新显示
    epd_display(test_buffer_bw, test_buffer_red);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // 清理
    epd_canvas_destroy(canvas);
    free(test_buffer_bw);
    free(test_buffer_red);
    
    ESP_LOGI(TAG, "=== 字体显示测试完成 ===");
}
#endif

void app_main(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "SSD1680 测试程序启动");
    ESP_LOGI(TAG, "配置：BASIC=%d, GEOMETRY=%d, FONT=%d", 
             ENABLE_BASIC_TEST, ENABLE_GEOMETRY_TEST, ENABLE_FONT_TEST);
    
    // 1. 初始化显示屏
    ret = epd_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "显示屏初始化失败");
        return;
    }
    
    ESP_LOGI(TAG, "显示屏初始化成功");
    
    // 2. 根据配置运行对应测试
#if ENABLE_BASIC_TEST
    basic_test();
#elif ENABLE_GEOMETRY_TEST
    geometry_test();
#elif ENABLE_FONT_TEST
    font_test();
#else
    ESP_LOGW(TAG, "警告：所有测试都已禁用");
#endif
    
    ESP_LOGI(TAG, "程序结束");
}
