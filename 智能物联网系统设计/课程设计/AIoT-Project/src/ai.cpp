/**
 * AI识别服务模块实现
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>  // 使用HTTPClient替代WiFiClientSecure
#include <ArduinoJson.h>
#include <base64.h>  // ESP32内置Base64编码库
#include "ai.h"
#include "config.h"
#include "secrets.h"
#include "constants.h"
#include "display.h"
#include "led.h"

// ==================== 辅助函数声明 ====================
bool parseAIResponse(const String& response, GarbageType& type, float& confidence);
String encodeImageToBase64(const uint8_t* imageData, size_t imageDataSize);
String buildPrompt();
bool performAIRequest(const String& imageUrl, GarbageType& type, float& confidence);
static String buildAIRequestUrl();

// DashScope 的 qwen3.5 系列默认开启思考，板端请求需要显式关闭并放宽等待时间。
static const uint32_t AI_CONNECT_TIMEOUT_MS = 15000;
static const uint16_t AI_RESPONSE_TIMEOUT_MS = 65000;

// ==================== Base64编码函数 ====================
String encodeImageToBase64(const uint8_t* imageData, size_t imageDataSize) {
    INFO_PRINTF("开始Base64编码，图片大小: %d 字节\n", imageDataSize);

    // 使用ESP32内置的base64编码库
    String base64Image = base64::encode(imageData, imageDataSize);

    INFO_PRINTF("Base64编码完成，编码后大小: %d 字节\n", base64Image.length());
    return base64Image;
}
String encodeImageToBase64(const uint8_t* imageData, size_t imageDataSize);

static String buildAIRequestUrl() {
    String base = String(AI_API_ENDPOINT);
    base.trim();

    if (base.endsWith("/chat/completions")) {
        return base;
    }
    if (base.endsWith("/v1")) {
        return base + "/chat/completions";
    }
    if (base.endsWith("/v1/")) {
        return base + "chat/completions";
    }
    if (base.endsWith("/compatible-mode")) {
        return base + "/v1/chat/completions";
    }
    if (base.endsWith("/compatible-mode/")) {
        return base + "v1/chat/completions";
    }
    if (base.endsWith("/")) {
        return base + "chat/completions";
    }
    return base + "/chat/completions";
}

// ==================== 初始化AI识别服务 ====================
bool initAI() {
    INFO_PRINTLN("初始化AI识别服务...");

    // 检查WiFi连接
    if (WiFi.status() != WL_CONNECTED) {
        ERROR_PRINTLN("WiFi未连接，无法初始化AI服务");
        return false;
    }

    INFO_PRINTLN("AI识别服务初始化成功");
    return true;
}

String buildPrompt() {
    return "请识别这张图片中的垃圾类型。这是一个二分类任务：可回收垃圾或其他垃圾。\n\n"
           "可回收垃圾包括：纸张、塑料瓶、玻璃瓶、金属罐、纸箱等。\n"
           "其他垃圾包括：食物残渣、用过的纸巾、陶瓷碎片、烟头等。\n\n"
           "请以JSON格式返回识别结果，格式如下：\n"
           "{\n"
           "  \"type\": \"recyclable\" 或 \"other\",\n"
           "  \"confidence\": 0.0-1.0之间的置信度数值\n"
           "}\n\n"
           "请只返回JSON，不要包含其他文字。\n/no_think";
}

bool performAIRequest(const String& imageUrl, GarbageType& type, float& confidence) {
    HTTPClient http;
    String url = buildAIRequestUrl();

    INFO_PRINTF("API URL: %s\n", url.c_str());
    INFO_PRINTF("图片URL长度: %d 字节\n", imageUrl.length());

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(AI_API_KEY));
    http.addHeader("Connection", "close");
    http.setConnectTimeout(AI_CONNECT_TIMEOUT_MS);
    http.setTimeout(AI_RESPONSE_TIMEOUT_MS);

    INFO_PRINTLN("HTTP连接已建立");

    JsonDocument doc;
    doc["model"] = AI_MODEL;
    doc["messages"][0]["role"] = "user";
    doc["messages"][0]["content"][0]["type"] = "text";
    doc["messages"][0]["content"][0]["text"] = buildPrompt();
    doc["messages"][0]["content"][1]["type"] = "image_url";
    doc["messages"][0]["content"][1]["image_url"]["url"] = imageUrl;
    doc["max_tokens"] = 80;
    doc["enable_thinking"] = false;
    doc["thinking_budget"] = 128;

    String requestBody;
    serializeJson(doc, requestBody);

    INFO_PRINTF("请求体大小: %d 字节\n", requestBody.length());
    INFO_PRINTLN("发送HTTP POST请求...");

    int httpResponseCode = http.POST(requestBody);

    INFO_PRINTF("HTTP响应码: %d\n", httpResponseCode);

    if (httpResponseCode != 200) {
        ERROR_PRINTF("HTTP请求失败，响应码: %d\n", httpResponseCode);

        if (httpResponseCode > 0) {
            String response = http.getString();
            ERROR_PRINTF("响应内容: %s\n", response.c_str());
        } else {
            ERROR_PRINTF("HTTP错误: %s\n", http.errorToString(httpResponseCode).c_str());
        }

        http.end();
        return false;
    }

    String responseBody = http.getString();
    INFO_PRINTF("响应体大小: %d 字节\n", responseBody.length());

    bool success = parseAIResponse(responseBody, type, confidence);

    http.end();
    return success;
}

// ==================== 调用AI识别API ====================
bool recognizeGarbage(const uint8_t* imageData, size_t imageDataSize, GarbageType& type, float& confidence) {
    INFO_PRINTLN("开始AI识别...");
    INFO_PRINTF("图片数据大小: %d 字节\n", imageDataSize);

    String base64Image = encodeImageToBase64(imageData, imageDataSize);
    String imageUrl = "data:image/jpeg;base64," + base64Image;
    INFO_PRINTF("Data URL长度: %d 字节\n", imageUrl.length());

    return performAIRequest(imageUrl, type, confidence);
}

bool recognizeGarbageByUrl(const char* imageUrl, GarbageType& type, float& confidence) {
    if (imageUrl == NULL || strlen(imageUrl) == 0) {
        ERROR_PRINTLN("图片URL无效");
        return false;
    }

    INFO_PRINTLN("开始基于URL的AI识别...");
    return performAIRequest(String(imageUrl), type, confidence);
}

// ==================== 解析AI响应 ====================
bool parseAIResponse(const String& response, GarbageType& type, float& confidence) {
    INFO_PRINTLN("解析AI响应...");

    // 解析JSON响应
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        ERROR_PRINTLN("JSON解析失败");
        ERROR_PRINTF("错误: %s\n", error.c_str());
        return false;
    }

    // 提取AI返回的内容
    const char* content = doc["choices"][0]["message"]["content"];
    if (content == NULL) {
        ERROR_PRINTLN("无法提取AI内容");
        return false;
    }

    INFO_PRINTF("AI内容: %s\n", content);

    // 解析AI返回的JSON内容
    JsonDocument contentDoc;
    error = deserializeJson(contentDoc, content);

    if (error) {
        ERROR_PRINTLN("AI内容JSON解析失败");
        // 尝试从文本中提取类型（容错处理）
        if (strstr(content, "recyclable") != NULL) {
            type = GARBAGE_RECYCLABLE;
            confidence = 0.7;
            INFO_PRINTLN("从容错处理中识别为可回收垃圾");
            return true;
        } else if (strstr(content, "other") != NULL) {
            type = GARBAGE_OTHER;
            confidence = 0.7;
            INFO_PRINTLN("从容错处理中识别为其他垃圾");
            return true;
        }
        return false;
    }

    // 提取类型和置信度
    const char* typeStr = contentDoc["type"];
    confidence = contentDoc["confidence"] | 0.8;

    if (typeStr == NULL) {
        ERROR_PRINTLN("无法提取垃圾类型");
        return false;
    }

    // 转换为枚举类型
    if (strcmp(typeStr, "recyclable") == 0) {
        type = GARBAGE_RECYCLABLE;
    } else if (strcmp(typeStr, "other") == 0) {
        type = GARBAGE_OTHER;
    } else {
        ERROR_PRINTF("未知的垃圾类型: %s\n", typeStr);
        return false;
    }

    INFO_PRINTF("识别结果: %s (%.2f%%)\n", GARBAGE_NAMES[type], confidence * 100);
    return true;
}
