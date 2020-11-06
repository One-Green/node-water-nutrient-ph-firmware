One-Green node Water+Nutrient+phDowner firmware
===============================================


Water supplier for sprinkler node, including:
    - tank, nutrient, pH downer level
    - auto adjusting pH level, nutrient level

Based on two boards (Soft serial communication) :
    - Atmega 2560 for inputs/outputs
    - ESP32 for Wifi/TFT display/MQTT interaction

Requirements:
    - Board: 1x ESP32, 1x Atmel Atmega 2560 (Mega 2560 Pro mini)
    - Firmware: Arduino
    - Arduino IDE (https://www.arduino.cc/en/main/software)
    - AVR Compiler (Available on Arduino IDO Board manager)
    - ESP32 compiler (https://github.com/espressif/arduino-esp32)


Arduino/C++ Library:
    - https://github.com/adafruit/Adafruit-ST7735-Library/ (Available on Arduino IDE Package manager)
    - https://github.com/bblanchon/ArduinoJson (Available on Arduino IDE Package manager)
    - https://github.com/knolleary/pubsubclient (Available on Arduino IDE Package manager)
