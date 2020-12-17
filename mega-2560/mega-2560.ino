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
#include "OGIO.h"

#define RXD10 10
#define TXD11 11

SoftwareSerial EXSerial(RXD10, TXD11);  // Rx, Tx
OGIO io_handler;                        // sensors/actuator handler

char buffer[10];                        // EXSerial/EXSerial buffer
String CMD;                             // CMD received from ESP32

// ------------------------------------ // Exchange command ESP32-Mega
char CMD_ALIVE[2] = "A";

char CMD_GET_WATER_LEVEL[3] = "S0";
char CMD_GET_NUTRIENT_LEVEL[3] = "S1";
char CMD_GET_PH_DOWNER_LEVEL[3] = "S2";
char CMD_GET_TDS[3] = "S3";
char CMD_GET_PH[3] = "S4";

char CMD_GET_WATER_PUMP_STATE[3] = "P0";
char CMD_GET_NUTRIENT_PUMP_STATE[3] = "P1";
char CMD_GET_PH_DOWNER_PUMP_STATE[3] = "P2";
char CMD_GET_MIXER_PUMP_STATE[3] = "P3";

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
    EXSerial.begin(4800);                 // ESP32 Serial communication
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

void writeIntegerEXSerial(int intValue) {

    int data[2];
    data[0] = intValue & 0xFF;
    data[1] = (intValue >> 8);
    EXSerial.write(data[0]);
    EXSerial.write(data[1]);
    delay(50);
}

void writeBoolEXSerial(bool boolValue) {
    int boolToInt = (int) boolValue;
    EXSerial.write(boolToInt);
    delay(50);
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
    Serial.println("[EXSerial] unknown/no CMD received from ESP32");
}

void flushEXSerial() {
    while (EXSerial.available() > 0)
        EXSerial.read();
}

void loop() {

    int sensor_value;
    bool actuator_state;

    CMD = readCMDFromESP();

    if (CMD == String(CMD_ALIVE)) {
        callBackWriteAlive();
    }

        // Sensors callbacks
    else if (CMD == String(CMD_GET_WATER_LEVEL)) {
        sensor_value = io_handler.getWaterLevelCM();
        writeIntegerEXSerial(sensor_value);
        Serial.println("[EXSerial] Responding to CMD=" + CMD + ", water level=" + sensor_value);

    } else if (CMD == String(CMD_GET_NUTRIENT_LEVEL)) {
        sensor_value = io_handler.getNutrientLevelCM();
        writeIntegerEXSerial(sensor_value);
        Serial.println("[EXSerial] Responding to CMD=" + CMD + ", nutrient level=" + sensor_value);

    } else if (CMD == String(CMD_GET_PH_DOWNER_LEVEL)) {
        sensor_value = io_handler.getPhDownerLevelCM();
        writeIntegerEXSerial(sensor_value);
        Serial.println("[EXSerial] Responding to CMD=" + CMD + ", ph downer level=" + sensor_value);

    } else if (CMD == String(CMD_GET_TDS)) {
        sensor_value = io_handler.getPhLevel();
        writeIntegerEXSerial(sensor_value);
        Serial.println("[EXSerial] Responding to CMD=" + CMD + ", tds=" + sensor_value);

    } else if (CMD == String(CMD_GET_PH)) {
        sensor_value = io_handler.getTDS();
        writeIntegerEXSerial(sensor_value);
        Serial.println("[EXSerial] Responding to CMD=" + CMD + ", pH=" + sensor_value);
    }

        // Actuator status callback
    else if (CMD == String(CMD_GET_WATER_PUMP_STATE)) {
        actuator_state = (bool) io_handler.getWaterPumpStatus();
        writeBoolEXSerial(actuator_state);
        Serial.println("[EXSerial] Responding to CMD=" + CMD + ", water pump=" + String(actuator_state));

    } else if (CMD == String(CMD_GET_NUTRIENT_PUMP_STATE)) {
        actuator_state = (bool) io_handler.getNutrientPumpStatus();
        writeBoolEXSerial(actuator_state);
        Serial.println("[EXSerial] Responding to CMD=" + CMD + ", nutrient pump=" + String(actuator_state));

    } else if (CMD == String(CMD_GET_PH_DOWNER_PUMP_STATE)) {
        actuator_state= (bool) io_handler.getPhDownerPumpStatus();
        writeBoolEXSerial(actuator_state);
        Serial.println("[EXSerial] Responding to CMD=" + CMD + ", ph downer pump=" + String(actuator_state));

    } else if (CMD == String(CMD_GET_MIXER_PUMP_STATE)) {
        actuator_state= (bool) io_handler.getMixerPumpStatus();
        writeBoolEXSerial(actuator_state);
        Serial.println("[EXSerial] Responding to CMD=" + CMD + ", mixer pump=" + String(actuator_state));

    }

        // Actuator ON/OFF
    else if (CMD == String(CMD_ON_WATER_PUMP)) {
        io_handler.onWaterPump();
        Serial.println("[EXSerial] Actuator CMD=" + CMD + ", water pump=ON");
    } else if (CMD == String(CMD_OFF_WATER_PUMP)) {
        io_handler.offWaterPump();
        Serial.println("[EXSerial] Actuator CMD=" + CMD + ", water pump=OFF");
    } else if (CMD == String(CMD_ON_NUTRIENT_PUMP)) {
        io_handler.onNutrientPump();
        Serial.println("[EXSerial] Actuator CMD=" + CMD + ", nutrient pump=ON");
    } else if (CMD == String(CMD_OFF_NUTRIENT_PUMP)) {
        io_handler.offNutrientPump();
        Serial.println("[EXSerial] Actuator CMD=" + CMD + ", nutrient pump=OFF");
    } else if (CMD == String(CMD_ON_PH_DOWNER_PUMP)) {
        io_handler.onPhDownerPump();
        Serial.println("[EXSerial] Actuator CMD=" + CMD + ", pH downer pump=ON");
    } else if (CMD == String(CMD_OFF_PH_DOWNER_PUMP)) {
        io_handler.offPhDownerPump();
        Serial.println("[EXSerial] Actuator CMD=" + CMD + ", pH downer pump=OFF");
    } else if (CMD == String(CMD_ON_MIXER_PUMP)) {
        io_handler.onMixerPump();
        Serial.println("[EXSerial] Actuator CMD=" + CMD + ", mixer pump=ON");
    } else if (CMD == String(CMD_OFF_MIXER_PUMP)) {
        io_handler.offMixerPump();
        Serial.println("[EXSerial] Actuator CMD=" + CMD + ", mixer pump=OFF");
    } else
        callBackUnknown();

}
