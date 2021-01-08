#ifndef SOFTWARE_SERIAL_DEBUG_H
#define SOFTWARE_SERIAL_DEBUG_H
#include <Arduino.h>
#include <SoftwareSerial.h>

extern SoftwareSerial debugPort;
extern void debugPortBegin(int baud);
#endif //SOFTWARE_SERIAL_DEBUG_H
