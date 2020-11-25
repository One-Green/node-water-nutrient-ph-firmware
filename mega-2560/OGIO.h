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

    void onWaterPump();
    void offWaterPump();
    void onNutrientPump();
    void offNutrientPump();
    void onPhDownerPump();
    void offPhDownerPump();
    void onMixerPump();
    void offMixerPump();

    int getWaterPumpStatus();
    int getNutrientPumpStatus();
    int getPhDownerPumpStatus();
    int getMixerPumpStatus();

    int getMedianNum(int bArray[], int iFilterLen);
    int baseUltrasonicReader(int trigger, int echo);
};


#endif //NODE_WATER_ARDUINO_OGIO_H
