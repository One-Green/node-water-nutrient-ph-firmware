//
// Created by jkl on 02/11/2020.
//

#include "Arduino.h"
#include "OGIO.h"

int WaterLevelTriggerPin = 2;
int WaterLevelEchoPin = 3;

int NutrientLevelTriggerPin = 4;
int NutrientLevelEchoPin = 5;

int pHDownerLevelTriggerPin = 6;
int pHDownerLevelEchoPin = 7;

int pHSense = A0;
int TdsSensorPin = A1;

void OGIO::initR() {

    pinMode(WaterLevelTriggerPin, OUTPUT);
    pinMode(WaterLevelEchoPin, INPUT);

    pinMode(NutrientLevelTriggerPin, OUTPUT);
    pinMode(NutrientLevelEchoPin, INPUT);

    pinMode(pHDownerLevelTriggerPin, OUTPUT);
    pinMode(pHDownerLevelEchoPin, INPUT);

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

    // TODO clean sampling/ analog read stuff
    int VREF = 5.0; // analog reference voltage(Volt) of the ADC
    int SCOUNT = 30;
    int analogBuffer[SCOUNT]; // store the analog value in the array, read from ADC
    int analogBufferTemp[SCOUNT];
    int analogBufferIndex = 0, copyIndex = 0;
    float averageVoltage = 0;
    float tdsValue = 0;
    float temperature = 25;
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
        float compensationVolatge = averageVoltage / compensationCoefficient; //temperature compensation
        tdsValue = (
                           133.42 * compensationVolatge * compensationVolatge * compensationVolatge -
                           255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge
                   ) * 0.5; //convert voltage value to tds value

        return tdsValue;
    }
}

void OGIO::onWaterPump() {

}

void OGIO::offWaterPump() {

}

void OGIO::onNutrientPump() {

}

void OGIO::offNutrientPump() {

}

void OGIO::onPhDownerPump() {

}

void OGIO::offPhDownerPump() {

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