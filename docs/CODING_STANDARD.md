# SSD1680 项目重构规范与接口定义

**版本**: 1.0  
**日期**: 2026-03-02  
**状态**: 执行中

---

## 一、编码规范

### 1.1 命名规范

#### 文件命名
- 头文件：`.h` 扩展名，小写字母，下划线分隔
- 源文件：`.c` 扩展名，小写字母，下划线分隔
- 组件头文件：`<组件名>.h`
- 组件源文件：`<组件名>.c`

#### 函数命名
- 格式：`<模块前缀>_<功能描述>`
- 示例：
  - `epd_init()` - 初始化设备
  - `epd_canvas_create()` - 创建画布
  - `epd_draw_line()` - 绘制直线

#### 变量命名
- 全局变量：`g_<变量名>`
- 局部变量：小写字母，下划线分隔
- 常量：全大写字母，下划线分隔
- 指针变量：`<类型>_t *<变量名>`

#### 类型定义
- 结构体：`<模块名>_<类型名>_t`
- 枚举：`<模块名>_<枚举名>_t`
- 示例：
  ```c
  typedef struct {
      uint8_t *buffer_bw;
      uint8_t *buffer_red;
  } epd_canvas_t;
  ```

### 1.2 注释规范

#### 文件头注释
```c
/**
 * @file epd_driver.h
 * @brief SSD1680 硬件驱动头文件
 * 
 * @version 1.0
 * @date 2026-03-02
 */
```

#### 函数注释
```c
/**
 * @brief 初始化 SSD1680 显示屏
 * 
 * @param config 设备配置指针，NULL 使用默认配置
 * @return epd_handle_t* 设备句柄，NULL 表示失败
 * 
 * @note 此函数会初始化 SPI、GPIO 和显存
 * @attention 首次调用前请确保电源已稳定
 */
esp_err_t epd_init(const epd_config_t *config);
```

#### 结构体注释
```c
/**
 * @brief SSD1680 设备配置结构
 */
typedef struct {
    gpio_num_t pin_rst;     /*!< 复位引脚 */
    gpio_num_t pin_dc;      /*!< 数据/命令选择引脚 */
    gpio_num_t pin_cs;      /*!< 片选引脚 */
    gpio_num_t pin_busy;    /*!< 忙信号引脚 */
    uint32_t spi_freq_hz;   /*!< SPI 频率 (Hz) */
} epd_config_t;
```

### 1.3 代码格式规范

#### 缩进和对齐
- 使用 4 个空格缩进（不使用 Tab）
- 操作符两侧留空格
- 控制语句括号前留空格

```c
// 正确示例
if (condition) {
    do_something();
}

// 错误示例
if(condition){
    do_something();
}
```

#### 行长度
- 单行代码不超过 100 字符
- 超长字符串需要分行

```c
const char *long_string = "This is a very long string that exceeds "
                          "the 100 character limit and needs to be "
                          "split into multiple lines.";
```

#### 空行使用
- 函数之间空一行
- 逻辑块之间空一行
- 函数内部避免连续空行

### 1.4 错误处理规范

#### 返回值约定
- 成功：`ESP_OK` (0)
- 失败：对应的 `esp_err_t` 错误码

```c
esp_err_t epd_init(void)
{
    esp_err_t ret;
    
    ret = some_operation();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Operation failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}
```

#### 参数验证
- 所有公共函数必须验证输入参数
- 指针参数必须检查 NULL
- 范围参数必须检查边界

```c
if (canvas == NULL || buffer == NULL) {
    ESP_LOGE(TAG, "Invalid parameter: NULL pointer");
    return ESP_ERR_INVALID_ARG;
}

if (width > EPD_MAX_WIDTH) {
    ESP_LOGE(TAG, "Width %d exceeds maximum %d", width, EPD_MAX_WIDTH);
    return ESP_ERR_INVALID_SIZE;
}
```

#### 错误日志
- 错误级别：`ESP_LOGE()`
- 警告级别：`ESP_LOGW()`
- 信息级别：`ESP_LOGI()`
- 调试级别：`ESP_LOGD()`

---

## 二、接口定义

### 2.1 硬件驱动层接口 (epd_driver.h)

#### 数据结构定义

```c
/**
 * @brief SSD1680 设备配置结构
 */
typedef struct {
    gpio_num_t pin_rst;     /*!< 复位引脚，默认 GPIO_NUM_4 */
    gpio_num_t pin_dc;      /*!< 数据/命令选择引脚，默认 GPIO_NUM_6 */
    gpio_num_t pin_cs;      /*!< 片选引脚，默认 GPIO_NUM_21 */
    gpio_num_t pin_busy;    /*!< 忙信号引脚，默认 GPIO_NUM_5 */
    gpio_num_t pin_mosi;    /*!< MOSI 引脚，默认 GPIO_NUM_11 */
    gpio_num_t pin_sclk;    /*!< SCLK 引脚，默认 GPIO_NUM_12 */
    uint32_t spi_freq_hz;   /*!< SPI 频率 (Hz)，默认 8000000 */
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
    uint8_t *buffer_bw;               /*!< 黑白显存指针 */
    uint8_t *buffer_red;              /*!< 红色显存指针 */
    bool initialized;                 /*!< 初始化标志 */
} epd_handle_t;
```

#### 核心接口

```c
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
```

#### 底层操作接口（内部使用）

```c
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
```

### 2.2 GUI 层接口 (epd_gui.h)

#### 数据结构定义

```c
/**
 * @brief 画布对象结构
 */
typedef struct {
    uint8_t *buffer_bw;         /*!< 黑白显存缓冲区 */
    uint8_t *buffer_red;        /*!< 红色显存缓冲区 */
    uint16_t width;             /*!< 逻辑宽度（用户坐标） */
    uint16_t height;            /*!< 逻辑高度（用户坐标） */
    uint16_t width_bytes;       /*!< 每行字节数 */
    uint16_t color;             /*!< 当前绘图颜色 */
} epd_canvas_t;

/**
 * @brief 颜色定义
 */
#define EPD_COLOR_WHITE     0xFF    /*!< 白色 */
#define EPD_COLOR_BLACK     0x00    /*!< 黑色 */
#define EPD_COLOR_RED       0x01    /*!< 红色 */
```

#### 画布管理接口

```c
/**
 * @brief 创建画布对象
 * 
 * @param buffer_bw 黑白显存缓冲区（用户分配，大小 EPD_BUFFER_SIZE）
 * @param buffer_red 红色显存缓冲区（用户分配，可为 NULL）
 * @param width 逻辑宽度（用户坐标，通常 296）
 * @param height 逻辑高度（用户坐标，通常 152）
 * @return epd_canvas_t* 画布对象指针，NULL 表示失败
 */
epd_canvas_t* epd_canvas_create(uint8_t *buffer_bw, 
                                uint8_t *buffer_red, 
                                uint16_t width, 
                                uint16_t height);

/**
 * @brief 销毁画布对象
 * @param canvas 画布对象指针
 */
void epd_canvas_destroy(epd_canvas_t *canvas);

/**
 * @brief 清空画布
 * @param canvas 画布对象指针
 * @param color 填充颜色
 */
void epd_canvas_clear(epd_canvas_t *canvas, uint16_t color);
```

#### 绘图接口

```c
/**
 * @brief 设置像素点
 * 
 * @param canvas 画布对象指针
 * @param x X 坐标（用户坐标）
 * @param y Y 坐标（用户坐标）
 * @param color 颜色
 */
void epd_canvas_set_pixel(epd_canvas_t *canvas, 
                          uint16_t x, uint16_t y, 
                          uint16_t color);

/**
 * @brief 绘制直线
 * 
 * @param canvas 画布对象指针
 * @param x1 起点 X 坐标
 * @param y1 起点 Y 坐标
 * @param x2 终点 X 坐标
 * @param y2 终点 Y 坐标
 * @param color 颜色
 */
void epd_draw_line(epd_canvas_t *canvas, 
                   uint16_t x1, uint16_t y1, 
                   uint16_t x2, uint16_t y2, 
                   uint16_t color);

/**
 * @brief 绘制矩形
 * 
 * @param canvas 画布对象指针
 * @param x1 左上角 X 坐标
 * @param y1 左上角 Y 坐标
 * @param x2 右下角 X 坐标
 * @param y2 右下角 Y 坐标
 * @param color 颜色
 * @param filled 填充模式：0=空心，1=实心
 */
void epd_draw_rectangle(epd_canvas_t *canvas, 
                        uint16_t x1, uint16_t y1, 
                        uint16_t x2, uint16_t y2, 
                        uint16_t color, uint8_t filled);

/**
 * @brief 绘制圆形
 * 
 * @param canvas 画布对象指针
 * @param center_x 圆心 X 坐标
 * @param center_y 圆心 Y 坐标
 * @param radius 半径
 * @param color 颜色
 * @param filled 填充模式：0=空心，1=实心
 */
void epd_draw_circle(epd_canvas_t *canvas, 
                     uint16_t center_x, uint16_t center_y, 
                     uint16_t radius, uint16_t color, 
                     uint8_t filled);
```

#### 文本渲染接口

```c
/**
 * @brief 显示单个字符
 * 
 * @param canvas 画布对象指针
 * @param x X 坐标（左上角）
 * @param y Y 坐标（左上角）
 * @param chr 字符
 * @param font 字体对象指针
 * @param color 颜色
 */
void epd_show_char(epd_canvas_t *canvas, 
                   uint16_t x, uint16_t y, 
                   char chr, const epd_font_t *font, 
                   uint16_t color);

/**
 * @brief 显示字符串（不支持换行）
 * 
 * @param canvas 画布对象指针
 * @param x X 坐标（左上角）
 * @param y Y 坐标（左上角）
 * @param str 字符串
 * @param font 字体对象指针
 * @param color 颜色
 */
void epd_show_string(epd_canvas_t *canvas, 
                     uint16_t x, uint16_t y, 
                     const char *str, const epd_font_t *font, 
                     uint16_t color);

/**
 * @brief 显示字符串（支持智能换行）
 * 
 * @param canvas 画布对象指针
 * @param x X 坐标（左上角）
 * @param y Y 坐标（左上角）
 * @param str 字符串
 * @param font 字体对象指针
 * @param color 颜色
 * @param max_width 最大宽度（像素），0 表示使用画布宽度
 * @param max_height 最大高度（像素），0 表示使用画布高度
 * 
 * @note 按单词换行，不会切断单词
 * @note 支持 '\\n' 强制换行符
 */
void epd_show_string_wrap(epd_canvas_t *canvas, 
                          uint16_t x, uint16_t y, 
                          const char *str, const epd_font_t *font, 
                          uint16_t color,
                          uint16_t max_width, uint16_t max_height);

/**
 * @brief 显示数字
 * 
 * @param canvas 画布对象指针
 * @param x X 坐标（左上角）
 * @param y Y 坐标（左上角）
 * @param num 数字值
 * @param len 显示位数
 * @param font 字体对象指针
 * @param color 颜色
 */
void epd_show_num(epd_canvas_t *canvas, 
                  uint16_t x, uint16_t y, 
                  uint32_t num, uint8_t len, 
                  const epd_font_t *font, 
                  uint16_t color);
```

### 2.3 字体系统接口 (epd_font.h)

#### 数据结构定义

```c
/**
 * @brief 字体对象结构
 */
typedef struct {
    const void *data;       /*!< 字模数据指针 */
    uint8_t width;          /*!< 字体宽度（像素） */
    uint8_t height;         /*!< 字体高度（像素） */
    uint8_t first_char;     /*!< 第一个字符 ASCII 码 */
    uint8_t last_char;      /*!< 最后一个字符 ASCII 码 */
    uint16_t char_bytes;    /*!< 每个字符的字节数 */
} epd_font_t;

/**
 * @brief 字体大小枚举
 */
typedef enum {
    EPD_FONT_SIZE_6X8 = 0,   /*!< 6x8 字体 */
    EPD_FONT_SIZE_6X12,      /*!< 6x12 字体 */
    EPD_FONT_SIZE_8X16,      /*!< 8x16 字体 */
    EPD_FONT_SIZE_12X24      /*!< 12x24 字体 */
} epd_font_size_t;
```

#### 字体接口

```c
/**
 * @brief 获取字体对象
 * 
 * @param size 字体大小
 * @return const epd_font_t* 字体对象指针，NULL 表示不支持
 */
const epd_font_t* epd_font_get(epd_font_size_t size);

/**
 * @brief 获取字符宽度
 * 
 * @param font 字体对象指针
 * @return uint8_t 字符宽度（像素）
 */
uint8_t epd_font_get_width(const epd_font_t *font);

/**
 * @brief 获取字符高度
 * 
 * @param font 字体对象指针
 * @return uint8_t 字符高度（像素）
 */
uint8_t epd_font_get_height(const epd_font_t *font);

/**
 * @brief 计算字符串宽度
 * 
 * @param font 字体对象指针
 * @param str 字符串
 * @return uint16_t 字符串总宽度（像素）
 */
uint16_t epd_font_get_string_width(const epd_font_t *font, const char *str);
```

### 2.4 工具函数接口 (epd_utils.h)

```c
/**
 * @brief 内存设置辅助函数
 * 
 * @param buffer 缓冲区指针
 * @param value 设置值
 * @param size 缓冲区大小
 * @return esp_err_t
 */
esp_err_t epd_utils_memset(uint8_t *buffer, uint8_t value, size_t size);

/**
 * @brief 内存复制辅助函数
 * 
 * @param dest 目标缓冲区
 * @param src 源缓冲区
 * @param size 复制大小
 * @return esp_err_t
 */
esp_err_t epd_utils_memcpy(uint8_t *dest, const uint8_t *src, size_t size);

/**
 * @brief 字节反转（用于显存数据）
 * 
 * @param buffer 数据缓冲区
 * @param size 数据大小
 * @return esp_err_t
 */
esp_err_t epd_utils_reverse_bytes(uint8_t *buffer, size_t size);
```

---

## 三、兼容性说明

### 3.1 向下兼容

为保证现有代码无需修改即可使用新接口，提供以下兼容层：

```c
// 旧接口（保留，内部调用新接口）
esp_err_t epd_init(void);
void epd_clear(void);
void epd_update(void);
void epd_display(const uint8_t *buffer_bw, const uint8_t *buffer_red);

// 新接口（推荐使用）
epd_handle_t* epd_init(const epd_config_t *config);
esp_err_t epd_clear(epd_handle_t *handle);
esp_err_t epd_update(epd_handle_t *handle);
esp_err_t epd_display(epd_handle_t *handle, 
                      const uint8_t *buffer_bw, 
                      const uint8_t *buffer_red);
```

### 3.2 迁移指南

从旧接口迁移到新接口的示例：

```c
// 旧代码
void app_main(void)
{
    epd_init();
    epd_clear();
    // ...
    epd_update();
}

// 新代码
void app_main(void)
{
    epd_handle_t *epd = epd_init(NULL);
    if (epd == NULL) {
        ESP_LOGE(TAG, "Init failed");
        return;
    }
    
    epd_clear(epd);
    // ...
    epd_update(epd);
    
    epd_deinit(epd);
}
```

---

## 四、质量要求

### 4.1 代码质量指标

- **注释覆盖率**: ≥ 70%
- **单元测试覆盖率**: ≥ 85%
- **编译警告**: 0 个
- **静态分析**: 通过
- **内存泄漏**: 0 个

### 4.2 性能指标

- **SPI 传输效率**: 不低于原有水平
- **刷新时间**: 30 秒（硬件限制）
- **内存使用**: 不超过原有 120%
- **启动时间**: 不超过原有 150%

### 4.3 测试要求

- 所有公共接口必须有单元测试
- 边界条件必须测试
- 异常场景必须测试
- 长时间运行测试（24 小时）

---

## 五、修订历史

| 版本 | 日期 | 作者 | 变更说明 |
|-----|------|------|---------|
| 1.0 | 2026-03-02 | AI Assistant | 初始版本 |
