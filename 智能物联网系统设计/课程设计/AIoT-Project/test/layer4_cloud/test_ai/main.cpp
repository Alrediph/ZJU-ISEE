/**
 * AI Recognition Test Program (OBS Presigned URL Route)
 * 上传图片到华为云OBS，生成预签名URL，再调用AI识别。
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "network.h"
#include "camera.h"
#include "obs.h"
#include "ai.h"
#include "secrets.h"
#include "config.h"
#include "constants.h"

#define TEST_STEP_DELAY 1000

void printMemoryStats(const char* label) {
    Serial.println(label);
    Serial.print("  Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    Serial.print("  Free PSRAM: ");
    Serial.print(ESP.getFreePsram());
    Serial.println(" bytes");
}

void printFrameBufferInfo(camera_fb_t* fb, const char* label) {
    Serial.println(label);
    Serial.print("  Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
    Serial.print("  Resolution: ");
    Serial.print(fb->width);
    Serial.print("x");
    Serial.println(fb->height);
    Serial.print("  Format: ");
    Serial.println(fb->format == PIXFORMAT_JPEG ? "JPEG" : "Other");
}

bool debugPresignedUrlGet(const char* presignedUrl) {
    Serial.println("\n[DEBUG] 验证预签名URL可访问性");
    Serial.println("-------------------------------------");

    HTTPClient http;
    http.begin(presignedUrl);
    http.setConnectTimeout(15000);
    http.setTimeout(20000);

    int httpCode = http.GET();
    Serial.printf("[INFO] GET响应码: %d\n", httpCode);

    if (httpCode <= 0) {
        Serial.printf("[ERROR] GET失败: %s\n", http.errorToString(httpCode).c_str());
        http.end();
        return false;
    }

    Serial.printf("[INFO] Content-Type: %s\n", http.header("Content-Type").c_str());
    Serial.printf("[INFO] Content-Length: %s\n", http.header("Content-Length").c_str());

    WiFiClient* stream = http.getStreamPtr();
    size_t totalRead = 0;
    uint8_t buffer[256];
    unsigned long startTime = millis();

    while (http.connected() && millis() - startTime < 5000) {
        size_t available = stream->available();
        if (available == 0) {
            delay(20);
            continue;
        }

        size_t toRead = available > sizeof(buffer) ? sizeof(buffer) : available;
        int bytesRead = stream->readBytes(buffer, toRead);
        if (bytesRead <= 0) {
            break;
        }
        totalRead += bytesRead;
    }

    Serial.printf("[INFO] 已读取数据: %u 字节\n", static_cast<unsigned int>(totalRead));
    http.end();

    if (httpCode != 200) {
        Serial.println("[ERROR] 预签名URL返回非200状态码");
        return false;
    }

    if (totalRead == 0) {
        Serial.println("[ERROR] 预签名URL未返回任何图片数据");
        return false;
    }

    Serial.println("[OK] 预签名URL可访问");
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(5000);

    Serial.println();
    Serial.println("[TEST] AI Recognition Test Start (OBS URL Route)");
    Serial.println("=====================================");

    Serial.println("\n[STEP 1] Initialize WiFi");
    Serial.println("-------------------------------------");
    initWiFiMode();
    if (!connectWiFi()) {
        Serial.println("[FAIL] WiFi connection failed");
        Serial.println("[TEST] AI Test Aborted");
        return;
    }
    Serial.println("[OK] WiFi connected");
    Serial.printf("[INFO] IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[INFO] RSSI: %d dBm\n", WiFi.RSSI());
    delay(TEST_STEP_DELAY);

    Serial.println("\n[STEP 2] Sync NTP Time");
    Serial.println("-------------------------------------");
    if (!syncNTPTime()) {
        Serial.println("[FAIL] NTP time sync failed");
        Serial.println("[TEST] AI Test Aborted");
        return;
    }
    time_t now = time(nullptr);
    Serial.printf("[OK] Current UTC time: %s", ctime(&now));
    delay(TEST_STEP_DELAY);

    Serial.println("\n[STEP 3] Initialize Camera / OBS / AI");
    Serial.println("-------------------------------------");
    if (!initCamera()) {
        Serial.println("[FAIL] Camera initialization failed");
        Serial.println("[TEST] AI Test Aborted");
        return;
    }
    if (!initOBS()) {
        Serial.println("[FAIL] OBS initialization failed");
        Serial.println("[TEST] AI Test Aborted");
        return;
    }
    if (!initAI()) {
        Serial.println("[FAIL] AI initialization failed");
        Serial.println("[TEST] AI Test Aborted");
        return;
    }
    Serial.println("[OK] Modules initialized");
    delay(TEST_STEP_DELAY);

    Serial.println("\n[STEP 4] Capture Photo");
    Serial.println("-------------------------------------");
    camera_fb_t* photoBuffer = capturePhoto();
    if (photoBuffer == nullptr) {
        Serial.println("[FAIL] Photo capture failed");
        Serial.println("[TEST] AI Test Aborted");
        return;
    }
    printFrameBufferInfo(photoBuffer, "[OK] Photo captured:");
    printMemoryStats("[INFO] Memory state:");
    delay(TEST_STEP_DELAY);

    Serial.println("\n[STEP 5] Upload Photo To OBS");
    Serial.println("-------------------------------------");
    char objectName[64] = "";
    char objectUrl[256] = "";
    if (!uploadToOBS(photoBuffer->buf, photoBuffer->len, objectName, objectUrl)) {
        Serial.println("[FAIL] OBS upload failed");
        releasePhoto(photoBuffer);
        Serial.println("[TEST] AI Test Aborted");
        return;
    }
    Serial.println("[OK] OBS upload successful");
    Serial.printf("[INFO] Object Name: %s\n", objectName);
    Serial.printf("[INFO] Object URL: %s\n", objectUrl);
    delay(TEST_STEP_DELAY);

    Serial.println("\n[STEP 6] Generate Presigned GET URL");
    Serial.println("-------------------------------------");
    char presignedUrl[768] = "";
    if (!generatePresignedGetUrl(objectName, presignedUrl, sizeof(presignedUrl), OBS_PRESIGNED_URL_EXPIRES)) {
        Serial.println("[FAIL] Presigned URL generation failed");
        releasePhoto(photoBuffer);
        Serial.println("[TEST] AI Test Aborted");
        return;
    }
    Serial.println("[OK] Presigned URL generated");
    Serial.printf("[INFO] URL length: %u\n", static_cast<unsigned int>(strlen(presignedUrl)));
    Serial.printf("[INFO] URL preview: %.120s...\n", presignedUrl);
    delay(TEST_STEP_DELAY);

    Serial.println("\n[STEP 7] Validate Presigned URL");
    Serial.println("-------------------------------------");
    bool urlAccessible = debugPresignedUrlGet(presignedUrl);
    delay(TEST_STEP_DELAY);

    Serial.println("\n[STEP 8] Recognize Garbage By URL");
    Serial.println("-------------------------------------");
    GarbageType type = GARBAGE_UNKNOWN;
    float confidence = 0.0f;
    bool aiSuccess = recognizeGarbageByUrl(presignedUrl, type, confidence);

    if (aiSuccess) {
        Serial.println("[OK] AI recognition successful");
        Serial.printf("[INFO] Type: %s\n", GARBAGE_NAMES[type]);
        Serial.printf("[INFO] Confidence: %.2f%%\n", confidence * 100);
    } else {
        Serial.println("[FAIL] AI recognition failed");
    }

    Serial.println("\n[STEP 9] Release Resources");
    Serial.println("-------------------------------------");
    releasePhoto(photoBuffer);
    Serial.println("[OK] Photo buffer released");
    printMemoryStats("[INFO] Memory after release:");

    Serial.println("\n=====================================");
    Serial.println("[TEST] AI Recognition Test Complete");
    Serial.println("\n[SUMMARY]");
    Serial.printf("  - Presigned URL reachable: %s\n", urlAccessible ? "YES" : "NO");
    Serial.printf("  - AI recognition succeeded: %s\n", aiSuccess ? "YES" : "NO");
}

void loop() {
    delay(1000);
}
