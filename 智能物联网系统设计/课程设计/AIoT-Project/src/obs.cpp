/**
 * 华为云OBS存储模块实现
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include "obs.h"
#include "config.h"
#include "secrets.h"
#include "constants.h"
#include "display.h"
#include "led.h"

// ==================== 辅助函数声明 ====================
String getISO8601Time();
time_t getCurrentUnixTime();
String hmacSHA256(const String& key, const String& data);
void hmacSHA256Binary(const uint8_t* key, size_t keyLen, const String& data, uint8_t* result);
String hmacSHA1Base64(const String& key, const String& data);
String sha256Hash(const String& data);
String sha256HashBinary(const uint8_t* data, size_t length);
String urlEncode(const String& url);
String buildObjectName();
String buildCanonicalizedResource(const char* objectName);

// ==================== 初始化OBS ====================
bool initOBS() {
    INFO_PRINTLN("初始化OBS服务...");

    // 检查WiFi连接
    if (WiFi.status() != WL_CONNECTED) {
        ERROR_PRINTLN("WiFi未连接，无法初始化OBS");
        return false;
    }

    INFO_PRINTLN("OBS初始化成功");
    return true;
}

// ==================== 上传图片到OBS ====================
bool uploadToOBS(uint8_t* imageData, size_t dataSize, char* objectName, char* objectUrl) {
    INFO_PRINTLN("开始上传图片到OBS...");

    // 检查参数
    if (imageData == NULL || dataSize == 0) {
        ERROR_PRINTLN("图片数据无效");
        return false;
    }

    // 生成对象名称：Images/garbage_{timestamp}.jpg
    String generatedObjectName = buildObjectName();
    if (generatedObjectName.length() == 0 || generatedObjectName.length() >= 64) {
        ERROR_PRINTLN("对象名称生成失败或过长");
        return false;
    }
    generatedObjectName.toCharArray(objectName, 64);

    // 获取当前时间（ISO8601格式）
    String timestamp = getISO8601Time();
    if (timestamp.length() == 0) {
        ERROR_PRINTLN("获取时间失败");
        return false;
    }

    INFO_PRINTF("对象名称: %s\n", objectName);
    INFO_PRINTF("时间戳: %s\n", timestamp.c_str());

    // 构造签名字符串
    String date = timestamp.substring(0, 8);  // YYYYMMDD
    String amzDate = timestamp;  // YYYYMMDDTHHMMSSZ

    // HTTP方法
    String method = "PUT";

    // URI（虚拟主机风格：只包含对象名称）
    String uri = "/" + String(objectName);

    // 虚拟主机风格的Host头（桶名称 + endpoint）
    String virtualHost = String(OBS_BUCKET_NAME) + "." + String(OBS_ENDPOINT);

    // 查询字符串（空）
    String queryString = "";

    // 计算payload的SHA256哈希（使用二进制版本）
    String payloadHash = sha256HashBinary(imageData, dataSize);

    // 构造规范请求头（使用虚拟主机风格的Host）
    String canonicalHeaders = String("content-type:image/jpeg\n") +
                              "host:" + virtualHost + "\n" +
                              "x-amz-content-sha256:" + payloadHash + "\n" +
                              "x-amz-date:" + amzDate + "\n";

    String signedHeaders = "content-type;host;x-amz-content-sha256;x-amz-date";

    // 构造规范请求
    String canonicalRequest = method + "\n" +
                              urlEncode(uri) + "\n" +
                              queryString + "\n" +
                              canonicalHeaders + "\n" +
                              signedHeaders + "\n" +
                              payloadHash;

    INFO_PRINTF("规范请求: %s\n", canonicalRequest.c_str());

    // 构造待签名字符串
    String algorithm = "AWS4-HMAC-SHA256";
    String credentialScope = date + "/" + OBS_REGION + "/s3/aws4_request";

    String stringToSign = algorithm + "\n" +
                          amzDate + "\n" +
                          credentialScope + "\n" +
                          sha256Hash(canonicalRequest);

    INFO_PRINTF("待签名字符串: %s\n", stringToSign.c_str());

    // 计算签名密钥（使用二进制HMAC结果）
    uint8_t kDate[32], kRegion[32], kService[32], kSigning[32];
    String kSecret = "AWS4" + String(OBS_SECRET_KEY);

    // 第一步：HMAC(Secret Key, Date)
    hmacSHA256Binary((const uint8_t*)kSecret.c_str(), kSecret.length(), date, kDate);

    // 第二步：HMAC(kDate, Region)
    hmacSHA256Binary(kDate, 32, OBS_REGION, kRegion);

    // 第三步：HMAC(kRegion, Service)
    hmacSHA256Binary(kRegion, 32, "s3", kService);

    // 第四步：HMAC(kService, "aws4_request")
    hmacSHA256Binary(kService, 32, "aws4_request", kSigning);

    // 计算签名（使用二进制密钥）
    uint8_t signatureBin[32];
    hmacSHA256Binary(kSigning, 32, stringToSign, signatureBin);

    // 将签名转换为十六进制字符串
    String signature = "";
    for (int i = 0; i < 32; i++) {
        if (signatureBin[i] < 16) {
            signature += "0";
        }
        signature += String(signatureBin[i], HEX);
    }

    INFO_PRINTF("签名: %s\n", signature.c_str());

    // 构造Authorization头
    String authorization = algorithm + " " +
                          "Credential=" + String(OBS_ACCESS_KEY) + "/" + credentialScope + ", " +
                          "SignedHeaders=" + signedHeaders + ", " +
                          "Signature=" + signature;

    // 创建WiFiClientSecure
    WiFiClientSecure client;
    client.setInsecure();  // 跳过证书验证（简化实现）

    // 连接OBS服务器（使用虚拟主机）
    INFO_PRINTF("连接OBS服务器: %s\n", virtualHost.c_str());
    if (!client.connect(virtualHost.c_str(), 443)) {
        ERROR_PRINTLN("连接OBS服务器失败");
        return false;
    }

    // 构造HTTP PUT请求（使用虚拟主机风格的Host头）
    String request = method + " " + uri + " HTTP/1.1\r\n" +
                    "Host: " + virtualHost + "\r\n" +
                    "Content-Type: image/jpeg\r\n" +
                    "Content-Length: " + String(dataSize) + "\r\n" +
                    "x-amz-date: " + amzDate + "\r\n" +
                    "x-amz-content-sha256: " + payloadHash + "\r\n" +
                    "Authorization: " + authorization + "\r\n" +
                    "\r\n";

    INFO_PRINTLN("发送HTTP请求...");
    INFO_PRINTF("%s\n", request.c_str());

    // 发送请求头
    client.print(request);

    // 发送图片数据
    client.write(imageData, dataSize);

    // 等待响应
    INFO_PRINTLN("等待响应...");
    unsigned long startTime = millis();
    while (!client.available() && millis() - startTime < 10000) {
        delay(100);
    }

    // 读取响应
    String response = client.readString();
    INFO_PRINTF("响应: %s\n", response.c_str());

    // 检查响应状态码
    if (response.indexOf("200 OK") != -1 || response.indexOf("204 No Content") != -1) {
        INFO_PRINTLN("图片上传成功");

        // 生成对象URL
        generateObjectUrl(objectName, objectUrl, 256);
        INFO_PRINTF("对象URL: %s\n", objectUrl);

        client.stop();
        return true;
    } else {
        ERROR_PRINTLN("图片上传失败");
        ERROR_PRINTF("响应: %s\n", response.c_str());
        client.stop();
        return false;
    }
}

// ==================== 生成OBS对象URL ====================
void generateObjectUrl(const char* objectName, char* url, size_t urlSize) {
    // 虚拟主机风格URL
    String objectPath = urlEncode(String(objectName));
    snprintf(url, urlSize, "https://%s.%s/%s", OBS_BUCKET_NAME, OBS_ENDPOINT, objectPath.c_str());
}

bool generatePresignedGetUrl(const char* objectName, char* url, size_t urlSize, uint32_t expiresSeconds) {
    if (objectName == NULL || url == NULL || urlSize == 0 || expiresSeconds == 0) {
        ERROR_PRINTLN("预签名URL参数无效");
        return false;
    }

    time_t now = getCurrentUnixTime();
    if (now <= 0) {
        ERROR_PRINTLN("系统时间无效，无法生成预签名URL");
        return false;
    }

    unsigned long expiresAt = static_cast<unsigned long>(now + expiresSeconds);
    String expires = String(expiresAt);
    String canonicalizedResource = buildCanonicalizedResource(objectName);

    String stringToSign = String("GET\n\n\n") + expires + "\n" + canonicalizedResource;
    String signature = hmacSHA1Base64(OBS_SECRET_KEY, stringToSign);

    if (signature.length() == 0) {
        ERROR_PRINTLN("生成预签名URL签名失败");
        return false;
    }

    String objectPath = urlEncode(String(objectName));
    String query = String("AccessKeyId=") + urlEncode(String(OBS_ACCESS_KEY)) +
                   "&Expires=" + expires +
                   "&response-content-disposition=" + urlEncode("inline") +
                   "&response-content-type=" + urlEncode("image/jpeg") +
                   "&Signature=" + urlEncode(signature);

    String presignedUrl = String("https://") + OBS_BUCKET_NAME + "." + OBS_ENDPOINT + "/" + objectPath + "?" + query;

    if (presignedUrl.length() + 1 > urlSize) {
        ERROR_PRINTF("预签名URL过长: %d > %d\n", presignedUrl.length() + 1, urlSize);
        return false;
    }

    presignedUrl.toCharArray(url, urlSize);
    INFO_PRINTF("预签名URL生成成功，有效期: %lu 秒\n", static_cast<unsigned long>(expiresSeconds));
    return true;
}

// ==================== 辅助函数实现 ====================

/**
 * 获取ISO8601格式时间
 * 注意：需要NTP时间同步，这里简化实现使用编译时时间
 */
String getISO8601Time() {
    // 使用NTP同步的系统时间
    // 若NTP未同步则fallback到固定时间
    time_t now = time(nullptr);
    if (now < 1000000000) {
        // NTP时间未同步，使用固定时间（仅用于测试）
        WARN_PRINTLN("NTP时间未同步，使用固定时间");
        return "20260418T120000Z";
    }

    struct tm* timeinfo = gmtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%SZ", timeinfo);
    return String(buffer);
}

time_t getCurrentUnixTime() {
    time_t now = time(nullptr);
    if (now < 1000000000) {
        WARN_PRINTLN("系统时间未同步");
        return 0;
    }
    return now;
}

String buildObjectName() {
    return String(OBS_IMAGE_PATH) + "garbage_" + String(millis()) + ".jpg";
}

String buildCanonicalizedResource(const char* objectName) {
    String resource = String("/") + OBS_BUCKET_NAME + "/" + objectName;
    resource += "?response-content-disposition=inline&response-content-type=image/jpeg";
    return resource;
}

/**
 * HMAC-SHA256计算
 */
String hmacSHA256(const String& key, const String& data) {
    uint8_t result[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key.c_str(), key.length());
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)data.c_str(), data.length());
    mbedtls_md_hmac_finish(&ctx, result);
    mbedtls_md_free(&ctx);

    // 转换为十六进制字符串
    String hexResult = "";
    for (int i = 0; i < 32; i++) {
        if (result[i] < 16) {
            hexResult += "0";
        }
        hexResult += String(result[i], HEX);
    }

    return hexResult;
}

String hmacSHA1Base64(const String& key, const String& data) {
    uint8_t digest[20];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key.c_str(), key.length());
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)data.c_str(), data.length());
    mbedtls_md_hmac_finish(&ctx, digest);
    mbedtls_md_free(&ctx);

    unsigned char encoded[64];
    size_t encodedLen = 0;
    if (mbedtls_base64_encode(encoded, sizeof(encoded), &encodedLen, digest, sizeof(digest)) != 0) {
        return "";
    }
    encoded[encodedLen] = '\0';

    return String((const char*)encoded).substring(0, encodedLen);
}

/**
 * SHA256哈希计算
 */
String sha256Hash(const String& data) {
    uint8_t result[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char*)data.c_str(), data.length());
    mbedtls_md_finish(&ctx, result);
    mbedtls_md_free(&ctx);

    // 转换为十六进制字符串
    String hexResult = "";
    for (int i = 0; i < 32; i++) {
        if (result[i] < 16) {
            hexResult += "0";
        }
        hexResult += String(result[i], HEX);
    }

    return hexResult;
}

/**
 * URL编码
 */
String urlEncode(const String& url) {
    String encoded = "";
    for (unsigned int i = 0; i < url.length(); i++) {
        char c = url.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/') {
            encoded += c;
        } else {
            encoded += "%";
            if (c < 16) {
                encoded += "0";
            }
            encoded += String((unsigned char)c, HEX);
        }
    }
    return encoded;
}

/**
 * HMAC-SHA256计算（二进制结果版本）
 * 用于签名密钥派生
 */
void hmacSHA256Binary(const uint8_t* key, size_t keyLen, const String& data, uint8_t* result) {
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, key, keyLen);
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)data.c_str(), data.length());
    mbedtls_md_hmac_finish(&ctx, result);
    mbedtls_md_free(&ctx);
}

/**
 * SHA256哈希计算（二进制数据版本）
 * 用于计算payload哈希
 */
String sha256HashBinary(const uint8_t* data, size_t length) {
    uint8_t result[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, data, length);  // 直接使用二进制数据
    mbedtls_md_finish(&ctx, result);
    mbedtls_md_free(&ctx);

    // 转换为十六进制字符串
    String hexResult = "";
    for (int i = 0; i < 32; i++) {
        if (result[i] < 16) {
            hexResult += "0";
        }
        hexResult += String(result[i], HEX);
    }

    return hexResult;
}
