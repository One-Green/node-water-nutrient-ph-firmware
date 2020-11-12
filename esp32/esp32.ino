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
#include <ArduinoJson.h>
#include <Wire.h>

void setup(){
    Serial.begin(9600);
    while(!Serial){} // Waiting for serial connection

    Serial.println();
    Serial.println("Start I2C scanner ...");
    Serial.print("\r\n");
    byte count = 0;

    Wire.begin();
    for (byte i = 8; i < 120; i++)
    {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("Found I2C Device: ");
            Serial.print(" (0x");
            Serial.print(i, HEX);
            Serial.println(")");
            count++;
            delay(1);
        }
    }
    Serial.print("\r\n");
    Serial.println("Finish I2C scanner");
    Serial.print("Found ");
    Serial.print(count, HEX);
    Serial.println(" Device(s).");
}


void loop() {

    String strJSON;
    StaticJsonDocument<200> doc;
    Serial.println("[espSerial] Sending cmd=\"GET_SENSORS\"");
    delay(1000);

}