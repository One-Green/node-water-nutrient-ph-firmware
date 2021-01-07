#ifndef  SERIAL_BRIDGE_CONF_H
#define  SERIAL_BRIDGE_CONF_H
#include <Arduino.h>



/* Serial Bridge Max Response Timeout Def */
#define BRIDGE_RESP_TIMEOUT_MS 700

/* Serial Bridge Port, Baud and Pins Def */
#define BRIDGE_TX_PIN TX
#define BRIDGE_RX_PIN RX
#define BRIDGE_PORT Serial2
#define BRIDGE_BAUD 9600

#define DEBUG_PORT Serial//TODO: define DEBUG PORT

/* Communication Role Def*/

#define DEVICE_SLAVE //for slave device (sensor device)
// #define DEVICE_MASTER //for master device (gateway)

#endif
