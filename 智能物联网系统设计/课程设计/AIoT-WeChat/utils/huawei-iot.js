// utils/huawei-iot.js

// ========== 华为云 IoTDA 连接参数 ==========
const DOMAINNAME = 'starrysky2006';
const USERNAME = 'developer1';
const PASSWORD = '9&2gLXusQk2JwkOV2G$S1i2FN!Aul9xe';
const PROJECT_ID = '961ef7ebef4d491db148a8dd0208a894';
const DEVICE_ID = '69e6502fe094d6159234626c_ESP32-S3-CAM';
const IAM_HTTPS = 'iam.cn-east-3.myhuaweicloud.com';
const IOTDA_HTTPS = '015487493c.st1.iotda-app.cn-east-3.myhuaweicloud.com';
const SERVICE_ID = 'GarbageClassification';

// ========== Token 管理 ==========

function getHeaderValue(headers, name) {
  if (!headers) return '';
  var target = name.toLowerCase();
  for (var key in headers) {
    if (key.toLowerCase() === target) {
      return headers[key];
    }
  }
  return '';
}

function getErrorDetail(data) {
  if (!data) return '';
  if (typeof data === 'string') return data;
  return data.error_msg || data.message || data.error || JSON.stringify(data);
}

function createCloudError(type, message, options) {
  var err = new Error(message);
  options = options || {};
  err.type = type;
  err.statusCode = options.statusCode || 0;
  err.errorCode = options.errorCode || '';
  err.detail = options.detail || '';
  err.raw = options.raw || null;
  return err;
}

function createResponseError(action, res) {
  var data = res && res.data;
  var statusCode = res ? res.statusCode : 0;
  var errorCode = data && (data.error_code || data.code || data.errorCode);
  var detail = getErrorDetail(data);
  var text = (String(errorCode || '') + ' ' + detail).toLowerCase();
  var type = 'cloud_config';
  var message = action + '失败：连接设置或云平台配置错误';

  if (statusCode === 401 || statusCode === 403) {
    type = 'auth_config';
    message = action + '失败：账号、密码、Token 或权限配置错误';
  } else if (
    text.indexOf('offline') !== -1 ||
    text.indexOf('not online') !== -1 ||
    text.indexOf('disconnected') !== -1 ||
    text.indexOf('unreachable') !== -1 ||
    text.indexOf('timeout') !== -1 ||
    text.indexOf('设备不在线') !== -1 ||
    text.indexOf('离线') !== -1 ||
    text.indexOf('超时') !== -1
  ) {
    type = 'device_offline';
    message = action + '失败：设备不在线或命令无法到达设备';
  } else if (statusCode === 404) {
    message = action + '失败：项目、设备 ID 或 IoTDA 地址配置错误';
  } else if (statusCode === 400) {
    message = action + '失败：服务 ID、命令名称或设备模型配置错误';
  }

  return createCloudError(type, message, {
    statusCode: statusCode,
    errorCode: errorCode,
    detail: detail,
    raw: res
  });
}

function createNetworkError(action, err) {
  var detail = err && err.errMsg ? err.errMsg : '';
  return createCloudError(
    'network_config',
    action + '失败：无法连接到云平台，请检查请求域名、网络或小程序合法域名配置',
    { detail: detail, raw: err }
  );
}

/**
 * 获取 IAM Token
 * 返回 Promise<string> token
 */
function getToken() {
  return new Promise((resolve, reject) => {
    // 先检查缓存
    const cached = wx.getStorageSync('token');
    if (cached) {
      resolve(cached);
      return;
    }

    wx.request({
      url: `https://${IAM_HTTPS}/v3/auth/tokens`,
      data: {
        auth: {
          identity: {
            methods: ['password'],
            password: {
              user: {
                name: USERNAME,
                password: PASSWORD,
                domain: { name: DOMAINNAME }
              }
            }
          },
          scope: { project: { name: 'cn-east-3' } }
        }
      },
      method: 'POST',
      header: { 'content-type': 'application/json' },
      success(res) {
        const token = getHeaderValue(res.header, 'X-Subject-Token');
        if (token) {
          wx.setStorageSync('token', token);
          console.log('获取 Token 成功:', token);
          resolve(token);
        } else {
          reject(createResponseError('获取 Token', res));
        }
      },
      fail(err) {
        reject(createNetworkError('获取 Token', err));
      }
    });
  });
}

/**
 * 发送 capture 命令到 ESP32
 * service_id 需与 ESP32 端产品模型定义一致（GarbageClassification）
 * command_name: capture
 */
function sendCapture() {
  return getToken().then(token => {
    return sendCaptureWithToken(token);
  }).catch(err => {
    // Token 过期或无效时清除缓存重试一次
    if (err.statusCode === 401 || err.statusCode === 403) {
      wx.removeStorageSync('token');
      return getToken().then(token => {
        return sendCaptureWithToken(token);
      });
    }
    throw err;
  });
}

function sendCaptureWithToken(token) {
  return new Promise((resolve, reject) => {
    wx.request({
      url: `https://${IOTDA_HTTPS}/v5/iot/${PROJECT_ID}/devices/${DEVICE_ID}/commands`,
      data: JSON.stringify({
        service_id: SERVICE_ID,
        command_name: 'capture',
        paras: {}
      }),
      method: 'POST',
      header: {
        'content-type': 'application/json',
        'X-Auth-Token': token
      },
      success(res) {
        console.log('命令下发响应:', res);
        if (res.statusCode === 200) {
          resolve(res.data);
        } else {
          console.error('命令下发失败，响应:', res);
          reject(createResponseError('命令发送', res));
        }
      },
      fail(err) {
        console.error('命令发送网络请求失败:', err);
        reject(createNetworkError('命令发送', err));
      }
    });
  });
}

/**
 * 获取设备影子（备用，可用于查看设备状态）
 */
function getShadow() {
  return getToken().then(token => {
    return new Promise((resolve, reject) => {
      wx.request({
        url: `https://${IOTDA_HTTPS}/v5/iot/${PROJECT_ID}/devices/${DEVICE_ID}/shadow`,
        method: 'GET',
        header: {
          'content-type': 'application/json',
          'X-Auth-Token': token
        },
        success(res) {
          if (res.statusCode === 200 && res.data.shadow && res.data.shadow.length > 0) {
            resolve(res.data.shadow[0].reported.properties);
          } else {
            reject(new Error('设备影子数据为空'));
          }
        },
        fail(err) {
          reject(new Error('获取设备影子失败: ' + (err.errMsg || '')));
        }
      });
    });
  });
}

module.exports = {
  getToken,
  sendCapture,
  getShadow
};
