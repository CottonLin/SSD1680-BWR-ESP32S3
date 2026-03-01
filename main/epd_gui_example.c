/**
 * @file epd_gui_example.c
 * @brief GUI 使用示例代码
 * 
 * 此文件展示如何使用 GUI 模块进行几何图形绘制和字符显示
 * 仅供参考，不编译到最终固件中
 */

#include "epd_gui.h"
#include "ssd1680.h"
#include "esp_log.h"

static const char *TAG = "GUI_EXAMPLE";

/**
 * @brief GUI 功能测试示例
 */
void gui_example_usage(void)
{
    epd_canvas_t *canvas;
    const epd_font_t *font;
    
    ESP_LOGI(TAG, "=== GUI 功能测试开始 ===");
    
    // 1. 创建画布（使用底层驱动的显存）
    extern uint8_t *g_epd_buffer_bw;  // 假设已在 main.c 中定义
    canvas = epd_canvas_create(g_epd_buffer_bw, 296, 152);
    if (canvas == NULL) {
        ESP_LOGE(TAG, "画布创建失败");
        return;
    }
    
    // 2. 清屏（白色背景）
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    
    // 3. 绘制边框矩形
    epd_draw_rectangle(canvas, 0, 0, 295, 151, EPD_COLOR_BLACK, 0);
    
    // 4. 绘制对角线
    epd_draw_line(canvas, 0, 0, 295, 151, EPD_COLOR_BLACK);
    epd_draw_line(canvas, 295, 0, 0, 151, EPD_COLOR_BLACK);
    
    // 5. 绘制实心矩形
    epd_draw_rectangle(canvas, 50, 30, 100, 60, EPD_COLOR_BLACK, 1);
    
    // 6. 绘制空心圆
    epd_draw_circle(canvas, 200, 50, 30, EPD_COLOR_BLACK, 0);
    
    // 7. 绘制实心圆
    epd_draw_circle(canvas, 250, 50, 20, EPD_COLOR_BLACK, 1);
    
    // 8. 显示字符串（8x8 字体）
    font = epd_font_get(EPD_FONT_SIZE_8X8);
    if (font != NULL) {
        epd_show_string(canvas, 10, 10, "Hello SSD1680!", font, EPD_COLOR_BLACK);
    }
    
    // 9. 显示字符串（16x16 字体）
    font = epd_font_get(EPD_FONT_SIZE_16X16);
    if (font != NULL) {
        epd_show_string(canvas, 10, 80, "ESP32-S3", font, EPD_COLOR_BLACK);
    }
    
    // 10. 显示数字
    font = epd_font_get(EPD_FONT_SIZE_16X16);
    epd_show_num(canvas, 150, 80, 2024, 4, font, EPD_COLOR_BLACK);
    
    // 11. 刷新显示
    extern void epd_display(const uint8_t *buffer_bw, const uint8_t *buffer_red);
    extern void epd_update(void);
    epd_display(g_epd_buffer_bw, g_epd_buffer_bw);
    
    // 12. 销毁画布
    epd_canvas_destroy(canvas);
    
    ESP_LOGI(TAG, "=== GUI 功能测试完成 ===");
}

/**
 * @brief 绘制简单图表表示例
 */
void gui_draw_chart_example(void)
{
    epd_canvas_t *canvas;
    const epd_font_t *font;
    int i;
    int16_t data[20];
    int max_value = 100;
    int chart_height = 80;
    int chart_width = 280;
    int chart_x = 10;
    int chart_y = 40;
    
    canvas = epd_canvas_create(NULL, 296, 152);
    if (canvas == NULL) {
        return;
    }
    
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    
    // 生成模拟数据（正弦波）
    for (i = 0; i < 20; i++) {
        data[i] = (int16_t)(50 + 40 * sinf(i * 0.3f));
    }
    
    // 绘制坐标轴
    epd_draw_line(canvas, chart_x, chart_y, 
                 chart_x, chart_y + chart_height, EPD_COLOR_BLACK);
    epd_draw_line(canvas, chart_x, chart_y + chart_height, 
                 chart_x + chart_width, chart_y + chart_height, EPD_COLOR_BLACK);
    
    // 绘制折线图
    int step_x = chart_width / 19;
    for (i = 0; i < 19; i++) {
        int y1 = chart_y + chart_height - (data[i] * chart_height / max_value);
        int y2 = chart_y + chart_height - (data[i + 1] * chart_height / max_value);
        epd_draw_line(canvas, chart_x + i * step_x, y1, 
                     chart_x + (i + 1) * step_x, y2, EPD_COLOR_BLACK);
    }
    
    // 显示标题
    font = epd_font_get(EPD_FONT_SIZE_8X8);
    epd_show_string(canvas, chart_x, chart_y - 12, "Data Chart", font, EPD_COLOR_BLACK);
    
    epd_display(canvas->buffer, canvas->buffer);
    epd_canvas_destroy(canvas);
}

/**
 * @brief 绘制仪表盘示例
 */
void gui_draw_gauge_example(void)
{
    epd_canvas_t *canvas;
    const epd_font_t *font;
    char buffer[32];
    
    canvas = epd_canvas_create(NULL, 296, 152);
    if (canvas == NULL) {
        return;
    }
    
    epd_canvas_clear(canvas, EPD_COLOR_WHITE);
    
    // 绘制外框圆
    epd_draw_circle(canvas, 148, 76, 60, EPD_COLOR_BLACK, 0);
    
    // 绘制刻度线（简化示例）
    for (int angle = 0; angle < 360; angle += 30) {
        float rad = angle * 3.14159f / 180.0f;
        int x1 = 148 + (int)(55 * cosf(rad));
        int y1 = 76 + (int)(55 * sinf(rad));
        int x2 = 148 + (int)(60 * cosf(rad));
        int y2 = 76 + (int)(60 * sinf(rad));
        epd_draw_line(canvas, x1, y1, x2, y2, EPD_COLOR_BLACK);
    }
    
    // 显示数值
    font = epd_font_get(EPD_FONT_SIZE_16X16);
    snprintf(buffer, sizeof(buffer), "75");
    epd_show_string(canvas, 130, 70, buffer, font, EPD_COLOR_BLACK);
    
    epd_display(canvas->buffer, canvas->buffer);
    epd_canvas_destroy(canvas);
}
