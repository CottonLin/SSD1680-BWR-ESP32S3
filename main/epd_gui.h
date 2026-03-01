#ifndef EPD_GUI_H
#define EPD_GUI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ssd1680.h"
#include "epd_font.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 画布对象结构
 * 
 * 坐标系统说明：
 * - 用户坐标：物理屏幕坐标，X[0-295], Y[0-151]
 * - 内部坐标：SSD1680 RAM 映射坐标，X[0-151], Y[0-295]
 * - 自动转换：用户坐标 → 内部坐标（转置）
 */
typedef struct {
    uint8_t *buffer;            /**< 显存缓冲区指针 */
    uint16_t width;             /**< 逻辑宽度（用户坐标，296） */
    uint16_t height;            /**< 逻辑高度（用户坐标，152） */
    uint16_t internal_width;    /**< 内部宽度（SSD1680 X 方向，152） */
    uint16_t internal_height;   /**< 内部高度（SSD1680 Y 方向，296） */
    uint16_t width_bytes;       /**< 每行字节数（internal_width/8 = 19） */
    uint16_t color;             /**< 当前颜色 */
} epd_canvas_t;

/**
 * @brief 颜色定义
 */
#define EPD_COLOR_WHITE     0xFF    /**< 白色 */
#define EPD_COLOR_BLACK     0x00    /**< 黑色 */

/**
 * @brief 创建画布对象
 * 
 * @param buffer 显存缓冲区（由用户分配，大小为 EPD_BUFFER_SIZE）
 * @param width 逻辑宽度（用户坐标，通常 296）
 * @param height 逻辑高度（用户坐标，通常 152）
 * @return 画布对象指针，NULL 表示失败
 */
epd_canvas_t* epd_canvas_create(uint8_t *buffer, uint16_t width, uint16_t height);

/**
 * @brief 销毁画布对象
 * 
 * @param canvas 画布对象指针
 */
void epd_canvas_destroy(epd_canvas_t *canvas);

/**
 * @brief 清空画布
 * 
 * @param canvas 画布对象指针
 * @param color 填充颜色（EPD_COLOR_WHITE 或 EPD_COLOR_BLACK）
 */
void epd_canvas_clear(epd_canvas_t *canvas, uint16_t color);

/**
 * @brief 设置像素点（自动坐标转换）
 * 
 * @param canvas 画布对象指针
 * @param x 用户 X 坐标 [0, width-1]
 * @param y 用户 Y 坐标 [0, height-1]
 * @param color 颜色
 */
void epd_canvas_set_pixel(epd_canvas_t *canvas, uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief 绘制直线（Bresenham 算法）
 * 
 * @param canvas 画布对象指针
 * @param x1 起点 X 坐标
 * @param y1 起点 Y 坐标
 * @param x2 终点 X 坐标
 * @param y2 终点 Y 坐标
 * @param color 颜色
 */
void epd_draw_line(epd_canvas_t *canvas, uint16_t x1, uint16_t y1, 
                   uint16_t x2, uint16_t y2, uint16_t color);

/**
 * @brief 绘制矩形
 * 
 * @param canvas 画布对象指针
 * @param x1 左上角 X 坐标
 * @param y1 左上角 Y 坐标
 * @param x2 右下角 X 坐标
 * @param y2 右下角 Y 坐标
 * @param color 颜色
 * @param filled 填充模式
 *        - 0: 空心矩形（仅边框）
 *        - 1: 实心矩形（填充）
 */
void epd_draw_rectangle(epd_canvas_t *canvas, uint16_t x1, uint16_t y1, 
                        uint16_t x2, uint16_t y2, uint16_t color, uint8_t filled);

/**
 * @brief 绘制圆形（中点圆算法）
 * 
 * @param canvas 画布对象指针
 * @param center_x 圆心 X 坐标
 * @param center_y 圆心 Y 坐标
 * @param radius 半径
 * @param color 颜色
 * @param filled 填充模式
 *        - 0: 空心圆（仅圆周）
 *        - 1: 实心圆（填充）
 */
void epd_draw_circle(epd_canvas_t *canvas, uint16_t center_x, uint16_t center_y, 
                     uint16_t radius, uint16_t color, uint8_t filled);

/**
 * @brief 显示单个字符
 * 
 * @param canvas 画布对象指针
 * @param x X 坐标（左上角）
 * @param y Y 坐标（左上角）
 * @param chr 要显示的字符
 * @param font 字体对象指针
 * @param color 颜色
 */
void epd_show_char(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                   char chr, const epd_font_t *font, uint16_t color);

/**
 * @brief 显示字符串
 * 
 * @param canvas 画布对象指针
 * @param x X 坐标（左上角）
 * @param y Y 坐标（左上角）
 * @param str 要显示的字符串
 * @param font 字体对象指针
 * @param color 颜色
 */
void epd_show_string(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                     const char *str, const epd_font_t *font, uint16_t color);

/**
 * @brief 显示数字
 * 
 * @param canvas 画布对象指针
 * @param x X 坐标（左上角）
 * @param y Y 坐标（左上角）
 * @param num 要显示的数字
 * @param len 数字位数（显示宽度）
 * @param font 字体对象指针
 * @param color 颜色
 */
void epd_show_num(epd_canvas_t *canvas, uint16_t x, uint16_t y, 
                  uint32_t num, uint8_t len, const epd_font_t *font, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif /* EPD_GUI_H */
