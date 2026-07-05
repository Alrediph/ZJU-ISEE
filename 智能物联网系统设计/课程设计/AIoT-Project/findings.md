# Findings & Decisions

## Requirements

### 功能需求
- 完整的端到端垃圾分类识别系统，用于课程大作业演示
- 二分类系统：可回收垃圾、其他垃圾
- 使用OV3660摄像头拍摄垃圾图像
- 上传图片到华为云OBS对象存储
- 调用OpenAI兼容格式的国内AI API进行识别
- 驱动两个舵机模拟打开对应垃圾桶（简单开合动作）
- OLED显示屏显示状态和识别结果
- LED指示灯反馈系统状态（处理中/成功/失败）
- 微信小程序通过华为云IoTDA远程触发识别
- 基本错误提示功能

### 硬件需求
- ESP32-S3-CAM开发板
- OV3660摄像头模块
- SSD1306 OLED显示屏（128x64）
- 舵机 x 2
- LED灯（红、绿、蓝）
- 已有硬件，配置齐全

### 云服务需求
- 华为云IoTDA：设备接入和远程控制（已配置）
- 华为云OBS：图片存储服务（已配置）
- 国内AI API：图像识别服务（已有密钥）

### 网络需求
- 普通WiFi连接（家庭/实验室WiFi）

## Research Findings

### ESP32-S3-CAM硬件特性
- **PSRAM**: ESP32-S3配备8MB PSRAM，用于存储大对象（图像缓冲区）
- **摄像头引脚**: OV3660摄像头占用GPIO 0-18的大部分引脚
- **可用GPIO**: GPIO 1, 3, 19-21, 26-48（具体因开发板型号而异）
- **注意**: 必须查阅实际开发板原理图确认引脚分配

### 摄像头配置
- 使用ESP32 Camera Driver库
- 图像格式：JPEG（减少传输数据量）
- 分辨率：SVGA (800x600) 或 VGA (640x480)
- 帧缓冲区：存储在PSRAM，避免内部SRAM不足
- 内存管理：拍照后立即使用，用完释放帧缓冲区
- **重要发现**（参考reference/Arduino_Demo示例代码）：
  - 必须检测PSRAM是否存在：`psramFound()`
  - 如果有PSRAM：使用`CAMERA_GRAB_LATEST`模式，`fb_count=2`，高质量JPEG
  - 如果无PSRAM：使用`CAMERA_GRAB_WHEN_EMPTY`模式，`fb_count=1`，帧缓冲区在DRAM
  - 必须启用垂直翻转：`s->set_vflip(s, 1)`，否则图像是倒的
  - 亮度建议设置为1（稍微提高），饱和度建议设置为0（降低一点）

### 华为云IoTDA集成
- MQTT协议连接（参考代码已有实现）
- 端口：1883（非加密）或 8883（TLS加密）
- 认证：设备ID + 设备密钥
- 订阅主题：`$oc/devices/{device_id}/sys/commands/request/id/+`
- 发布主题：`$oc/devices/{device_id}/sys/properties/report`
- 自动重连机制：指数退避策略（参考代码已实现）

### 华为云OBS上传
- HTTP PUT方法上传
- 签名认证：AWS Signature Version 4（华为云OBS兼容）
- 请求头：Content-Type、Date、Authorization
- 文件命名：`garbage_{timestamp}_{random}.jpg`
- URL有效期：1小时（带签名）

### AI识别API
- 格式：OpenAI Chat Completions API兼容
- HTTP POST请求
- 请求体：模型名、图片URL、识别提示词
- 响应解析：提取分类结果和置信度
- 国内服务商选项：智谱GLM-4V、通义千问VL、百度文心一言等

### 舵机控制
- PWM信号控制角度
- 频率：50Hz
- 分辨率：16位
- 角度映射：0度=500μs脉冲，90度=2500μs脉冲
- 动作时序：开500ms → 保持1000ms → 关500ms

### 参考代码分析
**reference/HuaweiCloud_ESP32.ino 提供了：**
- WiFi连接管理（普通WiFi和企业级WiFi）
- MQTT连接和自动重连（PubSubClient库）
- OLED显示（Adafruit_SSD1306 + U8g2字体）
- BMP280传感器读取（可忽略）
- 三色LED控制（RGB LED）
- 光敏电阻读取（可忽略）
- 看门狗配置
- NTP时间同步
- 命令下发处理和响应

**可复用的代码：**
- WiFi连接逻辑
- MQTT连接和回调函数框架
- OLED显示函数
- LED控制函数
- 看门狗配置

## Technical Decisions

| Decision | Rationale |
|----------|-----------|
| 状态机驱动架构 | 流程清晰易追踪，便于调试和演示，适合课程项目规模，易于扩展（后续可添加红外传感器） |
| 二分类系统 | 简化演示复杂度，降低开发难度，满足课程要求，减少识别错误 |
| PlatformIO + Arduino框架 | 参考代码使用Arduino框架，PlatformIO提供更好的依赖管理和项目结构 |
| 仅上传OBS不本地存储 | 简化存储管理，避免SD卡依赖，降低硬件复杂度，节省开发时间 |
| 国内AI API（非OpenAI官方） | 访问更稳定、延迟更低、价格更便宜、符合国内使用环境 |
| PSRAM存储图像缓冲区 | ESP32-S3有8MB PSRAM，内部SRAM仅512KB，避免内存不足 |
| 基本错误提示 | 满足演示需求，降低开发复杂度，避免过度工程化 |
| 简单开合舵机动作 | 简化控制逻辑，易于演示，满足课程要求 |
| GPIO 19-48用于外设 | 摄像头占用GPIO 0-18，避免引脚冲突 |
| SVGA分辨率（800x600） | 平衡图像质量和传输速度，适合AI识别 |

## Issues Encountered

| Issue | Resolution |
|-------|------------|
| GPIO引脚冲突（LED引脚与摄像头引脚冲突） | 将LED和舵机改用GPIO 19-48范围，具体引脚需根据实际开发板原理图确定 |
| 参考代码功能过多（BMP280、光敏电阻、企业级WiFi） | 提取需要的部分（WiFi、MQTT、OLED、LED），忽略不需要的传感器代码 |
| 设计文档中的引脚定义为示例值 | 在文档中添加重要提示，强调必须根据实际开发板调整引脚配置 |
| secrets.h包含敏感信息 | 添加到.gitignore，创建secrets.h.template作为模板 |
| 测试程序中mqttClient变量不可访问 | 使用iotda.h提供的公共接口（printMQTTStatus）代替直接访问内部变量 |

## Test Program Development Findings

### 测试架构设计
- **分层测试架构**：从硬件到云端逐层递进，每层独立验证
  - Layer 1: 硬件基础层（LED、OLED、舵机）- 无依赖
  - Layer 2: 摄像头层 - 依赖硬件
  - Layer 3: 网络通信层（WiFi、IoTDA）- 依赖网络
  - Layer 4: 云服务层（OBS、AI）- 依赖云服务
- **独立测试程序**：每个测试程序完全独立，有自己的main.cpp
- **共享配置**：通过PlatformIO多环境管理，共享依赖库和头文件
- **解耦设计**：模块间交互使用预置数据，避免依赖其他模块

### PlatformIO配置发现
- **build_src_filter语法**：
  - 使用`+<file.cpp>`包含源文件
  - 使用`+<../test/path/main.cpp>`包含测试程序
  - 多个文件用空格分隔
- **环境继承**：使用`extends = env:esp32-s3-common`继承通用配置
- **依赖管理**：测试环境自动继承通用环境的lib_deps

### 编译验证结果
- **内存使用**：
  - Layer 1测试：RAM 9-13%, Flash 8-11%
  - Layer 2测试：RAM 13-14%, Flash 11-12%
  - Layer 3测试：RAM 9-14%, Flash 8-12%
  - Layer 4测试：RAM 14%, Flash 13-14%
- **编译时间**：首次编译20-55秒，后续增量编译更快
- **所有测试程序编译成功**：无错误、无警告

### 测试程序实现要点
- **预置数据**：硬编码在测试程序中，避免文件依赖
- **验证方式**：
  - 硬件测试：人工观察LED/OLED/舵机动作
  - 软件测试：串口输出日志判断
- **错误处理**：初始化失败时输出错误并停止
- **模块依赖**：通过build_src_filter包含必要的源文件

### 关键技术决策
| Decision | Rationale |
|----------|-----------|
| 分层测试架构 | 便于问题定位，从简单到复杂逐层验证 |
| 独立测试程序 | 避免相互干扰，每个测试可独立运行 |
| 预置数据硬编码 | 简化实现，避免文件I/O复杂度 |
| PlatformIO多环境 | 统一管理测试程序，共享配置和依赖 |
| 串口输出验证 | 便于调试，无需额外测试框架 |

## Resources

### 官方文档
- ESP32 Camera Driver: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/camera.html
- OV3660数据手册: https://www.ovt.com/sensors/ov3660/
- 华为云IoTDA设备接入: https://support.huaweicloud.com/devg-iothub/iot_02_1002.html
- 华为云OBS API: https://support.huaweicloud.com/api-obs/obs_04_0001.html

### Arduino库文档
- Adafruit SSD1306: https://github.com/adafruit/Adafruit_SSD1306
- PubSubClient (MQTT): https://pubsubclient.knolleary.net/
- ArduinoJson: https://arduinojson.org/

### 参考代码
- `reference/HuaweiCloud_ESP32.ino` - 华为云IoTDA集成完整示例

### 设计文档
- `docs/superpowers/specs/2026-04-18-esp32-garbage-classification-design.md` - 完整设计规格
- `README.md` - 详细架构文档
- `CLAUDE.md` - 项目概况

## Visual/Browser Findings
（暂无，开发过程中遇到可视化内容时补充）

---

## Implementation Findings (Phase 9)

Phase 9的关键技术发现已整合到详细文档中：

- **OBS图片上传功能实现**：华为云OBS虚拟主机风格、AWS Signature V4签名算法
- **WiFi功能重构**：职责分离、多WiFi支持、自动重连
- **IoTDA连接修复**：MQTT连接参数、配置完善
- **AI识别优化**：Base64编码、流程优化

**详细文档**: [docs/phase9_implementation_details.md](docs/phase9_implementation_details.md)

**⚠️ 重要提示**: 更新Phase 9技术发现时，请同步更新 [docs/phase9_implementation_details.md](docs/phase9_implementation_details.md)

---

*Update this file after every 2 view/browser/search operations*
*This prevents visual information from being lost*
