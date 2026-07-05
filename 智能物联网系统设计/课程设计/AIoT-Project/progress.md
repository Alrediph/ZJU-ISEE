# Progress Log

## Session: 2026-04-18

### Phase 0: 项目设计和规划
- **Status:** complete
- **Started:** 2026-04-18 15:30
- Actions taken:
  - 与用户讨论项目需求和目标
  - 确认项目类型：完整演示系统，用于课程大作业
  - 确认分类类型：二分类（可回收垃圾/其他垃圾）
  - 确认硬件配置：ESP32-S3-CAM + OV3660 + OLED + 舵机x2 + LED
  - 确认云服务：华为云IoTDA（已配置）、华为云OBS（已配置）、国内AI API（已有密钥）
  - 确认网络环境：普通WiFi
  - 确认触发方式：微信小程序远程控制（通过IoTDA）
  - 确认存储方案：仅上传OBS，不本地存储
  - 确认执行流程：简单直接（识别后立即驱动舵机）
  - 确认错误处理：基本错误提示
  - 确认舵机控制：简单开合（90度旋转，自动复位）
  - 提出3种架构方案（模块化分层、状态机驱动、事件驱动）
  - 用户选择：状态机驱动架构
  - 展示详细设计方案（4个部分，用户全部确认）
  - 编写设计文档到CLAUDE.md、README.md、docs/superpowers/specs/
  - 进行设计文档自我审查，修正引脚冲突问题
  - 创建开发计划文件（task_plan.md、findings.md、progress.md）
- Files created/modified:
  - CLAUDE.md (创建)
  - README.md (创建)
  - docs/superpowers/specs/2026-04-18-esp32-garbage-classification-design.md (创建)
  - task_plan.md (创建)
  - findings.md (创建)
  - progress.md (创建)

### Phase 1: 项目基础框架搭建
- **Status:** complete
- **Started:** 2026-04-18 17:00
- **Completed:** 2026-04-18 17:40
- Actions taken:
  - 确认GOOUUU ESP32-S3-CAM引脚配置
  - 更新设计文档中的GPIO引脚定义（避免冲突）
  - 配置platformio.ini（ESP32-S3、PSRAM、依赖库）
  - 创建配置文件（config.h、constants.h、secrets.h、secrets.h.template）
  - 实现OLED显示模块（display.h、display.cpp）
  - 实现LED控制模块（led.h、led.cpp，支持WS2812 RGB LED）
  - 实现基础WiFi连接功能
  - 实现状态机框架（main.ino）
  - 编译测试通过
- Files created/modified:
  - platformio.ini (更新依赖库配置)
  - include/config.h (创建，包含正确的引脚配置)
  - include/constants.h (创建，包含状态枚举和调试宏)
  - include/secrets.h (创建，敏感配置)
  - include/secrets.h.template (创建，配置模板)
  - include/display.h (创建，OLED显示接口)
  - include/led.h (创建，LED控制接口)
  - src/display.cpp (创建，OLED显示实现)
  - src/led.cpp (创建，LED控制实现)
  - src/main.ino (创建，主程序和状态机框架)
  - .gitignore (更新，添加secrets.h)
  - docs/superpowers/specs/2026-04-18-esp32-garbage-classification-design.md (更新引脚配置)

### Phase 2: 摄像头模块集成
- **Status:** complete
- **Started:** 2026-04-18 18:00
- **Completed:** 2026-04-18 18:15
- Actions taken:
  - 创建摄像头模块接口（camera.h）
  - 实现摄像头驱动配置（camera.cpp）
  - 配置OV3660摄像头使用PSRAM存储帧缓冲区
  - 实现initCamera()、capturePhoto()、releasePhoto()函数
  - 集成摄像头模块到主程序
  - 修复编译错误（添加Arduino.h、更新deprecated引脚名称）
  - 编译测试通过
- Files created/modified:
  - include/camera.h (创建，摄像头接口)
  - src/camera.cpp (创建，摄像头实现)
  - src/main.ino (更新，集成摄像头模块)
  - task_plan.md (更新Phase 2状态为complete)
  - progress.md (添加Phase 2工作日志)
  - findings.md (添加摄像头配置关键发现)
  - record/phase2_implementation.md (创建，详细实现记录)

### Phase 3: 舵机和LED控制
- **Status:** complete
- **Started:** 2026-04-18 19:00
- **Completed:** 2026-04-18 19:30
- Actions taken:
  - 创建舵机模块接口（servo.h）
  - 实现舵机驱动配置（servo.cpp）
  - 配置ESP32Servo库，使用GPIO 45和47控制两个舵机
  - 实现initServo()、openBin()、closeAllBins()、testServoAction()函数
  - LED控制功能已在Phase 1中完成
  - 集成舵机模块到主程序
  - 修复编译错误（修正display函数调用、更新ESP32Servo库名称）
  - 编译测试通过
- Files created/modified:
  - include/servo.h (创建，舵机接口)
  - src/servo.cpp (创建，舵机实现)
  - platformio.ini (更新，添加ESP32Servo库依赖)
  - src/main.ino (更新，集成舵机模块)

### Phase 4: 华为云IoTDA集成
- **Status:** complete
- **Started:** 2026-04-18 20:00
- **Completed:** 2026-04-18 20:30
- Actions taken:
  - 创建IoTDA模块接口（iotda.h）
  - 实现MQTT连接和自动重连（iotda.cpp）
  - 配置IoTDA设备信息（设备ID、密钥）
  - 实现命令下发处理逻辑（mqttCallback）
  - 实现状态上报功能（reportStatus、reportResult、reportError）
  - 集成IoTDA模块到主程序
  - 修复编译错误（前置声明、String拼接主题、更新secrets.h配置）
  - 编译测试通过
- Files created/modified:
  - include/iotda.h (创建，IoTDA接口)
  - src/iotda.cpp (创建，IoTDA实现)
  - include/secrets.h (更新，添加MQTT配置参数)
  - include/secrets.h.template (更新，添加MQTT配置模板)
  - src/main.ino (更新，集成IoTDA模块)

### Phase 5: 华为云OBS存储集成
- **Status:** complete
- **Started:** 2026-04-18 21:00
- **Completed:** 2026-04-18 21:30
- Actions taken:
  - 创建OBS模块接口（obs.h）
  - 实现OBS上传功能（obs.cpp）
  - 实现AWS Signature Version 4签名认证
  - 实现HMAC-SHA256和SHA256哈希计算
  - 实现图片上传到OBS功能
  - 实现URL生成功能
  - 集成OBS模块到主程序
  - 编译测试通过（RAM: 15.4%, Flash: 12.8%）
- Files created/modified:
  - include/obs.h (创建，OBS接口)
  - src/obs.cpp (创建，OBS实现)
  - include/secrets.h (更新，添加OBS_REGION配置)
  - include/secrets.h.template (更新，添加OBS_REGION模板)
  - src/main.ino (更新，集成OBS模块)

### Phase 6: AI识别服务集成
- **Status:** complete
- **Started:** 2026-04-18 21:35
- **Completed:** 2026-04-18 22:00
- Actions taken:
  - 创建AI识别模块接口（ai.h）
  - 实现AI识别功能（ai.cpp）
  - 实现OpenAI兼容API调用
  - 构造垃圾分类识别提示词
  - 实现响应解析逻辑（提取分类和置信度）
  - 添加容错处理（从文本中提取类型）
  - 集成AI模块到主程序
  - 编译测试通过（RAM: 15.4%, Flash: 12.9%）
- Files created/modified:
  - include/ai.h (创建，AI识别接口)
  - src/ai.cpp (创建，AI识别实现)
  - src/main.ino (更新，集成AI模块)

### Phase 7: 状态机核心逻辑
- **Status:** complete
- **Started:** 2026-04-18 22:05
- **Completed:** 2026-04-18 22:30
- Actions taken:
  - 实现完整的状态机逻辑
  - 实现所有状态处理函数（IDLE、CAPTURE、UPLOAD、RECOGNIZE、EXECUTE、ERROR）
  - 实现状态转换逻辑和模块协调
  - 集成摄像头、OBS、AI、舵机模块
  - 实现MQTT命令触发机制
  - 实现错误处理和资源清理
  - 编译测试通过（RAM: 15.9%, Flash: 15.3%）
- Files created/modified:
  - src/main.ino (更新，实现完整状态机逻辑)
  - src/iotda.cpp (更新，实现命令触发机制)

### Phase 8: 微信小程序对接
- **Status:** complete
- **Started:** 2026-04-18 22:35
- **Completed:** 2026-04-18 23:15
- Actions taken:
  - 创建IoTDA产品模型配置文档
  - 定义设备属性（Status、GarbageType、Confidence、Error）
  - 定义设备命令（capture、test）
  - 实现设备端历史记录存储功能
  - 添加uploadRecordToOBS函数，上传识别记录JSON到records/文件夹
  - 修改状态机，在识别完成后自动上传识别记录
  - 编译测试通过（RAM: 15.9%, Flash: 15.4%）
  - 微信小程序开发推迟到后续阶段
- Files created/modified:
  - docs/iotda_product_model_config.md (创建，IoTDA配置指南)
  - include/obs.h (更新，添加uploadRecordToOBS函数声明)
  - src/obs.cpp (更新，实现uploadRecordToOBS函数)
  - src/main.ino (更新，添加历史记录上传逻辑)
  - task_plan.md (更新，标记Phase 8完成)
  - progress.md (更新，添加Phase 8工作日志)

### Phase 8.1: OBS记录上传方案修正
- **Status:** complete
- **Started:** 2026-04-18 23:30
- **Completed:** 2026-04-18 23:50
- Actions taken:
  - 分析当前实现与用户需求的偏差
  - 创建修正计划文档（docs/obs_record_upload_fix_plan.md）
  - 移除设备端uploadRecordToOBS函数实现和声明
  - 扩展reportResult函数，添加更多属性字段（ImageUrl、ObjectName、Timestamp）
  - 在config.h中添加OBS配置常量（OBS_IMAGE_PATH、OBS_RECORD_PATH）
  - 修改状态机，移除记录上传步骤
  - 更新IoTDA产品模型文档，添加新属性定义和规则引擎配置说明
  - 更新README.md架构图，说明IoTDA规则引擎转发流程
  - 编译测试通过（RAM: 15.9%, Flash: 15.3%）
- Files created/modified:
  - docs/obs_record_upload_fix_plan.md (创建，修正计划)
  - include/config.h (更新，添加OBS路径配置)
  - include/iotda.h (更新，扩展reportResult函数签名)
  - src/iotda.cpp (更新，实现扩展的reportResult函数)
  - include/obs.h (更新，移除uploadRecordToOBS函数声明)
  - src/obs.cpp (更新，移除uploadRecordToOBS函数实现)
  - src/main.ino (更新，移除记录上传步骤，更新reportResult调用)
  - docs/iotda_product_model_config.md (更新，添加新属性和规则引擎配置)
  - README.md (更新，更新架构说明)
- Key changes:
  - 设备端不再直接上传识别记录到OBS
  - 设备只上报属性到IoTDA
  - IoTDA规则引擎自动转发属性到OBS的CSV文件
  - 减少设备端网络请求次数
  - 利用IoTDA规则引擎实现数据持久化

## Session: 2026-04-20

### Phase 9: 测试和优化
- **Status:** complete
- **Started:** 2026-04-20
- **Completed:** 2026-04-22
- Summary:
  - Phase 9.1: 基础实现优化（WiFi/MQTT自动重连、看门狗、多WiFi支持）
  - Phase 9.2: 测试程序开发（8个独立测试程序，分层测试架构）
  - Phase 9.3: WiFi功能重构（职责分离、多WiFi、自动重连、NTP同步）
  - Phase 9.4: OBS图片上传功能修复（5个问题修复、虚拟主机风格、签名算法）
  - Phase 9.5: AI识别流程优化（Base64编码、流程优化、降低耦合）
  - Phase 9.6: IoTDA连接修复（MQTT连接参数修复、配置完善）
- Compilation results:
  - 所有模块编译成功（RAM: 15-16%, Flash: 15%）
- Test results:
  - OBS图片上传功能测试通过 ✅
- Key achievements:
  - 完成系统稳定性优化
  - 实现完整的测试框架
  - 修复所有关键问题（OBS上传、IoTDA连接）
  - 优化系统架构（WiFi重构、AI流程优化）
- **详细文档**: [docs/phase9_implementation_details.md](docs/phase9_implementation_details.md)
- **⚠️ 重要提示**: 更新Phase 9内容时，请同步更新 [docs/phase9_implementation_details.md](docs/phase9_implementation_details.md)


  - test/layer4_cloud/test_obs/main.cpp (创建，OBS测试程序)
  - test/layer4_cloud/test_ai/main.cpp (创建，AI测试程序)
  - platformio.ini (更新，添加所有测试环境配置)
  - test/README.md (创建，测试使用指南)
  - task_plan.md (更新，标记Phase 9.2完成)
- Compilation results:
  - test_led: SUCCESS (RAM: 9.2%, Flash: 8.3%)
  - test_oled: SUCCESS (RAM: 9.3%, Flash: 8.5%)
  - test_servo: SUCCESS (RAM: 13.4%, Flash: 11.2%)
  - test_camera: SUCCESS (RAM: 13.8%, Flash: 11.5%)
  - test_wifi: SUCCESS (RAM: 9.4%, Flash: 8.6%)
  - test_iotda: SUCCESS (RAM: 13.8%, Flash: 11.5%)
  - test_obs: SUCCESS (RAM: 14.1%, Flash: 13.6%)
  - test_ai: SUCCESS (RAM: 14.1%, Flash: 13.5%)
- Key achievements:
  - 8个独立测试程序全部实现完成
  - 所有测试程序编译验证通过
  - 完整的测试使用指南文档
  - 清晰的测试流程和验证方式
- **使用指南**: [test/README.md](test/README.md)
- **详细文档**: [docs/test_plan.md](docs/test_plan.md)

### Phase 9.3: WiFi功能重构
- **Status:** complete
- **Started:** 2026-04-22 00:00
- **Completed:** 2026-04-22 00:30
- Actions taken:
  - 分析WiFi功能代码结构，识别重构目标
  - 设计重构方案：分离网络功能与显示逻辑
  - 修改network.h，新增高级WiFi功能接口
  - 修改network.cpp，实现多WiFi智能选择等功能
  - 修改main.ino，删除WiFi实现，保留显示逻辑
  - 更新test_wifi测试程序，支持多WiFi测试
  - 编译验证主程序和测试程序
  - 创建Git提交，记录重构改动
  - 更新项目文档（system_documentation.md）
- Files created/modified:
  - include/network.h (更新，新增10个函数声明)
  - src/network.cpp (更新，新增约200行WiFi功能实现)
  - src/main.ino (更新，删除约180行WiFi实现，添加network.h引用)
  - test/layer3_network/test_wifi/main.cpp (更新，增强测试覆盖)
  - docs/wifi_refactor_analysis.md (创建，风险分析文档)
  - docs/wifi_refactor_design_v2.md (创建，重构设计方案)
  - docs/system_documentation.md (更新，添加WiFi模块说明和重构记录)
- Compilation results:
  - esp32-s3-devkitc-1: SUCCESS (RAM: 16.3%, Flash: 15.2%)
  - test_wifi: SUCCESS (RAM: 13.7%, Flash: 10.9%)
- Key achievements:
  - 网络功能与显示逻辑完全分离
  - network模块不依赖display.h
  - 测试程序可直接使用多WiFi功能
  - 代码职责更清晰，易于维护和扩展
  - 支持多WiFi智能选择（按优先级）
  - 实现自动重连 + 指数退避策略
- **详细文档**: [docs/wifi_refactor_design_v2.md](docs/wifi_refactor_design_v2.md)

### Phase 9.4: OBS图片上传功能修复与优化
- **Status:** complete
- **Started:** 2026-04-22 01:00
- **Completed:** 2026-04-22 09:30
- Actions taken:
  - 修复时间偏差问题（RequestTimeTooSkewed）
    - 在测试程序中添加NTP时间同步步骤
    - 确保请求时间在服务器时间的±15分钟范围内
  - 修复区域配置不一致问题
    - 修改OBS_REGION为"cn-east-3"，与OBS_ENDPOINT一致
  - 修复签名不匹配问题（SignatureDoesNotMatch）
    - 添加hmacSHA256Binary函数，使用二进制HMAC结果进行密钥派生
    - 添加sha256HashBinary函数，直接处理二进制图片数据
    - 修复签名密钥派生算法，符合AWS Signature Version 4规范
  - 修复无效桶名称问题（InvalidBucketName）
    - 尝试在URI中添加桶名称（路径风格）
  - 修复虚拟主机域名必需问题（VirtualHostDomainRequired）
    - 改用虚拟主机风格（Virtual Hosted Style）
    - 桶名称在Host头中，URI只包含对象名称
    - 修改连接目标为虚拟主机域名
    - 更新URL生成函数
  - 编译测试通过，功能验证成功
  - 创建完整实现文档
- Files created/modified:
  - include/secrets.h (更新，修复OBS_REGION配置)
  - src/obs.cpp (更新，修复签名算法和虚拟主机风格)
    - 添加hmacSHA256Binary函数
    - 添加sha256HashBinary函数
    - 修改签名密钥派生逻辑
    - 修改URI构造（虚拟主机风格）
    - 修改Host头构造
    - 修改连接目标
    - 修改URL生成
  - test/layer4_cloud/test_obs/main.cpp (更新，添加NTP时间同步)
  - docs/obs_error_analysis.md (创建，时间偏差错误分析)
  - docs/obs_fix_record.md (创建，第一次修复记录)
  - docs/obs_signature_error_analysis.md (创建，签名错误分析)
  - docs/obs_signature_fix_record.md (创建，签名修复记录)
  - docs/obs_bucket_name_fix_record.md (创建，桶名称修复记录)
  - docs/obs_virtual_host_fix_record.md (创建，虚拟主机修复记录)
  - docs/archive/fixes/obs_fixes/obs_implementation_complete_record.md (创建，完整实现记录)
  - docs/archive/obs_work_summary.md (创建，工作总结)
  - docs/archive/obs_document_index.md (创建，文档索引)
- Compilation results:
  - test_obs: SUCCESS (RAM: 14.1%, Flash: 13.5%)
- Test results:
  - WiFi连接: ✅ OK
  - NTP时间同步: ✅ OK
  - OBS初始化: ✅ OK
  - 图片上传1: ✅ OK
  - 图片上传2: ✅ OK
  - URL生成: ✅ OK
- Key achievements:
  - 解决5个关键问题，实现OBS图片上传功能
  - 发现华为云OBS与AWS S3的主要区别：强制使用虚拟主机风格
  - 实现符合AWS Signature Version 4规范的签名算法
  - 正确处理二进制图片数据
  - 所有测试通过，功能正常
- **详细文档**: [docs/archive/fixes/obs_fixes/obs_implementation_complete_record.md](docs/archive/fixes/obs_fixes/obs_implementation_complete_record.md)

### Phase 9.5: AI识别流程优化
- **Status:** complete
- **Started:** 2026-04-21 21:00
- **Completed:** 2026-04-21 21:30
- Actions taken:
  - 修改AI识别流程，使用Base64编码本地图片
  - 调整状态机流程顺序：CAPTURE → RECOGNIZE → UPLOAD → EXECUTE
  - 先识别后上传，识别失败则不上传，节省资源
  - 降低AI识别与OBS上传的耦合度
  - 编译测试通过
- Files created/modified:
  - include/ai.h (更新，修改函数签名)
  - src/ai.cpp (更新，添加Base64编码功能)
  - include/constants.h (更新，调整状态枚举顺序)
  - src/main.ino (更新，调整状态机流程)
  - docs/ai_recognition_optimization_record.md (创建，优化记录)
- Compilation results:
  - esp32-s3-devkitc-1: SUCCESS (RAM: 16.3%, Flash: 15.1%)
- Key achievements:
  - 减少网络请求，无需先上传图片到OBS再识别
  - 降低延迟，识别失败时无需等待上传完成
  - 节省流量，识别失败则不上传图片
  - 降低耦合，AI识别不再依赖OBS上传
- **详细文档**: [docs/ai_recognition_optimization_record.md](docs/ai_recognition_optimization_record.md)

---

## Session: 2026-04-25

### Phase 9: Bug修复 - 基础错误处理
- **Status:** complete
- Actions taken:
  - 代码审查：逐模块检查错误处理逻辑
  - 修复1：舵机初始化使用错误的错误码（ERROR_CAMERA_INIT → ERROR_SERVO_INIT）
    - constants.h: ErrorCode枚举添加ERROR_SERVO_INIT
    - constants.h: ERROR_MESSAGES数组添加对应消息
    - main.ino: 修正lastError赋值
  - 修复2：IoTDA命令回调JSON解析失败不发送响应
    - iotda.cpp: 重构mqttCallback，先提取request_id再解析JSON
    - JSON解析失败/未知命令时仍然发送命令响应，避免IoTDA超时
- Files modified:
  - include/constants.h (更新ErrorCode枚举和ERROR_MESSAGES数组)
  - src/main.ino (修正舵机错误码)
  - src/iotda.cpp (重构命令回调)
- **Phase 9 整体完成，进入 Phase 10**

### Phase 10: 文档和演示准备
- **Status:** in_progress
- **Started:** 2026-04-25
(开发过程中补充测试结果)

| Test | Input | Expected | Actual | Status |
|------|-------|----------|--------|--------|
|      |       |          |        |        |

## Error Log
(开发过程中补充错误日志)

| Timestamp | Error | Attempt | Resolution |
|-----------|-------|---------|------------|
| 2026-04-18 16:30 | GPIO引脚冲突 | 1 | 检查ESP32-S3-CAM引脚图，将LED和舵机改用GPIO 19-48范围 |
| 2026-04-21 20:30 | MQTT连接失败(2) | 1 | 将<PubSubClient>库降级为2.7.0 |
| 2026-04-21 20:50 | MQTT连接失败(-1) | 3 | 将<PubSubClient.h>中的MQTT_MAX_PACKET_SIZE修改为更大 / 在platformio.ini中全局定义（在相关程序中定义可能无效，无法确定最先引入库的位置 |
| 2026-04-22 01:00 | OBS上传失败：RequestTimeTooSkewed | 1 | 在测试程序中添加NTP时间同步步骤，确保请求时间在服务器时间的±15分钟范围内 |
| 2026-04-22 01:10 | OBS签名区域不匹配 | 1 | 修改OBS_REGION为"cn-east-3"，与OBS_ENDPOINT一致 |
| 2026-04-22 01:20 | OBS上传失败：SignatureDoesNotMatch | 3 | 修复签名密钥派生算法，使用二进制HMAC结果而非十六进制字符串；添加二进制版本的SHA256函数处理图片数据 |
| 2026-04-22 01:25 | OBS上传失败：InvalidBucketName | 1 | 在URI中添加桶名称（路径风格），后被虚拟主机风格替代 |
| 2026-04-22 01:30 | OBS上传失败：VirtualHostDomainRequired | 1 | 改用虚拟主机风格（Virtual Hosted Style），桶名称在Host头中，URI只包含对象名称 |

## 5-Question Reboot Check
| Question | Answer |
|----------|--------|
| Where am I? | Phase 9完成，Phase 10进行中 |
| Where am I going? | Phase 1-10，共10个开发阶段 |
| What's the goal? | 构建完整的ESP32-S3-CAM垃圾分类识别系统用于课程大作业演示 |
| What have I learned? | 见findings.md |
| What have I done? | 见上方Phase日志（Phase 0~9全部完成，Phase 10开始） |

---
*Update after completing each phase or encountering errors*
