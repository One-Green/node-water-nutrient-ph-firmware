#include "Arduino.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ST7735.h"
#include "OGDisplay.h"

#define TFT_CS   15
#define TFT_RST  4
#define TFT_DC   2
#define TFT_MOSI 23
#define TFT_SCLK 18
// not required
// #define TFT_MISO 12

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);


void DisplayLib::initR() {

	tft.initR(INITR_BLACKTAB);
	tft.fillScreen(ST7735_BLACK);
	delay(500);
	// large block of text
	tft.fillScreen(ST7735_BLACK);
	drawtext("Screen initialised", ST7735_WHITE);

}


void DisplayLib::initWifi() {
	drawtext("Connecting to WIFI", ST7735_WHITE);
}


void DisplayLib::connectedWifi() {
	drawtext("Connected to WIFI", ST7735_WHITE);
}


void DisplayLib::printHeader(
		char *wifiSsid,
		IPAddress ip,
		char *nodeType,
		char *nodeTag) {
	tft.fillRect(0, 0, 128, 50, ST7735_WHITE);
	tft.fillRect(0, 50, 128, 160, ST7735_GREEN);
	tft.setTextColor(ST7735_BLACK);

	// print wifi SSID
	tft.setCursor(2, 2);
	tft.print("WIFI: ");
	tft.setCursor(35, 2);
	String ssid = String(wifiSsid);
	if (ssid.length() > 15) {
		ssid = ssid.substring(0, 12) + "...";
	}
	tft.print(ssid);

	// print IP
	tft.setCursor(2, 10);
	tft.print("IP: " + ip2Str(ip));

	// print node type
	tft.setCursor(2, 20);
	tft.print("Node Type: ");
	tft.print(nodeType);

	// print node tag
	tft.setCursor(2, 30);
	tft.print("Node Tag: ");
	tft.print(nodeTag);
}


void DisplayLib::printTemplate() {
	/*
	 *  Print static on ST7735
	 *
	 * */
	tft.setCursor(2, 50);
	tft.print("Water tk (cm): ");
	tft.setCursor(2, 60);
	tft.print("Nutr. tk (cm): ");
	tft.setCursor(2, 70);
	tft.print("pH tk (cm): ");
	tft.setCursor(2, 80);
	tft.print("pH: ");
	tft.setCursor(2, 90);
	tft.print("TDS: ");
    tft.setCursor(2, 100);
    tft.print("Water p: ");
    tft.setCursor(2, 110);
    tft.print("Nutr. p: ");
    tft.setCursor(2, 120);
    tft.print("ph down p: ");
    tft.setCursor(2, 130);
    tft.print("Mixer p: ");
}


void DisplayLib::updateDisplay(
        int water_level_cm,
        int nutrient_level_cm,
        int ph_downer_level_cm,
        int ph_level,
        int tds_level,
        bool water_pump_state,
        bool nutrient_pump_state,
        bool ph_downer_pump_state,
        bool mixer_pump_state){

	tft.fillRect(90, 50, 128, 50, ST7735_GREEN);

	tft.setCursor(95, 50);
	tft.print((int) water_level_cm);

	tft.setCursor(95, 60);
	tft.print((int) nutrient_level_cm);

	tft.setCursor(95, 70);
	tft.print((int) ph_downer_level_cm);

	tft.setCursor(95, 80);
	tft.print((int) ph_level);

    tft.setCursor(95, 90);
    tft.print((int) tds_level);

    // PRINT RELAY STATUS
	tft.setCursor(95, 100);
	if (water_pump_state){
		tft.print("OPEN");
	}
	else{
		tft.print("CLOSE");
	}

    tft.setCursor(95, 110);
    if (nutrient_pump_state){
        tft.print("OPEN");
    }
    else{
        tft.print("CLOSE");
    }

    tft.setCursor(95, 120);
    if (ph_downer_pump_state){
        tft.print("OPEN");
    }
    else{
        tft.print("CLOSE");
    }

    tft.setCursor(95, 130);
    if (mixer_pump_state){
        tft.print("OPEN");
    }
    else{
        tft.print("CLOSE");
    }
}

void DisplayLib::drawtext(char *text, uint16_t color) {
	tft.fillScreen(ST7735_BLACK);
	tft.setCursor(0, 0);
	tft.setTextColor(color);
	tft.setTextWrap(true);
	tft.print(text);
}

String DisplayLib::ip2Str(IPAddress ip) {
	// took from https://gist.github.com/loosak/76019faaefd5409fca67
	String s = "";
	for (int i = 0; i < 4; i++) {
		s += i ? "." + String(ip[i]) : String(ip[i]);
	}
	return s;
}


void DisplayLib::uptime()
{
	long days=0;
	long hours=0;
	long mins=0;
	long secs=0;
	secs = millis()/1000;
	mins=secs/60; //convert seconds to minutes
	hours=mins/60; //convert minutes to hours
	days=hours/24; //convert hours to days
	secs=secs-(mins*60); //subtract the coverted seconds to minutes in order to display 59 secs max
	mins=mins-(hours*60); //subtract the coverted minutes to hours in order to display 59 minutes max
	hours=hours-(days*24); //subtract the coverted hours to days in order to display 23 hours max

}
