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

int waterLevelCM = 0;
int nutrientLevelCM = 0;
int pHDownerLevelCM = 0;
int phLevel = 0;
int TDSLevel = 0;
String cmd = ""; // handle command received by ESP32 trough Wire
StaticJsonDocument<200> doc;

OGIO io_handler;

void setup() {
    Wire.begin(4);                  // join i2c bus with address #4
    Wire.onReceive(receiveEvent);   // register event
    Serial.begin(9600);             // start serial for output
    Serial.println("slave setup");  //
    io_handler.initR();             // I/O handler
    updatedSensors();               // update sensors values for first time
}


void updatedSensors() {
    /*
     *   Create doc JSON key values based
     *
     * */
    Serial.println("[Sensors] Updating sensors values ");
    doc["waterLevelCM"] = io_handler.getWaterLevelCM();
    doc["nutrientLevelCM"] = io_handler.getNutrientLevelCM();
    doc["pHDownerLevelCM"] = io_handler.getPhDownerLevelCM();
    doc["phLevel"] = io_handler.getPhLevel();
    doc["TDSLevel"] = io_handler.getTDS();
    Serial.println("[Sensors] Generated JSON: ");
    serializeJson(doc, Serial);
    Serial.println();
}

void loop() {
    //updatedSensors();
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
}
