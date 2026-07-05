/**
 * 华为云IoTDA模块头文件
 */

#ifndef IOTDA_H
#define IOTDA_H

#include <Arduino.h>
#include "constants.h"

// 初始化IoTDA连接
bool initIoTDA();

// 连接MQTT服务器
bool connectMQTT();

// 检查并维护MQTT连接
void checkMQTTConnection();

// 打印MQTT连接状态
void printMQTTStatus();

// 处理MQTT消息循环
void handleMQTTLoop();

// 上报设备状态
void reportStatus(const char* status);

// 上报识别结果
void reportResult(GarbageType type, float confidence, const char* objectName, const char* objectUrl, const char* timestamp);

// 上报错误
void reportError(ErrorCode error);

#endif // IOTDA_H
