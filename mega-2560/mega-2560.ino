/*
 *  One-Green node type: water+nutrient+pH
 *  Water supplier sprinkler nodes
 *
 *  Inputs / Outputs board, communicate with ESP32 by UART
 *
 *
 * Author: Shanmugathas Vigneswaran
 * email: shanmugathas.vigneswaran@outlook.fr
 * */


#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "OGIO.h"

#define RXD10 10
#define TXD11 11

SoftwareSerial EXSerial(RXD10, TXD11);  // Rx, Tx
OGIO io_handler;                        // sensors/actuator handler

StaticJsonDocument<200> doc;            // JSON static allocation for IO
char buffer[10];                        // Serial2/EXSerial buffer
String CMD;                             // CMD received from ESP32

// ------------------------------------ // Exchange command ESP32-Mega
char CMD_ALIVE[2] = "A";
char CMD_READ_IO[3] = "IO";
// ------------------------------------ // end

void setup() {
    Serial.begin(9600);                   // Debug Serial communication
    EXSerial.begin(9600);                 // ESP32 Serial communication
    io_handler.initR();                   // I/O setup digital pin mode
    Serial.println("Serial/ExSerial ok");
}

String readCMDFromESP() {
    /*
     *  Read received command from ESP32
     *
     * */
    if (EXSerial.available() > 0) {
        EXSerial.readBytes(buffer, 10);
        Serial.println("[EXSerial] CMD=" + String(buffer));
    }
    return String(buffer);
}

void callBackWriteAlive() {
    /*
     *  Write mega is alive
     *  TODO Implement ESP > MEGA reset if serial disconnected
     * */

    EXSerial.write(CMD_ALIVE, 2);
    Serial.println("[EXSerial] alive sent");
}

void callBackIOJson() {
    /*
     *  Send IO status
     *  sensors and actuator
     *  TODO: Implement actuator status
     *
     * */
    doc["water_level_cm"] = io_handler.getWaterLevelCM();
    doc["nutrient_level_cm"] = io_handler.getNutrientLevelCM();
    doc["ph_downer_level_cm"] = io_handler.getPhDownerLevelCM();
    doc["ph_level"] = io_handler.getPhLevel();
    doc["tds_level"] = io_handler.getTDS();
    serializeJson(doc, EXSerial);
    Serial.println("[EXSerial] Sensors JSON sent");
    serializeJsonPretty(doc, Serial);
}

void loop() {
    CMD = readCMDFromESP();

    if (CMD == String(CMD_ALIVE)) {
        callBackWriteAlive();
    }

    if (CMD == String(CMD_READ_IO)) {
        callBackIOJson();
    }

}