/**
 * 舵机控制模块头文件
 */

#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include "constants.h"

// 初始化舵机
bool initServo();

// 打开指定垃圾桶
void openBin(GarbageType type);

// 关闭所有垃圾桶
void closeAllBins();

// 测试舵机动作
void testServoAction();

#endif // SERVO_H
