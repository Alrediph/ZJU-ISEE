/**
 * LED控制模块
 */

#include <FastLED.h>
#include "config.h"
#include "constants.h"

// WS2812 LED数组
CRGB leds[1];

// ==================== 初始化LED ====================
bool initLED() {
    if (USE_ONBOARD_RGB_LED) {
        // 使用板载WS2812 RGB LED
        FastLED.addLeds<WS2812, RGB_LED_PIN, GRB>(leds, 1);
        FastLED.setBrightness(50);  // 设置亮度
        INFO_PRINTLN("LED初始化成功 (WS2812)");
    } else {
        // 使用外部LED（GPIO控制）
        pinMode(LED_RED_PIN, OUTPUT);
        pinMode(LED_GREEN_PIN, OUTPUT);
        pinMode(LED_BLUE_PIN, OUTPUT);
        digitalWrite(LED_RED_PIN, LED_STATE_OFF);
        digitalWrite(LED_GREEN_PIN, LED_STATE_OFF);
        digitalWrite(LED_BLUE_PIN, LED_STATE_OFF);
        INFO_PRINTLN("LED初始化成功 (外部LED)");
    }

    return true;
}

// ==================== 设置LED颜色 ====================
void setLEDColor(LEDColor color) {
    if (USE_ONBOARD_RGB_LED) {
        // WS2812 RGB LED
        switch (color) {
            case LED_OFF:
                leds[0] = CRGB::Black;
                break;
            case LED_RED:
                leds[0] = CRGB::Red;
                break;
            case LED_GREEN:
                leds[0] = CRGB::Green;
                break;
            case LED_BLUE:
                leds[0] = CRGB::Blue;
                break;
            case LED_YELLOW:
                leds[0] = CRGB::Yellow;
                break;
            case LED_PURPLE:
                leds[0] = CRGB::Purple;
                break;
            case LED_WHITE:
                leds[0] = CRGB::White;
                break;
        }
        FastLED.show();
    } else {
        // 外部LED
        digitalWrite(LED_RED_PIN, LED_STATE_OFF);
        digitalWrite(LED_GREEN_PIN, LED_STATE_OFF);
        digitalWrite(LED_BLUE_PIN, LED_STATE_OFF);

        switch (color) {
            case LED_OFF:
                // 全部关闭
                break;
            case LED_RED:
                digitalWrite(LED_RED_PIN, LED_STATE_ON);
                break;
            case LED_GREEN:
                digitalWrite(LED_GREEN_PIN, LED_STATE_ON);
                break;
            case LED_BLUE:
                digitalWrite(LED_BLUE_PIN, LED_STATE_ON);
                break;
            case LED_YELLOW:
                digitalWrite(LED_RED_PIN, LED_STATE_ON);
                digitalWrite(LED_GREEN_PIN, LED_STATE_ON);
                break;
            case LED_PURPLE:
                digitalWrite(LED_RED_PIN, LED_STATE_ON);
                digitalWrite(LED_BLUE_PIN, LED_STATE_ON);
                break;
            case LED_WHITE:
                digitalWrite(LED_RED_PIN, LED_STATE_ON);
                digitalWrite(LED_GREEN_PIN, LED_STATE_ON);
                digitalWrite(LED_BLUE_PIN, LED_STATE_ON);
                break;
        }
    }

    DEBUG_PRINTF("[LED] 设置颜色: %d\n", color);
}

// ==================== LED闪烁 ====================
void blinkLED(LEDColor color, int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        setLEDColor(color);
        delay(delayMs);
        setLEDColor(LED_OFF);
        delay(delayMs);
    }
}

// ==================== LED呼吸效果 ====================
void breatheLED(LEDColor color, int durationMs) {
    if (!USE_ONBOARD_RGB_LED) {
        // 外部LED不支持呼吸效果
        setLEDColor(color);
        delay(durationMs);
        setLEDColor(LED_OFF);
        return;
    }

    // WS2812呼吸效果
    int steps = 50;
    int stepDelay = durationMs / (steps * 2);

    CRGB baseColor;
    switch (color) {
        case LED_RED: baseColor = CRGB::Red; break;
        case LED_GREEN: baseColor = CRGB::Green; break;
        case LED_BLUE: baseColor = CRGB::Blue; break;
        case LED_YELLOW: baseColor = CRGB::Yellow; break;
        case LED_PURPLE: baseColor = CRGB::Purple; break;
        case LED_WHITE: baseColor = CRGB::White; break;
        default: baseColor = CRGB::Black; break;
    }

    // 渐亮
    for (int i = 0; i <= steps; i++) {
        leds[0] = baseColor;
        leds[0].nscale8(i * 255 / steps);
        FastLED.show();
        delay(stepDelay);
    }

    // 渐暗
    for (int i = steps; i >= 0; i--) {
        leds[0] = baseColor;
        leds[0].nscale8(i * 255 / steps);
        FastLED.show();
        delay(stepDelay);
    }
}
