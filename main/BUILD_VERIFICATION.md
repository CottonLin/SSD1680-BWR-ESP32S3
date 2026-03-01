# GUI 模块编译与验证指南

## 📁 文件结构

已创建的文件列表：

```
SSD1680_V0.1/
└── main/
    ├── ssd1680.h              # SSD1680 驱动头文件 (已有)
    ├── ssd1680.c              # SSD1680 驱动实现 (已有)
    ├── epd_gui.h              # ✅ GUI 头文件 (新建)
    ├── epd_gui.c              # ✅ GUI 实现 (新建)
    ├── epd_font.h             # ✅ 字体头文件 (新建)
    ├── epd_font.c             # ✅ 字体实现 (新建)
    ├── epd_gui_example.c      # ✅ 使用示例 (新建，不编译)
    ├── main.c                 # 主程序 (已有)
    ├── CMakeLists.txt         # ✅ 已更新 (包含新文件)
    ├── FONT_IMPORT_GUIDE.md   # ✅ 字模导入指南 (新建)
    ├── GUI_MIGRATION_REPORT.md # ✅ 移植报告 (新建)
    └── test/
        ├── test_epd_gui.c     # ✅ 单元测试 (新建)
        └── CMakeLists.txt     # ✅ 测试配置 (新建)
```

## 🔧 编译步骤

### 方法 1: 使用 IDF 命令行

```powershell
# 1. 打开 ESP-IDF 命令行终端
# 开始菜单 → ESP-IDF 5.5 → ESP-IDF CMD

# 2. 进入项目目录
cd c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1

# 3. 清理之前的编译（可选）
idf.py fullclean

# 4. 编译项目
idf.py build

# 预期输出:
# - 无错误
# - 无警告
# - 生成 SSD1680.bin
```

### 方法 2: 使用 VS Code 扩展

1. 打开 VS Code
2. 打开项目文件夹：`c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1`
3. 点击底部状态栏的 ESP-IDF 图标
4. 点击 "Build" 按钮

### 方法 3: 使用 CMake

```powershell
cd c:\Projects\Espressif_Projects\ESP-Informer\TEST\SSD1680\SSD1680_V0.1

# 配置项目
cmake -B build -G Ninja ^
  -DCMAKE_TOOLCHAIN_FILE=%IDF_PATH%\tools\cmake\toolchain-esp32s3.cmake ^
  -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build build
```

## ⚠️ 编译注意事项

### 1. 字模数据未导入

**当前状态**：`epd_font.c` 中的字模数组是 `extern` 声明

```c
// epd_font.c 第 24 行
extern const uint8_t ascii_font_8x8[];  // ⚠️ 只有声明，没有数据
extern const uint8_t ascii_font_16x16[]; // ⚠️ 只有声明，没有数据
```

**编译结果**：
- ✅ **编译成功**：因为只是 `extern` 声明
- ❌ **链接失败**：因为找不到实际数组定义
- ❌ **运行时错误**：如果强制运行会崩溃

**解决方案**：

有两种选择：

#### 选项 A: 导入实际字模数据（推荐）

参考 `FONT_IMPORT_GUIDE.md`，从 STM32 项目复制字模数据。

#### 选项 B: 创建空数组（临时测试）

如果暂时只需要测试几何图形功能，可以创建空数组：

```c
// epd_font.c - 临时测试用空数组
const uint8_t ascii_font_8x8[760] = {0};     // 760 字节全 0
const uint8_t ascii_font_16x16[3040] = {0};  // 3040 字节全 0
```

**注意**：这样字符显示会是全黑块，但几何图形功能正常。

### 2. 示例文件不编译

`epd_gui_example.c` 是参考代码，**不应该**添加到 CMakeLists.txt 中。

**验证**：
```bash
# 检查 CMakeLists.txt
cat main/CMakeLists.txt

# 应该看到:
# idf_component_register(SRCS "main.c" "ssd1680.c" "epd_gui.c" "epd_font.c"
#                     INCLUDE_DIRS ".")
# 
# 不包含 epd_gui_example.c ✅
```

## 🧪 单元测试

### 运行单元测试

```powershell
# 1. 编译并运行测试
idf.py test

# 预期输出:
# test_epd_gui.c:PASSED
# test_epd_gui.c:PASSED
# ...
# ALL TESTS PASSED
```

### 运行特定测试

```powershell
# 只运行画布测试
idf.py test --filter "Canvas"

# 只运行直线测试
idf.py test --filter "Line"

# 只运行圆形测试
idf.py test --filter "Circle"
```

## 🚀 烧录与验证

### 烧录到开发板

```powershell
# 1. 连接开发板
# 2. 确认 COM 端口（设备管理器查看）

# 3. 烧录
idf.py -p COM3 flash

# 4. 查看串口输出
idf.py -p COM3 monitor
```

### 预期输出

如果编译和烧录成功，应该看到：

```
I (0) cpu_start: Starting scheduler on APP CPU.
I (0) SSD1680: 初始化 SSD1680 显示屏
I (0) SSD1680: SPI 初始化完成
I (0) SSD1680: 硬件复位完成
I (0) SSD1680: SSD1680 初始化完成
I (0) EPD_GUI: 画布创建成功：296x152 (内部：152x296)
...
```

## ✅ 验证清单

### 编译验证

- [ ] 无语法错误
- [ ] 无链接错误
- [ ] 生成 SSD1680.bin 文件
- [ ] 文件大小合理（约 100-200KB）

### 功能验证（几何图形）

即使字模未导入，以下几何图形功能也应该正常工作：

- [ ] 画布创建成功
- [ ] 清屏功能正常
- [ ] 可以绘制像素点
- [ ] 可以绘制直线
- [ ] 可以绘制矩形
- [ ] 可以绘制圆形
- [ ] 屏幕刷新正常

### 功能验证（字符显示）

字符显示需要导入字模数据后才能测试：

- [ ] 导入 8x8 字模
- [ ] 导入 16x16 字模
- [ ] 显示字符测试
- [ ] 显示字符串测试
- [ ] 显示数字测试

## 🐛 常见问题

### Q1: 编译报错 "undefined reference to ascii_font_8x8"

**原因**：字模数据只有 `extern` 声明，没有实际定义

**解决方法**：

**临时方案**（测试用）：
```c
// epd_font.c
const uint8_t ascii_font_8x8[760] = {0};
const uint8_t ascii_font_16x16[3040] = {0};
```

**永久方案**：
参考 `FONT_IMPORT_GUIDE.md` 导入真实字模数据

### Q2: 编译警告 "unused variable"

**原因**：某些变量未使用

**解决方法**：
```c
// 添加 (void) 转换消除警告
(void)unused_variable;
```

### Q3: 烧录后屏幕无显示

**可能原因**：
1. 硬件连接问题
2. 电源不足
3. 初始化失败

**排查步骤**：
1. 检查串口日志
2. 确认 SPI 初始化成功
3. 确认 GPIO 引脚连接正确
4. 确认屏幕供电正常

### Q4: 显示的图形方向不对

**原因**：坐标系统理解错误

**解决方法**：
- 用户坐标：X[0-295], Y[0-151]（物理屏幕）
- GUI 层已自动转换，无需手动处理
- 如果方向不对，检查底层驱动的 Data Entry Mode 配置

## 📊 性能指标

### 编译时间（估算）

| 阶段 | 时间 |
|------|------|
| 预处理 | ~5 秒 |
| 编译 | ~15 秒 |
| 链接 | ~5 秒 |
| **总计** | **~25 秒** |

### 资源使用

| 项目 | 大小 | 占比 |
|------|------|------|
| 代码段 (.text) | ~6KB | <1% |
| 只读数据 (.rodata) | ~4KB | <1% |
| 已初始化数据 (.data) | ~12KB | <1% |
| 未初始化数据 (.bss) | ~2KB | <1% |

**总计**：约 24KB，占用 Flash 空间很小

## 📝 下一步

1. **立即**：
   - [ ] 导入字模数据（参考 `FONT_IMPORT_GUIDE.md`）
   - [ ] 运行编译测试

2. **短期**：
   - [ ] 运行单元测试
   - [ ] 烧录到硬件验证

3. **长期**：
   - [ ] 根据需求优化性能
   - [ ] 扩展更多功能

## 📞 获取帮助

如有问题，请检查：

1. **编译日志**：查看详细错误信息
2. **串口日志**：查看运行时输出
3. **项目文档**：
   - `GUI_MIGRATION_REPORT.md` - 移植报告
   - `FONT_IMPORT_GUIDE.md` - 字模导入指南
   - `ssd1680.h` - 驱动头文件

---

**更新日期**: 2026-02-28  
**ESP-IDF 版本**: V5.5  
**目标芯片**: ESP32-S3  
**状态**: ✅ 代码完成，待编译验证
