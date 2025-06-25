#include "WiFiTransmitter.h"

WiFiTransmitter* WiFiTransmitter::instance = nullptr;

WiFiTransmitter::WiFiTransmitter() : server(80), isConnected(false) {}

WiFiTransmitter* WiFiTransmitter::getInstance() {
    if (instance == nullptr) {
        instance = new WiFiTransmitter();
    }
    return instance;
}

void WiFiTransmitter::begin(const char* ssid, const char* password) {
    SAFE_ESP_LOGI("WiFiTransmitter", "Disconnecting and setting WiFi mode to STA...");
    WiFi.disconnect(true);         // 断开已有连接并清除配置
    delay(100);
    WiFi.mode(WIFI_STA);

    // 扫描附近网络
    SAFE_ESP_LOGI("WiFiTransmitter", "Scanning nearby WiFi networks...");
    int n = WiFi.scanNetworks();
    if (n == 0) {
        SAFE_ESP_LOGW("WiFiTransmitter", "No WiFi networks found.");
    } else {
        for (int i = 0; i < n; ++i) {
            SAFE_ESP_LOGI("WiFiTransmitter", "[%d] SSID: %s, RSSI: %d dBm, Channel: %d",
                          i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
        }
    }

    // 开始连接
    SAFE_ESP_LOGI("WiFiTransmitter", "Attempting to connect to SSID: %s", ssid);
    WiFi.begin(ssid, password);

    const unsigned long timeout = 15000;  // 15秒超时
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
        delay(500);
        SAFE_ESP_LOGV("WiFiTransmitter", "Connecting...");
    }

    if (WiFi.status() == WL_CONNECTED) {
        isConnected = true;
        SAFE_ESP_LOGI("WiFiTransmitter", "WiFi connected! IP address: %s", WiFi.localIP().toString().c_str());
        setupWebServer();
    } else {
        isConnected = false;
        SAFE_ESP_LOGE("WiFiTransmitter", "WiFi connection failed. Will retry later...");
        // 可选：设置一个重连标志，在 loop() 中周期性调用 begin() 进行重连
    }
}

void WiFiTransmitter::setupWebServer() {
    server.on("/api/send", HTTP_POST, [](AsyncWebServerRequest *request){}, nullptr,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            handleIncomingWiFiData(request, data, len);
        });

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
        String response = "{\"wifi_connected\":true}";
        request->send(200, "application/json", response);
        SAFE_ESP_LOGV("WiFiTransmitter", "Status request served");
    });

    server.begin();
    SAFE_ESP_LOGI("WiFiTransmitter", "Web server started on port 80");
}

void WiFiTransmitter::transmitLoRaData(const String& data) {
    if (!isConnected) {
        SAFE_ESP_LOGW("WiFiTransmitter", "WiFi not connected, cannot transmit");
        return;
    }
    // 这里你可以实现LoRa转发逻辑
    SAFE_ESP_LOGI("WiFiTransmitter", "Transmitting LoRa data: %s", data.c_str());
}

void WiFiTransmitter::handleIncomingWiFiData(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
    String payload = String((char*)data).substring(0, len);
    SAFE_ESP_LOGV("WiFiTransmitter", "Received data: %s", payload.c_str());
    request->send(200, "application/json", "{\"status\":\"received\"}");
}

bool WiFiTransmitter::isWiFiConnected() {
    return isConnected && WiFi.status() == WL_CONNECTED;
}
