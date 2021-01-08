//
// Created by shan on 06/11/2020.
//

#ifndef NODE_WATER_ARDUINO_OGIO_H
#define NODE_WATER_ARDUINO_OGIO_H


class OGIO {

public:
    void initR();

    int getWaterLevelCM();
    int getNutrientLevelCM();
    int getPhDownerLevelCM();
    float getPhLevel();
    float getTDS();

    void setWaterPump(uint8_t state);
    void setNutrientPump(uint8_t state);
    void setPHDownerPump(uint8_t state);
    void setMixerPump(uint8_t state);

    int getWaterPumpStatus();
    int getNutrientPumpStatus();
    int getPhDownerPumpStatus();
    int getMixerPumpStatus();

    int getMedianNum(int bArray[], int iFilterLen);
    int baseUltrasonicReader(int trigger, int echo);
};

extern OGIO io_handler;


#endif //NODE_WATER_ARDUINO_OGIO_H
