/*
This is an attempt to sync data transfer using examples from:
https://forum.arduino.cc/t/serial-input-basics-updated/382007
WARNING: Does not work as it is
*/

#include <Arduino.h>
#include <Wire.h>
#include "U8g2lib.h"

#define IMG_HEIGHT 64
#define IMG_WIDTH 64
#define BYTE_CHUNK 64
#define CHUNK_COUNT 8
#define IMG_BYTES 512

unsigned char imageArray[IMG_BYTES];
unsigned char byteArray[BYTE_CHUNK];
char marker[BYTE_CHUNK];
const char* header = "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef";
const char* footer = "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890";

int bytePos = 0;
int bytesRead = 0;
boolean isFrameReady = false;

void recvBytes();
void displayImg();

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

void setup() {
    Serial.begin(115200);
    Serial.println("Serial established");
    u8g2.begin();
    Serial.println("U8g2 began");
    u8g2.clearBuffer();
    Serial.println("U8g2 buffer cleared");
    u8g2.setDrawColor(0);
    Serial.println("Setup completed");
}

void loop() {
    recvBytes();
    displayImg();
}

void recvBytes() {
    static boolean recvInProgress = false;

    while (Serial.available()) {
        Serial.readBytes(marker, BYTE_CHUNK);
        if (recvInProgress) {
            if (strcmp(marker, footer) != 0) {
                bytesRead = Serial.readBytes(byteArray, BYTE_CHUNK);
                memcpy(imageArray + bytePos, byteArray, BYTE_CHUNK);
                bytePos += BYTE_CHUNK;
                Serial.println("Chunk copied");
            }
            else {
                recvInProgress = false;
                Serial.println("End receive");
                if (bytePos >= IMG_BYTES) {
                    isFrameReady = true;
                    Serial.println("Image transferred");
                }
            }
        }
        else if (strcmp(marker, header) !=0) {
            Serial.println("Start receive");
            recvInProgress = true;
            // Stop displaying when new data is copied into imageArray
            isFrameReady = false;
        }
    } 
}

void displayImg() {
    if (isFrameReady) {
        u8g2.clearBuffer();
        u8g2.drawXBM(32, 0, IMG_WIDTH, IMG_HEIGHT, imageArray);
        u8g2.sendBuffer();
        bytePos = 0;
    }
}
