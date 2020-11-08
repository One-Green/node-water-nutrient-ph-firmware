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
#include <Thread.h>
#include "OGIO.h"

int waterLevelCM = 0;
int nutrientLevelCM = 0;
int pHDownerLevelCM = 0;
int phLevel = 0;
int TDSLevel = 0;

/*
 * Source: https://www.arduino.cc/en/Reference/SoftwareSerial
 * Not all pins on the Mega and Mega 2560 support change interrupts,
 * so only the following can be used for RX: 10, 11, 12, 13, 14, 15, 50, 51, 52, 53, A8 (62), A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14 (68), A15 (69).
 */
SoftwareSerial espSerial(10, 11); // RX, TX
OGIO io_handler;
Thread readSensorThread = Thread();

void setup() {

    Serial.begin(115200);
    espSerial.begin(115200);

    io_handler.initR();

    readSensorThread.onRun(readSensors);
    readSensorThread.setInterval(500);

}


void readSensors() {
    waterLevelCM = io_handler.getWaterLevelCM();
    nutrientLevelCM = io_handler.getNutrientLevelCM();
    pHDownerLevelCM = io_handler.getPhDownerLevelCM();
    phLevel = io_handler.getPhLevel();
    TDSLevel = io_handler.getTDS();
}

void loop() {
    cmdHandler();

}


void cmdHandler() {


    String espSerialCmd;
    StaticJsonDocument<200> doc;

    if (Serial.available()) {
        Serial.println("[Serial] Command received");
        espSerialCmd = Serial.readString();
    }

    if (espSerial.available()) {
        Serial.println("[espSerial] Command received");
        espSerialCmd = Serial.readString();
    }

    if (espSerialCmd == "GET_SENSORS") {
        Serial.println("[Serial/espSerial] cmd=GET_SENSORS received");

        doc["waterLevelCM"] = waterLevelCM;
        doc["nutrientLevelCM"] = nutrientLevelCM;
        doc["pHDownerLevelCM"] = pHDownerLevelCM;
        doc["phLevel"] = phLevel;
        doc["TDSLevel"] = TDSLevel;

        Serial.println("Generated JSON: ");
        serializeJson(doc, espSerial);
        serializeJson(doc, Serial);

        Serial.println();
        Serial.println("[Serial/espSerial] JSON serialized on Serial/espSerial");


    }
}


