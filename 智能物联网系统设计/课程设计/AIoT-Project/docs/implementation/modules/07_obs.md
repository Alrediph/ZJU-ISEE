# 模块七：OBS 存储模块 (obs.cpp)

## 1. 模块定位

`obs.cpp` 是系统中最复杂的模块（446行），负责将拍摄的 JPEG 图片上传到华为云 OBS（对象存储服务），并生成短期有效的预签名下载 URL，供 AI 识别模块访问图片。

文件位置：`src/obs.cpp`（446行） | 头文件：`include/obs.h`

---

## 2. 技术背景

### 2.1 华为云 OBS API

华为云 OBS 兼容 Amazon S3 API，上传操作使用 HTTP PUT 方法，认证采用 **AWS Signature Version 4 (SigV4)** 签名算法。

### 2.2 为何需要预签名 URL？

OBS 桶设置为**私有读写**，外部无法直接访问存储对象。预签名 URL 通过在 URL 中嵌入临时签名凭证，允许持有者在指定时间内（600 秒）访问特定对象，无需公开桶权限。

---

## 3. AWS Signature V4 签名流程

这是模块最核心的技术实现，涉及多层 HMAC-SHA256 计算。

### 3.1 签名步骤总览

```
输入：图片数据 + 对象名称 + OBS 密钥
  │
  ├─ 步骤1：计算 Payload SHA256 哈希
  │    payloadHash = SHA256(imageData)
  │
  ├─ 步骤2：构造规范请求 (Canonical Request)
  │    "PUT\n/Images/garbage_xxx.jpg\n\ncontent-type:...\n...\npayloadHash"
  │
  ├─ 步骤3：构造待签名字符串 (String to Sign)
  │    "AWS4-HMAC-SHA256\n{timestamp}\n{scope}\nSHA256(CanonicalRequest)"
  │
  ├─ 步骤4：派生签名密钥 (Signing Key)
  │    kDate = HMAC("AWS4"+SecretKey, Date)
  │    kRegion = HMAC(kDate, Region)
  │    kService = HMAC(kRegion, "s3")
  │    kSigning = HMAC(kService, "aws4_request")
  │
  └─ 步骤5：生成签名
       signature = Hex(HMAC(kSigning, StringToSign))
```

### 3.2 签名密钥派生（关键实现）

这是整个 OBS 上传中最容易出错的部分。核心在于**每一步 HMAC 的输出是 32 字节的二进制数据，必须原样作为下一步的密钥**，而非转成十六进制字符串。

```cpp
void hmacSHA256Binary(const uint8_t* key, size_t keyLen,
                      const String& data, uint8_t* result) {
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    mbedtls_md_hmac_starts(&ctx, key, keyLen);          // 二进制密钥
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)data.c_str(), data.length());
    mbedtls_md_hmac_finish(&ctx, result);               // 二进制结果
    mbedtls_md_free(&ctx);
}
```

**常见错误**：将上一步的 HMAC 结果转为十六进制字符串后传递给下一步。正确的做法是直接传递二进制 `uint8_t[32]`。

```cpp
// 签名密钥派生（上传时使用）
uint8_t kDate[32], kRegion[32], kService[32], kSigning[32];
String kSecret = "AWS4" + String(OBS_SECRET_KEY);

hmacSHA256Binary((const uint8_t*)kSecret.c_str(), kSecret.length(), date, kDate);
hmacSHA256Binary(kDate, 32, OBS_REGION, kRegion);    // 注意：key是二进制32字节
hmacSHA256Binary(kRegion, 32, "s3", kService);
hmacSHA256Binary(kService, 32, "aws4_request", kSigning);

// 最终的签名使用二进制密钥计算
uint8_t signatureBin[32];
hmacSHA256Binary(kSigning, 32, stringToSign, signatureBin);

// 转为小写十六进制字符串
String signature = "";
for (int i = 0; i < 32; i++) {
    if (signatureBin[i] < 16) signature += "0";
    signature += String(signatureBin[i], HEX);
}
```

### 3.3 规范请求构造

```cpp
String canonicalRequest = method + "\n" +          // PUT
                          urlEncode(uri) + "\n" +  // /Images/garbage_xxx.jpg
                          queryString + "\n" +     // 空
                          canonicalHeaders + "\n" + // content-type:...\nhost:...\n...
                          signedHeaders + "\n" +    // content-type;host;...
                          payloadHash;              // SHA256(imageData)
```

### 3.4 待签名字符串

```cpp
String credentialScope = date + "/" + OBS_REGION + "/s3/aws4_request";
// 例如："20260418/cn-east-3/s3/aws4_request"

String stringToSign = "AWS4-HMAC-SHA256\n" +
                      amzDate + "\n" +           // 20260418T120000Z
                      credentialScope + "\n" +
                      sha256Hash(canonicalRequest);  // SHA256(规范请求)
```

---

## 4. 虚拟主机风格 URL

华为云 OBS 要求使用虚拟主机风格（Virtual Hosted-Style）URL：

**虚拟主机风格（正确）**：
```
{bucket}.obs.{region}.myhuaweicloud.com/Images/garbage_xxx.jpg
```

**路径风格（错误，OBS 会返回 403）**：
```
obs.{region}.myhuaweicloud.com/{bucket}/Images/garbage_xxx.jpg
```

代码中的实现：

```cpp
// 构造 Host 头
String virtualHost = String(OBS_BUCKET_NAME) + "." + String(OBS_ENDPOINT);
// 例如：my-bucket.obs.cn-east-3.myhuaweicloud.com

// HTTP 请求
String request = "PUT " + uri + " HTTP/1.1\r\n" +
                 "Host: " + virtualHost + "\r\n" +
                 ...
```

---

## 5. 上传实现

### 5.1 uploadToOBS()

```cpp
bool uploadToOBS(uint8_t* imageData, size_t dataSize,
                 char* objectName, char* objectUrl) {
    // ===== 1. 生成对象名称 =====
    String generatedName = buildObjectName();
    // 结果："Images/garbage_<millis>.jpg"
    generatedName.toCharArray(objectName, 64);

    // ===== 2. 获取时间戳 =====
    String timestamp = getISO8601Time();
    // 格式：20260418T120000Z（若NTP未同步则用固定值回退）

    // ===== 3. 计算 AWS SigV4 签名 =====
    // ... (见上文签名流程)

    // ===== 4. 建立 HTTPS 连接并发送 =====
    WiFiClientSecure client;
    client.setInsecure();  // 跳过证书验证
    client.connect(virtualHost.c_str(), 443);

    // 发送 HTTP 头 + 图片数据
    client.print(request);
    client.write(imageData, dataSize);

    // ===== 5. 等待并解析响应 =====
    unsigned long startTime = millis();
    while (!client.available() && millis() - startTime < 10000) {
        delay(100);  // 最多等10秒
    }

    String response = client.readString();

    // ===== 6. 检查状态码 =====
    if (response.indexOf("200 OK") != -1 || response.indexOf("204 No Content") != -1) {
        generateObjectUrl(objectName, objectUrl, 256);
        return true;
    }
    return false;
}
```

### 5.2 HTTPS 与证书

```cpp
client.setInsecure();  // 跳过 SSL 证书验证
```

这是一个务实的简化。在 ESP32 上验证完整的 SSL 证书链需要额外配置根证书，且会增加内存占用。对于课程演示环境，跳过验证是可以接受的权衡。

---

## 6. 预签名 URL 生成

与上传不同，预签名 URL 使用 **AWS Signature V2 风格**（HMAC-SHA1），这是 OBS 兼容的另一种认证方式：

```cpp
bool generatePresignedGetUrl(const char* objectName, char* url,
                              size_t urlSize, uint32_t expiresSeconds) {
    // 1. 获取当前 Unix 时间
    time_t now = getCurrentUnixTime();

    // 2. 计算过期时间
    unsigned long expiresAt = now + expiresSeconds;

    // 3. 构造待签名字符串
    String stringToSign = "GET\n\n\n" + String(expiresAt) + "\n" +
                          "/{bucket}/{object}?response-content-disposition=inline&...";

    // 4. HMAC-SHA1 签名 → Base64
    String signature = hmacSHA1Base64(OBS_SECRET_KEY, stringToSign);

    // 5. 构造完整预签名 URL
    String presignedUrl = "https://{bucket}.{endpoint}/{object}?"
                          "AccessKeyId=xxx&Expires=xxx&Signature=xxx";

    presignedUrl.toCharArray(url, urlSize);
    return true;
}
```

---

## 7. 加密算法工具函数

模块实现了完整的加密工具集，使用 mbedtls 库（ESP32 内置）：

| 函数 | 用途 | 输出格式 |
|------|------|---------|
| `sha256Hash(String)` | 字符串 SHA256 | Hex 字符串 |
| `sha256HashBinary(uint8_t*, size_t)` | 二进制数据 SHA256 | Hex 字符串 |
| `hmacSHA256(String key, String data)` | HMAC-SHA256 | Hex 字符串 |
| `hmacSHA256Binary(uint8_t* key, size_t, String data, uint8_t* result)` | HMAC-SHA256 二进制版 | 原始 32 字节 |
| `hmacSHA1Base64(String key, String data)` | HMAC-SHA1 | Base64 字符串 |
| `urlEncode(String)` | URL 编码 | 编码后字符串 |
| `getISO8601Time()` | 获取 ISO8601 时间 | `20260418T120000Z` |

---

## 8. 时间管理

OBS 签名对时间敏感（偏差不超过 15 分钟）：

```cpp
String getISO8601Time() {
    time_t now = time(nullptr);
    if (now < 1000000000) {  // 2001-09-09 之后的时间戳才 > 10亿
        // NTP 未同步，回退到固定时间
        return "20260418T120000Z";
    }

    struct tm* timeinfo = gmtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%SZ", timeinfo);
    return String(buffer);
}
```

时间戳校验 `now < 1000000000` 用于判断 NTP 是否已同步（未同步时 `time()` 返回接近 0 的值）。

---

## 9. 与其他模块的接口

| 函数 | 调用者 | 用途 |
|------|--------|------|
| `initOBS()` | main.ino setup() | 验证 WiFi 状态 |
| `uploadToOBS()` | main.ino STATE_UPLOAD | 上传图片 |
| `generatePresignedGetUrl()` | main.ino STATE_UPLOAD | 生成 AI 用的临时 URL |
| `generateObjectUrl()` | uploadToOBS() 内部 | 构造对象 URL |

---

## 10. 调试历程中的关键问题

此模块经历了最多的调试迭代（7 个修复记录在 `docs/archive/fixes/obs_fixes/`）：

1. **RequestTimeTooSkewed** — NTP 时间未同步，修复：WiFi 连接后立即同步 NTP
2. **SignatureDoesNotMatch** — 签名密钥派生错误（将二进制 key 当字符串用），修复：使用 `hmacSHA256Binary`
3. **InvalidBucketName** — 路径风格 URL 不被 OBS 支持，修复：切换到虚拟主机风格
4. **VirtualHostDomainRequired** — OBS 强制要求虚拟主机风格，确认方案正确
5. **Region 不匹配** — `OBS_REGION` 配置错误，修复：改为 `cn-east-3`
