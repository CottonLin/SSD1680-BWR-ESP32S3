#include "ssd1680.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include <string.h>

static const char *TAG = "SSD1680";

// 全局设备句柄（简化设计，单实例）
static epd_handle_t g_epd = {
    .spi_host = SPI2_HOST,  // VSPI_HOST 在 ESP32-S3 上对应 SPI2_HOST
    .spi_handle = NULL,
    .pin_rst = EPD_PIN_RST,
    .pin_dc = EPD_PIN_DC,
    .pin_cs = EPD_PIN_CS,
    .pin_busy = EPD_PIN_BUSY,
    .buffer_bw = NULL,
    .buffer_red = NULL
};

/**
 * @brief 初始化 GPIO 引脚
 */
static void epd_gpio_init(void)
{
    // 配置 RST 引脚 (输出)
    gpio_reset_pin(g_epd.pin_rst);
    gpio_set_direction(g_epd.pin_rst, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(g_epd.pin_rst, GPIO_PULLUP_ONLY);
    
    // 配置 DC 引脚 (输出)
    gpio_reset_pin(g_epd.pin_dc);
    gpio_set_direction(g_epd.pin_dc, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(g_epd.pin_dc, GPIO_PULLUP_ONLY);
    
    // 配置 CS 引脚 (输出)
    gpio_reset_pin(g_epd.pin_cs);
    gpio_set_direction(g_epd.pin_cs, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(g_epd.pin_cs, GPIO_PULLUP_ONLY);
    
    // 配置 BUSY 引脚 (输入)
    gpio_reset_pin(g_epd.pin_busy);
    gpio_set_direction(g_epd.pin_busy, GPIO_MODE_INPUT);
    gpio_set_pull_mode(g_epd.pin_busy, GPIO_PULLUP_ONLY);
    
    // 初始状态：CS 高电平（禁用），DC 高电平（数据模式）
    gpio_set_level(g_epd.pin_cs, 1);
    gpio_set_level(g_epd.pin_dc, 1);
    gpio_set_level(g_epd.pin_rst, 1);
}

/**
 * @brief 初始化 SPI 总线
 */
static esp_err_t epd_spi_init(void)
{
    esp_err_t ret;
    
    // SPI 总线配置
    spi_bus_config_t buscfg = {
        .mosi_io_num = EPD_SPI_MOSI_PIN,
        .miso_io_num = -1,  // SSD1680 不需要 MISO
        .sclk_io_num = EPD_SPI_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = EPD_BUFFER_SIZE * 2
    };
    
    // SPI 设备配置
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000,  // 8MHz (SSD1680 支持最高 10MHz)
        .mode = 0,  // SPI Mode 0 (CPOL=0, CPHA=0)
        .spics_io_num = g_epd.pin_cs,
        .queue_size = 1,
        .flags = SPI_DEVICE_NO_DUMMY
    };
    
    // 初始化 SPI 总线
    ret = spi_bus_initialize(g_epd.spi_host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI 总线初始化失败");
        return ret;
    }
    
    // 添加 SPI 设备
    ret = spi_bus_add_device(g_epd.spi_host, &devcfg, &g_epd.spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI 设备添加失败");
        spi_bus_free(g_epd.spi_host);
        return ret;
    }
    
    ESP_LOGI(TAG, "SPI 初始化完成");
    return ESP_OK;
}

/**
 * @brief 等待忙信号结束（BUSY 引脚检测）
 * 
 * 重要说明：
 * 1. SSD1680 的 BUSY 引脚逻辑：
 *    - BUSY=1 (高电平)：SSD1680 正在处理命令/刷新
 *    - BUSY=0 (低电平)：SSD1680 空闲，可以接收新命令
 * 
 * 2. 检测策略：
 *    - 循环检测 BUSY 引脚电平
 *    - BUSY 为低电平时立即返回
 *    - 不考虑超时问题（由调用方负责）
 * 
 * 3. 看门狗处理：
 *    - 不喂狗，不禁用
 *    - 通过配置看门狗超时时间解决（在 menuconfig 中设置为 60 秒）
 */
void epd_wait_busy(void)
{
    // 检测 BUSY 引脚，直到低电平
    while (gpio_get_level(g_epd.pin_busy) == 1) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/**
 * @brief 硬件复位
 */
void epd_hw_reset(void)
{
    // 复位序列：拉低 RST -> 延时 -> 拉高 RST -> 等待 BUSY 结束
    gpio_set_level(g_epd.pin_rst, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(g_epd.pin_rst, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    epd_wait_busy();
    
    ESP_LOGI(TAG, "硬件复位完成");
}

/**
 * @brief 写入命令
 */
void epd_write_cmd(uint8_t cmd)
{
    esp_err_t ret;
    
    // DC=0 表示命令
    gpio_set_level(g_epd.pin_dc, 0);
    
    // 发送命令字节
    uint8_t data = cmd;
    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &data
    };
    
    ret = spi_device_transmit(g_epd.spi_handle, &transaction);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI 命令发送失败");
    }
    
    // 恢复 DC=1 (数据模式)
    gpio_set_level(g_epd.pin_dc, 1);
}

/**
 * @brief 写入数据
 */
void epd_write_data(uint8_t data)
{
    esp_err_t ret;
    
    // DC=1 表示数据 (已在 epd_write_cmd 后设置为 1)
    gpio_set_level(g_epd.pin_dc, 1);
    
    // 发送数据字节
    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &data
    };
    
    ret = spi_device_transmit(g_epd.spi_handle, &transaction);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI 数据发送失败");
    }
}

/**
 * @brief 写入多个数据
 */
static void epd_write_data_batch(const uint8_t *data, size_t len)
{
    esp_err_t ret;
    
    gpio_set_level(g_epd.pin_dc, 1);
    
    spi_transaction_t transaction = {
        .length = len * 8,
        .tx_buffer = data
    };
    
    ret = spi_device_transmit(g_epd.spi_handle, &transaction);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI 批量数据发送失败");
    }
}

/**
 * @brief 初始化 SSD1680 显示屏
 */
esp_err_t epd_init(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "初始化 SSD1680 显示屏");
    
    // 1. 初始化 GPIO
    epd_gpio_init();
    
    // 2. 初始化 SPI 总线
    ret = epd_spi_init();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 3. 硬件复位
    epd_hw_reset();
    
    // 4. 软件复位
    epd_write_cmd(SSD1680_SW_RESET);
    epd_wait_busy();  // 等待复位完成
    
    // 5. 配置显示屏参数
    // 设置 Border 波形
    epd_write_cmd(SSD1680_BORDER_WAVEFORM_CTRL);
    epd_write_data(0x05);
    
    // 设置 Driver Output (295 行 - 1)
    // 注意：EPD_HEIGHT 现在是 296（SSD1680 内部 Y 方向高度）
    epd_write_cmd(SSD1680_DRIVER_OUTPUT_CTRL);
    epd_write_data((EPD_HEIGHT - 1) & 0xFF);
    epd_write_data(((EPD_HEIGHT - 1) >> 8) & 0xFF);
    epd_write_data(0x00);
    
    // 设置 Data Entry Mode
    // 0x03: X 地址递增，Y 地址递增（从左到右，从上到下）
    epd_write_cmd(SSD1680_DATA_ENTRY_MODE);
    epd_write_data(0x03);  // 使用 0x03 模式（X 递增）
    
    // 设置 RAM X 地址范围 (0 到 151/8 = 18)
    // 注意：EPD_WIDTH 现在是 152（SSD1680 内部 X 方向宽度）
    epd_write_cmd(SSD1680_SET_RAM_X_ADDRESS_RANGE);
    epd_write_data(0x00);
    epd_write_data((EPD_WIDTH / 8) - 1);
    
    // 设置 RAM Y 地址范围 (0 到 295)
    epd_write_cmd(SSD1680_SET_RAM_Y_ADDRESS_RANGE);
    epd_write_data(0x00);
    epd_write_data(0x00);
    epd_write_data((EPD_HEIGHT - 1) & 0xFF);
    epd_write_data(((EPD_HEIGHT - 1) >> 8) & 0xFF);
    
    // 设置 Display Update Control
    epd_write_cmd(SSD1680_DISPLAY_UPDATE_CTRL_1);
    epd_write_data(0x00);
    epd_write_data(0x80);
    
    // 设置温度传感器
    epd_write_cmd(SSD1680_TEMP_SENSOR_CONTROL);
    epd_write_data(0x80);  // 使用内部温度传感器
    
    // 设置 RAM X 地址计数器
    // 0x03 模式：从起始位置 (0x00) 开始
    epd_write_cmd(SSD1680_SET_RAM_X_ADDRESS_COUNTER);
    epd_write_data(0x00);  // 起始位置
    
    // 设置 RAM Y 地址计数器
    epd_write_cmd(SSD1680_SET_RAM_Y_ADDRESS_COUNTER);
    epd_write_data(0x00);
    epd_write_data(0x00);
    
    epd_wait_busy();
    
    ESP_LOGI(TAG, "SSD1680 初始化完成");
    return ESP_OK;
}

/**
 * @brief 刷新显示
 * 
 * 重要发现：
 * 1. SSD1680 在写入显存数据时就已经开始刷新（不是等 MASTER_ACTIVATION 后才开始）
 * 2. 0xF4 和 0xF7 波形的实际刷新时间相同（都是 30 秒全刷）
 * 3. 所谓"快速刷新"是因为显存写入和刷新并行进行
 * 
 * 刷新过程：
 * 1. 写入显存数据（约 100ms）-> 屏幕同时开始刷新
 * 2. 发送刷新命令（确保刷新被正确触发）
 * 3. 等待刷新完成（30 秒物理刷新时间）
 */
void epd_update(void)
{
    uint32_t start_time = esp_log_timestamp();
    
    ESP_LOGI(TAG, "=== 开始全屏刷新 ===");
    ESP_LOGI(TAG, "刷新模式：标准全刷（0xF4）");
    ESP_LOGI(TAG, "预计耗时：约 30 秒");
    
    // 1. 发送 Display Update Control 2
    epd_write_cmd(SSD1680_DISPLAY_UPDATE_CTRL_2);
    epd_write_data(0xF4);  // 0xF4 = 全刷模式（30 秒）
                           // 注意：0xF7 也是 30 秒，无快速效果
    
    // 2. 发送 Master Activation 启动刷新
    epd_write_cmd(SSD1680_MASTER_ACTIVATION);
    
    ESP_LOGI(TAG, "刷新命令已发送，等待完成...");
    
    // 3. 等待刷新完成（使用 BUSY 引脚检测）
    // SSD1680 全刷模式需要 30 秒（物理特性，无法加速）
    epd_wait_busy();
    
    uint32_t elapsed = esp_log_timestamp() - start_time;
    ESP_LOGI(TAG, "=== 全屏刷新完成 ===");
    ESP_LOGI(TAG, "实际耗时：%lu.%lu 秒", elapsed / 1000, (elapsed % 1000) / 100);
}

/**
 * @brief 写入显存数据并刷新
 * 
 * 优化策略：
 * 1. SSD1680 在写入显存时就开始刷新（并行进行）
 * 2. 写入显存后立即调用 epd_update() 发送刷新命令
 * 3. 总耗时 = 显存写入时间 (100ms) + 刷新等待 (30 秒)
 */
void epd_display(const uint8_t *buffer_bw, const uint8_t *buffer_red)
{
    ESP_LOGI(TAG, "=== 开始写入显存数据 ===");
    
    // 重要：在写入显存前，必须重新设置 RAM 地址计数器
    // 0x03 模式：从起始位置 (0x00) 开始
    epd_write_cmd(SSD1680_SET_RAM_X_ADDRESS_COUNTER);
    epd_write_data(0x00);  // 起始位置
    
    // 设置 RAM Y 地址计数器
    epd_write_cmd(SSD1680_SET_RAM_Y_ADDRESS_COUNTER);
    epd_write_data(0x00);
    epd_write_data(0x00);
    
    // 写入黑白显存 (0x24)
    ESP_LOGI(TAG, "写入黑白显存：%d 字节", EPD_BUFFER_SIZE);
    epd_write_cmd(SSD1680_WRITE_RAM_BW);
    epd_write_data_batch(buffer_bw, EPD_BUFFER_SIZE);
    
    // 重新设置 RAM 地址计数器（写入红色显存前也需要）
    epd_write_cmd(SSD1680_SET_RAM_X_ADDRESS_COUNTER);
    epd_write_data(0x00);  // 起始位置
    
    epd_write_cmd(SSD1680_SET_RAM_Y_ADDRESS_COUNTER);
    epd_write_data(0x00);
    epd_write_data(0x00);
    
    // 写入红色显存 (0x26)
    // 注意：根据 STM32 原代码，红色显存数据需要取反
    ESP_LOGI(TAG, "写入红色显存：%d 字节（取反）", EPD_BUFFER_SIZE);
    
    // 创建取反后的缓冲区
    uint8_t *inverted_red = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    if (inverted_red != NULL) {
        for (int i = 0; i < EPD_BUFFER_SIZE; i++) {
            inverted_red[i] = ~buffer_red[i];
        }
        epd_write_cmd(SSD1680_WRITE_RAM_RED);
        epd_write_data_batch(inverted_red, EPD_BUFFER_SIZE);
        free(inverted_red);
    } else {
        // 如果内存分配失败，直接写入原数据
        ESP_LOGW(TAG, "内存分配失败，使用原数据");
        epd_write_cmd(SSD1680_WRITE_RAM_RED);
        epd_write_data_batch(buffer_red, EPD_BUFFER_SIZE);
    }
    
    ESP_LOGI(TAG, "显存数据写入完成");
    
    // 刷新显示
    // 注意：此时 SSD1680 已经在后台开始刷新了
    // epd_update() 发送刷新命令确保刷新流程正确完成
    epd_update();
}

/**
 * @brief 清屏 (填充白色)
 */
void epd_clear(void)
{
    uint8_t *white_buffer = (uint8_t *)malloc(EPD_BUFFER_SIZE);
    if (white_buffer == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return;
    }
    
    // 填充白色 (0xFF)
    memset(white_buffer, 0xFF, EPD_BUFFER_SIZE);
    
    ESP_LOGI(TAG, "=== 开始清屏 ===");
    ESP_LOGI(TAG, "清屏颜色：白色 (0xFF)");
    ESP_LOGI(TAG, "数据大小：%d 字节", EPD_BUFFER_SIZE);
    
    // 清屏时需要设置 Border 波形为白色
    epd_write_cmd(SSD1680_BORDER_WAVEFORM_CTRL);
    epd_write_data(0x01);  // 0x01 = 白色
    
    // 写入显存并刷新
    epd_display(white_buffer, white_buffer);
    
    // 恢复 Border 波形配置
    epd_write_cmd(SSD1680_BORDER_WAVEFORM_CTRL);
    epd_write_data(0x05);  // 0x05 = 默认波形
    
    free(white_buffer);
    
    ESP_LOGI(TAG, "=== 清屏完成 ===");
}

/**
 * @brief 进入深度睡眠
 */
void epd_deep_sleep(void)
{
    epd_write_cmd(SSD1680_DEEP_SLEEP_MODE);
    epd_write_data(0x01);  // 进入 Deep Sleep 模式
    
    vTaskDelay(pdMS_TO_TICKS(200));
    
    ESP_LOGI(TAG, "已进入深度睡眠模式");
}


