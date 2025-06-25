/*
*FileName:    main.cpp
*Description:
*/

/* ----------------------------------------------------------------------------
* include
----------------------------------------------------------------------------- */
#include "main.h"
#include "LoraMesher.h"

/* ----------------------------------------------------------------------------
* defines
----------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
* variables
----------------------------------------------------------------------------- */
/*
* extern variables
*/
DISPLAY_MODEL *u8g2 = nullptr;

/*
* static variables
*/
static LoraMesher &radio = LoraMesher::getInstance();

void setup()
{
    initBoard();
    setupLoraMesher();
    setupWiFi();
}

void loop()
{
}

void initBoard(void)
{
    Serial.begin(115200);
    ESP_LOGI("INIT", "Board inits.");

    Wire.begin(I2C_SDA, I2C_SCL);

    initPMU();

    pinMode(BOARD_LED, OUTPUT);
    digitalWrite(BOARD_LED, LED_ON);

    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0)
    {
        ESP_LOGI("INIT", "Start OLED.");
        u8g2 = new DISPLAY_MODEL(U8G2_R0, U8X8_PIN_NONE);
        u8g2->begin();
        u8g2->clearBuffer();
        u8g2->setFlipMode(0);
        u8g2->setFontMode(1); // Transparent
        u8g2->setDrawColor(1);
        u8g2->setFontDirection(0);
        u8g2->firstPage();
        do
        {
            u8g2->setFont(u8g2_font_inb19_mr);
            u8g2->drawStr(0, 30, "EoRa");
        } while (u8g2->nextPage());
        u8g2->sendBuffer();
        u8g2->setFont(u8g2_font_fur11_tf);
        delay(3000);
    }
}

void setupLoraMesher(void)
{
    // 1. 安装GPIO中断服务
    //	gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    // 2. 初始化SPI
    SPIClass *mySPI = new SPIClass(HSPI);
    mySPI->begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);

    // 3. 配置LoRaMesher
    LoraMesher::LoraMesherConfig config;
    config.spi = mySPI;

    radio.begin(config);
    radio.start();
}

void setupWiFi(void)
{
	// 添加WiFi传输器初始化  
    WiFiTransmitter* wifiTx = WiFiTransmitter::getInstance();  
    wifiTx->begin("XDZY-1", "xidianzy"); 
}

