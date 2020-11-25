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

char CMD_ON_WATER_PUMP[3] = "H1";
char CMD_OFF_WATER_PUMP[3] = "L1";

char CMD_ON_NUTRIENT_PUMP[3] = "H2";
char CMD_OFF_NUTRIENT_PUMP[3] = "L2";

char CMD_ON_PH_DOWNER_PUMP[3] = "H2";
char CMD_OFF_PH_DOWNER_PUMP[3] = "L2";

char CMD_ON_MIXER_PUMP[3] = "H3";
char CMD_OFF_MIXER_PUMP[3] = "L3";

// ------------------------------------ // end

void setup() {
    Serial.begin(9600);                   // Debug Serial communication
    EXSerial.begin(115200);                 // ESP32 Serial communication
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

void callBackUnknown() {
    /*
     *  Write unknown command
     *  if command is not detected
     *  write "ER" to not lock ESP32 while waiting response
     *
     * */
    char _[2] = "ER";
    EXSerial.write(_, 2);
    Serial.println("[EXSerial] unknown CMD sent");
}

void callBackIOJson() {
    /*
     *  Send IO status
     *  sensors and actuator
     *  TODO: Implement actuator status
     *
     * */
    doc["water_level_cm"] = (int) io_handler.getWaterLevelCM();
    doc["nutrient_level_cm"] = (int) io_handler.getNutrientLevelCM();
    doc["ph_downer_level_cm"] = (int) io_handler.getPhDownerLevelCM();
    doc["ph_level"] = (int) io_handler.getPhLevel();
    doc["tds_level"] = (int) io_handler.getTDS();

    doc["water_pump_state"] = io_handler.getWaterPumpStatus();
    doc["nutrient_pump_state"] = io_handler.getNutrientPumpStatus();
    doc["ph_downer_pump_state"] = io_handler.getPhDownerPumpStatus();
    doc["mixer_pump_state"] = io_handler.getMixerPumpStatus();

    serializeJson(doc, EXSerial);
    Serial.println("[EXSerial] Sensors JSON sent");
    serializeJsonPretty(doc, Serial);
}

void loop() {
    CMD = readCMDFromESP();

    if (CMD == String(CMD_ALIVE)) { callBackWriteAlive(); }

    else if (CMD == String(CMD_READ_IO)) { callBackIOJson(); }

    else if (CMD == String(CMD_ON_WATER_PUMP)) { io_handler.onWaterPump(); }
    else if (CMD == String(CMD_OFF_WATER_PUMP)) { io_handler.offWaterPump(); }

    else if (CMD == String(CMD_ON_NUTRIENT_PUMP)) { io_handler.onNutrientPump(); }
    else if (CMD == String(CMD_OFF_NUTRIENT_PUMP)) { io_handler.offNutrientPump(); }

    else if (CMD == String(CMD_ON_PH_DOWNER_PUMP)) { io_handler.onPhDownerPump(); }
    else if (CMD == String(CMD_OFF_PH_DOWNER_PUMP)) { io_handler.offPhDownerPump(); }

    else if (CMD == String(CMD_ON_MIXER_PUMP)) { io_handler.onMixerPump(); }
    else if (CMD == String(CMD_OFF_MIXER_PUMP)) { io_handler.offMixerPump(); }

    else
        callBackUnknown();

}
