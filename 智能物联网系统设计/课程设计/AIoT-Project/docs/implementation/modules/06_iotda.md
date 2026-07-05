# 模块六：IoTDA 通信模块 (iotda.cpp)

## 1. 模块定位

`iotda.cpp` 实现与华为云 IoTDA（IoT Device Access）平台的 MQTT 通信，是系统的**远程控制入口**和**数据上报出口**。负责接收云端下发的 `capture` 命令触发识别流程，并将设备状态、识别结果和错误信息上报到云端。

文件位置：`src/iotda.cpp`（303行） | 头文件：`include/iotda.h`

---

## 2. MQTT 通信架构

### 2.1 连接模型

```
ESP32-S3 ──WiFi──► 华为云 IoTDA MQTT Broker
                      │
                 ┌────┴────┐
            订阅：命令下发主题
            发布：属性上报主题
```

### 2.2 MQTT 客户端初始化

```cpp
WiFiClient espClient;                    // 底层 TCP 连接
PubSubClient mqttClient(espClient);      // MQTT 协议客户端
```

**PubSubClient 版本**：固定使用 2.7.0 版本（较新版在 ESP32-S3 上存在兼容问题）。

### 2.3 配置参数

```cpp
// include/secrets.h 中定义
#define MQTT_SERVER   "xxx.iot-mqtts.cn-north-4.myhuaweicloud.com"
#define MQTT_PORT     1883                    // 非加密端口
#define CLIENT_ID     "device_id_0_0_2026042001"
#define MQTT_USER     "device_id"
#define MQTT_PASSWORD "device_secret_hash"
```

**CLIENT_ID 格式**：`{device_id}_0_0_{timestamp}`
- `device_id` — 华为云 IoTDA 注册的设备 ID
- `0_0` — 表示使用 MQTT 协议、非加密连接
- `timestamp` — 时间戳（通常为设备注册时间）

### 2.4 MQTT 主题定义

```cpp
// 在 include/secrets.h 中定义
#define MQTT_TOPIC_COMMANDS     // 订阅：接收命令
"$oc/devices/{device_id}/sys/commands/request/id/+"

#define MQTT_TOPIC_CMD_RESPONSE // 发布：命令响应
"$oc/devices/{device_id}/sys/commands/response/request_id="

#define MQTT_TOPIC_REPORT       // 发布：属性上报
"$oc/devices/{device_id}/sys/properties/report"
```

**主题中 `+` 通配符**：订阅时 `+` 匹配任意 request_id，用于接收所有命令。

---

## 3. 连接流程

### 3.1 initIoTDA()

```cpp
bool initIoTDA() {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);  // 注册消息回调

    if (!connectMQTT()) {
        return false;
    }
    return true;
}
```

### 3.2 connectMQTT() — 建立连接

```cpp
bool connectMQTT() {
    if (WiFi.status() != WL_CONNECTED) {
        return false;  // WiFi 是 MQTT 的前提
    }

    // 连接 MQTT Broker（华为云 IoTDA）
    if (mqttClient.connect(CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {

        // 订阅命令下发主题
        mqttClient.subscribe(MQTT_TOPIC_COMMANDS);

        return true;
    } else {
        // 打印详细错误码
        int state = mqttClient.state();
        // -4: TIMEOUT, -3: LOST, -2: FAILED, -1: DISCONNECTED
        // 1: BAD_PROTOCOL, 2: BAD_CLIENT_ID, 4: BAD_CREDENTIALS, 5: UNAUTHORIZED
        return false;
    }
}
```

### 3.3 MQTT 连接维护

```cpp
void handleMQTTLoop() {
    if (mqttClient.connected()) {
        mqttClient.loop();  // 处理 MQTT 保活和接收消息
    } else {
        checkMQTTConnection();  // 尝试重连
    }
}

void checkMQTTConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        return;  // WiFi 断开时无法重连 MQTT
    }

    if (!mqttClient.connected()) {
        if (connectMQTT()) {
            // 重连成功
        }
    }
}
```

---

## 4. 命令处理

### 4.1 mqttCallback() — 消息回调

```cpp
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // ===== 步骤1：安全拷贝（限制最大长度，防止溢出） =====
    if (length >= MAX_PAYLOAD_SIZE) {  // 512字节
        return;  // 过长消息丢弃
    }
    char payloadBuffer[MAX_PAYLOAD_SIZE];
    memcpy(payloadBuffer, payload, length);
    payloadBuffer[length] = '\0';

    // ===== 步骤2：从 topic 中提取 request_id =====
    char requestId[64] = "";
    const char* idStart = strstr(topic, "request_id=");
    if (idStart != NULL) {
        idStart += 11;  // 跳过 "request_id="
        strncpy(requestId, idStart, sizeof(requestId) - 1);
    } else {
        return;  // 无 request_id 则无法响应，必须放弃
    }

    // ===== 步骤3：解析 JSON 命令 =====
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payloadBuffer);

    if (error) {
        // JSON 解析失败也发送响应，避免 IoTDA 等待超时
        sendCommandResponse(requestId);
        return;
    }

    // ===== 步骤4：处理命令 =====
    const char* command = doc["command_name"] | "";

    if (strcmp(command, "capture") == 0) {
        captureTriggered = true;   // 触发主状态机
    } else if (strcmp(command, "test") == 0) {
        displayMessage("测试", "命令接收", "成功");
    }

    // ===== 步骤5：发送命令响应 =====
    sendCommandResponse(requestId);
}
```

### 4.2 命令响应机制

华为云 IoTDA 要求每条命令必须在超时时间（默认 30 秒）内回复响应，否则标记为超时。

```cpp
void sendCommandResponse(const char* requestId) {
    char responseTopic[256];
    snprintf(responseTopic, sizeof(responseTopic),
             "%s%s", MQTT_TOPIC_CMD_RESPONSE, requestId);

    mqttClient.publish(responseTopic, COMMAND_RESPONSE_DATA);
    // COMMAND_RESPONSE_DATA = {"result_code": 0, "response_name": "COMMAND_RESPONSE",
    //                          "paras": {"result": "success"}}
}
```

**关键设计**：即使 JSON 解析失败或命令未知，也会发送响应，避免 IoTDA 平台显示超时。这是从实际调试中总结出的经验（Phase 9 修复）。

### 4.3 跨模块触发

```cpp
// iotda.cpp 顶部
extern bool captureTriggered;  // 引用 main.ino 中定义的全局变量

// 在回调中设置
captureTriggered = true;  // 下一轮 stateMachineLoop() 检测到并启动流程
```

---

## 5. 属性上报

### 5.1 IoTDA 消息格式

```cpp
// include/constants.h
#define SERVICE_ID       "\"GarbageClassification\""
#define MQTT_BODY_FORMAT "{\"services\":[{\"service_id\":" SERVICE_ID ",\"properties\":{%s}}]}"
```

华为云 IoTDA 的属性上报需要按照产品模型中定义的服务 ID 格式化。

### 5.2 reportStatus() — 状态上报

```cpp
void reportStatus(const char* status) {
    if (!mqttClient.connected()) return;

    char properties[128];
    snprintf(properties, sizeof(properties), "\"Status\":\"%s\"", status);

    char jsonBuf[256];
    snprintf(jsonBuf, sizeof(jsonBuf), MQTT_BODY_FORMAT, properties);

    mqttClient.publish(MQTT_TOPIC_REPORT, jsonBuf);
}
```

上报时机：
- **定时**：每 60 秒（仅在 STATE_IDLE 时），上报 "在线"
- **事件**：识别流程完成后，上报 "完成"

### 5.3 reportResult() — 识别结果上报

```cpp
void reportResult(GarbageType type, float confidence,
                  const char* objectName, const char* objectUrl,
                  const char* timestamp) {
    if (!mqttClient.connected()) return;

    // 构造包含完整信息的属性 JSON
    char properties[384];
    snprintf(properties, sizeof(properties),
        "\"Status\":\"识别完成\","
        "\"GarbageType\":\"%s\","
        "\"Confidence\":%.2f,"
        "\"ImageUrl\":\"%s\","
        "\"ObjectName\":\"%s\","
        "\"Timestamp\":\"%s\"",
        GARBAGE_NAMES[type], confidence, objectUrl, objectName, timestamp);

    char jsonBuf[512];
    snprintf(jsonBuf, sizeof(jsonBuf), MQTT_BODY_FORMAT, properties);

    mqttClient.publish(MQTT_TOPIC_REPORT, jsonBuf);
}
```

**上报内容设计**：
- `ImageUrl` 是稳定的 OBS 对象 URL（非预签名URL），IoTDA 规则引擎可将其写入 records.csv
- `ObjectName` 和 `Timestamp` 用于记录关联
- 置信度保留两位小数

### 5.4 reportError() — 错误上报

```cpp
void reportError(ErrorCode error) {
    char properties[128];
    snprintf(properties, sizeof(properties), "\"Error\":\"%s\"", ERROR_MESSAGES[error]);
    // ...
}
```

---

## 6. 与其他模块的接口

| 函数 | 调用者 | 用途 |
|------|--------|------|
| `initIoTDA()` | main.ino setup() | MQTT 初始化连接 |
| `handleMQTTLoop()` | main.ino loop() | MQTT 维护（每100ms） |
| `reportStatus()` | main.ino | 状态属性上报 |
| `reportResult()` | main.ino STATE_RECOGNIZE | 识别结果上报 |
| `reportError()` | main.ino STATE_ERROR | 错误上报 |
| `mqttCallback()` | PubSubClient 库 | MQTT 消息回调（设置 captureTriggered） |

**跨模块数据流**：
- 输出：设置 `captureTriggered = true`（extern 变量）触发主状态机
- 对象引用：`mqttClient` 被 `display.cpp` 通过 `extern` 引用（显示 MQTT 状态）

---

## 7. 关键设计决策

1. **PubSubClient 锁定 2.7.0 版本**：更新版本的 MQTT 库在 ESP32-S3 上存在连接问题
2. **MQTT_MAX_PACKET_SIZE 设为 2048**：默认 256 字节不足以传输属性 JSON（含图片 URL）
3. **命令响应必须在回调中发送**：不能在状态机中异步回复，IoTDA 要求同步响应
4. **JSON 解析失败也发响应**：避免 IoTDA 平台显示命令超时（从实际调试中总结的经验）
5. **安全限制消息长度**：`MAX_PAYLOAD_SIZE = 512` 防止超长消息导致缓冲区溢出
