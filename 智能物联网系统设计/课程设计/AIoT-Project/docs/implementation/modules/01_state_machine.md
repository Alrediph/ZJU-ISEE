# 模块一：主程序与状态机 (main.ino)

## 1. 模块定位

`main.ino` 是系统的**业务协调核心**，不实现具体功能，而是负责：
- 系统启动时的模块初始化编排
- 运行时状态机的调度与转移
- 异常处理与资源清理
- 定时任务的触发

文件位置：`src/main.ino`（357行）

---

## 2. 全局数据结构

### 2.1 状态机变量

```cpp
SystemState currentState = STATE_IDLE;   // 当前状态，初始为空闲
ErrorCode lastError = ERROR_NONE;        // 最近一次错误码
unsigned long stateStartTime = 0;        // 进入当前状态的时间戳
```

`currentState` 是状态机的核心变量，`loop()` 中每 100ms 调用 `stateMachineLoop()` 检查一次，根据当前状态执行相应逻辑。

### 2.2 流程数据

```cpp
bool captureTriggered = false;           // iotda.cpp 通过 extern 写入
camera_fb_t* photoBuffer = NULL;         // 照片帧缓冲区（PSRAM中）
char objectName[64] = "";                // OBS对象名，如 "Images/garbage_123456.jpg"
char objectUrl[256] = "";                // OBS对象完整URL
char presignedImageUrl[768] = "";        // 预签名下载URL（传给AI）
char timestamp[32] = "";                 // 毫秒时间戳
GarbageType garbageType = GARBAGE_UNKNOWN;  // 识别结果类型
float confidence = 0.0;                  // 置信度 (0.0-1.0)
```

**内存分析**：这些全局变量合计约 1.2KB，常驻 DRAM。`photoBuffer` 是指针，实际帧数据在 PSRAM 中（约 30-60KB 的 JPEG）。

### 2.3 定时控制

```cpp
unsigned long lastStatusReportTime = 0;  // 上次上报时间，用于60秒定时
```

---

## 3. 系统初始化流程

### 3.1 setup() 设计原则

初始化遵循**快速失败（fail-fast）**原则：任一模块初始化失败，立即停止后续初始化，设置错误状态并阻塞。

```cpp
void setup() {
    Serial.begin(115200);
    esp_task_wdt_init(WDT_TIMEOUT, true);  // 30秒看门狗
    esp_task_wdt_add(NULL);

    // 阶段1：显示和指示（确保用户能看到状态）
    initOLED();    // 失败 → 死循环（用户无法看到错误信息）
    initLED();     // 失败 → 死循环
    setLEDColor(LED_BLUE);  // 蓝色 = 启动中

    // 阶段2：网络基础
    if (!initWiFi()) {               // WiFi连接
        lastError = ERROR_WIFI_CONNECT;
        currentState = STATE_ERROR;
        return;  // 注意：Arduino中setup()的return不会停止，需依赖状态机
    }

    // 阶段3：硬件外设
    if (!initCamera()) { ... }       // 摄像头
    if (!initServo())  { ... }       // 舵机

    // 阶段4：云服务
    if (!initIoTDA()) { ... }        // IoTDA
    if (!initOBS())  { ... }        // OBS
    if (!initAI())   { ... }        // AI

    // 就绪
    setLEDColor(LED_GREEN);
    currentState = STATE_IDLE;
}
```

**为什么 OLED/LED 初始化失败用死循环？**
这两个模块是用户感知系统状态的唯一途径。如果它们都失败了，用户无法知道系统状态，因此选择死循环（配合看门狗自动复位），而不是继续运行。

**为什么后续模块失败用 STATE_ERROR 而非死循环？**
后续模块失败时 OLED 已经可用，用户可以看到错误信息。状态机进入 ERROR 状态后会上报 IoTDA 并等待 5 秒后尝试恢复。

### 3.2 模块间初始化顺序依赖

```
OLED ──► LED ──► WiFi ──► Camera ──► Servo ──► IoTDA ──► OBS ──► AI
 │        │        │         │         │         │        │       │
显示基础  状态指示  网络基础   图像采集   动作执行  远程控制  云存储   AI服务
```

- WiFi 必须在 IoTDA/OBS/AI 之前（它们依赖网络）
- Camera 和 Servo 可以互换（无依赖关系）
- IoTDA 在 OBS/AI 之前是因为它是主控通道

---

## 4. 主循环设计

```cpp
void loop() {
    esp_task_wdt_reset();      // 喂狗（必须在30秒内至少调用一次）
    checkAndReconnectWiFi();   // WiFi自动重连维护
    handleMQTTLoop();          // MQTT消息接收与保活
    stateMachineLoop();        // 状态机核心调度
    checkAndReportStatus();    // 定时状态上报（60秒间隔）
    delay(100);                // 100ms 循环周期
}
```

### 4.1 为什么是 100ms？

- **响应延迟**：100ms 对于手动触发（IoTDA 命令）足够快
- **WiFi/MQTT 保活**：MQTT keepalive 为 120 秒，100ms 循环确保及时处理 ping
- **看门狗**：30 秒超时，100ms 周期完全安全
- **CPU 占用**：`delay(100)` 释放 CPU，降低功耗

### 4.2 checkAndReconnectWiFi() — WiFi 维护

```cpp
void checkAndReconnectWiFi() {
    if (!checkWiFiConnection()) {
        // 断开时显示状态
        displayMessage("Network Status", "WiFi Disconnected", "Reconnecting...");
        if (reconnectWiFi()) {
            displayMessage("Network Status", "WiFi Connected", "OK");
        } else {
            updateWiFiRetryDelay();  // 指数退避：5→10→20→40→60秒
        }
    }
}
```

此函数独立于状态机运行——即使 WiFi 在拍照中途断开，也会尝试重连，让后续 UPLOAD 状态有机会恢复。

### 4.3 checkAndReportStatus() — 定时上报

```cpp
void checkAndReportStatus() {
    if (millis() - lastStatusReportTime >= STATUS_REPORT_INTERVAL) {  // 60秒
        lastStatusReportTime = millis();
        if (currentState == STATE_IDLE) {  // 仅在空闲时上报
            reportStatus("在线");
            displayNetworkStatus();  // 更新OLED底部状态栏
        }
    }
}
```

限制在 `STATE_IDLE` 上报的原因：避免在业务流程中竞争 MQTT 连接，也避免干扰识别结果的上报。

---

## 5. 状态机实现详解

### 5.1 STATE_IDLE — 空闲等待

```cpp
case STATE_IDLE:
    if (captureTriggered) {
        captureTriggered = false;         // 清除触发标志
        currentState = STATE_CAPTURE;      // 状态转移
        stateStartTime = millis();         // 记录时间
        setLEDColor(LED_BLUE);            // 蓝色=工作中
        displayMessage("Capturing...", "Please Wait", "");
    }
    break;
```

**关键设计**：
- `captureTriggered = false` 必须在转移前清除，防止重复触发
- `stateStartTime` 记录转移时间，可用于超时检测（当前未使用，预留扩展）
- LED 颜色变化给用户明确的视觉反馈

### 5.2 STATE_CAPTURE — 拍照

```cpp
case STATE_CAPTURE: {
    photoBuffer = capturePhoto();  // 调用camera.cpp

    if (photoBuffer == NULL) {
        lastError = ERROR_CAMERA_CAPTURE;
        currentState = STATE_ERROR;
    } else {
        // 成功：打印大小，转移到上传状态
        INFO_PRINTF("拍照成功，大小: %d 字节\n", photoBuffer->len);
        currentState = STATE_UPLOAD;
        stateStartTime = millis();
        displayMessage("Uploading...", "Please Wait", "");
    }
    break;
}
```

**为什么使用大括号包裹？** C++ 中 case 内部声明变量（如 `photoBuffer = capturePhoto()` 不需要，但如果有局部变量则需要）。这里使用大括号是防御性编程，也便于阅读。

### 5.3 STATE_UPLOAD — 上传与预签名

这是最复杂的状态，包含两个子步骤：

```cpp
case STATE_UPLOAD: {
    // 生成时间戳
    snprintf(timestamp, sizeof(timestamp), "%lu", millis());

    // 步骤1：上传到OBS
    if (uploadToOBS(photoBuffer->buf, photoBuffer->len, objectName, objectUrl)) {

        // 步骤2：生成预签名URL（用于AI访问）
        if (!generatePresignedGetUrl(objectName, presignedImageUrl,
                                      sizeof(presignedImageUrl),
                                      OBS_PRESIGNED_URL_EXPIRES)) {
            lastError = ERROR_OBS_UPLOAD;
            releasePhoto(photoBuffer);
            photoBuffer = NULL;
            currentState = STATE_ERROR;
            break;
        }

        // 上传成功，释放照片缓冲区
        releasePhoto(photoBuffer);
        photoBuffer = NULL;

        currentState = STATE_RECOGNIZE;
        displayMessage("Recognizing...", "Please Wait", "");
    } else {
        lastError = ERROR_OBS_UPLOAD;
        releasePhoto(photoBuffer);
        photoBuffer = NULL;
        currentState = STATE_ERROR;
    }
    break;
}
```

**资源管理要点**：
- `photoBuffer` 在每个分支中都确保释放（成功路径、预签名失败路径、上传失败路径）
- 预签名 URL 有效期 `OBS_PRESIGNED_URL_EXPIRES` 为 600 秒，足够 AI 请求使用
- `objectName` 和 `objectUrl` 保留到上报阶段，供 IoTDA 上报使用

### 5.4 STATE_RECOGNIZE — AI 识别

```cpp
case STATE_RECOGNIZE: {
    if (recognizeGarbageByUrl(presignedImageUrl, garbageType, confidence)) {
        INFO_PRINTF("识别成功: %s (%.2f%%)\n",
                    GARBAGE_NAMES[garbageType], confidence * 100);

        // 上报结果（包含稳定的objectUrl，供后续查询）
        reportResult(garbageType, confidence, objectName, objectUrl, timestamp);

        currentState = STATE_EXECUTE;
        displayMessage("Result:", GARBAGE_NAMES[garbageType], "");
    } else {
        lastError = ERROR_AI_RESPONSE;
        currentState = STATE_ERROR;
    }
    break;
}
```

**设计细节**：
- 识别结果在进入 EXECUTE 状态前上报——即使后续舵机故障，结果也已记录
- `reportResult()` 上报的是稳定的 `objectUrl`（非预签名URL），用于 IoTDA 规则引擎写入 records.csv

### 5.5 STATE_EXECUTE — 执行动作

```cpp
case STATE_EXECUTE: {
    openBin(garbageType);     // 舵机动作
    reportStatus("完成");     // 上报完成状态
    setLEDColor(LED_GREEN);
    displayMessage("System Ready", "Awaiting Trigger", "");
    currentState = STATE_IDLE;
    break;
}
```

这是唯一没有失败路径的状态——舵机硬件故障在软件层面无法检测，直接返回 IDLE。

### 5.6 STATE_ERROR — 错误处理

```cpp
case STATE_ERROR:
    ERROR_PRINTF("系统错误: %s\n", ERROR_MESSAGES[lastError]);
    setLEDColor(LED_RED);
    displayError(ERROR_MESSAGES[lastError]);
    reportError(lastError);

    // 清理资源
    if (photoBuffer != NULL) {
        releasePhoto(photoBuffer);
        photoBuffer = NULL;
    }

    delay(5000);               // 给用户观察错误的时间
    currentState = STATE_IDLE;
    lastError = ERROR_NONE;    // 清除错误
    setLEDColor(LED_GREEN);
    displayMessage("System Ready", "Awaiting Trigger", "");
    break;
```

**关键行为**：
- `delay(5000)` 是阻塞延时，但在 ERROR 状态下可接受（系统不响应新命令）
- 错误恢复后清除 `lastError`，确保旧错误不影响后续判断
- IoTDA 上报错误信息，远端可感知故障

---

## 6. 跨模块通信机制

### 6.1 iotda → main：命令触发

```cpp
// iotda.cpp 中
extern bool captureTriggered;  // 声明外部变量

// MQTT 回调中
if (strcmp(command, "capture") == 0) {
    captureTriggered = true;   // 设置触发标志
}
```

这是最简单的跨模块通信方式——通过 `extern` 共享全局变量。

### 6.2 main → 各模块：函数调用

状态机通过直接调用各模块的公共接口函数完成操作，返回值指示成功/失败：

```cpp
photoBuffer = capturePhoto();                    // camera.cpp
uploadToOBS(...);                                 // obs.cpp
recognizeGarbageByUrl(...);                       // ai.cpp
openBin(garbageType);                             // servo.cpp
reportResult(...); reportStatus(...);             // iotda.cpp
```

### 6.3 main → display/led：状态指示

各状态下更新 OLED 和 LED，提供用户可感知的系统状态：

```cpp
setLEDColor(LED_BLUE);     // 忙碌
setLEDColor(LED_GREEN);    // 空闲/成功
setLEDColor(LED_RED);      // 错误
displayMessage(line1, line2, line3);  // 3行文本显示
```

---

## 7. 设计总结

| 设计要素 | 实现方式 |
|---------|---------|
| 调度框架 | 有限状态机（6状态），100ms循环轮询 |
| 初始化策略 | Fail-fast，OLED最早，WiFi优先于云服务 |
| 错误处理 | 统一ERROR状态，5秒后自动恢复 |
| 资源管理 | photoBuffer 在每个分支显式释放 |
| 跨模块通信 | extern全局变量 + 函数返回值 |
| 定时任务 | millis() 非阻塞定时，60秒上报一次 |
| 系统可靠性 | 看门狗 + WiFi自动重连 + 指数退避 |
