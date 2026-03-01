# SSD1680 调试指南

## 问题现象（已解决）

```
W (501) SSD1680: 等待 BUSY 超时 (timeout=20001ms, 最终 BUSY=1)
I (501) SSD1680: BUSY 等待时间：20001 ms
```

屏幕显示花屏，只有部分区域显示内容，且刷新未完成。

---

## 根本原因

### **BUSY 引脚未连接**

**实测证据：**
1. ✅ 屏幕有显示 - SPI 通信正常
2. ❌ BUSY 始终为 1 - 引脚悬空（上拉电阻导致）
3. ❌ 刷新未完成 - 无法检测 BUSY 下降沿

**结论：** 你的 SSD1680 模块的 BUSY 引脚**实际未连接**到 ESP32-S3。

---

## 最终解决方案

### 方案：固定延时策略（已实施）

不依赖 BUSY 引脚，改用固定延时确保命令执行完成。

**修改位置：** `ssd1680.c`

```c
// 修改前：依赖 BUSY 引脚
void epd_wait_busy(void)
{
    while (gpio_get_level(g_epd.pin_busy) == 1) {
        // 等待 BUSY 变低
    }
}

// 修改后：固定延时
void epd_wait_busy(void)
{
    const int min_wait = 50;  // 50ms 确保命令被接收
    vTaskDelay(pdMS_TO_TICKS(min_wait));
}

// epd_update() 中使用固定 3 秒延时
void epd_update(void)
{
    // ... 发送刷新命令 ...
    
    vTaskDelay(pdMS_TO_TICKS(50));  // 确保命令被接收
    vTaskDelay(pdMS_TO_TICKS(3000)); // 等待刷新完成（物理过程）
}
```

---

## 测试方法

### 测试 1：单像素测试

在 `main.c` 中创建简单测试图案：

```c
// 创建全白背景
memset(test_buffer_bw, 0xFF, EPD_BUFFER_SIZE);

// 在左上角画一个黑点 (x=0, y=0)
test_buffer_bw[0] = 0x7F;  // 第一个字节的最高位为 0（黑色）

// 显示
epd_display(test_buffer_bw, test_buffer_red);
```

**预期结果：** 屏幕左上角有一个黑点，其余全白。

### 测试 2：网格测试

```c
// 创建网格图案
for (int i = 0; i < EPD_BUFFER_SIZE; i++) {
    // 每隔 8 个像素画一条黑线
    test_buffer_bw[i] = (i % 2 == 0) ? 0x00 : 0xFF;
}
```

**预期结果：** 屏幕上显示水平黑白条纹。

---

## 常见问题

### Q1: 一直显示 BUSY 超时，但屏幕有显示

**原因：** BUSY 引脚极性错误或 BUSY 引脚未连接

**解决：**
1. 检查 BUSY 引脚是否正确连接到 GPIO5
2. 尝试修改 `epd_wait_busy()` 中的等待条件
3. 如果确定 BUSY 未连接，可以注释掉等待逻辑（不推荐）

### Q2: 屏幕显示花屏/乱码

**原因：**
1. RAM 地址计数器未正确设置
2. SPI 时序错误
3. 数据格式不匹配

**解决：**
1. 确保每次写入显存前都设置 RAM 地址计数器
2. 检查 SPI 模式是否为 Mode 0 (CPOL=0, CPHA=0)
3. 确认屏幕分辨率配置正确（296x152）

### Q3: 屏幕刷新很慢

**原因：** 墨水屏物理特性，正常现象

**说明：** 
- 全刷（Full Refresh）：2-4 秒
- 局刷（Partial Refresh）：0.5-1 秒
- 当前代码使用全刷模式

---

## 下一步

1. **重新编译烧录**
   ```bash
   cd TEST\SSD1680
   idf.py build
   idf.py -p <COM 端口> flash monitor
   ```

2. **观察日志**
   - 查看 `BUSY 初始状态` 的值
   - 查看超时时 `最终 BUSY` 的值
   - 如果一直超时，尝试修改 BUSY 等待逻辑

3. **拍照反馈**
   - 拍摄屏幕实际显示效果
   - 提供完整的串口日志
   - 说明硬件连接情况（特别是 BUSY 引脚）

---

## 联系支持

如果问题仍未解决，请提供：
1. 完整的串口日志
2. 屏幕显示效果照片
3. 硬件连接图（特别是 BUSY、RST、DC 引脚）
4. 是否使用逻辑分析仪/示波器观察波形
