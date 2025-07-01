#include "WiFiTransmitter.h"

#define SERVER_URL "http://192.168.43.100:8080/api/receive"  // 修改为你的电脑地址和端口

WiFiTransmitter& WiFiTransmitter::getInstance() {
    static WiFiTransmitter instance;
    return instance;
}

WiFiTransmitter::WiFiTransmitter() : isConnected(false) {
    // 构造时初始化状态
}

void WiFiTransmitter::begin(const char* ssid, const char* password) {
    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);
    const unsigned long timeout = 10000;  // 10秒超时
    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
        delay(500);
        // 可加日志输出：等待连接
    }

    if (WiFi.status() == WL_CONNECTED) {
        isConnected = true;
        // 可加日志输出连接成功和IP地址
    } else {
        isConnected = false;
        // 可加日志输出连接失败
    }
}

bool WiFiTransmitter::isWiFiConnected() {
    return isConnected && WiFi.status() == WL_CONNECTED;
}

bool WiFiTransmitter::sendPacketToServer(uint8_t* data, size_t len) {
    if (!isWiFiConnected()) {
        SAFE_ESP_LOGW("sendPacketToServer", "WiFi not connected, data cannot be sent!");
        return false;
    }

    static WiFiUDP udp;

    // 每次调用 beginPacket -> write -> endPacket 是完整的一次发送
    if (!udp.beginPacket(UDP_SERVER_IP, UDP_SERVER_PORT)) {
        SAFE_ESP_LOGE("sendPacketToServer", "beginPacket failed");
        return false;
    }

    udp.write(data, len);  // 写入整个 Packet<T> 的内容
    if (!udp.endPacket()) {
        SAFE_ESP_LOGE("sendPacketToServer", "endPacket failed");
        return false;
    }

    SAFE_ESP_LOGI("sendPacketToServer", "Sent UDP packet (%d bytes)", len);
    return true;
}


