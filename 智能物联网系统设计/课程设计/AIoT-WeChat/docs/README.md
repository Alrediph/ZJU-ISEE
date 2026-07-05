# 智能垃圾分类微信小程序 — 文档索引

## 项目简介

微信小程序端，配合华为云 IoTDA 和 ESP32 硬件，实现"拍照识别垃圾类别并展示历史记录"的完整演示流程。

**核心职责：**
1. 向华为云 IoTDA 下发 `capture` 命令，触发 ESP32 拍照识别
2. 从华为云 OBS 轮询 `Images/records.csv`，获取识别结果
3. 在页面上展示最新结果、历史记录、分页和图片预览

---

## 文件结构

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
│   ├── README.md                # ◀ 本文档：项目总览
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

---

## 模块关系图

```
┌─────────────────────────────────────────────────────┐
│                    pages/index/index.js              │
│                   (页面业务逻辑 - 总控)                │
│                                                      │
│  onCapture() → iot.sendCapture()                     │
│       ↓ 成功后                                        │
│  startPolling() → obs.getRecordCsv() (每3秒)          │
│       ↓ 发现新记录                                     │
│  loadPage() → obs.getImageUrl() (生成签名图片URL)      │
│       ↓                                               │
│  setData() → WXML 渲染                                │
└──────────┬────────────────────────┬──────────────────┘
           │                        │
           ▼                        ▼
┌─────────────────────┐  ┌──────────────────────────┐
│  utils/huawei-iot.js │  │  utils/obs-service.js     │
│                     │  │                           │
│  getToken()         │  │  HmacSha1.sign()          │
│    → IAM API        │  │    → 纯 JS HMAC-SHA1      │
│  sendCapture()      │  │  getRecordCsv()           │
│    → IoTDA API      │  │    → OBS GET 签名请求     │
│  getShadow()        │  │  parseRecordCsv()         │
│                     │  │  getImageUrl()            │
│                     │  │    → 查询参数签名          │
└─────────────────────┘  └──────────────────────────┘
```

---

## 数据流

### 主流程（点击"拍照识别"）

```
用户点击按钮
  │
  ▼
index.js: onCapture()
  │ 设置 isPolling=true，禁用按钮
  ▼
huawei-iot.js: sendCapture()
  │ 1. getToken() → IAM API → 获取 X-Subject-Token（缓存到 wx storage）
  │ 2. sendCaptureWithToken(token) → IoTDA API → POST /v5/iot/{projectId}/devices/{deviceId}/commands
  │   请求体: {"service_id": "GarbageClassification", "command_name": "capture", "paras": {}}
  ▼
成功 → index.js: startPolling()
  │ 每 3 秒轮询一次
  ▼
obs-service.js: getRecordCsv()
  │ 1. buildHeaders('GET', '/Images/records.csv')
  │ 2. 计算 V2 签名（HMAC-SHA1）
  │ 3. wx.request({url: + "?t=" + timestamp})
  │ 4. 返回 CSV 文本
  ▼
index.js: parseRecordCsv() 解析 CSV
  │ 判断 records.length > lastRecordCount?
  │   ├─ 是：停止轮询，更新 UI
  │   └─ 否：继续轮询（最多 10 次 = 30 秒超时）
  ▼
obs-service.js: getImageUrl(imageName)
  │ 查询参数签名方式生成临时 URL（15 分钟过期）
  │ 返回: https://bucket.obs.cn-east-3.myhuaweicloud.com/Images/xxx.jpg?AccessKeyId=xxx&Expires=xxx&Signature=xxx
  ▼
WXML 渲染: <image src="{{imageUrl}}" />
```

### 下拉刷新

```
用户下拉页面
  │
  ▼
onPullDownRefresh()
  │ getRecordCsv() → parseRecordCsv() → 刷新 allHistory
  │ loadLatestImage(records[0]) → loadPage(currentPage)
  │ wx.stopPullDownRefresh() → toast "刷新成功"
```

### 分页切换

```
用户点击"上一页"/"下一页"
  │
  ▼
prevPage() / nextPage()
  │
  ▼
loadPage(page)
  │ 1. slice(allHistory, start, start+pageSize)  →  提取当前页记录
  │ 2. 每条记录调用 withDisplayFields(record)
  │    → obs.getImageUrl(imageName) 生成签名 URL
  │    → CATEGORY_CLASS_MAP 映射 resultClass
  │ 3. setData({ displayHistory: pageRecords })
```

---

## 核心配置一览

| 配置项 | 值 | 所在文件 |
|--------|-----|---------|
| 华为云账号（domain） | `starrysky2006` | `utils/huawei-iot.js` |
| IAM 用户名 | `developer1` | `utils/huawei-iot.js` |
| 项目 ID | `961ef7ebef4d491db148a8dd0208a894` | `utils/huawei-iot.js` |
| 设备 ID | `69e6502fe094d6159234626c_ESP32-S3-CAM` | `utils/huawei-iot.js` |
| 服务 ID | `GarbageClassification` | `utils/huawei-iot.js` |
| 命令名 | `capture` | `utils/huawei-iot.js` |
| IAM Endpoint | `iam.cn-east-3.myhuaweicloud.com` | `utils/huawei-iot.js` |
| IoTDA Endpoint | `015487493c.st1.iotda-app.cn-east-3.myhuaweicloud.com` | `utils/huawei-iot.js` |
| OBS Endpoint | `obs.cn-east-3.myhuaweicloud.com` | `utils/obs-service.js` |
| OBS 桶名 | `iotda-obs-data` | `utils/obs-service.js` |
| CSV 路径 | `Images/records.csv` | `utils/obs-service.js` |
| OBS AK | `HPUAW2GYOGAJ2OGCYLLP` | `utils/obs-service.js` |
| 轮询间隔 | 3 秒 | `pages/index/index.js` |
| 轮询超时 | 30 秒（10 次） | `pages/index/index.js` |
| 分页大小 | 5 条/页 | `pages/index/index.js` |
| 图片签名过期 | 15 分钟 | `utils/obs-service.js` |
| 导航栏标题 | 智能垃圾分类 | `app.json` |
| 导航栏颜色 | `#4CAF50`（绿色） | `app.json` |

---

## 子文档索引

| 文档 | 内容 | 对应源文件 |
|------|------|-----------|
| [modules/iotda.md](modules/iotda.md) | IAM 认证 + IoTDA 命令下发 | `utils/huawei-iot.js` |
| [modules/obs-service.md](modules/obs-service.md) | OBS V2 签名 + CSV/图片获取 | `utils/obs-service.js` |
| [modules/page-logic.md](modules/page-logic.md) | 页面业务逻辑 | `pages/index/index.js` |
| [modules/ui.md](modules/ui.md) | UI 结构与样式 | `pages/index/index.wxml` + `wxss` + `app.*` |