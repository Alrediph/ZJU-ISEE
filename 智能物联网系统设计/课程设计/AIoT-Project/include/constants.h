#ifndef CONSTANTS_H
#define CONSTANTS_H

// ==================== 状态机状态 ====================
enum SystemState {
    STATE_IDLE,
    STATE_CAPTURE,
    STATE_UPLOAD,     // 上传图片到OBS + 生成预签名URL
    STATE_RECOGNIZE,  // 使用预签名URL调用AI识别
    STATE_EXECUTE,
    STATE_ERROR
};

// ==================== 垃圾分类类型 ====================
enum GarbageType {
    GARBAGE_UNKNOWN = 0,
    GARBAGE_RECYCLABLE = 1,  // 可回收垃圾
    GARBAGE_OTHER = 2        // 其他垃圾
};

// ==================== 错误代码 ====================
enum ErrorCode {
    ERROR_NONE = 0,
    ERROR_CAMERA_INIT,
    ERROR_CAMERA_CAPTURE,
    ERROR_SERVO_INIT,
    ERROR_WIFI_CONNECT,
    ERROR_MQTT_CONNECT,
    ERROR_OBS_UPLOAD,
    ERROR_OBS_AUTH,
    ERROR_AI_REQUEST,
    ERROR_AI_TIMEOUT,
    ERROR_AI_RESPONSE,
    ERROR_INVALID_RESULT
};

// ==================== LED颜色 ====================
enum LEDColor {
    LED_OFF = 0,
    LED_RED,
    LED_GREEN,
    LED_BLUE,
    LED_YELLOW,
    LED_PURPLE,
    LED_WHITE
};

// ==================== 调试级别 ====================
#define DEBUG_LEVEL_DEBUG 0
#define DEBUG_LEVEL_INFO  1
#define DEBUG_LEVEL_WARN  2
#define DEBUG_LEVEL_ERROR 3

// 当前调试级别
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_INFO

// 调试宏（带时间戳和模块标识）
#define DEBUG_PREFIX "[DEBUG] "
#define INFO_PREFIX  "[INFO]  "
#define WARN_PREFIX  "[WARN]  "
#define ERROR_PREFIX "[ERROR] "

// 获取时间戳的辅助函数（声明）
String getTimestamp();

#if CURRENT_DEBUG_LEVEL == DEBUG_LEVEL_DEBUG
    #define DEBUG_PRINT(x) Serial.print(DEBUG_PREFIX), Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.print(DEBUG_PREFIX), Serial.println(x)
    #define DEBUG_PRINTF(fmt, ...) Serial.printf("%s" fmt, DEBUG_PREFIX, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(fmt, ...)
#endif

#define INFO_PRINT(x) Serial.print(INFO_PREFIX), Serial.print(x)
#define INFO_PRINTLN(x) Serial.print(INFO_PREFIX), Serial.println(x)
#define INFO_PRINTF(fmt, ...) Serial.printf("%s" fmt, INFO_PREFIX, ##__VA_ARGS__)

#define WARN_PRINT(x) Serial.print(WARN_PREFIX), Serial.print(x)
#define WARN_PRINTLN(x) Serial.print(WARN_PREFIX), Serial.println(x)
#define WARN_PRINTF(fmt, ...) Serial.printf("%s" fmt, WARN_PREFIX, ##__VA_ARGS__)

#define ERROR_PRINT(x) Serial.print(ERROR_PREFIX), Serial.print(x)
#define ERROR_PRINTLN(x) Serial.print(ERROR_PREFIX), Serial.println(x)
#define ERROR_PRINTF(fmt, ...) Serial.printf("%s" fmt, ERROR_PREFIX, ##__VA_ARGS__)

// ==================== IoTDA消息格式定义 ====================
// 服务ID（根据云端产品模型配置）
#define SERVICE_ID             "\"GarbageClassification\""

// MQTT消息格式模板
#define MQTT_BODY_FORMAT       "{\"services\":[{\"service_id\":" SERVICE_ID ",\"properties\":{%s}}]}"

// 命令响应数据
#define COMMAND_RESPONSE_DATA  "{\"result_code\": 0,\"response_name\": \"COMMAND_RESPONSE\",\"paras\": {\"result\": \"success\"}}"

// ==================== 常量字符串 ====================
const char* const STATE_NAMES[] = {
    "IDLE",
    "CAPTURE",
    "UPLOAD",     // 上传图片到OBS + 生成预签名URL
    "RECOGNIZE",  // 使用预签名URL调用AI识别
    "EXECUTE",
    "ERROR"
};

const char* const GARBAGE_NAMES[] = {
    "Unknown",
    "Recyclable",
    "Other"
};

const char* const ERROR_MESSAGES[] = {
    "No Error",
    "Camera Init Failed",
    "Camera Capture Failed",
    "Servo Init Failed",
    "WiFi Connect Failed",
    "MQTT Connect Failed",
    "OBS Upload Failed",
    "OBS Auth Failed",
    "AI Request Failed",
    "AI Request Timeout",
    "AI Response Error",
    "Invalid Result"
};

#endif // CONSTANTS_H
