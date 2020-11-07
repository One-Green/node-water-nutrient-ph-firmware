/*
 *  One-Green node type: water+nutrient+pH
 *  Water supplier sprinkler nodes
 *
 *  Inputs / Outputs board, communicate with ESP32 by UART 0
 *
 *
 * Author: Shanmugathas Vigneswaran
 * email: shanmugathas.vigneswaran@outlook.fr
 * */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "OGIO.h"


SoftwareSerial espSerial(2, 3); // RX, TX
OGIO io_handler;

void setup() {
    Serial.begin(115200);
    espSerial.begin(115200);
    io_handler.initR();
}

void loop() {
    cmdHandler();

}


void cmdHandler() {


    String espSerialCmd;
    StaticJsonDocument<200> doc;

    if (Serial.available()) {
        Serial.println("[espSerial] Command received");
        espSerialCmd = Serial.readString();
    }

    if (espSerialCmd == "GET_SENSORS") {
        Serial.println("[espSerial] cmd=GET_SENSORS received");

        doc["waterLevelCM"] = io_handler.getWaterLevelCM();
        doc["nutrientLevelCM"] = io_handler.getNutrientLevelCM();
        doc["pHDownerLevelCM"] = io_handler.getPhDownerLevelCM();
        doc["phLevel"] = io_handler.getPhLevel();
        doc["TDSLevel"] = io_handler.getTDS();

        Serial.println("Generated JSON: ");
        serializeJson(doc, Serial);

    }
}


