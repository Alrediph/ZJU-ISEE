/**
 * AI识别服务模块接口
 */

#ifndef AI_H
#define AI_H

#include <Arduino.h>
#include "constants.h"

// ==================== AI识别配置 ====================
// 配置参数在secrets.h中定义

// ==================== 函数声明 ====================

/**
 * 初始化AI识别服务
 * @return true=成功, false=失败
 */
bool initAI();

/**
 * 调用AI识别API进行垃圾分类识别
 * @param imageData 图片数据指针
 * @param imageDataSize 图片数据大小
 * @param type 输出参数：识别出的垃圾类型
 * @param confidence 输出参数：识别置信度（0.0-1.0）
 * @return true=成功, false=失败
 */
bool recognizeGarbage(const uint8_t* imageData, size_t imageDataSize, GarbageType& type, float& confidence);

/**
 * 通过图片URL调用AI识别API进行垃圾分类识别
 * @param imageUrl 可访问的图片URL
 * @param type 输出参数：识别出的垃圾类型
 * @param confidence 输出参数：识别置信度（0.0-1.0）
 * @return true=成功, false=失败
 */
bool recognizeGarbageByUrl(const char* imageUrl, GarbageType& type, float& confidence);

#endif // AI_H
