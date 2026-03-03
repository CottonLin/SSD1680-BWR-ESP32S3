/**
 * @file epd_driver.c
 * @brief SSD1680 硬件驱动实现
 * 
 * @version 1.0
 * @date 2026-03-02
 */

#include "epd_driver.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "EPD_DRIVER";

// ==================== 全局设备句柄（兼容层） ====================
static epd_handle_t *g_epd_legacy = NULL;

// ==================== 公共接口实现 ====================

/**
 * @brief 获取设备句柄
 */
epd_handle_t* epd_get_handle(void)
{
    return g_epd_legacy;
}

// ==================== 内部辅助函数 ====================

/**
 * @brief 初始化 GPIO 引脚
 */
static void epd_gpio_init(epd_handle_t *handle)
{
    if (handle == NULL) {
        return;
    }
    
    // 配置 RST 引脚 (输出)
    gpio_reset_pin(handle->pin_rst);
    gpio_set_direction(handle->pin_rst, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(handle->pin_rst, GPIO_PULLUP_ONLY);
    
    // 配置 DC 引脚 (输出)
    gpio_reset_pin(handle->pin_dc);
    gpio_set_direction(handle->pin_dc, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(handle->pin_dc, GPIO_PULLUP_ONLY);
    
    // 配置 CS 引脚 (输出)
    gpio_reset_pin(handle->pin_cs);
    gpio_set_direction(handle->pin_cs, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(handle->pin_cs, GPIO_PULLUP_ONLY);
    
    // 配置 BUSY 引脚 (输入)
    gpio_reset_pin(handle->pin_busy);
    gpio_set_direction(handle->pin_busy, GPIO_MODE_INPUT);
    gpio_set_pull_mode(handle->pin_busy, GPIO_PULLUP_ONLY);
    
    // 初始状态：CS 高电平（禁用），DC 高电平（数据模式）
    gpio_set_level(handle->pin_cs, 1);
    gpio_set_level(handle->pin_dc, 1);
    gpio_set_level(handle->pin_rst, 1);
}

/**
 * @brief 初始化 SPI 总线
 */
static esp_err_t epd_spi_init(epd_handle_t *handle, uint32_t freq_hz)
{
    esp_err_t ret;
    
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // SPI 总线配置
    spi_bus_config_t buscfg = {
        .mosi_io_num = handle->pin_mosi,
        .miso_io_num = -1,  // SSD1680 不需要 MISO
        .sclk_io_num = handle->pin_sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = EPD_BUFFER_SIZE * 2
    };
    
    // SPI 设备配置
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = freq_hz,
        .mode = 0,  // SPI Mode 0 (CPOL=0, CPHA=0)
        .spics_io_num = handle->pin_cs,
        .queue_size = 1,
        .flags = SPI_DEVICE_NO_DUMMY
    };
    
    // 初始化 SPI 总线
    ret = spi_bus_initialize(handle->spi_host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI 总线初始化失败");
        return ret;
    }
    
    // 添加 SPI 设备
    ret = spi_bus_add_device(handle->spi_host, &devcfg, &handle->spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI 设备添加失败");
        spi_bus_free(handle->spi_host);
        return ret;
    }
    
    ESP_LOGI(TAG, "SPI 初始化完成 (频率：%lu Hz)", freq_hz);
    return ESP_OK;
}

/**
 * @brief 分配显存缓冲区
 */
static esp_err_t epd_allocate_buffers(epd_handle_t *handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 分配黑白显存
    handle->buffer_bw = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    if (handle->buffer_bw == NULL) {
        ESP_LOGE(TAG, "黑白显存分配失败");
        return ESP_ERR_NO_MEM;
    }
    
    // 分配红色显存
    handle->buffer_red = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    if (handle->buffer_red == NULL) {
        ESP_LOGE(TAG, "红色显存分配失败");
        free(handle->buffer_bw);
        handle->buffer_bw = NULL;
        return ESP_ERR_NO_MEM;
    }
    
    // 初始化为白色
    memset(handle->buffer_bw, 0xFF, EPD_BUFFER_SIZE);
    memset(handle->buffer_red, 0xFF, EPD_BUFFER_SIZE);
    
    ESP_LOGI(TAG, "显存分配完成 (%d 字节)", EPD_BUFFER_SIZE);
    return ESP_OK;
}

/**
 * @brief 释放显存缓冲区
 */
static void epd_free_buffers(epd_handle_t *handle)
{
    if (handle == NULL) {
        return;
    }
    
    if (handle->buffer_bw != NULL) {
        free(handle->buffer_bw);
        handle->buffer_bw = NULL;
    }
    
    if (handle->buffer_red != NULL) {
        free(handle->buffer_red);
        handle->buffer_red = NULL;
    }
    
    ESP_LOGI(TAG, "显存已释放");
}

// ==================== 底层操作实现 ====================

/**
 * @brief 等待忙信号结束
 */
void epd_wait_busy(epd_handle_t *handle)
{
    if (handle == NULL) {
        return;
    }
    
    // 检测 BUSY 引脚，直到低电平
    while (gpio_get_level(handle->pin_busy) == 1) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/**
 * @brief 硬件复位
 */
void epd_hw_reset(epd_handle_t *handle)
{
    if (handle == NULL) {
        return;
    }
    
    // 复位序列：拉低 RST -> 延时 -> 拉高 RST -> 等待 BUSY 结束
    gpio_set_level(handle->pin_rst, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(handle->pin_rst, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    epd_wait_busy(handle);
    
    ESP_LOGI(TAG, "硬件复位完成");
}

/**
 * @brief 写入命令
 */
void epd_write_cmd(epd_handle_t *handle, uint8_t cmd)
{
    if (handle == NULL) {
        return;
    }
    
    esp_err_t ret;
    
    // DC=0 表示命令
    gpio_set_level(handle->pin_dc, 0);
    
    // 发送命令字节
    uint8_t data = cmd;
    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &data
    };
    
    ret = spi_device_transmit(handle->spi_handle, &transaction);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI 命令发送失败");
    }
    
    // 恢复 DC=1 (数据模式)
    gpio_set_level(handle->pin_dc, 1);
}

/**
 * @brief 写入数据
 */
void epd_write_data(epd_handle_t *handle, uint8_t data)
{
    if (handle == NULL) {
        return;
    }
    
    esp_err_t ret;
    
    // DC=1 表示数据
    gpio_set_level(handle->pin_dc, 1);
    
    // 发送数据字节
    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &data
    };
    
    ret = spi_device_transmit(handle->spi_handle, &transaction);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI 数据发送失败");
    }
}

/**
 * @brief 写入多个数据
 */
static void epd_write_data_batch(epd_handle_t *handle, const uint8_t *data, size_t len)
{
    if (handle == NULL || data == NULL) {
        return;
    }
    
    esp_err_t ret;
    
    gpio_set_level(handle->pin_dc, 1);
    
    spi_transaction_t transaction = {
        .length = len * 8,
        .tx_buffer = data
    };
    
    ret = spi_device_transmit(handle->spi_handle, &transaction);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI 批量数据发送失败");
    }
}

// ==================== 核心接口实现 ====================

/**
 * @brief 初始化 SSD1680 设备
 */
epd_handle_t* epd_init(const epd_config_t *config)
{
    esp_err_t ret;
    epd_handle_t *handle;
    
    ESP_LOGI(TAG, "初始化 SSD1680 显示屏");
    
    // 1. 分配设备句柄
    handle = (epd_handle_t *)malloc(sizeof(epd_handle_t));
    if (handle == NULL) {
        ESP_LOGE(TAG, "设备句柄分配失败");
        return NULL;
    }
    memset(handle, 0, sizeof(epd_handle_t));
    
    // 2. 应用配置
    epd_config_t default_config;
    if (config == NULL) {
        default_config = epd_get_default_config();
        config = &default_config;
    }
    
    handle->pin_rst = config->pin_rst;
    handle->pin_dc = config->pin_dc;
    handle->pin_cs = config->pin_cs;
    handle->pin_busy = config->pin_busy;
    handle->pin_mosi = config->pin_mosi;
    handle->pin_sclk = config->pin_sclk;
    handle->spi_host = config->spi_host;
    
    // 3. 初始化 GPIO
    epd_gpio_init(handle);
    
    // 4. 初始化 SPI 总线
    ret = epd_spi_init(handle, config->spi_freq_hz);
    if (ret != ESP_OK) {
        free(handle);
        return NULL;
    }
    
    // 5. 分配显存缓冲区
    ret = epd_allocate_buffers(handle);
    if (ret != ESP_OK) {
        spi_bus_free(handle->spi_host);
        free(handle);
        return NULL;
    }
    
    // 6. 硬件复位
    epd_hw_reset(handle);
    
    // 7. 软件复位
    epd_write_cmd(handle, SSD1680_SW_RESET);
    epd_wait_busy(handle);
    
    // 8. 配置显示屏参数
    // 设置 Border 波形
    epd_write_cmd(handle, SSD1680_BORDER_WAVEFORM_CTRL);
    epd_write_data(handle, 0x05);
    
    // 设置 Driver Output (295 行 - 1)
    epd_write_cmd(handle, SSD1680_DRIVER_OUTPUT_CTRL);
    epd_write_data(handle, (EPD_HEIGHT - 1) & 0xFF);
    epd_write_data(handle, ((EPD_HEIGHT - 1) >> 8) & 0xFF);
    epd_write_data(handle, 0x00);
    
    // 设置 Data Entry Mode
    epd_write_cmd(handle, SSD1680_DATA_ENTRY_MODE);
    epd_write_data(handle, 0x03);
    
    // 设置 RAM X 地址范围
    epd_write_cmd(handle, SSD1680_SET_RAM_X_ADDRESS_RANGE);
    epd_write_data(handle, 0x00);
    epd_write_data(handle, (EPD_WIDTH / 8) - 1);
    
    // 设置 RAM Y 地址范围
    epd_write_cmd(handle, SSD1680_SET_RAM_Y_ADDRESS_RANGE);
    epd_write_data(handle, 0x00);
    epd_write_data(handle, 0x00);
    epd_write_data(handle, (EPD_HEIGHT - 1) & 0xFF);
    epd_write_data(handle, ((EPD_HEIGHT - 1) >> 8) & 0xFF);
    
    // 设置 Display Update Control
    epd_write_cmd(handle, SSD1680_DISPLAY_UPDATE_CTRL_1);
    epd_write_data(handle, 0x00);
    epd_write_data(handle, 0x80);
    
    // 设置温度传感器
    epd_write_cmd(handle, SSD1680_TEMP_SENSOR_CONTROL);
    epd_write_data(handle, 0x80);
    
    // 设置 RAM X 地址计数器
    epd_write_cmd(handle, SSD1680_SET_RAM_X_ADDRESS_COUNTER);
    epd_write_data(handle, 0x00);
    
    // 设置 RAM Y 地址计数器
    epd_write_cmd(handle, SSD1680_SET_RAM_Y_ADDRESS_COUNTER);
    epd_write_data(handle, 0x00);
    epd_write_data(handle, 0x00);
    
    epd_wait_busy(handle);
    
    handle->initialized = true;
    
    // 保存全局句柄（供 epd_get_handle() 使用）
    g_epd_legacy = handle;
    
    ESP_LOGI(TAG, "SSD1680 初始化完成");
    return handle;
}

/**
 * @brief 反初始化设备
 */
esp_err_t epd_deinit(epd_handle_t *handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "反初始化设备");
    
    // 释放显存
    epd_free_buffers(handle);
    
    // 移除 SPI 设备
    if (handle->spi_handle != NULL) {
        spi_bus_remove_device(handle->spi_handle);
        handle->spi_handle = NULL;
    }
    
    // 释放 SPI 总线
    spi_bus_free(handle->spi_host);
    
    // 释放设备句柄
    free(handle);
    
    ESP_LOGI(TAG, "设备已反初始化");
    return ESP_OK;
}

/**
 * @brief 刷新显示
 */
esp_err_t epd_update(epd_handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint32_t start_time = esp_log_timestamp();
    
    ESP_LOGI(TAG, "=== 开始全屏刷新 ===");
    ESP_LOGI(TAG, "刷新模式：标准全刷");
    ESP_LOGI(TAG, "预计耗时：约 30 秒");
    
    // 1. 发送 Display Update Control 2
    epd_write_cmd(handle, SSD1680_DISPLAY_UPDATE_CTRL_2);
    epd_write_data(handle, 0xF4);  // 0xF4 = 全刷模式（30 秒）
    
    // 2. 发送 Master Activation 启动刷新
    epd_write_cmd(handle, SSD1680_MASTER_ACTIVATION);
    
    ESP_LOGI(TAG, "刷新命令已发送，等待完成...");
    
    // 3. 等待刷新完成
    epd_wait_busy(handle);
    
    uint32_t elapsed = esp_log_timestamp() - start_time;
    ESP_LOGI(TAG, "=== 全屏刷新完成 ===");
    ESP_LOGI(TAG, "实际耗时：%lu.%lu 秒", elapsed / 1000, (elapsed % 1000) / 100);
    
    return ESP_OK;
}

/**
 * @brief 写入显存数据并刷新
 */
esp_err_t epd_display(epd_handle_t *handle, 
                      const uint8_t *buffer_bw, 
                      const uint8_t *buffer_red)
{
    if (handle == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (buffer_bw == NULL || buffer_red == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "=== 开始写入显存数据 ===");
    
    // 重要：重新设置 RAM 地址计数器
    epd_write_cmd(handle, SSD1680_SET_RAM_X_ADDRESS_COUNTER);
    epd_write_data(handle, 0x00);
    
    epd_write_cmd(handle, SSD1680_SET_RAM_Y_ADDRESS_COUNTER);
    epd_write_data(handle, 0x00);
    epd_write_data(handle, 0x00);
    
    // 写入黑白显存
    ESP_LOGI(TAG, "写入黑白显存：%d 字节", EPD_BUFFER_SIZE);
    epd_write_cmd(handle, SSD1680_WRITE_RAM_BW);
    epd_write_data_batch(handle, buffer_bw, EPD_BUFFER_SIZE);
    
    // 重新设置 RAM 地址计数器
    epd_write_cmd(handle, SSD1680_SET_RAM_X_ADDRESS_COUNTER);
    epd_write_data(handle, 0x00);
    
    epd_write_cmd(handle, SSD1680_SET_RAM_Y_ADDRESS_COUNTER);
    epd_write_data(handle, 0x00);
    epd_write_data(handle, 0x00);
    
    // 写入红色显存（取反）
    ESP_LOGI(TAG, "写入红色显存：%d 字节（取反）", EPD_BUFFER_SIZE);
    
    // 创建取反后的缓冲区
    uint8_t *inverted_red = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    if (inverted_red != NULL) {
        for (int i = 0; i < EPD_BUFFER_SIZE; i++) {
            inverted_red[i] = ~buffer_red[i];
        }
        epd_write_cmd(handle, SSD1680_WRITE_RAM_RED);
        epd_write_data_batch(handle, inverted_red, EPD_BUFFER_SIZE);
        free(inverted_red);
    } else {
        ESP_LOGW(TAG, "内存分配失败，使用原数据");
        epd_write_cmd(handle, SSD1680_WRITE_RAM_RED);
        epd_write_data_batch(handle, buffer_red, EPD_BUFFER_SIZE);
    }
    
    ESP_LOGI(TAG, "显存数据写入完成");
    
    // 刷新显示
    return epd_update(handle);
}

/**
 * @brief 清屏
 */
esp_err_t epd_clear(epd_handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "=== 开始清屏 ===");
    ESP_LOGI(TAG, "清屏颜色：白色 (0xFF)");
    
    // 清屏时设置 Border 波形为白色
    epd_write_cmd(handle, SSD1680_BORDER_WAVEFORM_CTRL);
    epd_write_data(handle, 0x01);
    
    // 填充白色缓冲区
    uint8_t *white_buffer = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    if (white_buffer == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return ESP_ERR_NO_MEM;
    }
    memset(white_buffer, 0xFF, EPD_BUFFER_SIZE);
    
    // 写入显存并刷新
    esp_err_t ret = epd_display(handle, white_buffer, white_buffer);
    
    // 恢复 Border 波形配置
    epd_write_cmd(handle, SSD1680_BORDER_WAVEFORM_CTRL);
    epd_write_data(handle, 0x05);
    
    free(white_buffer);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "=== 清屏完成 ===");
    }
    
    return ret;
}

/**
 * @brief 进入深度睡眠
 */
esp_err_t epd_deep_sleep(epd_handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    epd_write_cmd(handle, SSD1680_DEEP_SLEEP_MODE);
    epd_write_data(handle, 0x01);
    
    vTaskDelay(pdMS_TO_TICKS(200));
    
    ESP_LOGI(TAG, "已进入深度睡眠模式");
    return ESP_OK;
}

/**
 * @brief 唤醒设备
 */
esp_err_t epd_wake_up(epd_handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "唤醒设备");
    
    // 硬件复位
    epd_hw_reset(handle);
    
    // 重新初始化（与 epd_init 相同）
    // 此处省略，可以调用 epd_init 的配置部分
    
    return ESP_OK;
}

// ==================== 兼容层实现 ====================

esp_err_t epd_init_legacy(void)
{
    if (g_epd_legacy != NULL) {
        ESP_LOGW(TAG, "设备已初始化");
        return ESP_OK;
    }
    
    epd_config_t config = epd_get_default_config();
    g_epd_legacy = epd_init(&config);
    
    return (g_epd_legacy != NULL) ? ESP_OK : ESP_FAIL;
}

void epd_clear_legacy(void)
{
    if (g_epd_legacy != NULL) {
        epd_clear(g_epd_legacy);
    }
}

void epd_update_legacy(void)
{
    if (g_epd_legacy != NULL) {
        epd_update(g_epd_legacy);
    }
}

void epd_display_legacy(const uint8_t *buffer_bw, const uint8_t *buffer_red)
{
    if (g_epd_legacy != NULL) {
        epd_display(g_epd_legacy, buffer_bw, buffer_red);
    }
}

void epd_deep_sleep_legacy(void)
{
    if (g_epd_legacy != NULL) {
        epd_deep_sleep(g_epd_legacy);
    }
}
