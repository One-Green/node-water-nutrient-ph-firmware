#include "Wire.h"

void setup(){
  Serial.begin(9600); 
  while(!Serial){} // Waiting for serial connection
 
  Serial.println();
  Serial.println("Start I2C scanner ...");
  Serial.print("\r\n");
  byte count = 0;
  
  Wire.begin();
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0)
      {
      Serial.print("Found I2C Device: ");
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println(")");
      count++;
      delay(1);
      }
  }
  Serial.print("\r\n");
  Serial.println("Finish I2C scanner");
  Serial.print("Found ");
  Serial.print(count, HEX);
  Serial.println(" Device(s).");
}

void loop() {
  String test = "test";
  Wire.beginTransmission(4);  // transmit to device #9 / transmission sur arduino 2 a l adresse 0x09 (=9 en decimale)
  Wire.write(test.c_str());                 // sends x / envoi de la valeur de x
  Wire.endTransmission();        // stop transmitting / arret de la transmission

  delay(300);
    
  }
