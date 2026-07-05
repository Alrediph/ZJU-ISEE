/**
 * LED Test Program
 * Tests LED colors, blinking, and breathing effects
 */

#include <Arduino.h>
#include "led.h"
#include "constants.h"
#include "config.h"

// Test step delay (milliseconds)
#define TEST_STEP_DELAY 1000

// Test state
bool testCompleted = false;

void runLEDTest() {
    Serial.println();
    Serial.println("[TEST] LED Test Start");
    Serial.println("=====================================");

    // Step 1: Initialize LED
    Serial.println("[STEP] Initialize LED...");
    if (initLED()) {
        Serial.println("[OK] LED initialized");
    } else {
        Serial.println("[FAIL] LED initialization failed");
        return;
    }
    delay(TEST_STEP_DELAY);

    // Step 2: Test all colors
    Serial.println("\n[STEP] Testing LED colors...");

    // RED
    Serial.println("[STEP] Set LED to RED...");
    setLEDColor(LED_RED);
    Serial.println("[OK] LED is RED (observe)");
    delay(TEST_STEP_DELAY);

    // GREEN
    Serial.println("[STEP] Set LED to GREEN...");
    setLEDColor(LED_GREEN);
    Serial.println("[OK] LED is GREEN (observe)");
    delay(TEST_STEP_DELAY);

    // BLUE
    Serial.println("[STEP] Set LED to BLUE...");
    setLEDColor(LED_BLUE);
    Serial.println("[OK] LED is BLUE (observe)");
    delay(TEST_STEP_DELAY);

    // YELLOW
    Serial.println("[STEP] Set LED to YELLOW...");
    setLEDColor(LED_YELLOW);
    Serial.println("[OK] LED is YELLOW (observe)");
    delay(TEST_STEP_DELAY);

    // PURPLE
    Serial.println("[STEP] Set LED to PURPLE...");
    setLEDColor(LED_PURPLE);
    Serial.println("[OK] LED is PURPLE (observe)");
    delay(TEST_STEP_DELAY);

    // WHITE
    Serial.println("[STEP] Set LED to WHITE...");
    setLEDColor(LED_WHITE);
    Serial.println("[OK] LED is WHITE (observe)");
    delay(TEST_STEP_DELAY);

    // Step 3: Test blinking effect
    Serial.println("\n[STEP] Testing blink effect (RED, 3 times, 500ms interval)...");
    blinkLED(LED_RED, 3, 500);
    Serial.println("[OK] Blink effect complete");
    delay(TEST_STEP_DELAY);

    // Step 4: Test breathing effect
    Serial.println("\n[STEP] Testing breathe effect (BLUE, 5 seconds)...");
    breatheLED(LED_BLUE, 5000);
    Serial.println("[OK] Breathe effect complete");

    // Test complete
    Serial.println("\n=====================================");
    Serial.println("[TEST] LED Test Complete");
    Serial.println("[INFO] All tests passed. Observe LED behavior visually.");
    Serial.println("[INFO] Press RESET button to run test again.");

    testCompleted = true;
}

void setup() {
    // Initialize Serial
    Serial.begin(115200);

    // Wait for serial monitor to connect (with timeout)
    Serial.println("\n[INFO] Waiting for serial monitor...");
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime < 5000)) {
        delay(10);
    }

    if (!Serial) {
        // If serial monitor not connected, continue anyway (LED test can run standalone)
        delay(1000);
    } else {
        Serial.println("[INFO] Serial monitor connected");
    }

    delay(1000);  // Additional delay for user to see the message

    // Run test
    runLEDTest();
}

void loop() {
    // After test completes, show periodic status
    if (testCompleted) {
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 5000) {  // Print every 5 seconds
            lastPrint = millis();
            Serial.println("\n[INFO] Test completed. Press RESET to run again.");
        }
    }
    delay(100);
}
