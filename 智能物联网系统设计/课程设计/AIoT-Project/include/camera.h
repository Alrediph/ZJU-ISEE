#ifndef CAMERA_H
#define CAMERA_H

#include "esp_camera.h"
#include "config.h"
#include "constants.h"

// ==================== 摄像头模块 ====================

/**
 * 初始化摄像头
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool initCamera();

/**
 * 拍照并返回帧缓冲区
 * @return camera_fb_t* 帧缓冲区指针，失败返回nullptr
 */
camera_fb_t* capturePhoto();

/**
 * 释放帧缓冲区
 * @param fb 帧缓冲区指针
 */
void releasePhoto(camera_fb_t* fb);

/**
 * 打印摄像头信息
 */
void printCameraInfo();

#endif // CAMERA_H
