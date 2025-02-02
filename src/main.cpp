#include <Arduino.h>
#include <Wire.h>
#include "U8g2lib.h"

// 256 is the maximum rx buffer size
// Controller's BYTE_CHUNK being SLIGHTLY smaller than pc's CHUNK_SIZE
// helps minimize delay
const uint16_t BYTE_CHUNK = 240; 
const uint16_t IMG_HEIGHT = 64;
const uint16_t IMG_WIDTH = 88; // Must be divisible by 8
const uint16_t IMG_BYTES = ((IMG_WIDTH * IMG_HEIGHT) / 8);
const uint8_t DELAY = 1;
const uint8_t DELAY_OFFSET = 8; // Pad delay to wait for data
const uint8_t X_COORD = 21;
const uint8_t Y_COORD = 0;

uint8_t imageArray[IMG_BYTES];
uint8_t byteArray[BYTE_CHUNK];

int bytePos = 0; // Track where new bytes are being added to imageArray
int bytesRead = 0;
bool isReadyToReceive = false;

// Display Constructor 
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// Codes that run once
void setup() {
    Serial.begin(921600);
    Serial.println("Serial established");
    u8g2.setBusClock(1000000);
    u8g2.begin();
    u8g2.setDrawColor(0);
}

// Codes that run repeatedly
void loop() {
    /*
    Help to signal setup() is completed on initial port connection, since 
    controller's restart somehow takes too much time.
    */
    if (!isReadyToReceive) {
        delay(DELAY);
        Serial.write("F");
    }

    if (Serial.available()) {
        isReadyToReceive = true;
        // Store a byte chunk into byteArray
        unsigned long xStart = micros();
        if ((IMG_BYTES - bytePos) < BYTE_CHUNK) {
            // Handle last chunk being smaller than expected
            bytesRead = Serial.readBytesUntil('\n', imageArray + bytePos, BYTE_CHUNK);
            // Signal computer to send new data
            Serial.write("C");
        }
        else {
            // Handle most other chunks
            bytesRead = Serial.readBytes(imageArray + bytePos, BYTE_CHUNK);
            // Signal computer to send new data
            Serial.write("C");
            // Add delay to wait for chunk till it arrives in full
            // Slows down first chunk but speeds up last chunk
            delay(DELAY+ DELAY_OFFSET);
        }

        bytePos += bytesRead;

        unsigned long xEnd = micros();
        Serial.print("\nRead: ");
        Serial.println(xEnd - xStart);
    }

    // Display image when imageArray is full
    if (bytePos >= IMG_BYTES) {
        unsigned long displayStart = micros();
        u8g2.drawXBM(X_COORD, Y_COORD, IMG_WIDTH, IMG_HEIGHT, imageArray);
        u8g2.sendBuffer();
        bytePos = 0;
        unsigned long displayEnd = micros();
        Serial.print("\nDisplay: ");
        Serial.println(displayEnd - displayStart);
    }
}


