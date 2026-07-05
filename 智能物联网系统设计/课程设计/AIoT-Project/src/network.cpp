/**
 * Network Module - WiFi Utility Functions
 * Provides WiFi connection and management functionality
 */

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "network.h"
#include "secrets.h"
#include "config.h"
#include "constants.h"

// WiFi retry configuration
static int wifiRetryDelay = WIFI_RETRY_INIT;

/**
 * Initialize WiFi in station mode
 */
void initWiFiMode() {
    WiFi.mode(WIFI_STA);
    INFO_PRINTLN("WiFi mode set to Station");
}

/**
 * Connect to WiFi network
 * @return true if connected successfully
 */
bool connectWiFi() {
    INFO_PRINTF("Connecting to WiFi: %s\n", WIFI_SSID);

    // Ensure station mode
    if (WiFi.getMode() != WIFI_STA) {
        WiFi.mode(WIFI_STA);
    }

    // Start connection
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Wait for connection with timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_CONNECT_TIMEOUT / 500) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        INFO_PRINTLN("\nWiFi connected successfully");
        INFO_PRINTF("IP address: %s\n", WiFi.localIP().toString().c_str());
        wifiRetryDelay = WIFI_RETRY_INIT;
        return true;
    } else {
        ERROR_PRINTLN("\nWiFi connection failed");
        return false;
    }
}

/**
 * Disconnect WiFi
 */
void disconnectWiFi() {
    WiFi.disconnect(true);
    INFO_PRINTLN("WiFi disconnected");
}

/**
 * Check if WiFi is connected
 * @return true if connected
 */
bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

/**
 * Get WiFi signal strength (RSSI)
 * @return RSSI value in dBm
 */
int getWiFiRSSI() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}

/**
 * Get WiFi MAC address
 * @return MAC address string
 */
String getWiFiMAC() {
    return WiFi.macAddress();
}

/**
 * Get WiFi local IP
 * @return IP address string
 */
String getWiFiIP() {
    return WiFi.localIP().toString();
}

/**
 * Print WiFi status information
 */
void printWiFiStatus() {
    Serial.println("\n=== WiFi Status ===");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Status: Connected\n");
        Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
        Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
        Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
        Serial.printf("Subnet: %s\n", WiFi.subnetMask().toString().c_str());
    } else {
        Serial.println("Status: Disconnected");
    }
    Serial.println("===================");
}

/**
 * Get current WiFi retry delay
 * @return retry delay in milliseconds
 */
int getWiFiRetryDelay() {
    return wifiRetryDelay;
}

/**
 * Reset WiFi retry delay to initial value
 */
void resetWiFiRetryDelay() {
    wifiRetryDelay = WIFI_RETRY_INIT;
}

/**
 * Update WiFi retry delay (exponential backoff)
 */
void updateWiFiRetryDelay() {
    wifiRetryDelay = min(wifiRetryDelay * 2, WIFI_RETRY_MAX);
}

// ==================== 高级WiFi功能 ====================

/**
 * WiFi初始化（支持多网络选择）
 * @return true if connected successfully
 */
bool initWiFi() {
#if USE_MULTI_WIFI
    return connectToBestWiFi();
#else
    return setupWiFi();
#endif
}

/**
 * WiFi网络扫描
 * @return number of networks found
 */
int scanWiFi() {
    INFO_PRINTLN("正在扫描WiFi网络...");
    int n = WiFi.scanNetworks();
    if (n <= 0) {
        WARN_PRINTLN("未找到WiFi网络");
        return 0;
    }
    INFO_PRINTF("找到 %d 个WiFi网络\n", n);
    return n;
}

/**
 * 多WiFi智能选择连接
 * @return true if connected successfully
 */
bool connectToBestWiFi() {
    INFO_PRINTLN("正在查找可用的WiFi网络...");

    // 扫描网络
    int n = scanWiFi();
    if (n <= 0) {
        return false;
    }

    // 按优先级查找匹配的WiFi网络
    for (int i = 0; i < WIFI_NETWORK_COUNT; i++) {
        const char* targetSSID = wifiNetworks[i].ssid;
        const char* targetPassword = wifiNetworks[i].password;

        // 检查扫描结果中是否存在该网络
        for (int j = 0; j < n; j++) {
            if (strcmp(WiFi.SSID(j).c_str(), targetSSID) == 0) {
                int rssi = WiFi.RSSI(j);
                INFO_PRINTF("找到匹配网络: %s (优先级: %d, 信号强度: %d dBm)\n",
                            targetSSID, wifiNetworks[i].priority, rssi);

                // 尝试连接
                INFO_PRINTF("正在连接到 %s", targetSSID);
                WiFi.begin(targetSSID, targetPassword);

                // 等待连接
                int attempts = 0;
                while (WiFi.status() != WL_CONNECTED && attempts < WIFI_CONNECT_TIMEOUT / 500) {
                    delay(500);
                    INFO_PRINT(".");
                    attempts++;
                }

                if (WiFi.status() == WL_CONNECTED) {
                    INFO_PRINTLN("\nWiFi连接成功!");
                    INFO_PRINTF("已连接到: %s\n", targetSSID);
                    INFO_PRINTF("IP地址: %s\n", WiFi.localIP().toString().c_str());
                    INFO_PRINTF("信号强度: %d dBm\n", rssi);

                    // 同步NTP时间
                    syncNTPTime();

                    // 重置重试延迟
                    resetWiFiRetryDelay();

                    return true;
                } else {
                    WARN_PRINTF("\n连接 %s 失败，尝试下一个网络...\n", targetSSID);
                    WiFi.disconnect();
                }
                break;  // 找到匹配的网络后跳出内层循环
            }
        }
    }

    ERROR_PRINTLN("所有配置的WiFi网络都无法连接");
    return false;
}

/**
 * 单WiFi连接
 * @return true if connected successfully
 */
bool setupWiFi() {
    INFO_PRINTF("连接WiFi: %s\n", WIFI_SSID);

    // 确保WiFi处于站点模式
    if (WiFi.getMode() != WIFI_STA) {
        WiFi.mode(WIFI_STA);
        delay(100);
    }

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // 等待连接
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_CONNECT_TIMEOUT / 500) {
        delay(500);
        INFO_PRINT(".");
        attempts++;
    }
    INFO_PRINTLN();

    if (WiFi.status() == WL_CONNECTED) {
        INFO_PRINTLN("WiFi连接成功");
        INFO_PRINTF("IP地址: %s\n", WiFi.localIP().toString().c_str());

        // 同步NTP时间
        syncNTPTime();

        // 重置重试延迟
        resetWiFiRetryDelay();

        return true;
    } else {
        ERROR_PRINTLN("WiFi连接失败");
        printWiFiStatusCode();
        return false;
    }
}

/**
 * 检查WiFi连接状态
 * @return true if connected
 */
bool checkWiFiConnection() {
    return WiFi.status() == WL_CONNECTED;
}

/**
 * WiFi重连
 * @return true if reconnected successfully
 */
bool reconnectWiFi() {
#if USE_MULTI_WIFI
    return connectToBestWiFi();
#else
    return setupWiFi();
#endif
}

/**
 * 同步NTP时间
 * @return true if sync successful
 */
bool syncNTPTime() {
    INFO_PRINTLN("正在同步NTP时间...");

    // 配置NTP时间同步 (UTC+8时区)
    configTime(NTP_TIMEZONE * 3600, 0, NTP_SERVER1, NTP_SERVER2);

    // 等待时间同步完成
    struct tm timeinfo;
    int timeSyncAttempts = 0;
    while (!getLocalTime(&timeinfo) && timeSyncAttempts < NTP_SYNC_TIMEOUT) {
        INFO_PRINT(".");
        delay(1000);
        timeSyncAttempts++;
    }

    if (getLocalTime(&timeinfo)) {
        INFO_PRINTLN("\nNTP时间同步成功");
        char timeStr[30];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        INFO_PRINTF("当前时间: %s\n", timeStr);
        return true;
    } else {
        WARN_PRINTLN("\nNTP时间同步失败，将使用空时间");
        return false;
    }
}

/**
 * 打印WiFi状态码含义
 */
void printWiFiStatusCode() {
    INFO_PRINTF("WiFi状态码: %d\n", WiFi.status());

    switch (WiFi.status()) {
        case 0:  INFO_PRINTLN("WL_IDLE_STATUS: 正在改变状态"); break;
        case 1:  INFO_PRINTLN("WL_NO_SSID_AVAIL: 未找到该WiFi网络"); break;
        case 2:  INFO_PRINTLN("WL_SCAN_COMPLETED: 扫描完成"); break;
        case 3:  INFO_PRINTLN("WL_CONNECTED: 已连接"); break;
        case 4:  INFO_PRINTLN("WL_CONNECT_FAILED: 连接失败"); break;
        case 5:  INFO_PRINTLN("WL_CONNECTION_LOST: 连接丢失"); break;
        case 6:  INFO_PRINTLN("WL_DISCONNECTED: 已断开连接"); break;
        default: INFO_PRINTLN("未知状态"); break;
    }
}
