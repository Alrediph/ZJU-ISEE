# 页面业务逻辑

**源文件：** `pages/index/index.js`

## 职责

作为"总控"层，串联 IoTDA 命令下发模块和 OBS 数据获取模块：

1. 管理页面状态（轮询中、结果、分页、预览）
2. 点击"拍照识别" → 调用 IoTDA 发送命令 → 轮询 OBS 获取结果
3. 管理历史记录分页
4. 处理图片预览
5. 响应下拉刷新

---

## 模块引用与辅助函数

```javascript
const iot = require('../../utils/huawei-iot.js');
const obs = require('../../utils/obs-service.js');
```

### 垃圾分类颜色映射

```javascript
const CATEGORY_CLASS_MAP = {
  '可回收垃圾': 'recyclable',
  '可回收': 'recyclable',
  'Recyclable': 'recyclable',
  '有害垃圾': 'harmful',
  '有害': 'harmful',
  'Harmful': 'harmful',
  '厨余垃圾': 'kitchen',
  '厨余': 'kitchen',
  'Kitchen': 'kitchen',
  '其他垃圾': 'other',
  '其他': 'other',
  'Other': 'other'
};
```

支持中英文输入，映射到四种 CSS 样式类：`recyclable` / `harmful` / `kitchen` / `other`，分别对应蓝/红/绿/灰。

### withDisplayFields() — 补充页面显示字段

每条记录在展示前需要补充两个字段：

```javascript
function withDisplayFields(record) {
  if (!record) return {};
  var displayRecord = Object.assign({}, record);
  displayRecord.resultClass = CATEGORY_CLASS_MAP[displayRecord.result] || '';
  displayRecord.imageUrl = obs.getImageUrl(displayRecord.imageName);
  return displayRecord;
}
```

- `resultClass`：垃圾类别对应的 CSS 类名，用于颜色标识
- `imageUrl`：通过 OBS 模块生成的带签名临时访问 URL

### getCaptureErrorInfo() — 命令发送错误信息生成

```javascript
function getCaptureErrorInfo(err) {
  var title = '命令发送失败';
  var reason = '命令发送失败，请检查网络';
  var action = '请确认手机网络和小程序请求域名配置正常。';
  // ...
  if (err && err.type === 'device_offline') {
    reason = '设备不在线，命令无法到达设备';
    action = '请确认 ESP32 已上电、Wi-Fi 和 MQTT 已连接，并且设备在 IoTDA 控制台显示在线。';
  } else if (err && (err.type === 'auth_config' || err.type === 'cloud_config' || ...)) {
    reason = '连接设置或云平台配置错误，无法连接或下发命令';
    action = '请检查 IAM 账号密码、项目 ID、设备 ID、IoTDA 地址、service_id/command_name 和小程序合法域名。';
  }
  return { title, statusMessage: reason, modalContent: ... };
}
```

---

## 页面数据状态

```javascript
Page({
  data: {
    // 控制状态
    isPolling: false,       // 是否正在轮询
    statusMessage: '等待操作',  // 状态提示文字
    statusType: '',         // 状态类型（'error' 时变红）
    pollCount: 0,           // 轮询次数计数

    // 历史记录（全部）
    allHistory: [],         // 全部记录（最新在前）
    lastRecordCount: 0,     // 上次记录数（用于判断是否有新记录）

    // 分页控制
    currentPage: 1,         // 当前页码
    pageSize: 5,            // 每页条数
    totalPages: 0,          // 总页数
    displayHistory: [],     // 当前页显示的记录（带 imageUrl 和 resultClass）

    // 最新记录
    latestRecord: {},       // 最新一条记录
    latestImageUrl: '',     // 最新图片签名 URL
    latestImageLoaded: false, // 图片加载状态

    // 图片预览
    showPreview: false,      // 是否显示预览弹窗
    previewImageUrl: '',     // 预览图片 URL
    previewRecord: {}        // 预览记录
  },
  _pollTimer: null,  // 轮询定时器（不在 data 中，避免触发渲染）
  // ...
```

**设计考量：**
- `allHistory` 与 `displayHistory` 分离：全部记录只有纯文本数据，当前页记录才生成签名 URL，避免一次性生成所有图片 URL 浪费性能
- `_pollTimer` 放在 data 外部：定时器引用不需要触发页面渲染

---

## 页面生命周期

```javascript
onLoad() {
  this.loadHistory();       // 页面加载后立即加载历史记录
},

onUnload() {
  this.stopPolling();       // 页面销毁时停止轮询
},

onHide() {
  this.stopPolling();       // 页面隐藏时停止轮询（如切换到后台）
},
```

---

## 核心业务逻辑

### loadHistory() — 初始加载

页面启动时调用，读取 OBS 的 records.csv 并初始化所有数据：

```javascript
loadHistory() {
  var that = this;
  obs.getRecordCsv()
    .then(function(csvText) {
      var records = obs.parseRecordCsv(csvText);
      var totalPages = Math.ceil(records.length / that.data.pageSize);

      that.setData({
        allHistory: records,
        lastRecordCount: records.length,
        totalPages: totalPages,
        currentPage: 1,
        latestRecord: records.length > 0 ? withDisplayFields(records[0]) : {},
        latestImageUrl: '',
        latestImageLoaded: records.length === 0,
        displayHistory: []
      });

      if (records.length > 0) {
        that.loadLatestImage(records[0]);  // 加载最新图片
        that.loadPage(1);                   // 加载第一页
      }
    })
    .catch(function(err) {
      that.setData({
        statusMessage: '获取历史记录失败，请检查网络',
        statusType: 'error'
      });
    });
}
```

### onCapture() — 点击拍照识别

```javascript
onCapture() {
  var that = this;
  if (this.data.isPolling) return;  // 防止重复点击

  that.setData({
    isPolling: true,
    statusMessage: '正在发送指令...',
    statusType: '',
    pollCount: 0
  });

  iot.sendCapture()
    .then(function() {
      that.setData({
        statusMessage: '指令已发送，正在识别中...',
        statusType: ''
      });
      that.startPolling();  // 命令发送成功，开始轮询
    })
    .catch(function(err) {
      var errorInfo = getCaptureErrorInfo(err);
      that.setData({
        isPolling: false,
        statusMessage: errorInfo.statusMessage,
        statusType: 'error'
      });
      wx.showModal({
        title: errorInfo.title,
        content: errorInfo.modalContent,
        showCancel: false
      });
    });
}
```

### startPolling() — 开始轮询

每 3 秒读取一次 CSV，判断是否有新记录：

```javascript
startPolling() {
  var that = this;

  this._pollTimer = setInterval(function() {
    var newPollCount = that.data.pollCount + 1;

    obs.getRecordCsv()
      .then(function(csvText) {
        var records = obs.parseRecordCsv(csvText);
        var newCount = records.length;

        that.setData({ pollCount: newPollCount });

        if (newCount > that.data.lastRecordCount) {
          // ★ 发现新记录 → 停止轮询 → 更新 UI
          that.stopPolling();

          var newRecord = records[0];
          var latestRecord = withDisplayFields(newRecord);

          var totalPages = Math.ceil(records.length / that.data.pageSize);
          var currentPage = Math.min(that.data.currentPage, totalPages || 1);

          that.setData({
            allHistory: records,
            lastRecordCount: newCount,
            totalPages: totalPages,
            currentPage: currentPage,
            latestRecord: latestRecord,
            statusMessage: '识别完成: ' + latestRecord.result,
            statusType: '',
            isPolling: false
          });

          that.loadLatestImage(newRecord);
          that.loadPage(currentPage);

          wx.showToast({ title: '识别完成', icon: 'success' });

        } else {
          // 没有新记录，更新轮询计时
          that.setData({
            statusMessage: '正在识别中... (' + newPollCount + 's)',
            statusType: ''
          });
        }

        // 超时检查（10 次 = 30 秒）
        if (newPollCount >= 10) {
          that.stopPolling();
          that.setData({
            isPolling: false,
            statusMessage: '识别超时，请手动刷新查看',
            statusType: 'error',
            // ... 更新数据
          });
        }
      })
      .catch(function(err) {
        // 网络异常时静默失败，继续轮询
        that.setData({
          statusMessage: '正在识别中... (网络异常，继续尝试)',
          statusType: 'error'
        });
      });
  }, 3000);
}
```

**轮询流程决策树：**

```
onCapture() 成功
  → startPolling()
    → 每 3 秒:
      → getRecordCsv() → 解析
        ├─ 记录数增加 → 停止轮询 → 更新 UI
        └─ 记录数不变
            ├─ 已达 10 次 → 停止轮询 → 超时提示
            └─ 未到 10 次 → 继续
```

### stopPolling() — 停止轮询

```javascript
stopPolling() {
  if (this._pollTimer) {
    clearInterval(this._pollTimer);
    this._pollTimer = null;
  }
  this.setData({ isPolling: false });
}
```

### loadLatestImage() — 加载最新图片

```javascript
loadLatestImage(record) {
  if (!record || !record.imageName) return;

  var that = this;
  var signedUrl = obs.getImageUrl(record.imageName);
  var latestRecord = this.data.latestRecord;
  if (latestRecord && latestRecord.imageName === record.imageName) {
    latestRecord = Object.assign({}, latestRecord, { imageUrl: signedUrl });
  }
  that.setData({
    latestImageUrl: signedUrl,
    latestImageLoaded: false,
    latestRecord: latestRecord
  });
}
```

---

## 分页逻辑

### loadPage() — 加载指定页

```javascript
loadPage(page) {
  var pageSize = this.data.pageSize;
  var allHistory = this.data.allHistory;

  var start = (page - 1) * pageSize;
  var end = Math.min(start + pageSize, allHistory.length);

  // 只提取当前页记录，生成签名 URL 和 resultClass
  var pageRecords = allHistory.slice(start, end).map(function(record) {
    return withDisplayFields(record);
  });

  this.setData({
    currentPage: page,
    displayHistory: pageRecords
  });
}
```

### 翻页控制

```javascript
prevPage() {
  if (this.data.currentPage <= 1) return;
  this.loadPage(this.data.currentPage - 1);
},

nextPage() {
  if (this.data.currentPage >= this.data.totalPages) return;
  this.loadPage(this.data.currentPage + 1);
},
```

---

## 图片预览

### 历史记录点击

```javascript
onRecordTap(e) {
  var index = e.currentTarget.dataset.index;
  var record = this.data.displayHistory[index];
  if (!record) return;

  this.setData({
    showPreview: true,
    previewImageUrl: record.imageUrl,     // 使用已生成的签名 URL
    previewRecord: record
  });
}
```

### 最新记录点击

```javascript
onLatestRecordTap() {
  var record = this.data.latestRecord;
  if (!record || !record.imageName) return;

  this.setData({
    showPreview: true,
    previewImageUrl: record.imageUrl || obs.getImageUrl(record.imageName),
    previewRecord: record
  });
}
```

**为什么最新记录和历史记录分开处理？** 避免当前页面不是第一页时，点击最新结果却错误打开当前分页第 0 条记录。

### 关闭预览

```javascript
closePreview() {
  this.setData({
    showPreview: false,
    previewImageUrl: '',
    previewRecord: {}
  });
},
```

### 图片加载回调

```javascript
onImageLoad() {
  this.setData({ latestImageLoaded: true });
},

onImageError() {
  console.log('图片加载失败');
  this.setData({ latestImageLoaded: true });
},
```

---

## 下拉刷新

```javascript
onPullDownRefresh() {
  var that = this;

  obs.getRecordCsv()
    .then(function(csvText) {
      var records = obs.parseRecordCsv(csvText);
      var totalPages = Math.ceil(records.length / that.data.pageSize);
      var currentPage = Math.min(that.data.currentPage, totalPages || 1);

      that.setData({
        allHistory: records,
        lastRecordCount: records.length,
        totalPages: totalPages,
        currentPage: currentPage,
        latestRecord: records.length > 0 ? withDisplayFields(records[0]) : {},
        latestImageUrl: records.length > 0 ? that.data.latestImageUrl : '',
        latestImageLoaded: records.length === 0,
        displayHistory: records.length > 0 ? that.data.displayHistory : []
      });

      if (records.length > 0) {
        that.loadLatestImage(records[0]);
        that.loadPage(currentPage);
      }

      wx.stopPullDownRefresh();
      wx.showToast({ title: '刷新成功', icon: 'success' });
    })
    .catch(function(err) {
      wx.stopPullDownRefresh();
      wx.showToast({ title: '刷新失败', icon: 'none' });
    });
}
```

需要在 `index.json` 中启用 `"enablePullDownRefresh": true`。

---

## 完整方法清单

| 方法 | 触发时机 | 功能 |
|------|---------|------|
| `onLoad()` | 页面加载 | 加载历史记录 |
| `onUnload()` | 页面销毁 | 停止轮询 |
| `onHide()` | 页面隐藏 | 停止轮询 |
| `onPullDownRefresh()` | 下拉 | 重新读取 CSV 刷新数据 |
| `loadHistory()` | 初始加载 | 读取 CSV → 解析 → 设置数据 |
| `onCapture()` | 点击拍照按钮 | 发送命令 → 开始轮询 |
| `startPolling()` | 命令发送成功 | 每 3 秒轮询 CSV |
| `stopPolling()` | 发现记录/超时/页面隐藏 | 停止定时器 |
| `loadLatestImage()` | 新记录出现 | 生成最新图片签名 URL |
| `loadPage(page)` | 换页 | 提取当前页记录，生成签名 URL |
| `prevPage()` / `nextPage()` | 点击翻页按钮 | 切换分页 |
| `onRecordTap()` | 点击历史记录 | 打开图片预览 |
| `onLatestRecordTap()` | 点击最新结果 | 打开图片预览 |
| `closePreview()` | 点击关闭 | 关闭图片预览 |
| `preventClose()` | 点击预览内容 | 阻止关闭事件冒泡 |

---

## 数据流全景

```
                    ┌─────────────┐
                    │  用户操作    │
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
        onCapture()  onPullDown()  onRecordTap()
              │            │            │
              ▼            ▼            ▼
        iot.sendCapture  obs.getRecordCsv  setData(showPreview)
              │            │
              ▼            ▼
        startPolling()  parseRecordCsv()
              │            │
              └────▶  obs.getRecordCsv (循环)
                          │
                    发现新记录?
                     ├─ 是: stopPolling + 更新 UI
                     └─ 否: 继续（最多 10 次）
                          │
                    超时 → 停止 + 错误提示
```
