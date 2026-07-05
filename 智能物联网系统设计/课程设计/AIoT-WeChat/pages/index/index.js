// pages/index/index.js
const iot = require('../../utils/huawei-iot.js');
const obs = require('../../utils/obs-service.js');

// 垃圾分类类别对应的颜色标识 class
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

function withDisplayFields(record) {
  if (!record) return {};
  var displayRecord = Object.assign({}, record);
  displayRecord.resultClass = CATEGORY_CLASS_MAP[displayRecord.result] || '';
  displayRecord.imageUrl = obs.getImageUrl(displayRecord.imageName);
  return displayRecord;
}

function getCaptureErrorInfo(err) {
  var title = '命令发送失败';
  var reason = '命令发送失败，请检查网络';
  var action = '请确认手机网络和小程序请求域名配置正常。';

  if (err && err.type === 'device_offline') {
    reason = '设备不在线，命令无法到达设备';
    action = '请确认 ESP32 已上电、Wi-Fi 和 MQTT 已连接，并且设备在 IoTDA 控制台显示在线。';
  } else if (err && (
    err.type === 'auth_config' ||
    err.type === 'cloud_config' ||
    err.type === 'network_config'
  )) {
    reason = '连接设置或云平台配置错误，无法连接或下发命令';
    action = '请检查 IAM 账号密码、项目 ID、设备 ID、IoTDA 地址、service_id/command_name 和小程序合法域名。';
  }

  var detailParts = [];
  if (err && err.statusCode) detailParts.push('状态码：' + err.statusCode);
  if (err && err.errorCode) detailParts.push('错误码：' + err.errorCode);
  if (err && err.detail) detailParts.push('详情：' + err.detail);

  return {
    title: title,
    statusMessage: reason,
    modalContent: reason + '\n\n' + action + (detailParts.length ? '\n\n' + detailParts.join('\n') : '')
  };
}

Page({
  data: {
    // 控制状态
    isPolling: false,
    statusMessage: '等待操作',
    statusType: '',
    pollCount: 0,

    // 历史记录（全部）
    allHistory: [],
    lastRecordCount: 0,

    // 分页控制
    currentPage: 1,
    pageSize: 5,
    totalPages: 0,
    displayHistory: [],  // 当前页显示的记录（带 imageUrl 和 resultClass）

    // 最新记录
    latestRecord: {},
    latestImageUrl: '',
    latestImageLoaded: false,

    // 图片预览
    showPreview: false,
    previewImageUrl: '',
    previewRecord: {}
  },

  // 轮询定时器
  _pollTimer: null,

  // ========== 页面生命周期 ==========

  onLoad() {
    this.loadHistory();
  },

  onUnload() {
    this.stopPolling();
  },

  onHide() {
    this.stopPolling();
  },

  /**
   * 下拉刷新
   */
  onPullDownRefresh() {
    console.log('[下拉刷新] 开始刷新');
    var that = this;

    obs.getRecordCsv()
      .then(function(csvText) {
        console.log('[下拉刷新] CSV 内容长度:', csvText.length);

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
        that.setData({ statusType: '' });
        wx.showToast({
          title: '刷新成功',
          icon: 'success',
          duration: 1500
        });
      })
      .catch(function(err) {
        console.error('[下拉刷新失败]:', err);
        wx.stopPullDownRefresh();
        wx.showToast({
          title: '刷新失败',
          icon: 'none',
          duration: 1500
        });
      });
  },

  // ========== 核心业务逻辑 ==========

  /**
   * 初始加载历史记录
   */
  loadHistory() {
    var that = this;
    console.log('[初始加载] 开始获取历史记录');

    obs.getRecordCsv()
      .then(function(csvText) {
        console.log('[初始加载] CSV 内容长度:', csvText.length);
        console.log('[初始加载] CSV 前 200 字符:', csvText.substring(0, 200));

        var records = obs.parseRecordCsv(csvText);
        console.log('[初始加载] 解析后记录数:', records.length);
        if (records.length > 0) {
          console.log('[初始加载] 最新记录:', JSON.stringify(records[0], null, 2));
        }

        var totalPages = Math.ceil(records.length / that.data.pageSize);

        that.setData({
          allHistory: records,
          lastRecordCount: records.length,
          totalPages: totalPages,
          currentPage: records.length > 0 ? 1 : 1,
          latestRecord: records.length > 0 ? withDisplayFields(records[0]) : {},
          latestImageUrl: '',
          latestImageLoaded: records.length === 0,
          displayHistory: []
        });

        if (records.length > 0) {
          that.loadLatestImage(records[0]);
          that.loadPage(1);  // 加载第一页
        }
      })
      .catch(function(err) {
        console.error('[初始加载失败]:', err);
        that.setData({
          statusMessage: '获取历史记录失败，请检查网络',
          statusType: 'error'
        });
      });
  },

  /**
   * 加载指定页的记录（生成带签名的 imageUrl）
   */
  loadPage(page) {
    var that = this;
    var pageSize = this.data.pageSize;
    var allHistory = this.data.allHistory;

    var start = (page - 1) * pageSize;
    var end = Math.min(start + pageSize, allHistory.length);

    // 提取当前页的记录
    var pageRecords = allHistory.slice(start, end).map(function(record) {
      return withDisplayFields(record);
    });

    that.setData({
      currentPage: page,
      displayHistory: pageRecords
    });
  },

  /**
   * 上一页
   */
  prevPage() {
    if (this.data.currentPage <= 1) return;
    this.loadPage(this.data.currentPage - 1);
  },

  /**
   * 下一页
   */
  nextPage() {
    if (this.data.currentPage >= this.data.totalPages) return;
    this.loadPage(this.data.currentPage + 1);
  },

  /**
   * 拍照识别按钮
   */
  onCapture() {
    var that = this;
    if (this.data.isPolling) return;

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
        that.startPolling();
      })
      .catch(function(err) {
        console.error('发送指令失败:', err);
        var errorInfo = getCaptureErrorInfo(err);
        that.setData({
          isPolling: false,
          statusMessage: errorInfo.statusMessage,
          statusType: 'error'
        });
        wx.showModal({
          title: errorInfo.title,
          content: errorInfo.modalContent,
          showCancel: false,
          confirmText: '知道了'
        });
      });
  },

  /**
   * 开始轮询 OBS record.csv
   */
  startPolling() {
    var that = this;

    console.log('[轮询开始] lastRecordCount:', that.data.lastRecordCount);

    // 每 3 秒轮询一次
    this._pollTimer = setInterval(function() {
      var newPollCount = that.data.pollCount + 1;

      console.log('[轮询] 第', newPollCount, '次查询');

      obs.getRecordCsv()
        .then(function(csvText) {
          console.log('[轮询] CSV 内容长度:', csvText.length);

          var records = obs.parseRecordCsv(csvText);
          var newCount = records.length;

          console.log('[轮询] 解析后记录数:', newCount);
          console.log('[轮询] lastRecordCount:', that.data.lastRecordCount);
          console.log('[轮询] 是否有新记录:', newCount > that.data.lastRecordCount);

          // 只更新 pollCount，不更新其他数据，避免频繁刷新
          that.setData({ pollCount: newPollCount });

          if (newCount > that.data.lastRecordCount) {
            // 发现新记录，停止轮询
            that.stopPolling();

            // 更新历史记录
            var newRecord = records[0]; // 最新的记录（已 reverse）
            var latestRecord = withDisplayFields(newRecord);

            console.log('[轮询] 新记录详情:', JSON.stringify(latestRecord, null, 2));

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

            // 加载最新图片
            that.loadLatestImage(newRecord);

            that.loadPage(currentPage);

            wx.showToast({
              title: '识别完成',
              icon: 'success'
            });

            return;
          } else {
            // 没有新记录，只更新状态文字，不刷新数据
            that.setData({
              statusMessage: '正在识别中... (' + newPollCount + 's)',
              statusType: ''
            });
          }

          // 超时检查（30 秒）
          if (newPollCount >= 10) {
            that.stopPolling();
            var totalPages = Math.ceil(records.length / that.data.pageSize);
            var currentPage = Math.min(that.data.currentPage, totalPages || 1);

            // 超时后才更新数据
            that.setData({
              isPolling: false,
              statusMessage: '识别超时，请手动刷新查看',
              statusType: 'error',
              allHistory: records,
              lastRecordCount: newCount,
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
          }
        })
        .catch(function(err) {
          console.error('[轮询失败]:', err);
          // 静默失败，继续轮询，只更新状态文字
          that.setData({
            statusMessage: '正在识别中... (网络异常，继续尝试)',
            statusType: 'error'
          });
        });
    }, 3000);
  },

  /**
   * 停止轮询
   */
  stopPolling() {
    if (this._pollTimer) {
      clearInterval(this._pollTimer);
      this._pollTimer = null;
    }
    this.setData({ isPolling: false });
  },

  /**
   * 加载最新记录的图片（生成带签名的 imageUrl）
   */
  loadLatestImage(record) {
    if (!record || !record.imageName) return;

    var that = this;
    // 使用 imageName 生成带签名的 URL
    var signedUrl = obs.getImageUrl(record.imageName);
    var latestRecord = this.data.latestRecord;
    if (latestRecord && latestRecord.imageName === record.imageName) {
      latestRecord = Object.assign({}, latestRecord, { imageUrl: signedUrl });
    }
    that.setData({
      latestImageUrl: signedUrl,
      latestImageLoaded: false,
      latestRecord: latestRecord || this.data.latestRecord
    });
  },

  /**
   * 点击历史记录打开预览（使用 CSV 中的 imageUrl）
   */
  onRecordTap(e) {
    var index = e.currentTarget.dataset.index;
    var record = this.data.displayHistory[index];
    if (!record) return;

    this.setData({
      showPreview: true,
      previewImageUrl: record.imageUrl,
      previewRecord: record
    });
  },

  /**
   * 点击最新记录打开预览
   */
  onLatestRecordTap() {
    var record = this.data.latestRecord;
    if (!record || !record.imageName) return;

    this.setData({
      showPreview: true,
      previewImageUrl: record.imageUrl || obs.getImageUrl(record.imageName),
      previewRecord: record
    });
  },

  /**
   * 关闭图片预览
   */
  closePreview() {
    this.setData({
      showPreview: false,
      previewImageUrl: '',
      previewRecord: {}
    });
  },

  /**
   * 图片加载完成
   */
  onImageLoad() {
    this.setData({ latestImageLoaded: true });
  },

  /**
   * 图片加载失败
   */
  onImageError() {
    console.log('图片加载失败');
    this.setData({ latestImageLoaded: true });
  },

  /**
   * 阻止事件冒泡
   */
  preventClose() {
    // 阻止点击预览区域关闭
  }
});
