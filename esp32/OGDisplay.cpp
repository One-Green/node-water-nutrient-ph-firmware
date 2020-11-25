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
	// print node tag
	tft.setCursor(2, 50);
	tft.print("Moisture (Raw): ");
	tft.setCursor(2, 60);
	tft.print("Moisture (%)  : ");
	tft.setCursor(2, 70);
	tft.print("Config Min (%): ");
	tft.setCursor(2, 80);
	tft.print("Config Max (%): ");
	tft.setCursor(2, 90);
	tft.print("Valve status  : ");
}


void DisplayLib::updateDisplay(
		float moistureLevelADC,
		float moistureLevel,
		float configMin,
		float configMax,
		bool water_valve_signal){

	tft.fillRect(90, 50, 128, 50, ST7735_GREEN);

	tft.setCursor(95, 50);
	tft.print((int) moistureLevelADC);

	tft.setCursor(95, 60);
	tft.print((int) moistureLevel);

	tft.setCursor(95, 70);
	tft.print((int) configMin);

	tft.setCursor(95, 80);
	tft.print((int) configMax);

	tft.setCursor(95, 90);
	if (water_valve_signal){
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


void DisplayLib::printRegistryError() {
	String message = "Not registered, "
	                 "tag is already in database, "
	                 "to bypass change variable  CHECK_NODE_TAG_DUPLICATE to false";
	tft.fillRect(0, 50, 128, 160, ST7735_RED);
	tft.setCursor(0, 50);
	tft.print(message);
	delay(500);
	tft.fillRect(0, 50, 128, 160, ST7735_YELLOW);
	tft.setCursor(0, 50);
	tft.print(message);
	delay(500);
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
