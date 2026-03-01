#ifndef EPD_FONT_H
#define EPD_FONT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 字体对象结构
 */
typedef struct {
    const void *data;      /**< 字模数据指针 */
    uint8_t width;            /**< 字体宽度 (像素) */
    uint8_t height;           /**< 字体高度 (像素) */
    uint8_t first_char;       /**< 第一个字符的 ASCII 码 */
    uint8_t last_char;        /**< 最后一个字符的 ASCII 码 */
    uint16_t char_bytes;      /**< 每个字符的字节数 */
} epd_font_t;

/**
 * @brief 字体大小枚举
 * 
 * 支持的字体规格（行列式）：
 * - 6×8: 超紧凑字体，适合空间受限的显示
 * - 6×12: 窄高字体，适合特殊布局
 * - 8×16: 标准宽度字体，通用性强
 * - 12×24: 大标题字体，醒目清晰
 */
typedef enum {
    EPD_FONT_SIZE_6X8 = 0,        /**< 6×8 行列式字体 (asc2_0806) */
    EPD_FONT_SIZE_6X12,           /**< 6×12 行列式字体 (asc2_1206) */
    EPD_FONT_SIZE_8X16,           /**< 8×16 行列式字体 (asc2_1608) */
    EPD_FONT_SIZE_12X24           /**< 12×24 行列式字体 (asc2_2412) */
} epd_font_size_t;

/**
 * @brief 获取字体对象
 * @param size 字体大小
 * @return 字体对象指针，NULL 表示不支持的字体
 */
const epd_font_t* epd_font_get(epd_font_size_t size);

/**
 * @brief 获取字符宽度
 * @param font 字体对象指针
 * @return 字符宽度 (像素)
 */
static inline uint8_t epd_font_get_width(const epd_font_t *font)
{
    return font ? font->width : 0;
}

/**
 * @brief 获取字符高度
 * @param font 字体对象指针
 * @return 字符高度 (像素)
 */
static inline uint8_t epd_font_get_height(const epd_font_t *font)
{
    return font ? font->height : 0;
}

/**
 * @brief 获取字符串像素宽度
 * @param font 字体对象指针
 * @param str 字符串
 * @return 字符串总宽度 (像素)
 */
uint16_t epd_font_get_string_width(const epd_font_t *font, const char *str);

#ifdef __cplusplus
}
#endif

#endif /* EPD_FONT_H */
