/**
 * Servo Test Program
 * Tests servo initialization and basic movements
 *
 * Tests:
 * - Initialize servos
 * - Test servo 1 (recyclable bin) movement
 * - Test servo 2 (other waste bin) movement
 *
 * Verification: Manual observation of servo rotation
 */

#include <Arduino.h>
#include "servo.h"
#include "led.h"
#include "display.h"
#include "constants.h"
#include "config.h"

// Test step delay (milliseconds)
#define TEST_STEP_DELAY 1000
#define SERVO_HOLD_TIME_MS 1000

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(1000);  // Wait for serial monitor

    Serial.println();
    Serial.println("[TEST] Servo Test Start");
    Serial.println("=====================================");

    // Step 1: Initialize LED (required by servo module)
    Serial.println("[STEP] Initialize LED...");
    if (initLED()) {
        Serial.println("[OK] LED initialized");
    } else {
        Serial.println("[FAIL] LED initialization failed");
        return;
    }
    delay(TEST_STEP_DELAY);

    // Step 2: Initialize OLED display (required by servo module)
    Serial.println("[STEP] Initialize OLED display...");
    if (initOLED()) {
        Serial.println("[OK] OLED initialized");
        displayMessage("Servo Test", "Starting...", "");
    } else {
        Serial.println("[WARN] OLED initialization failed (continuing)");
    }
    delay(TEST_STEP_DELAY);

    // Step 3: Initialize Servos
    Serial.println("[STEP] Initialize Servos...");
    if (initServo()) {
        Serial.println("[OK] Servos initialized");
        displayMessage("Servo Test", "Init OK", "GPIO 1");
    } else {
        Serial.println("[FAIL] Servo initialization failed");
        displayMessage("Servo Test", "Init FAIL", "Check wiring");
        return;
    }
    delay(TEST_STEP_DELAY);

    // Step 4: Test Servo - Right Turn (Recyclable bin)
    Serial.println("\n[STEP] Testing Servo - Right Turn (Recyclable bin)...");
    Serial.println("[STEP] Action: Center(90°) -> Right(135°) -> Hold 1s -> Center(90°)");
    displayMessage("Servo Test", "Recyclable", "Right Turn");
    setLEDColor(LED_YELLOW);

    // Test right turn
    openBin(GARBAGE_RECYCLABLE);
    Serial.println("[OK] Right turn test complete (observe rotation)");

    setLEDColor(LED_OFF);
    delay(TEST_STEP_DELAY);

    // Step 5: Test Servo - Left Turn (Other waste bin)
    Serial.println("\n[STEP] Testing Servo - Left Turn (Other waste bin)...");
    Serial.println("[STEP] Action: Center(90°) -> Left(45°) -> Hold 1s -> Center(90°)");
    displayMessage("Servo Test", "Other Waste", "Left Turn");
    setLEDColor(LED_PURPLE);

    // Test left turn
    openBin(GARBAGE_OTHER);
    Serial.println("[OK] Left turn test complete (observe rotation)");

    setLEDColor(LED_OFF);
    delay(TEST_STEP_DELAY);

    // Step 6: Test closeAllBins function
    Serial.println("\n[STEP] Testing closeAllBins() - Reset to center position...");
    displayMessage("Reset Servo", "Moving to center", "90 degrees");
    closeAllBins();
    Serial.println("[OK] Servo reset to center position");
    delay(TEST_STEP_DELAY);

    // Test complete
    Serial.println("\n=====================================");
    Serial.println("[TEST] Servo Test Complete");
    Serial.println("[INFO] All tests passed. Observe servo left/right rotation behavior.");
    displayMessage("Test Complete", "All OK", "Single Servo Mode");
    setLEDColor(LED_GREEN);
}

void loop() {
    // Nothing to do in loop - test runs once in setup
    delay(1000);
}
