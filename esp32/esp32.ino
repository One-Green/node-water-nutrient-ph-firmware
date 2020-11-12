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

char GET_SENSORS[] = "GET_SENSORS";

const size_t capacity = JSON_OBJECT_SIZE(5) + 200;
StaticJsonDocument<200> sensorsDict;

void setup() {
    Serial.begin(9600);
    Wire.begin();
}


void loop() {
    getSensors();
    delay(500);
}


void getSensors() {
    Wire.beginTransmission(4);
    Wire.write(GET_SENSORS);
    Wire.endTransmission();

//    TODO: need  Deserialization failed ... need to check response
//    Wire.requestFrom(8, capacity);
//    DynamicJsonDocument doc(capacity);
//
//    Serial.println("[Wire] Generated JSON: ");
//    serializeJson(doc, Serial);
//
//    DeserializationError err = deserializeJson(doc, Wire);
//    switch (err.code()) {
//        case DeserializationError::Ok:
//            Serial.println(F("Deserialization succeeded"));
//            serializeJson(doc, Serial);
//            break;
//        case DeserializationError::InvalidInput:
//            Serial.println(F("Invalid input!"));
//            break;
//        case DeserializationError::NoMemory:
//            Serial.println(F("Not enough memory"));
//            break;
//        default:
//            Serial.println(F("Deserialization failed"));
//            break;
//    }
}