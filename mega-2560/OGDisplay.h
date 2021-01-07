#ifndef display
#define display

#if (ARDUINO >= 100)
#include "Arduino.h"
#include "SPI.h"
#include "WiFi.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ST7735.h"
#else

#include "WProgram.h"

#endif

class DisplayLib {

public:
    // constructor
    void initR();

    void drawtext(char *text, uint16_t color);

    void initWifi();

    void connectedWifi();

    void printHeader(char *wifiSsid, IPAddress ip, char *nodeType, char *nodeTag);

    String ip2Str(IPAddress ip);

    void printTemplate();

    void updateDisplay(int water_level_cm, int nutrient_level_cm, int ph_downer_level_cm,
                       int ph_level, int tds_level,
                       bool water_pump_state, bool nutrient_pump_state, bool ph_downer_pump_state,
                       bool mixer_pump_state,
                       int ph_min, int ph_max, int tds_min, int tds_max
    );

    void uptime();


private:

};


#endif
