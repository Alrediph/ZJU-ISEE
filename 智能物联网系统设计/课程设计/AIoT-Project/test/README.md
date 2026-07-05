# 测试程序使用指南

## 概述

本项目包含 **10 个独立测试程序**，采用 **4 层分层测试架构**，用于验证 ESP32-S3-CAM 垃圾分类识别系统各模块的功能。测试遵循"自底向上"原则，上层测试依赖下层模块的正确性。

## 测试架构

```
Layer 1: 硬件基础层（无依赖/低依赖）
  ├─ test_led           - LED 颜色与效果测试
  ├─ test_oled          - OLED 显示功能测试
  ├─ test_servo         - 舵机集成测试（含 LED + OLED 辅助显示）
  └─ test_servo_simple  - 舵机最小化直接测试（无项目模块依赖）

Layer 2: 摄像头层（依赖硬件）
  └─ test_camera        - 摄像头拍照与内存管理测试

Layer 3: 网络通信层（依赖网络）
  ├─ test_wifi          - WiFi 连接与重连测试
  ├─ test_iotda         - IoTDA MQTT 通信完整测试
  └─ test_iotda_minimal - IoTDA MQTT 最小化排障测试（无项目模块依赖）

Layer 4: 云服务层（依赖云服务）
  ├─ test_obs           - OBS 图片上传与 URL 生成测试
  └─ test_ai            - AI 识别端到端测试（拍照→上传→识别）
```

## 编译和上传

### 方法1：使用 PlatformIO 命令行

```bash
# 编译指定测试程序
pio run -e <环境名>

# 编译并上传
pio run -e <环境名> -t upload

# 打开串口监视器
pio device monitor

# 一键清理+编译+上传+监视
pio run -e <环境名> -t upload -t monitor
```

### 方法2：使用 PlatformIO IDE

1. 点击底部状态栏的环境选择器
2. 选择对应的测试环境（如 `env:test_led`）
3. 点击编译/上传按钮
4. 点击串口监视器按钮查看输出

### 所有测试环境一览

| 环境名              | 测试程序         | 所在层级   | 编译命令                              |
|---------------------|------------------|------------|----------------------------------------|
| `test_led`          | LED 测试         | Layer 1    | `pio run -e test_led -t upload`        |
| `test_oled`         | OLED 测试        | Layer 1    | `pio run -e test_oled -t upload`       |
| `test_servo`        | 舵机集成测试     | Layer 1    | `pio run -e test_servo -t upload`      |
| `test_servo_simple` | 舵机最小化测试   | Layer 1    | `pio run -e test_servo_simple -t upload` |
| `test_camera`       | 摄像头测试       | Layer 2    | `pio run -e test_camera -t upload`     |
| `test_wifi`         | WiFi 测试        | Layer 3    | `pio run -e test_wifi -t upload`       |
| `test_iotda`        | IoTDA 完整测试   | Layer 3    | `pio run -e test_iotda -t upload`      |
| `test_iotda_minimal`| IoTDA 最小化测试 | Layer 3    | `pio run -e test_iotda_minimal -t upload` |
| `test_obs`          | OBS 上传测试     | Layer 4    | `pio run -e test_obs -t upload`        |
| `test_ai`           | AI 识别测试      | Layer 4    | `pio run -e test_ai -t upload`         |

---

## 测试程序详细说明

### Layer 1 - 硬件基础层

#### test_led - LED 颜色与效果测试

**源代码：** `test/layer1_hardware/test_led/main.cpp`
**依赖模块：** `led.cpp`
**是否需要网络：** 否

**测试步骤：**
1. 初始化 LED 模块
2. 依次显示 6 种颜色（红、绿、蓝、黄、紫、白），各持续 1 秒
3. 测试闪烁效果：红色 LED 闪烁 3 次（500ms 间隔）
4. 测试呼吸效果：蓝色 LED 呼吸 5 秒
5. 测试完成后每 5 秒输出一次提示信息

**验证方式：** 人工观察 LED 颜色变化和动态效果

**预期串口输出：**
```
[TEST] LED Test Start
=====================================
[STEP] Initialize LED...
[OK] LED initialized
[STEP] Testing LED colors...
[STEP] Set LED to RED...
[OK] LED is RED (observe)
...
[STEP] Testing blink effect (RED, 3 times, 500ms interval)...
[OK] Blink effect complete
[STEP] Testing breathe effect (BLUE, 5 seconds)...
[OK] Breathe effect complete
=====================================
[TEST] LED Test Complete
```

**注意：** 测试结束后设备保持空闲，按 RESET 键可重新运行。

---

#### test_oled - OLED 显示功能测试

**源代码：** `test/layer1_hardware/test_oled/main.cpp`
**依赖模块：** `display.cpp`
**是否需要网络：** 否（内部创建 stub WiFiClient/PubSubClient 对象）

**测试步骤：**
1. 初始化 OLED 显示屏
2. 测试 `displayMessage()` — 三行文本显示
3. 测试 `displayTitleContent()` — 标题+内容双行显示
4. 测试 `displayProgress()` — 进度条动画（0%→25%→50%→75%→100%）
5. 测试 `displayError()` — 错误信息显示
6. 测试 `displayResult()` — 两种识别结果显示（Recyclable 95%、Other 87%）
7. 测试 `displayStatus()` — 成功/失败状态显示
8. 测试 `displayNetworkStatus()` — 网络状态栏显示（预期显示 `--`，因为无网络）

**验证方式：** 人工观察 OLED 屏幕显示内容是否正确

**预期串口输出：**
```
[TEST] OLED Test Start
=====================================
[STEP] Initialize OLED...
[OK] OLED initialized
[STEP] Testing displayMessage()...
[INFO] Displaying 3-line message
[OK] 3-line message displayed (observe OLED)
...
[TEST] OLED Test Complete
```

---

#### test_servo - 舵机集成测试

**源代码：** `test/layer1_hardware/test_servo/main.cpp`
**依赖模块：** `servo.cpp`、`led.cpp`、`display.cpp`
**是否需要网络：** 否

**测试步骤：**
1. 初始化 LED、OLED、舵机（LED 和 OLED 用于辅助显示测试状态）
2. 测试舵机右摆（可回收垃圾 `GARBAGE_RECYCLABLE`）：90°→135°→停留1秒→回中
3. 测试舵机左摆（其他垃圾 `GARBAGE_OTHER`）：90°→45°→停留1秒→回中
4. 测试 `closeAllBins()` 重置回中功能
5. 测试完成后 LED 显示绿色、OLED 显示完成信息

**舵机配置：**
- 引脚：GPIO 1
- 中点位置：90°
- 左摆（其他垃圾）：45°
- 右摆（可回收垃圾）：135°

**验证方式：** 人工观察舵机向左/右转动角度是否正确

**预期串口输出：**
```
[TEST] Servo Test Start
=====================================
[STEP] Initialize LED...
[OK] LED initialized
[STEP] Initialize OLED display...
[OK] OLED initialized
[STEP] Initialize Servos...
[OK] Servos initialized
[STEP] Testing Servo - Right Turn (Recyclable bin)...
[STEP] Action: Center(90°) -> Right(135°) -> Hold 1s -> Center(90°)
[OK] Right turn test complete (observe rotation)
[STEP] Testing Servo - Left Turn (Other waste bin)...
[STEP] Action: Center(90°) -> Left(45°) -> Hold 1s -> Center(90°)
[OK] Left turn test complete (observe rotation)
[STEP] Testing closeAllBins() - Reset to center position...
[OK] Servo reset to center position
=====================================
[TEST] Servo Test Complete
```

---

#### test_servo_simple - 舵机最小化测试

**源代码：** `test/layer1_hardware/test_servo_simple/main.cpp`
**依赖模块：** 无（直接使用 `ESP32Servo` 库）
**是否需要网络：** 否

**设计目的：** 当 `test_servo` 出现问题时，使用此最小化测试快速定位是 GPIO 引脚问题、舵机硬件问题还是程序模块问题。

**测试步骤：**
1. 等待 5 秒让用户打开串口监视器
2. 直接绑定 GPIO 14 到舵机（不经过任何项目模块）
3. 循环使舵机从 0° 逐步转到 100°（每次 +10°，间隔 500ms）
4. 测试完成后舵机回到 90° 中点位置

**与 test_servo 的区别：**
| 特性         | test_servo_simple    | test_servo           |
|--------------|----------------------|----------------------|
| 依赖模块     | 0（仅 ESP32Servo）   | 3（servo/led/display）|
| GPIO 引脚    | 14                   | 1                    |
| 测试范围     | 仅舵机转动          | 舵机+LED+OLED 联动   |
| 用途         | 硬件排障             | 功能集成验证         |

**验证方式：** 人工观察舵机是否按预期转动

**故障排查：** 如果 `test_servo_simple` 成功但 `test_servo` 失败，问题可能出在 `servo.cpp` 模块代码或 GPIO 1 引脚。如果 `test_servo_simple` 也失败，问题在舵机硬件或供电。

---

### Layer 2 - 摄像头层

#### test_camera - 摄像头拍照与内存管理测试

**源代码：** `test/layer2_camera/test_camera/main.cpp`
**依赖模块：** `camera.cpp`
**是否需要网络：** 否

**测试步骤：**
1. 打印摄像头初始化前的内存状态（Free Heap、Free PSRAM、Min Free Heap/PSRAM）
2. 初始化摄像头，打印初始化后的内存状态
3. 第一次拍照，输出照片信息（宽×高、字节数、格式）
4. 释放第一帧帧缓冲，打印释放后内存状态
5. 第二次拍照（验证内存管理 — 确保重复拍照无泄漏）
6. 释放第二帧帧缓冲
7. 打印最终内存状态，对比最小空闲内存判断是否有明显泄漏

**验证方式：** 串口输出照片信息和内存状态，观察：
- 两次拍照都能成功（不因内存不足失败）
- 释放帧缓冲后内存能恢复到相近水平

**预期串口输出：**
```
[TEST] Camera Test Start
=====================================
[STEP] Initial memory state:
[INFO] Before camera init:
  Free Heap: 280000 bytes
  Free PSRAM: 7800000 bytes
...
[STEP] First photo capture...
[INFO] First photo info:
  Width: 640 px
  Height: 480 px
  Size: 102400 bytes
  Format: JPEG
[STEP] Releasing first frame buffer...
[OK] First frame buffer released
[STEP] Second photo capture (memory test)...
[INFO] Second photo info:
  Width: 640 px
  Height: 480 px
  Size: 99800 bytes
  Format: JPEG
[STEP] Memory leak check:
  Minimum free heap during test: 120000 bytes
[OK] Memory management test complete
[TEST] Camera Test Complete
```

---

### Layer 3 - 网络通信层

#### test_wifi - WiFi 连接与重连测试

**源代码：** `test/layer3_network/test_wifi/main.cpp`
**依赖模块：** `network.cpp`（依赖 `secrets.h` 中的 WiFi 配置）
**是否需要网络：** 是

**测试步骤：**
1. 初始化 WiFi 模式（STA 模式）
2. 连接 WiFi，输出连接结果和配置信息（`USE_MULTI_WIFI` 状态）
3. 打印连接详情（IP、SSID、RSSI）
4. 测试连接检查 `checkWiFiConnection()`
5. 断开 WiFi `disconnectWiFi()`，验证断连成功
6. 重连 WiFi `reconnectWiFi()`，验证重连成功并打印连接信息
7. 稳定性监控：连续监测 5 秒，每秒输出连接状态和 RSSI
8. 输出详细网络信息（IP 地址、MAC 地址、信号强度）
9. 测试重试延迟管理（指数退避策略）：查看初始延迟 → 逐次加倍 → 重置

**验证方式：** 串口输出所有步骤结果

**前置条件：** 在 `include/secrets.h` 中配置正确的 WiFi SSID 和密码

**预期串口输出（摘要格式）：**
```
[STEP 1] Initialize WiFi Mode
[OK] WiFi mode initialized
[STEP 2] WiFi Initialization
[INFO] USE_MULTI_WIFI: true
[OK] WiFi connected successfully
[STEP 3] Test Connection Check
[OK] WiFi connection check passed
[STEP 4] Disconnect WiFi
[OK] WiFi disconnected successfully
[STEP 5] Test WiFi Reconnect
[OK] WiFi reconnected successfully
[STEP 6] Test Connection Stability
[INFO] Second 1: Connected (RSSI: -45 dBm)
...
[INFO] Connection stable
[STEP 7] Detailed Connection Info
[INFO] IP Address: 192.168.1.100
[INFO] MAC Address: AA:BB:CC:DD:EE:FF
[INFO] Signal Strength: -45 dBm
[STEP 8] Test Retry Delay Management
[TEST] WiFi Test Complete
[SUMMARY] Test Results:
  - WiFi mode initialization: OK
  - WiFi initialization: OK
  - Connection check: OK
  - Disconnection: OK
  - Reconnection: OK
  - Connection stability: OK
  - Retry delay management: OK
```

---

#### test_iotda - IoTDA MQTT 通信完整测试

**源代码：** `test/layer3_network/test_iotda/main.cpp`
**依赖模块：** `network.cpp`、`iotda.cpp`
**是否需要网络：** 是（WiFi + 华为云 IoTDA）

**测试步骤：**
1. 连接 WiFi
2. 初始化 IoTDA MQTT 连接，打印连接状态
3. 上报设备状态（预置数据：`"idle"`）
4. 验证命令订阅，等待 10 秒接收云端下发的命令（期间可在 IoTDA 控制台手动发送命令测试）
5. 上报模拟识别结果（预置数据：类型 `Recyclable`、置信度 95%、对象名 `test_image_001.jpg`）
6. 上报模拟错误信息（预置错误码：`ERROR_CAMERA_CAPTURE`）
7. MQTT 连接稳定性监控：连续 5 秒，每秒输出连接状态

**预置测试数据：**
| 字段            | 值                                             |
|-----------------|-------------------------------------------------|
| TEST_STATUS     | `"idle"`                                        |
| TEST_OBJECT_NAME| `"test_image_001.jpg"`                          |
| TEST_OBJECT_URL | `"https://test.obs.com/test_image_001.jpg"`     |
| TEST_TIMESTAMP  | `"2026-04-20 10:00:00"`                         |
| TEST_GARBAGE_TYPE | `GARBAGE_RECYCLABLE` (可回收垃圾)              |
| TEST_CONFIDENCE | 0.95 (95%)                                      |

**验证方式：** 串口输出连接和发布状态，并可在华为云 IoTDA 控制台查看设备上报的属性数据

**前置条件：**
- `include/secrets.h` 中配置正确的 WiFi 和 IoTDA（设备 ID、密钥）信息
- 设备已在华为云 IoTDA 控制台注册

**预期串口输出（摘要格式）：**
```
[TEST] IoTDA Test Start
=====================================
[STEP 1] Initialize WiFi
[OK] WiFi connected
[STEP 2] Initialize IoTDA
[OK] IoTDA initialized
[STEP 3] Report Device Status
[OK] Status reported
[STEP 4] Verify Command Subscription
[INFO] Waiting for commands (5 seconds)...
[OK] Command wait complete
[STEP 5] Report Recognition Result
[OK] Result reported
[STEP 6] Report Error
[OK] Error reported
[STEP 7] Test MQTT Connection Stability
[OK] MQTT connection monitoring complete
[TEST] IoTDA Test Complete
[SUMMARY] Test Results:
  - WiFi connection: OK
  - IoTDA initialization: OK
  - Status reporting: OK
  - Command subscription: OK
  - Result reporting: OK
  - Error reporting: OK
  - MQTT stability: OK
```

---

#### test_iotda_minimal - IoTDA MQTT 最小化排障测试

**源代码：** `test/layer3_network/test_iotda_minimal/main.cpp`
**依赖模块：** 无（直接使用 `WiFi.h` + `PubSubClient.h`，所有配置硬编码在文件中）
**是否需要网络：** 是（WiFi + 华为云 IoTDA）

**设计目的：** 当 `test_iotda` 出现 MQTT 连接问题时，使用此最小化测试快速定位是 IoTDA 配置问题、网络问题还是项目模块代码问题。**所有配置（WiFi、MQTT）均硬编码在主文件中，零外部依赖。**

**测试步骤：**
1. 打印所有配置信息（WiFi SSID、MQTT 服务器、设备 ID、客户端 ID、用户名、密码）
2. 连接 WiFi
3. 配置 MQTT 客户端（服务器地址、回调）
4. 连接 MQTT 服务器（详细的错误状态码解释）
5. 订阅命令主题
6. 进入主循环，每 10 秒自动发布一条测试消息（`Status: Test_N`）
7. 循环中自动处理 WiFi 断连重连和 MQTT 断连重连
8. 收到云端命令时自动打印完整 Topic 和 Payload

**与 test_iotda 的区别：**
| 特性         | test_iotda_minimal                  | test_iotda                    |
|--------------|--------------------------------------|-------------------------------|
| 配置文件     | 硬编码在主文件中                     | 依赖 `secrets.h`              |
| 依赖模块     | 0（仅系统库）                        | 2（network/iotda）            |
| MQTT 状态输出 | 详细的数字状态码及英文解释          | 简洁状态输出                  |
| 消息发布     | 循环发布测试消息（便于长时间测试）   | 一次性上报多种类型消息        |
| 用途         | MQTT 连接排障                        | IoTDA 完整功能验证            |

**验证方式：** 串口输出详细的连接过程；若连接失败，查看状态码解释定位问题

**MQTT 状态码速查（测试输出的关键排障信息）：**
| 状态码 | 含义                          | 排查方向                       |
|--------|-------------------------------|-------------------------------|
| 0      | MQTT_CONNECTED — 已连接       | 正常                          |
| -2     | MQTT_CONNECT_FAILED — 网络失败| 检查 WiFi 连接和 MQTT 服务器地址|
| -4     | MQTT_CONNECTION_TIMEOUT — 超时| 检查防火墙、服务器可达性       |
| 2      | MQTT_CONNECT_BAD_CLIENT_ID    | 检查 Client ID 格式           |
| 4      | MQTT_CONNECT_BAD_CREDENTIALS  | 检查用户名和密码               |
| 5      | MQTT_CONNECT_UNAUTHORIZED     | 检查设备认证信息               |

**预期串口输出：**
```
========================================
Minimal IoTDA MQTT Connection Test
========================================
[CONFIG] Configuration:
  WiFi SSID:     Xiaomi-3m9a
  MQTT Server:   015487493c.st1.iotda-device.cn-east-3.myhuaweicloud.com:1883
  Device ID:     69e6502fe094d6159234626c_ESP32-S3-CAM
  Client ID:     69e6502fe094d6159234626c_ESP32-S3-CAM_0_0_2026042109
  MQTT User:     69e6502fe094d6159234626c_ESP32-S3-CAM
[STEP 1] Connect WiFi
[OK] WiFi connected successfully
[STEP 2] Configure MQTT Client
[STEP 3] Connect MQTT Server
[OK] MQTT connected successfully
[STEP 4] Subscribe to Topics
[OK] Subscribed to: $oc/devices/.../sys/commands/
========================================
[TEST] Initialization Complete
[TEST] Publishing Test Message
[OK] Message published successfully
```

---

### Layer 4 - 云服务层

#### test_obs - OBS 图片上传与 URL 生成测试

**源代码：** `test/layer4_cloud/test_obs/main.cpp`
**依赖模块：** `network.cpp`、`obs.cpp`
**是否需要网络：** 是（WiFi + 华为云 OBS）
**是否需要摄像头：** 否（使用内置最小 JPEG 数组作为测试数据）

**测试步骤：**
1. 连接 WiFi
2. **同步 NTP 时间**（AWS SigV4 签名依赖准确的时间，时间偏差过大会导致签名失败）
3. 初始化 OBS 服务
4. 上传第一张测试图片（内置 1×1 红色像素 JPEG，约 280 字节）→ 输出对象名和 URL
5. 上传第二张测试图片（不同的 JPEG 数据）→ 输出对象名和 URL
6. 测试 `generateObjectUrl()` URL 构建功能

**内置测试图片说明：** 测试程序自带两个极小的 JPEG 数组（`TEST_IMAGE_1`、`TEST_IMAGE_2`），无需摄像头参与。这保证了即使摄像头未连接或故障，也能独立测试 OBS 上传功能。

**验证方式：** 串口输出上传结果和对象 URL，可在 OBS 控制台验证文件是否成功上传

**前置条件：**
- `include/secrets.h` 中配置正确的 OBS AK/SK、Endpoint 和 Bucket 信息
- WiFi 网络正常连接
- **NTP 时间同步必须成功**（否则 Auth 签名失败）

**预期串口输出（摘要格式）：**
```
[TEST] OBS Test Start
=====================================
[STEP 1] Initialize WiFi
[OK] WiFi connected
[STEP 1.5] Sync NTP Time
[OK] NTP time synced
[INFO] Current time: Sun Apr 27 10:30:00 2026
[STEP 2] Initialize OBS
[OK] OBS initialized
[STEP 3] Upload First Image
[OK] Upload 1 successful
[INFO] Object Name: Images/esp32_test_001.jpg
[INFO] Object URL: https://xxx.obs.cn-east-3.myhuaweicloud.com/Images/esp32_test_001.jpg
[STEP 4] Upload Second Image
[OK] Upload 2 successful
[STEP 5] Generate Object URL
[OK] Generated URL: https://xxx.obs.../test_object.jpg
[TEST] OBS Test Complete
[SUMMARY] Test Results:
  - WiFi connection: OK
  - NTP time sync: OK
  - OBS initialization: OK
  - Image upload 1: OK
  - Image upload 2: OK
  - URL generation: OK
```

---

#### test_ai - AI 识别端到端测试

**源代码：** `test/layer4_cloud/test_ai/main.cpp`
**依赖模块：** `network.cpp`、`camera.cpp`、`obs.cpp`、`ai.cpp`
**是否需要网络：** 是（WiFi + 华为云 OBS + 阿里云 DashScope AI）
**是否需要摄像头：** 是

**测试步骤：**
1. 连接 WiFi
2. 同步 NTP 时间（OBS 签名必须）
3. 初始化摄像头、OBS、AI 识别服务
4. 拍照 — 输出照片信息（大小、分辨率、格式）和当前内存状态
5. 上传照片到 OBS — 输出对象名和对象 URL
6. 生成预签名 GET URL（有效期 `OBS_PRESIGNED_URL_EXPIRES` 秒）
7. **验证预签名 URL 可访问性** — 通过 HTTP GET 请求测试 URL 是否可正常获取图片数据
8. 调用 AI 识别 API（使用预签名 URL 作为图片输入）— 输出垃圾类型和置信度
9. 释放照片帧缓冲，输出最终内存状态

**AI 识别流程：** 此测试使用的是 **OBS 预签名 URL 路由**（非 Base64 直接传输），即图片先上传到 OBS，生成临时可访问 URL 后再传给 AI API。这是生产环境使用的真实路径。

**验证方式：** 串口输出完整流程日志

**前置条件：**
- `include/secrets.h` 中配置正确的 WiFi、OBS（AK/SK/Bucket）和 AI API（密钥、端点）
- WiFi 网络正常连接
- 摄像头正常工作（可先运行 `test_camera` 验证）
- NTP 时间同步正常

**预期串口输出：**
```
[TEST] AI Recognition Test Start (OBS URL Route)
=====================================
[STEP 1] Initialize WiFi
[OK] WiFi connected
[STEP 2] Sync NTP Time
[OK] Current UTC time: Sun Apr 27 02:30:00 2026
[STEP 3] Initialize Camera / OBS / AI
[OK] Modules initialized
[STEP 4] Capture Photo
[OK] Photo captured:
  Size: 102400 bytes
  Resolution: 640x480
  Format: JPEG
[INFO] Memory state:
  Free Heap: 150000 bytes
  Free PSRAM: 7200000 bytes
[STEP 5] Upload Photo To OBS
[OK] OBS upload successful
[INFO] Object Name: Images/capture_001.jpg
[INFO] Object URL: https://xxx.obs...
[STEP 6] Generate Presigned GET URL
[OK] Presigned URL generated
[STEP 7] Validate Presigned URL
[DEBUG] 验证预签名URL可访问性
[INFO] GET响应码: 200
[INFO] Content-Length: 102400
[OK] 预签名URL可访问
[STEP 8] Recognize Garbage By URL
[OK] AI recognition successful
[INFO] Type: Recyclable
[INFO] Confidence: 95.23%
[STEP 9] Release Resources
[OK] Photo buffer released
[INFO] Memory after release:
  Free Heap: 155000 bytes
  Free PSRAM: 7500000 bytes
=====================================
[TEST] AI Recognition Test Complete
[SUMMARY]
  - Presigned URL reachable: YES
  - AI recognition succeeded: YES
```

---

## 测试流程建议

### 推荐测试顺序

按分层架构**自底向上**执行，确保下层稳定后再测试上层：

**1. Layer 1 测试（硬件基础层）**
按任意顺序执行：
- `test_led` → `test_oled` → `test_servo`
- 如舵机异常，用 `test_servo_simple` 快速排障

**2. Layer 2 测试（摄像头层）**
- `test_camera` — 验证拍照和内存管理

**3. Layer 3 测试（网络通信层）**
严格按顺序执行：
- `test_wifi` — 先确保网络连通
- `test_iotda` — 再测试云端通信
- 如 IoTDA 连接异常，用 `test_iotda_minimal` 最小化排障

**4. Layer 4 测试（云服务层）**
按任意顺序执行：
- `test_obs` → `test_ai` — OBS 测试无需摄像头，AI 测试需要摄像头

### 测试前检查清单

#### 硬件准备
- [x] ESP32-S3-CAM 开发板供电正常（5V / USB）
- [x] 摄像头模块（OV3660）正确连接
- [x] OLED 显示屏（I2C: SDA=GPIO 21, SCL=GPIO 47）正确连接
- [x] 舵机（GPIO 1）正确连接并独立供电（如需）
- [x] LED 灯（红=GPIO 38, 绿=GPIO 39, 蓝=GPIO 40）正确连接，或使用板载 WS2812
- [x] 串口监视器已打开（波特率 115200）

#### 软件配置
- [x] `include/secrets.h` 已从 `secrets.h.template` 创建并填写真实配置
- [x] WiFi SSID 和密码正确
- [x] 华为云 IoTDA 设备 ID 和密钥正确
- [x] 华为云 OBS AK/SK、Endpoint、Bucket 正确
- [x] 阿里云 DashScope API Key 正确

#### 逐层验证
- [x] **Layer 1:** `test_led` 通过 → `test_oled` 通过 → `test_servo` 通过
- [x] **Layer 2:** `test_camera` 拍照成功，内存无明显泄漏
- [x] **Layer 3:** `test_wifi` 连接稳定 → `test_iotda` MQTT 通信正常
- [x] **Layer 4:** `test_obs` 上传成功 → `test_ai` 识别成功

---

## 常见问题排查

### 编译错误

| 问题                       | 可能原因                           | 解决方法                                    |
|---------------------------|------------------------------------|---------------------------------------------|
| `fatal error: xxx.h: No such file or directory` | 头文件缺失                | 检查 `include/` 目录下是否存在对应头文件     |
| `undefined reference to 'xxx'` | 链接错误                       | 检查 `src/` 目录下是否存在对应实现文件       |
| `board_build.psram_type` 错误 | PSRAM 配置不兼容              | 确认使用 ESP32-S3 芯片和正确的 framework     |
| 编译环境选择错误            | IDE 底部状态栏选错环境              | 点击环境选择器切换目标环境                    |

### 运行错误

#### WiFi 连接失败
1. 检查 `include/secrets.h` 中的 `WIFI_SSID` 和 `WIFI_PASSWORD`
2. 确认 WiFi 频段为 2.4GHz（ESP32-S3 不支持 5GHz）
3. 确认设备在 WiFi 信号覆盖范围内（RSSI > -80 dBm）
4. 运行 `test_wifi` 查看详细错误信息

#### IoTDA MQTT 连接失败
1. **先运行 `test_iotda_minimal`** 进行最小化排障
2. 根据输出的 MQTT 状态码定位问题（参见上文状态码速查表）
3. 常见原因：设备 ID 格式错误、密钥不匹配、服务器地址/端口错误
4. 检查华为云 IoTDA 控制台设备是否"在线"

#### OBS 上传失败
1. **确认 NTP 时间同步成功**（日志中显示 `[OK] NTP time synced`）— 最常见原因
2. 检查 `include/secrets.h` 中的 OBS AK/SK（区分大小写）
3. 确认 OBS Bucket 名称、区域（Endpoint）正确
4. 检查 Bucket 权限策略是否允许当前 AK/SK

#### AI 识别失败
1. **先确保 `test_camera` 和 `test_obs` 均通过**
2. 检查 `include/secrets.h` 中的 AI API 密钥和端点
3. 确认 DashScope API Key 有效且余额充足
4. 查看串口输出中预签名 URL 是否可访问（`[STEP 7] Validate Presigned URL`）
5. 检查 AI API 返回的具体错误信息

---

## 后续工作

完成所有分层测试后，可以：
1. 编译并上传主程序 `pio run -e esp32-s3-devkitc-1 -t upload`
2. 通过 IoTDA 控制台下发命令触发完整垃圾分类流程
3. 在 IoTDA 控制台查看设备属性和历史数据
4. 根据实际需求扩展测试用例（如压力测试、长期稳定性测试）
