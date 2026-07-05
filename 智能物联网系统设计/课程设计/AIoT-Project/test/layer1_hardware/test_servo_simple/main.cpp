/**
 * 最简单的舵机测试程序
 * 直接测试舵机和GPIO，不依赖其他模块
 *
 * 测试步骤：
 * 1. 尝试绑定GPIO 14
 * 2. 如果失败，依次尝试其他GPIO（1, 21, 45, 47）
 * 3. 舵机来回摆动测试
 */

#include <Arduino.h>
#include <ESP32Servo.h>

// 测试的GPIO列表
const int testPins[] = {14, 1, 21, 45, 47};
const int numPins = 5;
int angle = 0;  // 舵机角度变量

// 舵机对象
Servo testServo;

void setup(){
    Serial.begin(115200);
    delay(5000);  // 等待串口监视器打开

    testServo.attach(14);
};

void loop() {
    for(int i = 0; i < 10; i++) {
        angle += 10;
        testServo.write(angle);
        delay(500);
    }
    angle = 0;
    // 测试完成，舵机保持在中间位置
    testServo.write(90);
    delay(1000);
}
