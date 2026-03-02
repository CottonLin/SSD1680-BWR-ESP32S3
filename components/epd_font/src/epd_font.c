#include "epd_font.h"
#include "esp_log.h"
#include "esp_err.h"
#include <string.h>

static const char *TAG = "EPD_FONT";

/**
 * @brief 6×8 字体对象
 */
static const epd_font_t g_font_6x8 = {
    .data = g_font_6x8_data,
    .width = 6,
    .height = 8,
    .first_char = 0x20,
    .last_char = 0x7E,
    .char_bytes = 8
};

/**
 * @brief 6×12 字体对象
 */
static const epd_font_t g_font_6x12 = {
    .data = g_font_6x12_data,
    .width = 6,
    .height = 12,
    .first_char = 0x20,
    .last_char = 0x7E,
    .char_bytes = 12
};

/**
 * @brief 8×16 字体对象
 */
static const epd_font_t g_font_8x16 = {
    .data = g_font_8x16_data,
    .width = 8,
    .height = 16,
    .first_char = 0x20,
    .last_char = 0x7E,
    .char_bytes = 16
};

/**
 * @brief 12×24 字体对象
 */
static const epd_font_t g_font_12x24 = {
    .data = g_font_12x24_data,
    .width = 12,
    .height = 24,
    .first_char = 0x20,
    .last_char = 0x7E,
    .char_bytes = 48
};

/**
 * @brief 获取字体对象
 */
const epd_font_t* epd_font_get(epd_font_size_t size)
{
    switch (size) {
    case EPD_FONT_SIZE_6X8:
        return &g_font_6x8;
    case EPD_FONT_SIZE_6X12:
        return &g_font_6x12;
    case EPD_FONT_SIZE_8X16:
        return &g_font_8x16;
    case EPD_FONT_SIZE_12X24:
        return &g_font_12x24;
    default:
        ESP_LOGE(TAG, "不支持的字体大小：%d", size);
        return NULL;
    }
}

/**
 * @brief 获取字符串像素宽度
 */
uint16_t epd_font_get_string_width(const epd_font_t *font, const char *str)
{
    if (font == NULL || str == NULL) {
        return 0;
    }
    
    size_t len = strlen(str);
    return (uint16_t)(len * font->width);
}

/**
 * @brief 初始化字体系统
 */
esp_err_t epd_font_init(void)
{
    ESP_LOGI(TAG, "字体系统初始化");
    
    if (g_font_6x8_data == NULL || g_font_8x16_data == NULL) {
        ESP_LOGW(TAG, "字体数据未导入，请用户自行添加字模数据");
        return ESP_ERR_NOT_SUPPORTED;
    }
    
    ESP_LOGI(TAG, "字体数据加载成功");
    ESP_LOGI(TAG, "  - 6×8 字体：就绪");
    ESP_LOGI(TAG, "  - 8×16 字体：就绪");
    
    return ESP_OK;
}
