/**
 * 舵机控制模块实现
 */

#include <Arduino.h>
#include <ESP32Servo.h>
#include "servo.h"
#include "config.h"
#include "display.h"
#include "led.h"

// 舵机对象
Servo servo;  // 单舵机对象

// ==================== 初始化舵机 ====================
bool initServo() {
    INFO_PRINTLN("初始化舵机...");

    // 分配定时器资源
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    // 设置舵机频率
    servo.setPeriodHertz(SERVO_FREQ);

    // 绑定引脚（使用标准脉冲宽度：500μs-2500μs对应0°-180°）
    // 注意：attach()返回PWM通道号（非0表示成功），某些GPIO可能不在官方支持列表但仍可工作
    int channel = servo.attach(SERVO_PIN, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
    if (channel == 0) {
        ERROR_PRINTLN("舵机绑定失败 - GPIO可能不支持PWM");
        // 不立即返回false，继续尝试初始化
        // return false;
    }

    // 初始化为中间位置
    servo.write(SERVO_ANGLE_CENTER);

    INFO_PRINTF("舵机初始化成功 (GPIO %d, PWM通道: %d, 初始位置: %d度)\n", SERVO_PIN, channel, SERVO_ANGLE_CENTER);
    return true;
}

// ==================== 打开指定垃圾桶 ====================
void openBin(GarbageType type) {
    INFO_PRINTF("打开垃圾桶: %s\n", GARBAGE_NAMES[type]);

    // 显示状态
    displayMessage("Processing", GARBAGE_NAMES[type], "Opening...");

    // 设置LED为处理中（蓝色）
    setLEDColor(LED_BLUE);

    if (type == GARBAGE_RECYCLABLE) {
        // 可回收垃圾：向右摆动
        INFO_PRINTLN("可回收垃圾 - 舵机向右摆动");
        servo.write(SERVO_ANGLE_RIGHT);  // 135°
        delay(500);  // 打开动作时间

        // 保持开启状态
        delay(SERVO_HOLD_TIME);

        // 回位
        servo.write(SERVO_ANGLE_CENTER);  // 90°
        delay(500);  // 回位动作时间

    } else if (type == GARBAGE_OTHER) {
        // 其他垃圾：向左摆动
        INFO_PRINTLN("其他垃圾 - 舵机向左摆动");
        servo.write(SERVO_ANGLE_LEFT);  // 45°
        delay(500);  // 打开动作时间

        // 保持开启状态
        delay(SERVO_HOLD_TIME);

        // 回位
        servo.write(SERVO_ANGLE_CENTER);  // 90°
        delay(500);  // 回位动作时间
    }

    // 设置LED为成功（绿色）
    setLEDColor(LED_GREEN);
    displayMessage("Processing Complete", GARBAGE_NAMES[type], "Categorized");

    INFO_PRINTLN("垃圾桶动作完成");
}

// ==================== 关闭所有垃圾桶 ====================
void closeAllBins() {
    DEBUG_PRINTLN("舵机复位到中间位置...");

    servo.write(SERVO_ANGLE_CENTER);  // 90°

    delay(500);  // 等待动作完成
}

// ==================== 测试舵机动作 ====================
void testServoAction() {
    INFO_PRINTLN("开始测试舵机动作...");

    // 测试向右摆动（可回收垃圾）
    INFO_PRINTLN("测试可回收垃圾 - 向右摆动");
    displayMessage("Testing Servo", "Recyclable", "Right Turn");
    setLEDColor(LED_YELLOW);

    servo.write(SERVO_ANGLE_RIGHT);  // 135°
    delay(1000);
    servo.write(SERVO_ANGLE_CENTER);  // 90°
    delay(1000);

    // 测试向左摆动（其他垃圾）
    INFO_PRINTLN("测试其他垃圾 - 向左摆动");
    displayMessage("Testing Servo", "Other Waste", "Left Turn");
    setLEDColor(LED_PURPLE);

    servo.write(SERVO_ANGLE_LEFT);  // 45°
    delay(1000);
    servo.write(SERVO_ANGLE_CENTER);  // 90°
    delay(1000);

    // 恢复初始状态
    setLEDColor(LED_OFF);
    displayMessage("Testing Complete", "Servo Normal", "");

    INFO_PRINTLN("舵机测试完成");
}
