// utils/obs-service.js

// ========== 华为云 OBS 连接参数 ==========
const ACCESS_KEY_ID = 'HPUAW2GYOGAJ2OGCYLLP';
const SECRET_ACCESS_KEY = 'aFEoJpm8QvYlXOfi6w3WXCLAoIoZav2ClfVIjiwE';
const ENDPOINT = 'obs.cn-east-3.myhuaweicloud.com';
const BUCKET_NAME = 'iotda-obs-data';
const OBS_PREFIX = 'https://' + BUCKET_NAME + '.' + ENDPOINT;
const RECORD_CSV_KEY = 'Images/records.csv';

// ========== HMAC-SHA1 纯 JS 实现 ==========
// 基于 Wei/JSAES 的轻量级 HMAC-SHA1
// 用于微信小程序环境（无 Node.js crypto）

/**
 * SHA-1 内部实现（简化版，纯 JS）
 */
var HmacSha1 = (function() {
  function rotl(n, s) { return (n << s) | (n >>> (32 - s)); }

  function sha1(message) {
    var H = [0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0];

    // 预处理：添加 padding
    var ml = message.length * 8;  // 消息长度（bits）

    // 计算需要填充的字节数
    // 消息 + 1 byte (0x80) + k bytes (0x00) + 8 bytes (长度) = n * 64 bytes
    var paddingBytes = 64 - ((message.length + 9) % 64);
    if (paddingBytes === 64) paddingBytes = 0;

    var totalLength = message.length + 1 + paddingBytes + 8;
    var padded = new Array(totalLength);

    // 复制原始消息
    for (var i = 0; i < message.length; i++) {
      padded[i] = message[i];
    }

    // 添加 0x80
    padded[message.length] = 0x80;

    // 填充 0x00
    for (var i = 0; i < paddingBytes; i++) {
      padded[message.length + 1 + i] = 0x00;
    }

    // 添加长度（big-endian，64 bits）
    for (var i = 0; i < 8; i++) {
      padded[totalLength - 8 + i] = 0x00;  // 前 4 bytes 为 0（消息长度 < 2^32）
    }
    padded[totalLength - 4] = (ml >>> 24) & 0xFF;
    padded[totalLength - 3] = (ml >>> 16) & 0xFF;
    padded[totalLength - 2] = (ml >>> 8) & 0xFF;
    padded[totalLength - 1] = ml & 0xFF;

    // 转换为 32-bit words
    var msg = [];
    for (var i = 0; i < padded.length; i += 4) {
      msg.push(
        (padded[i] << 24) |
        (padded[i + 1] << 16) |
        (padded[i + 2] << 8) |
        padded[i + 3]
      );
    }

    // 处理每个 512-bit block
    var W = [];
    for (var i = 0; i < msg.length; i += 16) {
      // 初始化 a, b, c, d, e（在循环外部）
      var a = H[0], b = H[1], c = H[2], d = H[3], e = H[4];

      for (var j = 0; j < 80; j++) {
        if (j < 16) W[j] = msg[i + j];
        else {
          W[j] = rotl(W[j-3] ^ W[j-8] ^ W[j-14] ^ W[j-16], 1);
        }
        var f, k;
        if (j < 20) { f = (b & c) | (~b & d); k = 0x5A827999; }
        else if (j < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
        else if (j < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
        else { f = b ^ c ^ d; k = 0xCA62C1D6; }
        var t = (rotl(a, 5) + f + e + k + W[j]) | 0;  // | 0 强制转换为 32-bit integer
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

  function hmac(key, data) {
    var blockSize = 64;
    var keyBytes = typeof key === 'string' ? strToBytes(key) : key;

    if (keyBytes.length > blockSize) {
      keyBytes = sha1(keyBytes);
      keyBytes = hashToBytes(keyBytes);
    }

    var ipad = [];
    var opad = [];
    for (var i = 0; i < blockSize; i++) {
      ipad.push((keyBytes[i] || 0x00) ^ 0x36);
      opad.push((keyBytes[i] || 0x00) ^ 0x5C);
    }

    var innerMsg = ipad.concat(data);
    var innerHash = sha1(innerMsg);
    var innerBytes = hashToBytes(innerHash);
    var outerMsg = opad.concat(innerBytes);
    var outerHash = sha1(outerMsg);
    return hashToBytes(outerHash);
  }

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

  function bytesToBase64(bytes) {
    var base64chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
    var result = '';
    var i = 0;
    while (i < bytes.length) {
      var b1 = bytes[i++];
      var b2 = i < bytes.length ? bytes[i++] : 256;  // 使用 256 作为"缺失"标记
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

  return {
    sign: function(key, data) {
      var dataBytes = typeof data === 'string' ? strToBytes(data) : data;
      var keyBytes = typeof key === 'string' ? strToBytes(key) : key;
      console.log('[HMAC] dataBytes length:', dataBytes.length);
      console.log('[HMAC] keyBytes length:', keyBytes.length);
      var hmacBytes = hmac(keyBytes, dataBytes);
      console.log('[HMAC] hmacBytes length:', hmacBytes.length);
      console.log('[HMAC] hmacBytes sample:', hmacBytes.slice(0, 5));
      var base64Result = bytesToBase64(hmacBytes);
      console.log('[HMAC] base64 result:', base64Result);
      return base64Result;
    }
  };
})();

// ========== 签名算法 (华为云 OBS V2) ==========

/**
 * 计算 OBS V2 签名
 */
function computeSignature(method, date, canonicalizedResource, contentMd5, contentType) {
  var stringToSign = method + '\n' + (contentMd5 || '') + '\n' + (contentType || '') + '\n' + date + '\n' + canonicalizedResource;
  console.log('===== OBS Signature Debug =====');
  console.log('StringToSign:', JSON.stringify(stringToSign));
  console.log('Method:', method);
  console.log('Date:', date);
  console.log('CanonicalizedResource:', canonicalizedResource);
  var signature = HmacSha1.sign(SECRET_ACCESS_KEY, stringToSign);
  console.log('Signature:', signature);
  console.log('===============================');
  return signature;
}

/**
 * 构造 GET 请求头
 */
function buildHeaders(method, objectKey) {
  var date = new Date().toUTCString();
  var canonicalizedResource = '/' + BUCKET_NAME + objectKey;
  // 微信小程序会自动添加 Content-Type: application/json，所以签名时也必须包含
  var signature = computeSignature(method, date, canonicalizedResource, '', 'application/json');
  var host = BUCKET_NAME + '.' + ENDPOINT;

  return {
    url: 'https://' + host + objectKey,
    headers: {
      'Host': host,
      'Authorization': 'OBS ' + ACCESS_KEY_ID + ':' + signature,
      'Date': date,
      'Content-Type': 'application/json'  // 显式添加，与签名一致
    }
  };
}

/**
 * 获取 record.csv 内容
 * 返回 Promise<string> CSV 文本
 */
function getRecordCsv() {
  var config = buildHeaders('GET', '/' + RECORD_CSV_KEY);

  // 添加时间戳参数，避免微信小程序缓存 GET 请求
  var timestamp = Date.now();
  var urlWithTimestamp = config.url + '?t=' + timestamp;

  console.log('[OBS GET] URL:', urlWithTimestamp);
  console.log('[OBS GET] Headers:', JSON.stringify(config.headers, null, 2));

  return new Promise(function(resolve, reject) {
    wx.request({
      url: urlWithTimestamp,
      method: 'GET',
      header: config.headers,
      success(res) {
        console.log('[OBS GET] Status:', res.statusCode);
        console.log('[OBS GET] Response Headers:', JSON.stringify(res.header, null, 2));
        if (res.statusCode === 200) {
          resolve(res.data);
        } else {
          console.error('[OBS GET] Error Body:', res.data);
          reject(new Error('获取 record.csv 失败: ' + res.statusCode + ' - ' + JSON.stringify(res.data)));
        }
      },
      fail(err) {
        console.error('[OBS GET] Network Error:', err);
        reject(new Error('获取 record.csv 网络请求失败: ' + (err.errMsg || '')));
      }
    });
  });
}

/**
 * 解析 CSV 文本为结构化数据
 * 格式: Status,GarbageType,Confidence,ImageUrl,ObjectName,Timestamp,Time
 * 返回: [{time, result, imageName}]
 */
function parseRecordCsv(csvText) {
  var lines = csvText.trim().split('\n');
  var records = [];

  // 跳过 header 行（如果存在）
  var startIndex = 0;
  if (lines.length > 0 && lines[0].indexOf('Status') !== -1) {
    startIndex = 1;
  }

  for (var i = startIndex; i < lines.length; i++) {
    var line = lines[i].trim();
    if (!line) continue;
    var parts = line.split(',');
    if (parts.length >= 7) {
      var rawTime = parts[6].trim(); // 如 "20260425T125427Z"
      var formattedTime = formatTime(rawTime);

      records.push({
        status: parts[0].trim(),     // 设备状态（如"识别完成"）
        result: parts[1].trim(),     // 垃圾类型（英文，如 "Recyclable"）
        confidence: parts[2].trim(), // AI 置信度
        imageUrl: parts[3].trim(),   // 图片直接访问 URL
        imageName: parts[4].trim(),  // OBS 对象名称（完整路径）
        timestamp: parts[5].trim(),  // 时间戳
        time: formattedTime,         // 格式化后的时间（如 "2026-04-25 12:54:27"）
        rawTime: rawTime             // 原始时间（备用）
      });
    }
  }

  // 从新到旧排列
  return records.reverse();
}

/**
 * 格式化时间字符串
 * 输入: "20260425T125427Z" 或类似格式
 * 输出: "2026-04-25 12:54:27"
 */
function formatTime(rawTime) {
  if (!rawTime) return '未知时间';

  // 尝试解析 ISO 8601 格式：20260425T125427Z
  var match = rawTime.match(/^(\d{4})(\d{2})(\d{2})T(\d{2})(\d{2})(\d{2})Z?$/);
  if (match) {
    var year = match[1];
    var month = match[2];
    var day = match[3];
    var hour = match[4];
    var minute = match[5];
    var second = match[6];
    return year + '-' + month + '-' + day + ' ' + hour + ':' + minute + ':' + second;
  }

  // 如果已经是其他格式，直接返回
  return rawTime;
}

/**
 * 获取图片 URL（带签名，用于直接访问）
 * 使用查询参数签名方式，支持 <image> 标签直接加载
 * 返回: string URL
 */
function getImageUrl(imageName) {
  if (!imageName) return '';

  // 构造对象键：如果是完整路径（含 /）则直接使用，否则默认放在 Images/ 目录下
  var objectKey;
  if (imageName.indexOf('/') !== -1) {
    // 完整路径，如 "Images/garbage_27100.jpg"
    var segments = imageName.split('/');
    var encodedSegments = segments.map(function(s) { return encodeURIComponent(s); });
    objectKey = '/' + encodedSegments.join('/');
  } else {
    // 仅文件名，补全 Images/ 路径
    objectKey = '/Images/' + encodeURIComponent(imageName);
  }

  // 计算过期时间（当前时间 + 15分钟）
  var expires = Math.floor(Date.now() / 1000) + 900;

  // 构造 StringToSign（查询参数签名格式）
  var canonicalizedResource = '/' + BUCKET_NAME + objectKey;
  var stringToSign = 'GET\n\n\n' + expires + '\n' + canonicalizedResource;

  // 计算签名
  var signature = HmacSha1.sign(SECRET_ACCESS_KEY, stringToSign);

  // 构造带签名的 URL
  var url = OBS_PREFIX + objectKey +
            '?AccessKeyId=' + ACCESS_KEY_ID +
            '&Expires=' + expires +
            '&Signature=' + encodeURIComponent(signature);

  return url;
}

/**
 * 获取图片内容（用于缓存或特殊处理）
 * 返回 Promise 图片 base64 数据
 */
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

module.exports = {
  getRecordCsv,
  parseRecordCsv,
  getImageUrl,
  getImage
};
