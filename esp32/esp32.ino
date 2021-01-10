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
char *WIFI_SSID = "Ciau";
char *WIFI_PASSWORD = "EaSyNeTMilk2";

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
bool ctl_water_pump = false;
bool ctl_nutrient_pump = false;
bool ctl_ph_downer_pump = false;
bool ctl_mixer_pump = false;
int ctl_ph_level_min = 0;
int ctl_ph_level_max = 0;
int ctl_tds_level_min = 0;
int ctl_tds_level_max = 0;

// ----------------------------------   // Sensors Regs Values
int water_level_cm = 0;
int nutrient_level_cm = 0;
int ph_downer_level_cm = 0;
int ph_level = 0;
int tds_level = 0;

// ----------------------------------   // Actuators
bool last_water_pump_state = false;
bool last_nutrient_pump_state = false;
bool last_ph_downer_pump_state = false;
bool last_mixer_pump_state = false;
//pumps id for serial bridge
#define WATER_PUMP_ID 1
#define NUTRIENT_PUMP_ID 2
#define PH_DOWNER_PUMP_ID 3
#define MIXER_PUMP_ID 4

// ---------------------------------- // Class(es) instantiation(s)
WiFiClient espClient;
PubSubClient client(espClient);
DisplayLib displayLib;


void setup() {

    /* Serial Ports Init */
    Serial.begin(9600);
    Serial.println("Serial Begin OK");

    /* Serial Bridge Init */
    Serial2.begin(BRIDGE_BAUD);
    SerialEndpoint.begin();
    Serial.println("Serial Bridge Begin OK");

    /* Display Init */
    displayLib.initR();
    displayLib.initWifi();

    /* Connect to WiFi */
    connectToWiFiNetwork();

    /*Display WiFi Info */
    displayLib.printHeader(WIFI_SSID, WiFi.localIP(), NODE_TYPE, NODE_TAG);
    displayLib.printTemplate();

    /* MQTT connexion */
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);

    /* Init MQTT Pub Sensor Timer */
    pubSensorTimer = millis();
}


void loop() {

    //reconnect MQTT Client if not connected
    if (!client.connected()) {
        reconnect_mqtt();
    }

    client.loop();

    SerialEndpoint.loop();
    
    //Pub sensors every 10 secs and only if the client is connected
    if (client.connected() && (millis() - pubSensorTimer > (sensorPubRateSec * 1000)))
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

/* WiFi Functions */

void connectToWiFiNetwork()
{
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
    if (tag != NODE_TAG) return;
    
    // Handle water pump
    if (ctl_water_pump != last_water_pump_state) {
        last_water_pump_state = ctl_water_pump;
        if (ctl_water_pump) //turn-ON and confirm 
        { 
            while(!SerialEndpoint.setPumpState(WATER_PUMP_ID, 1)) //maybe add timeout
            {
                Serial.println("[I/O] Waiting for water pump activation");
            }
            Serial.println("[I/O] Water pump is OPENED");
        } 
        else //turn-OFF and confirm 
        {
            while (!SerialEndpoint.setPumpState(WATER_PUMP_ID, 0)) //maybe add timeout
            {
                Serial.println("[I/O] Waiting for water pump closing");
            }
            Serial.println("[I/O] Water pump is CLOSED");
        }
    }
    // handle nutrient pump
    if (ctl_nutrient_pump != last_nutrient_pump_state) {
        last_nutrient_pump_state = ctl_nutrient_pump;
        if (ctl_nutrient_pump) {
            while (!SerialEndpoint.setPumpState(NUTRIENT_PUMP_ID, 1)) //maybe add timeout
            {
                Serial.println("[I/O] Waiting for nutrient pump activation");
            }
            Serial.println("[I/O] Nutrient pump is OPENED");
        } else {
            while (!SerialEndpoint.setPumpState(NUTRIENT_PUMP_ID, 0)) //maybe add timeout
            {
                Serial.println("[I/O] Waiting for nutrient pump closing");
            }
            Serial.println("[I/O] Nutrient pump is CLOSED");
        }
    }

    // handle pH downer pump
    if (ctl_ph_downer_pump != last_ph_downer_pump_state) 
    {
        last_ph_downer_pump_state = ctl_ph_downer_pump;

        if (ctl_ph_downer_pump) 
        {
            while (!SerialEndpoint.setPumpState(PH_DOWNER_PUMP_ID, 1)) //maybe add timeout
            {
                Serial.println("[I/O] Waiting for pH downer pump activation");
            }
            Serial.println("[I/O] pH downer pump is OPENED");
        } 
        else
        {
            while (!SerialEndpoint.setPumpState(PH_DOWNER_PUMP_ID, 0)) //maybe add timeout
            {
                Serial.println("[I/O] Waiting for pH downer pump closing");
            }
            Serial.println("[I/O] pH downer pump is CLOSED");
        }
    }

    // handle mixer pump
    if (ctl_mixer_pump != last_mixer_pump_state)
    {
        last_mixer_pump_state = ctl_mixer_pump;

        if (ctl_mixer_pump)
        {
            while (!SerialEndpoint.setPumpState(MIXER_PUMP_ID, 1)) //maybe add timeout
            {
                Serial.println("[I/O] Waiting for mixer pump activation");
            }
            Serial.println("[I/O] mixer pump is OPENED");
        }
        else
        {
            while (!SerialEndpoint.setPumpState(MIXER_PUMP_ID, 0)) //maybe add timeout
            {
                Serial.println("[I/O] Waiting for mixer pump closing");
            }
            Serial.println("[I/O] mixer pump is CLOSED");
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
    //client.publish(SENSOR_TOPIC, line_proto_char);
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
    uint16_t * sensoArrPtr = NULL;
    if (!SerialEndpoint.getAllSensorsValues())
    {
      Serial.println("Getting sensors values : ERROR!");
      return;
    }
    sensoArrPtr = SerialEndpoint.getSensorsArr();
    Serial.println("Getting sensors values : OK!");
    water_level_cm = (int)*(sensoArrPtr++);
    nutrient_level_cm = (int)*(sensoArrPtr++);
    ph_downer_level_cm = (int)*(sensoArrPtr++);
    tds_level = (int)(*(sensoArrPtr++) / 100.00);
    ph_level = (int)(*(sensoArrPtr++) / 100.00);
}
