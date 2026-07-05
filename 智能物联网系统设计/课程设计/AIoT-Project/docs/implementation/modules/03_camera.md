# 模块三：摄像头模块 (camera.cpp)

## 1. 模块定位

`camera.cpp` 封装了 OV3660 摄像头的初始化和拍照操作，为上层状态机提供简洁的图像采集接口。模块管理 PSRAM 帧缓冲区的分配与释放。

文件位置：`src/camera.cpp`（161行） | 头文件：`include/camera.h`

---

## 2. 硬件配置

### 2.1 摄像头引脚映射

OV3660 是并行接口摄像头，占用 ESP32-S3 的 16 个 GPIO：

```cpp
// include/config.h
#define Y2_GPIO_NUM    11    // 数据位 D0-D7（8根并行数据线）
#define Y3_GPIO_NUM     9
#define Y4_GPIO_NUM     8
#define Y5_GPIO_NUM    10
#define Y6_GPIO_NUM    12
#define Y7_GPIO_NUM    18
#define Y8_GPIO_NUM    17
#define Y9_GPIO_NUM    16

#define XCLK_GPIO_NUM  15    // 主时钟输出 (20MHz)
#define PCLK_GPIO_NUM  13    // 像素时钟输入
#define VSYNC_GPIO_NUM  6    // 垂直同步
#define HREF_GPIO_NUM   7    // 水平参考
#define SIOD_GPIO_NUM   4    // SCCB 数据（I2C-like）
#define SIOC_GPIO_NUM   5    // SCCB 时钟
#define PWDN_GPIO_NUM  -1    // 未使用（常低）
#define RESET_GPIO_NUM -1    // 未使用（常高）
```

### 2.2 图像参数

```cpp
#define CAMERA_PIXFORMAT  PIXFORMAT_JPEG   // JPEG压缩格式
#define CAMERA_FRAMESIZE  FRAMESIZE_VGA    // 640×480 分辨率
#define CAMERA_QUALITY    10               // JPEG质量 (1-63, 越小越好)
#define CAMERA_FB_COUNT   2                // 双帧缓冲
```

---

## 3. 初始化流程

### 3.1 initCamera() 实现

```cpp
bool initCamera() {
    // ========== 步骤1：填充配置结构体 ==========
    camera_config_t config;
    // ... 16个引脚分配 ...
    config.xclk_freq_hz = 20000000;          // 20MHz 主时钟
    config.frame_size = CAMERA_FRAMESIZE;     // VGA 640×480
    config.pixel_format = CAMERA_PIXFORMAT;   // JPEG
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;  // 帧缓冲存在PSRAM
    config.jpeg_quality = CAMERA_QUALITY;     // 质量10
    config.fb_count = CAMERA_FB_COUNT;        // 双缓冲

    // ========== 步骤2：PSRAM 检测与配置优化 ==========
    if (psramFound()) {
        config.jpeg_quality = 10;          // 高质量JPEG
        config.fb_count = 2;               // 双缓冲提升稳定性
        config.grab_mode = CAMERA_GRAB_LATEST;  // 获取最新帧
    } else {
        config.frame_size = FRAMESIZE_SVGA;     // 降级到800×600
        config.fb_location = CAMERA_FB_IN_DRAM; // 帧缓冲存在DRAM
    }

    // ========== 步骤3：初始化摄像头硬件 ==========
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        return false;  // 硬件初始化失败
    }

    // ========== 步骤4：传感器参数调优 ==========
    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);           // 垂直翻转（摄像头倒装的补偿）
    s->set_brightness(s, 1);      // 亮度 +1
    s->set_whitebal(s, 1);        // 开启白平衡
    s->set_awb_gain(s, 1);        // 自动白平衡增益
    s->set_exposure_ctrl(s, 1);   // 自动曝光控制
    s->set_aec_value(s, 168);     // 曝光值
    s->set_gain_ctrl(s, 1);       // 自动增益控制
    s->set_wpc(s, 1);             // 白像素校正
    s->set_raw_gma(s, 1);         // 伽马校正
    s->set_lenc(s, 1);            // 镜头校正
    s->set_dcw(s, 1);             // 下采样使能
    s->set_hmirror(s, 0);         // 不进行水平镜像

    return true;
}
```

### 3.2 PSRAM 检测的作用

ESP32-S3-CAM 开发板配备 8MB PSRAM，但代码需要兼容无 PSRAM 的情况：

| 特性 | 有 PSRAM | 无 PSRAM |
|------|---------|---------|
| 帧缓冲区位置 | `CAMERA_FB_IN_PSRAM` | `CAMERA_FB_IN_DRAM` |
| JPEG 质量 | 10（高质量） | 12（稍低） |
| 缓冲区数量 | 2（双缓冲） | 1 |
| 帧获取模式 | `CAMERA_GRAB_LATEST` | `CAMERA_GRAB_WHEN_EMPTY` |

**双缓冲区的好处**：摄像头硬件写入一个缓冲区时，CPU 可读取另一个，减少等待。

### 3.3 传感器调优参数说明

```cpp
s->set_vflip(s, 1);          // 垂直翻转：摄像头在PCB上通常是倒装的
s->set_brightness(s, 1);     // 亮度+1：稍微提高，改善暗光表现
s->set_whitebal(s, 1);       // 开启自动白平衡
s->set_exposure_ctrl(s, 1);  // 开启自动曝光
s->set_aec_value(s, 168);    // 自动曝光目标值（168/255）
s->set_lenc(s, 1);           // 镜头畸变校正
s->set_dcw(s, 1);            // 下采样：可将高分辨率传感器输出缩放到VGA
```

这些参数在 Phase 9 和最近的提交中经过多次调试优化，目标是获得清晰、色彩准确的垃圾物品照片，以提高 AI 识别准确率。

---

## 4. 拍照与释放

### 4.1 capturePhoto()

```cpp
camera_fb_t* capturePhoto() {
    delay(CAPTURE_DELAY);  // 等待摄像头稳定（100ms）

    camera_fb_t *fb = esp_camera_fb_get();  // 获取帧缓冲区指针

    if (fb == nullptr) {
        return nullptr;  // 获取失败
    }

    if (fb->len == 0) {
        esp_camera_fb_return(fb);  // 归还空帧
        return nullptr;
    }

    // 成功：打印帧信息
    INFO_PRINTF("拍照成功: %d x %d, 大小: %d 字节\n",
                fb->width, fb->height, fb->len);
    return fb;
}
```

**返回的 `camera_fb_t` 结构体**：
```cpp
typedef struct {
    uint8_t* buf;     // 指向帧数据（在PSRAM中）
    size_t len;       // 数据长度（字节）
    size_t width;     // 图像宽度（像素）
    size_t height;    // 图像高度（像素）
    pixformat_t format;  // 像素格式
} camera_fb_t;
```

**关键点**：
- 帧数据存储在 PSRAM 中，VGA JPEG 通常 30-60KB
- 调用者负责在不需要时调用 `releasePhoto()` 释放
- `CAPTURE_DELAY` 为 100ms，让摄像头在拍照前稳定曝光和白平衡

### 4.2 releasePhoto()

```cpp
void releasePhoto(camera_fb_t* fb) {
    if (fb != nullptr) {
        esp_camera_fb_return(fb);  // 归还到帧缓冲池
    }
}
```

**为何要及时释放？**
帧缓冲区数量有限（2个），不及时归还会导致后续拍照无法获取缓冲区。在状态机中，UPLOAD 状态上传完成后立即释放，ERROR 状态也会清理。

---

## 5. 与其他模块的接口

| 函数 | 调用者 | 用途 |
|------|--------|------|
| `initCamera()` | main.ino setup() | 初始化摄像头硬件 |
| `capturePhoto()` | main.ino STATE_CAPTURE | 拍照获取帧缓冲 |
| `releasePhoto()` | main.ino (多处) | 归还帧缓冲 |
| `printCameraInfo()` | initCamera() 内部 | 打印诊断信息 |

---

## 6. 关键设计决策

1. **JPEG 格式而非 RGB**：JPEG 压缩后体积小（~40KB vs ~900KB），适合网络上传
2. **VGA 分辨率**：640×480 在清晰度和文件大小间平衡，AI 模型对此分辨率足够
3. **PSRAM 优先**：充分利用 8MB PSRAM，避免占用宝贵的内部 DRAM
4. **垂直翻转**：摄像头物理安装方向与逻辑方向相反，必须软翻转
5. **自动曝光与白平衡**：适应不同光照条件的演示环境
