#include "SerialEndpoint.h"
#include "OGIO.h"
// #define TEST //sensor values simulation for serial bridge test
static SerialEndpointClass *self;
static uint8_t recBuffer[SERIAL_BUFF_SIZE];

/* Serial Bridge Commands

  - Get Commands Request Format  [CMD][CRC_LOW][CRC_HIGH]
  - Get Commands Response Format [CMD][VALUE][CRC_LOW][CRC_HIGH]


  - Set Commands Request Format  [CMD][VALUE][CRC_LOW][CRC_HIGH]
  - Set Commands Response Format [CMD][VALUE][CRC_LOW][CRC_HIGH]
*/

//Connections and Errors Control Commands
#define CMD_NACK                      0x00
#define CMD_ACK                       0x01
#define CMD_BAD_CRC                   0xFE
#define CMD_ERROR                     0xFF

//Get Commands
#define CMD_GET_WATER_LEVEL 0x02
#define CMD_GET_NUTRIENT_LEVEL 0x03
#define CMD_GET_PH_DOWNER_LEVEL 0x04
#define CMD_GET_TDS 0x05
#define CMD_GET_PH 0x06

#define CMD_GET_WATER_PUMP_STATE 0x07
#define CMD_GET_NUTRIENT_PUMP_STATE 0x08
#define CMD_GET_PH_DOWNER_PUMP_STATE 0x09
#define CMD_GET_MIXER_PUMP_STATE 0x0A

//Set Commands
#define CMD_SET_WATER_PUMP_STATE 0x0B
#define CMD_SET_NUTRIENT_PUMP_STATE 0x0C
#define CMD_SET_DOWNER_PUMP_STATE 0x0D
#define CMD_SET_MIXER_PUMP_STATE 0x0E


static void attendSerial(char *data, uint8_t size)
{
  // Data format here:
  // [command][data][crc][crc]
  // [command][crc][crc]
  // Attend
  if(size < 3) return self->sendNack(); // bad data


  if (!checkCrc(data, size)) // bad crc
  {
    self->sendNack();
    return;
  }

  size = size - 2; //update buffer size (without CRC)
  uint8_t msgType = data[0];

  //Slave Device Commands Attends
  #ifdef DEVICE_SLAVE
  //Attend Sensor Readings
  if (self->attendGetSensorValueReq(msgType)) return;

  //Attend Get Relay
  if (self->attendGetPumpStateReq(msgType)) return;

  //Attend Set Relay
  if (self->attendSetPumpStateReq(msgType, data))return;
   
  #endif
}

SerialEndpointClass::SerialEndpointClass()
{
  self = this;
  _hadError = false;
  waitingResponse = false;
  responseTimeoutTimer = millis();
}

//Private Methods
void SerialEndpointClass::sendCommand(uint8_t command)
{
  buffer[0] = command;
  uint8_t size = 1;
  size = appendCrc((char*)buffer, size);
  slip.send((char*)buffer, size);
}

void SerialEndpointClass::sendCommandValue8(uint8_t command, uint8_t value) //for int and  bool values
{
  buffer[0] = command;
  buffer[1] = value;
  uint8_t size = 2;
  size = appendCrc((char*)buffer, size);
  slip.send((char*)buffer, size);
}

void SerialEndpointClass::sendCommandValue16(uint8_t command, uint16_t value) //for float and  uint16_t values
{
  buffer[0] = command;
  buffer[1] = value & 0xFF; //value LOW Byte
  buffer[2] = (value >> 8) & 0xFF; //value HIGH Byte
  uint8_t size = 3;
  size = appendCrc((char*)buffer, size);// apend [crcLow][crcHigh]. New size = size + 2
  slip.send((char*)buffer, size);
}

uint8_t SerialEndpointClass::parseValue8(char * buff) //parse and get int or bool value 
{
  return buff[1];
}

uint16_t SerialEndpointClass::parseValue16(char * buff) //parse and get uint16_t 
{
  uint16_t val;
  val = buff[1] & 0x00FF;
  val |= ((uint16_t)buff[2] << 8) & 0xFF00;
  return val;
}

void SerialEndpointClass::clearPendingErrorFlag()
{
  if (this->waitingResponse)
  {
    this->waitingResponse = false;
    this->_hadError = false;
  }
}

void SerialEndpointClass::waitForBufferResponse()
{
  responseTimeoutTimer = millis();
  _hadError = false;
  waitingResponse = true;
  while(waitingResponse && (millis() - responseTimeoutTimer < BRIDGE_RESP_TIMEOUT_MS))
  {
    loop();
  }
  if(waitingResponse) _hadError = true;
  waitingResponse = false;
}

//Public Methods

void SerialEndpointClass::begin()
{
  memset(incomingSensorVal, 0, 5);
  memset(incomingPumpState, 0, 4);
  
  slip.begin(attendSerial);
  this->sendAck();  // Send ack for flushing any pending messages sent from the gateway before we were ready
}

void SerialEndpointClass::loop()
{
  slip.loop();
}

void SerialEndpointClass::sendAck()
{
  this->sendCommand(CMD_ACK);
}

void SerialEndpointClass::sendNack()
{
  this->sendCommand(CMD_NACK);
}


// Slave Device API Methods
#ifdef DEVICE_SLAVE
void SerialEndpointClass::sendConfirmation()
{
  this->sendNack();
}

bool SerialEndpointClass::attendGetSensorValueReq(uint8_t sensorCommand)
{
  bool status = false;
  uint16_t sensorVal = 0;
  if (sensorCommand == CMD_GET_WATER_LEVEL)
  {
    //get water level
    #ifdef TEST
    sensorVal = 10;
    #else
    sensorVal = io_handler.getWaterLevelCM();
    #endif
    status = true;
  }
  else if (sensorCommand == CMD_GET_NUTRIENT_LEVEL)
  {
    //get nutrient level
    #ifdef TEST
    sensorVal = 20;
    #else
    sensorVal = io_handler.getNutrientLevelCM();
    #endif
    status = true;
  }
  else if (sensorCommand == CMD_GET_PH_DOWNER_LEVEL)
  {
    //get ph downer level
    #ifdef TEST
    sensorVal = 30;
    #else
    sensorVal = io_handler.getPhDownerLevelCM();
    #endif
    status = true;
  }
  else if (sensorCommand == CMD_GET_TDS)
  {
    //get tds val
    #ifdef TEST
    sensorVal = (uint16_t)(99.56);
    #else
    sensorVal = (uint16_t)(io_handler.getTDS());
    #endif
    status = true;
  }
  else if (sensorCommand == CMD_GET_PH)
  {
    //get ph val
    #ifdef TEST
    sensorVal = (uint16_t)(50.5);
    #else
    sensorVal = (uint16_t)(io_handler.getPhLevel());
    #endif
    status = true;
  }
  /* Send sensor value */
  this->sendCommandValue16(sensorCommand, sensorVal);
  return status;
}

bool SerialEndpointClass::attendGetPumpStateReq(uint8_t pumpCommand)
{
  bool status = false;
  uint8_t pumpState = 0;
  if (pumpCommand == CMD_GET_WATER_PUMP_STATE)
  {
    //get water pump state
    pumpState = io_handler.getWaterPumpStatus();
    status = true;
  }
   else if (pumpCommand == CMD_GET_NUTRIENT_PUMP_STATE)
  {
    //get nutrient pump state
    pumpState = io_handler.getNutrientPumpStatus();
    status = true;
  }
   else if (pumpCommand == CMD_GET_PH_DOWNER_PUMP_STATE)
  {
    //get downer pump state
    pumpState = io_handler.getPhDownerPumpStatus();
    status = true;
  }
   else if (pumpCommand == CMD_GET_MIXER_PUMP_STATE)
  {
    //get mixer pump state
    pumpState = io_handler.getMixerPumpStatus();
    status = true;
  }
  this->sendCommandValue8(pumpCommand, pumpState);
  return status;  
}

bool SerialEndpointClass::attendSetPumpStateReq(uint8_t pumpCommand, char * buffData)
{
  bool status = false;
  uint8_t pumpState = buffData[1];
  if (pumpCommand == CMD_SET_WATER_PUMP_STATE)
  {
    //set water pump state
    Serial.print("set water pump state : ");
    Serial.println(pumpState);
    io_handler.setWaterPump(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_SET_NUTRIENT_PUMP_STATE)
  {
    //set nutrient pump state
    Serial.print("set nutrient pump state : ");
    Serial.println(pumpState);
    io_handler.setNutrientPump(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_SET_DOWNER_PUMP_STATE)
  {
    //set downer pump state
    Serial.print("set downer pump state : ");
    Serial.println(pumpState);    
    io_handler.setPHDownerPump(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_SET_MIXER_PUMP_STATE)
  {
    //set mixer pump state
    Serial.print("set mixer pump state : ");
    Serial.println(pumpState); 
    io_handler.setMixerPump(pumpState);
    status = true;
  }
  this->sendCommandValue8(pumpCommand, pumpState);
  BRIDGE_PORT.flush();
  return status;
}
#endif

bool SerialEndpointClass::hadError()
{
  bool res = _hadError;
  _hadError = false;
  return res;
}

SerialEndpointClass SerialEndpoint;
