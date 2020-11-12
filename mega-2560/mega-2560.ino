/*
 *  One-Green node type: water+nutrient+pH
 *  Water supplier sprinkler nodes
 *
 *  Inputs / Outputs board, communicate with ESP32 by I2C address = 4
 *
 *
 * Author: Shanmugathas Vigneswaran
 * email: shanmugathas.vigneswaran@outlook.fr
 * */

#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Thread.h>
#include "OGIO.h"

char GET_SENSORS[] = "GET_SENSORS";

int waterLevelCM = 0;
int nutrientLevelCM = 0;
int pHDownerLevelCM = 0;
int phLevel = 0;
int TDSLevel = 0;

String cmd = ""; // handle command received by ESP32 trough Wire
StaticJsonDocument<200> sensorsDict;
OGIO io_handler;

void setup() {
    Wire.begin(4);                  // join i2c bus with address #4
    Wire.onReceive(receiveEvent);   // register event
    Wire.onRequest(requestEvent);   //
    Serial.begin(9600);             // start serial for output
    Serial.println("[Wire] Slave started");  // print slave has started
    io_handler.initR();             // I/O handler
}


void getSensors() {
    /*
     *   Read sensors values and
     *   create doc JSON key values based
     *
     * */
    Serial.println("[Sensors] Updating sensors values ");
    sensorsDict["waterLevelCM"] = io_handler.getWaterLevelCM();
    sensorsDict["nutrientLevelCM"] = io_handler.getNutrientLevelCM();
    sensorsDict["pHDownerLevelCM"] = io_handler.getPhDownerLevelCM();
    sensorsDict["phLevel"] = io_handler.getPhLevel();
    sensorsDict["TDSLevel"] = io_handler.getTDS();
    serializeJson(sensorsDict, Wire);
    Serial.println("[Sensors] Generated JSON: ");
    serializeJson(sensorsDict, Serial);
    Serial.println();
}

void loop() {
    delay(100);
}

void receiveEvent(int howMany) {
    /*
     *  Event listener callback function
     *
     * */

    cmd = ""; // flush last command

    while (Wire.available()) {
        cmd += (char) Wire.read();
    }
    Serial.print("[Wire] Received command : ");
    Serial.println(cmd);

    if (cmd == GET_SENSORS) {
        Serial.println("[Wire] Request sensors values detected ...");
    }
}


void requestEvent() {
//
//    if (cmd == GET_SENSORS) {
//        getSensors();
//    }
}

