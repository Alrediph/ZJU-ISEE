# ESP32-S3-CAM 智能垃圾分类识别系统

基于 ESP32-S3-CAM 的智能垃圾分类识别系统，集成华为云平台和 AI 识别，实现完整的端到端垃圾分类流程。用于课程大作业演示。

## 核心功能

- **图像采集**：OV3660 摄像头拍照（VGA 分辨率 640x480、JPEG 高质量、PSRAM 帧缓冲）
- **云端存储**：图片上传到华为云 OBS（AWS Signature V4 签名认证）
- **AI 识别**：调用 OpenAI 兼容 API（DashScope 通义千问）进行垃圾分类识别
- **低负载识图链路**：通过 OBS 预签名 URL 将图片提供给 AI 模型，避免板端直接发送大 Base64 数据
- **硬件控制**：单舵机模拟垃圾桶开合（左摆=其他垃圾，右摆=可回收垃圾）
- **状态显示**：OLED 显示屏（128x64）+ WS2812 RGB LED 状态指示
- **远程控制**：通过华为云 IoTDA MQTT 远程触发识别，支持微信小程序扩展

## 系统特性

- **状态机驱动架构**：IDLE → CAPTURE → UPLOAD → RECOGNIZE → EXECUTE → IDLE
- **二分类系统**：可回收垃圾 / 其他垃圾
- **云端集成**：华为云 IoTDA + OBS + DashScope AI API
- **完整流程**：拍照 → 上传 OBS → AI 识别 → 舵机执行 → 结果上报
- **WiFi 多网络支持**：自动扫描、优先级连接、指数退避重连
- **NTP 时间同步**：自动同步系统时间，确保 OBS 签名时效

## 系统架构

```
华为云 IoTDA
  │  (MQTT: 命令下发 & 属性上报)
  ▼
ESP32-S3-CAM 设备
  │
  ├─ OV3660 摄像头 ──→ 拍照 (JPEG, SVGA 640x480, 高质量)
  ├─ 华为云 OBS ──────→ 图片上传 (AWS Signature V4)
  ├─ DashScope AI ────→ 垃圾分类识别 (预签名 URL)
  ├─ 舵机 (GPIO 1) ───→ 左摆=其他垃圾 / 右摆=可回收垃圾
  ├─ OLED (I2C) ──────→ 状态显示
  └─ WS2812 LED ──────→ 状态指示
       │
       ▼
 华为云 IoTDA ← 上报识别结果 (GarbageType, Confidence, ImageUrl)
       │
       ├─ (规则引擎) ──→ OBS CSV 持久化
       └─ (API 查询) ──→ 微信小程序展示
```

### 状态机流程

```
IDLE (空闲等待命令)
  ↓ 收到 IoTDA capture 命令
CAPTURE (OV3660 拍照)
  ↓ 拍照成功
UPLOAD (上传 JPEG 到华为云 OBS + 生成预签名 URL)
  ↓ 上传成功
RECOGNIZE (通过预签名 URL 调用 DashScope AI 识别)
  ↓ 识别成功
EXECUTE (舵机动作 + LED 指示 + 结果上报 IoTDA)
  ↓ 完成
IDLE (返回空闲)
  │
  └── 任何步骤失败 → ERROR (LED 红色 + 显示错误 + 等待 5s → IDLE)
```

## 快速开始

### 1. 环境准备

- Visual Studio Code + PlatformIO 插件
- Python 3.x
- ESP32-S3-CAM 开发板 + OV3660 摄像头 + SSD1306 OLED + 舵机 + WS2812 LED
- 华为云账号（开通 IoTDA、OBS）
- AI API 密钥（推荐 DashScope 通义千问）

### 2. 配置项目

```bash
# 克隆项目
git clone <repository-url>
cd AIoT-Project

# 复制配置模板
cp include/secrets.h.template include/secrets.h

# 编辑 secrets.h，填入您的配置
# 包括: WiFi、华为云 IoTDA、华为云 OBS、AI API
```

### 3. 云服务配置

按照 **[云服务配置清单](docs/guides/cloud_service_config_checklist.md)** 依次完成：

1. **华为云 IoTDA**：创建产品、定义设备模型（属性+命令）、注册设备、获取连接参数
2. **华为云 OBS**：创建桶、获取 AK/SK、配置桶策略（可选公开读取）
3. **AI 服务**：获取 API 密钥、配置模型名称

详细配置见 [IoTDA 产品模型配置文档](docs/iotda_product_model_config.md)。

### 4. 硬件接线

| 外设 | 引脚 | 说明 |
|------|------|------|
| OV3660 摄像头 | GPIO 0-18 | 按 config.h 定义的引脚接线 |
| SSD1306 OLED | SDA=GPIO 21, SCL=GPIO 47 | I2C 接口 |
| 舵机 | GPIO 1 | PWM 控制 |
| WS2812 LED | GPIO 48 | 板载 RGB LED |

### 5. 编译和烧录

```bash
# 编译主程序
pio run -e esp32-s3-devkitc-1

# 烧录到设备
pio run -e esp32-s3-devkitc-1 -t upload

# 查看串口输出
pio device monitor -b 115200
```

### 6. 运行测试

所有测试程序使用 `pio run -e <环境名> -t upload` 编译烧录：

| 测试层级 | 环境名 | 测试内容 |
|---------|--------|---------|
| Layer 1 | `test_led` | LED 颜色和效果 |
| Layer 1 | `test_oled` | OLED 显示功能 |
| Layer 1 | `test_servo` | 舵机动作测试 |
| Layer 2 | `test_camera` | 摄像头拍照测试 |
| Layer 3 | `test_wifi` | WiFi 连接测试 |
| Layer 3 | `test_iotda` | IoTDA MQTT 通信测试 |
| Layer 4 | `test_obs` | OBS 图片上传测试 |
| Layer 4 | `test_ai` | AI 识别测试 |

详细测试说明见 [测试程序使用指南](test/README.md)。

### 7. 启动系统

烧录主程序后，设备将自动：
1. 连接 WiFi → NTP 时间同步
2. 初始化 OLED、LED、摄像头、舵机
3. 连接 IoTDA（MQTT）
4. 进入 IDLE 空闲状态（LED 绿色）

使用华为云 IoTDA 控制台发送 `capture` 命令即可触发完整识别流程。

## 文档

- **[系统完整说明文档](docs/system_documentation.md)** - 系统架构、模块说明、使用指南、故障排查
- **[微信小程序开发指南](docs/wechat_miniprogram_guide.md)** - IoTDA API 接入、属性查询、命令下发、图片展示
- **[云服务配置清单](docs/guides/cloud_service_config_checklist.md)** - 华为云 IoTDA、OBS、AI 服务配置步骤
- **[IoTDA产品模型配置](docs/iotda_product_model_config.md)** - IoTDA 产品模型详细配置指南（含规则引擎数据转发）
- **[测试计划文档](docs/test_plan.md)** - 测试架构、测试用例、验证方法
- **[测试程序使用指南](test/README.md)** - 8个独立测试程序的编译、烧录、验证方法
- **[OBS预签名URL识图实施记录](docs/archive/fixes/ai_fixes/obs_presigned_url_ai_solution_record.md)** - 当前 AI 识图方案的实现总结

## 硬件配置

| 外设 | 引脚 | 类型 | 说明 |
|------|------|------|------|
| OV3660 摄像头 | GPIO 0-18 | 并行 DVP | 8位数据总线 |
| SSD1306 OLED | SDA=GPIO 21, SCL=GPIO 47 | I2C | 128x64 分辨率 |
| 舵机 | GPIO 1 | PWM 50Hz | 左摆=其他垃圾(45度)，右摆=可回收垃圾(135度) |
| WS2812 RGB LED | GPIO 48 | 单线通信 | 板载 LED |

### LED 状态指示

| 颜色 | 状态 |
|------|------|
| 蓝色 | 处理中（拍照、上传、识别） |
| 绿色 | 空闲 / 成功 |
| 红色 | 错误 |

## 项目结构

```
AIoT-Project/
├── include/                # 头文件
│   ├── config.h            # 硬件和系统配置
│   ├── constants.h         # 枚举、调试宏、常量
│   ├── secrets.h           # 敏感配置（需自行创建，已 gitignore）
│   └── secrets.h.template  # 配置模板（复制后修改）
│
├── src/                    # 源文件
│   ├── main.ino            # 主程序 + 状态机逻辑
│   ├── camera.cpp          # 摄像头驱动
│   ├── display.cpp         # OLED 显示
│   ├── network.cpp         # WiFi 连接管理 + NTP
│   ├── iotda.cpp           # IoTDA MQTT 通信
│   ├── obs.cpp             # OBS 图片上传 + 签名
│   ├── ai.cpp              # AI 识别 API 调用
│   ├── servo.cpp           # 舵机控制
│   └── led.cpp             # WS2812 LED 控制
│
├── test/                   # 8个独立测试程序
│   ├── layer1_hardware/    # test_led, test_oled, test_servo
│   ├── layer2_camera/      # test_camera
│   ├── layer3_network/     # test_wifi, test_iotda
│   ├── layer4_cloud/       # test_obs, test_ai
│   └── README.md           # 测试使用指南
│
├── docs/                   # 文档
│   ├── system_documentation.md              # 系统完整说明
│   ├── wechat_miniprogram_guide.md          # 微信小程序开发指南
│   ├── guides/cloud_service_config_checklist.md   # 云服务配置清单
│   ├── iotda_product_model_config.md        # IoTDA 配置指南
│   ├── test_plan.md                         # 测试计划
│   ├── phase9_implementation_details.md     # Phase 9 实现细节
│   ├── troubleshooting/                     # 故障排查
│   └── archive/                             # 历史归档
│
├── platformio.ini          # PlatformIO 构建配置
├── README.md               # 本文件（项目说明）
└── CLAUDE.md               # AI 辅助开发规范
```

## 核心模块

| 模块 | 文件 | 功能 |
|------|------|------|
| **网络模块** | `network.h/.cpp` | WiFi 连接管理、多网络优先级选择、自动重连（指数退避）、NTP 时间同步 |
| **摄像头模块** | `camera.h/.cpp` | OV3660 驱动初始化、JPEG 拍照、PSRAM 帧缓冲区管理 |
| **IoTDA 模块** | `iotda.h/.cpp` | MQTT 连接维护、命令订阅与回调、属性上报（状态/结果/错误） |
| **OBS 模块** | `obs.h/.cpp` | AWS Signature V4 签名、虚拟主机风格上传、预签名 URL 生成 |
| **AI 模块** | `ai.h/.cpp` | 支持 Base64 和 URL 两种识图方式、响应 JSON 解析、容错处理 |
| **舵机模块** | `servo.h/.cpp` | PWM 控制（50Hz）、角度映射（45度-135度）、动作时序 |
| **显示模块** | `display.h/.cpp` | OLED 消息/进度/错误/结果显示、网络状态栏 |
| **LED 模块** | `led.h/.cpp` | WS2812 颜色控制、闪烁/呼吸效果 |

## 开发状态

| 阶段 | 状态 | 内容 |
|------|------|------|
| Phase 1-3 | 完成 | 基础框架、摄像头、舵机/LED |
| Phase 4-6 | 完成 | IoTDA、OBS、AI 识别 |
| Phase 7-8 | 完成 | 状态机核心逻辑、云端对接 |
| Phase 9 | 完成 | WiFi/OBS/AI 优化、错误修复、测试框架 |
| Phase 10 | 进行中 | 文档完善、演示准备、微信小程序 |

**编译状态**：通过
- RAM: 16.5% (54,096 / 327,680 字节)
- Flash: 15.8% (1,032,721 / 6,553,600 字节)

**测试状态**：8个独立测试程序全部编译通过，OBS 上传功能测试通过

## 重要提醒

**使用前必须配置 `include/secrets.h`**：

```cpp
// WiFi
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

// 华为云 IoTDA
#define DEVICE_ID "your_device_id"
#define MQTT_SERVER "your_endpoint.iot-mqtts.cn-north-4.myhuaweicloud.com"
#define CLIENT_ID "your_device_id_0_0_2026042001"
#define MQTT_USER "your_device_id"
#define MQTT_PASSWORD "your_mqtt_password_hash"

// 华为云 OBS
#define OBS_ACCESS_KEY "your_access_key"
#define OBS_SECRET_KEY "your_secret_key"
#define OBS_BUCKET_NAME "your_bucket_name"

// AI API（DashScope 示例）
#define AI_API_KEY "your_api_key"
#define AI_API_ENDPOINT "https://dashscope.aliyuncs.com/compatible-mode"
#define AI_MODEL "qwen3.5-flash"
```

配置模板文件为 `include/secrets.h.template`，复制后修改即可。
详细配置步骤请参考 **[云服务配置清单](docs/guides/cloud_service_config_checklist.md)**。

## 故障排查

常见问题及解决方案：

| 问题 | 可能原因 | 解决方式 |
|------|---------|---------|
| WiFi 连接失败 | SSID/密码错误 | 检查 secrets.h 配置 |
| MQTT 连接失败(2) | PubSubClient 库版本 | 使用 v2.7.0（platformio.ini 中指定） |
| MQTT 连接失败(-1) | 数据包超限 | 全局定义 MQTT_MAX_PACKET_SIZE=1024 |
| OBS RequestTimeTooSkewed | NTP 未同步 | 确保 NTP 同步成功后再上传 |
| OBS SignatureDoesNotMatch | 签名算法错误 | 使用二进制 HMAC 派生密钥 |
| OBS VirtualHostDomainRequired | 桶访问风格 | 使用虚拟主机风格 URL |

更多故障排查见 `docs/troubleshooting/` 目录。

## 许可证

课程项目，仅供学习使用。

---

**文档版本**：3.0 | **最后更新**：2026-04-25
