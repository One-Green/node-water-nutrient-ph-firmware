#include "SoftwareSerialDebug.h"
#include <Arduino.h>
#include "SerialBridgeConf.h"

SoftwareSerial debugPort (BRIDGE_RX_PIN, BRIDGE_TX_PIN);//RX,TX

void debugPortBegin(int baud)
{
    debugPort.begin(baud);
}
