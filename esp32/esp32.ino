/*
 *  One-Green node type: water+nutrient+pH
 *  Water supplier sprinkler nodes
 *  ESP32 board firmware
 *  Purpose:
 *      - Interact with mega-2560 board for Inputs/Outputs
 *      - Interact with master trough MQTT
 *      - Interact with ST7735 screen
 *
 * ----------- ESP32 UART ----------------
 * UART	    RX IO	TX IO	CTS	    RTS
 * UART0	GPIO3	GPIO1	N/A	    N/A
 * UART1	GPIO9	GPIO10	GPIO6	GPIO11
 * UART2	GPIO16	GPIO17	GPIO8	GPIO7
 * source : https://circuits4you.com/2018/12/31/esp32-hardware-serial2-example/
 * ----------------------------------------
 * Wiring :
 *  ESP32           Mega
 *  GPIO16 (RX) ->  D11 (TX)
 *  GPIO17 (TX) ->  D10 (RX)
 *
 * Author: Shanmugathas Vigneswaran
 * email: shanmugathas.vigneswaran@outlook.fr
 * */

#include <ArduinoJson.h>

#define RXD2 16
#define TXD2 17

StaticJsonDocument<200> doc;
char buffer[10];                        // Serial2/EXSerial buffer
// ----------------------------------   // Sensors
int water_level_cm;
int nutrient_level_cm;
int ph_downer_level_cm;
int ph_level;
int ph_level;
// ----------------------------------   // Actuator
bool water_pump_state;
bool nutrient_pump_state;
bool ph_downer_pump_state;
bool mixer_pump_state;
// ------------------------------------ // Exchange command ESP32-Mega
char CMD_ALIVE[2] = "A";
char CMD_READ_IO[3] = "IO";

void setup() {
    Serial.begin(9600);
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    Serial.println("Serial/Serial2 ok");
}

void waitMega() {
    /*
     *  Wait Mega response
     * */
    Serial.print("[Serial2] Waiting response ...");
    while (Serial2.available() <= 0) {
        delay(20);
        Serial.print('.');
    }
    Serial.println("");
}

bool checkMegaAlive() {
    /*
     *  Check if mega board is responding
     *  to Serial2 trigger
     *  TODO Implement ESP > MEGA reset if serial disconnected
     *
     * */
    Serial2.write(CMD_ALIVE, 2);
    delay(20);
    waitMega();
    Serial2.readBytes(buffer, 10);
    // Serial.println("[Serial2] alive=" + String(buffer));
    if (String(buffer) == String(CMD_ALIVE)) {
        return true;
    } else
        return false;
}

void readMegaIO() {
    /*
     *  Read sensors and actuators status
     *  buffer > JSON handled by ArduinoJson
     *
     * */
    Serial2.write(CMD_READ_IO, 3);
    delay(20)
    waitMega();
    Serial.print("[Serial2] Datas received : ");
    DeserializationError error = deserializeJson(doc, Serial2);
    serializeJsonPretty(doc, Serial);
    if (error) {
        Serial.print(F("[Serial2] deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    else:
        water_level_cm = doc["water_level_cm"];
        nutrient_level_cm = doc["nutrient_level_cm"];
        ph_downer_level_cm = doc["ph_downer_level_cm"];
        ph_level = doc["ph_level"];
        ph_level = doc["ph_level"];
        water_pump_state = doc["water_pump_state"];
        nutrient_pump_state = doc["nutrient_pump_state"];
        ph_downer_pump_state = doc["ph_downer_pump_state"];
        mixer_pump_state = doc["mixer_pump_state"];

}

void loop() {
    readMegaIO();
}