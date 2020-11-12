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

StaticJsonDocument<200> doc;

OGIO io_handler;
Thread readSensorThread = Thread();

void setup() {
    Wire.begin(4);                  // join i2c bus with address #4
    Wire.onReceive(receiveEvent);   // register event
    Serial.begin(9600);             // start serial for output
    Serial.println("slave setup");  //
    io_handler.initR();             // I/O handler
    readSensorThread.onRun(readSensors);
    readSensorThread.setInterval(500);

}


void readSensors() {

    waterLevelCM = io_handler.getWaterLevelCM();
    nutrientLevelCM = io_handler.getNutrientLevelCM();
    pHDownerLevelCM = io_handler.getPhDownerLevelCM();
    phLevel = io_handler.getPhLevel();
    TDSLevel = io_handler.getTDS();

    doc["waterLevelCM"] = waterLevelCM;
    doc["nutrientLevelCM"] = nutrientLevelCM;
    doc["pHDownerLevelCM"] = pHDownerLevelCM;
    doc["phLevel"] = phLevel;
    doc["TDSLevel"] = TDSLevel;

    Serial.println("Generated JSON: ");
    serializeJson(doc, Serial);

}

void loop() {
    delay(100);
}


void receiveEvent(int howMany) {
    while (1 < Wire.available()) // loop through all but the last
    {
        char c = Wire.read(); // receive byte as a character
        Serial.print(c);         // print the character
    }
    int x = Wire.read();    // receive byte as an integer
    Serial.println(x);         // print the integer
}

