/**
 * 摄像头模块实现
 */

#include <Arduino.h>
#include "camera.h"

// ==================== 初始化摄像头 ====================
bool initCamera() {
    INFO_PRINTLN("初始化摄像头...");

    // 摄像头配置结构体
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;  // 20MHz XCLK
    config.frame_size = CAMERA_FRAMESIZE;
    config.pixel_format = CAMERA_PIXFORMAT;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;  // 使用PSRAM存储帧缓冲区
    config.jpeg_quality = CAMERA_QUALITY;
    config.fb_count = CAMERA_FB_COUNT;

    // 如果检测到PSRAM，使用更高质量的配置
    if(psramFound()){
        config.jpeg_quality = 10;  // 进一步提升JPEG质量
        config.fb_count = 2;      // 双缓冲区，提升稳定性
        config.grab_mode = CAMERA_GRAB_LATEST;
        INFO_PRINTLN("检测到PSRAM，使用高质量配置");
    } else {
        // 如果没有PSRAM，限制帧大小并使用DRAM
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
        WARN_PRINTLN("未检测到PSRAM，使用SVGA分辨率");
    }

    // 初始化摄像头
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ERROR_PRINTF("摄像头初始化失败: 0x%x\n", err);
        return false;
    }

    // 获取传感器对象并配置
    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr) {
        ERROR_PRINTLN("无法获取传感器对象");
        return false;
    }

    // 配置传感器参数（优化图像质量）
    s->set_vflip(s, 1);         // 垂直翻转（重要！）
    s->set_brightness(s, 1);    // 亮度稍微提高
    s->set_contrast(s, 0);      // 对比度
    s->set_saturation(s, 0);    // 饱和度降低一点
    s->set_special_effect(s, 0);  // 特效 0-6
    s->set_whitebal(s, 1);      // 白平衡
    s->set_awb_gain(s, 1);      // 自动白平衡增益
    s->set_wb_mode(s, 0);       // 白平衡模式
    s->set_exposure_ctrl(s, 1); // 曝光控制
    s->set_aec_value(s, 168);   // 曝光值
    s->set_ae_level(s, 0);      // 自动曝光等级 -2 to 2
    s->set_gain_ctrl(s, 1);     // 增益控制
    s->set_agc_gain(s, 0);      // 自动增益
    s->set_gainceiling(s, (gainceiling_t)0);  // 增益上限
    s->set_bpc(s, 0);           // 黑像素校正
    s->set_wpc(s, 1);           // 白像素校正
    s->set_raw_gma(s, 1);       // 伽马校正
    s->set_lenc(s, 1);          // 镜头校正
    s->set_hmirror(s, 0);       // 水平镜像
    s->set_dcw(s, 1);           // 下采样
    s->set_colorbar(s, 0);      // 彩条测试

    INFO_PRINTLN("摄像头初始化成功");
    printCameraInfo();

    return true;
}

// ==================== 拍照 ====================
camera_fb_t* capturePhoto() {
    DEBUG_PRINTLN("开始拍照...");

    // 等待摄像头稳定
    delay(CAPTURE_DELAY);

    // 获取帧缓冲区
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb == nullptr) {
        ERROR_PRINTLN("拍照失败: 无法获取帧缓冲区");
        return nullptr;
    }

    // 验证帧数据
    if (fb->len == 0) {
        ERROR_PRINTLN("拍照失败: 帧数据为空");
        esp_camera_fb_return(fb);
        return nullptr;
    }

    INFO_PRINTF("拍照成功: %d x %d, 大小: %d 字节\n", fb->width, fb->height, fb->len);

    return fb;
}

// ==================== 释放帧缓冲区 ====================
void releasePhoto(camera_fb_t* fb) {
    if (fb != nullptr) {
        esp_camera_fb_return(fb);
        DEBUG_PRINTLN("帧缓冲区已释放");
    }
}

// ==================== 打印摄像头信息 ====================
void printCameraInfo() {
    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr) {
        WARN_PRINTLN("无法获取传感器信息");
        return;
    }

    INFO_PRINTLN("========== 摄像头信息 ==========");
    INFO_PRINTF("分辨率: %s\n",
        CAMERA_FRAMESIZE == FRAMESIZE_QVGA ? "320x240 (QVGA)" :
        CAMERA_FRAMESIZE == FRAMESIZE_VGA ? "640x480 (VGA)" :
        CAMERA_FRAMESIZE == FRAMESIZE_SVGA ? "800x600 (SVGA)" :
        CAMERA_FRAMESIZE == FRAMESIZE_XGA ? "1024x768 (XGA)" :
        CAMERA_FRAMESIZE == FRAMESIZE_SXGA ? "1280x1024 (SXGA)" :
        CAMERA_FRAMESIZE == FRAMESIZE_UXGA ? "1600x1200 (UXGA)" : "Unknown");

    INFO_PRINTF("像素格式: %s\n",
        CAMERA_PIXFORMAT == PIXFORMAT_JPEG ? "JPEG" :
        CAMERA_PIXFORMAT == PIXFORMAT_RGB565 ? "RGB565" :
        CAMERA_PIXFORMAT == PIXFORMAT_YUV422 ? "YUV422" : "Unknown");

    INFO_PRINTF("JPEG质量: %d (1-63, 越小质量越高)\n", CAMERA_QUALITY);
    INFO_PRINTF("帧缓冲区数量: %d\n", CAMERA_FB_COUNT);
    INFO_PRINTF("帧缓冲区位置: PSRAM\n");

    // 打印传感器ID
    INFO_PRINTF("传感器ID: 0x%x\n", s->id.PID);

    INFO_PRINTLN("================================");
}
