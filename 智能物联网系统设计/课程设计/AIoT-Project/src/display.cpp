/**
 * OLED显示模块
 */

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "constants.h"

// OLED显示对象
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 外部MQTT客户端对象（定义在iotda.cpp中）
extern PubSubClient mqttClient;

// ==================== 初始化OLED ====================
bool initOLED() {
    // 初始化I2C
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

    // 初始化SSD1306
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        ERROR_PRINTLN("SSD1306初始化失败");
        return false;
    }

    // 清屏
    display.clearDisplay();
    display.display();

    INFO_PRINTLN("OLED初始化成功");
    return true;
}

// ==================== 显示消息（3行） ====================
void displayMessage(const char* line1, const char* line2, const char* line3) {
    display.clearDisplay();

    // 设置字体大小
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // 显示第一行
    display.setCursor(0, 0);
    display.println(line1);

    // 显示第二行
    display.setCursor(0, 16);
    display.println(line2);

    // 显示第三行
    display.setCursor(0, 32);
    display.println(line3);

    display.display();

    DEBUG_PRINTF("[OLED] %s | %s | %s\n", line1, line2, line3);
}

// ==================== 显示标题和内容 ====================
void displayTitleContent(const char* title, const char* content) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // 标题
    display.setCursor(0, 0);
    display.println(title);

    // 分隔线
    display.drawLine(0, 12, SCREEN_WIDTH - 1, 12, SSD1306_WHITE);

    // 内容
    display.setCursor(0, 20);
    display.println(content);

    display.display();
}

// ==================== 显示进度 ====================
void displayProgress(const char* title, int progress) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // 标题
    display.setCursor(0, 0);
    display.println(title);

    // 进度条边框
    int barX = 0;
    int barY = 30;
    int barWidth = SCREEN_WIDTH - 1;
    int barHeight = 10;
    display.drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);

    // 进度条填充
    int fillWidth = (barWidth - 2) * progress / 100;
    display.fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, SSD1306_WHITE);

    // 进度百分比
    display.setCursor(SCREEN_WIDTH / 2 - 10, 50);
    display.printf("%d%%", progress);

    display.display();
}

// ==================== 显示错误 ====================
void displayError(const char* errorMsg) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // 错误标题
    display.setCursor(0, 0);
    display.println("Error:");

    // 错误信息
    display.setCursor(0, 16);
    display.println(errorMsg);

    display.display();

    ERROR_PRINTF("[OLED] Error: %s\n", errorMsg);
}

// ==================== 显示识别结果 ====================
void displayResult(const char* garbageType, float confidence) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // 标题
    display.setCursor(0, 0);
    display.println("Result:");

    // 分类类型
    display.setCursor(0, 16);
    display.setTextSize(2);  // 大字体
    display.println(garbageType);

    // 置信度
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.printf("Confidence: %.1f%%", confidence * 100);

    display.display();
}

// ==================== 显示状态信息（统一格式） ====================
void displayStatus(const char* title, const char* status, bool success) {
    display.clearDisplay();

    // 标题
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(title);

    // 分隔线
    display.drawFastHLine(0, 10, SCREEN_WIDTH, SSD1306_WHITE);

    // 状态文本（大字体）
    display.setTextSize(2);
    display.setCursor(0, 25);
    display.print(status);

    // 成功标记
    if (success) {
        display.setTextSize(2);
        display.setCursor(100, 45);
        display.print("OK");
    }

    display.display();
}

// ==================== 显示网络状态 ====================
void displayNetworkStatus() {
    // 底部状态栏
    display.drawFastHLine(0, 54, SCREEN_WIDTH, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 56);

    // WiFi状态
    if (WiFi.status() == WL_CONNECTED) {
        display.print("WiFi: OK");
    } else {
        display.print("WiFi: --");
    }

    // MQTT状态
    display.setCursor(70, 56);
    if (mqttClient.connected()) {
        display.print("MQTT: OK");
    } else {
        display.print("MQTT: --");
    }

    display.display();
}
