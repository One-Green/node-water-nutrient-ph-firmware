#ifndef SERIAL_ENDPOINT_H
#define SERIAL_ENDPOINT_H

#include <Arduino.h>
#include "SerialBridgeConf.h"
#include "Slip.h"
#include "CRC.h"

#define SERIAL_BUFF_SIZE 254
#define LEN_INDEX 0
#define PLLEN_INDEX 5

#define DEFAULT_PAN 0x01

class SerialEndpointClass
{
private:
  Slip slip;
  unsigned long responseTimeoutTimer;
  uint8_t buffer[SERIAL_BUFF_SIZE];
  void sendCommand(uint8_t command);
  void sendCommandValue8(uint8_t command, uint8_t value); //for int and  bool values
  void sendCommandValue16(uint8_t command, uint16_t value); //for float and  uint16_t values

  uint8_t parseValue8(char * buff); //parse and get int or bool value 
  uint16_t parseValue16(char * buff); //parse and get uint16_t 

  /* Clear the error-flag when new command is received */
  void clearPendingErrorFlag();

   /**
   * Serial Bridge Response Validator. Check if there is an error or there is no response from slave side
   *
   * @param none.
   * @return none.
   */
  void waitForBufferResponse();

public:
  bool _hadError;
  bool waitingResponse;
  SerialEndpointClass();
  void begin();
  void loop();
  void sendAck();
  void sendNack();

  // Master Device API Methods
  #ifdef DEVICE_MASTER
  /* Get Sensor Value*/
  void getSensorValueReq(uint8_t sensorCommand);

  /* Get Pump State */
  void getPumpStateReq(uint8_t pumpCommand);

  /* Set Pump State */
  void setPumpStateReq(uint8_t pumpCommand, uint8_t state);

  /* Attend get sensor value commands : get sensor values */
  bool attendGetSensorValueRes(uint8_t sensorCommand, char * buffData);

  /* Attend get pump state commands : get relay state */
  bool attendGetPumpStateRes(uint8_t pumpCommand, char * buffData);

  /* Attend Set PUMP State commands : validate if process is success */
  bool attendSetPumpStateRes(uint8_t pumpCommand, char * buffData);

  #endif
  // Slave Device API Methods

  #ifdef DEVICE_SLAVE
  /* Send process confirmation */
  void sendConfirmation();

  /* Attend Get Sensor commands */
  bool attendGetSensorValueReq(uint8_t sensorCommand);

  /* Attend Get PUMP State commands */
  bool attendGetPumpStateReq(uint8_t pumpCommand);

  /* Attend Set PUMP State commands */
  bool attendSetPumpStateReq(uint8_t pumpCommand, char * buffData);
  #endif

  /**
   * Check if an error is ocurred trough Serial Bridge.
   * @param none.
   * @return true = error, false = no error.
   */
  bool hadError();
};

extern SerialEndpointClass SerialEndpoint;

#endif //SERIAL_ENDPOINT_H
