#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>

/**
 * Network Module - WiFi Utility Functions
 */

// ==================== 基础WiFi功能 ====================
// Initialize WiFi in station mode
void initWiFiMode();

// Connect to WiFi network (simple version for testing)
bool connectWiFi();

// Disconnect WiFi
void disconnectWiFi();

// Check if WiFi is connected
bool isWiFiConnected();

// Get WiFi signal strength (RSSI)
int getWiFiRSSI();

// Get WiFi MAC address
String getWiFiMAC();

// Get WiFi local IP
String getWiFiIP();

// Print WiFi status information
void printWiFiStatus();

// ==================== 高级WiFi功能 ====================
// WiFi初始化（支持多网络选择）
bool initWiFi();

// WiFi网络扫描
int scanWiFi();

// 多WiFi智能选择连接
bool connectToBestWiFi();

// 单WiFi连接
bool setupWiFi();

// 检查WiFi连接状态
bool checkWiFiConnection();

// WiFi重连
bool reconnectWiFi();

// ==================== 重试延迟管理 ====================
// Get current WiFi retry delay
int getWiFiRetryDelay();

// Reset WiFi retry delay to initial value
void resetWiFiRetryDelay();

// Update WiFi retry delay (exponential backoff)
void updateWiFiRetryDelay();

// ==================== NTP时间同步 ====================
// Sync NTP time
bool syncNTPTime();

// ==================== 调试辅助 ====================
// Print WiFi status code meaning
void printWiFiStatusCode();

#endif // NETWORK_H
