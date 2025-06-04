/*
 *FileName:    common.cpp
 *Description:
 */

/* ----------------------------------------------------------------------------
 * include
----------------------------------------------------------------------------- */
#include "common.h"

/* ----------------------------------------------------------------------------
 * defines
----------------------------------------------------------------------------- */
#define OLED_LOG_FONT u8g2_font_6x10_tf
#define OLED_LOG_LINE_CHARS 21
#define OLED_LOG_MAX_LINES 6

/* ----------------------------------------------------------------------------
 * variables
----------------------------------------------------------------------------- */
/*
 * extern variables
 */
extern DISPLAY_MODEL *u8g2;

/*
 * static variables
 */
static String oledLogBuffer[OLED_LOG_MAX_LINES];
static int oledLogLine = 0;

void logToOled(const char* msg) {
    if (!u8g2) return;
    String str(msg);
    int start = 0;
    while (start < str.length()) {
        String line = str.substring(start, start + OLED_LOG_LINE_CHARS);
        oledLogBuffer[oledLogLine] = line;
        oledLogLine = (oledLogLine + 1) % OLED_LOG_MAX_LINES;
        start += OLED_LOG_LINE_CHARS;
    }
    u8g2->clearBuffer();
    u8g2->setFont(OLED_LOG_FONT);
    for (int i = 0; i < OLED_LOG_MAX_LINES; ++i) {
        int idx = (oledLogLine + i) % OLED_LOG_MAX_LINES;
        if (oledLogBuffer[idx].length() > 0) {
            u8g2->setCursor(0, i * 10); // Y坐标从0开始，每行10像素
            u8g2->print(oledLogBuffer[idx]);
        }
    }
    u8g2->sendBuffer();
}