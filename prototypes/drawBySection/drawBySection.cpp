/*
Print byte chunks immediately as they arrive. Lower resources consumption.
WARNING: No longer works with current client. 
 */

#include <Arduino.h>
#include <Wire.h>
#include "U8g2lib.h"

#define BYTE_CHUNK 64
#define IMG_HEIGHT 64
#define IMG_WIDTH 88

uint8_t byteArray[BYTE_CHUNK];

int bytePos = 0;
int yCoord = 0;

boolean isFrameReady = false;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

void setup() {
    Serial.begin(115200);
    u8g2.begin();
    u8g2.setDrawColor(0);
}

void loop() {
    int bytesRead = 0;

    while (Serial.available()) {
        bytesRead = Serial.readBytes(byteArray, BYTE_CHUNK);
    }

    if (bytesRead && yCoord < IMG_HEIGHT) {
        u8g2.drawXBM(32, yCoord, IMG_WIDTH, IMG_HEIGHT/8, byteArray);
        u8g2.sendBuffer();
        yCoord += BYTE_CHUNK;
    }

    if (yCoord >= IMG_HEIGHT) {
        yCoord = 0;
    }
}

