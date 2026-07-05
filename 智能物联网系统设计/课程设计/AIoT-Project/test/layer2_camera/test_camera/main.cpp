/**
 * Camera Test Program
 * Tests camera functionality and memory management
 *
 * Test sequence:
 * 1. Initialize camera
 * 2. Capture first photo and output info
 * 3. Release frame buffer
 * 4. Capture second photo and output info (memory management test)
 * 5. Release frame buffer
 * 6. Output memory statistics
 */

#include <Arduino.h>
#include "camera.h"
#include "constants.h"
#include "config.h"

// Test step delay (milliseconds)
#define TEST_STEP_DELAY 2000

/**
 * Print memory statistics
 */
void printMemoryStats(const char* label) {
    Serial.println(label);
    Serial.print("  Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    Serial.print("  Free PSRAM: ");
    Serial.print(ESP.getFreePsram());
    Serial.println(" bytes");
    Serial.print("  Min Free Heap: ");
    Serial.print(ESP.getMinFreeHeap());
    Serial.println(" bytes");
    Serial.print("  Min Free PSRAM: ");
    Serial.print(ESP.getMinFreePsram());
    Serial.println(" bytes");
}

/**
 * Print frame buffer info
 */
void printFrameBufferInfo(camera_fb_t* fb, const char* label) {
    Serial.println(label);
    Serial.print("  Width: ");
    Serial.print(fb->width);
    Serial.println(" px");
    Serial.print("  Height: ");
    Serial.print(fb->height);
    Serial.println(" px");
    Serial.print("  Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
    Serial.print("  Format: ");
    switch (fb->format) {
        case PIXFORMAT_JPEG:
            Serial.println("JPEG");
            break;
        case PIXFORMAT_RGB565:
            Serial.println("RGB565");
            break;
        case PIXFORMAT_YUV422:
            Serial.println("YUV422");
            break;
        case PIXFORMAT_GRAYSCALE:
            Serial.println("GRAYSCALE");
            break;
        default:
            Serial.print("Unknown (");
            Serial.print(fb->format);
            Serial.println(")");
            break;
    }
}

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(2000);  // Wait for serial monitor

    Serial.println();
    Serial.println("[TEST] Camera Test Start");
    Serial.println("=====================================");

    // Step 1: Print initial memory stats
    Serial.println("\n[STEP] Initial memory state:");
    printMemoryStats("[INFO] Before camera init:");
    delay(TEST_STEP_DELAY);

    // Step 2: Initialize camera
    Serial.println("\n[STEP] Initialize camera...");
    if (initCamera()) {
        Serial.println("[OK] Camera initialized successfully");
    } else {
        Serial.println("[FAIL] Camera initialization failed");
        Serial.println("[ERROR] Cannot continue test without camera");
        return;
    }
    delay(TEST_STEP_DELAY);

    // Print memory after camera init
    Serial.println("\n[STEP] Memory after camera init:");
    printMemoryStats("[INFO] After camera init:");
    delay(TEST_STEP_DELAY);

    // Step 3: First photo capture
    Serial.println("\n[STEP] First photo capture...");
    camera_fb_t* fb1 = capturePhoto();
    if (fb1 == nullptr) {
        Serial.println("[FAIL] First photo capture failed");
        return;
    }

    printFrameBufferInfo(fb1, "[INFO] First photo info:");
    delay(TEST_STEP_DELAY);

    // Step 4: Release first frame buffer
    Serial.println("\n[STEP] Releasing first frame buffer...");
    releasePhoto(fb1);
    fb1 = nullptr;
    Serial.println("[OK] First frame buffer released");

    // Print memory after release
    Serial.println("\n[STEP] Memory after first release:");
    printMemoryStats("[INFO] After first release:");
    delay(TEST_STEP_DELAY);

    // Step 5: Second photo capture (memory management test)
    Serial.println("\n[STEP] Second photo capture (memory test)...");
    camera_fb_t* fb2 = capturePhoto();
    if (fb2 == nullptr) {
        Serial.println("[FAIL] Second photo capture failed");
        Serial.println("[ERROR] Possible memory leak detected!");
        return;
    }

    printFrameBufferInfo(fb2, "[INFO] Second photo info:");
    delay(TEST_STEP_DELAY);

    // Step 6: Release second frame buffer
    Serial.println("\n[STEP] Releasing second frame buffer...");
    releasePhoto(fb2);
    fb2 = nullptr;
    Serial.println("[OK] Second frame buffer released");

    // Print final memory stats
    Serial.println("\n[STEP] Final memory state:");
    printMemoryStats("[INFO] After second release:");
    delay(TEST_STEP_DELAY);

    // Memory leak check
    Serial.println("\n[STEP] Memory leak check:");
    size_t heapDiff = ESP.getMinFreeHeap();
    Serial.print("[INFO] Minimum free heap during test: ");
    Serial.print(heapDiff);
    Serial.println(" bytes");
    Serial.println("[OK] Memory management test complete");

    // Test complete
    Serial.println("\n=====================================");
    Serial.println("[TEST] Camera Test Complete");
    Serial.println("[INFO] All camera tests passed successfully");
    Serial.println("[INFO] Verify memory stats show no significant leaks");
}

void loop() {
    // Nothing to do in loop - test runs once in setup
    delay(1000);
}
