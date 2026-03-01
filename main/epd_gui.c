#include "epd_gui.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const char *TAG = "EPD_GUI";

/**
 * @brief 坐标转换：用户坐标 → SSD1680 内部坐标
 * 
 * 坐标映射关系：
 * 用户坐标 (x, y): x[0-295], y[0-151]  // 物理屏幕
 * 内部坐标 (X, Y): X[0-151], Y[0-295]  // SSD1680 RAM
 * 
 * 转换公式：
 *   内部 X = 用户 Y
 *   内部 Y = 用户 X
 * 
 * @param canvas 画布对象
 * @param user_x 用户 X 坐标
 * @param user_y 用户 Y 坐标
 * @param internal_x 输出：内部 X 坐标
 * @param internal_y 输出：内部 Y 坐标
 */
static inline void coordinate_transform(epd_canvas_t *canvas, 
                                        uint16_t user_x, uint16_t user_y,
                                        uint16_t *internal_x, uint16_t *internal_y)
{
    if (internal_x == NULL || internal_y == NULL) {
        return;
    }
    
    if (user_x >= canvas->width || user_y >= canvas->height) {
        ESP_LOGW(TAG, "坐标越界：(%d, %d)", user_x, user_y);
        return;
    }
    
    *internal_x = (EPD_WIDTH - 1) - user_y;
    *internal_y = user_x;
}

/**
 * @brief 创建画布对象
 */
epd_canvas_t* epd_canvas_create(uint8_t *buffer, uint16_t width, uint16_t height)
{
    epd_canvas_t *canvas;
    
    if (buffer == NULL) {
        ESP_LOGE(TAG, "缓冲区指针为空");
        return NULL;
    }
    
    canvas = (epd_canvas_t *)malloc(sizeof(epd_canvas_t));
    if (canvas == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return NULL;
    }
    
    memset(canvas, 0, sizeof(epd_canvas_t));
    
    canvas->buffer = buffer;
    canvas->width = width;
    canvas->height = height;
    canvas->internal_width = EPD_WIDTH;
    canvas->internal_height = EPD_HEIGHT;
    canvas->width_bytes = (canvas->internal_width + 7) / 8;
    canvas->color = EPD_COLOR_WHITE;
    
    ESP_LOGI(TAG, "画布创建成功：%dx%d (内部：%dx%d)", 
             width, height, canvas->internal_width, canvas->internal_height);
    
    return canvas;
}

/**
 * @brief 销毁画布对象
 */
void epd_canvas_destroy(epd_canvas_t *canvas)
{
    if (canvas != NULL) {
        free(canvas);
        ESP_LOGI(TAG, "画布已销毁");
    }
}

/**
 * @brief 清空画布
 */
void epd_canvas_clear(epd_canvas_t *canvas, uint16_t color)
{
    if (canvas == NULL || canvas->buffer == NULL) {
        ESP_LOGE(TAG, "画布未初始化");
        return;
    }
    
    memset(canvas->buffer, color, EPD_BUFFER_SIZE);
    ESP_LOGI(TAG, "画布已清空，颜色：0x%02X", color);
}

/**
 * @brief 设置像素点
 */
void epd_canvas_set_pixel(epd_canvas_t *canvas, uint16_t x, uint16_t y, uint16_t color)
{
    uint16_t internal_x, internal_y;
    uint32_t byte_addr;
    uint8_t bit_mask;
    
    if (canvas == NULL || canvas->buffer == NULL) {
        return;
    }
    
    if (x >= canvas->width || y >= canvas->height) {
        ESP_LOGW(TAG, "像素坐标越界：(%d, %d)", x, y);
        return;
    }
    
    coordinate_transform(canvas, x, y, &internal_x, &internal_y);
    
    byte_addr = (internal_x / 8) + (internal_y * canvas->width_bytes);
    bit_mask = 0x80 >> (internal_x % 8);
    
    if (color == EPD_COLOR_BLACK) {
        canvas->buffer[byte_addr] &= ~bit_mask;
    } else {
        canvas->buffer[byte_addr] |= bit_mask;
    }
}

/**
 * @brief 绘制直线（Bresenham 算法）
 */
void epd_draw_line(epd_canvas_t *canvas, uint16_t x1, uint16_t y1, 
                   uint16_t x2, uint16_t y2, uint16_t color)
{
    int dx, dy;
    int sx, sy;
    int err, e2;
    
    if (canvas == NULL) {
        return;
    }
    
    dx = abs((int)x2 - (int)x1);
    dy = abs((int)y2 - (int)y1);
    
    sx = (x1 < x2) ? 1 : -1;
    sy = (y1 < y2) ? 1 : -1;
    
    err = dx - dy;
    
    while (1) {
        epd_canvas_set_pixel(canvas, x1, y1, color);
        
        if (x1 == x2 && y1 == y2) {
            break;
        }
        
        e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

/**
 * @brief 绘制矩形
 */
void epd_draw_rectangle(epd_canvas_t *canvas, uint16_t x1, uint16_t y1, 
                        uint16_t x2, uint16_t y2, uint16_t color, uint8_t filled)
{
    uint16_t min_x, max_x, min_y, max_y;
    
    if (canvas == NULL) {
        return;
    }
    
    if (filled) {
        min_x = (x1 < x2) ? x1 : x2;
        max_x = (x1 > x2) ? x1 : x2;
        min_y = (y1 < y2) ? y1 : y2;
        max_y = (y1 > y2) ? y1 : y2;
        
        for (uint16_t y = min_y; y <= max_y; y++) {
            epd_draw_line(canvas, min_x, y, max_x, y, color);
        }
    } else {
        epd_draw_line(canvas, x1, y1, x2, y1, color);
        epd_draw_line(canvas, x2, y1, x2, y2, color);
        epd_draw_line(canvas, x2, y2, x1, y2, color);
        epd_draw_line(canvas, x1, y2, x1, y1, color);
    }
}

/**
 * @brief 绘制圆形（中点圆算法）
 */
void epd_draw_circle(epd_canvas_t *canvas, uint16_t center_x, uint16_t center_y, 
                     uint16_t radius, uint16_t color, uint8_t filled)
{
    int16_t x, y;
    int16_t err;
    
    if (canvas == NULL || radius == 0) {
        return;
    }
    
    x = 0;
    y = (int16_t)radius;
    err = 3 - 2 * radius;
    
    if (filled) {
        while (x <= y) {
            epd_draw_line(canvas, center_x - x, center_y + y, 
                         center_x + x, center_y + y, color);
            epd_draw_line(canvas, center_x - x, center_y - y, 
                         center_x + x, center_y - y, color);
            epd_draw_line(canvas, center_x - y, center_y + x, 
                         center_x + y, center_y + x, color);
            epd_draw_line(canvas, center_x - y, center_y - x, 
                         center_x + y, center_y - x, color);
            
            if (err > 0) {
                y--;
                err = err + 4 * (x - y) + 10;
            } else {
                err = err + 4 * x + 6;
            }
            x++;
        }
    } else {
        while (x <= y) {
            epd_canvas_set_pixel(canvas, center_x + x, center_y + y, color);
            epd_canvas_set_pixel(canvas, center_x - x, center_y + y, color);
            epd_canvas_set_pixel(canvas, center_x + x, center_y - y, color);
            epd_canvas_set_pixel(canvas, center_x - x, center_y - y, color);
            epd_canvas_set_pixel(canvas, center_x + y, center_y + x, color);
            epd_canvas_set_pixel(canvas, center_x - y, center_y + x, color);
            epd_canvas_set_pixel(canvas, center_x + y, center_y - x, color);
            epd_canvas_set_pixel(canvas, center_x - y, center_y - x, color);
            
            if (err > 0) {
                y--;
                err = err + 4 * (x - y) + 10;
            } else {
                err = err + 4 * x + 6;
            }
            x++;
        }
    }
}

/**
 * @brief 显示单个字符
 */
void epd_show_char(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                   char chr, const epd_font_t *font, uint16_t color)
{
    uint16_t char_index;
    const uint8_t *font_ptr;
    uint8_t font_data;
    int row, col, byte_idx, bit_idx;
    uint16_t bytes_per_line;
    
    if (canvas == NULL || font == NULL) {
        return;
    }
    
    if (chr < font->first_char || chr > font->last_char) {
        ESP_LOGW(TAG, "字符超出范围：0x%02X", chr);
        return;
    }

    char_index = (chr - font->first_char) * font->char_bytes;
    font_ptr = (const uint8_t *)font->data + char_index;
    
    // 计算每行字节数（行列式字模：每行数据连续存储）
    bytes_per_line = (font->width + 7) / 8;
    
    // 行列式字模解析：按行读取
    for (row = 0; row < font->height; row++) {
        for (col = 0; col < font->width; col++) {
            byte_idx = col / 8;
            bit_idx = col % 8;
            // 每行数据连续存储，直接按行偏移
            font_data = font_ptr[row * bytes_per_line + byte_idx];
            
            // 高位在前：bit7 对应第 1 列，bit0 对应第 8 列
            if (font_data & (0x80 >> bit_idx)) {
                epd_canvas_set_pixel(canvas, x + col, y + row, color);
            }
        }
    }
}

/**
 * @brief 计算字符串宽度（不包含字符间距）
 */
static uint16_t calculate_string_width(const char *str, const epd_font_t *font)
{
    uint16_t width = 0;
    const char *p = str;
    
    while (*p != '\0' && *p != ' ' && *p != '\n') {
        width += font->width;
        p++;
    }
    
    return width;
}

/**
 * @brief 显示字符串（支持智能换行，按单词换行）
 * @param canvas 画布对象
 * @param x 起始 X 坐标
 * @param y 起始 Y 坐标
 * @param str 字符串指针
 * @param font 字体对象指针
 * @param color 颜色
 * @param max_width 最大宽度（像素），0 表示不限制
 * @param max_height 最大高度（像素），0 表示不限制
 * @note 当 max_width>0 时，字符串超出宽度会自动换行（按单词换行，不会切断单词）
 * @note 当 max_height>0 时，超出高度会停止显示
 */
void epd_show_string_wrap(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                          const char *str, const epd_font_t *font, uint16_t color,
                          uint16_t max_width, uint16_t max_height)
{
    uint16_t current_x = x;
    uint16_t current_y = y;
    uint16_t line_height = font->height + 2;  // 行高 = 字体高度 + 2 像素行间距
    uint16_t char_spacing = 1;  // 字符间距
    const char *word_start;
    uint16_t word_width;
    uint16_t remaining_width;
    
    if (canvas == NULL || font == NULL || str == NULL) {
        return;
    }
    
    // 如果未指定最大宽度，使用画布右边界
    if (max_width == 0) {
        max_width = canvas->width;
    }
    
    // 如果未指定最大高度，使用画布下边界
    if (max_height == 0) {
        max_height = canvas->height;
    }
    
    while (*str != '\0') {
        // 检查是否超出下边界
        if (current_y + font->height > max_height) {
            break;
        }
        
        // 遇到换行符，强制换行
        if (*str == '\n') {
            current_x = x;
            current_y += line_height;
            str++;
            continue;
        }
        
        // 遇到空格，显示空格并更新位置
        if (*str == ' ') {
            current_x += font->width + char_spacing;
            str++;
            continue;
        }
        
        // 找到单词的起始位置
        word_start = str;
        
        // 计算当前单词的宽度（直到空格、换行或字符串结束）
        word_width = calculate_string_width(str, font);
        
        // 计算当前行剩余宽度
        remaining_width = max_width - current_x;
        
        // 智能换行判断：如果单词宽度 > 剩余宽度，则先换行
        if (word_width > remaining_width && current_x > x) {
            // 换到下一行
            current_x = x;
            current_y += line_height;
            
            // 检查换行后是否超出高度
            if (current_y + font->height > max_height) {
                break;
            }
        }
        
        // 显示单词中的每个字符
        while (*str != '\0' && *str != ' ' && *str != '\n') {
            // 显示字符
            epd_show_char(canvas, current_x, current_y, *str, font, color);
            
            // 更新 X 坐标（增加字符间距）
            current_x += font->width + char_spacing;
            str++;
        }
    }
}

/**
 * @brief 显示字符串（兼容旧版本，不支持自动换行）
 */
void epd_show_string(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                     const char *str, const epd_font_t *font, uint16_t color)
{
    uint16_t offset_x = 0;
    
    if (canvas == NULL || font == NULL || str == NULL) {
        return;
    }
    
    while (*str != '\0') {
        epd_show_char(canvas, x + offset_x, y, *str, font, color);
        // 增加 1 像素字符间距，避免字符间黑线
        offset_x += font->width + 1;
        str++;
    }
}

/**
 * @brief 显示数字
 */
void epd_show_num(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                  uint32_t num, uint8_t len, const epd_font_t *font, uint16_t color)
{
    uint8_t i;
    uint8_t show_num;
    
    if (canvas == NULL || font == NULL) {
        return;
    }
    
    if (len == 0) {
        len = 1;
    }
    
    for (i = 0; i < len; i++) {
        show_num = num % 10;
        epd_show_char(canvas, x + (len - i - 1) * font->width, y, 
                     show_num + '0', font, color);
        num /= 10;
    }
}
