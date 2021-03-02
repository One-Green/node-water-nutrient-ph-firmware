//
// Created by jkl on 02/11/2020.
//

#include "Arduino.h"
#include "OGIO.h"

// ----------------------------  // Sensors
int WaterLevelTriggerPin = 2;
int WaterLevelEchoPin = 3;

int NutrientLevelTriggerPin = 4;
int NutrientLevelEchoPin = 5;

int pHDownerLevelTriggerPin = 6;
int pHDownerLevelEchoPin = 7;

#define pHSense A0
#define TdsSensorPin A1
// ----------------------------  // Actuators

int WaterPumpPin = 22;
int NutrientPumpPin = 23;
int phDownerPumpPin = 24;
int MixerPumpPin = 25;

void OGIO::initR() {

    pinMode(WaterLevelTriggerPin, OUTPUT);
    pinMode(WaterLevelEchoPin, INPUT);

    pinMode(NutrientLevelTriggerPin, OUTPUT);
    pinMode(NutrientLevelEchoPin, INPUT);

    pinMode(pHDownerLevelTriggerPin, OUTPUT);
    pinMode(pHDownerLevelEchoPin, INPUT);

    pinMode(WaterPumpPin, OUTPUT);
    digitalWrite(WaterPumpPin, LOW);

    pinMode(NutrientPumpPin, OUTPUT);
    digitalWrite(NutrientPumpPin, LOW);

    pinMode(phDownerPumpPin, OUTPUT);
    digitalWrite(phDownerPumpPin, LOW);

    pinMode(MixerPumpPin, OUTPUT);
    digitalWrite(MixerPumpPin, LOW);

    pinMode(pHSense, INPUT);
    pinMode(TdsSensorPin, INPUT);
}


int OGIO::getWaterLevelCM() {
    return baseUltrasonicReader(WaterLevelTriggerPin, WaterLevelEchoPin);
}

int OGIO::getNutrientLevelCM() {
    return baseUltrasonicReader(NutrientLevelTriggerPin, NutrientLevelEchoPin);
}

int OGIO::getPhDownerLevelCM() {
    return baseUltrasonicReader(pHDownerLevelTriggerPin, pHDownerLevelEchoPin);
}

float OGIO::getPhLevel() {

    int samples = 10;
    float adc_resolution = 1024.0;
    int measurings = 0;
    for (int i = 0; i < samples; i++) {
        measurings += analogRead(pHSense);
        delay(10);
    }
    float voltage = 5 / adc_resolution * measurings / samples;
    float ph = 7 + ((2.5 - voltage) / 0.18);

    return ph;
}

float OGIO::getTDS() {
    /*
     *  source https://wiki.keyestudio.com/KS0429_keyestudio_TDS_Meter_V1.0
     *
     * */
    static unsigned long analogSampleTimepoint = millis();
    if (millis() - analogSampleTimepoint > 40U) //every 40 milliseconds,read the analog value from the ADC
    {
        analogSampleTimepoint = millis();
        analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); //read the analog value and store into the buffer
        analogBufferIndex++;
        if (analogBufferIndex == SCOUNT)
            analogBufferIndex = 0;
    }
    static unsigned long printTimepoint = millis();
    if (millis() - printTimepoint > 800U) {
        printTimepoint = millis();
        for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
            analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float) VREF /
                         1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
        float compensationCoefficient = 1.0 + 0.02 * (temperature -
                                                      25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
        float compensationVoltage = averageVoltage / compensationCoefficient; //temperature compensation
        tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage -
                    255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) *
                   0.5; //convert voltage value to tds value
                // Serial.print("TDS Value:");
                // Serial.print(tdsValue, 0);
                // Serial.println("ppm");
        return tdsValue;
    }


}

void OGIO::setWaterPump(uint8_t state) {
    digitalWrite(WaterPumpPin, state);
}

void OGIO::setNutrientPump(uint8_t state) {
    digitalWrite(NutrientPumpPin, state);
}

void OGIO::setPHDownerPump(uint8_t state) {
    digitalWrite(phDownerPumpPin, state);
}

void OGIO::setMixerPump(uint8_t state) {
    digitalWrite(MixerPumpPin, state);
}


int OGIO::getWaterPumpStatus() {
    return digitalRead(WaterPumpPin);
}

int OGIO::getNutrientPumpStatus() {
    return digitalRead(NutrientPumpPin);
}

int OGIO::getPhDownerPumpStatus() {
    return digitalRead(phDownerPumpPin);
}

int OGIO::getMixerPumpStatus() {
    return digitalRead(MixerPumpPin);
}

int OGIO::getMedianNum(int bArray[], int iFilterLen) {
    /*
     *  source https://wiki.keyestudio.com/KS0429_keyestudio_TDS_Meter_V1.0
     *
     * */

    int bTab[iFilterLen];
    for (byte i = 0; i < iFilterLen; i++)
        bTab[i] = bArray[i];
    int i, j, bTemp;
    for (j = 0; j < iFilterLen - 1; j++) {
        for (i = 0; i < iFilterLen - j - 1; i++) {
            if (bTab[i] > bTab[i + 1]) {
                bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }
    if ((iFilterLen & 1) > 0)
        bTemp = bTab[(iFilterLen - 1) / 2];
    else
        bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
    return bTemp;
}

int OGIO::baseUltrasonicReader(int trigger, int echo) {
    long duration;
    int distance;
    // Clear the trigPin by setting it LOW:
    digitalWrite(trigger, LOW);
    delayMicroseconds(5);
    // Trigger the sensor by setting the trigPin high for 10 microseconds:
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);
    // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
    duration = pulseIn(echo, HIGH);
    // Calculate the distance:
    distance = duration * 0.034 / 2;
    return distance;
}

OGIO io_handler;