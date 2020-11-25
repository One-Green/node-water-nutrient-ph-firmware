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
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include "OGDisplay.h"

// -----------------------------------  // MQTT parameters
char *NODE_TYPE = "water";
char *NODE_TAG = ":-)";                 // not required for node type = water
char *WIFI_SSID = "*";
char *WIFI_PASSWORD = "*";

// -----------------------------------  // MQTT parameters
char *MQTT_SERVER = "af120153-db6a-4fdd-a81b-6d902b00e936.nodes.k8s.fr-par.scw.cloud";
int MQTT_PORT = 32500;
char *SENSOR_TOPIC = "water/sensor";
char *SENSOR_CONTROLLER = "water/controller";
// Parameter from Master, provided by MQTT JSON
bool ctl_water_pump;
bool ctl_nutrient_pump;
bool ctl_ph_downer_pump;
bool ctl_mixer_pump;
int ctl_ph_level_min;
int ctl_ph_level_max;
int ctl_tds_level_min;
int ctl_tds_level_max;

// ----------------------------------   // Communication ESP32 - MEGA 2560
#define RXD2 16
#define TXD2 17
StaticJsonDocument<200> doc;            // Json
char buffer[10];                        // Serial2/EXSerial buffer

// ----------------------------------   // Sensors
int water_level_cm;
int nutrient_level_cm;
int ph_downer_level_cm;
int ph_level;
int tds_level;

// ----------------------------------   // Actuator
bool water_pump_state;
bool nutrient_pump_state;
bool ph_downer_pump_state;
bool mixer_pump_state;

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

// ---------------------------------- // Class(es) instantiation(s)
WiFiClient espClient;
PubSubClient client(espClient);
DisplayLib displayLib;


void setup() {
    Serial.begin(9600);
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    Serial.println("Serial/Serial2 ok");

    displayLib.initR();
    displayLib.initWifi();
    Serial.print("[WIFI] Connecting to ");
    Serial.print(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("\n");
    displayLib.connectedWifi();
    Serial.println("[WIFI] Connected ");
    Serial.print("[WIFI] IP address: ");
    Serial.println(WiFi.localIP());
    displayLib.printHeader(WIFI_SSID, WiFi.localIP(), NODE_TYPE, NODE_TAG);
    displayLib.printTemplate();

    // MQTT connexion
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);
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

bool readMegaIO() {
    /*
     *  Read sensors and actuators status
     *  buffer > JSON handled by ArduinoJson
     *  return true if successfully read
     * */
    Serial2.write(CMD_READ_IO, 3);
    delay(20);
    waitMega();
    Serial.print("[Serial2] Datas received : ");
    DeserializationError error = deserializeJson(doc, Serial2);
    serializeJsonPretty(doc, Serial);
    if (error) {
        Serial.print(F("[Serial2] deserializeJson() failed: "));
        Serial.println(error.f_str());
        return false;
    } else {
        water_level_cm = doc["water_level_cm"];
        nutrient_level_cm = doc["nutrient_level_cm"];
        ph_downer_level_cm = doc["ph_downer_level_cm"];
        ph_level = doc["ph_level"];
        tds_level = doc["ph_level"];
        water_pump_state = doc["water_pump_state"];
        nutrient_pump_state = doc["nutrient_pump_state"];
        ph_downer_pump_state = doc["ph_downer_pump_state"];
        mixer_pump_state = doc["mixer_pump_state"];
        return true;
    }
}


void reconnect_mqtt() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("[MQTT] Attempting connection... with client name = ");
        String client_name = String(NODE_TYPE) + "-" + String(NODE_TYPE);
        int clt_len = client_name.length() + 1;
        char clt_name_char[clt_len];
        client_name.toCharArray(clt_name_char, clt_len);
        Serial.println(clt_name_char);

        // Attempt to connect
        if (client.connect(clt_name_char)) {
            Serial.println("[MQTT] Client connected");
            // Subscribe
            client.subscribe(SENSOR_CONTROLLER);
        } else {
            Serial.print("[MQTT] failed, rc=");
            Serial.print(client.state());
            Serial.println("[MQTT] try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}


void mqttCallback(char *topic, byte *message, unsigned int length) {
    Serial.print("[MQTT] Receiving << on topic: ");
    Serial.print(topic);
    Serial.print(". JSON message: ");
    String messageTemp;

    for (int i = 0; i < length; i++) {
        Serial.print((char) message[i]);
        messageTemp += (char) message[i];
    }
    Serial.println();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, messageTemp);
    JsonObject obj = doc.as<JsonObject>();

    ctl_water_pump = obj[String("ctl_water_pump")];
    ctl_nutrient_pump = obj[String("ctl_nutrient_pump")];
    ctl_ph_downer_pump = obj[String("ctl_ph_downer_pump")];
    ctl_mixer_pump = obj[String("ctl_mixer_pump")];
    ctl_ph_level_min = obj[String("ctl_ph_level_min")];
    ctl_ph_level_max = obj[String("ctl_ph_level_max")];
    ctl_tds_level_min = obj[String("ctl_tds_level_min")];
    ctl_tds_level_max = obj[String("ctl_tds_level_max")];

    // TODO : add actuator flow below

}

String generateInfluxLineProtocol() {
    String lineProtoStr =
            "water,tag=" + String(NODE_TAG)
            + " water_level_cm=" + String(water_level_cm) + "i,"
            + " nutrient_level_cm=" + String(nutrient_level_cm) + "i,"
            + " ph_downer_level_cm=" + String(ph_downer_level_cm) + "i,"
            + " ph_level=" + String(ph_level) + "i,"
            + " tds_level=" + String(tds_level) + "i" ;

    return lineProtoStr;
}


void loop() {
    // reconnect MQTT Client if not connected
    if (!client.connected()) {
        reconnect_mqtt();
    }
    client.loop();


    if (readMegaIO()) {
        String line_proto = generateInfluxLineProtocol();
        Serial.println(line_proto);
        // convert string to char and publish to mqtt
        int line_proto_len = line_proto.length() + 1;
        char line_proto_char[line_proto_len];
        line_proto.toCharArray(line_proto_char, line_proto_len);
        client.publish(SENSOR_TOPIC, line_proto_char);
    };
}
