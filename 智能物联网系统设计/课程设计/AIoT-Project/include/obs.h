/**
 * 华为云OBS存储模块接口
 */

#ifndef OBS_H
#define OBS_H

#include <Arduino.h>
#include "constants.h"

// ==================== OBS配置 ====================
// 配置参数在secrets.h中定义

// ==================== 函数声明 ====================

/**
 * 初始化OBS服务
 * @return true=成功, false=失败
 */
bool initOBS();

/**
 * 上传图片到OBS
 * @param imageData 图片数据指针
 * @param dataSize 图片数据大小
 * @param objectName OBS对象名称（输出参数）
 * @param objectUrl OBS对象URL（输出参数）
 * @return true=成功, false=失败
 */
bool uploadToOBS(uint8_t* imageData, size_t dataSize, char* objectName, char* objectUrl);

/**
 * 生成OBS对象URL
 * @param objectName 对象名称
 * @param url 输出URL缓冲区
 * @param urlSize URL缓冲区大小
 */
void generateObjectUrl(const char* objectName, char* url, size_t urlSize);

/**
 * 生成OBS对象预签名下载URL
 * @param objectName OBS对象名称
 * @param url 输出URL缓冲区
 * @param urlSize URL缓冲区大小
 * @param expiresSeconds 有效期（秒）
 * @return true=成功, false=失败
 */
bool generatePresignedGetUrl(const char* objectName, char* url, size_t urlSize, uint32_t expiresSeconds);

#endif // OBS_H
