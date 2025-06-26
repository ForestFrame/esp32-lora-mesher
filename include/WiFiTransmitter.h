#pragma once

#include <WiFi.h>
#include <WiFiUdp.h>
#include "LogManager.h"

class WiFiTransmitter {
private:
    bool isConnected;

    // 私有构造函数，禁止外部实例化
    WiFiTransmitter();

public:
    // 返回单例实例的引用
    static WiFiTransmitter& getInstance();

    // 禁止拷贝和赋值
    WiFiTransmitter(const WiFiTransmitter&) = delete;
    WiFiTransmitter& operator=(const WiFiTransmitter&) = delete;

    void begin(const char* ssid, const char* password);
    bool isWiFiConnected() const;

    // 主动向上位机发送二进制数据，返回是否成功
    bool sendPacketToServer(uint8_t* data, size_t len);
};
