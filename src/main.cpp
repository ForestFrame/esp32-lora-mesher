/*
 *FileName:    main.cpp
 *Description:
 */

/* ----------------------------------------------------------------------------
 * include
----------------------------------------------------------------------------- */
#include "main.h"
#include <stdio.h>
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
static LoraMesher& radio = LoraMesher::getInstance();

void setup()
{
    initBoard();
}

void loop()
{
    // put your main code here, to run repeatedly:
}

void initBoard(void)
{
    Serial.begin(115200);
    LOGI("Board inits.");

    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
    Wire.begin(I2C_SDA, I2C_SCL);

    initPMU();

    pinMode(BOARD_LED, OUTPUT);
    digitalWrite(BOARD_LED, LED_ON);

    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0)
    {
        LOGI("Start OLED.");
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

    if (u8g2)
    {
        u8g2->clearBuffer();
        u8g2->firstPage();
        do
        {
            u8g2->setCursor(0, 16);
            u8g2->println("Waiting to receive data.");
        } while (u8g2->nextPage());
    }
}

void setupLoraMesher() {
    // Example on how to change the module. See LoraMesherConfig to see all the configurable parameters.
    LoraMesher::LoraMesherConfig config;
    config.module = LoraMesher::LoraModules::SX1262_MOD;

    //Init the loramesher with a processReceivedPackets function
    radio.begin(config);

    //Start LoRaMesher
    radio.start();

    Serial.println("Lora initialized");
}
