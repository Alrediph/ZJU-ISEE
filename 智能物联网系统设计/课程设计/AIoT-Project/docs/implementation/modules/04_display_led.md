# 模块四：显示与LED模块 (display.cpp + led.cpp)

## 1. 模块定位

这两个模块负责系统的**人机交互输出**，为用户提供视觉状态反馈：
- **display.cpp**：SSD1306 OLED 显示屏（128×64 像素），提供文字信息
- **led.cpp**：WS2812 RGB LED 或外部 GPIO LED，提供颜色状态指示

文件位置：`src/display.cpp`（209行）、`src/led.cpp`（150行）

---

## 2. OLED 显示模块

### 2.1 硬件接口

```cpp
// include/config.h
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_ADDRESS  0x3C      // SSD1306 I2C 默认地址
#define OLED_SDA_PIN  21        // I2C 数据线
#define OLED_SCL_PIN  47        // I2C 时钟线

// src/display.cpp
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
```

**引脚选择说明**：GPIO 21 和 47 是 ESP32-S3 上未被摄像头占用的可用引脚。GPIO 8/9（ESP32 默认 I2C 引脚）已被摄像头占用，因此使用软件 I2C 连接到其他引脚。

### 2.2 初始化

```cpp
bool initOLED() {
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);  // 软件I2C

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        return false;
    }

    display.clearDisplay();
    display.display();
    return true;
}
```

`SSD1306_SWITCHCAPVCC` 表示使用内部电荷泵产生 OLED 驱动电压（大多数模块使用此设置）。

### 2.3 核心显示函数

#### displayMessage() — 三行文本

这是最常用的显示函数，系统几乎所有状态都通过它展示：

```cpp
void displayMessage(const char* line1, const char* line2, const char* line3) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);   // 第1行 (Y=0)
    display.println(line1);
    display.setCursor(0, 16);  // 第2行 (Y=16)
    display.println(line2);
    display.setCursor(0, 32);  // 第3行 (Y=32)
    display.println(line3);

    display.display();
}
```

**布局**：128×64 屏幕中，每行文本高 8 像素（`setTextSize(1)`）。三行文本使用 Y=0/16/32 的间距，留出足够间距。

#### displayNetworkStatus() — 底部状态栏

```cpp
void displayNetworkStatus() {
    display.drawFastHLine(0, 54, SCREEN_WIDTH, SSD1306_WHITE);  // 分割线

    display.setTextSize(1);
    display.setCursor(0, 56);
    // WiFi 状态：左边
    if (WiFi.status() == WL_CONNECTED) {
        display.print("WiFi: OK");
    } else {
        display.print("WiFi: --");
    }

    // MQTT 状态：右边（X=70）
    display.setCursor(70, 56);
    if (mqttClient.connected()) {         // extern 引用 iotda.cpp 中的对象
        display.print("MQTT: OK");
    } else {
        display.print("MQTT: --");
    }
    display.display();
}
```

**设计要点**：
- 通过 `extern PubSubClient mqttClient` 引用 iotda.cpp 中的 MQTT 客户端对象
- 水平分割线将主显示区（Y=0-53）与状态栏（Y=55-63）分开
- 60 秒定时上报时刷新，提供实时连接状态

#### 其他辅助函数

```cpp
void displayTitleContent(title, content);  // 标题+分割线+内容
void displayProgress(title, progress);     // 带进度条的显示
void displayError(errorMsg);              // 错误信息显示
void displayResult(garbageType, conf);    // 识别结果（大字显示类型）
void displayStatus(title, status, ok);    // 统一状态格式
```

---

## 3. LED 控制模块

### 3.1 双模式支持

```cpp
// include/config.h
#define USE_ONBOARD_RGB_LED false  // false=外部LED, true=板载WS2812

// 板载 WS2812 (GPIO 48)
#define RGB_LED_PIN 48

// 外部 GPIO LED
#define LED_RED_PIN    38
#define LED_GREEN_PIN  39
#define LED_BLUE_PIN   40
#define LED_STATE_ON    0   // 低电平点亮（共阳LED）
#define LED_STATE_OFF   1   // 高电平熄灭
```

**模式切换逻辑**：`initLED()` 根据 `USE_ONBOARD_RGB_LED` 选择初始化方式：

```cpp
bool initLED() {
    if (USE_ONBOARD_RGB_LED) {
        // WS2812 模式：通过 FastLED 库驱动
        FastLED.addLeds<WS2812, RGB_LED_PIN, GRB>(leds, 1);
        FastLED.setBrightness(50);  // 50% 亮度
    } else {
        // 外部LED模式：GPIO 数字输出
        pinMode(LED_RED_PIN, OUTPUT);
        pinMode(LED_GREEN_PIN, OUTPUT);
        pinMode(LED_BLUE_PIN, OUTPUT);
    }
    return true;
}
```

### 3.2 颜色映射

```cpp
void setLEDColor(LEDColor color) {
    if (USE_ONBOARD_RGB_LED) {
        switch (color) {
            case LED_OFF:   leds[0] = CRGB::Black;   break;
            case LED_RED:   leds[0] = CRGB::Red;     break;
            case LED_GREEN: leds[0] = CRGB::Green;   break;
            case LED_BLUE:  leds[0] = CRGB::Blue;    break;
            case LED_YELLOW:leds[0] = CRGB::Yellow;  break;
            case LED_PURPLE:leds[0] = CRGB::Purple;  break;
            case LED_WHITE: leds[0] = CRGB::White;   break;
        }
        FastLED.show();  // 立即刷新
    } else {
        // 外部GPIO模式：通过组合 R/G/B 引脚实现颜色
        // 如：YELLOW = RED + GREEN, PURPLE = RED + BLUE
    }
}
```

### 3.3 系统状态颜色约定

| 状态 | LED 颜色 | 含义 |
|------|---------|------|
| 启动中 | 蓝色 | 系统初始化过程 |
| 空闲/成功 | 绿色 | 等待触发或操作完成 |
| 处理中 | 蓝色 | 拍照、上传、识别过程中 |
| 错误 | 红色 | 任何步骤失败 |
| 测试中 | 黄色/紫色 | 测试程序运行中 |

### 3.4 高级效果

**闪烁效果** — 用于测试和提醒：
```cpp
void blinkLED(LEDColor color, int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        setLEDColor(color);
        delay(delayMs);
        setLEDColor(LED_OFF);
        delay(delayMs);
    }
}
```

**呼吸效果** — 仅 WS2812 支持：
```cpp
void breatheLED(LEDColor color, int durationMs) {
    // 渐亮 → 渐暗，通过 nscale8 调整亮度
    int steps = 50;
    for (int i = 0; i <= steps; i++) {
        leds[0] = baseColor;
        leds[0].nscale8(i * 255 / steps);  // 0% → 100%
        FastLED.show();
        delay(stepDelay);
    }
    // 渐暗类似，反向循环
}
```

---

## 4. 设计要点

### 4.1 OLED 信息密度设计

128×64 屏幕空间有限，信息展示遵循以下原则：
- **正常状态**：三行文本（标题 + 状态 + 详情）
- **进度状态**：标题 + 进度条 + 百分比
- **网络状态**：底部固定状态栏，主区域留给业务
- **识别结果**：大字显示类型名，小字显示置信度

### 4.2 WS2812 vs 外部LED的选择

默认使用外部 GPIO LED（`USE_ONBOARD_RGB_LED = false`）：
- 外部 LED 更醒目（3个独立LED可分布于不同位置）
- WS2812 需要 FastLED 库，编译体积更大
- WS2812 支持丰富的颜色和效果，但项目仅需 7 种颜色

### 4.3 模块独立性

- `display.cpp` 依赖 `iotda.cpp` 中的 `mqttClient` 对象（通过 extern）
- `led.cpp` 完全独立，仅依赖 `constants.h` 中的颜色枚举
- 两个模块都不依赖 `main.ino`
