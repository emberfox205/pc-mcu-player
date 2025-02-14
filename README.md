# Computer-Microcontroller Player 

Solution to display videos/animations directly from computer to a simulated microcontroller. 

---

## Overview

Embedded C++ codes and a Python client to send xbm files through serial.

Made to target ESP32 Devkit-V1 / DevkitC-V4 connected to a SSD1306 128x64 through I2C.

Currently can only achieve ~6 FPS.

Demonstration: [Bad Apple!!](https://www.youtube.com/watch?v=zMEHnW_Ern8&t=1s)

## Usage 

1. Make sure you have [Wokwi for VSCode](https://docs.wokwi.com/vscode/getting-started) installed. 
2. Make adjustments as needed to `main.cpp` and `client.py`.
3. Build `main.cpp`.
4. Open `diagram.json` and run the simulator from there.
5. Run `client.py`.

> [!warning]
> Developed using Wokwi simulator and PlatformIO extension for VSCode. Might not work with actual boards.
