#include <Arduino.h>
#include "OGIO.h"

OGIO io_handler;

void setup() {
    Serial.begin(9600);
    io_handler.initR();
}

void loop() {

    int water_level_cm = io_handler.getWaterLevelCM();
    int nutrient_level_cm = io_handler.getNutrientLevelCM();
    int ph_downer_level_cm = io_handler.getPhDownerLevelCM();

    float ph_level = io_handler.getPhLevel();
    float tds_ppm = io_handler.getTDS();

    Serial.print("Water level CM=");
    Serial.println(water_level_cm);

    Serial.print("Nutrient level CM=");
    Serial.println(nutrient_level_cm);

    Serial.print("ph Downer level CM=");
    Serial.println(ph_downer_level_cm);

    Serial.print("pH Level=");
    Serial.println(ph_level);

    Serial.print("TDS Level (ppm) =");
    Serial.println(tds_ppm);


}

