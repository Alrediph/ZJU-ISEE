/**
 * Minimal IoTDA MQTT Connection Test
 *
 * 目的：以最小系统原则排查 MQTT 连接问题
 * - 所有代码和配置在一个文件中
 * - 参考已验证的 HuaweiCloud_ESP32.ino
 * - 逐步添加功能，定位问题所在
 *
 * 注意：MQTT_MAX_PACKET_SIZE 和 MQTT_KEEPALIVE 已在 platformio.ini 中全局定义
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ==================== 配置区域 ====================

// WiFi 配置
const char* WIFI_SSID = "Xiaomi-3m9a";
const char* WIFI_PASSWORD = "zju-6-6015";

// 华为云 IoTDA 配置
const char* DEVICE_ID = "69e6502fe094d6159234626c_ESP32-S3-CAM";
const char* MQTT_SERVER = "015487493c.st1.iotda-device.cn-east-3.myhuaweicloud.com";
const int MQTT_PORT = 1883;
const char* CLIENT_ID = "69e6502fe094d6159234626c_ESP32-S3-CAM_0_0_2026042109";
const char* MQTT_USER = "69e6502fe094d6159234626c_ESP32-S3-CAM";
const char* MQTT_PASSWORD = "cfa724c58730c9edf7576a5ebd9e9ac82d71b044989207a0af939cfbb15ac504";

// MQTT 主题
String MQTT_TOPIC_REPORT = "$oc/devices/" + String(DEVICE_ID) + "/sys/properties/report";
String MQTT_TOPIC_COMMANDS = "$oc/devices/" + String(DEVICE_ID) + "/sys/commands/";

// ==================== 全局对象 ====================

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ==================== 测试状态 ====================

bool wifiConnected = false;
bool mqttConnected = false;
unsigned long lastTestTime = 0;
int testCount = 0;

// ==================== 函数声明 ====================

bool connectWiFi();
bool connectMQTT();
void printMQTTState();
void mqttCallback(char* topic, byte* payload, unsigned int length);

// ==================== Setup ====================

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(3000);  // 等待串口监视器打开

    Serial.println();
    Serial.println("========================================");
    Serial.println("Minimal IoTDA MQTT Connection Test");
    Serial.println("========================================");
    Serial.println();

    // 打印配置信息
    Serial.println("[CONFIG] Configuration:");
    Serial.println("----------------------------------------");
    Serial.printf("  WiFi SSID:     %s\n", WIFI_SSID);
    Serial.printf("  MQTT Server:   %s:%d\n", MQTT_SERVER, MQTT_PORT);
    Serial.printf("  Device ID:     %s\n", DEVICE_ID);
    Serial.printf("  Client ID:     %s\n", CLIENT_ID);
    Serial.printf("  MQTT User:     %s\n", MQTT_USER);
    Serial.printf("  MQTT Password: %s\n", MQTT_PASSWORD);
    Serial.println();

    // Step 1: 连接 WiFi
    Serial.println("[STEP 1] Connect WiFi");
    Serial.println("----------------------------------------");
    wifiConnected = connectWiFi();

    if (!wifiConnected) {
        Serial.println("[ERROR] WiFi connection failed!");
        Serial.println("[INFO] Test aborted. Check WiFi configuration.");
        return;
    }

    Serial.println("[OK] WiFi connected successfully");
    Serial.printf("[INFO] IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[INFO] Signal Strength: %d dBm\n", WiFi.RSSI());
    Serial.println();

    // Step 2: 配置 MQTT 客户端
    Serial.println("[STEP 2] Configure MQTT Client");
    Serial.println("----------------------------------------");

    // 设置 MQTT 服务器和回调
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    Serial.println("[OK] MQTT client configured");
    Serial.println();

    // Step 3: 连接 MQTT
    Serial.println("[STEP 3] Connect MQTT Server");
    Serial.println("----------------------------------------");
    mqttConnected = connectMQTT();

    if (!mqttConnected) {
        Serial.println("[ERROR] MQTT connection failed!");
        Serial.println("[INFO] Check MQTT configuration and network connectivity.");
        return;
    }

    Serial.println("[OK] MQTT connected successfully");
    Serial.println();

    // Step 4: 订阅主题
    Serial.println("[STEP 4] Subscribe to Topics");
    Serial.println("----------------------------------------");

    if (mqttClient.subscribe(MQTT_TOPIC_COMMANDS.c_str())) {
        Serial.printf("[OK] Subscribed to: %s\n", MQTT_TOPIC_COMMANDS.c_str());
    } else {
        Serial.println("[WARN] Subscription failed (but connection is OK)");
    }
    Serial.println();

    // 测试完成
    Serial.println("========================================");
    Serial.println("[TEST] Initialization Complete");
    Serial.println("========================================");
    Serial.println();
    Serial.println("[INFO] Entering main loop...");
    Serial.println("[INFO] Will publish test message every 10 seconds");
    Serial.println();
}

// ==================== Loop ====================

void loop() {
    // 检查 WiFi 连接
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WARN] WiFi disconnected! Reconnecting...");
        wifiConnected = connectWiFi();
        if (!wifiConnected) {
            delay(5000);
            return;
        }
    }

    // 检查 MQTT 连接
    if (!mqttClient.connected()) {
        Serial.println("[WARN] MQTT disconnected! Reconnecting...");
        mqttConnected = connectMQTT();
        if (!mqttConnected) {
            delay(5000);
            return;
        }

        // 重新订阅
        mqttClient.subscribe(MQTT_TOPIC_COMMANDS.c_str());
    }

    // 处理 MQTT 消息
    mqttClient.loop();

    // 定时发布测试消息
    unsigned long currentTime = millis();
    if (currentTime - lastTestTime > 10000) {  // 每 10 秒
        lastTestTime = currentTime;
        testCount++;

        Serial.println();
        Serial.println("[TEST] Publishing Test Message");
        Serial.println("----------------------------------------");

        // 构建测试消息
        String testMessage = "{\"services\":[{\"service_id\":\"GarbageClassification\",\"properties\":{\"Status\":\"Test_"
                            + String(testCount) + "\"}}]}";

        Serial.printf("[INFO] Test #%d\n", testCount);
        Serial.printf("[INFO] Topic:   %s\n", MQTT_TOPIC_REPORT.c_str());
        Serial.printf("[INFO] Message: %s\n", testMessage.c_str());

        // 发布消息
        if (mqttClient.publish(MQTT_TOPIC_REPORT.c_str(), testMessage.c_str())) {
            Serial.println("[OK] Message published successfully");
        } else {
            Serial.println("[ERROR] Message publish failed");
        }
    }
}

// ==================== WiFi 连接 ====================

bool connectWiFi() {
    Serial.println("[INFO] Connecting to WiFi...");

    // 设置 WiFi 模式
    WiFi.mode(WIFI_STA);
    delay(100);

    // 开始连接
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // 等待连接
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {  // 最多等待 10 秒
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    // 检查连接状态
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    } else {
        Serial.printf("[ERROR] WiFi status: %d\n", WiFi.status());
        return false;
    }
}

// ==================== MQTT 连接 ====================

bool connectMQTT() {
    Serial.println("[INFO] Connecting to MQTT server...");
    Serial.printf("[INFO] Server: %s:%d\n", MQTT_SERVER, MQTT_PORT);
    Serial.printf("[INFO] Client ID: %s\n", CLIENT_ID);
    Serial.printf("[INFO] Username: %s\n", MQTT_USER);
    Serial.printf("[INFO] Password: %s\n", MQTT_PASSWORD);

    // 尝试连接
    Serial.println("[INFO] Calling mqttClient.connect()...");

    bool result = mqttClient.connect(CLIENT_ID, MQTT_USER, MQTT_PASSWORD);

    if (result) {
        Serial.println("[OK] MQTT connection established");
        return true;
    } else {
        int state = mqttClient.state();
        Serial.printf("[ERROR] MQTT connection failed, state: %d\n", state);
        printMQTTState();
        return false;
    }
}

// ==================== 打印 MQTT 状态 ====================

void printMQTTState() {
    int state = mqttClient.state();

    Serial.print("[INFO] MQTT State: ");

    switch (state) {
        case -4:
            Serial.println("MQTT_CONNECTION_TIMEOUT (-4)");
            Serial.println("       Meaning: Server didn't respond within timeout");
            break;
        case -3:
            Serial.println("MQTT_CONNECTION_LOST (-3)");
            Serial.println("       Meaning: Connection was lost");
            break;
        case -2:
            Serial.println("MQTT_CONNECT_FAILED (-2)");
            Serial.println("       Meaning: Network connection failed");
            break;
        case -1:
            Serial.println("MQTT_DISCONNECTED (-1)");
            Serial.println("       Meaning: Client is disconnected");
            break;
        case 0:
            Serial.println("MQTT_CONNECTED (0)");
            Serial.println("       Meaning: Client is connected");
            break;
        case 1:
            Serial.println("MQTT_CONNECT_BAD_PROTOCOL (1)");
            Serial.println("       Meaning: Server doesn't support requested protocol");
            break;
        case 2:
            Serial.println("MQTT_CONNECT_BAD_CLIENT_ID (2)");
            Serial.println("       Meaning: Client ID rejected");
            break;
        case 3:
            Serial.println("MQTT_CONNECT_UNAVAILABLE (3)");
            Serial.println("       Meaning: Server unavailable");
            break;
        case 4:
            Serial.println("MQTT_CONNECT_BAD_CREDENTIALS (4)");
            Serial.println("       Meaning: Bad username or password");
            break;
        case 5:
            Serial.println("MQTT_CONNECT_UNAUTHORIZED (5)");
            Serial.println("       Meaning: Not authorized");
            break;
        default:
            Serial.printf("UNKNOWN (%d)\n", state);
            break;
    }
}

// ==================== MQTT 回调 ====================

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.println();
    Serial.println("[MQTT] Message Received");
    Serial.println("----------------------------------------");
    Serial.printf("  Topic:   %s\n", topic);

    // 打印消息内容
    Serial.print("  Payload: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println();
}
