/**
 * OLED Test Program
 * Tests all OLED display functions
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "display.h"
#include "constants.h"
#include "config.h"

// Test step delay (milliseconds)
#define TEST_STEP_DELAY 2000

// Stub WiFiClient and PubSubClient for standalone test
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(1000);  // Wait for serial monitor

    Serial.println();
    Serial.println("[TEST] OLED Test Start");
    Serial.println("=====================================");

    // Step 1: Initialize OLED
    Serial.println("[STEP] Initialize OLED...");
    if (initOLED()) {
        Serial.println("[OK] OLED initialized");
    } else {
        Serial.println("[FAIL] OLED initialization failed");
        Serial.println("[ERROR] Check I2C connections and address");
        return;
    }
    delay(TEST_STEP_DELAY);

    // Step 2: Test displayMessage() - 3 lines
    Serial.println("\n[STEP] Testing displayMessage()...");
    Serial.println("[INFO] Displaying 3-line message");
    displayMessage("Line 1: Hello", "Line 2: OLED Test", "Line 3: Success!");
    Serial.println("[OK] 3-line message displayed (observe OLED)");
    delay(TEST_STEP_DELAY);

    // Step 3: Test displayTitleContent()
    Serial.println("\n[STEP] Testing displayTitleContent()...");
    Serial.println("[INFO] Displaying title and content");
    displayTitleContent("System Status", "All modules OK");
    Serial.println("[OK] Title/Content displayed (observe OLED)");
    delay(TEST_STEP_DELAY);

    // Step 4: Test displayProgress() - multiple values
    Serial.println("\n[STEP] Testing displayProgress()...");

    Serial.println("[INFO] Progress: 0%");
    displayProgress("Uploading...", 0);
    delay(500);

    Serial.println("[INFO] Progress: 25%");
    displayProgress("Uploading...", 25);
    delay(500);

    Serial.println("[INFO] Progress: 50%");
    displayProgress("Uploading...", 50);
    delay(500);

    Serial.println("[INFO] Progress: 75%");
    displayProgress("Uploading...", 75);
    delay(500);

    Serial.println("[INFO] Progress: 100%");
    displayProgress("Uploading...", 100);
    Serial.println("[OK] Progress bar test complete (observe OLED)");
    delay(TEST_STEP_DELAY);

    // Step 5: Test displayError()
    Serial.println("\n[STEP] Testing displayError()...");
    Serial.println("[INFO] Displaying error message");
    displayError("Test Error Message");
    Serial.println("[OK] Error message displayed (observe OLED)");
    delay(TEST_STEP_DELAY);

    // Step 6: Test displayResult()
    Serial.println("\n[STEP] Testing displayResult()...");
    Serial.println("[INFO] Displaying recognition result (Recyclable)");
    displayResult("RECYCLABLE", 0.95);
    Serial.println("[OK] Result 1 displayed (observe OLED)");
    delay(TEST_STEP_DELAY);

    Serial.println("[INFO] Displaying recognition result (Other)");
    displayResult("OTHER", 0.87);
    Serial.println("[OK] Result 2 displayed (observe OLED)");
    delay(TEST_STEP_DELAY);

    // Step 7: Test displayStatus()
    Serial.println("\n[STEP] Testing displayStatus()...");
    Serial.println("[INFO] Displaying status (success=true)");
    displayStatus("WiFi", "Connected", true);
    Serial.println("[OK] Status (success) displayed (observe OLED)");
    delay(TEST_STEP_DELAY);

    Serial.println("[INFO] Displaying status (success=false)");
    displayStatus("MQTT", "Disconnected", false);
    Serial.println("[OK] Status (fail) displayed (observe OLED)");
    delay(TEST_STEP_DELAY);

    // Step 8: Test displayNetworkStatus()
    // Note: WiFi is not connected, so it should show "WiFi: --" and "MQTT: --"
    Serial.println("\n[STEP] Testing displayNetworkStatus()...");
    Serial.println("[INFO] Displaying network status (no WiFi connected)");
    displayMessage("Network Status", "Check below:", "");
    delay(500);
    displayNetworkStatus();
    Serial.println("[OK] Network status displayed (observe OLED)");
    Serial.println("[INFO] Expected: WiFi: --, MQTT: -- (no connection)");
    delay(TEST_STEP_DELAY);

    // Test complete
    Serial.println("\n=====================================");
    Serial.println("[TEST] OLED Test Complete");
    Serial.println("[INFO] All display functions tested.");
    Serial.println("[INFO] Please visually verify all displayed content.");

    // Final display
    displayMessage("Test Complete!", "All functions OK", "Ready for use");
}

void loop() {
    // Nothing to do in loop - test runs once in setup
    delay(1000);
}
