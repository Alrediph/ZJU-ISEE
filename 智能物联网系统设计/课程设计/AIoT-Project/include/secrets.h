#ifndef SECRETS_H
#define SECRETS_H

// ==================== WiFi配置 ====================
// 单WiFi配置（向后兼容）
#define WIFI_SSID "Xiaomi-3m9a"
#define WIFI_PASSWORD "zju-6-6015"

// 多WiFi网络配置（优先级支持）
struct WiFiNetwork {
    const char* ssid;
    const char* password;
    int priority;  // 优先级，数字越小优先级越高
};

// WiFi网络列表（按优先级排序）
#define WIFI_NETWORK_COUNT 2
const WiFiNetwork wifiNetworks[WIFI_NETWORK_COUNT] = {
    {"Xiaomi-3m9a", "zju-6-6015", 1},        // 优先级1（最高）
    {"LAPTOP-PLD5E9BR 6539", "TianXuan", 2}       // 优先级2（备用）
};

// ==================== 华为云IoTDA配置 ====================
#define DEVICE_ID "69e6502fe094d6159234626c_ESP32-S3-CAM"  // 设备ID

// MQTT连接参数（手动配置）
#define MQTT_SERVER   "015487493c.st1.iotda-device.cn-east-3.myhuaweicloud.com"
#define MQTT_PORT     1883
#define CLIENT_ID     "69e6502fe094d6159234626c_ESP32-S3-CAM_0_0_2026042109"
#define MQTT_USER     "69e6502fe094d6159234626c_ESP32-S3-CAM"
#define MQTT_PASSWORD "cfa724c58730c9edf7576a5ebd9e9ac82d71b044989207a0af939cfbb15ac504"

// MQTT主题定义（使用简单路径，参考HuaweiCloud_ESP32.ino）
#define MQTT_TOPIC_REPORT       "$oc/devices/" DEVICE_ID "/sys/properties/report"
#define MQTT_TOPIC_COMMANDS     "$oc/devices/" DEVICE_ID "/sys/commands/"
#define MQTT_TOPIC_CMD_RESPONSE "$oc/devices/" DEVICE_ID "/sys/commands/response/request_id="

// ==================== 华为云OBS配置 ====================
#define OBS_ACCESS_KEY "HPUAW2GYOGAJ2OGCYLLP"
#define OBS_SECRET_KEY "aFEoJpm8QvYlXOfi6w3WXCLAoIoZav2ClfVIjiwE"
#define OBS_ENDPOINT "obs.cn-east-3.myhuaweicloud.com"
#define OBS_BUCKET_NAME "iotda-obs-data"
#define OBS_REGION "cn-east-3"  // OBS所在区域（与endpoint一致）

// ==================== AI识别API配置 ====================
#define AI_API_KEY "sk-70b03c1264924ccb808c184bb4a0c018"
#define AI_API_ENDPOINT "https://dashscope.aliyuncs.com/compatible-mode"
#define AI_MODEL "qwen3.5-flash"

#endif // SECRETS_H
