/**
 * 华为云IoTDA模块实现
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "iotda.h"
#include "config.h"
#include "secrets.h"
#include "constants.h"
#include "display.h"
#include "led.h"

// MQTT客户端
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// 前置声明
void mqttCallback(char* topic, byte* payload, unsigned int length);
void sendCommandResponse(const char* requestId);

// ==================== 全局变量声明 ====================
extern bool captureTriggered;  // 拍照触发标志（定义在main.ino中）

// 消息缓冲区大小
#define MAX_PAYLOAD_SIZE 512
bool initIoTDA() {
    INFO_PRINTLN("初始化IoTDA连接...");

    // 配置MQTT服务器
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    // 尝试连接MQTT
    if (!connectMQTT()) {
        ERROR_PRINTLN("MQTT连接失败");
        return false;
    }

    INFO_PRINTLN("IoTDA初始化成功");
    return true;
}

// ==================== 连接MQTT服务器 ====================
bool connectMQTT() {
    INFO_PRINTLN("连接MQTT服务器...");

    // 检查WiFi连接
    if (WiFi.status() != WL_CONNECTED) {
        ERROR_PRINTLN("WiFi未连接，无法连接MQTT");
        return false;
    }

    // 打印详细连接信息
    INFO_PRINTF("MQTT服务器: %s:%d\n", MQTT_SERVER, MQTT_PORT);
    INFO_PRINTF("CLIENT_ID: %s\n", CLIENT_ID);
    INFO_PRINTF("MQTT_USER: %s\n", MQTT_USER);
    INFO_PRINTF("MQTT_PASSWORD: %s\n", MQTT_PASSWORD);
    INFO_PRINTF("WiFi信号强度: %d dBm\n", WiFi.RSSI());

    // 尝试连接（使用正确的参数顺序：CLIENT_ID, MQTT_USER, MQTT_PASSWORD）
    INFO_PRINTLN("正在建立MQTT连接...");
    if (mqttClient.connect(CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
        INFO_PRINTLN("MQTT连接成功");

        // 订阅命令下发主题
        if (mqttClient.subscribe(MQTT_TOPIC_COMMANDS)) {
            INFO_PRINTLN("订阅命令主题成功");
        } else {
            WARN_PRINTLN("订阅命令主题失败");
        }

        return true;
    } else {
        int state = mqttClient.state();
        ERROR_PRINTF("MQTT连接失败，错误代码: %d\n", state);

        // 打印详细错误信息
        switch (state) {
            case -4: ERROR_PRINTLN("MQTT_CONNECTION_TIMEOUT: 连接超时"); break;
            case -3: ERROR_PRINTLN("MQTT_CONNECTION_LOST: 连接丢失"); break;
            case -2: ERROR_PRINTLN("MQTT_CONNECT_FAILED: 连接失败"); break;
            case -1: ERROR_PRINTLN("MQTT_DISCONNECTED: 已断开连接"); break;
            case 0:  INFO_PRINTLN("MQTT_CONNECTED: 已连接"); break;
            case 1:  ERROR_PRINTLN("MQTT_CONNECT_BAD_PROTOCOL: 协议错误"); break;
            case 2:  ERROR_PRINTLN("MQTT_CONNECT_BAD_CLIENT_ID: 客户端ID错误"); break;
            case 3:  ERROR_PRINTLN("MQTT_CONNECT_UNAVAILABLE: 服务不可用"); break;
            case 4:  ERROR_PRINTLN("MQTT_CONNECT_BAD_CREDENTIALS: 凭据错误"); break;
            case 5:  ERROR_PRINTLN("MQTT_CONNECT_UNAUTHORIZED: 未授权"); break;
            default: ERROR_PRINTF("未知错误: %d\n", state); break;
        }

        return false;
    }
}

// ==================== 检查并维护MQTT连接 ====================
void checkMQTTConnection() {
    // 先检查WiFi连接状态
    if (WiFi.status() != WL_CONNECTED) {
        // WiFi未连接，无法维护MQTT
        return;
    }

    if (!mqttClient.connected()) {
        WARN_PRINTLN("MQTT连接断开，尝试重连...");
        displayMessage("IoTDA", "MQTT断开", "重连中...");

        if (connectMQTT()) {
            INFO_PRINTLN("MQTT重连成功");
            displayMessage("IoTDA", "MQTT已连接", "OK");
            delay(500);
        } else {
            ERROR_PRINTLN("MQTT重连失败");
            printMQTTStatus();
        }
    }
}

// ==================== 打印MQTT连接状态 ====================
void printMQTTStatus() {
    int state = mqttClient.state();
    INFO_PRINTF("MQTT状态码: %d\n", state);

    switch (state) {
        case -4: INFO_PRINTLN("MQTT_CONNECTION_TIMEOUT: 连接超时"); break;
        case -3: INFO_PRINTLN("MQTT_CONNECTION_LOST: 连接丢失"); break;
        case -2: INFO_PRINTLN("MQTT_CONNECT_FAILED: 连接失败"); break;
        case -1: INFO_PRINTLN("MQTT_DISCONNECTED: 已断开连接"); break;
        case 0:  INFO_PRINTLN("MQTT_CONNECTED: 已连接"); break;
        case 1:  INFO_PRINTLN("MQTT_CONNECT_BAD_PROTOCOL: 协议错误"); break;
        case 2:  INFO_PRINTLN("MQTT_CONNECT_BAD_CLIENT_ID: 客户端ID错误"); break;
        case 3:  INFO_PRINTLN("MQTT_CONNECT_UNAVAILABLE: 服务不可用"); break;
        case 4:  INFO_PRINTLN("MQTT_CONNECT_BAD_CREDENTIALS: 凭据错误"); break;
        case 5:  INFO_PRINTLN("MQTT_CONNECT_UNAUTHORIZED: 未授权"); break;
        default: INFO_PRINTLN("未知状态"); break;
    }
}

// ==================== 处理MQTT消息循环 ====================
void handleMQTTLoop() {
    if (mqttClient.connected()) {
        mqttClient.loop();
    } else {
        checkMQTTConnection();
    }
}

// ==================== MQTT消息回调函数 ====================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    INFO_PRINTLN("\n收到MQTT消息:");
    INFO_PRINTF("主题: %s\n", topic);

    // 安全处理：检查消息长度
    if (length >= MAX_PAYLOAD_SIZE) {
        WARN_PRINTLN("消息过长，已丢弃");
        return;
    }

    // 复制消息到缓冲区
    char payloadBuffer[MAX_PAYLOAD_SIZE];
    memcpy(payloadBuffer, payload, length);
    payloadBuffer[length] = '\0';

    INFO_PRINTF("内容: %s\n", payloadBuffer);

    // 优先提取request_id（无论JSON解析是否成功，都要能响应）
    char requestId[64] = "";
    const char* idStart = strstr(topic, "request_id=");
    if (idStart != NULL) {
        idStart += 11;  // 跳过 "request_id="
        strncpy(requestId, idStart, sizeof(requestId) - 1);
        requestId[sizeof(requestId) - 1] = '\0';
        INFO_PRINTF("Request ID: %s\n", requestId);
    } else {
        WARN_PRINTLN("无法提取request_id，跳过命令处理");
        return;  // 没有request_id就无法响应，只能放弃
    }

    // 解析JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payloadBuffer);

    if (error) {
        ERROR_PRINTLN("JSON解析失败");
        ERROR_PRINTF("错误: %s\n", error.c_str());
        // JSON解析失败也发送响应（带错误码），避免IoTDA等待超时
        sendCommandResponse(requestId);
        return;
    }

    // 提取命令参数（带默认值）
    const char* command = doc["command_name"] | "";

    INFO_PRINTF("命令: %s\n", command);

    // 处理不同命令
    if (strcmp(command, "capture") == 0) {
        // 触发拍照识别
        INFO_PRINTLN("收到拍照命令");
        captureTriggered = true;  // 触发状态机进入CAPTURE状态

    } else if (strcmp(command, "test") == 0) {
        // 测试命令
        INFO_PRINTLN("收到测试命令");
        displayMessage("测试", "命令接收", "成功");
    } else {
        // 未知命令也发送响应，不让IoTDA挂起
        WARN_PRINTF("未知命令: %s\n", command);
    }

    // 发送命令响应
    sendCommandResponse(requestId);
}

// ==================== 发送命令响应 ====================
void sendCommandResponse(const char* requestId) {
    // 构建响应主题
    char responseTopic[256];
    snprintf(responseTopic, sizeof(responseTopic), "%s%s", MQTT_TOPIC_CMD_RESPONSE, requestId);

    // 发布响应（使用预定义的响应数据）
    if (mqttClient.publish(responseTopic, COMMAND_RESPONSE_DATA)) {
        INFO_PRINTLN("命令响应已发送");
    } else {
        ERROR_PRINTLN("命令响应发送失败");
    }
}

// ==================== 上报设备状态 ====================
void reportStatus(const char* status) {
    if (!mqttClient.connected()) {
        WARN_PRINTLN("MQTT未连接，无法上报状态");
        return;
    }

    // 构建属性JSON
    char properties[128];
    snprintf(properties, sizeof(properties), "\"Status\":\"%s\"", status);

    // 构建完整消息（使用预定义格式）
    char jsonBuf[256];
    snprintf(jsonBuf, sizeof(jsonBuf), MQTT_BODY_FORMAT, properties);

    // 发布消息
    if (mqttClient.publish(MQTT_TOPIC_REPORT, jsonBuf)) {
        INFO_PRINTF("状态上报成功: %s\n", status);
    } else {
        ERROR_PRINTLN("状态上报失败");
    }
}

// ==================== 上报识别结果 ====================
void reportResult(GarbageType type, float confidence, const char* objectName, const char* objectUrl, const char* timestamp) {
    if (!mqttClient.connected()) {
        WARN_PRINTLN("MQTT未连接，无法上报结果");
        return;
    }

    // 构建属性JSON（包含完整识别信息）
    char properties[384];
    snprintf(properties, sizeof(properties),
        "\"Status\":\"识别完成\",\"GarbageType\":\"%s\",\"Confidence\":%.2f,\"ImageUrl\":\"%s\",\"ObjectName\":\"%s\",\"Timestamp\":\"%s\"",
        GARBAGE_NAMES[type], confidence, objectUrl, objectName, timestamp);

    // 构建完整消息（使用预定义格式）
    char jsonBuf[512];
    snprintf(jsonBuf, sizeof(jsonBuf), MQTT_BODY_FORMAT, properties);

    // 发布消息
    if (mqttClient.publish(MQTT_TOPIC_REPORT, jsonBuf)) {
        INFO_PRINTF("识别结果上报成功: %s (%.2f%%)\n", GARBAGE_NAMES[type], confidence * 100);
        INFO_PRINTF("图片URL: %s\n", objectUrl);
    } else {
        ERROR_PRINTLN("识别结果上报失败");
    }
}

// ==================== 上报错误 ====================
void reportError(ErrorCode error) {
    if (!mqttClient.connected()) {
        WARN_PRINTLN("MQTT未连接，无法上报错误");
        return;
    }

    // 构建属性JSON
    char properties[128];
    snprintf(properties, sizeof(properties), "\"Error\":\"%s\"", ERROR_MESSAGES[error]);

    // 构建完整消息（使用预定义格式）
    char jsonBuf[256];
    snprintf(jsonBuf, sizeof(jsonBuf), MQTT_BODY_FORMAT, properties);

    // 发布消息
    if (mqttClient.publish(MQTT_TOPIC_REPORT, jsonBuf)) {
        INFO_PRINTF("错误上报成功: %s\n", ERROR_MESSAGES[error]);
    } else {
        ERROR_PRINTLN("错误上报失败");
    }
}
