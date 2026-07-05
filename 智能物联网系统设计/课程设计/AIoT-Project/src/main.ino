/**
 * ESP32-S3-CAM 垃圾分类识别系统
 * 主程序入口
 */

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>  // NTP时间同步
#include <esp_task_wdt.h>  // 看门狗
#include "config.h"
#include "constants.h"
#include "secrets.h"  // WiFi网络配置
#include "network.h"  // WiFi功能模块
#include "display.h"
#include "led.h"
#include "camera.h"
#include "servo.h"
#include "iotda.h"
#include "obs.h"
#include "ai.h"

// ==================== 全局变量 ====================
SystemState currentState = STATE_IDLE;
ErrorCode lastError = ERROR_NONE;
unsigned long stateStartTime = 0;

// 状态机数据
bool captureTriggered = false;  // 拍照触发标志
camera_fb_t* photoBuffer = NULL;  // 照片缓冲区
char objectName[64] = "";  // OBS对象名称
char objectUrl[256] = "";  // OBS对象URL
char presignedImageUrl[768] = "";  // AI识图使用的预签名URL
char recordName[64] = "";  // OBS记录名称
char timestamp[32] = "";  // 时间戳
GarbageType garbageType = GARBAGE_UNKNOWN;  // 识别结果
float confidence = 0.0;  // 置信度

// 定时任务
unsigned long lastStatusReportTime = 0;  // 上次状态上报时间

// ==================== 函数声明 ====================
void checkAndReconnectWiFi();  // WiFi重连维护（包含显示逻辑）
void stateMachineLoop();
void checkAndReportStatus();  // 定时状态上报
String getTimestamp();  // 获取时间戳辅助函数

// ==================== 获取时间戳 ====================
String getTimestamp() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "00:00:00";  // 返回默认时间
    }
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return String(buffer);
}

// ==================== Setup ====================
void setup() {
    Serial.begin(115200);
    delay(1000);

    // 配置看门狗
    INFO_PRINTLN("配置看门狗...");
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);

    INFO_PRINTLN("================================");
    INFO_PRINTLN("ESP32-S3-CAM 垃圾分类识别系统");
    INFO_PRINTLN("================================");

    // Init OLED显示
    if (!initOLED()) {
        ERROR_PRINTLN("OLED初始化失败");
        while (1) delay(1000);
    }
    displayMessage("Booting...", "Init OLED", "OK");

    // Init LED
    if (!initLED()) {
        ERROR_PRINTLN("LED初始化失败");
        displayMessage("Booting...", "Init LED", "Failed");
        while (1) delay(1000);
    }
    setLEDColor(LED_BLUE);
    displayMessage("Booting...", "Init LED", "OK");

    // 初始化WiFi
    displayMessage("Booting...", "WiFi Connect", "");
    if (initWiFi()) {
        displayMessage("Booting...", "WiFi Connect", "OK");
        INFO_PRINTF("WiFi已连接: %s\n", WiFi.localIP().toString().c_str());
    } else {
        displayMessage("Booting...", "WiFi Connect", "Failed");
        ERROR_PRINTLN("WiFi连接失败");
        lastError = ERROR_WIFI_CONNECT;
        currentState = STATE_ERROR;
        return;
    }

    // Init Camera
    displayMessage("Booting...", "Init Camera", "");
    if (!initCamera()) {
        ERROR_PRINTLN("摄像头初始化失败");
        displayMessage("Booting...", "Init Camera", "Failed");
        lastError = ERROR_CAMERA_INIT;
        currentState = STATE_ERROR;
        return;
    }
    displayMessage("Booting...", "Init Camera", "OK");

    // Init Servo
    if (!initServo()) {
        ERROR_PRINTLN("舵机初始化失败");
        displayMessage("Booting...", "Init Servo", "Failed");
        lastError = ERROR_SERVO_INIT;
        currentState = STATE_ERROR;
        return;
    }
    displayMessage("Booting...", "Init Servo", "OK");

    // 初始化IoTDA
    displayMessage("Booting...", "IoTDA Connect", "");
    if (!initIoTDA()) {
        ERROR_PRINTLN("IoTDA初始化失败");
        displayMessage("Booting...", "IoTDA Connect", "Failed");
        lastError = ERROR_MQTT_CONNECT;
        currentState = STATE_ERROR;
        return;
    }
    displayMessage("Booting...", "IoTDA Connect", "OK");

    // Init OBS
    if (!initOBS()) {
        ERROR_PRINTLN("OBS初始化失败");
        displayMessage("Booting...", "Init OBS", "Failed");
        lastError = ERROR_OBS_UPLOAD;
        currentState = STATE_ERROR;
        return;
    }
    displayMessage("Booting...", "Init OBS", "OK");

    // Init AI识别服务
    if (!initAI()) {
        ERROR_PRINTLN("AI识别服务初始化失败");
        displayMessage("Booting...", "Init AI", "Failed");
        lastError = ERROR_AI_REQUEST;
        currentState = STATE_ERROR;
        return;
    }
    displayMessage("Booting...", "Init AI", "OK");

    // System Ready
    displayMessage("System Ready", "Awaiting Trigger", "");
    setLEDColor(LED_GREEN);
    currentState = STATE_IDLE;

    INFO_PRINTLN("系统初始化完成");
}

// ==================== Loop ====================
void loop() {
    // 1. 喂狗（防止系统死机）
    esp_task_wdt_reset();

    // 2. 网络维护层
    checkAndReconnectWiFi();    // WiFi自动重连
    handleMQTTLoop();           // MQTT消息处理

    // 3. 业务逻辑层
    stateMachineLoop();         // 状态机执行
    checkAndReportStatus();     // 定时状态上报

    // 4. 延时（降低CPU占用）
    delay(100);
}

// ==================== 检查并重连WiFi ====================
void checkAndReconnectWiFi() {
    if (!checkWiFiConnection()) {
        WARN_PRINTLN("\nWiFi连接断开，正在重连...");
        displayMessage("Network Status", "WiFi Disconnected", "Reconnecting...");

        // 尝试重连WiFi
        if (reconnectWiFi()) {
            displayMessage("Network Status", "WiFi Connected", "OK");
            delay(500);
        } else {
            WARN_PRINTF("WiFi重连失败，%d秒后重试...\n", getWiFiRetryDelay()/1000);
            displayMessage("Network Status", "Reconnect Failed", "");
            updateWiFiRetryDelay();
        }
    }
}

// ==================== 定时状态上报 ====================
void checkAndReportStatus() {
    unsigned long currentTime = millis();

    // 检查是否到达上报时间
    if (currentTime - lastStatusReportTime >= STATUS_REPORT_INTERVAL) {
        lastStatusReportTime = currentTime;

        // 仅在空闲状态时上报（避免干扰业务流程）
        if (currentState == STATE_IDLE) {
            INFO_PRINTLN("定时状态上报...");

            // 上报系统状态
            reportStatus("在线");

            // 更新OLED显示
            displayNetworkStatus();
        }
    }
}

// ==================== 状态机循环 ====================
void stateMachineLoop() {
    switch (currentState) {
        case STATE_IDLE:
            // 空闲状态，等待触发
            if (captureTriggered) {
                INFO_PRINTLN("触发拍照流程");
                captureTriggered = false;
                currentState = STATE_CAPTURE;
                stateStartTime = millis();
                setLEDColor(LED_BLUE);
                displayMessage("Capturing...", "Please Wait", "");
            }
            break;

        case STATE_CAPTURE: {
            // 拍照状态
            INFO_PRINTLN("开始拍照...");
            photoBuffer = capturePhoto();

            if (photoBuffer == NULL) {
                ERROR_PRINTLN("拍照失败");
                lastError = ERROR_CAMERA_CAPTURE;
                currentState = STATE_ERROR;
            } else {
                INFO_PRINTF("拍照成功，大小: %d 字节\n", photoBuffer->len);
                currentState = STATE_UPLOAD;  // 先上传并生成URL，再识别
                stateStartTime = millis();
                displayMessage("Uploading...", "Please Wait", "");
            }
            break;
        }

        case STATE_UPLOAD: {
            // 上传状态
            INFO_PRINTLN("开始上传图片到OBS...");

            // 生成时间戳（用于图片名称和记录）
            snprintf(timestamp, sizeof(timestamp), "%lu", millis());

            if (uploadToOBS(photoBuffer->buf, photoBuffer->len, objectName, objectUrl)) {
                INFO_PRINTLN("图片上传成功");
                INFO_PRINTF("对象URL: %s\n", objectUrl);

                if (!generatePresignedGetUrl(objectName, presignedImageUrl, sizeof(presignedImageUrl), OBS_PRESIGNED_URL_EXPIRES)) {
                    ERROR_PRINTLN("预签名URL生成失败");
                    lastError = ERROR_OBS_UPLOAD;

                    releasePhoto(photoBuffer);
                    photoBuffer = NULL;

                    currentState = STATE_ERROR;
                    break;
                }

                INFO_PRINTLN("预签名URL生成成功");

                // 释放照片缓冲区（上传完成后不再需要）
                releasePhoto(photoBuffer);
                photoBuffer = NULL;

                currentState = STATE_RECOGNIZE;
                stateStartTime = millis();
                displayMessage("Recognizing...", "Please Wait", "");
            } else {
                ERROR_PRINTLN("图片上传失败");
                lastError = ERROR_OBS_UPLOAD;

                // 释放照片缓冲区
                releasePhoto(photoBuffer);
                photoBuffer = NULL;

                currentState = STATE_ERROR;
            }
            break;
        }

        case STATE_RECOGNIZE: {
            // 识别状态
            INFO_PRINTLN("开始基于OBS URL的AI识别...");

            if (recognizeGarbageByUrl(presignedImageUrl, garbageType, confidence)) {
                INFO_PRINTF("识别成功: %s (%.2f%%)\n", GARBAGE_NAMES[garbageType], confidence * 100);

                // 上报识别结果（包含稳定对象URL，供后续系统使用）
                reportResult(garbageType, confidence, objectName, objectUrl, timestamp);

                currentState = STATE_EXECUTE;
                stateStartTime = millis();
                displayMessage("Result:", GARBAGE_NAMES[garbageType], "");
            } else {
                ERROR_PRINTLN("AI识别失败");
                lastError = ERROR_AI_RESPONSE;
                currentState = STATE_ERROR;
            }
            break;
        }

        case STATE_EXECUTE: {
            // 执行状态
            INFO_PRINTLN("执行舵机动作...");

            // 根据垃圾类型打开对应垃圾桶
            openBin(garbageType);

            // 上报状态
            reportStatus("完成");

            // 返回空闲状态
            setLEDColor(LED_GREEN);
            displayMessage("System Ready", "Awaiting Trigger", "");
            currentState = STATE_IDLE;

            INFO_PRINTLN("执行完成，返回空闲状态");
            break;
        }

        case STATE_ERROR:
            // 错误状态
            ERROR_PRINTF("系统错误: %s\n", ERROR_MESSAGES[lastError]);
            setLEDColor(LED_RED);
            displayError(ERROR_MESSAGES[lastError]);

            // 上报错误
            reportError(lastError);

            // 清理资源
            if (photoBuffer != NULL) {
                releasePhoto(photoBuffer);
                photoBuffer = NULL;
            }

            delay(5000);
            currentState = STATE_IDLE;
            lastError = ERROR_NONE;
            setLEDColor(LED_GREEN);
            displayMessage("System Ready", "Awaiting Trigger", "");
            break;
    }
}
