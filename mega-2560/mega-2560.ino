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
#include "OGIO.h"

const byte SLAVE_ADDRESS = 4;
int waterLevelCM = 0;
int nutrientLevelCM = 0;
int pHDownerLevelCM = 0;
int phLevel = 0;
int TDSLevel = 0;
char command;
OGIO io_handler;
enum {
    CMD_READ_ID = 1,
    CMD_READ_WATER_LEVEL = 2,
    CMD_READ_NUTRIENT_LEVEL = 3,
    CMD_READ_PH_DOWNER_LEVEL = 4,
    CMD_READ_PH_LEVEL = 5,
    CMD_READ_TDS_LEVEL = 6,
};

void setup() {
    command = 0;
    Wire.begin(SLAVE_ADDRESS);      // join i2c bus with address #4
    Wire.onReceive(receiveEvent);  // interrupt handler for incoming messages
    Wire.onRequest(requestEvent);  // interrupt handler for when data is wanted
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
    waterLevelCM = io_handler.getWaterLevelCM();
    nutrientLevelCM = io_handler.getNutrientLevelCM();
    pHDownerLevelCM = io_handler.getPhDownerLevelCM();
    phLevel = io_handler.getPhLevel();
    TDSLevel = io_handler.getTDS();
}

void loop() {
    getSensors();
}

void receiveEvent(int howMany) {
    /*
     * Change command type when master change it
     * */
    command = Wire.read();
}

void SendInteger(int val) {
    /*
     *  Write integer to master
     *  int = 2 bytes
     *
     * */
    byte buf[2];
    buf[0] = val >> 8;
    buf[1] = val & 0xFF;
    Wire.write(buf, 2);
}

void requestEvent() {
    /*
     * Handle command from master
     *
     * */
    switch (command) {
        case CMD_READ_ID: Wire.write(0x55); break;
        case CMD_READ_WATER_LEVEL: SendInteger(waterLevelCM); break;
        case CMD_READ_NUTRIENT_LEVEL: SendInteger(nutrientLevelCM); break;
        case CMD_READ_PH_DOWNER_LEVEL: SendInteger(pHDownerLevelCM); break;
        case CMD_READ_PH_LEVEL: SendInteger(phLevel); break;
        case CMD_READ_TDS_LEVEL: SendInteger(TDSLevel); break;
        //case CMD_READ_NUTRIENT_LEVEL: Wire.write(digitalRead(8)); break;
    }
}