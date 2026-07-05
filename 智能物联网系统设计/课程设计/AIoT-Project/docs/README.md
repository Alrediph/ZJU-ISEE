# 项目文档导航

本索引提供项目文档的快速导航，帮助您快速找到所需信息。

## 💻 系统实现文档

详细介绍系统各模块的实现逻辑和代码解析，适用于课程答辩和技术深入理解。

| 文档 | 说明 |
|------|------|
| [系统实现总览](implementation/system_implementation.md) | 总体架构、状态机设计、数据流、模块依赖关系 |
| [01 - 主程序与状态机](implementation/modules/01_state_machine.md) | main.ino — 初始化流程、状态转移、主循环逻辑 |
| [02 - WiFi 网络模块](implementation/modules/02_network.md) | network.cpp — 多WiFi选择、NTP同步、重连机制 |
| [03 - 摄像头模块](implementation/modules/03_camera.md) | camera.cpp — OV3660驱动、PSRAM帧缓冲、拍照流程 |
| [04 - 显示与LED模块](implementation/modules/04_display_led.md) | display.cpp + led.cpp — OLED显示、WS2812 LED |
| [05 - 舵机控制模块](implementation/modules/05_servo.md) | servo.cpp — PWM舵机控制、垃圾分类动作 |
| [06 - IoTDA 通信模块](implementation/modules/06_iotda.md) | iotda.cpp — MQTT通信、命令处理、属性上报 |
| [07 - OBS 存储模块](implementation/modules/07_obs.md) | obs.cpp — AWS SigV4签名、图片上传、预签名URL |
| [08 - AI 识别模块](implementation/modules/08_ai.md) | ai.cpp — DashScope API、提示词设计、结果解析 |

### 推荐阅读顺序

1. **系统实现总览** → 了解整体架构和状态机设计
2. **01 主程序与状态机** → 理解核心调度逻辑
3. **06 IoTDA / 07 OBS / 08 AI** → 深入云服务模块
4. **02 网络 / 03 摄像头 / 04 显示LED / 05 舵机** → 理解硬件相关模块

