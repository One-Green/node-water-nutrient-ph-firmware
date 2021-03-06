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

  //Attend set pump status commands reponse
  if (self->attendSetPumpStateRes(msgType, data)) return;
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
  uint8_t pumpCommandFinal = CMD_SET_WATER_PUMP_STATE;
  switch (pumpCommand)
  {
  case 1:
    pumpCommandFinal = CMD_SET_WATER_PUMP_STATE;
    break;
  case 2:
    pumpCommandFinal = CMD_SET_NUTRIENT_PUMP_STATE;
    break;
  case 3:
    pumpCommandFinal = CMD_SET_DOWNER_PUMP_STATE;
    break;
  case 4:
    pumpCommandFinal = CMD_SET_MIXER_PUMP_STATE;
    break;
  }
  this->sendCommandValue8(pumpCommandFinal, state);
  this->waitForBufferResponse();
}


bool SerialEndpointClass::attendGetSensorValueRes(uint8_t sensorCommand, char * buffData)
{
  bool status = false;
  uint16_t sensorVal = 0;
  sensorVal = this->parseValue16(buffData);
  if (sensorCommand == CMD_GET_WATER_LEVEL)
  {
    incomingSensorVal[0] = sensorVal;
    DEBUG_PORT.print("RX -> Water Level : ");
    DEBUG_PORT.println(incomingSensorVal[0]);
    status = true;
  }
  else if (sensorCommand == CMD_GET_NUTRIENT_LEVEL)
  {
    incomingSensorVal[1] = sensorVal;
    DEBUG_PORT.print("RX -> Nutrient Level : ");
    DEBUG_PORT.println(incomingSensorVal[1]);
    status = true;
  }
  else if (sensorCommand == CMD_GET_PH_DOWNER_LEVEL)
  {
    incomingSensorVal[2] = sensorVal;
    DEBUG_PORT.print("RX -> PH Downer Level : ");
    DEBUG_PORT.println(incomingSensorVal[2]);
    status = true;
  }
  else if (sensorCommand == CMD_GET_TDS)
  {
    incomingSensorVal[3] = sensorVal;
    DEBUG_PORT.print("RX -> TDS Value : ");
    DEBUG_PORT.println((float)(incomingSensorVal[3]));
    status = true;
  }
  else if (sensorCommand == CMD_GET_PH)
  {
    incomingSensorVal[4] = sensorVal;
    DEBUG_PORT.print("RX -> PH Value : ");
    DEBUG_PORT.println((float)(incomingSensorVal[4]));
    status = true;
  }
  this->clearPendingErrorFlag();
  return status;
}

bool SerialEndpointClass::attendSetPumpStateRes(uint8_t pumpCommand, char * buffData)
{
  bool status = false;
  uint8_t pumpState = buffData[1];
  if (pumpCommand == CMD_SET_WATER_PUMP_STATE)
  {
    incomingPumpState[0] = pumpState;
    DEBUG_PORT.print("RX -> Water Pump State - Confirmed : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_SET_NUTRIENT_PUMP_STATE)
  {
    incomingPumpState[1] = pumpState;
    DEBUG_PORT.print("RX -> Nutrient Pump State - Confirmed : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_SET_DOWNER_PUMP_STATE)
  {
    incomingPumpState[2] = pumpState;
    DEBUG_PORT.print("RX -> Downer Pump State - Confirmed : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
   else if (pumpCommand == CMD_SET_MIXER_PUMP_STATE)
  {
    incomingPumpState[3] = pumpState;
    DEBUG_PORT.print("RX -> Mixer Pump State - Confirmed : ");
    DEBUG_PORT.println(pumpState);
    status = true;
  }
  this->clearPendingErrorFlag();
  BRIDGE_PORT.flush();
  return status; 
}

bool SerialEndpointClass::getAllSensorsValues()
{
  this->getSensorValueReq(CMD_GET_WATER_LEVEL);
  if (this->hadError()) return false; //maybe return false
  delay(10);

  this->getSensorValueReq(CMD_GET_NUTRIENT_LEVEL);
  if (this->hadError()) return false; //maybe return false
  delay(10);

  this->getSensorValueReq(CMD_GET_PH_DOWNER_LEVEL);
  if (this->hadError()) return false; //maybe return false
  delay(10);

  this->getSensorValueReq(CMD_GET_TDS);
  if (this->hadError()) return false; //maybe return false
  delay(10);

  this->getSensorValueReq(CMD_GET_PH);
  if (this->hadError()) return false; //maybe return false
  
  return true;
}

uint16_t * SerialEndpointClass::getSensorsArr()
{
  return incomingSensorVal;
}

bool SerialEndpointClass::setPumpState(uint8_t pumpId, uint8_t state)
{
  this->setPumpStateReq(pumpId, state);
  if (this->hadError())
  {
    Serial.println("Getting relay Status : ERROR");
    return false;
  }
  Serial.println(incomingPumpState[pumpId - 1]);
  return (incomingPumpState[pumpId - 1] == state);
}
#endif

bool SerialEndpointClass::hadError()
{
  bool res = _hadError;
  _hadError = false;
  return res;
}

SerialEndpointClass SerialEndpoint;
