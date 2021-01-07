#include "SerialEndpoint.h"

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

  //Master Device Commands Attends
  #ifdef DEVICE_MASTER
  //Attend get sensor value commands response
  if (self->attendGetSensorValueRes(msgType, data)) return;

  //Attend get pump status commands reponse
  if (self->attendGetPumpStateRes(msgType, data)) return;

  //Attend set pump status commands reponse
  if (self->attendSetPumpStateRes(msgType, data)) return;
  #endif

  //Slave Device Commands Attends
  #ifdef DEVICE_SLAVE
  //Attend Sensor Readings
  if (self->attendGetSensorValueReq(msgType)) return;

  //Attend Get Relay
  if (self->attendGetPumpStateReq(msgType)) return;

  //Attend Set Relay
  if (self->attendSetPumpStateReq(msgType, data))
  {
    self->sendConfirmation();
    return;
  } 
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
  slip.begin(115200, attendSerial);
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


// Master Device API Methods
#ifdef DEVICE_MASTER
void SerialEndpointClass::getSensorValueReq(uint8_t sensorCommand)
{
  this->sendCommand(sensorCommand);
  this->waitForBufferResponse();
}

void SerialEndpointClass::getPumpStateReq(uint8_t pumpCommand)
{
  this->sendCommand(pumpCommand);
  this->waitForBufferResponse();
}

void SerialEndpointClass::setPumpStateReq(uint8_t pumpCommand, uint8_t state)
{
  this->sendCommandValue8(pumpCommand, state);
  this->waitForBufferResponse();
}


bool SerialEndpointClass::attendGetSensorValueRes(uint8_t sensorCommand, char * buffData)
{
  bool status = false;
  uint16_t sensorVal = 0;
  sensorVal = this->parseValue16(buffData);
  if (sensorCommand == CMD_GET_WATER_LEVEL)
  {
    //TODO: process and send water level
    DEBUG_PORT.print("Water Level : ");
    DEBUG_PORT.println(sensorVal);
    status = true;
  }
  else if (sensorCommand == CMD_GET_NUTRIENT_LEVEL)
  {
    //TODO: process and send nutrient level
    DEBUG_PORT.print("Nutrient Level : ");
    DEBUG_PORT.println(sensorVal);
    status = true;
  }
  else if (sensorCommand == CMD_GET_PH_DOWNER_LEVEL)
  {
    //TODO: process and send ph downer level
    DEBUG_PORT.print("PH Downer Level : ");
    DEBUG_PORT.println(sensorVal);
    status = true;
  }
  else if (sensorCommand == CMD_GET_TDS)
  {
    //TODO: process and send tds val
    DEBUG_PORT.print("TDS Value : ");
    DEBUG_PORT.println((float)(sensorVal / 100.00));
    status = true;
  }
  else if (sensorCommand == CMD_GET_PH)
  {
    //TODO: process and send ph val
    DEBUG_PORT.print("PH Value : ");
    DEBUG_PORT.println((float)(sensorVal / 100.00));
    status = true;
  }
  this->clearPendingErrorFlag();
  return status;
}

bool SerialEndpointClass::attendGetPumpStateRes(uint8_t pumpCommand, char * buffData)
{
  bool status = false;
  uint8_t pumpState = buffData[1];
  if (pumpCommand == CMD_GET_WATER_PUMP_STATE)
  {
    //TODO: process water pump state
    DEBUG_PORT.print("Water Pump State : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_GET_NUTRIENT_PUMP_STATE)
  {
    //TODO: process nutrient pump state
    DEBUG_PORT.print("Nutrient Pump State : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_GET_PH_DOWNER_PUMP_STATE)
  {
    //TODO: process downer pump state
    DEBUG_PORT.print("Downer Pump State : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_GET_MIXER_PUMP_STATE)
  {
    //TODO: process mixer pump state
    DEBUG_PORT.print("Mixer Pump State : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
  this->clearPendingErrorFlag();
  return status;
}

bool SerialEndpointClass::attendSetPumpStateRes(uint8_t pumpCommand, char * buffData)
{
  bool status = false;
  uint8_t pumpState = buffData[1];
  if (pumpCommand == CMD_GET_WATER_PUMP_STATE)
  {
    //TODO: process water pump state
    DEBUG_PORT.print("Water Pump State - Confirmed : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_GET_NUTRIENT_PUMP_STATE)
  {
    //TODO: process nutrient pump state
    DEBUG_PORT.print("Nutrient Pump State - Confirmed : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_GET_PH_DOWNER_PUMP_STATE)
  {
    //TODO: process downer pump state
    DEBUG_PORT.print("Downer Pump State - Confirmed : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_GET_MIXER_PUMP_STATE)
  {
    //TODO: process mixer pump state
    DEBUG_PORT.print("Mixer Pump State - Confirmed : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
  this->clearPendingErrorFlag();
  return status; 
}

#endif

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
    //TODO: get water level
    status = true;
  }
  else if (sensorCommand == CMD_GET_NUTRIENT_LEVEL)
  {
    //TODO: get nutrient level
    status = true;
  }
  else if (sensorCommand == CMD_GET_PH_DOWNER_LEVEL)
  {
    //TODO: get ph downer level
    status = true;
  }
  else if (sensorCommand == CMD_GET_TDS)
  {
    //TODO: get tds val
    status = true;
  }
  else if (sensorCommand == CMD_GET_PH)
  {
    //TODO: get ph val
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
    //TODO: get water pump state
    status = true;
  }
   else if (pumpCommand == CMD_GET_NUTRIENT_PUMP_STATE)
  {
    //TODO: get nutrient pump state
    status = true;
  }
   else if (pumpCommand == CMD_GET_PH_DOWNER_PUMP_STATE)
  {
    //TODO: get downer pump state
    status = true;
  }
   else if (pumpCommand == CMD_GET_MIXER_PUMP_STATE)
  {
    //TODO: get mixer pump state
    status = true;
  }
  this->sendCommand8(pumpCommand, pumpState);
  return status;  
}

bool SerialEndpointClass::attendSetPumpStateReq(uint8_t pumpCommand, char * buffData)
{
  bool status = false;
  uint8_t pumpState = buffData[1];
  if (pumpCommand == CMD_SET_WATER_PUMP_STATE)
  {
    //TODO: set water pump state
    status = true;
  }
   else if (pumpCommand == CMD_SET_NUTRIENT_PUMP_STATE)
  {
    //TODO: set nutrient pump state
    status = true;
  }
   else if (pumpCommand == CMD_SET_DOWNER_PUMP_STATE)
  {
    //TODO: set downer pump state
    status = true;
  }
   else if (pumpCommand == CMD_SET_MIXER_PUMP_STATE)
  {
    //TODO: set mixer pump state
    status = true;
  }
  this->sendCommand8(pumpCommand, pumpState);
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
