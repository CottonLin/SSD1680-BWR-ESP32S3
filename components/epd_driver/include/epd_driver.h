/**
 * @file epd_driver.h
 * @brief SSD1680 硬件驱动头文件
 * 
 * @version 1.0
 * @date 2026-03-02
 */

#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// ==================== 引脚定义 ====================
// 默认引脚配置
#define EPD_SPI_SCLK_PIN_DEFAULT    GPIO_NUM_12   // VSPI CLK
#define EPD_SPI_MOSI_PIN_DEFAULT    GPIO_NUM_11   // VSPI MOSI
#define EPD_PIN_RST_DEFAULT         GPIO_NUM_4    // 复位引脚
#define EPD_PIN_DC_DEFAULT          GPIO_NUM_6    // 数据/命令选择
#define EPD_PIN_CS_DEFAULT          GPIO_NUM_21   // 片选
#define EPD_PIN_BUSY_DEFAULT        GPIO_NUM_5    // 忙信号输入

// ==================== 屏幕参数 ====================
// SSD1680 内部 RAM 映射固定为：X 方向 152 像素（19 字节），Y 方向 296 行
#define EPD_WIDTH                   152           // SSD1680 内部 X 方向宽度 (像素)
#define EPD_HEIGHT                  296           // SSD1680 内部 Y 方向高度 (行)
#define EPD_BUFFER_SIZE             ((EPD_WIDTH * EPD_HEIGHT) / 8)  // 显存大小 = 5624 字节

// ==================== 颜色定义 ====================
#define EPD_WHITE                   0xFF
#define EPD_BLACK                   0x00
#define EPD_RED                     EPD_BLACK     // 红色通道使用黑色

// ==================== SSD1680 命令寄存器 ====================
#define SSD1680_DRIVER_OUTPUT_CTRL          0x01
#define SSD1680_GATE_DRIVING_VOLTAGE        0x03
#define SSD1680_SOURCE_DRIVING_VOLTAGE      0x04
#define SSD1680_BOOSTER_SOFT_START          0x0C
#define SSD1680_GATE_SCAN_START_POSITION    0x0F
#define SSD1680_DEEP_SLEEP_MODE             0x10
#define SSD1680_DATA_ENTRY_MODE             0x11
#define SSD1680_SW_RESET                    0x12
#define SSD1680_TEMP_SENSOR_CONTROL         0x18
#define SSD1680_TEMP_SENSOR_CONTROL_WRITE   0x1A
#define SSD1680_MASTER_ACTIVATION           0x20
#define SSD1680_DISPLAY_UPDATE_CTRL_1       0x21
#define SSD1680_DISPLAY_UPDATE_CTRL_2       0x22
#define SSD1680_WRITE_RAM_BW                0x24
#define SSD1680_WRITE_RAM_RED               0x26
#define SSD1680_WRITE_VCOM_VALUE            0x2C
#define SSD1680_WRITE_LUT_REGISTER          0x32
#define SSD1680_SET_DUMMY_LINE_PERIOD       0x3A
#define SSD1680_SET_GATE_LINE_WIDTH         0x3B
#define SSD1680_BORDER_WAVEFORM_CTRL        0x3C
#define SSD1680_SET_RAM_X_ADDRESS_RANGE     0x44
#define SSD1680_SET_RAM_Y_ADDRESS_RANGE     0x45
#define SSD1680_SET_RAM_X_ADDRESS_COUNTER   0x4E
#define SSD1680_SET_RAM_Y_ADDRESS_COUNTER   0x4F

// ==================== 数据结构 ====================

/**
 * @brief SSD1680 设备配置结构
 */
typedef struct {
    gpio_num_t pin_rst;         /*!< 复位引脚，默认 GPIO_NUM_4 */
    gpio_num_t pin_dc;          /*!< 数据/命令选择引脚，默认 GPIO_NUM_6 */
    gpio_num_t pin_cs;          /*!< 片选引脚，默认 GPIO_NUM_21 */
    gpio_num_t pin_busy;        /*!< 忙信号引脚，默认 GPIO_NUM_5 */
    gpio_num_t pin_mosi;        /*!< MOSI 引脚，默认 GPIO_NUM_11 */
    gpio_num_t pin_sclk;        /*!< SCLK 引脚，默认 GPIO_NUM_12 */
    uint32_t spi_freq_hz;       /*!< SPI 频率 (Hz)，默认 8000000 */
    spi_host_device_t spi_host; /*!< SPI 主机，默认 SPI2_HOST */
} epd_config_t;

/**
 * @brief SSD1680 设备句柄
 */
typedef struct {
    spi_device_handle_t spi_handle;   /*!< SPI 设备句柄 */
    gpio_num_t pin_rst;               /*!< 复位引脚 */
    gpio_num_t pin_dc;                /*!< 数据/命令选择引脚 */
    gpio_num_t pin_cs;                /*!< 片选引脚 */
    gpio_num_t pin_busy;              /*!< 忙信号引脚 */
    gpio_num_t pin_mosi;              /*!< MOSI 引脚 */
    gpio_num_t pin_sclk;              /*!< SCLK 引脚 */
    spi_host_device_t spi_host;       /*!< SPI 主机 */
    uint8_t *buffer_bw;               /*!< 黑白显存指针 */
    uint8_t *buffer_red;              /*!< 红色显存指针 */
    bool initialized;                 /*!< 初始化标志 */
} epd_handle_t;

// ==================== 默认配置 ====================

/**
 * @brief 获取默认配置
 * @return epd_config_t 默认配置结构
 */
static inline epd_config_t epd_get_default_config(void)
{
    epd_config_t config = {
        .pin_rst = EPD_PIN_RST_DEFAULT,
        .pin_dc = EPD_PIN_DC_DEFAULT,
        .pin_cs = EPD_PIN_CS_DEFAULT,
        .pin_busy = EPD_PIN_BUSY_DEFAULT,
        .pin_mosi = EPD_SPI_MOSI_PIN_DEFAULT,
        .pin_sclk = EPD_SPI_SCLK_PIN_DEFAULT,
        .spi_freq_hz = 8000000,
        .spi_host = SPI2_HOST
    };
    return config;
}

// ==================== 核心接口 ====================

/**
 * @brief 初始化 SSD1680 设备
 * 
 * @param config 设备配置指针，NULL 使用默认配置
 * @return epd_handle_t* 设备句柄，NULL 表示失败
 * 
 * @note 默认配置:
 *  - SPI 频率：8MHz
 *  - 引脚：使用默认引脚定义
 *  - SPI 主机：SPI2_HOST
 */
epd_handle_t* epd_init(const epd_config_t *config);

/**
 * @brief 获取设备句柄
 * 
 * @return epd_handle_t* 设备句柄，NULL 表示未初始化
 * 
 * @note 在 epd_init() 后调用此函数获取句柄
 */
epd_handle_t* epd_get_handle(void);

/**
 * @brief 反初始化设备
 * 
 * @param handle 设备句柄
 * @return esp_err_t 
 *  - ESP_OK: 成功
 *  - ESP_ERR_INVALID_ARG: 句柄无效
 */
esp_err_t epd_deinit(epd_handle_t *handle);

/**
 * @brief 写入显存数据并刷新显示
 * 
 * @param handle 设备句柄
 * @param buffer_bw 黑白显存数据指针
 * @param buffer_red 红色显存数据指针
 * @return esp_err_t
 *  - ESP_OK: 成功
 *  - ESP_ERR_INVALID_ARG: 句柄无效或缓冲区为空
 *  - ESP_FAIL: 刷新失败
 */
esp_err_t epd_display(epd_handle_t *handle, 
                      const uint8_t *buffer_bw, 
                      const uint8_t *buffer_red);

/**
 * @brief 清屏（填充白色）
 * 
 * @param handle 设备句柄
 * @return esp_err_t
 */
esp_err_t epd_clear(epd_handle_t *handle);

/**
 * @brief 刷新显示
 * 
 * @param handle 设备句柄
 * @return esp_err_t
 */
esp_err_t epd_update(epd_handle_t *handle);

/**
 * @brief 进入深度睡眠模式
 * 
 * @param handle 设备句柄
 * @return esp_err_t
 */
esp_err_t epd_deep_sleep(epd_handle_t *handle);

/**
 * @brief 唤醒设备（从深度睡眠）
 * 
 * @param handle 设备句柄
 * @return esp_err_t
 */
esp_err_t epd_wake_up(epd_handle_t *handle);

// ==================== 底层操作接口（内部使用） ====================

/**
 * @brief 硬件复位
 * @param handle 设备句柄
 */
void epd_hw_reset(epd_handle_t *handle);

/**
 * @brief 等待忙信号结束
 * @param handle 设备句柄
 */
void epd_wait_busy(epd_handle_t *handle);

/**
 * @brief 写入命令
 * @param handle 设备句柄
 * @param cmd 命令字节
 */
void epd_write_cmd(epd_handle_t *handle, uint8_t cmd);

/**
 * @brief 写入数据
 * @param handle 设备句柄
 * @param data 数据字节
 */
void epd_write_data(epd_handle_t *handle, uint8_t data);

// ==================== 兼容层接口（保留旧接口） ====================

/**
 * @brief 初始化显示屏（兼容旧版本）
 * @return esp_err_t
 * @deprecated 请使用 epd_init() 代替
 */
esp_err_t epd_init_legacy(void);

/**
 * @brief 清屏（兼容旧版本）
 * @deprecated 请使用 epd_clear() 代替
 */
void epd_clear_legacy(void);

/**
 * @brief 刷新显示（兼容旧版本）
 * @deprecated 请使用 epd_update() 代替
 */
void epd_update_legacy(void);

/**
 * @brief 写入显存并刷新（兼容旧版本）
 * @param buffer_bw 黑白显存
 * @param buffer_red 红色显存
 * @deprecated 请使用 epd_display() 代替
 */
void epd_display_legacy(const uint8_t *buffer_bw, const uint8_t *buffer_red);

/**
 * @brief 进入深度睡眠（兼容旧版本）
 * @deprecated 请使用 epd_deep_sleep() 代替
 */
void epd_deep_sleep_legacy(void);

#ifdef __cplusplus
}
#endif

#endif // EPD_DRIVER_H
