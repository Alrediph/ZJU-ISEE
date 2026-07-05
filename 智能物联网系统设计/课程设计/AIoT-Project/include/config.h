#ifndef CONFIG_H
#define CONFIG_H

// ==================== 摄像头配置 ====================
#define CAMERA_PIXFORMAT PIXFORMAT_JPEG
#define CAMERA_FRAMESIZE FRAMESIZE_VGA  // 640x480
#define CAMERA_QUALITY 10  // JPEG质量 (1-63, 越小质量越高)，10为高质量
#define CAMERA_FB_COUNT 2  // 帧缓冲区数量，提升拍照稳定性

// 摄像头引脚定义（GOOUUU ESP32-S3-CAM）
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     15
#define SIOD_GPIO_NUM      4
#define SIOC_GPIO_NUM      5
#define Y9_GPIO_NUM       16
#define Y8_GPIO_NUM       17
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM       10
#define Y4_GPIO_NUM        8
#define Y3_GPIO_NUM        9
#define Y2_GPIO_NUM       11
#define VSYNC_GPIO_NUM     6
#define HREF_GPIO_NUM      7
#define PCLK_GPIO_NUM     13

// ==================== OLED配置 ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C
// I2C引脚：GPIO 8, 9已被摄像头占用，需要使用其他引脚
// 使用软件I2C，选择可用GPIO
#define OLED_SDA_PIN 21   // I2C SDA (GPIO 1可用)
#define OLED_SCL_PIN 47  // I2C SCL (GPIO 21可用)

// ==================== 舵机配置 ====================
// 根据GOOUUU ESP32-S3-CAM引脚图：
// - GPIO 19, 20用于USB，不可用
// - GPIO 2是板载LED
// - 可用GPIO：1, 21, 45, 47, 48
// 单舵机配置
#define SERVO_PIN             1  // 舵机引脚 (GPIO 1)
#define SERVO_ANGLE_CENTER    90  // 中间位置（初始位置）
#define SERVO_ANGLE_LEFT      45  // 向左摆动角度（其他垃圾）
#define SERVO_ANGLE_RIGHT    135  // 向右摆动角度（可回收垃圾）
#define SERVO_FREQ        50     // PWM频率 50Hz
#define SERVO_RESOLUTION  16     // PWM分辨率 16位

// 舵机脉冲宽度（用于attach函数）
#define SERVO_PULSE_MIN    500    // 0度对应的脉冲宽度(微秒)
#define SERVO_PULSE_MAX   2500    // 180度对应的脉冲宽度(微秒)
#define SERVO_HOLD_TIME 1000    // 舵机保持开启时间(毫秒)

// ==================== LED配置 ====================
// GPIO 48是板载WS2812 RGB LED，可直接使用
// 如果使用外部LED，可用GPIO 1或21
#define USE_ONBOARD_RGB_LED false  // 使用板载RGB LED (GPIO 48)
#define RGB_LED_PIN 48            // WS2812数据引脚
// 如果不使用板载LED，可使用以下引脚连接外部LED
// 注意：GPIO 2也是板载LED，可作为蓝灯或状态指示
#define LED_RED_PIN    38    // 外部红灯 (GPIO 38)
#define LED_GREEN_PIN  39   // 外部绿灯 (GPIO 39)
#define LED_BLUE_PIN   40    // 板载LED (GPIO 40)
#define LED_STATE_ON  0     // LED低电平点亮
#define LED_STATE_OFF 1     // LED高电平熄灭

// ==================== 按钮配置（可选） ====================
#define BUTTON_PIN 0   // BOOT按钮
#define BUTTON_DEBOUNCE 50  // 消抖时间(毫秒)

// ==================== 时序配置 ====================
#define CAPTURE_DELAY     100   // 拍照稳定延时(毫秒)
#define UPLOAD_TIMEOUT    10000 // 上传超时(毫秒)
#define RECOGNIZE_TIMEOUT 15000 // 识别超时(毫秒)

// 定时任务配置
#define STATUS_REPORT_INTERVAL 60000  // 状态上报间隔(毫秒，60秒)

// ==================== 看门狗配置 ====================
#define WDT_TIMEOUT 30  // 看门狗超时时间(秒)

// ==================== 网络配置 ====================
#define WIFI_CONNECT_TIMEOUT 10000  // WiFi连接超时(毫秒)
#define MQTT_RECONNECT_DELAY 5000   // MQTT重连延时(毫秒)

// WiFi重连配置（指数退避策略）
#define WIFI_RETRY_INIT 5000    // WiFi重试初始延迟(毫秒)
#define WIFI_RETRY_MAX 60000    // WiFi重试最大延迟(毫秒)

// WiFi扫描配置（多网络支持）
#define WIFI_SCAN_TIMEOUT 10000  // WiFi扫描超时(毫秒)
#define USE_MULTI_WIFI true     // 是否启用多WiFi网络支持（默认关闭，保持向后兼容）

// NTP时间同步配置
#define NTP_SERVER1 "ntp.aliyun.com"
#define NTP_SERVER2 "pool.ntp.org"
#define NTP_TIMEZONE 8          // UTC+8时区
#define NTP_SYNC_TIMEOUT 10     // NTP同步超时(秒)

// ==================== OBS配置 ====================
#define OBS_UPLOAD_TIMEOUT 30000  // OBS上传超时(毫秒)
#define OBS_IMAGE_PATH "Images/"  // 图片存储路径
#define OBS_RECORD_PATH "Images/records.csv"  // 识别记录CSV文件路径（IoTDA规则引擎转发目标）
#define OBS_PRESIGNED_URL_EXPIRES 600  // 预签名URL有效期(秒)

// ==================== AI识别配置 ====================
#define AI_API_TIMEOUT 20000  // AI API超时(毫秒)

#endif // CONFIG_H
