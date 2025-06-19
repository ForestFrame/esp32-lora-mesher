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
}

void loop()
{
    uint32_t packetCount = TestDataGenerator::getInstance().getGeneratedPacketCount();
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
    // Example on how to change the module. See LoraMesherConfig to see all the configurable parameters.
    LoraMesher::LoraMesherConfig config;
    SPIClass *mySPI = new SPIClass(HSPI);
    mySPI->begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
    config.spi = mySPI;

    // Init the loramesher with a processReceivedPackets function
    radio.begin(config);

    // Start LoRaMesher
    radio.start();

    ESP_LOGW("INIT", "Lora initialized.");
}
