# 智能垃圾分类微信小程序

与华为云 IoTDA 和 OBS 联合控制 ESP32 设备，实现模拟垃圾分类功能。

详细的软件端实现逻辑、核心代码说明见：[`docs/README.md`](docs/README.md)。

## 项目简介

ESP32 设备收到小程序发送的 `capture` 指令后，自动完成：拍照物品 → 识别垃圾类别 → 驱动对应分类动作 → 上传图片到 OBS → 追加识别记录。小程序负责发送指令、轮询结果并展示历史识别记录。

## 功能

- **拍照识别**：一键发送 capture 指令到华为云 IoTDA，转发给 ESP32 执行识别
- **结果展示**：显示最新识别结果（图片 + 类别 + 时间）
- **历史记录**：从 OBS 获取 records.csv 解析所有历史识别记录，支持点击查看图片
- **四类垃圾标识**：可回收（蓝）、有害（红）、厨余（绿）、其他（灰）

## 技术架构

```
微信小程序 (单页面)
├── utils/huawei-iot.js  — IAM 认证 + IoTDA 命令下发
├── utils/obs-service.js — OBS AK/SK V2 签名 + 数据获取
└── pages/index/         — 单页面 UI + 业务逻辑

华为云服务
├── IoTDA  — 命令下发 API（转发 capture 到 ESP32）
└── OBS    — 对象存储（图片 + record.csv 历史记录）

ESP32 设备
收到 capture → 拍照 → 识别 → 驱动动作 → 上传OBS → 更新记录
```

## 开发

使用微信开发者工具打开项目进行开发和调试。

### 目录结构

```
AIoT-WeChat/
├── app.js                       # 全局入口（简化版，仅 wx.login）
├── app.json                     # 页面注册 + 窗口配置（单页面）
├── app.wxss                     # 全局样式（背景色 + 字体）
├── project.config.json          # 微信开发者工具配置
├── sitemap.json                 # 搜索索引配置
│
├── pages/
│   └── index/
│       ├── index.js             # 核心业务逻辑（capture → 轮询 → 展示）
│       ├── index.wxml           # 页面模板（控制区 + 结果 + 历史 + 预览）
│       ├── index.wxss           # 页面样式（绿色主题 + 四色分类标识）
│       └── index.json           # 页面配置（开启下拉刷新）
│
├── utils/
│   ├── huawei-iot.js            # 华为云 IAM 认证 + IoTDA 命令下发
│   ├── obs-service.js           # OBS V2 签名 + CSV/图片获取
│   └── util.js                  # 基础工具（时间格式化）
│
├── docs/
│   ├── README.md                # 项目总览
│   └── modules/
│       ├── iotda.md             # IoTDA 模块详解
│       ├── obs-service.md       # OBS 模块详解
│       ├── page-logic.md        # 页面业务逻辑详解
│       └── ui.md                # UI 结构详解
│
├── scripts/                     # 调试/验证脚本
│   ├── upload_test_data.py
│   ├── verify_obs_images.py
│   ├── check_csv_content.py
│   ├── verify_sha1_rounds.py
│   ├── verify_obs_access.py
│   └── test_sha1_debug.js
│
└── README.md                    # 项目根 README
```

## 关键设计决策

- **模块化拆分**：IoTDA 和 OBS 功能独立为 utils 模块，职责清晰
- **轮询完成检测**：发送 capture 后每 3 秒轮询 records.csv，发现新记录即停止
- **配置硬编码**：连接参数直接在代码中配置，适合演示项目

## 参考

- 华为云 IoTDA API 文档：[华为云 IoTDA](https://support.huaweicloud.com/iothub/index.html)
- 华为云 OBS API 文档：[OBS成长地图](https://support.huaweicloud.com/obs/index.html) [OBS 获取对象](https://support.huaweicloud.com/api-obs/obs_04_0052.html)
