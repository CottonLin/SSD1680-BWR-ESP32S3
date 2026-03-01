#ifndef SSD1680_H
#define SSD1680_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

// ==================== 引脚定义 ====================
// 根据你的实际接线配置
#define EPD_SPI_SCLK_PIN    GPIO_NUM_12   // VSPI CLK
#define EPD_SPI_MOSI_PIN    GPIO_NUM_11   // VSPI MOSI
#define EPD_PIN_RST         GPIO_NUM_4    // 复位引脚
#define EPD_PIN_DC          GPIO_NUM_6    // 数据/命令选择
#define EPD_PIN_CS          GPIO_NUM_21   // 片选
#define EPD_PIN_BUSY        GPIO_NUM_5    // 忙信号输入

// ==================== 屏幕参数 ====================
// SSD1680 内部 RAM 映射固定为：X 方向 152 像素（19 字节），Y 方向 296 行
// 这是 SSD1680 驱动的固定映射方式，无法更改
// 显存大小：152 * 296 / 8 = 5624 字节
#define EPD_WIDTH           152           // SSD1680 内部 X 方向宽度 (像素)
#define EPD_HEIGHT          296           // SSD1680 内部 Y 方向高度 (行)
#define EPD_BUFFER_SIZE     ((EPD_WIDTH * EPD_HEIGHT) / 8)  // 显存大小 (字节) = 5624

// ==================== 颜色定义 ====================
#define EPD_WHITE           0xFF
#define EPD_BLACK           0x00
#define EPD_RED             EPD_BLACK     // 红色通道使用黑色

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
typedef struct {
    spi_host_device_t spi_host;
    spi_device_handle_t spi_handle;
    gpio_num_t pin_rst;
    gpio_num_t pin_dc;
    gpio_num_t pin_cs;
    gpio_num_t pin_busy;
    uint8_t *buffer_bw;      // 黑白显存
    uint8_t *buffer_red;     // 红色显存
} epd_handle_t;

// ==================== 函数声明 ====================
/**
 * @brief 初始化 SSD1680 显示屏
 * @return esp_err_t 错误代码
 */
esp_err_t epd_init(void);

/**
 * @brief 硬件复位
 */
void epd_hw_reset(void);

/**
 * @brief 等待忙信号结束
 */
void epd_wait_busy(void);

/**
 * @brief 写入命令
 * @param cmd 命令字节
 */
void epd_write_cmd(uint8_t cmd);

/**
 * @brief 写入数据
 * @param data 数据字节
 */
void epd_write_data(uint8_t data);

/**
 * @brief 写入显存数据
 * @param buffer_bw 黑白显存指针
 * @param buffer_red 红色显存指针
 */
void epd_display(const uint8_t *buffer_bw, const uint8_t *buffer_red);

/**
 * @brief 清屏 (填充白色)
 */
void epd_clear(void);

/**
 * @brief 刷新显示
 */
void epd_update(void);

/**
 * @brief 进入深度睡眠
 */
void epd_deep_sleep(void);

#endif // SSD1680_H
