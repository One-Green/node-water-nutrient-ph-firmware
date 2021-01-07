#ifndef  SERIAL_BRIDGE_CONF_H
#define  SERIAL_BRIDGE_CONF_H
#include <Arduino.h>
#include "SoftwareSerialDebug.h"


/* Serial Bridge Max Response Timeout Def */
#define BRIDGE_RESP_TIMEOUT_MS 700

/* Serial Bridge Port, Baud and Pins Def */
#define BRIDGE_TX_PIN 11
#define BRIDGE_RX_PIN 10
#define BRIDGE_PORT debugPort
#define BRIDGE_BAUD 9600

#define DEBUG_PORT Serial//TODO: define DEBUG PORT

/* Communication Role Def*/

#define DEVICE_SLAVE //for slave device (sensor device)
// #define DEVICE_MASTER //for master device (gateway)

#endif
