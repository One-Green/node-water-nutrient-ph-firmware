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
#include "SerialBridgeConf.h"
#include "SerialEndpoint.h"

#define RXD10 10
#define TXD11 11

SoftwareSerial EXSerial(RXD10, TXD11);  // Rx, Tx

void setup() 
{
    Serial.begin(9600);                   // Debug Serial communication
    EXSerial.begin(4800);                 // ESP32 Serial communication
    SerialEndpoint.begin();               // Init Serial Bridge
    io_handler.initR();                   // I/O setup digital pin mode
    Serial.println("Serial/ExSerial ok");
}

void loop() 
{
    SerialEndpoint.loop();
}
