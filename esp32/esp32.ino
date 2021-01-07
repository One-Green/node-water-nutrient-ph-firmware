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
#include "SerialBridgeConf.h"
#include "SerialEndpoint.h"
// -----------------------------------  // Node parameters
char *NODE_TYPE = "water";
char *NODE_TAG = "water";

// -----------------------------------  // Wifi parameters
char *WIFI_SSID = "*";
char *WIFI_PASSWORD = "*";

// -----------------------------------  // MQTT parameters
char *MQTT_SERVER = "010e7d5e-3a86-4c87-a4bc-8d7a82bf5d2e.nodes.k8s.fr-par.scw.cloud";
char *MQTT_USER = "admin";
char *MQTT_PASSWORD = "admin";
int MQTT_PORT = 30180;
char *SENSOR_TOPIC = "water/sensor";
char *SENSOR_CONTROLLER = "water/controller";
unsigned long pubSensorTimer;
const int sensorPubRateSec = 10; //send sensor values each 10 sec
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
char buffer[10];                        // Serial2/EXSerial buffer

// ----------------------------------   // Sensors
int water_level_cm;
int nutrient_level_cm;
int ph_downer_level_cm;
int ph_level;
int tds_level;

// ----------------------------------   // Actuator
bool last_water_pump_state = false;
bool last_nutrient_pump_state = false;
bool last_ph_downer_pump_state = false;
bool last_mixer_pump_state = false;

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

// ---------------------------------- // Class(es) instantiation(s)
WiFiClient espClient;
PubSubClient client(espClient);
DisplayLib displayLib;


void setup() {

    /* Serial Ports Init */
    Serial.begin(9600);
    Serial.println("Serial/Serial2 ok");

    /* Serial Bridge Init */
    SerialEndpoint.begin();

    /* Display Init */
    displayLib.initR();
    displayLib.initWifi();

    /* Connect to WiFi */
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

    /*Display WiFi Info */
    displayLib.printHeader(WIFI_SSID, WiFi.localIP(), NODE_TYPE, NODE_TAG);
    displayLib.printTemplate();

    /* MQTT connexion */
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);

    pubSensorTimer = millis();
}


void loop() {

    //reconnect MQTT Client if not connected
    if (!client.connected()) {
        reconnect_mqtt();
    }

    client.loop();

    SerialEndpoint.loop();
    
    if (client.connected() && (millis() - pubSensorTimer > sensorPubRateSec))
    {
        //get sensors values from mega
        readAllMegaSensors();
        //send sensors to MQTT Broker
        pubSensorsVals();
        //update timer
        pubSensorTimer = millis();
    }


    // update TFT screen
    displayLib.updateDisplay(water_level_cm, nutrient_level_cm, ph_downer_level_cm,
                             ph_level, tds_level, last_water_pump_state, last_nutrient_pump_state,
                             last_ph_downer_pump_state, last_mixer_pump_state,
                             ctl_ph_level_min, ctl_ph_level_max, ctl_tds_level_min, ctl_tds_level_max
    );

    delay(300);
}


/* MQTT Functions */

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
        if (client.connect(clt_name_char, MQTT_USER, MQTT_PASSWORD)) {
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
    /*
     * Read JSON from controller through MQTT
     * and parse to Arduino datatype
     *
     * */
    Serial.print("[MQTT] Receiving << on topic: ");
    Serial.print(topic);
    Serial.print(". JSON message: ");
    String tmpMessage;

    for (int i = 0; i < length; i++) {
        tmpMessage += (char) message[i];
    }

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, tmpMessage);
    JsonObject obj = doc.as<JsonObject>();
    serializeJsonPretty(doc, Serial);
    Serial.println();

    /* Parse params from MQTT Payload*/

    //Parsing Tag param
    String tag = obj[String("tag")];

    //Parsing Water Pumps param
    ctl_water_pump = obj[String("water_pump_signal")];
    //Parsing Nutrient Pumps param
    ctl_nutrient_pump = obj[String("nutrient_pump_signal")];
    //Parsing PH Downer Pumps param
    ctl_ph_downer_pump = obj[String("ph_downer_pump_signal")];

    // TODO: how to handle mixer ?
    // ctl_mixer_pump = obj[String("mixer_pump")];

    ctl_ph_level_min = obj[String("ph_min_level")];
    ctl_ph_level_max = obj[String("ph_max_level")];
    ctl_tds_level_min = obj[String("tds_min_level")];
    ctl_tds_level_max = obj[String("tds_max_level")];

    // TODO : add actuator flow below
    if (tag == NODE_TAG) {
        // Handle water pump
        if (ctl_water_pump != last_water_pump_state) {
            last_water_pump_state = ctl_water_pump;
            /*
                TODO:Logic to Implement: ask pump state from mega and it's diferent, update
            */
            if (ctl_water_pump) {
                while (readMegaBool(CMD_ON_WATER_PUMP) != 1) {
                    readMegaBool(CMD_ON_WATER_PUMP);
                    Serial.println("[I/O] Waiting for water pump activation");
                }
                Serial.println("[I/O] Water pump is OPENED");
            } else {
                while (readMegaBool(CMD_OFF_WATER_PUMP) != 0) {
                    readMegaBool(CMD_OFF_WATER_PUMP);
                    Serial.println("[I/O] Waiting for water pump closing");
                }
                Serial.println("[I/O] Water pump is CLOSED");
            }
        }
        // handle nutrient pump
        if (ctl_nutrient_pump != last_nutrient_pump_state) {
            last_nutrient_pump_state = ctl_nutrient_pump;
            if (last_nutrient_pump_state) {
                while (readMegaBool(CMD_ON_NUTRIENT_PUMP) != 1) {
                    readMegaBool(CMD_ON_NUTRIENT_PUMP);
                    Serial.println("[I/O] Waiting for nutrient pump activation");
                }
                Serial.println("[I/O] Nutrient pump is OPENED");
            } else {
                while (readMegaBool(CMD_OFF_NUTRIENT_PUMP) != 0) {
                    readMegaBool(CMD_OFF_NUTRIENT_PUMP);
                    Serial.println("[I/O] Waiting for nutrient pump closing");
                }
                Serial.println("[I/O] Nutrient pump is CLOSED");
            }
        }

        // handle pH downer pump
        if (ctl_nutrient_pump != last_ph_downer_pump_state) {
            last_ph_downer_pump_state = ctl_nutrient_pump;
            if (last_ph_downer_pump_state) {
                while (readMegaBool(CMD_ON_PH_DOWNER_PUMP) != 1) {
                    readMegaBool(CMD_ON_PH_DOWNER_PUMP);
                    Serial.println("[I/O] Waiting for pH downer pump activation");
                }
                Serial.println("[I/O] pH downer pump is OPENED");
            } else {
                while (readMegaBool(CMD_OFF_PH_DOWNER_PUMP) != 0) {
                    readMegaBool(CMD_OFF_PH_DOWNER_PUMP);
                    Serial.println("[I/O] Waiting for pH downer pump closing");
                }
                Serial.println("[I/O] pH downer pump is CLOSED");
            }
        }

    }
}

void pubSensorsVals()
{
    String line_proto = generateInfluxLineProtocol();
    Serial.println(line_proto);
    // convert string to char and publish to mqtt
    int line_proto_len = line_proto.length() + 1;
    char line_proto_char[line_proto_len];
    line_proto.toCharArray(line_proto_char, line_proto_len);
    client.publish(SENSOR_TOPIC, line_proto_char);
}

/* Sensors Payload Builder */
String generateInfluxLineProtocol() {
    /*
     * Format sensors values for InfluxDB/Line protocol
     * */
    String lineProtoStr =
            "water,tag=" + String(NODE_TAG)
            + " water_level_cm=" + String(water_level_cm) + "i,"
            + "nutrient_level_cm=" + String(nutrient_level_cm) + "i,"
            + "ph_downer_level_cm=" + String(ph_downer_level_cm) + "i,"
            + "ph_level=" + String(ph_level) + "i,"
            + "tds_level=" + String(tds_level) + "i";

    return lineProtoStr;
}

/* Serial Bridge Functions */

/* Get All Sensor Values from Arduino Mega*/
void readAllMegaSensors()
{
    uint16_t  sensorArr[5];
    SerialEndpoint.getAllSensorsValues(sensorArr);

    water_level_cm = sensorArr[0];
    nutrient_level_cm = sensorArr[1];
    ph_downer_level_cm = sensorArr[2];
    ph_level = (sensorArr[3] / 100.00);
    tds_level = (sensorArr[4] / 100.00);
}