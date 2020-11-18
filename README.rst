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

Arduino/C++ Library for ESP32 Board:
    - https://github.com/adafruit/Adafruit-ST7735-Library/ (Available on Arduino IDE Package manager)
    - https://github.com/bblanchon/ArduinoJson (Available on Arduino IDE Package manager)
    - https://github.com/knolleary/pubsubclient (Available on Arduino IDE Package manager)
    - https://github.com/plerup/espsoftwareserial (Available on Arduino IDE Package manager)
    - https://github.com/ivanseidel/ArduinoThread (Available on Arduino IDE Package manager)

Arduino/C++ Library for Mega 2560 Board:
    - https://github.com/bblanchon/ArduinoJson (Available on Arduino IDE Package manager)
    - https://github.com/ivanseidel/ArduinoThread (Available on Arduino IDE Package manager)

Special Thanks to:
    - To https://github.com/nickgammon:
        - http://www.gammon.com.au/forum/?id=10896 I2C communication tutorial