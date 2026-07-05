# IoTDA 命令下发模块

**源文件：** `utils/huawei-iot.js`

## 职责

1. 通过华为云 IAM API 获取认证 Token
2. 使用 Token 调用 IoTDA API 向 ESP32 设备下发命令
3. 提供设备影子查询接口（备用）

---

## 连接配置

文件顶部集中管理所有华为云连接参数：

```javascript
const DOMAINNAME = 'starrysky2006';
const USERNAME = 'developer1';
const PASSWORD = '9&2gLXusQk2JwkOV2G$S1i2FN!Aul9xe';
const PROJECT_ID = '961ef7ebef4d491db148a8dd0208a894';
const DEVICE_ID = '69e6502fe094d6159234626c_ESP32-S3-CAM';
const IAM_HTTPS = 'iam.cn-east-3.myhuaweicloud.com';
const IOTDA_HTTPS = '015487493c.st1.iotda-app.cn-east-3.myhuaweicloud.com';
const SERVICE_ID = 'GarbageClassification';
```

其中 `SERVICE_ID` 必须与 ESP32 端产品模型一致。硬件端 `AIoT-Project/include/constants.h` 中定义的服务 ID 也是 `GarbageClassification`。

---

## 辅助函数

### getHeaderValue()

从响应头中不区分大小写地获取指定头字段的值。因为华为云响应头大小写不一，需要用此函数兼容读取：

```javascript
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
```

**用途：** 主要用来从 IAM 响应头中提取 `X-Subject-Token`。

### 错误工厂函数

```javascript
function createCloudError(type, message, options) {
  var err = new Error(message);
  options = options || {};
  err.type = type;                 // 错误分类标识
  err.statusCode = options.statusCode || 0;
  err.errorCode = options.errorCode || '';
  err.detail = options.detail || '';
  err.raw = options.raw || null;
  return err;
}
```

### createResponseError()

根据 HTTP 状态码和响应内容，将 IoTDA 错误自动归类：

```javascript
function createResponseError(action, res) {
  var data = res && res.data;
  var statusCode = res ? res.statusCode : 0;
  var errorCode = data && (data.error_code || data.code || data.errorCode);
  var detail = getErrorDetail(data);

  // 检查设备离线关键词
  if (text.indexOf('offline') !== -1 || text.indexOf('设备不在线') !== -1 || ...) {
    type = 'device_offline';
    message = action + '失败：设备不在线或命令无法到达设备';
  } else if (statusCode === 401 || statusCode === 403) {
    type = 'auth_config';    // 认证相关错误
  } else if (statusCode === 404) {
    // 项目/设备/IoTDA 地址错误
  } else if (statusCode === 400) {
    // 服务ID/命令名/设备模型配置错误
  }
  // ... 返回结构化错误对象
}
```

**错误类型对应表：**

| type | 触发条件 | 含义 |
|------|---------|------|
| `auth_config` | 401/403 | 账号、密码、Token 或权限错误 |
| `cloud_config` | 其他错误码 | IoTDA 配置错误 |
| `network_config` | 网络请求失败 | 网络不通或未配置合法域名 |
| `device_offline` | 错误消息含 offline 等关键词 | 设备不在线 |

### createNetworkError()

```javascript
function createNetworkError(action, err) {
  var detail = err && err.errMsg ? err.errMsg : '';
  return createCloudError(
    'network_config',
    action + '失败：无法连接到云平台，请检查请求域名、网络或小程序合法域名配置',
    { detail: detail, raw: err }
  );
}
```

---

## 核心函数

### getToken() — 获取 IAM Token

**流程：**
1. 先检查 `wx.getStorageSync('token')` 缓存，有则直接返回
2. 没有缓存则调用 IAM API 获取
3. 成功后存入缓存

```javascript
function getToken() {
  return new Promise((resolve, reject) => {
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
```

**注意：** IAM scope 的区域（`cn-east-3`）必须与 IoTDA Endpoint 的区域一致，否则 Token 无法用于该区域的 IoTDA。

### sendCapture() — 发送识别命令

对外暴露的入口函数，封装了 Token 获取和自动重试逻辑：

```javascript
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
```

**设计要点：**
- Token 过期时（401/403），自动清除缓存、重新获取 Token、重试命令
- 如果重试仍失败，错误会向上传播给页面层处理

### sendCaptureWithToken() — 实际发送命令

```javascript
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
        if (res.statusCode === 200) {
          resolve(res.data);
        } else {
          reject(createResponseError('命令发送', res));
        }
      },
      fail(err) {
        reject(createNetworkError('命令发送', err));
      }
    });
  });
}
```

**请求体细节：**
- 使用 `JSON.stringify()` 包装请求体 — 微信小程序环境下必须显式序列化，否则 IoTDA 可能返回 400 Bad Request
- `service_id` 必须与 ESP32 产品模型中的 service ID 一致
- `command_name` 必须与 ESP32 端解析的命令名一致
- `paras` 为空对象，当前命令不需要额外参数

### getShadow() — 获取设备影子（备用）

```javascript
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
```

当前页面层未使用此函数，保留作为备用调试接口。可用来查询设备在线状态、最近上报数据等。

---

## 导出接口

```javascript
module.exports = {
  getToken,       // 获取 IAM Token
  sendCapture,    // 发送 capture 命令
  getShadow       // 获取设备影子
};
```

---

## 调用关系

```
页面层 (index.js)
  │
  ├─ iot.sendCapture()
  │    ├─ getToken()              ──→ IAM API (POST /v3/auth/tokens)
  │    └─ sendCaptureWithToken()  ──→ IoTDA API (POST /v5/iot/.../commands)
  │
  └─ iot.getShadow()              ──→ IoTDA API (GET /v5/iot/.../shadow)
```

---

## 注意事项

1. **Token 缓存生命周期：** Token 缓存没有过期时间检查，完全依赖 401/403 响应触发重获取。华为云 IAM Token 有效期一般为 24 小时。
2. **区域一致性：** IAM scope 区域必须与 IoTDA 区域一致，跨区域 Token 无法使用。
3. **`service_id` 匹配：** 这个值必须与 ESP32 产品模型中的 service id 完全一致（大小写敏感）。
4. **请求体序列化：** 微信小程序 `wx.request` 不支持自动序列化对象，必须手动调用 `JSON.stringify()`。
5. **AK/SK 安全：** IAM 密码直接硬编码在源码中，适合课程演示。正式产品应通过后端代理或云函数调用 IoTDA，避免密钥泄露。
