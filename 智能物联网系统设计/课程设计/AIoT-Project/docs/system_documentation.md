# ESP32-S3-CAM 智能垃圾分类识别系统 - 完整说明文档

## 项目概况

这是一个基于 ESP32-S3-CAM 的智能垃圾分类识别系统，用于课程大作业演示。系统实现了从图像采集、云端 AI 识别到硬件控制的完整端到端流程。

### 系统特性

- **二分类系统**：可回收垃圾、其他垃圾
- **云端 AI 识别**：调用 OpenAI 兼容 API 进行图像识别
- **云端存储**：图片和识别记录上传到华为云 OBS
- **远程控制**：通过华为云 IoTDA 实现远程触发识别
- **硬件反馈**：舵机模拟垃圾桶开合，LED 和 OLED 显示状态

---

## 系统架构

### 整体架构

```
微信小程序/控制台
    ↓ (命令下发)
华为云 IoTDA (MQTT)
    ↓ (capture 命令)
ESP32-S3-CAM 设备
    ↓ (上传图片)
华为云 OBS (images/)
    ↓ (预签名URL)
OpenAI 兼容 API
    ↓ (返回识别结果)
ESP32-S3-CAM 设备
    ↓ (上报结果)
华为云 IoTDA
    ↓ (规则引擎数据转发)
华为云 OBS (records/)
    ↓ (显示结果)
微信小程序/控制台
```

### 状态机架构

系统采用状态机驱动架构，核心状态流转：

```
IDLE (空闲)
  ↓ (收到 capture 命令)
CAPTURE (拍照)
  ↓ (拍照成功)
UPLOAD (上传图片到 OBS + 生成预签名URL)
  ↓ (上传成功)
RECOGNIZE (AI 识别 - 使用预签名URL)
  ↓ (识别成功)
EXECUTE (执行舵机动作)
  ↓ (完成)
IDLE (返回空闲)
```

**错误处理**：任何状态失败都会进入 ERROR 状态，显示错误信息后返回 IDLE。

**当前实现说明**：
- 先上传后识别：图片先进入 OBS，再通过预签名 URL 提供给模型访问
- 预签名 URL：桶可保持私有，无需配置公共读
- 低负载请求：AI 请求体不再包含大 Base64 图片数据

---

## 硬件配置

### 开发板

- **型号**：GOOUUU ESP32-S3-CAM
- **芯片**：ESP32-S3-WROOM-1
- **Flash**：16MB
- **PSRAM**：8MB

### 外设连接

| 外设 | GPIO 引脚 | 说明 |
|------|-----------|------|
| OV3660 摄像头 | GPIO 0-18 | 摄像头专用引脚 |
| WS2812 RGB LED | GPIO 48 | 状态指示灯 |
| 舵机（单舵机） | GPIO 1 | PWM 控制，左摆=其他垃圾，右摆=可回收垃圾 |
| SSD1306 OLED | I2C (GPIO 21=SDA, GPIO 47=SCL) | 显示屏 |

### LED 状态指示

| 颜色 | 状态 |
|------|------|
| 蓝色 | 处理中（拍照、上传、识别） |
| 绿色 | 空闲/成功 |
| 红色 | 错误 |

---

## 软件模块

### 模块结构

```
include/
├── config.h          # 硬件配置（引脚定义）
├── constants.h       # 常量定义（状态枚举、错误码）
├── secrets.h         # 敏感配置（WiFi、云服务密钥）
├── network.h         # WiFi网络功能模块
├── display.h         # OLED 显示模块
├── led.h             # LED 控制模块
├── camera.h          # 摄像头模块
├── servo.h           # 舵机控制模块
├── iotda.h           # 华为云 IoTDA 模块
├── obs.h             # 华为云 OBS 存储模块
└── ai.h              # AI 识别模块

src/
├── main.ino          # 主程序（状态机 + 业务协调）
├── network.cpp       # WiFi网络功能实现
├── display.cpp       # OLED 显示实现
├── led.cpp           # LED 控制实现
├── camera.cpp        # 摄像头实现
├── servo.cpp         # 舵机控制实现
├── iotda.cpp         # IoTDA 实现MQTT 连接和命令处理
├── obs.cpp           # OBS 实现（图片和记录上传）
└── ai.cpp            # AI 识别实现
```

### 核心模块说明

#### 1. 主程序 (main.ino)

**职责**：
- 系统初始化（WiFi、摄像头、舵机、IoTDA、OBS、AI）
- 状态机循环
- 模块协调
- **显示逻辑协调**（WiFi状态显示等）

**关键变量**：
```cpp
SystemState currentState = STATE_IDLE;  // 当前状态
ErrorCode lastError = ERROR_NONE;       // 错误码
bool captureTriggered = false;          // 拍照触发标志
camera_fb_t* photoBuffer = NULL;        // 照片缓冲区
char objectName[64];                    // OBS 对象名称
char objectUrl[256];                    // OBS 对象 URL
char presignedImageUrl[768];            // AI识图使用的预签名URL
char timestamp[32];                     // 时间戳
GarbageType garbageType;                // 识别结果
float confidence;                       // 置信度
unsigned long lastStatusReportTime = 0; // 上次状态上报时间
```

**WiFi重连维护**：
```cpp
void checkAndReconnectWiFi();  // WiFi重连 + 显示逻辑
```

#### 2. WiFi网络模块 (network.cpp)

**职责**：
- WiFi连接管理（单WiFi / 多WiFi智能选择）
- 网络扫描和自动重连
- NTP时间同步
- 重试延迟管理（指数退避）

**关键函数**：
```cpp
// 基础WiFi功能
void initWiFiMode();                    // 设置WiFi模式
bool connectWiFi();                     // 简单WiFi连接（测试用）
bool isWiFiConnected();                 // 检查连接状态
int getWiFiRSSI();                      // 获取信号强度

// 高级WiFi功能
bool initWiFi();                        // WiFi初始化（支持多网络）
int scanWiFi();                         // 网络扫描
bool connectToBestWiFi();               // 多WiFi智能选择
bool setupWiFi();                       // 单WiFi连接
bool checkWiFiConnection();             // 检查连接状态
bool reconnectWiFi();                   // WiFi重连

// 重试延迟管理
int getWiFiRetryDelay();                // 获取重试延迟
void resetWiFiRetryDelay();             // 重置延迟
void updateWiFiRetryDelay();            // 指数退避更新

// NTP时间同步
bool syncNTPTime();                     // 同步NTP时间
```

**多WiFi配置**：
```cpp
// secrets.h 中定义
const WiFiNetwork wifiNetworks[WIFI_NETWORK_COUNT] = {
    {"Xiaomi-3m9a", "password1", 1},     // 优先级1（最高）
    {"LAPTOP-xxx", "password2", 2}       // 优先级2（备用）
};

// config.h 中启用
#define USE_MULTI_WIFI true  // 启用多WiFi模式
```

**架构特点**：
- ✅ 纯网络功能，不依赖display.h
- ✅ 支持多WiFi智能选择（按优先级）
- ✅ 自动重连 + 指数退避策略
- ✅ 测试程序可直接使用完整功能

#### 3. 摄像头模块 (camera.cpp)

**职责**：
- 初始化 OV3660 摄像头
- 配置 PSRAM 帧缓冲区
- 拍照和释放资源

**关键函数**：
```cpp
bool initCamera();                    // 初始化摄像头
camera_fb_t* capturePhoto();          // 拍照
void releasePhoto(camera_fb_t* fb);   // 释放帧缓冲区
```

**配置参数**：
- 分辨率：VGA (640x480)
- 格式：JPEG
- 帧缓冲区：存储在 PSRAM
- 质量：高质量 JPEG（有PSRAM时 quality=10，无PSRAM时 quality=12）

#### 3. 华为云 IoTDA 模块 (iotda.cpp)

**职责**：
- MQTT 连接和自动重连
- 命令接收和处理
- 状态和结果上报

**关键函数**：
```cpp
bool initIoTDA();                     // 初始化 MQTT 连接
void handleMQTTLoop();                // 处理 MQTT 消息循环
void mqttCallback(...);               // MQTT 消息回调
void reportStatus(const char* status);         // 上报设备状态
void reportResult(GarbageType type, float confidence);  // 上报识别结果
void reportError(ErrorCode error);    // 上报错误
```

**MQTT 主题**：
- 订阅：`$oc/devices/{device_id}/sys/commands/request/id/+`
- 发布：`$oc/devices/{device_id}/sys/properties/report`

**命令处理**：
- `capture`：触发拍照识别流程
- `test`：测试命令（用于验证连接）

#### 4. 华为云 OBS 存储模块 (obs.cpp)

**职责**：
- 图片上传到 OBS
- 生成图片对象 URL
- 生成短时有效的预签名 GET URL
- 上传阶段使用 AWS Signature Version 4 签名认证

**关键函数**：
```cpp
bool initOBS();                       // 初始化 OBS 服务
bool uploadToOBS(uint8_t* imageData, size_t dataSize, char* objectName, char* objectUrl);  // 上传图片
void generateObjectUrl(const char* objectName, char* url, size_t urlSize);  // 生成对象 URL
bool generatePresignedGetUrl(const char* objectName, char* url, size_t urlSize, uint32_t expiresSeconds);  // 生成预签名下载URL
```

**OBS 文件结构**：
```
{bucket-name}/
├── Images/
│   ├── garbage_123456.jpg
│   └── garbage_789012.jpg
└── Images/
    └── records.csv  (IoTDA规则引擎转发)
```

**识别记录 JSON 格式**：
```json
{
  "timestamp": "123456",
  "imageUrl": "https://your-bucket.obs.cn-north-4.myhuaweicloud.com/images/garbage_123456.jpg",
  "garbageType": "可回收垃圾",
  "confidence": 0.95
}
```

#### 5. AI 识别模块 (ai.cpp)

**职责**：
- 调用 OpenAI 兼容 API
- 支持本地图片输入和 URL 输入两种识图方式
- 当前主流程采用 OBS 预签名 URL 输入
- 构造识别提示词
- 解析响应和容错处理

**关键函数**：
```cpp
bool initAI();                        // 初始化 AI 服务
bool recognizeGarbage(const uint8_t* imageData, size_t imageDataSize, GarbageType& type, float& confidence);  // 识别垃圾
bool recognizeGarbageByUrl(const char* imageUrl, GarbageType& type, float& confidence);  // 基于图片URL识别垃圾
```

**当前图片传输方式**：
- 图片先上传到华为云 OBS
- 为私有对象生成短时有效的预签名 URL
- 将该 URL 作为 `image_url.url` 传入模型
- 调试时仍保留 Base64 直传能力，但不是主流程

**API 调用格式**：
```json
{
  "model": "gpt-4-vision-preview",
  "messages": [
    {
      "role": "user",
      "content": [
        {
          "type": "text",
          "text": "请识别这张图片中的垃圾类型..."
        },
        {
          "type": "image_url",
          "image_url": {
            "url": "https://your-bucket.obs.cn-east-3.myhuaweicloud.com/Images/garbage_xxx.jpg?...Signature=..."
          }
        }
      ]
    }
  ],
  "max_tokens": 100
}
```

**识别提示词**：
```
请识别这张图片中的垃圾类型。这是一个二分类任务：可回收垃圾或其他垃圾。

可回收垃圾包括：纸张、塑料瓶、玻璃瓶、金属罐、纸箱等。
其他垃圾包括：食物残渣、用过的纸巾、陶瓷碎片、烟头等。

请以JSON格式返回识别结果，格式如下：
{
  "type": "recyclable" 或 "other",
  "confidence": 0.0-1.0之间的置信度数值
}

请只返回JSON，不要包含其他文字。
```

#### 6. 舵机控制模块 (servo.cpp)

**职责**：
- 控制单舵机左右摆动，模拟垃圾桶分类
- 可回收垃圾→向右摆动到135°，其他垃圾→向左摆动到45°

**关键函数**：
```cpp
bool initServo();                     // 初始化舵机
void openBin(GarbageType type);       // 打开对应垃圾桶（左右摆动）
void closeAllBins();                  // 关闭所有垃圾桶（回到中间90°）
```

**配置参数**：
- GPIO：1（可通过 `SERVO_PIN` 配置）
- 初始位置：90°（中间位置）
- 可回收垃圾：向右摆动到135°，保持1000ms，回到90°
- 其他垃圾：向左摆动到45°，保持1000ms，回到90°

#### 7. 显示模块 (display.cpp)

**职责**：
- OLED 显示状态和结果
- 多行文本显示

**关键函数**：
```cpp
bool initOLED();                      // 初始化 OLED
void displayMessage(const char* line1, const char* line2, const char* line3);  // 显示消息
void displayError(const char* error); // 显示错误
```

#### 8. LED 控制模块 (led.cpp)

**职责**：
- WS2812 RGB LED 控制
- 颜色设置

**关键函数**：
```cpp
bool initLED();                       // 初始化 LED
void setLEDColor(LEDColor color);     // 设置 LED 颜色
```

---

## 云服务配置

### 1. 华为云 IoTDA 配置

#### 1.1 创建产品和设备

详细步骤请参考：`docs/iotda_product_model_config.md`

**产品模型定义**：

**属性（Properties）**：
| 属性名称 | 标识符 | 数据类型 | 读写类型 |
|---------|--------|---------|---------|
| 设备状态 | Status | string | 只读 |
| 垃圾类型 | GarbageType | string | 只读 |
| 置信度 | Confidence | decimal | 只读 |
| 错误信息 | Error | string | 只读 |

**命令（Commands）**：
| 命令名称 | 标识符 | 参数 |
|---------|--------|------|
| 拍照识别 | capture | {"command": "capture"} |
| 测试命令 | test | {"command": "test"} |

#### 1.2 获取设备信息

在华为云 IoTDA 控制台注册设备后，记录以下信息：
- **设备 ID**：用于 MQTT 连接
- **设备密钥**：用于 MQTT 认证
- **MQTT 服务器地址**：在实例详情页面查看

#### 1.3 配置 secrets.h

在 `include/secrets.h` 中配置：

```cpp
// 华为云IoTDA配置
#define MQTT_SERVER "your-mqtt-server.iot-mqtts.cn-north-4.myhuaweicloud.com"
#define MQTT_PORT 1883
#define CLIENT_ID "your-device-id_0_0_2026042001"
#define MQTT_USER "your-device-id"
#define MQTT_PASSWORD "your-device-secret-hash"
```

### 2. 华为云 OBS 配置

#### 2.1 创建 OBS 桶

1. 登录华为云控制台
2. 进入"对象存储服务 OBS"
3. 创建桶：
   - **桶名**：自定义（全局唯一）
   - **区域**：选择与 IoTDA 相同的区域（如 cn-north-4）
   - **存储类别**：标准存储
   - **桶策略**：私有读写

#### 2.2 获取访问密钥

1. 进入"我的凭证" → "访问密钥"
2. 创建访问密钥，记录：
   - **Access Key ID**：访问密钥 ID
   - **Secret Access Key**：秘密访问密钥

#### 2.3 配置 secrets.h

在 `include/secrets.h` 中配置：

```cpp
// 华为云OBS配置
#define OBS_ENDPOINT "your-bucket.obs.cn-north-4.myhuaweicloud.com"
#define OBS_ACCESS_KEY "your-access-key-id"
#define OBS_SECRET_KEY "your-secret-access-key"
#define OBS_REGION "cn-north-4"
```

#### 2.4 创建文件夹结构

在 OBS 桶中创建以下文件夹：
- `Images/`：存储拍照图片（代码中 `OBS_IMAGE_PATH` 配置）
- `Images/records.csv`：存储识别记录（由 IoTDA 规则引擎转发维护）

### 3. AI 识别服务配置

#### 3.1 选择 AI 服务商

支持 OpenAI 兼容 API 的国内服务商：
- 智谱 GLM-4V
- 通义千问 VL
- 百度文心一言
- 其他兼容服务

#### 3.2 获取 API 密钥

根据选择的服务商，获取 API 密钥。

#### 3.3 配置 secrets.h

在 `include/secrets.h` 中配置：

```cpp
// AI识别服务配置
#define AI_API_ENDPOINT "https://dashscope.aliyuncs.com/compatible-mode"
#define AI_API_KEY "your-api-key"
#define AI_MODEL "qwen3.5-flash"
```

### 4. WiFi 配置

在 `include/secrets.h` 中配置：

```cpp
// WiFi配置
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"
```

---

## 编译和烧录

### 1. 安装开发环境

1. 安装 Visual Studio Code
2. 安装 PlatformIO 插件
3. 安装 Python 3.x

### 2. 配置项目

```bash
# 克隆项目
git clone <repository-url>
cd AIoT-Project

# 复制配置模板
cp include/secrets.h.template include/secrets.h

# 编辑 secrets.h，填入您的配置
# 使用您喜欢的编辑器编辑 include/secrets.h
```

### 3. 编译项目

```bash
# 编译
pio run

# 或在 VS Code 中点击底部状态栏的 ✓ 图标
```

### 4. 烧录到设备

```bash
# 连接 ESP32-S3-CAM 到电脑（USB-C）

# 烧录
pio run --target upload

# 或在 VS Code 中点击底部状态栏的 → 图标
```

### 5. 查看串口输出

```bash
# 查看串口输出
pio device monitor

# 或在 VS Code 中点击底部状态栏的 🔌 图标
```

**波特率**：115200

---

## 使用说明

### 1. 启动设备

1. 给 ESP32-S3-CAM 上电
2. 观察串口输出，确认以下初始化步骤：
   - OLED 初始化成功
   - WiFi 连接成功
   - 摄像头初始化成功
   - 舵机初始化成功
   - IoTDA 连接成功
   - OBS 初始化成功
   - AI 初始化成功
3. OLED 显示"系统就绪"，LED 显示绿色

### 2. 触发识别

#### 方式 1：使用华为云 IoTDA 控制台

1. 登录华为云 IoTDA 控制台
2. 进入设备详情页面
3. 点击"命令"标签
4. 选择"同步命令下发"
5. 选择命令：`capture`
6. 点击"发送"
7. 观察设备响应和状态变化

#### 方式 2：使用 MQTT 工具（如 MQTT.fx）

1. 连接到 IoTDA MQTT 服务器
2. 订阅主题：`$oc/devices/{device_id}/sys/commands/request/id/+`
3. 发布命令消息到主题：`$oc/devices/{device_id}/sys/commands/request/id/{request_id}`
4. 消息内容：
   ```json
   {
     "paras": {
       "command": "capture"
     }
   }
   ```

### 3. 查看识别结果

#### 方式 1：华为云 IoTDA 控制台

在设备详情页面查看设备影子，可以看到：
- Status：设备状态
- GarbageType：识别结果
- Confidence：置信度

#### 方式 2：OBS 控制台

1. 登录华为云 OBS 控制台
2. 进入桶列表
3. 查看 `images/` 文件夹中的图片
4. 查看 `records/` 文件夹中的识别记录 JSON 文件

### 4. 观察硬件反馈

- **LED 指示**：
  - 蓝色：处理中
  - 绿色：成功
  - 红色：错误

- **OLED 显示**：
  - 显示当前状态
  - 显示识别结果

- **舵机动作**：
   - 可回收垃圾：舵机向右摆动（90° → 135° → 90°）
   - 其他垃圾：舵机向左摆动（90° → 45° → 90°）

---

## 完整流程示例

### 正常流程

```
1. 设备启动，初始化所有模块
   → OLED: "系统就绪"
   → LED: 绿色

2. 用户通过 IoTDA 控制台发送 capture 命令
   → 设备收到命令
   → captureTriggered = true

3. 状态机进入 CAPTURE 状态
   → LED: 蓝色
   → OLED: "拍照中..."
   → 摄像头拍照
   → photoBuffer 获取照片数据

4. 状态机进入 UPLOAD 状态
   → OLED: "上传中..."
   → 上传图片到 OBS: Images/garbage_123456.jpg
   → 生成预签名图片 URL
   → 释放 photoBuffer

5. 状态机进入 RECOGNIZE 状态
   → OLED: "识别中..."
   → 调用 AI API，传入预签名图片 URL
   → 解析识别结果：可回收垃圾，置信度 0.95
   → 上报识别结果到 IoTDA

6. 状态机进入 EXECUTE 状态
   → OLED: "识别结果: 可回收垃圾"
   → 舵机向右摆动（90° → 135°）
   → 保持 1000ms
   → 舵机回中（135° → 90°）
   → 上报状态"完成"到 IoTDA
   → 上报状态"完成"到 IoTDA

7. 返回 IDLE 状态
   → OLED: "系统就绪"
   → LED: 绿色
```

### 错误流程示例

```
1. 设备启动，初始化失败（如摄像头初始化失败）
   → lastError = ERROR_CAMERA_INIT
   → currentState = STATE_ERROR

2. 状态机进入 ERROR 状态
   → LED: 红色
   → OLED: "错误: 摄像头初始化失败"
   → 上报错误到 IoTDA
   → 清理资源
   → 延迟 5000ms

3. 返回 IDLE 状态
   → lastError = ERROR_NONE
   → OLED: "系统就绪"
   → LED: 绿色
```

---

## 内存使用情况

编译结果（Release 模式）：
- **RAM**：15.9% (52,144 / 327,680 字节)
- **Flash**：15.4% (1,007,993 / 6,553,600 字节)

**内存优化**：
- 图像缓冲区存储在 PSRAM（8MB）
- 及时释放帧缓冲区
- 使用 String 对象避免内存泄漏

---

## 故障排查

### 1. WiFi 连接失败

**症状**：串口输出"WiFi连接失败"

**排查步骤**：
1. 检查 WiFi SSID 和密码是否正确
2. 检查 WiFi 信号强度
3. 检查路由器是否允许新设备连接
4. 尝试重启路由器和设备

### 2. MQTT 连接失败

**症状**：串口输出"MQTT连接失败"

**排查步骤**：
1. 检查 IoTDA 服务器地址和端口是否正确
2. 检查设备 ID 和密钥是否正确
3. 检查设备是否在 IoTDA 控制台注册
4. 检查设备状态是否为"在线"

### 3. 摄像头初始化失败

**症状**：串口输出"摄像头初始化失败"

**排查步骤**：
1. 检查摄像头连接是否正常
2. 检查 PSRAM 是否启用
3. 检查摄像头型号是否为 OV3660
4. 尝试降低分辨率或质量

### 4. OBS 上传失败

**症状**：串口输出"图片上传失败"

**排查步骤**：
1. 检查 OBS Endpoint、Access Key、Secret Key 是否正确
2. 检查 OBS 桶是否存在
3. 检查 OBS 桶权限设置
4. 检查网络连接是否正常

### 5. AI 识别失败

**症状**：串口输出"AI识别失败"

**排查步骤**：
1. 检查 AI API URL 和 Key 是否正确
2. 检查图片 URL 是否可访问
3. 检查 AI API 配额是否用完
4. 检查 AI API 响应格式是否正确

### 6. 舵机不动作

**症状**：舵机不转动

**排查步骤**：
1. 检查舵机电源连接
2. 检查舵机信号线连接
3. 检查舵机型号和 PWM 频率
4. 尝试手动调用 `testServoAction()` 测试

---

## 扩展功能

### 1. 添加红外传感器触发

修改 `STATE_IDLE` 状态处理：

```cpp
case STATE_IDLE:
    // 检查红外传感器
    if (digitalRead(IR_SENSOR_PIN) == HIGH) {
        INFO_PRINTLN("红外传感器触发");
        captureTriggered = true;
    }

    // 检查 MQTT 命令
    if (captureTriggered) {
        // ...
    }
    break;
```

### 2. 添加更多垃圾类型

修改 `constants.h`：

```cpp
enum GarbageType {
    GARBAGE_RECYCLABLE = 0,   // 可回收垃圾
    GARBAGE_HAZARDOUS = 1,    // 有害垃圾
    GARBAGE_KITCHEN = 2,      // 厨余垃圾
    GARBAGE_OTHER = 3,        // 其他垃圾
    GARBAGE_UNKNOWN = 4       // 未知
};

const char* GARBAGE_NAMES[] = {
    "可回收垃圾",
    "有害垃圾",
    "厨余垃圾",
    "其他垃圾",
    "未知"
};
```

修改舵机控制：

```cpp
void openBin(GarbageType type) {
    switch (type) {
        case GARBAGE_RECYCLABLE:
            // 舵机 1 动作
            break;
        case GARBAGE_HAZARDOUS:
            // 舵机 2 动作
            break;
        case GARBAGE_KITCHEN:
            // 舵机 3 动作
            break;
        case GARBAGE_OTHER:
            // 舵机 4 动作
            break;
    }
}
```

### 3. 添加语音播报

添加语音模块（如 DFPlayer Mini），在识别完成后播报结果：

```cpp
void speakResult(GarbageType type) {
    switch (type) {
        case GARBAGE_RECYCLABLE:
            dfPlayer.play(1);  // 播放"可回收垃圾"
            break;
        case GARBAGE_OTHER:
            dfPlayer.play(2);  // 播放"其他垃圾"
            break;
    }
}
```

---

## 项目文件清单

```
AIoT-Project/
├── include/
│   ├── config.h              # 硬件配置
│   ├── constants.h           # 常量定义
│   ├── secrets.h             # 敏感配置（需自行创建）
│   ├── secrets.h.template    # 配置模板
│   ├── display.h             # OLED 显示接口
│   ├── led.h                 # LED 控制接口
│   ├── camera.h              # 摄像头接口
│   ├── servo.h               # 舵机控制接口
│   ├── iotda.h               # IoTDA 接口
│   ├── obs.h                 # OBS 存储接口
│   └── ai.h                  # AI 识别接口
├── src/
│   ├── main.ino              # 主程序
│   ├── display.cpp           # OLED 显示实现
│   ├── led.cpp               # LED 控制实现
│   ├── camera.cpp            # 摄像头实现
│   ├── servo.cpp             # 舵机控制实现
│   ├── iotda.cpp             # IoTDA 实现
│   ├── obs.cpp               # OBS 实现
│   └── ai.cpp                # AI 识别实现
├── docs/
│   ├── iotda_product_model_config.md  # IoTDA 配置指南
│   └── system_documentation.md        # 系统说明文档（本文档）
├── reference/
│   └── HuaweiCloud_ESP32.ino # 华为云参考代码
├── platformio.ini            # PlatformIO 配置
├── task_plan.md              # 任务计划
├── progress.md               # 进度日志
├── findings.md               # 研究发现
├── README.md                 # 项目说明
└── CLAUDE.md                 # 项目概况
```

---

## 开发历程与优化记录

### Phase 9.3: WiFi功能重构 (2026-04-22)

#### 重构目标
将WiFi功能从main.ino移动到network.cpp，实现网络功能与显示逻辑的分离。

#### 重构成果

**代码改动统计**：
- network.h: +30行（新增函数声明）
- network.cpp: +200行（实现WiFi功能）
- main.ino: -180行（删除WiFi实现，保留显示逻辑）
- test_wifi/main.cpp: +50行（增强测试覆盖）

**架构改进**：
```
重构前：
  network.cpp (基础WiFi) - 仅connectWiFi()
  main.ino (网络+显示混合) - 包含displayMessage()

重构后：
  network.cpp (纯网络功能) - 不依赖display.h
  main.ino (业务协调) - 调用network + 显示
```

**新增功能**：
- ✅ 多WiFi智能选择（按优先级）
- ✅ 网络扫描和自动重连
- ✅ 重试延迟管理（指数退避）
- ✅ NTP时间同步
- ✅ 测试程序支持完整WiFi功能

**编译验证**：
- ✅ 主程序: RAM 16.3%, Flash 15.2%
- ✅ 测试程序: RAM 13.7%, Flash 10.9%

**设计原则**：
- 网络功能与显示逻辑完全分离
- network模块不依赖display.h
- 通过返回值传递结果，调用者决定显示方式
- 函数命名简洁清晰（initWiFi而非initWiFiWithDisplay）

**详细文档**：
- `docs/wifi_refactor_design_v2.md` - 重构设计方案
- `docs/wifi_refactor_analysis.md` - 风险分析

---

## 云服务配置清单

### 必需配置

| 服务 | 配置项 | 获取位置 |
|------|--------|---------|
| WiFi | SSID, Password | 路由器设置 |
| 华为云 IoTDA | 服务器地址, 设备 ID, 设备密钥 | IoTDA 控制台 |
| 华为云 OBS | Endpoint, Access Key, Secret Key, Region | OBS 控制台 + 我的凭证 |
| AI 服务 | API URL, API Key, Model | AI 服务商控制台 |

### 配置步骤总结

1. **创建华为云账号**（如果没有）
2. **配置 IoTDA**：
   - 创建 IoTDA 实例
   - 创建产品和设备
   - 定义产品模型（属性和命令）
   - 获取设备 ID 和密钥
3. **配置 OBS**：
   - 创建 OBS 桶
   - 创建 `images/` 和 `records/` 文件夹
   - 获取访问密钥
4. **配置 AI 服务**：
   - 选择 AI 服务商
   - 获取 API 密钥
5. **配置 secrets.h**：
   - 填入所有配置信息
6. **编译和烧录**：
   - 编译项目
   - 烧录到设备
7. **测试**：
   - 使用 IoTDA 控制台发送命令
   - 观察设备响应和结果

---

## 联系和支持

如有问题，请查看：
- 项目文档：`docs/` 目录
- 华为云文档：https://support.huaweicloud.com/
- PlatformIO 文档：https://docs.platformio.org/

---

**文档版本**：1.2
**最后更新**：2026-04-25

**更新日志**：
- v1.2 (2026-04-25): 更新为 OBS 预签名 URL 识图主方案，修正文档中的旧 Base64 直传描述
- v1.1 (2026-04-22): 新增WiFi功能重构记录，更新模块结构说明
- v1.0 (2026-04-18): 初始版本
**作者**：Claude Code Assistant
