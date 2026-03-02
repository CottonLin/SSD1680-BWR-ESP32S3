#include "epd_gui.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const char *TAG = "EPD_GUI";

/**
 * @brief 坐标转换：用户坐标 → SSD1680 内部坐标
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
    
    *internal_x = (canvas->internal_height - 1) - user_y;
    *internal_y = user_x;
}

/**
 * @brief 创建画布对象
 */
epd_canvas_t* epd_canvas_create(uint8_t *buffer_bw, uint8_t *buffer_red, 
                                uint16_t width, uint16_t height)
{
    epd_canvas_t *canvas;
    
    if (buffer_bw == NULL) {
        ESP_LOGE(TAG, "黑白缓冲区指针为空");
        return NULL;
    }
    
    canvas = (epd_canvas_t *)malloc(sizeof(epd_canvas_t));
    if (canvas == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return NULL;
    }
    
    memset(canvas, 0, sizeof(epd_canvas_t));
    
    canvas->buffer_bw = buffer_bw;
    canvas->buffer_red = buffer_red;
    canvas->width = width;
    canvas->height = height;
    canvas->internal_width = 152;
    canvas->internal_height = 296;
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
esp_err_t epd_canvas_clear(epd_canvas_t *canvas, uint16_t color)
{
    if (canvas == NULL || canvas->buffer_bw == NULL) {
        ESP_LOGE(TAG, "画布未初始化");
        return ESP_ERR_INVALID_ARG;
    }
    
    memset(canvas->buffer_bw, color, EPD_BUFFER_SIZE);
    
    if (canvas->buffer_red != NULL) {
        memset(canvas->buffer_red, color, EPD_BUFFER_SIZE);
    }
    
    ESP_LOGD(TAG, "画布已清空，颜色：0x%02X", color);
    
    return ESP_OK;
}

/**
 * @brief 设置像素点
 */
esp_err_t epd_canvas_set_pixel(epd_canvas_t *canvas, uint16_t x, uint16_t y, uint16_t color)
{
    uint16_t internal_x, internal_y;
    uint32_t byte_addr;
    uint8_t bit_mask;
    
    if (canvas == NULL || canvas->buffer_bw == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (x >= canvas->width || y >= canvas->height) {
        ESP_LOGW(TAG, "像素坐标越界：(%d, %d)", x, y);
        return ESP_ERR_INVALID_ARG;
    }
    
    coordinate_transform(canvas, x, y, &internal_x, &internal_y);
    
    byte_addr = (internal_x / 8) + (internal_y * canvas->width_bytes);
    bit_mask = 0x80 >> (internal_x % 8);
    
    if (color == EPD_COLOR_BLACK) {
        canvas->buffer_bw[byte_addr] &= ~bit_mask;
        if (canvas->buffer_red != NULL) {
            canvas->buffer_red[byte_addr] |= bit_mask;
        }
    } else if (color == EPD_COLOR_RED) {
        canvas->buffer_bw[byte_addr] |= bit_mask;
        if (canvas->buffer_red != NULL) {
            canvas->buffer_red[byte_addr] &= ~bit_mask;
        }
    } else {
        canvas->buffer_bw[byte_addr] |= bit_mask;
        if (canvas->buffer_red != NULL) {
            canvas->buffer_red[byte_addr] |= bit_mask;
        }
    }
    
    return ESP_OK;
}

/**
 * @brief 绘制直线（Bresenham 算法）
 */
esp_err_t epd_draw_line(epd_canvas_t *canvas, uint16_t x1, uint16_t y1, 
                        uint16_t x2, uint16_t y2, uint16_t color)
{
    int dx, dy;
    int sx, sy;
    int err, e2;
    
    if (canvas == NULL) {
        return ESP_ERR_INVALID_ARG;
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
    
    return ESP_OK;
}

/**
 * @brief 绘制矩形
 */
esp_err_t epd_draw_rectangle(epd_canvas_t *canvas, uint16_t x1, uint16_t y1, 
                             uint16_t x2, uint16_t y2, uint16_t color, bool filled)
{
    uint16_t min_x, max_x, min_y, max_y;
    
    if (canvas == NULL) {
        return ESP_ERR_INVALID_ARG;
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
    
    return ESP_OK;
}

/**
 * @brief 绘制圆形（中点圆算法）
 */
esp_err_t epd_draw_circle(epd_canvas_t *canvas, uint16_t center_x, uint16_t center_y, 
                          uint16_t radius, uint16_t color, bool filled)
{
    int16_t x, y;
    int16_t err;
    
    if (canvas == NULL || radius == 0) {
        return ESP_ERR_INVALID_ARG;
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
    
    return ESP_OK;
}

/**
 * @brief 绘制三角形
 */
esp_err_t epd_draw_triangle(epd_canvas_t *canvas, uint16_t x1, uint16_t y1,
                            uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                            uint16_t color, bool filled)
{
    if (canvas == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (filled) {
        int16_t min_y = (int16_t)y1;
        int16_t max_y = (int16_t)y1;
        
        if ((int16_t)y2 < min_y) min_y = (int16_t)y2;
        if ((int16_t)y3 < min_y) min_y = (int16_t)y3;
        if ((int16_t)y2 > max_y) max_y = (int16_t)y2;
        if ((int16_t)y3 > max_y) max_y = (int16_t)y3;
        
        for (int16_t y = min_y; y <= max_y; y++) {
            if (y < 0 || y >= canvas->height) continue;
            
            uint16_t x_min = 0xFFFF;
            uint16_t x_max = 0;
            bool intersect = false;
            
            if (y1 != y2) {
                uint16_t x_intersect = x1 + (uint32_t)(y - y1) * (x2 - x1) / (y2 - y1);
                if (x_intersect < x_min) x_min = x_intersect;
                if (x_intersect > x_max) x_max = x_intersect;
                intersect = true;
            }
            
            if (y2 != y3) {
                uint16_t x_intersect = x2 + (uint32_t)(y - y2) * (x3 - x2) / (y3 - y2);
                if (x_intersect < x_min) x_min = x_intersect;
                if (x_intersect > x_max) x_max = x_intersect;
                intersect = true;
            }
            
            if (y3 != y1) {
                uint16_t x_intersect = x3 + (uint32_t)(y - y3) * (x1 - x3) / (y1 - y3);
                if (x_intersect < x_min) x_min = x_intersect;
                if (x_intersect > x_max) x_max = x_intersect;
                intersect = true;
            }
            
            if (intersect && x_min <= x_max) {
                epd_draw_line(canvas, x_min, (uint16_t)y, x_max, (uint16_t)y, color);
            }
        }
    } else {
        epd_draw_line(canvas, x1, y1, x2, y2, color);
        epd_draw_line(canvas, x2, y2, x3, y3, color);
        epd_draw_line(canvas, x3, y3, x1, y1, color);
    }
    
    return ESP_OK;
}

/**
 * @brief 显示单个字符
 */
esp_err_t epd_show_char(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                        char chr, const epd_font_t *font, uint16_t color)
{
    if (canvas == NULL || font == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (chr < font->first_char || chr > font->last_char) {
        ESP_LOGW(TAG, "字符超出范围：0x%02X", chr);
        return ESP_ERR_INVALID_ARG;
    }

    // 二维数组格式：直接通过索引访问字符数据
    const uint8_t *font_data = (const uint8_t *)font->data;
    uint16_t char_index = (chr - font->first_char) * font->char_bytes;
    const uint8_t *char_ptr = font_data + char_index;
    
    uint16_t bytes_per_line = (font->width + 7) / 8;
    
    for (uint8_t row = 0; row < font->height; row++) {
        for (uint8_t col = 0; col < font->width; col++) {
            uint8_t byte_idx = col / 8;
            uint8_t bit_idx = col % 8;
            uint8_t font_data_byte = char_ptr[row * bytes_per_line + byte_idx];
            
            if (font_data_byte & (0x80 >> bit_idx)) {
                epd_canvas_set_pixel(canvas, x + col, y + row, color);
            }
        }
    }
    
    return ESP_OK;
}

/**
 * @brief 计算字符串宽度
 */
static uint16_t calculate_string_width(const char *str, const epd_font_t *font)
{
    if (str == NULL || font == NULL) {
        return 0;
    }
    
    uint16_t string_width = 0;
    const char *p = str;
    
    while (*p != '\0' && *p != ' ' && *p != '\n') {
        string_width += font->width;
        p++;
    }
    
    return string_width;
}

/**
 * @brief 显示字符串（支持智能换行）
 */
esp_err_t epd_show_string_wrap(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                               const char *str, const epd_font_t *font, uint16_t color,
                               uint16_t max_width, uint16_t max_height)
{
    uint16_t current_x = x;
    uint16_t current_y = y;
    uint16_t line_height = font->height + 2;
    uint16_t char_spacing = 1;
    
    if (canvas == NULL || font == NULL || str == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (max_width == 0) {
        max_width = canvas->width;
    }
    
    if (max_height == 0) {
        max_height = canvas->height;
    }
    
    while (*str != '\0') {
        if (current_y + font->height > max_height) {
            break;
        }
        
        if (*str == '\n') {
            current_x = x;
            current_y += line_height;
            str++;
            continue;
        }
        
        if (*str == ' ') {
            current_x += font->width + char_spacing;
            str++;
            continue;
        }
        
        uint16_t word_width = calculate_string_width(str, font);
        uint16_t remaining_width = max_width - current_x;
        
        if (word_width > remaining_width && current_x > x) {
            current_x = x;
            current_y += line_height;
            
            if (current_y + font->height > max_height) {
                break;
            }
        }
        
        while (*str != '\0' && *str != ' ' && *str != '\n') {
            epd_show_char(canvas, current_x, current_y, *str, font, color);
            current_x += font->width + char_spacing;
            str++;
        }
    }
    
    return ESP_OK;
}

/**
 * @brief 显示字符串（兼容版本）
 */
esp_err_t epd_show_string(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                          const char *str, const epd_font_t *font, uint16_t color)
{
    uint16_t offset_x = 0;
    
    if (canvas == NULL || font == NULL || str == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    while (*str != '\0') {
        epd_show_char(canvas, x + offset_x, y, *str, font, color);
        offset_x += font->width + 1;
        str++;
    }
    
    return ESP_OK;
}

/**
 * @brief 显示数字
 */
esp_err_t epd_show_num(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                       uint32_t num, uint8_t len, const epd_font_t *font, uint16_t color)
{
    if (canvas == NULL || font == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (len == 0) {
        len = 1;
    }
    
    for (uint8_t i = 0; i < len; i++) {
        uint8_t show_num = num % 10;
        epd_show_char(canvas, x + (len - i - 1) * font->width, y, 
                     show_num + '0', font, color);
        num /= 10;
    }
    
    return ESP_OK;
}

/**
 * @brief 获取字符串宽度
 */
uint16_t epd_get_string_width(const char *str, const epd_font_t *font)
{
    if (str == NULL || font == NULL) {
        return 0;
    }
    
    uint16_t char_spacing = 1;
    uint16_t width = 0;
    const char *p = str;
    
    while (*p != '\0') {
        width += font->width + char_spacing;
        p++;
    }
    
    return width;
}

/**
 * @brief 获取字符串高度
 */
uint16_t epd_get_string_height(const char *str, const epd_font_t *font, uint16_t max_width)
{
    if (str == NULL || font == NULL) {
        return 0;
    }
    
    uint16_t line_height = font->height + 2;
    uint16_t char_spacing = 1;
    
    if (max_width == 0) {
        max_width = 0xFFFF;
    }
    
    uint16_t current_width = 0;
    uint16_t line_count = 1;
    const char *p = str;
    
    while (*p != '\0') {
        if (*p == '\n') {
            line_count++;
            current_width = 0;
            p++;
            continue;
        }
        
        if (*p == ' ') {
            current_width += font->width + char_spacing;
            p++;
            continue;
        }
        
        uint16_t word_width = 0;
        const char *word_start = p;
        while (*p != '\0' && *p != ' ' && *p != '\n') {
            word_width += font->width + char_spacing;
            p++;
        }
        
        if (current_width + word_width > max_width && current_width > 0) {
            line_count++;
            current_width = 0;
        }
        
        current_width += word_width;
    }
    
    return line_count * line_height;
}
