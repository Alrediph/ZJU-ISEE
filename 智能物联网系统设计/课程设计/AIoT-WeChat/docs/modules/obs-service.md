# OBS 数据获取模块

**源文件：** `utils/obs-service.js`

## 职责

1. 纯 JS 实现 HMAC-SHA1 签名算法（微信环境无 crypto 模块）
2. 构造 OBS V2 签名请求头，从 OBS 读取 `Images/records.csv`
3. 解析 CSV 为结构化数据
4. 为图片生成临时签名 URL（查询参数签名方式），支持 `<image>` 标签直接加载

---

## OBS 连接配置

```javascript
const ACCESS_KEY_ID = 'HPUAW2GYOGAJ2OGCYLLP';
const SECRET_ACCESS_KEY = 'aFEoJpm8QvYlXOfi6w3WXCLAoIoZav2ClfVIjiwE';
const ENDPOINT = 'obs.cn-east-3.myhuaweicloud.com';
const BUCKET_NAME = 'iotda-obs-data';
const OBS_PREFIX = 'https://' + BUCKET_NAME + '.' + ENDPOINT;  // https://iotda-obs-data.obs.cn-east-3.myhuaweicloud.com
const RECORD_CSV_KEY = 'Images/records.csv';
```

---

## HMAC-SHA1 纯 JS 实现

### 为什么需要纯 JS 实现？

微信小程序运行环境没有 Node.js 的 `crypto` 模块，不能直接使用 `require('crypto')`。必须使用纯 JS 实现 SHA-1 哈希和 HMAC 算法。

### SHA-1 实现

```javascript
var HmacSha1 = (function() {
  function rotl(n, s) { return (n << s) | (n >>> (32 - s)); }

  function sha1(message) {
    var H = [0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0];

    // 预处理：添加 padding
    var ml = message.length * 8;
    var paddingBytes = 64 - ((message.length + 9) % 64);
    if (paddingBytes === 64) paddingBytes = 0;

    var totalLength = message.length + 1 + paddingBytes + 8;
    var padded = new Array(totalLength);

    // 复制原始消息 + 添加 0x80 + 填充 0x00
    for (var i = 0; i < message.length; i++) padded[i] = message[i];
    padded[message.length] = 0x80;
    for (var i = 0; i < paddingBytes; i++) padded[message.length + 1 + i] = 0x00;

    // 添加长度（big-endian，64 bits）— 消息长度 < 2^32，前4字节为0
    padded[totalLength - 4] = (ml >>> 24) & 0xFF;
    padded[totalLength - 3] = (ml >>> 16) & 0xFF;
    padded[totalLength - 2] = (ml >>> 8) & 0xFF;
    padded[totalLength - 1] = ml & 0xFF;

    // 转换为 32-bit words
    var msg = [];
    for (var i = 0; i < padded.length; i += 4) {
      msg.push((padded[i] << 24) | (padded[i+1] << 16) | (padded[i+2] << 8) | padded[i+3]);
    }

    // 处理每个 512-bit block
    var W = [];
    for (var i = 0; i < msg.length; i += 16) {
      // ★ 关键：a, b, c, d, e 必须在 j 循环外部初始化
      var a = H[0], b = H[1], c = H[2], d = H[3], e = H[4];

      for (var j = 0; j < 80; j++) {
        if (j < 16) W[j] = msg[i + j];
        else W[j] = rotl(W[j-3] ^ W[j-8] ^ W[j-14] ^ W[j-16], 1);

        var f, k;
        if (j < 20)      { f = (b & c) | (~b & d); k = 0x5A827999; }
        else if (j < 40) { f = b ^ c ^ d;          k = 0x6ED9EBA1; }
        else if (j < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
        else             { f = b ^ c ^ d;          k = 0xCA62C1D6; }

        var t = (rotl(a, 5) + f + e + k + W[j]) | 0;
        e = d; d = c; c = rotl(b, 30); b = a; a = t;
      }
      H[0] = (H[0] + a) | 0;
      H[1] = (H[1] + b) | 0;
      H[2] = (H[2] + c) | 0;
      H[3] = (H[3] + d) | 0;
      H[4] = (H[4] + e) | 0;
    }
    return H;
  }
  // ...
```

**重要 Bug 修复记录：** 早期版本将 `var a, b, c, d, e` 的初始化放在了 `j` 循环（80轮）内部，导致每轮都重置为初始 H 值，80 轮的累积状态被破坏。修复后将初始化移到 `j` 循环外、`i` 循环内。

### HMAC 实现

```javascript
function hmac(key, data) {
  var blockSize = 64;
  var keyBytes = typeof key === 'string' ? strToBytes(key) : key;

  // 密钥超过 blockSize 时先哈希
  if (keyBytes.length > blockSize) {
    keyBytes = sha1(keyBytes);
    keyBytes = hashToBytes(keyBytes);
  }

  var ipad = [];
  var opad = [];
  for (var i = 0; i < blockSize; i++) {
    ipad.push((keyBytes[i] || 0x00) ^ 0x36);  // ★ 注意括号优先级
    opad.push((keyBytes[i] || 0x00) ^ 0x5C);
  }

  var innerMsg = ipad.concat(data);
  var innerHash = sha1(innerMsg);
  var innerBytes = hashToBytes(innerHash);
  var outerMsg = opad.concat(innerBytes);
  var outerHash = sha1(outerMsg);
  return hashToBytes(outerHash);
}
```

**注意：** `(keyBytes[i] || 0x00) ^ 0x36` 的括号不能省略。`x || y ^ z` 的优先级是 `x || (y ^ z)`，如果 keyBytes[i] 是 `undefined`，结果会是 `0x00 ^ 0x36` 而非预期值。

### 辅助函数

```javascript
// 字符串 → 字节数组（支持 UTF-8）
function strToBytes(str) {
  var bytes = [];
  for (var i = 0; i < str.length; i++) {
    var c = str.charCodeAt(i);
    if (c < 128) bytes.push(c);
    else if (c < 2048) { bytes.push(192 | (c >> 6), 128 | (c & 63)); }
    else { bytes.push(224 | (c >> 12), 128 | ((c >> 6) & 63), 128 | (c & 63)); }
  }
  return bytes;
}

// 哈希数组 → 字节数组
function hashToBytes(hash) {
  var bytes = [];
  for (var i = 0; i < hash.length; i++) {
    bytes.push((hash[i] >> 24) & 0xFF);
    bytes.push((hash[i] >> 16) & 0xFF);
    bytes.push((hash[i] >> 8) & 0xFF);
    bytes.push(hash[i] & 0xFF);
  }
  return bytes;
}

// 字节数组 → Base64
function bytesToBase64(bytes) {
  var base64chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
  var result = '';
  var i = 0;
  while (i < bytes.length) {
    var b1 = bytes[i++];
    var b2 = i < bytes.length ? bytes[i++] : 256;  // 256 表示"缺失"
    var b3 = i < bytes.length ? bytes[i++] : 256;

    var enc1 = b1 >> 2;
    var enc2 = ((b1 & 3) << 4) | (b2 === 256 ? 0 : (b2 >> 4));
    var enc3 = b2 === 256 ? 64 : ((b2 & 15) << 2) | (b3 === 256 ? 0 : (b3 >> 6));
    var enc4 = b3 === 256 ? 64 : (b3 & 63);

    result += base64chars[enc1] + base64chars[enc2] +
              (enc3 === 64 ? '=' : base64chars[enc3]) +
              (enc4 === 64 ? '=' : base64chars[enc4]);
  }
  return result;
}
```

### 对外签名接口

```javascript
return {
  sign: function(key, data) {
    var dataBytes = typeof data === 'string' ? strToBytes(data) : data;
    var keyBytes = typeof key === 'string' ? strToBytes(key) : key;
    var hmacBytes = hmac(keyBytes, dataBytes);
    return bytesToBase64(hmacBytes);
  }
};
```

---

## 核心函数

### computeSignature() — 计算 OBS V2 签名

```javascript
function computeSignature(method, date, canonicalizedResource, contentMd5, contentType) {
  var stringToSign = method + '\n'
    + (contentMd5 || '') + '\n'
    + (contentType || '') + '\n'
    + date + '\n'
    + canonicalizedResource;

  return HmacSha1.sign(SECRET_ACCESS_KEY, stringToSign);
}
```

**OBS V2 签名字符串格式：**

```
HTTP Method
Content-MD5 (可为空)
Content-Type (可为空)
Date (RFC 1123 格式)
CanonicalizedResource (/bucket/object-key)
```

### buildHeaders() — 构造签名请求头

```javascript
function buildHeaders(method, objectKey) {
  var date = new Date().toUTCString();
  var canonicalizedResource = '/' + BUCKET_NAME + objectKey;
  // 微信小程序自动添加 Content-Type: application/json，签名必须包含
  var signature = computeSignature(method, date, canonicalizedResource, '', 'application/json');
  var host = BUCKET_NAME + '.' + ENDPOINT;

  return {
    url: 'https://' + host + objectKey,
    headers: {
      'Host': host,
      'Authorization': 'OBS ' + ACCESS_KEY_ID + ':' + signature,
      'Date': date,
      'Content-Type': 'application/json'  // 必须显式添加，与签名一致
    }
  };
}
```

**重要：** 微信小程序 `wx.request` 会自动添加 `Content-Type: application/json` 请求头。为了避免签名不匹配导致 403，签名计算时必须将 `Content-Type` 设为 `application/json`，请求头中也要显式设置。

### getRecordCsv() — 获取 CSV 文件

```javascript
function getRecordCsv() {
  var config = buildHeaders('GET', '/' + RECORD_CSV_KEY);

  // 添加时间戳参数，避免微信缓存 GET 请求
  var timestamp = Date.now();
  var urlWithTimestamp = config.url + '?t=' + timestamp;

  return new Promise(function(resolve, reject) {
    wx.request({
      url: urlWithTimestamp,
      method: 'GET',
      header: config.headers,
      success(res) {
        if (res.statusCode === 200) {
          resolve(res.data);
        } else {
          reject(new Error('获取 record.csv 失败: ' + res.statusCode + ' - ' + JSON.stringify(res.data)));
        }
      },
      fail(err) {
        reject(new Error('获取 record.csv 网络请求失败: ' + (err.errMsg || '')));
      }
    });
  });
}
```

**缓存规避：** `?t=` + `Date.now()` 时间戳参数确保每次请求 URL 不同，避免微信小程序缓存 GET 请求导致读到旧数据。

### parseRecordCsv() — 解析 CSV

```javascript
function parseRecordCsv(csvText) {
  var lines = csvText.trim().split('\n');
  var records = [];

  // 跳过 header 行
  var startIndex = 0;
  if (lines.length > 0 && lines[0].indexOf('Status') !== -1) {
    startIndex = 1;
  }

  for (var i = startIndex; i < lines.length; i++) {
    var line = lines[i].trim();
    if (!line) continue;
    var parts = line.split(',');
    if (parts.length >= 7) {
      records.push({
        status: parts[0].trim(),     // 设备状态（如"识别完成"）
        result: parts[1].trim(),     // 垃圾类型（英文，如 "Recyclable"）
        confidence: parts[2].trim(), // AI 置信度
        imageUrl: parts[3].trim(),   // 图片直接访问 URL
        imageName: parts[4].trim(),  // OBS 对象名称（完整路径）
        timestamp: parts[5].trim(),  // 时间戳
        time: formatTime(parts[6].trim()),  // 格式化后的时间
        rawTime: parts[6].trim()     // 原始时间
      });
    }
  }
  return records.reverse();  // 最新在前
}
```

**CSV 格式（7 列）：**

```
Status,GarbageType,Confidence,ImageUrl,ObjectName,Timestamp,Time
识别完成,Recyclable,0.85,https://...,Images/garbage_27100.jpg,27100,20260425T125427Z
```

**字段映射：**

| CSV 列 | 字段名 | 说明 |
|--------|--------|------|
| 第 1 列 | `status` | 设备状态信息 |
| 第 2 列 | `result` | 垃圾类型（英文/中文） |
| 第 3 列 | `confidence` | 置信度 |
| 第 4 列 | `imageUrl` | 图片直链 URL |
| 第 5 列 | `imageName` | OBS 对象存储路径 |
| 第 6 列 | `timestamp` | 时间戳 |
| 第 7 列 | `time` / `rawTime` | 格式化/原始时间 |

### formatTime() — 时间格式化

```javascript
function formatTime(rawTime) {
  if (!rawTime) return '未知时间';

  // 解析 ISO 8601 格式：20260425T125427Z
  var match = rawTime.match(/^(\d{4})(\d{2})(\d{2})T(\d{2})(\d{2})(\d{2})Z?$/);
  if (match) {
    return match[1] + '-' + match[2] + '-' + match[3] + ' '
         + match[4] + ':' + match[5] + ':' + match[6];
  }
  return rawTime;
}
```

**输入示例：** `20260425T125427Z` → **输出：** `2026-04-25 12:54:27`

### getImageUrl() — 生成图片签名 URL

```javascript
function getImageUrl(imageName) {
  if (!imageName) return '';

  // 构造对象键
  var objectKey;
  if (imageName.indexOf('/') !== -1) {
    var segments = imageName.split('/');
    var encodedSegments = segments.map(function(s) { return encodeURIComponent(s); });
    objectKey = '/' + encodedSegments.join('/');
  } else {
    objectKey = '/Images/' + encodeURIComponent(imageName);
  }

  // 过期时间：当前时间 + 15 分钟
  var expires = Math.floor(Date.now() / 1000) + 900;

  // 查询参数签名 StringToSign
  var canonicalizedResource = '/' + BUCKET_NAME + objectKey;
  var stringToSign = 'GET\n\n\n' + expires + '\n' + canonicalizedResource;

  var signature = HmacSha1.sign(SECRET_ACCESS_KEY, stringToSign);

  return OBS_PREFIX + objectKey
    + '?AccessKeyId=' + ACCESS_KEY_ID
    + '&Expires=' + expires
    + '&Signature=' + encodeURIComponent(signature);
}
```

**签名方式选择：** 使用"查询参数签名"（Query String Signature）而非请求头签名，因为生成的 URL 可以直接用在 `<image>` 标签的 `src` 属性中，浏览器/小程序会自动发起 GET 请求加载图片。

**生成的 URL 格式：**

```
https://iotda-obs-data.obs.cn-east-3.myhuaweicloud.com/Images/garbage_27100.jpg
  ?AccessKeyId=HPUAW2GYOGAJ2OGCYLLP
  &Expires=1713512345
  &Signature=xxxxxxx
```

### getImage() — 获取图片二进制数据（备用）

```javascript
function getImage(imageName) {
  var objectKey = '/ESP32/' + imageName;
  var config = buildHeaders('GET', objectKey);
  return new Promise(function(resolve, reject) {
    wx.request({
      url: config.url,
      method: 'GET',
      header: config.headers,
      responseType: 'arraybuffer',
      success(res) {
        if (res.statusCode === 200) {
          var base64 = wx.arrayBufferToBase64(res.data);
          resolve('data:image/jpeg;base64,' + base64);
        } else {
          reject(new Error('获取图片失败: ' + res.statusCode));
        }
      },
      fail(err) {
        reject(new Error('获取图片网络请求失败: ' + (err.errMsg || '')));
      }
    });
  });
}
```

当前页面层未使用此函数（使用 `getImageUrl()` + `<image>` 标签效果更好），保留作为备用。

---

## 导出接口

```javascript
module.exports = {
  getRecordCsv,    // 获取 CSV 文本
  parseRecordCsv,  // 解析 CSV 为结构化数据
  getImageUrl,     // 生成图片签名 URL
  getImage         // 获取图片 base64 数据
};
```

---

## 调用关系

```
页面层 (index.js)
  │
  ├─ obs.getRecordCsv()
  │    └─ buildHeaders('GET', '/Images/records.csv')
  │         └─ computeSignature() → HmacSha1.sign(SECRET_ACCESS_KEY, stringToSign)
  │
  ├─ obs.parseRecordCsv(csvText)  (纯函数，无外部调用)
  │
  └─ obs.getImageUrl(imageName)
       └─ HmacSha1.sign(SECRET_ACCESS_KEY, stringToSign)  (查询参数签名格式)
```

---

## 注意事项

1. **Content-Type 一致性：** 微信 `wx.request` 会自动添加 `Content-Type: application/json` 请求头，签名计算时必须包含该值，且请求头中必须显式设置，否则返回 403。
2. **密钥暴露风险：** OBS AK/SK 直接硬编码在前端源码中，适合课程演示。正式产品应通过后端代理签名。
3. **图片 URL 时效性：** `getImageUrl()` 生成的 URL 有效期 15 分钟，页面切换后需要重新生成。
4. **SHA-1 验证方法：** 可以用已知测试向量验证实现正确性：SHA-1("abc") 应输出 `a9993e364706816aba3e25717850c26c9cd0d89d`。
5. **GET 缓存规避：** 时间戳参数 `?t=` 只对小程序缓存有效。OBS 本身不会缓存，但小程序端会。
