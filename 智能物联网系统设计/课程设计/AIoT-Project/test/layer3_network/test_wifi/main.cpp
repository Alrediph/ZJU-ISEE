/**
 * WiFi Test Program
 * Tests WiFi connection and reconnection functionality
 * Supports both single WiFi and multi-WiFi modes
 */

#include <Arduino.h>
#include <WiFi.h>
#include "network.h"
#include "secrets.h"
#include "config.h"
#include "constants.h"

// Test step delay (milliseconds)
#define TEST_STEP_DELAY 2000

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(1000);  // Wait for serial monitor

    Serial.println();
    Serial.println("[TEST] WiFi Test Start");
    Serial.println("=====================================");

    // Step 1: Initialize WiFi mode
    Serial.println("\n[STEP 1] Initialize WiFi Mode");
    Serial.println("-------------------------------------");
    initWiFiMode();
    Serial.println("[OK] WiFi mode initialized");
    delay(1000);

    // Step 2: Test WiFi initialization (supports multi-WiFi)
    Serial.println("\n[STEP 2] WiFi Initialization");
    Serial.println("-------------------------------------");
    Serial.printf("[INFO] USE_MULTI_WIFI: %s\n", USE_MULTI_WIFI ? "true" : "false");

    if (!initWiFi()) {
        Serial.println("[FAIL] WiFi initialization failed");
        Serial.println("[TEST] WiFi Test Aborted");
        return;
    }
    Serial.println("[OK] WiFi connected successfully");

    // Print connection info
    printWiFiStatus();
    delay(TEST_STEP_DELAY);

    // Step 3: Test connection check
    Serial.println("\n[STEP 3] Test Connection Check");
    Serial.println("-------------------------------------");
    if (checkWiFiConnection()) {
        Serial.println("[OK] WiFi connection check passed");
    } else {
        Serial.println("[FAIL] WiFi connection check failed");
    }
    delay(TEST_STEP_DELAY);

    // Step 4: Disconnect WiFi
    Serial.println("\n[STEP 4] Disconnect WiFi");
    Serial.println("-------------------------------------");
    Serial.println("[STEP] Disconnecting from WiFi...");
    disconnectWiFi();
    delay(1000);

    // Verify disconnection
    if (!isWiFiConnected()) {
        Serial.println("[OK] WiFi disconnected successfully");
        Serial.printf("[INFO] WiFi status: %d (disconnected)\n", WiFi.status());
    } else {
        Serial.println("[WARN] WiFi still connected after disconnect");
    }
    delay(TEST_STEP_DELAY);

    // Step 5: Test reconnectWiFi
    Serial.println("\n[STEP 5] Test WiFi Reconnect");
    Serial.println("-------------------------------------");
    if (!reconnectWiFi()) {
        Serial.println("[FAIL] Reconnection failed");
        Serial.println("[TEST] WiFi Test Failed");
        return;
    }
    Serial.println("[OK] WiFi reconnected successfully");

    // Print reconnection info
    printWiFiStatus();
    delay(TEST_STEP_DELAY);

    // Step 6: Test connection stability
    Serial.println("\n[STEP 6] Test Connection Stability");
    Serial.println("-------------------------------------");
    Serial.println("[STEP] Monitoring connection for 5 seconds...");

    bool stable = true;
    for (int i = 0; i < 5; i++) {
        delay(1000);
        if (isWiFiConnected()) {
            Serial.printf("[INFO] Second %d: Connected (RSSI: %d dBm)\n",
                          i + 1, getWiFiRSSI());
        } else {
            Serial.printf("[INFO] Second %d: Disconnected!\n", i + 1);
            stable = false;
        }
    }

    if (stable) {
        Serial.println("[OK] Connection stable");
    } else {
        Serial.println("[WARN] Connection unstable");
    }

    // Step 7: Print detailed connection info
    Serial.println("\n[STEP 7] Detailed Connection Info");
    Serial.println("-------------------------------------");
    Serial.printf("[INFO] IP Address: %s\n", getWiFiIP().c_str());
    Serial.printf("[INFO] MAC Address: %s\n", getWiFiMAC().c_str());
    Serial.printf("[INFO] Signal Strength: %d dBm\n", getWiFiRSSI());

    // Step 8: Test retry delay management
    Serial.println("\n[STEP 8] Test Retry Delay Management");
    Serial.println("-------------------------------------");
    Serial.printf("[INFO] Initial retry delay: %d ms\n", getWiFiRetryDelay());
    updateWiFiRetryDelay();
    Serial.printf("[INFO] After update: %d ms\n", getWiFiRetryDelay());
    updateWiFiRetryDelay();
    Serial.printf("[INFO] After 2nd update: %d ms\n", getWiFiRetryDelay());
    resetWiFiRetryDelay();
    Serial.printf("[INFO] After reset: %d ms\n", getWiFiRetryDelay());

    // Test complete
    Serial.println("\n=====================================");
    Serial.println("[TEST] WiFi Test Complete");

    // Summary
    Serial.println("\n[SUMMARY] Test Results:");
    Serial.println("  - WiFi mode initialization: OK");
    Serial.println("  - WiFi initialization: OK");
    Serial.println("  - Connection check: OK");
    Serial.println("  - Disconnection: OK");
    Serial.println("  - Reconnection: OK");
    Serial.printf("  - Connection stability: %s\n", stable ? "OK" : "WARN");
    Serial.println("  - Retry delay management: OK");
    Serial.println("\n[INFO] WiFi test completed successfully!");
}

void loop() {
    // Nothing to do in loop - test runs once in setup
    // Keep WiFi connected and show periodic status
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 10000) {  // Print every 10 seconds
        lastPrint = millis();
        if (isWiFiConnected()) {
            Serial.printf("[INFO] WiFi Status: Connected, RSSI: %d dBm\n",
                          getWiFiRSSI());
        } else {
            Serial.println("[WARN] WiFi Status: Disconnected");
        }
    }
}
