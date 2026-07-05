# UI 结构与样式

## 文件清单

| 文件 | 职责 | 路径 |
|------|------|------|
| `app.json` | 全局配置：页面注册、导航栏、窗口样式 | 项目根目录 |
| `app.wxss` | 全局样式：背景色、字体 | 项目根目录 |
| `index.wxml` | 页面模板：结构布局 | `pages/index/` |
| `index.wxss` | 页面样式：组件样式、分类颜色 | `pages/index/` |
| `index.json` | 页面配置：下拉刷新开关 | `pages/index/` |

---

## 全局配置 (`app.json`)

```json
{
  "pages": [
    "pages/index/index"
  ],
  "window": {
    "navigationBarTextStyle": "white",
    "navigationBarTitleText": "智能垃圾分类",
    "navigationBarBackgroundColor": "#4CAF50"
  },
  "style": "v2",
  "componentFramework": "glass-easel",
  "sitemapLocation": "sitemap.json",
  "lazyCodeLoading": "requiredComponents"
}
```

- **单页面注册：** 只有 `pages/index/index` 一个页面
- **绿色导航栏：** 标题"智能垃圾分类"，背景色 `#4CAF50`
- **白色文字：** 导航栏文字为白色（深色背景）

## 页面配置 (`index.json`)

```json
{
  "usingComponents": {},
  "enablePullDownRefresh": true,
  "backgroundTextStyle": "dark"
}
```

- `enablePullDownRefresh: true` 开启下拉刷新，触发 `onPullDownRefresh()` 方法
- `usingComponents` 为空，未使用任何自定义组件

## 全局样式 (`app.wxss`)

```css
page {
  background-color: #f5f5f5;
  font-family: -apple-system, BlinkMacSystemFont, 'Helvetica Neue', Helvetica,
    'PingFang SC', 'Microsoft YaHei', Arial, sans-serif;
  color: #333;
}
```

- 浅灰背景（`#f5f5f5`），配系统默认字体栈

---

## 页面结构 (`index.wxml`)

页面由四部分组成：

```
page-container
├── control-section          # ① 拍照控制区
│   ├── capture-btn           # 拍照按钮（禁用态灰色）
│   └── status-text           # 状态提示文字
│
├── latest-section           # ② 最近识别结果
│   ├── section-header        # 标题
│   ├── empty-state           # 空状态（无记录时显示）
│   └── latest-card           # 结果卡片（有记录时显示）
│       ├── card-image-wrapper  # 图片区（400rpx）
│       │   ├── card-image       # 图片（带加载中提示）
│       │   └── image-loading    # 加载中遮罩
│       └── card-info          # 信息区
│           ├── card-category    # 分类名 + 颜色点
│           └── card-time        # 时间
│
├── history-section          # ③ 历史记录列表
│   ├── section-header        # 标题 + 记录数
│   ├── history-list          # 记录列表
│   │   └── history-item       # 单条记录（缩略图 + 分类 + 时间）
│   └── pagination            # 分页控制
│       ├── prev-btn           # 上一页
│       ├── page-info          # 当前页/总页数
│       └── next-btn           # 下一页
│
└── preview-overlay          # ④ 图片预览模态框
    └── preview-container
        ├── preview-image      # 大图
        ├── preview-info       # 分类 + 时间
        └── preview-close      # 关闭按钮
```

### 关键 WXML 逻辑

**拍照按钮（禁用态控制）：**

```xml
<button class="capture-btn {{isPolling ? 'polling' : ''}}"
        bindtap="onCapture"
        disabled="{{isPolling}}">
  <view class="capture-content">
    <text class="btn-icon" wx:if="{{!isPolling}}">📷</text>
    <text class="btn-text">{{isPolling ? '正在识别中...' : '拍照识别'}}</text>
  </view>
</button>
```

- `isPolling` 为 `true` 时：按钮变灰（`.polling`）、禁用、文字变为"正在识别中..."
- 拍照图标只在非轮询状态显示

**最新结果卡片：**

```xml
<view class="latest-card" wx:else bindtap="onLatestRecordTap">
  <view class="card-image-wrapper">
    <image class="card-image" src="{{latestImageUrl || ''}}"
           mode="aspectFill" binderror="onImageError" bindload="onImageLoad" />
    <view class="image-loading" wx:if="{{!latestImageLoaded}}">
      <text>加载中...</text>
    </view>
  </view>
  <view class="card-info">
    <view class="card-category {{latestRecord.resultClass || ''}}">
      <text class="category-dot"></text>
      <text class="category-text">{{latestRecord.result || '未知'}}</text>
    </view>
    <text class="card-time">{{latestRecord.time || '未知时间'}}</text>
  </view>
</view>
```

- `resultClass` 控制颜色点 + 分类文字的颜色
- `latestImageLoaded` 控制加载中遮罩显示

**历史记录列表 + 分页：**

```xml
<view class="history-list">
  <view class="history-item" wx:for="{{displayHistory}}" wx:key="time"
        wx:for-index="idx" bindtap="onRecordTap" data-index="{{idx}}">
    <image class="history-thumb" src="{{item.imageUrl || ''}}" mode="aspectFill" />
    <view class="history-info">
      <view class="history-category {{item.resultClass || ''}}">
        <text class="category-dot"></text>
        <text class="category-text">{{item.result}}</text>
      </view>
      <text class="history-time">{{item.time}}</text>
    </view>
  </view>
</view>

<view class="pagination" wx:if="{{totalPages > 1}}">
  <button class="page-btn" bindtap="prevPage" disabled="{{currentPage <= 1}}">上一页</button>
  <text class="page-info">第{{currentPage}}/{{totalPages}}页</text>
  <button class="page-btn" bindtap="nextPage" disabled="{{currentPage >= totalPages}}">下一页</button>
</view>
```

- 分页只在一页以上时显示
- 首页时"上一页"禁用，末页时"下一页"禁用

**图片预览模态框：**

```xml
<view class="preview-overlay" wx:if="{{showPreview}}" bindtap="closePreview">
  <view class="preview-container" catchtap="preventClose">
    <image class="preview-image" src="{{previewImageUrl}}" mode="aspectFit" />
    <view class="preview-info">
      <text class="preview-category {{previewRecord.resultClass || ''}}">
        <text class="category-dot"></text>
        <text class="category-text">{{previewRecord.result}}</text>
      </text>
      <text class="preview-time">{{previewRecord.time}}</text>
    </view>
    <view class="preview-close" bindtap="closePreview">
      <text class="close-icon">✕</text>
    </view>
  </view>
</view>
```

- 全屏黑色半透明遮罩（`rgba(0,0,0,0.9)`）
- 点击遮罩区域关闭，点击预览内容不关闭（`catchtap`）
- 右上角有 ✕ 关闭按钮

---

## 页面样式 (`index.wxss`)

### 布局与卡片

```css
.page-container {
  min-height: 100vh;
  background-color: #f5f5f5;
  padding: 20rpx;
  box-sizing: border-box;
  padding-bottom: 40rpx;
}

.control-section, .latest-section, .history-section {
  background: white;
  border-radius: 16rpx;
  padding: 30rpx;
  margin-bottom: 30rpx;
  box-shadow: 0 2rpx 10rpx rgba(0, 0, 0, 0.08);
}
```

三个主要区块使用统一的白色圆角卡片样式。

### 拍照按钮

```css
.capture-btn {
  width: 80%;
  height: 110rpx;
  border-radius: 55rpx;
  background: linear-gradient(135deg, #4CAF50, #388E3C);
  color: white;
  font-size: 36rpx;
  font-weight: bold;
  box-shadow: 0 4rpx 15rpx rgba(76, 175, 80, 0.3);
}

.capture-btn:active {
  transform: scale(0.95);
}

.capture-btn.polling {
  background: linear-gradient(135deg, #9E9E9E, #757575);
}
```

- 绿色渐变圆形按钮，带阴影
- 点击缩放动画
- 轮询状态下变为灰色

### 最新结果卡片

```css
.card-image-wrapper {
  width: 100%;
  height: 400rpx;     /* 足够大的图片区域 */
  position: relative;
  background: #eee;
  flex-shrink: 0;      /* 不被内容压缩 */
}

.card-image {
  width: 100%;
  height: 100%;
}

.card-info {
  padding: 20rpx;
  display: flex;
  justify-content: space-between;
  align-items: center;
}
```

### 分类颜色标识

```css
.category-dot {
  display: inline-block;
  width: 16rpx;
  height: 16rpx;
  border-radius: 50%;
  margin-right: 10rpx;
  vertical-align: middle;
}

/* 四色分类 */
.recyclable .category-dot { background-color: #2196F3; }  /* 蓝 */
.recyclable .category-text { color: #2196F3; }

.harmful .category-dot { background-color: #F44336; }     /* 红 */
.harmful .category-text { color: #F44336; }

.kitchen .category-dot { background-color: #4CAF50; }     /* 绿 */
.kitchen .category-text { color: #4CAF50; }

.other .category-dot { background-color: #9E9E9E; }       /* 灰 */
.other .category-text { color: #9E9E9E; }
```

四种垃圾分类颜色：
| 分类 | CSS 类名 | 颜色 | 色值 |
|------|---------|------|------|
| 可回收 | `recyclable` | 蓝色 | `#2196F3` |
| 有害 | `harmful` | 红色 | `#F44336` |
| 厨余 | `kitchen` | 绿色 | `#4CAF50` |
| 其他 | `other` | 灰色 | `#9E9E9E` |

### 图片预览模态框

```css
.preview-overlay {
  position: fixed;
  top: 0; left: 0; right: 0; bottom: 0;
  background: rgba(0, 0, 0, 0.9);
  z-index: 1000;
  display: flex;
  align-items: center;
  justify-content: center;
}

.preview-container {
  width: 90%;
  height: 80%;
  position: relative;
  display: flex;
  flex-direction: column;
  align-items: center;
}

.preview-image {
  width: 100%;
  flex: 1;            /* 自动填满剩余空间 */
  border-radius: 8rpx;
}

.preview-close {
  position: absolute;
  top: 20rpx;
  right: 20rpx;
  z-index: 1001;
}
```

### 分页控制

```css
.pagination {
  display: flex;
  justify-content: center;
  align-items: center;
  margin-top: 30rpx;
  padding-top: 20rpx;
  border-top: 1rpx solid #f0f0f0;
}

.page-btn {
  width: 128rpx;
  height: 64rpx;
  line-height: 64rpx;
  font-size: 24rpx;
  background: #4CAF50;
  color: white;
  border-radius: 30rpx;
  flex: 0 0 128rpx;   /* 固定宽度，不被压缩 */
}

.page-btn[disabled] {
  background: #e0e0e0;
  color: #999;
}

.page-info {
  flex: 1 1 auto;
  min-width: 0;
  margin: 0 12rpx;
  text-align: center;
  font-size: 24rpx;
  color: #666;
  white-space: nowrap;
}
```

- 按钮固定 128rpx 宽度，避免文字变化导致布局抖动
- 页码文字 `white-space: nowrap` 防止换行

---

## UI 状态对照表

| 状态 | isPolling | statusType | 按钮文字 | 状态文字 | 按钮样式 |
|------|-----------|------------|---------|---------|---------|
| 初始 | `false` | `''` | 拍照识别 | 等待操作 | 绿色 |
| 发送中 | `true` | `''` | 正在识别中... | 正在发送指令... | 灰色 |
| 轮询中 | `true` | `''` | 正在识别中... | 正在识别中... (Ns) | 灰色 |
| 完成 | `false` | `''` | 拍照识别 | 识别完成: xxx | 绿色 |
| 超时 | `false` | `'error'` | 拍照识别 | 识别超时，请手动刷新查看 | 绿色 |
| 失败 | `false` | `'error'` | 拍照识别 | (错误详情) | 绿色 |
| 网络异常 | `true` | `'error'` | 正在识别中... | 正在识别中... (网络异常) | 灰色 |

---

## 交互反馈

| 操作 | 反馈 |
|------|------|
| 点击拍照 | 状态文字更新 + 按钮禁用变灰 |
| 命令发送成功 | toast 无（状态文字更新） |
| 识别完成 | toast "识别完成" + 新结果展示 |
| 识别超时 | 状态文字变红 + 手动刷新提示 |
| 命令发送失败 | 弹窗显示错误详情 |
| 下拉刷新 | toast "刷新成功/失败" |
| 图片加载中 | 遮罩"加载中..." |
| 图片加载失败 | 控制台 log，隐藏加载遮罩 |
