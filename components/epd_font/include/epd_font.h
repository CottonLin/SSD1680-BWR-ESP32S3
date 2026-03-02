#ifndef EPD_FONT_H
#define EPD_FONT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 字体对象结构
 * 
 * 字体数据结构说明：
 * - 前 6 字节为字体头信息：
 *   - [0]: first_char (第一个字符的 ASCII 码)
 *   - [1]: last_char (最后一个字符的 ASCII 码)
 *   - [2]: width (字体宽度，像素)
 *   - [3]: height (字体高度，像素)
 *   - [4-5]: char_bytes (每个字符的字节数，小端序)
 * - 从第 6 字节开始为字模数据
 */
typedef struct {
    const uint8_t *data;      /**< 字模数据指针（包含字体头） */
    uint8_t width;            /**< 字体宽度 (像素) */
    uint8_t height;           /**< 字体高度 (像素) */
    uint8_t first_char;       /**< 第一个字符的 ASCII 码 */
    uint8_t last_char;        /**< 最后一个字符的 ASCII 码 */
    uint16_t char_bytes;      /**< 每个字符的字节数 */
} epd_font_t;

/**
 * @brief 字体大小枚举
 * 
 * 支持的字体规格：
 * - 6×8: 超紧凑字体，适合空间受限的显示
 * - 6×12: 窄高字体，适合特殊布局
 * - 8×16: 标准宽度字体，通用性强
 * - 12×24: 大标题字体，醒目清晰
 */
typedef enum {
    EPD_FONT_SIZE_6X8 = 0,        /**< 6×8 字体 */
    EPD_FONT_SIZE_6X12,           /**< 6×12 字体 */
    EPD_FONT_SIZE_8X16,           /**< 8×16 字体 */
    EPD_FONT_SIZE_12X24           /**< 12×24 字体 */
} epd_font_size_t;

/**
 * @brief 字体数据外部声明
 * 
 * 使用说明：
 * 1. 用户需要在外部文件中定义字模数据数组
 * 2. 字模数据格式要求：
 *    - 阴码点阵
 *    - 逐行式取模
 *    - 顺向（高位在前）
 *    - C51 格式
 * 3. 字模数据数组必须包含字体头（前 6 字节）
 * 
 * 示例（以 8×16 字体为例）：
 * 
 * const uint8_t g_font_8x16_data[] = {
 *     // 字体头（6 字节）
 *     0x20,       // first_char: 第一个字符 (空格)
 *     0x7E,       // last_char: 最后一个字符 (~)
 *     0x08,       // width: 宽度 8 像素
 *     0x10,       // height: 高度 16 像素
 *     0x10, 0x00, // char_bytes: 每个字符 16 字节 (小端序)
 *     
 *     // 字模数据（从空格字符开始）
 *     // 字符 0x20 (空格): 16 字节
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *     
 *     // 字符 0x21 (!): 16 字节
 *     0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10,
 *     0x10, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00,
 *     
 *     // ... 后续字符
 * };
 * 
 * const epd_font_t g_font_8x16 = {
 *     .data = g_font_8x16_data,
 *     .width = 8,
 *     .height = 16,
 *     .first_char = 0x20,
 *     .last_char = 0x7E,
 *     .char_bytes = 16
 * };
 */

/**
 * @brief 6×8 字体数据（用户需自行导入）
 * 
 * 字模数据位置标注：
 * - 数组名称：g_font_6x8_data
 * - 数据大小：95 字符 × 8 字节 + 6 字节头 = 766 字节
 * - 字符范围：0x20 (空格) ~ 0x7E (~)
 * 
 * 使用方法：
 * 1. 在用户文件中定义 const uint8_t g_font_6x8_data[766]
 * 2. 按照上述格式填充字体头和字模数据
 * 3. 在本文件中 extern 声明
 */
extern const uint8_t g_font_6x8_data[];

/**
 * @brief 6×12 字体数据（用户需自行导入）
 * 
 * 字模数据位置标注：
 * - 数组名称：g_font_6x12_data
 * - 数据大小：95 字符 × 12 字节 + 6 字节头 = 1146 字节
 * - 字符范围：0x20 (空格) ~ 0x7E (~)
 * 
 * 使用方法：
 * 1. 在用户文件中定义 const uint8_t g_font_6x12_data[1146]
 * 2. 按照上述格式填充字体头和字模数据
 * 3. 在本文件中 extern 声明
 */
extern const uint8_t g_font_6x12_data[];

/**
 * @brief 8×16 字体数据（用户需自行导入）
 * 
 * 字模数据位置标注：
 * - 数组名称：g_font_8x16_data
 * - 数据大小：95 字符 × 16 字节 + 6 字节头 = 1526 字节
 * - 字符范围：0x20 (空格) ~ 0x7E (~)
 * 
 * 使用方法：
 * 1. 在用户文件中定义 const uint8_t g_font_8x16_data[1526]
 * 2. 按照上述格式填充字体头和字模数据
 * 3. 在本文件中 extern 声明
 */
extern const uint8_t g_font_8x16_data[];

/**
 * @brief 12×24 字体数据（用户需自行导入）
 * 
 * 字模数据位置标注：
 * - 数组名称：g_font_12x24_data
 * - 数据大小：95 字符 × 48 字节 + 6 字节头 = 4566 字节
 * - 字符范围：0x20 (空格) ~ 0x7E (~)
 * 
 * 使用方法：
 * 1. 在用户文件中定义 const uint8_t g_font_12x24_data[4566]
 * 2. 按照上述格式填充字体头和字模数据
 * 3. 在本文件中 extern 声明
 */
extern const uint8_t g_font_12x24_data[];

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

/**
 * @brief 初始化字体系统
 * @return esp_err_t
 *         - ESP_OK: 成功
 *         - ESP_ERR_NOT_SUPPORTED: 字体数据未导入
 */
esp_err_t epd_font_init(void);

#ifdef __cplusplus
}
#endif

#endif /* EPD_FONT_H */
