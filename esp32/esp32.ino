/*
 *  One-Green node type: water+nutrient+pH
 *  Water supplier sprinkler nodes
 *  ESP32 board firmware
 *  Purpose:
 *      - Interact with mega-2560 board for Inputs/Outputs
 *      - Interact with master trough MQTT
 *      - Interact with ST7735 screen
 *
 * I2C wiring :
 *
 *     +-----+-----------+-------+
 *     |     | Mega 2560 | ESP32 |
 *     +=====+===========+=======+
 *     | SDA |  20       |  21   |
 *     | SCL |  21       |  22   |
 *     +-----+-----------+-------+
 *
 * Author: Shanmugathas Vigneswaran
 * email: shanmugathas.vigneswaran@outlook.fr
 * */

#include <Arduino.h>
#include <Wire.h>

int SLAVE_ADDRESS = 4;
enum {
    CMD_READ_ID = 1,
    CMD_READ_WATER_LEVEL = 2,
    CMD_READ_NUTRIENT_LEVEL = 3,
    CMD_READ_PH_DOWNER_LEVEL = 4,
    CMD_READ_PH_LEVEL = 5,
    CMD_READ_TDS_LEVEL = 6,
};

void sendCommand(const byte cmd, const int responseSize) {
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(cmd);
    Wire.endTransmission();
    Wire.requestFrom(SLAVE_ADDRESS, responseSize);
}

void setup() {
    Wire.begin();
    Serial.begin(9600);
    sendCommand(CMD_READ_ID, 1);
    if (Wire.available()) {
        Serial.print("Slave is ID: ");
        Serial.println(Wire.read(), DEC);
    } else
        Serial.println("No response to ID request");

}

void loop() {
    int val;
    sendCommand(CMD_READ_WATER_LEVEL, 2);
    val = Wire.read();
    val <<= 8;
    val |= Wire.read();
    Serial.print("Water level : ");
    Serial.println(val, DEC);

    sendCommand(CMD_READ_NUTRIENT_LEVEL, 2);
    val = Wire.read();
    val <<= 8;
    val |= Wire.read();
    Serial.print("Nutrient level : ");
    Serial.println(val, DEC);

    sendCommand(CMD_READ_PH_DOWNER_LEVEL, 2);
    val = Wire.read();
    val <<= 8;
    val |= Wire.read();
    Serial.print("PH downer level : ");
    Serial.println(val, DEC);

    sendCommand(CMD_READ_PH_LEVEL, 2);
    val = Wire.read();
    val <<= 8;
    val |= Wire.read();
    Serial.print("PH level : ");
    Serial.println(val, DEC);

    sendCommand(CMD_READ_TDS_LEVEL, 2);
    val = Wire.read();
    val <<= 8;
    val |= Wire.read();
    Serial.print("TDS level : ");
    Serial.println(val, DEC);

    delay(500);
}