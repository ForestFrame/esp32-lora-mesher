/*
 *FileName:     common.h
 *Description:
 */


#ifndef __COMMON_H__
#define __COMMON_H__

/* ----------------------------------------------------------------------------
 * include
----------------------------------------------------------------------------- */
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "utilities.h"
#include <U8g2lib.h>

/* ----------------------------------------------------------------------------
 * defines
----------------------------------------------------------------------------- */
#define LOGI(fmt, ...) Serial.printf("[INFO][%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGD(fmt, ...) Serial.printf("[DBUG][%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGW(fmt, ...) Serial.printf("[WARN][%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGE(fmt, ...) Serial.printf("[ERR ][%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define OLED_LOGE(fmt, ...) do { \
    char buf[64]; \
    snprintf(buf, sizeof(buf), "[ERR][%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    logToOled(buf); \
} while(0)

#define DISPLAY_MODEL U8G2_SSD1306_128X64_NONAME_F_HW_I2C

/* ----------------------------------------------------------------------------
 * structs typedef
----------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------
 * function prototypes
----------------------------------------------------------------------------- */
void logToOled(const char *msg);

#endif