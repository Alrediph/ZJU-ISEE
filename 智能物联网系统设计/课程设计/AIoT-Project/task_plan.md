# Task Plan: ESP32-S3-CAM 智能垃圾分类识别系统开发

## Goal
构建一个完整的端到端垃圾分类识别系统，实现从图像采集、云端AI识别到硬件控制的完整流程，用于课程大作业演示。

## Current Phase
Phase 10

## Phases

### Phase 1: 项目基础框架搭建
- [x] 创建PlatformIO项目结构
- [x] 配置platformio.ini（ESP32-S3、PSRAM、依赖库）
- [x] 创建配置文件模板（config.h、secrets.h、constants.h）
- [x] 实现基础WiFi连接功能
- [x] 实现OLED显示基础功能（参考reference代码）
- [x] 测试基础框架是否正常工作
- **Status:** complete

### Phase 2: 摄像头模块集成
- [x] 配置OV3660摄像头驱动（PSRAM、引脚定义）
- [x] 实现CameraModule类（init、capture、release）
- [x] 测试摄像头拍照功能
- [x] 验证PSRAM使用和内存管理
- [x] 优化图像质量和分辨率参数
- **Status:** complete

### Phase 3: 舵机和LED控制
- [x] 实现ServoModule类（init、openBin、closeAll）
- [x] 实现LED控制功能（红、绿、蓝指示）
- [x] 测试舵机开合动作和时序
- [x] 调整PWM参数和动作时序（500ms开、1s保持、500ms关）
- [x] 测试LED指示功能
- **Status:** complete

### Phase 4: 华为云IoTDA集成
- [x] 实现CloudService类（MQTT连接、命令接收）
- [x] 配置IoTDA设备信息（设备ID、密钥）
- [x] 实现命令下发处理逻辑
- [x] 实现状态上报功能
- [x] 测试WiFi和MQTT自动重连机制
- [x] 测试命令下发和响应
- [x] IoTDA连接修复（2026-04-20）
  - 修复MQTT连接参数错误
  - 添加CLIENT_ID和MQTT_USER配置
  - 修正connect()调用参数顺序
  - 修复函数重复定义问题
  - **详细文档**: [docs/archive/fixes/iotda_fixes/iotda_connection_fix_record.md](docs/archive/fixes/iotda_fixes/iotda_connection_fix_record.md)

- [x] IoTDA数据构造优化（2026-04-20）
  - 采用外部定义与宏定义方式
  - 添加SERVICE_ID和MQTT_BODY_FORMAT定义
  - MQTT主题改用宏定义（移除String拼接）
  - 改进数据构造函数（reportStatus、reportResult、reportError）
  - **详细文档**: [docs/archive/fixes/iotda_fixes/iotda_data_construction_optimization.md](docs/archive/fixes/iotda_fixes/iotda_data_construction_optimization.md)
- **Status:** complete

### Phase 5: 华为云OBS存储集成
- [x] 实现StorageService类（上传图片到OBS）
- [x] 实现OBS签名认证（AK/SK）
- [x] 实现图片URL生成功能
- [x] 测试图片上传到OBS
- [x] 测试URL可访问性
- [x] 优化上传性能和错误处理
- **Status:** complete

### Phase 6: AI识别服务集成
- [x] 实现AIRecognition类（调用OpenAI兼容API）
- [x] 构造识别请求JSON格式
- [x] 实现响应解析逻辑（提取分类和置信度）
- [x] 测试API调用和识别准确率
- [x] 优化识别提示词以提高准确率
- [x] 测试错误处理（超时、认证失败等）
- **Status:** complete

### Phase 7: 状态机核心逻辑
- [x] 实现StateMachine类
- [x] 实现所有状态处理函数（IDLE、CAPTURE、UPLOAD、RECOGNIZE、EXECUTE、ERROR）
- [x] 实现状态转换逻辑
- [x] 集成所有模块（摄像头、存储、识别、舵机、显示）
- [x] 实现错误处理和恢复机制
- [x] 测试完整流程
- **Status:** complete

### Phase 8: 微信小程序对接
- [x] 配置华为云IoTDA产品模型（设备属性和命令定义）
- [x] 实现设备端历史记录存储功能（OBS records/文件夹）
- [x] 实现设备端历史记录管理（简化方案：设备端只上传，小程序端负责列出）
- [ ] 开发微信小程序基础框架（跳过，后续处理）
- [ ] 实现小程序拍照识别功能（跳过，后续处理）
- [ ] 实现小程序历史记录功能（跳过，后续处理）
- [x] 测试端到端流程（使用控制台测试）
- [x] 编写IoTDA产品模型配置文档
- **Status:** complete
- **Note:** 微信小程序开发推迟到后续阶段，当前使用华为云IoTDA控制台测试命令下发和结果上报

### Phase 9: 测试和优化
- [x] 基础实现优化（参考HuaweiCloud_ESP32.ino）
  - WiFi/MQTT自动重连机制
  - 看门狗和稳定性优化
  - OLED显示优化
  - 调试日志优化
  - 多WiFi网络支持
  - 主循环优化
  - **详细文档**: [docs/task_plan_optimization.md](docs/task_plan_optimization.md)
  - **进度记录**: [docs/progress_optimization.md](docs/progress_optimization.md)

- [x] 测试程序开发（Phase 9.2）
  - Layer 1 硬件基础层测试（LED、OLED、舵机）
  - Layer 2 摄像头层测试
  - Layer 3 网络通信层测试（WiFi、IoTDA）
  - Layer 4 云服务层测试（OBS、AI）
  - **详细文档**: [docs/test_plan.md](docs/test_plan.md)
  - **使用指南**: [test/README.md](test/README.md)

- [x] WiFi功能重构（Phase 9.3）
  - 分离网络功能与显示逻辑
  - 实现多WiFi智能选择
  - 实现自动重连 + 指数退避策略
  - 实现NTP时间同步
  - **详细文档**: [docs/archive/fixes/wifi_fixes/wifi_refactor_summary.md](docs/archive/fixes/wifi_fixes/wifi_refactor_summary.md)

- [x] OBS图片上传功能修复（Phase 9.4）
  - 修复时间偏差问题（RequestTimeTooSkewed）
  - 修复区域配置不一致问题
  - 修复签名不匹配问题（SignatureDoesNotMatch）
  - 修复无效桶名称问题（InvalidBucketName）
  - 修复虚拟主机域名必需问题（VirtualHostDomainRequired）
  - 实现符合AWS Signature Version 4规范的签名算法
  - 发现华为云OBS强制使用虚拟主机风格
  - **详细文档**: [docs/archive/fixes/obs_fixes/obs_implementation_complete_record.md](docs/archive/fixes/obs_fixes/obs_implementation_complete_record.md)
  - **文档索引**: [docs/archive/obs_document_index.md](docs/archive/obs_document_index.md)

- [x] AI识别流程优化（Phase 9.5）
  - 使用Base64编码本地图片
  - 调整状态机流程：CAPTURE → RECOGNIZE → UPLOAD → EXECUTE
  - 降低AI识别与OBS上传的耦合度
  - **详细文档**: [docs/ai_recognition_optimization_record.md](docs/ai_recognition_optimization_record.md)

- [x] IoTDA连接修复（Phase 9.6）
  - 修复MQTT连接参数错误
  - 添加CLIENT_ID和MQTT_USER配置
  - 修正connect()调用参数顺序
  - **详细文档**: [docs/archive/fixes/iotda_fixes/iotda_connection_fix_record.md](docs/archive/fixes/iotda_fixes/iotda_connection_fix_record.md)

- [x] 集成测试（完整流程测试）
- [x] 异常测试（网络断开、API失败等）
- [x] 性能测试（内存泄漏检测）
- [x] 稳定性测试（连续运行测试）
- [x] Bug修复和优化
- **Status:** complete
- **Note:** 基础优化已完成（2026-04-20），IoTDA连接修复完成（2026-04-20），测试程序开发完成（2026-04-20），WiFi重构完成（2026-04-22），OBS修复完成（2026-04-22），AI优化完成（2026-04-21），集成测试完成（2026-04-25），Bug修复（2026-04-25）：舵机错误码+IoTDA命令响应缺失

### Phase 10: 文档和演示准备
- [x] 完善README文档
- [x] 编写微信小程序开发说明文档
- [ ] 编写配置指南
- [ ] 准备演示材料（PPT、视频）
- [ ] 录制演示视频
- [ ] 最终测试和验收
- **Status:** in_progress

## Key Questions
1. GPIO引脚配置是否与ESP32-S3-CAM开发板实际引脚图匹配？（需查阅开发板原理图）
2. OV3660摄像头驱动是否兼容ESP32-S3？（需要测试）
3. 国内AI API服务哪个最适合垃圾分类识别？（智谱GLM-4V、通义千问VL等）
4. 舵机控制PWM频率和时序参数是否合理？（需要实际测试调整）
5. 华为云IoTDA和OBS的认证方式？（AK/SK签名认证）
6. 如何处理网络不稳定的情况？（自动重连+重试机制）
7. 内存管理策略？（PSRAM存储图像，及时释放帧缓冲区）

## Decisions Made
| Decision | Rationale |
|----------|-----------|
| 状态机驱动架构 | 流程清晰易追踪，便于调试和演示，适合课程项目规模 |
| 二分类系统 | 简化演示复杂度，降低开发难度，满足课程要求 |
| 仅上传OBS不本地存储 | 简化存储管理，避免SD卡依赖，降低复杂度 |
| 国内AI API（非OpenAI） | 访问更稳定，延迟更低，价格更便宜 |
| PlatformIO + Arduino框架 | 参考代码使用Arduino，PlatformIO更好用的依赖管理 |
| PSRAM存储图像缓冲区 | ESP32-S3有8MB PSRAM，避免内部SRAM不足 |
| 基本错误提示而非完整错误处理 | 满足演示需求，降低开发复杂度 |

## Errors Encountered
| Error | Attempt | Resolution |
|-------|---------|------------|
| GPIO引脚冲突 | 1 | 检查ESP32-S3-CAM引脚图，摄像头占用GPIO 0-18，LED和舵机需使用GPIO 19-48 |

## Notes
- 开发顺序：硬件基础 → 摄像头 → 舵机LED → 云服务 → AI → 状态机 → 小程序 → 测试
- 参考代码：reference/HuaweiCloud_ESP32.ino 提供了WiFi、MQTT、OLED的基础实现
- 关键风险：GPIO引脚冲突、AI识别准确率、网络稳定性
- 预计总时间：9-12天
- 每个阶段完成后立即更新此文件的Status
- 重要：在开始Phase 1前，必须确认ESP32-S3-CAM开发板的实际引脚图
