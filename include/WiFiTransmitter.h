#ifndef WIFI_TRANSMITTER_H
#define WIFI_TRANSMITTER_H

#include <WiFi.h>
#include "LoraMesher.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class WiFiTransmitter {
public:
    static WiFiTransmitter* getInstance();

    void begin(const char* ssid, const char* password);
    void setupWebServer();
    void transmitLoRaData(const String& data);
    void handleIncomingWiFiData(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    bool isWiFiConnected();

private:
    WiFiTransmitter();  // 私有构造函数，防止外部实例化
    static WiFiTransmitter* instance;

    AsyncWebServer server;
    bool isConnected;
};

#endif
