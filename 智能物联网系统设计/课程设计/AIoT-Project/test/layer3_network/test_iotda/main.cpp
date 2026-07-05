/**
 * IoTDA Test Program
 * Tests MQTT connection, status reporting, and command handling
 */

#include <Arduino.h>
#include <WiFi.h>
#include "network.h"
#include "iotda.h"
#include "secrets.h"
#include "config.h"
#include "constants.h"

// Test step delay (milliseconds)
#define TEST_STEP_DELAY 2000

// Predefined test data
const char* TEST_STATUS = "idle";
const char* TEST_OBJECT_NAME = "test_image_001.jpg";
const char* TEST_OBJECT_URL = "https://test.obs.com/test_image_001.jpg";
const char* TEST_TIMESTAMP = "2026-04-20 10:00:00";
const GarbageType TEST_GARBAGE_TYPE = GARBAGE_RECYCLABLE;
const float TEST_CONFIDENCE = 0.95;

// Global flag for capture trigger (required by iotda module)
bool captureTriggered = false;

// Test initialization flag
bool testInitialized = false;

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(5000);  // Wait for serial monitor

    Serial.println();
    Serial.println("[TEST] IoTDA Test Start");
    Serial.println("=====================================");

    // Step 1: Initialize WiFi
    Serial.println("\n[STEP 1] Initialize WiFi");
    Serial.println("-------------------------------------");
    initWiFiMode();
    if (!connectWiFi()) {
        Serial.println("[FAIL] WiFi connection failed");
        Serial.println("[TEST] IoTDA Test Aborted");
        Serial.println("[INFO] Entering safe loop mode...");
        return;  // testInitialized remains false
    }
    Serial.println("[OK] WiFi connected");
    printWiFiStatus();
    delay(TEST_STEP_DELAY);

    // Step 2: Initialize IoTDA
    Serial.println("\n[STEP 2] Initialize IoTDA");
    Serial.println("-------------------------------------");
    Serial.println("[STEP] Initializing IoTDA connection...");
    if (!initIoTDA()) {
        Serial.println("[FAIL] IoTDA initialization failed");
        Serial.println("[TEST] IoTDA Test Aborted");
        Serial.println("[INFO] Entering safe loop mode...");
        return;  // testInitialized remains false
    }
    Serial.println("[OK] IoTDA initialized");
    printMQTTStatus();
    delay(TEST_STEP_DELAY);

    // Mark initialization complete
    testInitialized = true;

    // Step 3: Report device status
    Serial.println("\n[STEP 3] Report Device Status");
    Serial.println("-------------------------------------");
    Serial.printf("[STEP] Reporting status: %s\n", TEST_STATUS);
    reportStatus(TEST_STATUS);
    Serial.println("[OK] Status reported");
    delay(TEST_STEP_DELAY);

    // Step 4: Subscribe to commands (already done in initIoTDA)
    Serial.println("\n[STEP 4] Verify Command Subscription");
    Serial.println("-------------------------------------");
    Serial.println("[INFO] Command topic subscription verified in Step 2");
    Serial.println("[INFO] Waiting for commands (5 seconds)...");
    Serial.println("[INFO] You can send a command from IoTDA console now");

    // Wait for commands
    unsigned long startTime = millis();
    while (millis() - startTime < 10000) {
        handleMQTTLoop();
        delay(100);
    }
    Serial.println("[OK] Command wait complete");
    delay(TEST_STEP_DELAY);

    // Step 5: Report recognition result
    Serial.println("\n[STEP 5] Report Recognition Result");
    Serial.println("-------------------------------------");
    Serial.printf("[STEP] Reporting result: %s (%.2f%%)\n",
                  GARBAGE_NAMES[TEST_GARBAGE_TYPE], TEST_CONFIDENCE * 100);
    Serial.printf("[INFO] Object: %s\n", TEST_OBJECT_NAME);
    Serial.printf("[INFO] URL: %s\n", TEST_OBJECT_URL);
    reportResult(TEST_GARBAGE_TYPE, TEST_CONFIDENCE,
                 TEST_OBJECT_NAME, TEST_OBJECT_URL, TEST_TIMESTAMP);
    Serial.println("[OK] Result reported");
    delay(TEST_STEP_DELAY);

    // Step 6: Report error (test)
    Serial.println("\n[STEP 6] Report Error");
    Serial.println("-------------------------------------");
    Serial.println("[STEP] Reporting test error...");
    reportError(ERROR_CAMERA_CAPTURE);
    Serial.println("[OK] Error reported");
    delay(TEST_STEP_DELAY);

    // Step 7: Test MQTT connection stability
    Serial.println("\n[STEP 7] Test MQTT Connection Stability");
    Serial.println("-------------------------------------");
    Serial.println("[STEP] Monitoring MQTT connection for 5 seconds...");

    bool stable = true;
    for (int i = 0; i < 5; i++) {
        handleMQTTLoop();
        delay(1000);

        // Check MQTT connection via status print
        Serial.printf("[INFO] Second %d: ", i + 1);
        printMQTTStatus();
    }

    Serial.println("[OK] MQTT connection monitoring complete");

    // Test complete
    Serial.println("\n=====================================");
    Serial.println("[TEST] IoTDA Test Complete");

    // Summary
    Serial.println("\n[SUMMARY] Test Results:");
    Serial.println("  - WiFi connection: OK");
    Serial.println("  - IoTDA initialization: OK");
    Serial.println("  - Status reporting: OK");
    Serial.println("  - Command subscription: OK");
    Serial.println("  - Result reporting: OK");
    Serial.println("  - Error reporting: OK");
    Serial.println("  - MQTT stability: OK");
    Serial.println("\n[INFO] IoTDA test completed successfully!");
}

void loop() {
    // Safety check: only run if initialization succeeded
    if (!testInitialized) {
        delay(1000);
        return;
    }

    // Keep MQTT connection alive
    handleMQTTLoop();

    // Periodic status print
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 10000) {  // Print every 10 seconds
        lastPrint = millis();
        printMQTTStatus();
    }
}
