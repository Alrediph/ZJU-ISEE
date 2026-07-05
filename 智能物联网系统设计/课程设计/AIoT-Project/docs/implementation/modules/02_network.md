# 模块二：WiFi 网络模块 (network.cpp)

## 1. 模块定位

`network.cpp` 是系统的**网络基础层**，为所有云服务模块提供 WiFi 连接能力。模块设计遵循**纯网络功能**原则，不依赖任何显示或业务模块，可被测试程序独立使用。

文件位置：`src/network.cpp`（345行） | 头文件：`include/network.h`

---

## 2. 架构设计

### 2.1 功能分层

```
┌─────────────────────────────────────┐
│    业务层（main.ino调用）            │
│    initWiFi() / reconnectWiFi()      │
├─────────────────────────────────────┤
│    高级功能                          │
│    connectToBestWiFi()  多WiFi选择   │
│    setupWiFi()          单WiFi连接    │
│    syncNTPTime()        NTP时间同步  │
├─────────────────────────────────────┤
│    基础功能                          │
│    connectWiFi()        简单连接     │
│    isWiFiConnected()    状态检查     │
│    getWiFiRSSI()        信号强度     │
├─────────────────────────────────────┤
│    重试管理                          │
│    get/reset/update RetryDelay()     │
│    指数退避: 5s→10s→20s→40s→60s    │
└─────────────────────────────────────┘
```

### 2.2 重试延迟机制

```cpp
// 静态变量保持状态
static int wifiRetryDelay = WIFI_RETRY_INIT;  // 初始5秒

int getWiFiRetryDelay()     { return wifiRetryDelay; }
void resetWiFiRetryDelay()  { wifiRetryDelay = WIFI_RETRY_INIT; }
void updateWiFiRetryDelay() { wifiRetryDelay = min(wifiRetryDelay * 2, WIFI_RETRY_MAX); }
```

**指数退避策略**：

| 重试次数 | 延迟 | 说明 |
|---------|------|------|
| 第1次 | 5秒 | WIFI_RETRY_INIT |
| 第2次 | 10秒 | ×2 |
| 第3次 | 20秒 | ×2 |
| 第4次 | 40秒 | ×2 |
| 第5次 | 60秒 | WIFI_RETRY_MAX（封顶） |
| 成功后 | 重置回5秒 | resetWiFiRetryDelay() |

**为何需要指数退避？**
WiFi 断开可能是暂时干扰，也可能路由器重启。频繁重连浪费功耗且无意义，指数退避可以平衡响应速度和资源消耗。

---

## 3. 多 WiFi 智能选择

### 3.1 配置方式

在 `include/secrets.h` 中定义 WiFi 网络列表：

```cpp
// WiFi网络配置结构体
struct WiFiNetwork {
    const char* ssid;
    const char* password;
    int priority;  // 优先级（数字越小优先级越高）
};

const WiFiNetwork wifiNetworks[WIFI_NETWORK_COUNT] = {
    {"Xiaomi-3m9a",   "password1", 1},   // 优先连接
    {"LAPTOP-xxx",    "password2", 2}    // 备用
};
```

在 `include/config.h` 中启用：

```cpp
#define USE_MULTI_WIFI true
```

### 3.2 connectToBestWiFi() 实现逻辑

```cpp
bool connectToBestWiFi() {
    // 步骤1：扫描所有可用网络
    int n = WiFi.scanNetworks();

    // 步骤2：按配置的优先级顺序查找
    for (int i = 0; i < WIFI_NETWORK_COUNT; i++) {
        const char* targetSSID = wifiNetworks[i].ssid;

        // 步骤3：在扫描结果中匹配
        for (int j = 0; j < n; j++) {
            if (strcmp(WiFi.SSID(j).c_str(), targetSSID) == 0) {
                // 步骤4：找到匹配，尝试连接
                WiFi.begin(targetSSID, wifiNetworks[i].password);

                // 步骤5：等待连接（超时10秒）
                int attempts = 0;
                while (WiFi.status() != WL_CONNECTED
                       && attempts < WIFI_CONNECT_TIMEOUT / 500) {
                    delay(500);
                    attempts++;
                }

                // 步骤6：连接成功 → NTP同步 + 重置退避
                if (WiFi.status() == WL_CONNECTED) {
                    syncNTPTime();
                    resetWiFiRetryDelay();
                    return true;
                }
                // 失败 → 断开，尝试下一个
                WiFi.disconnect();
                break;
            }
        }
    }
    return false;  // 所有网络都连接失败
}
```

**设计要点**：
- **优先级扫描**：按配置顺序（`priority` 字段）尝试，而非信号强度
- **串行尝试**：一个网络失败后才尝试下一个，避免反复切换
- **连接成功后自动同步 NTP**：确保后续 OBS 签名的时间准确性

### 3.3 单 WiFi 模式

当 `USE_MULTI_WIFI = false` 时，使用传统的单网络连接：

```cpp
bool setupWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // 等待连接，超时10秒
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED
           && attempts < WIFI_CONNECT_TIMEOUT / 500) {
        delay(500);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        syncNTPTime();
        resetWiFiRetryDelay();
        return true;
    }
    return false;
}
```

---

## 4. NTP 时间同步

### 4.1 为何需要 NTP？

OBS 的 AWS Signature V4 认证要求请求时间与服务器时间偏差不超过 15 分钟。ESP32 无 RTC 电池，上电后时间为 1970 年，必须通过 NTP 同步。

### 4.2 实现

```cpp
bool syncNTPTime() {
    // 配置为 UTC+8（北京时间）
    configTime(NTP_TIMEZONE * 3600, 0, NTP_SERVER1, NTP_SERVER2);

    // 等待同步完成，超时10秒
    struct tm timeinfo;
    int timeSyncAttempts = 0;
    while (!getLocalTime(&timeinfo) && timeSyncAttempts < NTP_SYNC_TIMEOUT) {
        delay(1000);
        timeSyncAttempts++;
    }

    if (getLocalTime(&timeinfo)) {
        return true;
    }
    return false;  // 超时失败，OBS将使用fallback时间
}
```

**NTP 服务器选择**：
- 主服务器 `ntp.aliyun.com`（阿里云 NTP，国内延迟低）
- 备用服务器 `pool.ntp.org`（全球 NTP 池）

---

## 5. 连接超时与状态码

### 5.1 超时控制

```cpp
#define WIFI_CONNECT_TIMEOUT 10000  // 10秒
```

连接等待循环每 500ms 检查一次状态，因此实际尝试次数为 `10000/500 = 20` 次。

### 5.2 WiFi 状态码诊断

```cpp
void printWiFiStatusCode() {
    switch (WiFi.status()) {
        case 0:  // WL_IDLE_STATUS - 正在改变状态
        case 1:  // WL_NO_SSID_AVAIL - 未找到该网络
        case 2:  // WL_SCAN_COMPLETED - 扫描完成
        case 3:  // WL_CONNECTED - 已连接
        case 4:  // WL_CONNECT_FAILED - 连接失败
        case 5:  // WL_CONNECTION_LOST - 连接丢失
        case 6:  // WL_DISCONNECTED - 已断开
    }
}
```

此函数用于调试输出，帮助定位连接失败原因。

---

## 6. 与其他模块的接口

### 6.1 对外暴露的函数

| 函数 | 调用者 | 用途 |
|------|--------|------|
| `initWiFi()` | main.ino setup() | 初始化WiFi（自动选择单/多模式） |
| `checkWiFiConnection()` | main.ino loop() | 检查连接状态 |
| `reconnectWiFi()` | main.ino loop() | 断开后重连 |
| `getWiFiRetryDelay()` / `updateWiFiRetryDelay()` | main.ino loop() | 退避策略管理 |
| `getWiFiRSSI()` / `getWiFiIP()` / `getWiFiMAC()` | display / debug | 信息查询 |

### 6.2 依赖关系

```
network.cpp
  └─ 依赖: WiFi.h (ESP32 Arduino核心库)
  └─ 依赖: time.h (NTP时间)
  └─ 依赖: secrets.h (WiFi配置)
  └─ 依赖: config.h (超时、NTP参数)
  └─ 不依赖: display.h (纯网络功能)
```

模块不依赖 `display.h` 是一个重要的设计决策——在 Phase 9.3 重构中，WiFi 显示逻辑（如 "WiFi Disconnected"）被移到 `main.ino` 的 `checkAndReconnectWiFi()` 中，`network.cpp` 只负责网络操作本身。

---

## 7. 关键设计决策

1. **多WiFi优先级选择而非信号强度选择**：演示环境网络可预期，按配置优先级更可靠
2. **NTP 同步置于 WiFi 连接成功后立即执行**：确保 OBS 签名可用，而非等到实际使用时
3. **指数退避封顶 60 秒**：避免长期断网后重连间隔过长
4. **静态变量 `wifiRetryDelay`**：文件级作用域，避免污染全局命名空间
