/**
 * LED控制模块头文件
 */

#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include "constants.h"

// 初始化LED
bool initLED();

// 设置LED颜色
void setLEDColor(LEDColor color);

// LED闪烁
void blinkLED(LEDColor color, int times, int delayMs);

// LED呼吸效果
void breatheLED(LEDColor color, int durationMs);

#endif // LED_H
