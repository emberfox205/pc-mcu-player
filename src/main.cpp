#include <Arduino.h>
#include <Wire.h>
#include "U8g2lib.h"

// Baud rate must be consistent between this and the Python client
const uint32_t BAUD_RATE = 115200;

// Image dimensions 
const uint16_t IMG_HEIGHT = 64;     
const uint16_t IMG_WIDTH = 88;      // Must be divisible by 8
const uint16_t IMG_BYTES = ((IMG_WIDTH * IMG_HEIGHT) / 8);

// Position (upper-left corner) of the animation on the OLED screen
const uint8_t X_COORD = 21;         
const uint8_t Y_COORD = 0;

// Serial receive buffer size
const uint16_t RX_BUFFER_SIZE = 1024;

// Determine number of buffer frames
const uint16_t IMG_RACK = 32;       

// (Unused) Track number of bytes read by Serial.readBytes()
static uint16_t bytesRead = 0;

// Index to track which frame slot is being written to 
uint16_t currentWriteFrame = 0; 
// Index to determine frame in imageArray to display
uint16_t currentDisplayFrame = 0;      

// Status of the frames held in imageArray
bool imageStatus[IMG_RACK] = {false}; 
uint8_t imageArray[IMG_RACK][IMG_BYTES]; // Ring buffer

// Task management
TaskHandle_t serialHandle = NULL;
TaskHandle_t displayHandle = NULL;

// For signaling setup() connection on initial port connection
bool isReady = false;             
const uint8_t DELAY = 5;

// Display Constructor
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0); 

void serialProcess(void * parameter) {
    for (;;) {
        // Signal that setup() is completed, as it takes a long time on initial start
        if (!isReady) {
            delay(DELAY);
            Serial.write('S');
        }

        while (Serial.available()) {
            isReady = true;
            bytesRead = Serial.readBytes(imageArray[currentWriteFrame], IMG_BYTES);
            imageStatus[currentWriteFrame] = 1;
            // Wait so as not to overwrite written frames
            uint16_t nextWriteFrame = (currentWriteFrame + 1) % IMG_RACK;
            while (imageStatus[nextWriteFrame]) {
                taskYIELD();
            }
            currentWriteFrame = nextWriteFrame;
            Serial.write("F");
        }
        taskYIELD();
    }
}

void display(void * parameter) {
    for (;;) {
        if (imageStatus[currentDisplayFrame]) {
            u8g2.clearBuffer();
            u8g2.drawXBM(
                X_COORD, 
                Y_COORD, 
                IMG_WIDTH, 
                IMG_HEIGHT, 
                imageArray[currentDisplayFrame]
            );
            u8g2.sendBuffer();
            imageStatus[currentDisplayFrame] = 0;
            currentDisplayFrame = (currentDisplayFrame + 1) % IMG_RACK;
        }
        // taskYIELD() (probably) only works with tasks with at least the same priority
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

// Codes that run once
void setup() {
    Serial.setRxBufferSize(RX_BUFFER_SIZE);
    Serial.begin(BAUD_RATE);
    Serial.println("Serial started");
    // Might speed up I2C display speed
    u8g2.setBusClock(1000000);
    u8g2.begin();
    u8g2.setDrawColor(0);

    xTaskCreate(
        serialProcess,
        "serialProcess",
        35000, // Arbitrary value
        NULL,
        0,
        &serialHandle
    );

    xTaskCreate(
        display,
        "display",
        25000, // Also arbitrary value
        NULL,
        1,
        &displayHandle
    );

    // Wait until imageArray is filled before displaying
    if (displayHandle != NULL) {
        vTaskSuspend(displayHandle);
    }
}

// Codes that run repeatedly
void loop() {
    if (imageStatus[IMG_RACK-1] && displayHandle != NULL) {
        vTaskResume(displayHandle);
    }
}
