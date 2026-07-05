# ESP32-S3-CAM 智能垃圾分类识别系统 — 系统实现总览

## 1. 项目背景与目标

本项目设计并实现了一个基于 ESP32-S3-CAM 的嵌入式智能垃圾分类识别系统。系统以 ESP32-S3 微控制器为核心，集成 OV3660 摄像头进行图像采集，通过 WiFi 将图片上传至华为云 OBS 对象存储，利用阿里云 DashScope（Qwen 视觉大模型）进行 AI 图像识别，最终驱动舵机模拟垃圾桶分类动作，同时通过华为云 IoTDA 平台实现远程命令下发和状态上报。

### 核心指标

| 指标 | 说明 |
|------|------|
| 分类类型 | 二分类：可回收垃圾 / 其他垃圾 |
| 图像分辨率 | VGA (640×480) JPEG |
| 端到端延迟 | 约 5-10 秒（含上传+AI识别+执行） |
| 控制方式 | 华为云 IoTDA MQTT 远程触发 |
| 云存储 | 华为云 OBS（AWS Signature V4 认证） |
| AI 模型 | 阿里云 DashScope Qwen3.5-Flash（视觉语言模型） |

---

## 2. 系统总体架构

### 2.1 架构分层

系统采用**四层架构**设计，自上而下分为业务协调层、功能模块层、硬件抽象层和云服务层：

```
┌─────────────────────────────────────────────────────┐
│                  业务协调层 (main.ino)                │
│         状态机调度 · 模块协调 · 异常处理              │
├─────────────────────────────────────────────────────┤
│                    功能模块层                         │
│  ┌─────────┐ ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │ network │ │  camera  │ │  servo   │ │display  │ │
│  │   WiFi  │ │ OV3660   │ │  PWM控制 │ │ OLED   │ │
│  └─────────┘ └──────────┘ └──────────┘ └─────────┘ │
├─────────────────────────────────────────────────────┤
│                    云服务层                           │
│  ┌─────────┐ ┌──────────┐ ┌──────────┐              │
│  │  iotda  │ │   obs    │ │    ai    │              │
│  │ IoTDA   │ │ OBS存储  │ │ AI识别   │              │
│  └─────────┘ └──────────┘ └──────────┘              │
├─────────────────────────────────────────────────────┤
│                    硬件抽象层                         │
│  ESP32-S3 · OV3660 · SSD1306 · WS2812 · Servo       │
└─────────────────────────────────────────────────────┘
```

### 2.2 数据流架构

系统核心数据流经过 6 个阶段，形成完整的端到端链路：

```
微信小程序 / IoTDA控制台
       │ (MQTT capture命令)
       ▼
┌─────────────────┐
│  STATE_IDLE     │ ◄── 等待触发
└───────┬─────────┘
        │ captureTriggered = true
        ▼
┌─────────────────┐
│  STATE_CAPTURE  │ OV3660拍摄 → 帧缓冲区(PSRAM)
└───────┬─────────┘
        │ JPEG数据 ~30-60KB
        ▼
┌─────────────────┐
│  STATE_UPLOAD   │ AWS SigV4签名 → PUT上传 → OBS Images/
│                 │ 生成预签名GET URL (有效期600s)
└───────┬─────────┘
        │ 预签名URL
        ▼
┌─────────────────┐
│ STATE_RECOGNIZE │ POST → DashScope API → JSON解析
└───────┬─────────┘
        │ type + confidence
        ▼
┌─────────────────┐
│ STATE_EXECUTE   │ 舵机摆动 → IoTDA上报结果
└───────┬─────────┘
        │
        ▼
    返回 STATE_IDLE
```

---

## 3. 状态机设计

### 3.1 设计理念

系统采用**有限状态机（FSM）**作为核心调度框架。状态机将所有业务逻辑封装在独立的状态处理函数中，通过状态转移实现流程编排。这种设计带来以下优势：

- **清晰的流程控制**：每个状态职责单一，状态转移条件明确
- **易于调试**：当前状态可观测（OLED + 串口输出），转移历史可追踪
- **错误隔离**：任何状态出错统一进入 ERROR 状态处理，避免级联故障
- **易于扩展**：添加新状态只需增加枚举值和对应的 case 分支

### 3.2 状态定义

```cpp
// include/constants.h
enum SystemState {
    STATE_IDLE,       // 空闲等待
    STATE_CAPTURE,    // 拍照采集
    STATE_UPLOAD,     // 上传OBS + 生成预签名URL
    STATE_RECOGNIZE,  // AI识别
    STATE_EXECUTE,    // 执行舵机动作
    STATE_ERROR       // 错误处理
};
```

### 3.3 状态转移图

```
              ┌──────────────────────────────────┐
              │                                  │
              ▼                                  │
         STATE_IDLE ──(触发)──► STATE_CAPTURE    │
              ▲                    │             │
              │               (成功)│ (失败)      │
              │                    ▼       ▼      │
              │              STATE_UPLOAD  STATE_ERROR
              │                 │         ▲      │
              │            (成功)│ (失败)   │      │
              │                 ▼       ▼  │      │
              │           STATE_RECOGNIZE──┘      │
              │              │         ▲          │
              │         (成功)│ (失败)   │          │
              │              ▼       ▼  │          │
              │         STATE_EXECUTE───┘          │
              │              │                     │
              └──────────────┘                     │
                                                   │
              STATE_ERROR ──(5秒后)──► STATE_IDLE  │
              └──────────────────────────────────┘
```

### 3.4 主循环实现

主循环（`loop()`）以 100ms 为周期运行，包含四个层次：

```cpp
// src/main.ino
void loop() {
    esp_task_wdt_reset();     // 1. 看门狗喂狗
    checkAndReconnectWiFi();  // 2. 网络维护层（WiFi重连）
    handleMQTTLoop();         //    MQTT消息处理
    stateMachineLoop();       // 3. 业务逻辑层（状态机）
    checkAndReportStatus();   // 4. 定时状态上报（60秒周期）
    delay(100);
}
```

**设计要点**：
- **看门狗喂狗放在首位**：防止业务流程阻塞导致系统复位
- **网络维护与业务解耦**：WiFi/MQTT 维护独立于状态机，任何状态下都有网络保障
- **定时上报仅在 IDLE 状态触发**：避免在业务流程中干扰 MQTT 通信
- **100ms 循环周期**：平衡了响应速度和 CPU 占用

---

## 4. 初始化流程

系统启动遵循严格的初始化顺序，每一步失败都会阻止后续初始化并进入错误状态：

```
setup()
  ├─ Serial.begin(115200)
  ├─ esp_task_wdt_init(30s)        // 配置看门狗
  ├─ initOLED()                    // 1. OLED（最早初始化，用于后续显示）
  ├─ initLED() → LED_BLUE          // 2. LED（蓝色=启动中）
  ├─ initWiFi()                    // 3. WiFi（网络基础，后续云服务依赖）
  │   ├─ 多WiFi模式：扫描→按优先级匹配→连接→NTP同步
  │   └─ 单WiFi模式：直接连接→NTP同步
  ├─ initCamera()                  // 4. 摄像头（OV3660 + PSRAM检测）
  ├─ initServo()                   // 5. 舵机（PWM初始化，回中90°）
  ├─ initIoTDA()                   // 6. IoTDA（MQTT连接+订阅命令主题）
  ├─ initOBS()                     // 7. OBS（验证WiFi连接状态）
  ├─ initAI()                      // 8. AI（验证WiFi连接状态）
  └─ STATE_IDLE → LED_GREEN       // 9. 就绪
```

**初始化顺序的设计考量**：
1. OLED 最早初始化——确保任何时候都能向用户显示状态
2. WiFi 在摄像头之前——WiFi 初始化较慢（可能数秒），先启动可以并行等待
3. IoTDA 在 OBS/AI 之前——IoTDA 是控制入口，需要优先就绪
4. 每个模块的 `init()` 函数负责自身错误处理，setup 只判断返回值决定是否继续

---

## 5. 模块依赖关系

```
                    ┌─────────────┐
                    │  main.ino   │  (业务协调)
                    └──┬──┬──┬──┬┘
                       │  │  │  │
          ┌────────────┼──┼──┼──┼────────────┐
          │            │  │  │  │            │
          ▼            ▼  ▼  ▼  ▼            ▼
    ┌─────────┐  ┌──────────────────┐  ┌─────────┐
    │ network │  │ display / led    │  │  servo  │
    │ (WiFi)  │  │ (OLED / WS2812)  │  │ (PWM)   │
    └────┬────┘  └──────────────────┘  └─────────┘
         │
    ┌────┴────────────────┐
    │                     │
    ▼                     ▼
┌─────────┐          ┌─────────┐
│  iotda  │          │   obs   │──► 华为云OBS
│ (MQTT)  │          │ (HTTP)  │
└─────────┘          └────┬────┘
      │                   │
      │              ┌────┴────┐
      │              │   ai    │──► DashScope API
      │              │ (HTTP)  │
      │              └─────────┘
      │
 华为云IoTDA
```

**依赖说明**：
- `network` 是底层依赖，`iotda`、`obs`、`ai` 都依赖 WiFi 连接
- `display` 和 `servo` 相对独立，仅被 `main.ino` 调用
- `obs` 的输出（预签名URL）是 `ai` 的输入
- `ai` 的输出（识别结果）是 `servo` 的输入
- `iotda` 通过全局变量 `captureTriggered` 与 `main.ino` 通信

---

## 6. 全局数据结构

```cpp
// src/main.ino - 全局变量
SystemState currentState = STATE_IDLE;   // 当前状态
ErrorCode lastError = ERROR_NONE;        // 最近错误码
unsigned long stateStartTime = 0;        // 当前状态开始时间

// 状态机流转数据
bool captureTriggered = false;           // 拍照触发标志（iotda.cpp设置）
camera_fb_t* photoBuffer = NULL;         // 照片帧缓冲区指针
char objectName[64] = "";                // OBS对象名称
char objectUrl[256] = "";                // OBS对象URL（用于结果上报）
char presignedImageUrl[768] = "";        // 预签名URL（传给AI识别）
char timestamp[32] = "";                 // 时间戳
GarbageType garbageType = GARBAGE_UNKNOWN;  // 识别结果类型
float confidence = 0.0;                  // 识别置信度

unsigned long lastStatusReportTime = 0;  // 上次状态上报时间
```

**设计考量**：
- 数据在状态间传递采用全局变量方式（Arduino 环境限制，避免复杂对象传递）
- `photoBuffer` 需要显式释放，在 UPLOAD 成功或 ERROR 状态下清理
- `presignedImageUrl` 长度 768 字节，可容纳完整的预签名 URL（含签名参数）
- `captureTriggered` 为跨模块通信变量，在 `iotda.cpp` 中通过 `extern` 声明写入

---

## 7. 错误处理机制

### 7.1 错误码体系

```cpp
// include/constants.h
enum ErrorCode {
    ERROR_NONE = 0,
    ERROR_CAMERA_INIT,      // 摄像头初始化失败
    ERROR_CAMERA_CAPTURE,   // 拍照失败
    ERROR_SERVO_INIT,       // 舵机初始化失败
    ERROR_WIFI_CONNECT,     // WiFi连接失败
    ERROR_MQTT_CONNECT,     // MQTT连接失败
    ERROR_OBS_UPLOAD,       // OBS上传失败
    ERROR_OBS_AUTH,         // OBS认证失败
    ERROR_AI_REQUEST,       // AI请求失败
    ERROR_AI_TIMEOUT,       // AI请求超时
    ERROR_AI_RESPONSE,      // AI响应解析失败
    ERROR_INVALID_RESULT    // 无效识别结果
};
```

### 7.2 错误处理流程

1. 任何模块函数返回失败 → 设置 `lastError` → 状态切换到 `STATE_ERROR`
2. `STATE_ERROR` 处理：
   - 设置 LED 为红色
   - OLED 显示错误信息
   - 通过 `reportError()` 上报 IoTDA
   - 清理 `photoBuffer`（如果存在）
   - 等待 5 秒后返回 `STATE_IDLE`
3. 错误不传播——每个状态只检测自己的操作结果，通过状态变量传递错误信息

### 7.3 WiFi 断连处理

WiFi 具有特殊的容错处理，独立于状态机主流程：

```cpp
void checkAndReconnectWiFi() {
    if (!checkWiFiConnection()) {
        displayMessage("Network Status", "WiFi Disconnected", "Reconnecting...");
        if (reconnectWiFi()) {
            displayMessage("Network Status", "WiFi Connected", "OK");
        } else {
            updateWiFiRetryDelay();  // 指数退避：5s→10s→20s→40s→60s
        }
    }
}
```

---

## 8. 关键技术决策

### 8.1 为何采用 OBS 预签名 URL 方案而非 Base64 直传？

| 维度 | Base64 直传 | OBS 预签名 URL（采用） |
|------|------------|----------------------|
| 请求体大小 | ~80KB（Base64编码后） | ~2KB（仅URL） |
| 内存占用 | 高（需同时持有原始+Base64） | 低（上传后即可释放） |
| 传输可靠性 | 差（大body易超时） | 好（HTTP PUT 可靠上传） |
| AI请求延迟 | 高（传输大body） | 低（仅传输URL文本） |
| OBS桶安全性 | 无需OBS | 桶可保持私有 |

**结论**：图片先上传 OBS 再生成预签名 URL 给 AI 的方案，显著降低了 AI 请求的传输负担和失败率，同时不牺牲 OBS 桶的安全性。

### 8.2 为何采用状态机而非线性流程？

- 线性流程（`capture(); upload(); recognize(); execute();`）无法优雅处理中间失败
- 状态机天然支持错误恢复，且每个状态可独立超时和重试
- 状态机配合 `loop()` 是非阻塞的，不会阻塞 MQTT 消息接收和看门狗喂狗

### 8.3 关于舵机数量的调整

原始设计为双舵机（每个垃圾桶一个），最终简化为单舵机方案：
- 可回收垃圾 → 向右摆动 135°
- 其他垃圾 → 向左摆动 45°
- 中心位置 90° 为初始/关闭状态
- 节省一个 GPIO 和一个舵机硬件成本

---

## 9. 文件组织

```
src/
├── main.ino          # 主程序：初始化 + 状态机 + 主循环
├── network.cpp       # WiFi：连接管理、多网络、NTP
├── camera.cpp        # 摄像头：OV3660驱动、拍照
├── display.cpp       # OLED：SSD1306显示
├── led.cpp           # LED：WS2812/外部LED控制
├── servo.cpp         # 舵机：单舵机PWM控制
├── iotda.cpp         # IoTDA：MQTT通信、命令处理
├── obs.cpp           # OBS：AWS SigV4上传、预签名URL
└── ai.cpp            # AI：DashScope API调用

include/
├── config.h          # 硬件引脚、时序参数
├── constants.h       # 枚举定义、调试宏、MQTT格式
├── secrets.h         # 敏感配置（WiFi密码、云服务密钥）
└── *.h               # 各模块头文件
```

---

## 10. 模块实现详解

各模块的详细实现说明请参阅以下文档：

| 模块 | 文档 | 核心内容 |
|------|------|---------|
| 主程序 & 状态机 | [01_state_machine.md](modules/01_state_machine.md) | 初始化流程、状态转移、主循环逻辑 |
| WiFi 网络 | [02_network.md](modules/02_network.md) | 多WiFi选择、NTP同步、重连机制 |
| 摄像头 | [03_camera.md](modules/03_camera.md) | OV3660驱动、PSRAM帧缓冲、传感器调优 |
| 显示 & LED | [04_display_led.md](modules/04_display_led.md) | OLED多行显示、WS2812色彩控制 |
| 舵机控制 | [05_servo.md](modules/05_servo.md) | PWM舵机控制、垃圾分类动作 |
| IoTDA 通信 | [06_iotda.md](modules/06_iotda.md) | MQTT协议、命令处理、属性上报 |
| OBS 存储 | [07_obs.md](modules/07_obs.md) | AWS SigV4签名、虚拟主机上传、预签名URL |
| AI 识别 | [08_ai.md](modules/08_ai.md) | DashScope API、提示词设计、结果解析 |

---

**文档版本**：1.0
**创建日期**：2026-04-26
