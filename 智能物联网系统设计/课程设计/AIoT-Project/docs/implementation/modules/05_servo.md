# 模块五：舵机控制模块 (servo.cpp)

## 1. 模块定位

`servo.cpp` 负责通过 PWM 信号控制单舵机，将垃圾分类结果转化为物理动作——舵机向左或向右摆动，模拟对应垃圾桶的开启。

文件位置：`src/servo.cpp`（127行） | 头文件：`include/servo.h`

---

## 2. 硬件配置

### 2.1 PWM 参数

```cpp
// include/config.h
#define SERVO_PIN            1    // PWM 信号引脚
#define SERVO_ANGLE_CENTER   90   // 中间位置（初始/关闭）
#define SERVO_ANGLE_LEFT     45   // 向左 = 其他垃圾
#define SERVO_ANGLE_RIGHT    135  // 向右 = 可回收垃圾
#define SERVO_FREQ           50   // 50Hz (标准舵机周期 20ms)
#define SERVO_RESOLUTION     16   // 16位分辨率 (65535 steps)
#define SERVO_PULSE_MIN      500  // 0° 对应 500μs
#define SERVO_PULSE_MAX      2500 // 180° 对应 2500μs
#define SERVO_HOLD_TIME      1000 // 保持时间 1秒
```

### 2.2 角度-脉冲对应关系

标准舵机的 PWM 控制原理：

```
脉冲宽度    角度
  500μs  →   0°
 1000μs  →  45°  (SERVO_ANGLE_LEFT — 其他垃圾)
 1500μs  →  90°  (SERVO_ANGLE_CENTER — 关闭/初始)
 2000μs  →  135° (SERVO_ANGLE_RIGHT — 可回收垃圾)
 2500μs  →  180°
```

50Hz 频率下，周期为 20ms（20000μs），脉宽 500-2500μs 对应 2.5%-12.5% 占空比。

### 2.3 GPIO 引脚选择

ESP32-S3 上可用 GPIO 受限（摄像头占用大量引脚），选择 GPIO 1 的原因：
- GPIO 19/20 用于 USB，不可用
- GPIO 2 是板载 LED
- GPIO 1 是少数可自由使用的引脚

---

## 3. 初始化

```cpp
bool initServo() {
    // 分配PWM定时器（ESP32Servo库需要手动分配）
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    // 设置PWM频率
    servo.setPeriodHertz(SERVO_FREQ);  // 50Hz

    // 绑定引脚并设置脉冲范围
    // attach() 返回 PWM 通道号（非0=成功）
    int channel = servo.attach(SERVO_PIN, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
    if (channel == 0) {
        ERROR_PRINTLN("舵机绑定失败 - GPIO可能不支持PWM");
        // 不返回 false，继续尝试
        // 原因：某些GPIO不在官方支持列表但实际可用
    }

    // 初始位置：中间
    servo.write(SERVO_ANGLE_CENTER);  // 90°
    return true;
}
```

**关键设计细节**：
- `ESP32PWM::allocateTimer()` 分配 ESP32-S3 的 4 个硬件定时器之一给舵机
- `attach()` 的返回值检查被注释掉了——因为 GPIO 1 不在 ESP32Servo 库的官方支持列表中，但硬件上可以工作
- 初始化后立即将舵机置于中间位置（90°），避免上电时舵机乱动

---

## 4. 垃圾分类动作

### 4.1 openBin() — 核心控制函数

```cpp
void openBin(GarbageType type) {
    displayMessage("Processing", GARBAGE_NAMES[type], "Opening...");
    setLEDColor(LED_BLUE);

    if (type == GARBAGE_RECYCLABLE) {
        // 可回收垃圾 → 右摆
        servo.write(SERVO_ANGLE_RIGHT);  // 90° → 135°
        delay(500);                       // 等待摆到位

        delay(SERVO_HOLD_TIME);           // 保持 1000ms

        servo.write(SERVO_ANGLE_CENTER);  // 135° → 90°
        delay(500);                       // 等待回位

    } else if (type == GARBAGE_OTHER) {
        // 其他垃圾 → 左摆
        servo.write(SERVO_ANGLE_LEFT);    // 90° → 45°
        delay(500);

        delay(SERVO_HOLD_TIME);           // 保持 1000ms

        servo.write(SERVO_ANGLE_CENTER);  // 45° → 90°
        delay(500);
    }

    setLEDColor(LED_GREEN);
    displayMessage("Processing Complete", GARBAGE_NAMES[type], "Categorized");
}
```

### 4.2 动作时序

以可回收垃圾为例，完整时序：

```
T+0ms    舵机在90°（中间）
T+0ms    servo.write(135°) — 开始向右转动
T+500ms  舵机到达135° — 动作完成
T+1500ms 保持结束（SERVO_HOLD_TIME = 1000ms）
T+1500ms servo.write(90°) — 开始回中
T+2000ms 舵机回到90° — 回位完成

总耗时：2000ms
```

**delay() 的作用**：
- 第一个 `delay(500)`：等待舵机从当前角度转到目标角度。500ms 对 45° 的转动范围足够
- `delay(SERVO_HOLD_TIME)`：保持打开状态 1 秒，模拟 "打开垃圾桶 → 投入垃圾 → 关闭" 的时间感
- 第二个 `delay(500)`：等待舵机回到中间位置

### 4.3 为何使用 blocking delay？

舵机动作是完整流程的最后一步，之前的所有操作（上传、AI 识别）已完成。此时没有并发任务需要处理，使用 `delay()` 是最简捷的实现方式。如果需要非阻塞的舵机控制，可以使用定时器+状态机，但这里没必要引入额外复杂度。

---

## 5. 其他辅助函数

```cpp
void closeAllBins() {
    servo.write(SERVO_ANGLE_CENTER);  // 统一回到90°
    delay(500);
}

void testServoAction() {
    // 测试序列：右摆 → 回中 → 左摆 → 回中
    servo.write(SERVO_ANGLE_RIGHT);  delay(1000);
    servo.write(SERVO_ANGLE_CENTER); delay(1000);
    servo.write(SERVO_ANGLE_LEFT);   delay(1000);
    servo.write(SERVO_ANGLE_CENTER); delay(1000);
}
```

`testServoAction()` 用于独立测试舵机硬件，在测试程序 `test_servo` 中调用。

---

## 6. 与其他模块的接口

| 函数 | 调用者 | 用途 |
|------|--------|------|
| `initServo()` | main.ino setup() | PWM 初始化 |
| `openBin(type)` | main.ino STATE_EXECUTE | 执行分类动作 |
| `closeAllBins()` | 测试程序 | 手动复位 |
| `testServoAction()` | 测试程序 | 硬件验证 |

`openBin()` 内部调用 `displayMessage()` 和 `setLEDColor()`，提供视觉反馈。

---

## 7. 关键设计决策

1. **单舵机替代双舵机**：原始设计计划使用 2 个舵机分别控制两个垃圾桶，简化为单舵机通过左右摆动区分类型。减少硬件复杂性且节省 GPIO
2. **角度选择**：45°/90°/135° 提供 45° 的摆动范围，动作明显可见，同时避免舵机接近 0°/180° 的机械极限
3. **PWM 周期 50Hz**：这是标准模拟舵机的频率，大多数 SG90/MG996R 等舵机都使用此频率
4. **不检测舵机硬件故障**：软件层面无法获取舵机的实际位置反馈（普通舵机无编码器），因此 `openBin()` 没有返回值，假设总是成功
5. **blocking delay 在此处可接受**：舵机动作是流程的最后一步，不影响系统响应
