/**
 * OLED显示模块头文件
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// 初始化OLED
bool initOLED();

// 显示消息（3行）
void displayMessage(const char* line1, const char* line2 = "", const char* line3 = "");

// 显示标题和内容
void displayTitleContent(const char* title, const char* content);

// 显示进度
void displayProgress(const char* title, int progress);

// 显示错误
void displayError(const char* errorMsg);

// 显示识别结果
void displayResult(const char* garbageType, float confidence);

// 显示状态信息（统一格式）
void displayStatus(const char* title, const char* status, bool success);

// 显示网络状态（WiFi和MQTT）
void displayNetworkStatus();

#endif // DISPLAY_H
