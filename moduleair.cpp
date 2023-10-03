#include <WString.h>
#include <pgmspace.h>

#define SOFTWARE_VERSION_STR "ModuleAirV2.1-Stream-092023"
#define SOFTWARE_VERSION_STR_SHORT "V2.1-Stream-092023"
String SOFTWARE_VERSION(SOFTWARE_VERSION_STR);
String SOFTWARE_VERSION_SHORT(SOFTWARE_VERSION_STR_SHORT);

#include <Arduino.h>
#include "PxMatrix.h"
#include <SPI.h>

/*****************************************************************
 * IMPORTANT                                          *
 *****************************************************************/

//On force l'utilisation des 2 SPI

//Dans SPI.cpp

// #if CONFIG_IDF_TARGET_ESP32
// SPIClass SPI(VSPI);
// SPIClass SPI_H(HSPI);
// #else
// SPIClass SPI(FSPI);
// #endif

// Dans SPI.h

// extern SPIClass SPI_H; en bas

//Dans PXMatrix

//On remplace tous les SPI. par SPI_H.

//On définit les pins:

// // HW SPI PINS
// #define SPI_BUS_CLK 14
// #define SPI_BUS_MOSI 13
// #define SPI_BUS_MISO 12
// #define SPI_BUS_SS 4

//on remplace la glcdfont.c original dans AdaFruitGFX => mod dans le dossier Fonts

/*****************************************************************
 * IMPORTANT FIN                                          *
 *****************************************************************/

#include <MHZ16_uart.h> // CO2
#include <MHZ19.h>

#include "ccs811.h" // CCS811

#include "ca-root.h"

// includes ESP32 libraries
#define FORMAT_SPIFFS_IF_FAILED true
#include <FS.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HardwareSerial.h>

#if ESP_IDF_VERSION_MAJOR >= 4
#if (ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(1, 0, 6))
#include "sha/sha_parallel_engine.h"
#else
#include <esp32/sha.h>
#endif
#else
//#include <hwcrypto/sha.h>
#endif

#include <WebServer.h>
#include <ESPmDNS.h>
#include <MD5Builder.h>

// includes external libraries

#include "./Fonts/oledfont.h"	  // avoids including the default Arial font, needs to be included before SSD1306.h
#include "./Fonts/Font4x7Fixed.h" // modified Pour l'affichage des unités
#include "./Fonts/Font4x5Fixed.h" //pour l'affichage des infos de debug
#include <SSD1306Wire.h>

#define ARDUINOJSON_ENABLE_ARDUINO_STREAM 0
#define ARDUINOJSON_ENABLE_ARDUINO_PRINT 0
#define ARDUINOJSON_DECODE_UNICODE 0
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <StreamString.h>
#include "./bmx280_i2c.h"
#include "./configuration.h"

// includes files
#include "./intl.h"
#include "./utils.h"
#include "defines.h"
#include "ext_def.h"
#include "html-content.h"

/*****************************************************************
 * CONFIGURATION                                          *
 *****************************************************************/
namespace cfg
{
	unsigned debug = DEBUG;

	unsigned time_for_wifi_config = TIME_FOR_WIFI_CONFIG;
	unsigned sending_intervall_ms = SENDING_INTERVALL_MS;
	unsigned screen_intervall_ms = SCREEN_INTERVALL_MS;

	char current_lang[3];

	// credentials for basic auth of internal web server
	bool www_basicauth_enabled = WWW_BASICAUTH_ENABLED;
	char www_username[LEN_WWW_USERNAME];
	char www_password[LEN_CFG_PASSWORD];

	// wifi credentials
	char wlanssid[LEN_WLANSSID];
	char wlanpwd[LEN_CFG_PASSWORD];

	// credentials of the sensor in access point mode
	char fs_ssid[LEN_FS_SSID] = FS_SSID;
	char fs_pwd[LEN_CFG_PASSWORD] = FS_PWD;

	// main config

	bool has_wifi = HAS_WIFI;

	// (in)active sensors
	bool sds_read = SDS_READ;
	bool npm_read = NPM_READ;
	// bool npm_fulltime = NPM_FULLTIME;
	bool bmx280_read = BMX280_READ;
	bool mhz16_read = MHZ16_READ;
	bool mhz19_read = MHZ19_READ;
	bool ccs811_read = CCS811_READ;

	// Location

	char height_above_sealevel[8] = "0";

	// send to "APIs"
	bool send2dusti = SEND2SENSORCOMMUNITY;
	bool send2madavi = SEND2MADAVI;
	bool send2custom = SEND2CUSTOM;
	bool send2custom2 = SEND2CUSTOM2;
	bool send2csv = SEND2CSV;

	// (in)active displays
	bool has_ssd1306 = HAS_SSD1306;
	bool has_matrix = HAS_MATRIX;
	bool display_measure = DISPLAY_MEASURE;
	bool display_forecast = DISPLAY_FORECAST;
	bool display_logo = DISPLAY_LOGO;
	bool display_alert = DISPLAY_ALERT;
	bool display_wifi_info = DISPLAY_WIFI_INFO;
	bool display_device_info = DISPLAY_DEVICE_INFO;

	// API settings
	bool ssl_madavi = SSL_MADAVI;
	bool ssl_dusti = SSL_SENSORCOMMUNITY;

	// API AirCarto
	char host_custom[LEN_HOST_CUSTOM];
	char url_custom[LEN_URL_CUSTOM];
	bool ssl_custom = SSL_CUSTOM;
	unsigned port_custom = PORT_CUSTOM;
	char user_custom[LEN_USER_CUSTOM] = USER_CUSTOM;
	char pwd_custom[LEN_CFG_PASSWORD] = PWD_CUSTOM;

	// API AtmoSud
	char host_custom2[LEN_HOST_CUSTOM2];
	char url_custom2[LEN_URL_CUSTOM2];
	bool ssl_custom2 = SSL_CUSTOM2;
	unsigned port_custom2 = PORT_CUSTOM2;
	char user_custom2[LEN_USER_CUSTOM2] = USER_CUSTOM2;
	char pwd_custom2[LEN_CFG_PASSWORD] = PWD_CUSTOM2;

	// First load
	void initNonTrivials(const char *id)
	{
		strcpy(cfg::current_lang, CURRENT_LANG);
		strcpy_P(www_username, WWW_USERNAME);
		strcpy_P(www_password, WWW_PASSWORD);
		strcpy_P(wlanssid, WLANSSID);
		strcpy_P(wlanpwd, WLANPWD);
		strcpy_P(host_custom, HOST_CUSTOM);
		strcpy_P(url_custom, URL_CUSTOM);
		strcpy_P(host_custom2, HOST_CUSTOM2);
		strcpy_P(url_custom2, URL_CUSTOM2);

		if (!*fs_ssid)
		{
			strcpy(fs_ssid, SSID_BASENAME);
			strcat(fs_ssid, id);
		}
	}
}

bool spiffs_matrix;

static byte booltobyte(bool array[8])
{

	byte result = 0;
	for (int i = 0; i < 8; i++)
	{
		if (array[i])
		{
			result |= (byte)(1 << (7 - i));
		}
	}

	return result;
}

// define size of the config JSON
#define JSON_BUFFER_SIZE 2300
// define size of the AtmoSud Forecast API JSON
#define JSON_BUFFER_SIZE2 500

LoggerConfig loggerConfigs[LoggerCount];

// test variables
long int sample_count = 0;
bool bmx280_init_failed = false;
bool ccs811_init_failed = false;
bool moduleair_selftest_failed = false;

WebServer server(80);

// include JSON config reader
#include "./moduleair-cfg.h"

/*****************************************************************
 * Display definitions                                           *
 *****************************************************************/

SSD1306Wire *oled_ssd1306 = nullptr; // as pointer

//For the matrix
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
#define matrix_width 64
#define matrix_height 32
uint8_t display_draw_time = 30; //10-50 is usually fine
PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

uint8_t logos[3] = {0, 0, 0};
uint8_t logo_index = 0;
bool has_logo;

extern const uint8_t gamma8[]; //for gamma correction

struct RGB
{
	byte R;
	byte G;
	byte B;
};

struct RGB displayColor
{
	0, 0, 0
};

uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);
uint16_t myCUSTOM = display.color565(displayColor.R, displayColor.G, displayColor.B);
uint16_t myCOLORS[8] = {myRED, myGREEN, myCYAN, myWHITE, myYELLOW, myCYAN, myMAGENTA, myBLACK};

void IRAM_ATTR display_updater()
{
	// Increment the counter and set the time of ISR
	portENTER_CRITICAL_ISR(&timerMux);
	display.display(display_draw_time);
	portEXIT_CRITICAL_ISR(&timerMux);
}

void display_update_enable(bool is_enable)
{
	if (is_enable)
	{
		debug_outln_info(F("Call display_update_enable function with: "), F("true"));
		timerAttachInterrupt(timer, &display_updater, true);
		timerAlarmWrite(timer, 4000, true);
		timerAlarmEnable(timer);
	}
	else
	{
		debug_outln_info(F("Call display_update_enable function with: "), F("false"));
		timerDetachInterrupt(timer);
		timerAlarmDisable(timer);
	}
}

void drawImage(int x, int y, int h, int w, uint16_t image[])
{
	int imageHeight = h;
	int imageWidth = w;
	int counter = 0;
	for (int yy = 0; yy < imageHeight; yy++)
	{
		for (int xx = 0; xx < imageWidth; xx++)
		{
			display.drawPixel(xx + x, yy + y, image[counter]);
			counter++;
		}
	}
}

void drawImage2(int x, int y, int h, int w, uint8_t image[])
{
	int imageHeight = h;
	int imageWidth = w;
	int counter = 0;
	for (int yy = 0; yy < imageHeight; yy++)
	{
		for (int xx = 0; xx < imageWidth; xx++)
		{
			display.drawPixel(xx + x, yy + y, word(image[counter], image[counter + 1])); //creer ici avec le word
			counter += 2;
		}
	}
}

bool gamma_correction = true; //Gamma correction

struct RGB interpolateint(float valueSensor, int step1, int step2, int step3, bool correction)
{
	struct RGB result;
	uint16_t rgb565;

	if (valueSensor == 0)
	{
		result.R = 0;
		result.G = 255; // VERT
		result.B = 0;
	}
	else if (valueSensor > 0 && valueSensor < step1)
	{
		result.R = 0;
		result.G = 255; // VERT
		result.B = 0;
	}
	else if (valueSensor >= step1 && valueSensor < step2)
	{
		result.R = 255;
		result.G = 255; // jaune
		result.B = 0;
	}
	else if (valueSensor >= step2 && valueSensor < step3)
	{
		result.R = 255;
		result.G = 140; // orange
		result.B = 0;
	}
	else if (valueSensor >= step3)
	{
		result.R = 255;
		result.G = 0; // ROUGE
		result.B = 0;
	}
	else
	{
		result.R = 0;
		result.G = 0;
		result.B = 0;
	}

	if (correction == true)
	{
		result.R = pgm_read_byte(&gamma8[result.R]);
		result.G = pgm_read_byte(&gamma8[result.G]);
		result.B = pgm_read_byte(&gamma8[result.B]);
	}

	rgb565 = ((result.R & 0b11111000) << 8) | ((result.G & 0b11111100) << 3) | (result.B >> 3);
	return result;
}

struct RGB interpolateindice(int valueIndice, bool correction)
{

	struct RGB result;
	uint16_t rgb565;

	switch (valueIndice)
	{
	case 1:
		result.R = 80;
		result.G = 240; //blue
		result.B = 230;
		break;
	case 2:
		result.R = 80;
		result.G = 204; //green
		result.B = 170;
		break;
	case 3:
		result.R = 237;
		result.G = 230; //yellow
		result.B = 97;
		break;
	case 4:
		result.R = 237;
		result.G = 94; //orange
		result.B = 88;
		break;
	case 5:
		result.R = 136;
		result.G = 26; //red
		result.B = 51;
		break;
	case 6:
		result.R = 115;
		result.G = 40; //violet
		result.B = 125;
		break;
	default:
		result.R = 0;
		result.G = 0;
		result.B = 0;
	}

	if (correction == true)
	{
		result.R = pgm_read_byte(&gamma8[result.R]);
		result.G = pgm_read_byte(&gamma8[result.G]);
		result.B = pgm_read_byte(&gamma8[result.B]);
	}

	rgb565 = ((result.R & 0b11111000) << 8) | ((result.G & 0b11111100) << 3) | (result.B >> 3);
	//Debug.println(rgb565); // to get list of color if drawGradient is acitvated
	return result;
}

struct RGB interpolate(float valueSensor, int step1, int step2, int step3, int step4, int step5, bool correction)
{

	byte endColorValueR;
	byte startColorValueR;
	byte endColorValueG;
	byte startColorValueG;
	byte endColorValueB;
	byte startColorValueB;

	int valueLimitHigh;
	int valueLimitLow;
	struct RGB result;
	uint16_t rgb565;

	if (valueSensor == 0)
	{
		result.R = 80;
		result.G = 240; //blue
		result.B = 230;
	}
	else if (valueSensor > 0 && valueSensor <= step5)
	{
		if (valueSensor <= step1)
		{
			valueLimitHigh = step1;
			valueLimitLow = 0;
			endColorValueR = 80;
			startColorValueR = 80; //blue to green
			endColorValueG = 204;
			startColorValueG = 240;
			endColorValueB = 170;
			startColorValueB = 230;
		}
		else if (valueSensor > step1 && valueSensor <= step2)
		{
			valueLimitHigh = step2;
			valueLimitLow = step1;
			endColorValueR = 237;
			startColorValueR = 80;
			endColorValueG = 230; //green to yellow
			startColorValueG = 204;
			endColorValueB = 97;
			startColorValueB = 170;
		}
		else if (valueSensor > step2 && valueSensor <= step3)
		{
			valueLimitHigh = step3;
			valueLimitLow = step2;
			endColorValueR = 237;
			startColorValueR = 237;
			endColorValueG = 94; //yellow to orange
			startColorValueG = 230;
			endColorValueB = 88;
			startColorValueB = 97;
		}
		else if (valueSensor > step3 && valueSensor <= step4)
		{
			valueLimitHigh = step4;
			valueLimitLow = step3;
			endColorValueR = 136;
			startColorValueR = 237;
			endColorValueG = 26; // orange to red
			startColorValueG = 94;
			endColorValueB = 51;
			startColorValueB = 88;
		}
		else if (valueSensor > step4 && valueSensor <= step5)
		{
			valueLimitHigh = step5;
			valueLimitLow = step4;
			endColorValueR = 115;
			startColorValueR = 136;
			endColorValueG = 40; // red to violet
			startColorValueG = 26;
			endColorValueB = 125;
			startColorValueB = 51;
		}

		result.R = (byte)(((endColorValueR - startColorValueR) * ((valueSensor - valueLimitLow) / (valueLimitHigh - valueLimitLow))) + startColorValueR);
		result.G = (byte)(((endColorValueG - startColorValueG) * ((valueSensor - valueLimitLow) / (valueLimitHigh - valueLimitLow))) + startColorValueG);
		result.B = (byte)(((endColorValueB - startColorValueB) * ((valueSensor - valueLimitLow) / (valueLimitHigh - valueLimitLow))) + startColorValueB);
	}
	else if (valueSensor > step5)
	{
		result.R = 115;
		result.G = 40; //violet
		result.B = 125;
	}
	else
	{
		result.R = 0;
		result.G = 0;
		result.B = 0;
	}

	//Gamma Correction

	if (correction == true)
	{
		result.R = pgm_read_byte(&gamma8[result.R]);
		result.G = pgm_read_byte(&gamma8[result.G]);
		result.B = pgm_read_byte(&gamma8[result.B]);
	}

	rgb565 = ((result.R & 0b11111000) << 8) | ((result.G & 0b11111100) << 3) | (result.B >> 3);
	//Debug.println(rgb565); // to get list of color if drawGradient is acitvated
	return result;
}

struct RGB interpolateint2(float valueSensor, int step1, int step2, bool correction)
{

	struct RGB result;
	uint16_t rgb565;

	if (valueSensor == 0)
	{
		result.R = 0;
		result.G = 255; // Green entre 0 et 800
		result.B = 0;
	}
	else if (valueSensor > 0 && valueSensor < step1)
	{
		result.R = 0;
		result.G = 255; // Green entre 0 et 800
		result.B = 0;
	}
	else if (valueSensor >= step1 && valueSensor < step2)
	{
		result.R = 255;
		result.G = 140; // Orange entre 800 et 1500
		result.B = 0;
	}
	else if (valueSensor >= step2)
	{
		result.R = 255;
		result.G = 0; // Rouge supérieur à 1500
		result.B = 0;
	}
	else
	{
		result.R = 0;
		result.G = 0;
		result.B = 0;
	}

	if (correction == true)
	{
		result.R = pgm_read_byte(&gamma8[result.R]);
		result.G = pgm_read_byte(&gamma8[result.G]);
		result.B = pgm_read_byte(&gamma8[result.B]);
	}

	rgb565 = ((result.R & 0b11111000) << 8) | ((result.G & 0b11111100) << 3) | (result.B >> 3);
	//Debug.println(rgb565); // to get list of color if drawGradient is acitvated
	return result;
}

struct RGB interpolateint3(float valueSensor, int step1, int step2, bool correction) // Humi
{
	struct RGB result;
	uint16_t rgb565;

	if (valueSensor == 0)
	{
		result.R = 255;
		result.G = 0; // red
		result.B = 0;
	}
	else if (valueSensor > 0 && valueSensor < step1)
	{
		result.R = 255;
		result.G = 0; // red
		result.B = 0;
	}
	else if (valueSensor >= step1 && valueSensor < step2)
	{
		result.R = 0;
		result.G = 255; // green
		result.B = 0;
	}
	else if (valueSensor > step2)
	{
		result.R = 255;
		result.G = 0; // red
		result.B = 0;
	}
	else
	{
		result.R = 0;
		result.G = 0;
		result.B = 0;
	}

	if (correction == true)
	{
		result.R = pgm_read_byte(&gamma8[result.R]);
		result.G = pgm_read_byte(&gamma8[result.G]);
		result.B = pgm_read_byte(&gamma8[result.B]);
	}

	rgb565 = ((result.R & 0b11111000) << 8) | ((result.G & 0b11111100) << 3) | (result.B >> 3);
	//Debug.println(rgb565); // to get list of color if drawGradient is acitvated
	return result;
}

struct RGB interpolateint4(float valueSensor, int step1, int step2, bool correction) // temp
{
	struct RGB result;
	uint16_t rgb565;

	if (valueSensor >= -128 && valueSensor < step1)
	{
		result.R = 0;
		result.G = 0; // Bleu / Trop froid inférieur à 19 (step1)
		result.B = 255;
	}
	else if (valueSensor >= step1 && valueSensor < step2)
	{
		result.R = 0;
		result.G = 255; // Green ok
		result.B = 0;
	}
	else if (valueSensor >= step2)
	{
		result.R = 255;
		result.G = 0; // RED / trop chaud supérieur à 28
		result.B = 0;
	}
	else
	{
		result.R = 0;
		result.G = 0;
		result.B = 0;
	}

	if (correction == true)
	{
		result.R = pgm_read_byte(&gamma8[result.R]);
		result.G = pgm_read_byte(&gamma8[result.G]);
		result.B = pgm_read_byte(&gamma8[result.B]);
	}

	rgb565 = ((result.R & 0b11111000) << 8) | ((result.G & 0b11111100) << 3) | (result.B >> 3);
	//Debug.println(rgb565); // to get list of color if drawGradient is acitvated
	return result;
}

//You can use drawGradient once in order to get the list of colors and then create an image which is much faster to display

void drawgradient(int x, int y, float valueSensor, int step1, int step2, int step3, int step4, int step5)
{
	int gradientHeight = 7;
	int gradientWidth = 64;
	int pixelvalue[64];
	RGB pixelcolors[64];

	debug_outln_info(F("Pixel values"));
	for (uint8_t i = 0; i < gradientWidth; i++)
	{
		pixelvalue[i] = (int)((i * step5) / (gradientWidth - 1));
		Debug.print(F(" "));
		Debug.print(F(pixelvalue[i]));
		if (i == 63)
		{
			Debug.printf("\n");
		}
	}

	for (uint8_t j = 0; j < gradientWidth; j++)
	{
		int value = pixelvalue[j];
		pixelcolors[j] = interpolate(value, step1, step2, step3, step4, step5, true);
	}

	for (uint8_t k = 0; k < gradientHeight; k++)
	{

		for (int l = 0; l < gradientWidth; l++)
		{
			uint16_t myPIXEL = display.color565(pixelcolors[l].R, pixelcolors[l].G, pixelcolors[l].B);
			display.drawPixel(x + l, y + k, myPIXEL);
		}
	}
}

void drawCentreString(const String &buf, int x, int y, int offset)
{
	int16_t x1, y1;
	uint16_t w, h;
	display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
	display.setCursor(((64 - offset) - w) / 2, y);		//si 1 seul chiffre => taille de 2 chiffres !!!
	display.print(buf);
}

void messager1(float valueSensor, int step1, int step2, int step3)
{
	display.setFont(NULL);
	display.setTextSize(1);

	if (valueSensor > -1 && valueSensor < step1)
	{
		drawCentreString(INTL_GOOD, 0, 25, 0);
	}
	else if (valueSensor >= step1 && valueSensor < step2)
	{
		drawCentreString(INTL_MEDIUM, 0, 25, 0);
	}
	else if (valueSensor >= step2 && valueSensor < step3)
	{
		drawCentreString(INTL_DEGRADED, 0, 25, 0);
	}
	else if (valueSensor >= step3)
	{
		drawCentreString(INTL_BAD, 0, 25, 0);
	}
	else
	{
		drawCentreString(INTL_ERR, 0, 25, 0);
	}
}

void messager2(float valueSensor, int step1, int step2)
{
	display.setFont(NULL);
	display.setTextSize(1);

	if (valueSensor > -1 && valueSensor < step1)
	{
		drawCentreString(INTL_WELL, 0, 25, 0);
	}
	else if (valueSensor >= step1 && valueSensor < step2)
	{
		drawCentreString(INTL_AERATE_PLS, 0, 25, 0); // entre 800 et 1500
	}
	else if (valueSensor >= step2)
	{
		drawCentreString(INTL_AERATE_PLS, 0, 25, 0);
	}
	else
	{
		drawCentreString(INTL_ERR, 0, 25, 0);
	}
}

void messager3(float valueSensor, int step1, int step2) // humi
{
	display.setFont(NULL);
	display.setTextSize(1);

	if (valueSensor > -1 && valueSensor < step1)
	{
		drawCentreString(INTL_DRY, 0, 25, 0);
	}
	else if (valueSensor >= step1 && valueSensor < step2)
	{
		drawCentreString(INTL_IDEAL, 0, 25, 0);
	}
	else if (valueSensor >= step2)
	{
		drawCentreString(INTL_WET, 0, 25, 0);
	}
	else
	{
		drawCentreString(INTL_ERR, 0, 25, 0);
	}
}

void messager4(float valueSensor, int step1, int step2) // temp
{
	display.setFont(NULL);
	display.setTextSize(1);

	if (valueSensor > -128 && valueSensor < step1)
	{
		drawCentreString(INTL_COLD, 0, 25, 0);
		display.print(INTL_COLD);
	}
	else if (valueSensor >= step1 && valueSensor < step2)
	{
		drawCentreString(INTL_TOK, 0, 25, 0);
	}
	else if (valueSensor >= step2)
	{
		drawCentreString(INTL_WARM, 0, 25, 0);
	}
	else
	{
		drawCentreString(INTL_ERR, 0, 25, 0);
	}
}

void messager5(int value) // Indice Atmo
{
	display.setFont(NULL);
	display.setTextSize(1);

	switch (value)
	{
	case 1:
		drawCentreString(INTL_GOOD, 0, 25, 0);
		break;
	case 2:
		drawCentreString(INTL_MEDIUM, 0, 25, 0);
		break;
	case 3:
		drawCentreString(INTL_DEGRADED, 0, 25, 0);
		break;
	case 4:
		drawCentreString(INTL_BAD, 0, 25, 0);
		break;
	case 5:
#if defined(INTL_EN)
		drawCentreString(INTL_VERY_BAD, 0, 25, 0);
#endif
#if defined(INTL_FR)
		display.setFont(&Font4x7Fixed);
		display.setCursor(0, 31);
		display.print(INTL_VERY_BAD);
#endif
		break;
	case 6:
#if defined(INTL_EN)
		drawCentreString(INTL_EXT_BAD, 0, 25, 0);
#endif
#if defined(INTL_FR)
		display.setFont(&Font4x7Fixed);
		display.setCursor(0, 31);
		display.print(INTL_EXT_BAD);
#endif
		break;
	default:
		drawCentreString(INTL_ERR, 0, 25, 0);
	}
}

/*****************************************************************
 * Forecast                                               *
 *****************************************************************/
uint8_t *forecast_screen_0 = nullptr;
uint8_t *forecast_screen_1 = nullptr;
uint8_t *forecast_screen_2 = nullptr;
uint8_t *forecast_screen_3 = nullptr;
uint8_t *forecast_screen_4 = nullptr;
uint8_t *forecast_screen_5 = nullptr;
uint8_t *forecast_screen_6 = nullptr;
uint8_t *forecast_screen_7 = nullptr;
uint8_t *forecast_screen_8 = nullptr;
uint8_t *forecast_screen_9 = nullptr;
uint8_t *forecast_screen_10 = nullptr;

uint8_t forecast_selector;
uint8_t last_selector = 10;

/*****************************************************************
 * Logo                                               *
 *****************************************************************/
uint8_t *logo_screen = nullptr;

/*****************************************************************
 * Alert                                               *
 *****************************************************************/
uint8_t *alert_screen = nullptr;

/*****************************************************************
 * Serial declarations                                           *
 *****************************************************************/
#define serialSDS (Serial1)
#define serialNPM (Serial1)
#define serialMHZ (Serial2)

/*****************************************************************
 * BMP/BME280 declaration                                        *
 *****************************************************************/
BMX280 bmx280;

/*****************************************************************
 * MH-Z16 declaration                                        *
 *****************************************************************/
MHZ16_uart mhz16;

/*****************************************************************
 * MH-Z19 declaration                                        *
 *****************************************************************/
MHZ19 mhz19;

/*****************************************************************
 * CCS811 declaration                                        *
 *****************************************************************/
CCS811 ccs811(-1);

/*****************************************************************
 * Time                                       *
 *****************************************************************/

// time management varialbles
bool send_now = false;
bool screen_now = false;
unsigned long starttime;
unsigned long screentime;
unsigned long time_end_setup;
unsigned long time_before_config;
int prec;
unsigned long time_point_device_start_ms;
unsigned long starttime_SDS;
unsigned long starttime_NPM;
unsigned long starttime_MHZ16;
unsigned long starttime_MHZ19;
unsigned long starttime_CCS811;
unsigned long last_NPM;
unsigned long act_micro;
unsigned long act_milli;
unsigned long last_micro = 0;
unsigned long min_micro = 1000000000;
unsigned long max_micro = 0;

unsigned long sending_time = 0;
int last_sendData_returncode;

bool wifi_connection_lost;

/*****************************************************************
 * SDS variables and enums                                      *
 *****************************************************************/

bool is_SDS_running;

// To read SDS responses

enum
{
	SDS_REPLY_HDR = 10,
	SDS_REPLY_BODY = 8
} SDS_waiting_for;

/*****************************************************************
 * NPM variables and enums                                       *
 *****************************************************************/

bool is_NPM_running = false;
bool nextpmconnected; //important to test nextpm and avoid endless loops

// To read NPM responses
enum
{
	NPM_REPLY_HEADER_16 = 16,
	NPM_REPLY_STATE_16 = 14,
	NPM_REPLY_BODY_16 = 13,
	NPM_REPLY_CHECKSUM_16 = 1
} NPM_waiting_for_16; // for concentration

enum
{
	NPM_REPLY_HEADER_4 = 4,
	NPM_REPLY_STATE_4 = 2,
	NPM_REPLY_CHECKSUM_4 = 1
} NPM_waiting_for_4; // for change

enum
{
	NPM_REPLY_HEADER_5 = 5,
	NPM_REPLY_STATE_5 = 3,
	NPM_REPLY_DATA_5 = 2,
	NPM_REPLY_CHECKSUM_5 = 1
} NPM_waiting_for_5; // for fan speed

enum
{
	NPM_REPLY_HEADER_6 = 6,
	NPM_REPLY_STATE_6 = 4,
	NPM_REPLY_DATA_6 = 3,
	NPM_REPLY_CHECKSUM_6 = 1
} NPM_waiting_for_6; // for version

enum
{
	NPM_REPLY_HEADER_8 = 8,
	NPM_REPLY_STATE_8 = 6,
	NPM_REPLY_BODY_8 = 5,
	NPM_REPLY_CHECKSUM_8 = 1
} NPM_waiting_for_8; // for temperature/humidity

String current_state_npm;
String current_th_npm;

/*****************************************************************
 * Data variables                                      *
 *****************************************************************/
float last_value_BMX280_T = -128.0;
float last_value_BMX280_P = -1.0;
float last_value_BME280_H = -1.0;

uint32_t sds_pm10_sum = 0;
uint32_t sds_pm25_sum = 0;
uint32_t sds_val_count = 0;
uint32_t sds_pm10_max = 0;
uint32_t sds_pm10_min = 20000;
uint32_t sds_pm25_max = 0;
uint32_t sds_pm25_min = 20000;

uint32_t npm_pm1_sum = 0;
uint32_t npm_pm10_sum = 0;
uint32_t npm_pm25_sum = 0;
uint32_t npm_pm1_sum_pcs = 0;
uint32_t npm_pm10_sum_pcs = 0;
uint32_t npm_pm25_sum_pcs = 0;
uint16_t npm_val_count = 0;

float last_value_SDS_P1 = -1.0;
float last_value_SDS_P2 = -1.0;
float last_value_NPM_P0 = -1.0;
float last_value_NPM_P1 = -1.0;
float last_value_NPM_P2 = -1.0;
float last_value_NPM_N1 = -1.0;
float last_value_NPM_N10 = -1.0;
float last_value_NPM_N25 = -1.0;

float last_value_MHZ16 = -1.0;
uint32_t mhz16_sum = 0;
uint16_t mhz16_val_count = 0;

float last_value_MHZ19 = -1.0;
uint32_t mhz19_sum = 0;
uint16_t mhz19_val_count = 0;

float last_value_CCS811 = -1.0;
uint32_t ccs811_sum = 0;
uint16_t ccs811_val_count = 0;

String last_data_string;
int last_signal_strength;
int last_disconnect_reason;
// int last_connect_reason;

String esp_chipid;

String last_value_SDS_version;
String last_value_NPM_version;

unsigned long SDS_error_count;
unsigned long NPM_error_count;
unsigned long MHZ16_error_count;
unsigned long MHZ19_error_count;
unsigned long CCS811_error_count;
unsigned long WiFi_error_count;

unsigned long last_page_load = millis();

bool wificonfig_loop = false;
uint8_t sntp_time_set;

unsigned long count_sends = 0;
unsigned long last_display_millis_oled = 0;
unsigned long last_display_millis_matrix = 0;
uint8_t next_display_count = 0;

struct struct_wifiInfo
{
	char ssid[LEN_WLANSSID];
	uint8_t encryptionType;
	int32_t RSSI;
	int32_t channel;
};

struct struct_wifiInfo *wifiInfo;
uint8_t count_wifiInfo;

#define msSince(timestamp_before) (act_milli - (timestamp_before))

const char data_first_part[] PROGMEM = "{\"software_version\": \"" SOFTWARE_VERSION_STR "\", \"sensordatavalues\":[";
const char JSON_SENSOR_DATA_VALUES[] PROGMEM = "sensordatavalues";

static String displayGenerateFooter(unsigned int screen_count)
{
	String display_footer;
	for (unsigned int i = 0; i < screen_count; ++i)
	{
		display_footer += (i != (next_display_count % screen_count)) ? " . " : " o ";
	}
	return display_footer;
}

/*****************************************************************
 * display values                                                *
 *****************************************************************/
static void display_debug(const String &text1, const String &text2)
{
	debug_outln_info(F("output debug text to displays..."));

	if (cfg::has_ssd1306)
	{
		if (oled_ssd1306)
		{
			oled_ssd1306->clear();
			oled_ssd1306->displayOn();
			oled_ssd1306->setTextAlignment(TEXT_ALIGN_LEFT);
			oled_ssd1306->drawString(0, 12, text1);
			oled_ssd1306->drawString(0, 24, text2);
			oled_ssd1306->display();
		}
	}
}

/*****************************************************************
 * read SDS011 sensor serial and firmware date                   *
 *****************************************************************/
static String SDS_version_date()
{
	if (cfg::sds_read && !last_value_SDS_version.length())
	{
		debug_outln_verbose(FPSTR(DBG_TXT_START_READING), FPSTR(DBG_TXT_SDS011_VERSION_DATE));
		is_SDS_running = SDS_cmd(PmSensorCmd::Start);
		delay(250);
		serialSDS.flush();
		// Query Version/Date
		SDS_rawcmd(0x07, 0x00, 0x00);
		delay(400);
		const constexpr uint8_t header_cmd_response[2] = {0xAA, 0xC5};
		while (serialSDS.find(header_cmd_response, sizeof(header_cmd_response)))
		{
			uint8_t data[8];
			unsigned r = serialSDS.readBytes(data, sizeof(data));
			if (r == sizeof(data) && data[0] == 0x07 && SDS_checksum_valid(data))
			{
				char tmp[20];
				snprintf_P(tmp, sizeof(tmp), PSTR("%02d-%02d-%02d(%02x%02x)"),
						   data[1], data[2], data[3], data[4], data[5]);
				last_value_SDS_version = tmp;
				break;
			}
		}
		debug_outln_verbose(FPSTR(DBG_TXT_END_READING), FPSTR(DBG_TXT_SDS011_VERSION_DATE));
	}

	return last_value_SDS_version;
}

/*****************************************************************
 * NPM functions     *
 *****************************************************************/
static int8_t NPM_get_state()
{
	int8_t result = -1;
	NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
	debug_outln_info(F("State NPM..."));
	NPM_cmd(PmSensorCmd2::State);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_4)
	{
		const uint8_t constexpr header[2] = {0x81, 0x16};
		uint8_t state[1];
		uint8_t checksum[1];
		uint8_t test[4];

		switch (NPM_waiting_for_4)
		{
		case NPM_REPLY_HEADER_4:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_4 = NPM_REPLY_STATE_4;
			break;
		case NPM_REPLY_STATE_4:
			serialNPM.readBytes(state, sizeof(state));
			NPM_state(state[0]);
			result = state[0];
			NPM_waiting_for_4 = NPM_REPLY_CHECKSUM_4;
			break;
		case NPM_REPLY_CHECKSUM_4:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], checksum, sizeof(checksum));
			NPM_data_reader(test, 4);
			NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
			if (NPM_checksum_valid_4(test))
			{
				debug_outln_info(F("Checksum OK..."));
			}
			break;
		}
	}
	return result;
}

static bool NPM_start_stop()
{
	bool result;
	NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
	debug_outln_info(F("Switch start/stop NPM..."));
	NPM_cmd(PmSensorCmd2::Change);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_4)
	{
		const uint8_t constexpr header[2] = {0x81, 0x15};
		uint8_t state[1];
		uint8_t checksum[1];
		uint8_t test[4];

		switch (NPM_waiting_for_4)
		{
		case NPM_REPLY_HEADER_4:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_4 = NPM_REPLY_STATE_4;
			break;
		case NPM_REPLY_STATE_4:
			serialNPM.readBytes(state, sizeof(state));
			NPM_state(state[0]);

			if (bitRead(state[0], 0) == 0)
			{
				debug_outln_info(F("NPM start..."));
				result = true;
			}
			else if (bitRead(state[0], 0) == 1)
			{
				debug_outln_info(F("NPM stop..."));
				result = false;
			}
			else
			{
				result = !is_NPM_running; //DANGER BECAUSE NON INITIALISED
			}

			NPM_waiting_for_4 = NPM_REPLY_CHECKSUM_4;
			break;
		case NPM_REPLY_CHECKSUM_4:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], checksum, sizeof(checksum));
			NPM_data_reader(test, 4);
			NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
			if (NPM_checksum_valid_4(test))
			{
				debug_outln_info(F("Checksum OK..."));
			}
			break;
		}
	}
	return result;
}

static String NPM_version_date()
{
	delay(250);
	NPM_waiting_for_6 = NPM_REPLY_HEADER_6;
	debug_outln_info(F("Version NPM..."));
	NPM_cmd(PmSensorCmd2::Version);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_6)
	{
		const uint8_t constexpr header[2] = {0x81, 0x17};
		uint8_t state[1];
		uint8_t data[2];
		uint8_t checksum[1];
		uint8_t test[6];

		switch (NPM_waiting_for_6)
		{
		case NPM_REPLY_HEADER_6:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_6 = NPM_REPLY_STATE_6;
			break;
		case NPM_REPLY_STATE_6:
			serialNPM.readBytes(state, sizeof(state));
			NPM_state(state[0]);
			NPM_waiting_for_6 = NPM_REPLY_DATA_6;
			break;
		case NPM_REPLY_DATA_6:
			if (serialNPM.readBytes(data, sizeof(data)) == sizeof(data))
			{
				NPM_data_reader(data, 2);
				uint16_t NPMversion = word(data[0], data[1]);
				last_value_NPM_version = String(NPMversion);
				// debug_outln_verbose(FPSTR(DBG_TXT_END_READING), FPSTR(DBG_TXT_NPM_VERSION_DATE));
				debug_outln_info(F("Next PM Firmware: "), last_value_NPM_version);
			}
			NPM_waiting_for_6 = NPM_REPLY_CHECKSUM_6;
			break;
		case NPM_REPLY_CHECKSUM_6:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			NPM_data_reader(test, 6);
			NPM_waiting_for_6 = NPM_REPLY_HEADER_6;
			if (NPM_checksum_valid_6(test))
			{
				debug_outln_info(F("Checksum OK..."));
			}
			break;
		}
	}
	return last_value_NPM_version;
}

static void NPM_fan_speed()
{

	NPM_waiting_for_5 = NPM_REPLY_HEADER_5;
	debug_outln_info(F("Set fan speed to 50 %..."));
	NPM_cmd(PmSensorCmd2::Speed);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_5)
	{
		const uint8_t constexpr header[2] = {0x81, 0x21};
		uint8_t state[1];
		uint8_t data[1];
		uint8_t checksum[1];
		uint8_t test[5];

		switch (NPM_waiting_for_5)
		{
		case NPM_REPLY_HEADER_5:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_5 = NPM_REPLY_STATE_5;
			break;
		case NPM_REPLY_STATE_5:
			serialNPM.readBytes(state, sizeof(state));
			NPM_state(state[0]);
			NPM_waiting_for_5 = NPM_REPLY_DATA_5;
			break;
		case NPM_REPLY_DATA_5:
			if (serialNPM.readBytes(data, sizeof(data)) == sizeof(data))
			{
				NPM_data_reader(data, 1);
			}
			NPM_waiting_for_5 = NPM_REPLY_CHECKSUM_5;
			break;
		case NPM_REPLY_CHECKSUM_5:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			NPM_data_reader(test, 5);
			NPM_waiting_for_5 = NPM_REPLY_HEADER_5;
			if (NPM_checksum_valid_5(test))
			{
				debug_outln_info(F("Checksum OK..."));
			}
			break;
		}
	}
}

static String NPM_temp_humi()
{
	uint16_t NPM_temp;
	uint16_t NPM_humi;
	NPM_waiting_for_8 = NPM_REPLY_HEADER_8;
	debug_outln_info(F("Temperature/Humidity in Next PM..."));
	NPM_cmd(PmSensorCmd2::Temphumi);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_8)
	{
		const uint8_t constexpr header[2] = {0x81, 0x14};
		uint8_t state[1];
		uint8_t data[4];
		uint8_t checksum[1];
		uint8_t test[8];

		switch (NPM_waiting_for_8)
		{
		case NPM_REPLY_HEADER_8:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_8 = NPM_REPLY_STATE_8;
			break;
		case NPM_REPLY_STATE_8:
			serialNPM.readBytes(state, sizeof(state));
			NPM_state(state[0]);
			NPM_waiting_for_8 = NPM_REPLY_BODY_8;
			break;
		case NPM_REPLY_BODY_8:
			if (serialNPM.readBytes(data, sizeof(data)) == sizeof(data))
			{
				NPM_data_reader(data, 4);
				NPM_temp = word(data[0], data[1]);
				NPM_humi = word(data[2], data[3]);
				debug_outln_verbose(F("Temperature (°C): "), String(NPM_temp / 100.0f));
				debug_outln_verbose(F("Relative humidity (%): "), String(NPM_humi / 100.0f));
			}
			NPM_waiting_for_8 = NPM_REPLY_CHECKSUM_8;
			break;
		case NPM_REPLY_CHECKSUM_16:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			NPM_data_reader(test, 8);
			if (NPM_checksum_valid_8(test))
				debug_outln_info(F("Checksum OK..."));
			NPM_waiting_for_8 = NPM_REPLY_HEADER_8;
			break;
		}
	}
	return String(NPM_temp / 100.0f) + " / " + String(NPM_humi / 100.0f);
}

/*****************************************************************
 * write config to spiffs                                        *
 *****************************************************************/
static bool writeConfig()
{

	debug_outln_info(F("cfg::has_matrix: "), String(cfg::has_matrix));

	if (cfg::has_matrix && spiffs_matrix)
	{
		display_update_enable(false); //prevent crash
	}

	//COMPARER SPIFFS ORIGIN ET NEW

	// if (!cfg::has_matrix && spiffs_matrix)
	// {
	// 	display_update_enable(false);
	// }

	// if (cfg::has_matrix && !spiffs_matrix)
	// {
	// 	//rien
	// }

	// if (!cfg::has_matrix && !spiffs_matrix)
	// {
	// 	//rien

	// }

	// if (cfg::has_matrix && spiffs_matrix)
	// {
	// 	// display_update_enable(false);
	// }

	DynamicJsonDocument json(JSON_BUFFER_SIZE);
	debug_outln_info(F("Saving config..."));
	json["SOFTWARE_VERSION"] = SOFTWARE_VERSION;

	for (unsigned e = 0; e < sizeof(configShape) / sizeof(configShape[0]); ++e)
	{
		ConfigShapeEntry c;
		memcpy_P(&c, &configShape[e], sizeof(ConfigShapeEntry));
		switch (c.cfg_type)
		{
		case Config_Type_Bool:
			json[c.cfg_key()].set(*c.cfg_val.as_bool);
			break;
		case Config_Type_UInt:
		case Config_Type_Time:
			json[c.cfg_key()].set(*c.cfg_val.as_uint);
			break;
		case Config_Type_Password:
		case Config_Type_Hex:
		case Config_Type_String:
			json[c.cfg_key()].set(c.cfg_val.as_str);
			break;
		};
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

	SPIFFS.remove(F("/config.json.old"));
	SPIFFS.rename(F("/config.json"), F("/config.json.old"));

	File configFile = SPIFFS.open(F("/config.json"), "w");
	if (configFile)
	{
		serializeJsonPretty(json, Debug);
		serializeJson(json, configFile);
		configFile.close();
		debug_outln_info(F("Config written successfully."));
	}
	else
	{
		debug_outln_error(F("failed to open config file for writing"));
		return false;
	}

#pragma GCC diagnostic pop

	return true;
}

/*****************************************************************
 * read config from spiffs                                       *
 *****************************************************************/

/* backward compatibility for the times when we stored booleans as strings */
static bool boolFromJSON(const DynamicJsonDocument &json, const __FlashStringHelper *key)
{
	if (json[key].is<const char *>())
	{
		return !strcmp_P(json[key].as<const char *>(), PSTR("true"));
	}
	return json[key].as<bool>();
}

static void readConfig(bool oldconfig = false)
{
	bool rewriteConfig = false;

	String cfgName(F("/config.json"));
	if (oldconfig)
	{
		cfgName += F(".old");
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	File configFile = SPIFFS.open(cfgName, "r");
	if (!configFile)
	{
		if (!oldconfig)
		{
			return readConfig(true /* oldconfig */);
		}

		debug_outln_error(F("failed to open config file."));
		return;
	}

	debug_outln_info(F("opened config file..."));
	DynamicJsonDocument json(JSON_BUFFER_SIZE);
	DeserializationError err = deserializeJson(json, configFile.readString());
	configFile.close();
#pragma GCC diagnostic pop

	if (!err)
	{
		serializeJsonPretty(json, Debug);
		debug_outln_info(F("parsed json..."));
		for (unsigned e = 0; e < sizeof(configShape) / sizeof(configShape[0]); ++e)
		{
			ConfigShapeEntry c;
			memcpy_P(&c, &configShape[e], sizeof(ConfigShapeEntry));
			if (json[c.cfg_key()].isNull())
			{
				continue;
			}
			switch (c.cfg_type)
			{
			case Config_Type_Bool:
				*(c.cfg_val.as_bool) = boolFromJSON(json, c.cfg_key());
				break;
			case Config_Type_UInt:
			case Config_Type_Time:
				*(c.cfg_val.as_uint) = json[c.cfg_key()].as<unsigned int>();
				break;
			case Config_Type_String:
			case Config_Type_Hex:
			case Config_Type_Password:
				strncpy(c.cfg_val.as_str, json[c.cfg_key()].as<const char *>(), c.cfg_len);
				c.cfg_val.as_str[c.cfg_len] = '\0';
				break;
			};
		}
		String writtenVersion(json["SOFTWARE_VERSION"].as<const char *>());
		if (writtenVersion.length() && writtenVersion[0] == 'N' && SOFTWARE_VERSION != writtenVersion)
		{
			debug_outln_info(F("Rewriting old config from: "), writtenVersion);
			// would like to do that, but this would wipe firmware.old which the two stage loader
			// might still need
			// SPIFFS.format();
			rewriteConfig = true;
		}
		if (cfg::sending_intervall_ms < READINGTIME_SDS_MS)
		{
			cfg::sending_intervall_ms = READINGTIME_SDS_MS;
		}
		if (boolFromJSON(json, F("bmp280_read")) || boolFromJSON(json, F("bme280_read")))
		{
			cfg::bmx280_read = true;
			rewriteConfig = true;
		}
	}
	else
	{
		debug_outln_error(F("failed to load json config"));

		if (!oldconfig)
		{
			return readConfig(true /* oldconfig */);
		}
	}

	if (rewriteConfig)
	{
		writeConfig();
	}
}

static void init_config()
{

	debug_outln_info(F("mounting FS..."));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

	bool spiffs_begin_ok = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);

#pragma GCC diagnostic pop

	if (!spiffs_begin_ok)
	{
		debug_outln_error(F("failed to mount FS"));
		return;
	}
	readConfig();
}

/*****************************************************************
 * Prepare information for data Loggers                          *
 *****************************************************************/
static void createLoggerConfigs()
{
	loggerConfigs[LoggerSensorCommunity].destport = PORT_DUSTI;
	if (cfg::send2dusti && cfg::ssl_dusti)
	{
		loggerConfigs[LoggerSensorCommunity].destport = 443;
	}

	loggerConfigs[LoggerMadavi].destport = PORT_MADAVI;
	if (cfg::send2madavi && cfg::ssl_madavi)
	{
		loggerConfigs[LoggerMadavi].destport = 443;
	}

	loggerConfigs[LoggerCustom].destport = cfg::port_custom;
	if (cfg::send2custom && cfg::ssl_custom)
	{
		loggerConfigs[LoggerCustom].destport = 443;
	}
	loggerConfigs[LoggerCustom2].destport = cfg::port_custom2;
	if (cfg::send2custom2 && cfg::ssl_custom2)
	{
		loggerConfigs[LoggerCustom2].destport = 443;
	}
}

/*****************************************************************
 * dew point helper function                                     *
 *****************************************************************/
static float dew_point(const float temperature, const float humidity)
{
	float dew_temp;
	const float k2 = 17.62;
	const float k3 = 243.12;

	dew_temp = k3 * (((k2 * temperature) / (k3 + temperature)) + log(humidity / 100.0f)) / (((k2 * k3) / (k3 + temperature)) - log(humidity / 100.0f));

	return dew_temp;
}

/*****************************************************************
 * Pressure at sea level function                                     *
 *****************************************************************/
static float pressure_at_sealevel(const float temperature, const float pressure)
{
	float pressure_at_sealevel;

	pressure_at_sealevel = pressure * pow(((temperature + 273.15f) / (temperature + 273.15f + (0.0065f * readCorrectionOffset(cfg::height_above_sealevel)))), -5.255f);

	return pressure_at_sealevel;
}

/*****************************************************************
 * html helper functions                                         *
 *****************************************************************/
static void start_html_page(String &page_content, const String &title)
{
	last_page_load = millis();

	RESERVE_STRING(s, LARGE_STR);
	s = FPSTR(WEB_PAGE_HEADER);
	s.replace("{t}", title);
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), s);

	server.sendContent_P(WEB_PAGE_HEADER_HEAD);

	s = FPSTR(WEB_PAGE_HEADER_BODY);
	s.replace("{t}", title);
	if (title != " ")
	{
		s.replace("{n}", F("&raquo;"));
	}
	else
	{
		s.replace("{n}", emptyString);
	}
	s.replace("{id}", esp_chipid);
	// s.replace("{macid}", esp_mac_id);
	// s.replace("{mac}", WiFi.macAddress());
	page_content += s;
}

static void end_html_page(String &page_content)
{
	if (page_content.length())
	{
		server.sendContent(page_content);
	}
	server.sendContent_P(WEB_PAGE_FOOTER);
}

static void add_form_input(String &page_content, const ConfigShapeId cfgid, const __FlashStringHelper *info, const int length)
{
	RESERVE_STRING(s, MED_STR);
	s = F("<tr>"
		  "<td title='[&lt;= {l}]'>{i}:&nbsp;</td>"
		  "<td style='width:{l}em'>"
		  "<input type='{t}' name='{n}' id='{n}' placeholder='{i}' value='{v}' maxlength='{l}'/>"
		  "</td></tr>");
	String t_value;
	ConfigShapeEntry c;
	memcpy_P(&c, &configShape[cfgid], sizeof(ConfigShapeEntry));
	switch (c.cfg_type)
	{
	case Config_Type_UInt:
		t_value = String(*c.cfg_val.as_uint);
		s.replace("{t}", F("number"));
		break;
	case Config_Type_Time:
		t_value = String((*c.cfg_val.as_uint) / 1000);
		s.replace("{t}", F("number"));
		break;
	case Config_Type_Password:
		s.replace("{t}", F("text"));
		info = FPSTR(INTL_PASSWORD);
	case Config_Type_Hex:
		s.replace("{t}", F("hex"));
	default:
		t_value = c.cfg_val.as_str;
		t_value.replace("'", "&#39;");
		s.replace("{t}", F("text"));
	}
	s.replace("{i}", info);
	s.replace("{n}", String(c.cfg_key()));
	s.replace("{v}", t_value);
	s.replace("{l}", String(length));
	page_content += s;
}

static String form_checkbox(const ConfigShapeId cfgid, const String &info, const bool linebreak)
{
	RESERVE_STRING(s, MED_STR);
	s = F("<label for='{n}'>"
		  "<input type='checkbox' name='{n}' value='1' id='{n}' {c}/>"
		  "<input type='hidden' name='{n}' value='0'/>"
		  "{i}</label><br/>");
	if (*configShape[cfgid].cfg_val.as_bool)
	{
		s.replace("{c}", F(" checked='checked'"));
	}
	else
	{
		s.replace("{c}", emptyString);
	};
	s.replace("{i}", info);
	s.replace("{n}", String(configShape[cfgid].cfg_key()));
	if (!linebreak)
	{
		s.replace("<br/>", emptyString);
	}
	return s;
}

static String form_submit(const String &value)
{
	String s = F("<tr>"
				 "<td>&nbsp;</td>"
				 "<td>"
				 "<input type='submit' name='submit' value='{v}' />"
				 "</td>"
				 "</tr>");
	s.replace("{v}", value);
	return s;
}

static String form_select_lang()
{
	String s_select = F(" selected='selected'");
	String s = F("<tr>"
				 "<td>" INTL_LANGUAGE ":&nbsp;</td>"
				 "<td>"
				 "<select id='current_lang' name='current_lang'>"
				 "<option value='FR'>Français (FR)</option>"
				 "<option value='EN'>English (EN)</option>"
				 "</select>"
				 "</td>"
				 "</tr>");

	s.replace("'" + String(cfg::current_lang) + "'>", "'" + String(cfg::current_lang) + "'" + s_select + ">");
	return s;
}

static void add_warning_first_cycle(String &page_content)
{
	String s = FPSTR(INTL_TIME_TO_FIRST_MEASUREMENT);
	unsigned int time_to_first = cfg::sending_intervall_ms - msSince(starttime);
	if (time_to_first > cfg::sending_intervall_ms)
	{
		time_to_first = 0;
	}
	s.replace("{v}", String(((time_to_first + 500) / 1000)));
	page_content += s;
}

static void add_age_last_values(String &s)
{
	s += "<b>";
	unsigned int time_since_last = msSince(starttime);
	if (time_since_last > cfg::sending_intervall_ms)
	{
		time_since_last = 0;
	}
	s += String((time_since_last + 500) / 1000);
	s += FPSTR(INTL_TIME_SINCE_LAST_MEASUREMENT);
	s += FPSTR(WEB_B_BR_BR);
}

/*****************************************************************
 * Webserver request auth: prompt for BasicAuth
 *
 * -Provide BasicAuth for all page contexts except /values and images
 *****************************************************************/
static bool webserver_request_auth()
{
	if (cfg::www_basicauth_enabled && !wificonfig_loop)
	{
		debug_outln_info(F("validate request auth..."));
		if (!server.authenticate(cfg::www_username, cfg::www_password))
		{
			server.requestAuthentication(BASIC_AUTH, "Sensor Login", F("Authentication failed"));
			return false;
		}
	}
	return true;
}

static void sendHttpRedirect()
{
	server.sendHeader(F("Location"), F("http://192.168.4.1/config"));
	server.send(302, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), emptyString);
}

/*****************************************************************
 * Webserver root: show all options                              *
 *****************************************************************/
static void webserver_root()
{
	// Reactivate the interrupt to turn on the matrix if the user return to homepage

	if (cfg::has_matrix)
	{
		display_update_enable(true);
	}

	if (WiFi.status() != WL_CONNECTED)
	{
		sendHttpRedirect();
	}
	else
	{
		if (!webserver_request_auth())
		{
			return;
		}

		RESERVE_STRING(page_content, XLARGE_STR);
		start_html_page(page_content, emptyString);
		debug_outln_info(F("ws: root ..."));

		// Enable Pagination
		page_content += FPSTR(WEB_ROOT_PAGE_CONTENT);
		page_content.replace(F("{t}"), FPSTR(INTL_CURRENT_DATA));
		page_content.replace(F("{s}"), FPSTR(INTL_DEVICE_STATUS));
		page_content.replace(F("{conf}"), FPSTR(INTL_CONFIGURATION));
		page_content.replace(F("{restart}"), FPSTR(INTL_RESTART_SENSOR));
		page_content.replace(F("{debug}"), FPSTR(INTL_DEBUG_LEVEL));
		end_html_page(page_content);
	}
}

/*****************************************************************
 * Webserver config: show config page                            *
 *****************************************************************/
static void webserver_config_send_body_get(String &page_content)
{
	auto add_form_checkbox = [&page_content](const ConfigShapeId cfgid, const String &info)
	{
		page_content += form_checkbox(cfgid, info, true);
	};

	auto add_form_checkbox_sensor = [&add_form_checkbox](const ConfigShapeId cfgid, __const __FlashStringHelper *info)
	{
		add_form_checkbox(cfgid, add_sensor_type(info));
	};

	debug_outln_info(F("begin webserver_config_body_get ..."));
	page_content += F("<form method='POST' action='/config' style='width:100%;'>\n"
					  "<input class='radio' id='r1' name='group' type='radio' checked>"
					  "<input class='radio' id='r2' name='group' type='radio'>"
					  "<input class='radio' id='r3' name='group' type='radio'>"
					  "<input class='radio' id='r4' name='group' type='radio'>"
					  "<div class='tabs'>"
					  "<label class='tab' id='tab1' for='r1'>" INTL_WIFI_SETTINGS "</label>"
					  "<label class='tab' id='tab2' for='r2'>");
	page_content += FPSTR(INTL_MORE_SETTINGS);
	page_content += F("</label>"
					  "<label class='tab' id='tab3' for='r3'>");
	page_content += FPSTR(INTL_SENSORS);
	page_content += F(
		"</label>"
		"<label class='tab' id='tab4' for='r4'>APIs");
	// page_content += F("</label>"
	// 				  "<label class='tab' id='tab6' for='r6'>");
	// page_content += FPSTR(INTL_SCREENS);
	page_content += F("</label></div><div class='panels'>"
					  "<div class='panel' id='panel1'>");

	if (wificonfig_loop)
	{ // scan for wlan ssids
		page_content += F("<div id='wifilist'>" INTL_WIFI_NETWORKS "</div><br/>");
	}
	add_form_checkbox(Config_has_wifi, FPSTR(INTL_WIFI_ACTIVATION));
	page_content += FPSTR(TABLE_TAG_OPEN);
	add_form_input(page_content, Config_wlanssid, FPSTR(INTL_FS_WIFI_NAME), LEN_WLANSSID - 1);
	add_form_input(page_content, Config_wlanpwd, FPSTR(INTL_PASSWORD), LEN_CFG_PASSWORD - 1);
	page_content += FPSTR(TABLE_TAG_CLOSE_BR);
	page_content += F("<hr/>\n<br/><b>");

	page_content += FPSTR(INTL_AB_HIER_NUR_ANDERN);
	page_content += FPSTR(WEB_B_BR);
	page_content += FPSTR(BR_TAG);

	// Paginate page after ~ 1500 Bytes
	server.sendContent(page_content);
	page_content = emptyString;

	add_form_checkbox(Config_www_basicauth_enabled, FPSTR(INTL_BASICAUTH));
	page_content += FPSTR(TABLE_TAG_OPEN);
	add_form_input(page_content, Config_www_username, FPSTR(INTL_USER), LEN_WWW_USERNAME - 1);
	add_form_input(page_content, Config_www_password, FPSTR(INTL_PASSWORD), LEN_CFG_PASSWORD - 1);
	page_content += FPSTR(TABLE_TAG_CLOSE_BR);
	page_content += FPSTR(BR_TAG);

	// Paginate page after ~ 1500 Bytes
	server.sendContent(page_content);

	if (!wificonfig_loop)
	{
		page_content = FPSTR(INTL_FS_WIFI_DESCRIPTION);
		page_content += FPSTR(BR_TAG);

		page_content += FPSTR(TABLE_TAG_OPEN);
		add_form_input(page_content, Config_fs_ssid, FPSTR(INTL_FS_WIFI_NAME), LEN_FS_SSID - 1);
		add_form_input(page_content, Config_fs_pwd, FPSTR(INTL_PASSWORD), LEN_CFG_PASSWORD - 1);
		page_content += FPSTR(TABLE_TAG_CLOSE_BR);

		// Paginate page after ~ 1500 Bytes
		server.sendContent(page_content);
	}

	page_content = tmpl(FPSTR(WEB_DIV_PANEL), String(2));

	page_content += F("<b>" INTL_LOCATION "</b>&nbsp;");
	page_content += FPSTR(TABLE_TAG_OPEN);
	add_form_input(page_content, Config_height_above_sealevel, FPSTR(INTL_HEIGHT_ABOVE_SEALEVEL), LEN_HEIGHT_ABOVE_SEALEVEL - 1);
	page_content += FPSTR(TABLE_TAG_CLOSE_BR);

	// Paginate page after ~ 1500 Bytes

	server.sendContent(page_content);
	page_content = emptyString;

	page_content = FPSTR(WEB_BR_LF_B);
	page_content += F(INTL_DISPLAY);
	page_content += FPSTR(WEB_B_BR);
	add_form_checkbox(Config_has_ssd1306, FPSTR(INTL_SSD1306));
	add_form_checkbox(Config_has_matrix, FPSTR(INTL_MATRIX));
	add_form_checkbox(Config_display_measure, FPSTR(INTL_DISPLAY_MEASURES));
	add_form_checkbox(Config_display_forecast, FPSTR(INTL_DISPLAY_FORECAST));
	add_form_checkbox(Config_display_logo, FPSTR(INTL_DISPLAY_LOGO));
	add_form_checkbox(Config_display_alert, FPSTR(INTL_DISPLAY_ALERT));
	add_form_checkbox(Config_display_wifi_info, FPSTR(INTL_DISPLAY_WIFI_INFO));
	add_form_checkbox(Config_display_device_info, FPSTR(INTL_DISPLAY_DEVICE_INFO));

	server.sendContent(page_content);

	// page_content = FPSTR(WEB_BR_LF_B);
	// page_content += F(INTL_ONLINE_CONFIG "</b>&nbsp;");
	// add_form_checkbox(Config_online_config, FPSTR(INTL_ALLOW));
	// server.sendContent(page_content);

	page_content = FPSTR(WEB_BR_LF_B);
	page_content += F(INTL_FIRMWARE "</b>&nbsp;");

	// page_content += FPSTR(TABLE_TAG_OPEN);
	// page_content += form_select_lang();
	// page_content += FPSTR(TABLE_TAG_CLOSE_BR);

	page_content += FPSTR(TABLE_TAG_OPEN);
	add_form_input(page_content, Config_debug, FPSTR(INTL_DEBUG_LEVEL), 1);
	add_form_input(page_content, Config_sending_intervall_ms, FPSTR(INTL_MEASUREMENT_INTERVAL), 5);
	add_form_input(page_content, Config_screen_intervall_ms, FPSTR(INTL_SCREEN_INTERVAL), 5);
	add_form_input(page_content, Config_time_for_wifi_config, FPSTR(INTL_DURATION_ROUTER_MODE), 5);
	page_content += FPSTR(TABLE_TAG_CLOSE_BR);

	server.sendContent(page_content);

	//ICI

	page_content = tmpl(FPSTR(WEB_DIV_PANEL), String(3));

	page_content += FPSTR("<b>");
	page_content += FPSTR(INTL_PM_SENSORS);
	page_content += FPSTR(WEB_B_BR);
	add_form_checkbox_sensor(Config_sds_read, FPSTR(INTL_SDS011));
	add_form_checkbox_sensor(Config_npm_read, FPSTR(INTL_NPM));
	// add_form_checkbox_sensor(Config_npm_fulltime, FPSTR(INTL_NPM_FULLTIME));

	// Paginate page after ~ 1500 Bytes  //ATTENTION RYTHME PAGINATION !
	server.sendContent(page_content);
	page_content = emptyString;

	page_content += FPSTR(WEB_BR_LF_B);
	page_content += FPSTR(INTL_THP_SENSORS);
	page_content += FPSTR(WEB_B_BR);

	add_form_checkbox_sensor(Config_bmx280_read, FPSTR(INTL_BMX280));

	// // Paginate page after ~ 1500 Bytes
	// server.sendContent(page_content);
	// page_content = emptyString;

	page_content += FPSTR(WEB_BR_LF_B);
	page_content += FPSTR(INTL_CO2_SENSORS);
	page_content += FPSTR(WEB_B_BR);

	add_form_checkbox_sensor(Config_mhz16_read, FPSTR(INTL_MHZ16));
	add_form_checkbox_sensor(Config_mhz19_read, FPSTR(INTL_MHZ19));

	// // Paginate page after ~ 1500 Bytes
	server.sendContent(page_content);
	page_content = emptyString;

	page_content += FPSTR(WEB_BR_LF_B);
	page_content += FPSTR(INTL_VOC_SENSORS);
	page_content += FPSTR(WEB_B_BR);

	add_form_checkbox_sensor(Config_ccs811_read, FPSTR(INTL_CCS811));

	// Paginate page after ~ 1500 Bytes
	server.sendContent(page_content);
	//page_content = emptyString;

	page_content = tmpl(FPSTR(WEB_DIV_PANEL), String(4));

	// //page_content += tmpl(FPSTR(INTL_SEND_TO), F("APIs"));
	// page_content += tmpl(FPSTR(INTL_SEND_TO), F(""));

	page_content += FPSTR("<b>");
	page_content += FPSTR(INTL_SEND_TO);
	page_content += FPSTR(WEB_B_BR);

	// page_content += FPSTR(BR_TAG);
	page_content += form_checkbox(Config_send2dusti, FPSTR(WEB_SENSORCOMMUNITY), false);
	page_content += FPSTR(WEB_NBSP_NBSP_BRACE);
	page_content += form_checkbox(Config_ssl_dusti, FPSTR(WEB_HTTPS), false);
	page_content += FPSTR(WEB_BRACE_BR);
	// page_content += FPSTR("<br>");
	page_content += form_checkbox(Config_send2madavi, FPSTR(WEB_MADAVI), false);
	page_content += FPSTR(WEB_NBSP_NBSP_BRACE);
	page_content += form_checkbox(Config_ssl_madavi, FPSTR(WEB_HTTPS), false);
	page_content += FPSTR(WEB_BRACE_BR);
	// page_content += FPSTR("<br>");

	add_form_checkbox(Config_send2csv, FPSTR(WEB_CSV));

	server.sendContent(page_content);
	page_content = emptyString;

	page_content += FPSTR(BR_TAG);
	page_content += form_checkbox(Config_send2custom, FPSTR(INTL_SEND_TO_OWN_API), false);
	page_content += FPSTR(WEB_NBSP_NBSP_BRACE);
	page_content += form_checkbox(Config_ssl_custom, FPSTR(WEB_HTTPS), false);
	page_content += FPSTR(WEB_BRACE_BR);

	server.sendContent(page_content);
	page_content = FPSTR(TABLE_TAG_OPEN);
	add_form_input(page_content, Config_host_custom, FPSTR(INTL_SERVER), LEN_HOST_CUSTOM - 1);
	add_form_input(page_content, Config_url_custom, FPSTR(INTL_PATH), LEN_URL_CUSTOM - 1);
	add_form_input(page_content, Config_port_custom, FPSTR(INTL_PORT), MAX_PORT_DIGITS);
	add_form_input(page_content, Config_user_custom, FPSTR(INTL_USER), LEN_USER_CUSTOM - 1);
	add_form_input(page_content, Config_pwd_custom, FPSTR(INTL_PASSWORD), LEN_CFG_PASSWORD - 1);
	page_content += FPSTR(TABLE_TAG_CLOSE_BR);

	page_content += FPSTR(BR_TAG);
	page_content += form_checkbox(Config_send2custom2, FPSTR(INTL_SEND_TO_OWN_API2), false);
	page_content += FPSTR(WEB_NBSP_NBSP_BRACE);
	page_content += form_checkbox(Config_ssl_custom2, FPSTR(WEB_HTTPS), false);
	page_content += FPSTR(WEB_BRACE_BR);

	server.sendContent(page_content);
	page_content = emptyString;
	page_content = FPSTR(TABLE_TAG_OPEN);
	add_form_input(page_content, Config_host_custom2, FPSTR(INTL_SERVER2), LEN_HOST_CUSTOM2 - 1);
	add_form_input(page_content, Config_url_custom2, FPSTR(INTL_PATH2), LEN_URL_CUSTOM2 - 1);
	add_form_input(page_content, Config_port_custom2, FPSTR(INTL_PORT2), MAX_PORT_DIGITS2);
	add_form_input(page_content, Config_user_custom2, FPSTR(INTL_USER2), LEN_USER_CUSTOM2 - 1);
	add_form_input(page_content, Config_pwd_custom2, FPSTR(INTL_PASSWORD2), LEN_CFG_PASSWORD2 - 1);
	page_content += FPSTR(TABLE_TAG_CLOSE_BR);

	//server.sendContent(page_content);
	// page_content = tmpl(FPSTR(WEB_DIV_PANEL), String(6));
	// page_content += FPSTR("<b>");
	// page_content += FPSTR(INTL_LOGOS);
	// page_content += FPSTR(WEB_B_BR);

	page_content += F("</div></div>");
	page_content += form_submit(FPSTR(INTL_SAVE_AND_RESTART));
	page_content += FPSTR(BR_TAG);
	page_content += FPSTR(WEB_BR_FORM);
	if (wificonfig_loop)
	{ // scan for wlan ssids
		page_content += F("<script>window.setTimeout(load_wifi_list,1000);</script>");
	}
	server.sendContent(page_content);
	page_content = emptyString;
}

static void webserver_config_send_body_post(String &page_content)
{
	String masked_pwd;

	for (unsigned e = 0; e < sizeof(configShape) / sizeof(configShape[0]); ++e)
	{
		ConfigShapeEntry c;
		memcpy_P(&c, &configShape[e], sizeof(ConfigShapeEntry));
		const String s_param(c.cfg_key());
		if (!server.hasArg(s_param))
		{
			continue;
		}
		const String server_arg(server.arg(s_param));

		switch (c.cfg_type)
		{
		case Config_Type_UInt:
			*(c.cfg_val.as_uint) = server_arg.toInt();
			break;
		case Config_Type_Time:
			*(c.cfg_val.as_uint) = server_arg.toInt() * 1000;
			break;
		case Config_Type_Bool:
			*(c.cfg_val.as_bool) = (server_arg == "1");
			break;
		case Config_Type_String:
			strncpy(c.cfg_val.as_str, server_arg.c_str(), c.cfg_len);
			c.cfg_val.as_str[c.cfg_len] = '\0';
			break;
		case Config_Type_Password:
			if (server_arg.length())
			{
				server_arg.toCharArray(c.cfg_val.as_str, LEN_CFG_PASSWORD);
			}
			break;
		case Config_Type_Hex:
			strncpy(c.cfg_val.as_str, server_arg.c_str(), c.cfg_len);
			c.cfg_val.as_str[c.cfg_len] = '\0';
			break;
		}
	}

	page_content += FPSTR(INTL_SENSOR_IS_REBOOTING);

	server.sendContent(page_content);
	page_content = emptyString;
}

static void sensor_restart()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

	SPIFFS.end();

#pragma GCC diagnostic pop

	if (cfg::npm_read)
	{
		serialNPM.end();
	}
	else
	{
		serialSDS.end();
	}
	debug_outln_info(F("Restart."));
	delay(500);
	ESP.restart();
	// should not be reached
	while (true)
	{
		yield();
	}
}

static void webserver_config()
{
	if (WiFi.getMode() == WIFI_MODE_STA)
	{
		debug_outln_info(F("STA"));
		if (cfg::has_matrix)
		{
			display_update_enable(false);
		}
	}

	if (WiFi.getMode() == WIFI_MODE_AP)
	{
		debug_outln_info(F("AP"));
	}

	if (!webserver_request_auth())
	{
		return;
	}

	debug_outln_info(F("ws: config page ..."));

	server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
	server.sendHeader(F("Pragma"), F("no-cache"));
	server.sendHeader(F("Expires"), F("0"));
	// Enable Pagination (Chunked Transfer)
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);

	RESERVE_STRING(page_content, XLARGE_STR);

	start_html_page(page_content, FPSTR(INTL_CONFIGURATION));
	if (wificonfig_loop)
	{ // scan for wlan ssids
		page_content += FPSTR(WEB_CONFIG_SCRIPT);
	}

	if (server.method() == HTTP_GET)
	{
		webserver_config_send_body_get(page_content);
	}
	else
	{
		webserver_config_send_body_post(page_content);
	}
	end_html_page(page_content);

	if (server.method() == HTTP_POST)
	{
		display_debug(F("Writing config"), emptyString);

		if (writeConfig())
		{
			display_debug(F("Writing config"), F("and restarting"));
			sensor_restart();
		}
	}
}

/*****************************************************************
 * Webserver wifi: show available wifi networks                  *
 *****************************************************************/
static void webserver_wifi()
{
	String page_content;

	debug_outln_info(F("wifi networks found: "), String(count_wifiInfo));
	if (count_wifiInfo == 0)
	{
		page_content += FPSTR(BR_TAG);
		page_content += FPSTR(INTL_NO_NETWORKS);
		page_content += FPSTR(BR_TAG);
	}
	else
	{
		std::unique_ptr<int[]> indices(new int[count_wifiInfo]);
		debug_outln_info(F("ws: wifi ..."));
		for (unsigned i = 0; i < count_wifiInfo; ++i)
		{
			indices[i] = i;
		}
		for (unsigned i = 0; i < count_wifiInfo; i++)
		{
			for (unsigned j = i + 1; j < count_wifiInfo; j++)
			{
				if (wifiInfo[indices[j]].RSSI > wifiInfo[indices[i]].RSSI)
				{
					std::swap(indices[i], indices[j]);
				}
			}
		}
		int duplicateSsids = 0;
		for (int i = 0; i < count_wifiInfo; i++)
		{
			if (indices[i] == -1)
			{
				continue;
			}
			for (int j = i + 1; j < count_wifiInfo; j++)
			{
				if (strncmp(wifiInfo[indices[i]].ssid, wifiInfo[indices[j]].ssid, sizeof(wifiInfo[0].ssid)) == 0)
				{
					indices[j] = -1; // set dup aps to index -1
					++duplicateSsids;
				}
			}
		}

		page_content += FPSTR(INTL_NETWORKS_FOUND);
		page_content += String(count_wifiInfo - duplicateSsids);
		page_content += FPSTR(BR_TAG);
		page_content += FPSTR(BR_TAG);
		page_content += FPSTR(TABLE_TAG_OPEN);
		// if (n > 30) n=30;
		for (int i = 0; i < count_wifiInfo; ++i)
		{
			if (indices[i] == -1)
			{
				continue;
			}
			// Print SSID and RSSI for each network found
			page_content += wlan_ssid_to_table_row(wifiInfo[indices[i]].ssid, ((wifiInfo[indices[i]].encryptionType == WIFI_AUTH_OPEN) ? " " : u8"🔒"), wifiInfo[indices[i]].RSSI);
		}
		page_content += FPSTR(TABLE_TAG_CLOSE_BR);
		page_content += FPSTR(BR_TAG);
	}
	server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
 * Webserver root: show latest values                            *
 *****************************************************************/
static void webserver_values()
{

	if (WiFi.status() != WL_CONNECTED)
	{
		sendHttpRedirect();
		return;
	}

	RESERVE_STRING(page_content, XLARGE_STR);
	start_html_page(page_content, FPSTR(INTL_CURRENT_DATA));
	const String unit_Deg("°");
	const String unit_P("hPa");
	const String unit_T("°C");
	const String unit_CO2("ppm");
	const String unit_COV("ppb");
	const String unit_NC();
	const String unit_LA(F("dB(A)"));
	float dew_point_temp;

	const int signal_quality = calcWiFiSignalQuality(last_signal_strength);
	debug_outln_info(F("ws: values ..."));
	if (!count_sends)
	{
		page_content += F("<b style='color:red'>");
		add_warning_first_cycle(page_content);
		page_content += FPSTR(WEB_B_BR_BR);
	}
	else
	{
		add_age_last_values(page_content);
	}

	auto add_table_pm_value = [&page_content](const __FlashStringHelper *sensor, const __FlashStringHelper *param, const float &value)
	{
		add_table_row_from_value(page_content, sensor, param, check_display_value(value, -1, 1, 0), F("µg/m³"));
	};

	auto add_table_nc_value = [&page_content](const __FlashStringHelper *sensor, const __FlashStringHelper *param, const float value)
	{
		add_table_row_from_value(page_content, sensor, param, check_display_value(value, -1, 1, 0), F("#/cm³"));
	};

	auto add_table_t_value = [&page_content](const __FlashStringHelper *sensor, const __FlashStringHelper *param, const float value)
	{
		add_table_row_from_value(page_content, sensor, param, check_display_value(value, -128, 1, 0), "°C");
	};

	auto add_table_h_value = [&page_content](const __FlashStringHelper *sensor, const __FlashStringHelper *param, const float value)
	{
		add_table_row_from_value(page_content, sensor, param, check_display_value(value, -1, 1, 0), "%");
	};

	auto add_table_co2_value = [&page_content](const __FlashStringHelper *sensor, const __FlashStringHelper *param, const float &value)
	{
		add_table_row_from_value(page_content, sensor, param, check_display_value(value, -1, 1, 0).substring(0, check_display_value(value, -1, 1, 0).indexOf(".")), "ppm"); //remove after .
	};

	auto add_table_voc_value = [&page_content](const __FlashStringHelper *sensor, const __FlashStringHelper *param, const float &value)
	{
		add_table_row_from_value(page_content, sensor, param, check_display_value(value, -1, 1, 0).substring(0, check_display_value(value, -1, 1, 0).indexOf(".")), "ppb"); //remove after .
	};

	auto add_table_value = [&page_content](const __FlashStringHelper *sensor, const __FlashStringHelper *param, const String &value, const String &unit)
	{
		add_table_row_from_value(page_content, sensor, param, value, unit);
	};

	server.sendContent(page_content);
	page_content = F("<table cellspacing='0' cellpadding='5' class='v'>\n"
					 "<thead><tr><th>" INTL_SENSOR "</th><th> " INTL_PARAMETER "</th><th>" INTL_VALUE "</th></tr></thead>");

	if (cfg::sds_read)
	{
		add_table_pm_value(FPSTR(SENSORS_SDS011), FPSTR(WEB_PM25), last_value_SDS_P2);
		add_table_pm_value(FPSTR(SENSORS_SDS011), FPSTR(WEB_PM10), last_value_SDS_P1);
		page_content += FPSTR(EMPTY_ROW);
	}
	if (cfg::npm_read)
	{
		add_table_pm_value(FPSTR(SENSORS_NPM), FPSTR(WEB_PM1), last_value_NPM_P0);
		add_table_pm_value(FPSTR(SENSORS_NPM), FPSTR(WEB_PM25), last_value_NPM_P2);
		add_table_pm_value(FPSTR(SENSORS_NPM), FPSTR(WEB_PM10), last_value_NPM_P1);
		add_table_nc_value(FPSTR(SENSORS_NPM), FPSTR(WEB_NC1k0), last_value_NPM_N1);
		add_table_nc_value(FPSTR(SENSORS_NPM), FPSTR(WEB_NC2k5), last_value_NPM_N25);
		add_table_nc_value(FPSTR(SENSORS_NPM), FPSTR(WEB_NC10), last_value_NPM_N10);
		page_content += FPSTR(EMPTY_ROW);
	}

	if (cfg::bmx280_read)
	{
		const char *const sensor_name = (bmx280.sensorID() == BME280_SENSOR_ID) ? SENSORS_BME280 : SENSORS_BMP280;
		add_table_t_value(FPSTR(sensor_name), FPSTR(INTL_TEMPERATURE), last_value_BMX280_T);
		add_table_value(FPSTR(sensor_name), FPSTR(INTL_PRESSURE), check_display_value(last_value_BMX280_P / 100.0f, (-1 / 100.0f), 2, 0), unit_P);
		add_table_value(FPSTR(sensor_name), FPSTR(INTL_PRESSURE_AT_SEALEVEL), last_value_BMX280_P != -1.0f ? String(pressure_at_sealevel(last_value_BMX280_T, last_value_BMX280_P / 100.0f), 2) : "-", unit_P);
		if (bmx280.sensorID() == BME280_SENSOR_ID)
		{
			add_table_h_value(FPSTR(sensor_name), FPSTR(INTL_HUMIDITY), last_value_BME280_H);
			dew_point_temp = dew_point(last_value_BMX280_T, last_value_BME280_H);
			add_table_value(FPSTR(sensor_name), FPSTR(INTL_DEW_POINT), isnan(dew_point_temp) ? "-" : String(dew_point_temp, 1), unit_T);
		}
		page_content += FPSTR(EMPTY_ROW);
	}

	if (cfg::mhz16_read)
	{
		const char *const sensor_name = SENSORS_MHZ16;
		add_table_co2_value(FPSTR(sensor_name), FPSTR(INTL_CO2), last_value_MHZ16);
		page_content += FPSTR(EMPTY_ROW);
	}

	if (cfg::mhz19_read)
	{
		const char *const sensor_name = SENSORS_MHZ19;
		add_table_co2_value(FPSTR(sensor_name), FPSTR(INTL_CO2), last_value_MHZ19);
		page_content += FPSTR(EMPTY_ROW);
	}

	if (cfg::ccs811_read)
	{
		const char *const sensor_name = SENSORS_CCS811;
		add_table_voc_value(FPSTR(sensor_name), FPSTR(INTL_VOC), last_value_CCS811);
		page_content += FPSTR(EMPTY_ROW);
	}

	server.sendContent(page_content);
	page_content = emptyString;

	add_table_value(F("WiFi"), FPSTR(INTL_SIGNAL_STRENGTH), String(last_signal_strength), "dBm");
	add_table_value(F("WiFi"), FPSTR(INTL_SIGNAL_QUALITY), String(signal_quality), "%");

	page_content += FPSTR(TABLE_TAG_CLOSE_BR);
	page_content += FPSTR(BR_TAG);
	end_html_page(page_content);
}

/*****************************************************************
 * Webserver root: show device status
 *****************************************************************/
static void webserver_status()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		sendHttpRedirect();
		return;
	}

	RESERVE_STRING(page_content, XLARGE_STR);
	start_html_page(page_content, FPSTR(INTL_DEVICE_STATUS));

	debug_outln_info(F("ws: status ..."));
	server.sendContent(page_content);
	page_content = F("<table cellspacing='0' cellpadding='5' class='v'>\n"
					 "<thead><tr><th> " INTL_PARAMETER "</th><th>" INTL_VALUE "</th></tr></thead>");
	String versionHtml(SOFTWARE_VERSION);
	versionHtml += F("/ST:");
	versionHtml += String(!moduleair_selftest_failed);
	versionHtml += '/';
	versionHtml.replace("/", FPSTR(BR_TAG));
	add_table_row_from_value(page_content, FPSTR(INTL_FIRMWARE), versionHtml);
	add_table_row_from_value(page_content, F("Free Memory"), String(ESP.getFreeHeap()));
	time_t now = time(nullptr);
	add_table_row_from_value(page_content, FPSTR(INTL_TIME_UTC), ctime(&now));
	add_table_row_from_value(page_content, F("Uptime"), delayToString(millis() - time_point_device_start_ms));
	if (cfg::sds_read)
	{
		page_content += FPSTR(EMPTY_ROW);
		add_table_row_from_value(page_content, FPSTR(SENSORS_SDS011), last_value_SDS_version);
	}
	if (cfg::npm_read)
	{
		page_content += FPSTR(EMPTY_ROW);
		add_table_row_from_value(page_content, FPSTR(SENSORS_NPM), last_value_NPM_version);
	}
	page_content += FPSTR(EMPTY_ROW);
	page_content += F("<tr><td colspan='2'><b>" INTL_ERROR "</b></td></tr>");
	String wifiStatus(WiFi_error_count);
	wifiStatus += '/';
	wifiStatus += String(last_signal_strength);
	wifiStatus += '/';
	wifiStatus += String(last_disconnect_reason);
	add_table_row_from_value(page_content, F("WiFi"), wifiStatus);

	// if (last_update_returncode != 0)
	// {
	// 	add_table_row_from_value(page_content, F("OTA Return"),
	// 							 last_update_returncode > 0 ? String(last_update_returncode) : HTTPClient::errorToString(last_update_returncode));
	// }
	for (unsigned int i = 0; i < LoggerCount; ++i)
	{
		if (loggerConfigs[i].errors)
		{
			const __FlashStringHelper *logger = loggerDescription(i);
			if (logger)
			{
				add_table_row_from_value(page_content, logger, String(loggerConfigs[i].errors));
			}
		}
	}

	if (last_sendData_returncode != 0)
	{
		add_table_row_from_value(page_content, F("Data Send Return"),
								 last_sendData_returncode > 0 ? String(last_sendData_returncode) : HTTPClient::errorToString(last_sendData_returncode));
	}
	if (cfg::sds_read)
	{
		add_table_row_from_value(page_content, FPSTR(SENSORS_SDS011), String(SDS_error_count));
	}
	if (cfg::npm_read)
	{
		add_table_row_from_value(page_content, FPSTR(SENSORS_NPM), String(NPM_error_count));
	}
	if (cfg::mhz16_read)
	{
		add_table_row_from_value(page_content, FPSTR(SENSORS_MHZ16), String(MHZ16_error_count));
	}
	if (cfg::mhz19_read)
	{
		add_table_row_from_value(page_content, FPSTR(SENSORS_MHZ19), String(MHZ19_error_count));
	}
	if (cfg::ccs811_read)
	{
		add_table_row_from_value(page_content, FPSTR(SENSORS_CCS811), String(CCS811_error_count));
	}
	server.sendContent(page_content);
	page_content = emptyString;

	if (count_sends > 0)
	{
		page_content += FPSTR(EMPTY_ROW);
		add_table_row_from_value(page_content, F(INTL_NUMBER_OF_MEASUREMENTS), String(count_sends));
		if (sending_time > 0)
		{
			add_table_row_from_value(page_content, F(INTL_TIME_SENDING_MS), String(sending_time), "ms");
		}
	}

	page_content += FPSTR(TABLE_TAG_CLOSE_BR);
	end_html_page(page_content);
}

/*****************************************************************
 * Webserver read serial ring buffer                             *
 *****************************************************************/
static void webserver_serial()
{
	String s(Debug.popLines());

	server.send(s.length() ? 200 : 204, FPSTR(TXT_CONTENT_TYPE_TEXT_PLAIN), s);
}

/*****************************************************************
 * Webserver set debug level                                     *
 *****************************************************************/
static void webserver_debug_level()
{
	if (!webserver_request_auth())
	{
		return;
	}

	RESERVE_STRING(page_content, LARGE_STR);
	start_html_page(page_content, FPSTR(INTL_DEBUG_LEVEL));

	if (server.hasArg("lvl"))
	{
		debug_outln_info(F("ws: debug level ..."));

		const int lvl = server.arg("lvl").toInt();
		if (lvl >= 0 && lvl <= 5)
		{
			cfg::debug = lvl;
			page_content += F("<h3>");
			page_content += FPSTR(INTL_DEBUG_SETTING_TO);
			page_content += ' ';

			const __FlashStringHelper *lvlText;
			switch (lvl)
			{
			case DEBUG_ERROR:
				lvlText = F(INTL_ERROR);
				break;
			case DEBUG_WARNING:
				lvlText = F(INTL_WARNING);
				break;
			case DEBUG_MIN_INFO:
				lvlText = F(INTL_MIN_INFO);
				break;
			case DEBUG_MED_INFO:
				lvlText = F(INTL_MED_INFO);
				break;
			case DEBUG_MAX_INFO:
				lvlText = F(INTL_MAX_INFO);
				break;
			default:
				lvlText = F(INTL_NONE);
			}

			page_content += lvlText;
			page_content += F(".</h3>");
		}
	}

	page_content += F("<br/><pre id='slog' class='panels'>");
	page_content += Debug.popLines();
	page_content += F("</pre>");
	page_content += F("<script>"
					  "function slog_update() {"
					  "fetch('/serial').then(r => r.text()).then((r) => {"
					  "document.getElementById('slog').innerText += r;}).catch(err => console.log(err));};"
					  "setInterval(slog_update, 3000);"
					  "</script>");
	page_content += F("<h4>");
	page_content += FPSTR(INTL_DEBUG_SETTING_TO);
	page_content += F("</h4>"
					  "<table style='width:100%;'>"
					  "<tr><td style='width:25%;'><a class='b' href='/debug?lvl=0'>" INTL_NONE "</a></td>"
					  "<td style='width:25%;'><a class='b' href='/debug?lvl=1'>" INTL_ERROR "</a></td>"
					  "<td style='width:25%;'><a class='b' href='/debug?lvl=3'>" INTL_MIN_INFO "</a></td>"
					  "<td style='width:25%;'><a class='b' href='/debug?lvl=5'>" INTL_MAX_INFO "</a></td>"
					  "</tr><tr>"
					  "</tr>"
					  "</table>");

	end_html_page(page_content);
}

/*****************************************************************
 * Webserver remove config                                       *
 *****************************************************************/
static void webserver_removeConfig()
{
	// For any work with SPIFFS or server, the interrupts must be deactivated. The matrix is turned off.

	if (cfg::has_matrix)
	{
		display_update_enable(false);
	}

	if (!webserver_request_auth())
	{
		return;
	}

	RESERVE_STRING(page_content, LARGE_STR);
	start_html_page(page_content, FPSTR(INTL_DELETE_CONFIG));
	debug_outln_info(F("ws: removeConfig ..."));

	if (server.method() == HTTP_GET)
	{
		page_content += FPSTR(WEB_REMOVE_CONFIG_CONTENT);
	}
	else
	{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		// Silently remove the desaster backup
		SPIFFS.remove(F("/config.json.old"));
		if (SPIFFS.exists(F("/config.json")))
		{ // file exists
			debug_outln_info(F("removing config.json..."));
			if (SPIFFS.remove(F("/config.json")))
			{
				page_content += F("<h3>" INTL_CONFIG_DELETED ".</h3>");
			}
			else
			{
				page_content += F("<h3>" INTL_CONFIG_CAN_NOT_BE_DELETED ".</h3>");
			}
		}
		else
		{
			page_content += F("<h3>" INTL_CONFIG_NOT_FOUND ".</h3>");
		}
#pragma GCC diagnostic pop
	}
	end_html_page(page_content);
}

/*****************************************************************
 * Webserver reset NodeMCU                                       *
 *****************************************************************/
static void webserver_reset()
{
	// For any work with SPIFFS or server, the interrupts must be deactivated. The matrix is turned off.

	if (cfg::has_matrix)
	{
		display_update_enable(false);
	}

	if (!webserver_request_auth())
	{
		return;
	}

	String page_content;
	page_content.reserve(512);

	start_html_page(page_content, FPSTR(INTL_RESTART_SENSOR));
	debug_outln_info(F("ws: reset ..."));

	if (server.method() == HTTP_GET)
	{
		page_content += FPSTR(WEB_RESET_CONTENT);
	}
	else
	{

		sensor_restart();
	}
	end_html_page(page_content);
}

/*****************************************************************
 * Webserver data.json                                           *
 *****************************************************************/
static void webserver_data_json()
{
	String s1;
	unsigned long age = 0;

	debug_outln_info(F("ws: data json..."));
	if (!count_sends)
	{
		s1 = FPSTR(data_first_part);
		s1 += "]}";
		age = cfg::sending_intervall_ms - msSince(starttime);
		if (age > cfg::sending_intervall_ms)
		{
			age = 0;
		}
		age = 0 - age;
	}
	else
	{
		s1 = last_data_string;
		age = msSince(starttime);
		if (age > cfg::sending_intervall_ms)
		{
			age = 0;
		}
	}
	String s2 = F(", \"age\":\"");
	s2 += String((long)((age + 500) / 1000));
	s2 += F("\", \"sensordatavalues\"");
	s1.replace(F(", \"sensordatavalues\""), s2);
	server.send(200, FPSTR(TXT_CONTENT_TYPE_JSON), s1);
}

/*****************************************************************
 * Webserver metrics endpoint                                    *
 *****************************************************************/
static void webserver_metrics_endpoint()
{
	debug_outln_info(F("ws: /metrics"));
	RESERVE_STRING(page_content, XLARGE_STR);
	page_content = F("software_version{version=\"" SOFTWARE_VERSION_STR "\",$i} 1\nuptime_ms{$i} $u\nsending_intervall_ms{$i} $s\nnumber_of_measurements{$i} $c\n");
	String id(F("node=\"" SENSOR_BASENAME));
	//String id(F("node=\"" HOSTNAME_BASE));
	id += esp_chipid;
	id += '\"';
	page_content.replace("$i", id);
	page_content.replace("$u", String(msSince(time_point_device_start_ms)));
	page_content.replace("$s", String(cfg::sending_intervall_ms));
	page_content.replace("$c", String(count_sends));
	DynamicJsonDocument json2data(JSON_BUFFER_SIZE);
	DeserializationError err = deserializeJson(json2data, last_data_string);
	if (!err)
	{
		for (JsonObject measurement : json2data[FPSTR(JSON_SENSOR_DATA_VALUES)].as<JsonArray>())
		{
			page_content += measurement["value_type"].as<const char *>();
			page_content += '{';
			page_content += id;
			page_content += "} ";
			page_content += measurement["value"].as<const char *>();
			page_content += '\n';
		}
		page_content += F("last_sample_age_ms{");
		page_content += id;
		page_content += "} ";
		page_content += String(msSince(starttime));
		page_content += '\n';
	}
	else
	{
		debug_outln_error(FPSTR(DBG_TXT_DATA_READ_FAILED));
	}
	page_content += F("# EOF\n");
	debug_outln(page_content, DEBUG_MED_INFO);
	server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_PLAIN), page_content);
}

/*****************************************************************
 * Webserver Images                                              *
 *****************************************************************/

static void webserver_favicon()
{
	server.sendHeader(F("Cache-Control"), F("max-age=2592000, public"));

	server.send_P(200, TXT_CONTENT_TYPE_IMAGE_PNG,
				  AIRCARTO_INFO_LOGO_PNG, AIRCARTO_INFO_LOGO_PNG_SIZE);
}

/*****************************************************************
 * Webserver page not found                                      *
 *****************************************************************/
static void webserver_not_found()
{

	last_page_load = millis();
	debug_outln_info(F("ws: not found ..."));

	if (WiFi.status() != WL_CONNECTED)
	{
		if ((server.uri().indexOf(F("success.html")) != -1) || (server.uri().indexOf(F("detect.html")) != -1))
		{
			server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), FPSTR(WEB_IOS_REDIRECT));
		}
		else
		{
			sendHttpRedirect();
		}
	}
	else
	{
		server.send(404, FPSTR(TXT_CONTENT_TYPE_TEXT_PLAIN), F("Not found."));
	}
}

static void webserver_static()
{
	server.sendHeader(F("Cache-Control"), F("max-age=2592000, public"));

	if (server.arg(String('r')) == F("logo"))
	{

		server.send_P(200, TXT_CONTENT_TYPE_IMAGE_PNG,
					  AIRCARTO_INFO_LOGO_PNG, AIRCARTO_INFO_LOGO_PNG_SIZE);
	}
	else if (server.arg(String('r')) == F("css"))
	{
		server.send_P(200, TXT_CONTENT_TYPE_TEXT_CSS,
					  WEB_PAGE_STATIC_CSS, sizeof(WEB_PAGE_STATIC_CSS) - 1);
	}
	else
	{
		webserver_not_found();
	}
}

/*****************************************************************
 * Webserver setup                                               *
 *****************************************************************/
static void setup_webserver()
{
	server.on("/", webserver_root);
	server.on(F("/config"), webserver_config);
	server.on(F("/wifi"), webserver_wifi);
	server.on(F("/values"), webserver_values);
	server.on(F("/status"), webserver_status);
	server.on(F("/generate_204"), webserver_config);
	server.on(F("/fwlink"), webserver_config);
	server.on(F("/debug"), webserver_debug_level);
	server.on(F("/serial"), webserver_serial);
	server.on(F("/removeConfig"), webserver_removeConfig);
	server.on(F("/reset"), webserver_reset);
	server.on(F("/data.json"), webserver_data_json);
	server.on(F("/metrics"), webserver_metrics_endpoint);
	server.on(F("/favicon.ico"), webserver_favicon);
	server.on(F(STATIC_PREFIX), webserver_static);
	server.onNotFound(webserver_not_found);
	debug_outln_info(F("Starting Webserver... "));
	server.begin();
}

static int selectChannelForAp()
{
	std::array<int, 14> channels_rssi;
	std::fill(channels_rssi.begin(), channels_rssi.end(), -100);

	for (unsigned i = 0; i < std::min((uint8_t)14, count_wifiInfo); i++)
	{
		if (wifiInfo[i].RSSI > channels_rssi[wifiInfo[i].channel])
		{
			channels_rssi[wifiInfo[i].channel] = wifiInfo[i].RSSI;
		}
	}

	if ((channels_rssi[1] < channels_rssi[6]) && (channels_rssi[1] < channels_rssi[11]))
	{
		return 1;
	}
	else if ((channels_rssi[6] < channels_rssi[1]) && (channels_rssi[6] < channels_rssi[11]))
	{
		return 6;
	}
	else
	{
		return 11;
	}
}

/*****************************************************************
 * WifiConfig                                                    *
 *****************************************************************/

static void wifiConfig()
{

	if (cfg::has_matrix)
	{
		display.fillScreen(myBLACK);
		display.setTextColor(myWHITE);
		display.setFont(NULL);
		display.setTextSize(1);
		display.setCursor(1, 0);
		display.print(INTL_CONFIGURE);
		display.setCursor(1, 11);
		display.print(INTL_THE_WIFI);
		display.setCursor(1, 22);
		display.print("3 min.");

		for (int i = 0; i < 5; i++)
		{
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop1);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop2);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop3);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop4);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop5);
			delay(200);
		}
		display.fillScreen(myBLACK);
		display_update_enable(false);
	}

	debug_outln_info(F("Starting WiFiManager"));
	debug_outln_info(F("AP ID: "), String(cfg::fs_ssid));
	debug_outln_info(F("Password: "), String(cfg::fs_pwd));

	wificonfig_loop = true;
	WiFi.disconnect(true, true);

	debug_outln_info(F("scan for wifi networks..."));
	int8_t scanReturnCode = WiFi.scanNetworks(false /* scan async */, true /* show hidden networks */);
	if (scanReturnCode < 0)
	{
		debug_outln_error(F("WiFi scan failed. Treating as empty. "));
		count_wifiInfo = 0;
	}
	else
	{
		count_wifiInfo = (uint8_t)scanReturnCode;
	}

	delete[] wifiInfo;
	wifiInfo = new struct_wifiInfo[std::max(count_wifiInfo, (uint8_t)1)];

	for (unsigned i = 0; i < count_wifiInfo; i++)
	{
		String SSID;
		uint8_t *BSSID;

		memset(&wifiInfo[i], 0, sizeof(struct_wifiInfo));
		WiFi.getNetworkInfo(i, SSID, wifiInfo[i].encryptionType, wifiInfo[i].RSSI, BSSID, wifiInfo[i].channel);
		SSID.toCharArray(wifiInfo[i].ssid, sizeof(wifiInfo[0].ssid));
	}

	// Use 13 channels if locale is not "EN"
	wifi_country_t wifi;
	wifi.policy = WIFI_COUNTRY_POLICY_MANUAL;
	strcpy(wifi.cc, INTL_LANG);
	wifi.nchan = (INTL_LANG[0] == 'E' && INTL_LANG[1] == 'N') ? 11 : 13;
	wifi.schan = 1;

	WiFi.mode(WIFI_AP);
	const IPAddress apIP(192, 168, 4, 1);
	WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
	WiFi.softAP(cfg::fs_ssid, cfg::fs_pwd, selectChannelForAp());
	// In case we create a unique password at first start
	debug_outln_info(F("AP Password is: "), cfg::fs_pwd);

	DNSServer dnsServer;
	// Ensure we don't poison the client DNS cache
	dnsServer.setTTL(0);
	dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer.start(53, "*", apIP); // 53 is port for DNS server

	setup_webserver();

	// 10 minutes timeout for wifi config
	last_page_load = millis();

	if (cfg::has_matrix)
	{
		display_update_enable(true); //reactivate matrix during the X min config time  ATTENTION ICI true/false?
		display.fillScreen(myBLACK);
		display.setTextColor(myWHITE);
		display.setFont(NULL);
		display.setTextSize(1);
		display.setCursor(1, 0);
		display.print(INTL_CONFIGURE);
		display.setCursor(1, 11);
		display.print(INTL_THE_WIFI);
		display.setCursor(1, 22);
		display.print("3 min.");
		for (int i = 0; i < 5; i++)
		{
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop1);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop2);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop3);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop4);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop5);
			delay(200);
		}
		time_before_config = millis();
	}

	while ((millis() - last_page_load) < cfg::time_for_wifi_config + 500)
	{
		dnsServer.processNextRequest();
		server.handleClient();
		yield();

		int div_entiere = (millis() - time_before_config) / 200;

		if (prec != div_entiere)
		{
			display.fillRect(47, 15, 16, 16, myBLACK);
		}

		switch (div_entiere)
		{
		case 0:
			drawImage(47, 15, 16, 16, wifiloop1);
			prec = div_entiere;
			break;
		case 1:
			drawImage(47, 15, 16, 16, wifiloop2);
			prec = div_entiere;
			break;
		case 2:
			drawImage(47, 15, 16, 16, wifiloop3);
			prec = div_entiere;
			break;
		case 3:
			drawImage(47, 15, 16, 16, wifiloop4);
			prec = div_entiere;
			break;
		case 4:
			drawImage(47, 15, 16, 16, wifiloop5);
			prec = div_entiere;
			break;
		case 5:
			time_before_config = millis();
			prec = div_entiere;
			break;
		}
	}

	if (cfg::has_matrix)
	{
		display_update_enable(true);
		display.fillScreen(myBLACK);
		display.setTextColor(myWHITE);
		display.setFont(NULL); //&Font4x5Fixed
		display.setTextSize(1);
		display.setCursor(1, 0);
		display.print(INTL_CONFIGURE);
		display.setCursor(1, 11);
		display.print(INTL_THE_WIFI);
		display.setCursor(1, 22);
		display.print("3 min.");
		for (int i = 0; i < 5; i++)
		{
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop1);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop2);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop3);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop4);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop5);
			delay(200);
		}
		display.fillScreen(myBLACK);
	}

	WiFi.softAPdisconnect(true);
	dnsServer.stop(); // A VOIR
	delay(100);
	// WiFi.disconnect(true, true);
	WiFi.mode(WIFI_OFF); //A tenter

	debug_outln_info(F("---- Result Webconfig ----"));
	debug_outln_info(F("WiFi: "), cfg::has_wifi);
	debug_outln_info(F("WLANSSID: "), cfg::wlanssid);
	debug_outln_info(FPSTR(DBG_TXT_SEP));
	debug_outln_info_bool(F("SDS: "), cfg::sds_read);
	debug_outln_info_bool(F("NPM: "), cfg::npm_read);
	debug_outln_info_bool(F("BMX: "), cfg::bmx280_read);
	debug_outln_info_bool(F("MHZ16: "), cfg::mhz16_read);
	debug_outln_info_bool(F("MHZ19: "), cfg::mhz19_read);
	debug_outln_info_bool(F("CCS811: "), cfg::ccs811_read);
	debug_outln_info(FPSTR(DBG_TXT_SEP));
	debug_outln_info_bool(F("SensorCommunity: "), cfg::send2dusti);
	debug_outln_info_bool(F("Madavi: "), cfg::send2madavi);
	debug_outln_info_bool(F("CSV: "), cfg::send2csv);
	debug_outln_info_bool(F("AirCarto: "), cfg::send2custom);
	debug_outln_info_bool(F("AtmoSud: "), cfg::send2custom2);
	debug_outln_info(FPSTR(DBG_TXT_SEP));
	debug_outln_info_bool(F("Display: "), cfg::has_ssd1306);
	debug_outln_info_bool(F("Matrix: "), cfg::has_matrix);
	debug_outln_info_bool(F("Display Measures: "), cfg::display_measure);
	debug_outln_info_bool(F("Display forecast: "), cfg::display_forecast);
	debug_outln_info_bool(F("Display logo: "), cfg::display_logo);
	debug_outln_info_bool(F("Display alert: "), cfg::display_alert);
	debug_outln_info(F("Debug: "), String(cfg::debug));
	wificonfig_loop = false; // VOIR ICI
}

static void waitForWifiToConnect(int maxRetries)
{
	int retryCount = 0;
	while ((WiFi.status() != WL_CONNECTED) && (retryCount < maxRetries))
	{
		delay(500);
		debug_out(".", DEBUG_MIN_INFO);
		++retryCount;
	}
}

/*****************************************************************
 * get screens                                         *
 *****************************************************************/

void getScreenAircarto(unsigned int screen, String type, String id)
{
	debug_outln_info(String(ESP.getFreeHeap()));
	String reponseAPI;
	DynamicJsonDocument screenarray(65536);
	debug_outln_info(String(screenarray.capacity()));
	HTTPClient http;
	http.setTimeout(20 * 1000);

	String urlAirCarto1 = "http://data.moduleair.fr/imageCreate/index?screen=";
	String urlAirCarto2 = "&type=";
	String urlAirCarto3 = "&device=";
	String serverPath = urlAirCarto1 + String(screen) + urlAirCarto2 + type + urlAirCarto3 + id;

	//String serverPath = "http://data.moduleair.fr/imageCreate/index?screen=0&type=logo&device=50349E9EF0C8";

	debug_outln_info(F("Call: "), serverPath);
	http.begin(serverPath.c_str());

	//bool HTTPClient::begin(String url, const char* CAcert)

	int httpResponseCode = http.GET();

	if (httpResponseCode > 0)
	{
		reponseAPI = http.getString();
		//debug_outln_info(F("Response: "), reponseAPI);
		if (reponseAPI == "null")
		{
			debug_outln_info(F("Response = null"));
			if (type == "mod")
			{
				switch (screen)
				{
				case 0:
					if (forecast_screen_0 != nullptr)
					{
						free(forecast_screen_0);
						forecast_screen_0 = nullptr;
					}
					break;
				case 1:
					if (forecast_screen_1 != nullptr)
					{
						free(forecast_screen_1);
						forecast_screen_1 = nullptr;
					}
					break;
				case 2:
					if (forecast_screen_2 != nullptr)
					{
						free(forecast_screen_2);
						forecast_screen_2 = nullptr;
					}
					break;
				case 3:
					if (forecast_screen_3 != nullptr)
					{
						free(forecast_screen_3);
						forecast_screen_3 = nullptr;
					}
					break;
				case 4:
					if (forecast_screen_4 != nullptr)
					{
						free(forecast_screen_4);
						forecast_screen_4 = nullptr;
					}
					break;
				case 5:
					if (forecast_screen_5 != nullptr)
					{
						free(forecast_screen_5);
						forecast_screen_5 = nullptr;
					}
					break;
				case 6:
					if (forecast_screen_6 != nullptr)
					{
						free(forecast_screen_6);
						forecast_screen_6 = nullptr;
					}
					break;
				case 7:
					if (forecast_screen_7 != nullptr)
					{
						free(forecast_screen_7);
						forecast_screen_7 = nullptr;
					}
					break;
				case 8:
					if (forecast_screen_8 != nullptr)
					{
						free(forecast_screen_6);
						forecast_screen_8 = nullptr;
					}
					break;
				case 9:
					if (forecast_screen_9 != nullptr)
					{
						free(forecast_screen_9);
						forecast_screen_9 = nullptr;
					}
					break;
				case 10:
					if (forecast_screen_10 != nullptr)
					{
						free(forecast_screen_10);
						forecast_screen_10 = nullptr;
					}
					break;
				}
			forecast_selector = 0;
			}
			if (type == "logo")
			{
				logos[2] = 0;
			}
			if (type == "alert")
			{
				if (alert_screen != nullptr)
				{
					free(alert_screen);
					alert_screen = nullptr;
				}
			}
		}
		else
		{
			debug_outln_info(String(ESP.getFreeHeap()));
			DeserializationError error = deserializeJson(screenarray, reponseAPI.c_str());

			if (strcmp(error.c_str(), "Ok") == 0)
			{
				JsonArray screenarraybyte = screenarray.as<JsonArray>();

				if (type == "mod")
				{
					switch (screen)
					{
					case 0:
						if (forecast_screen_0 != nullptr)
						{
							free(forecast_screen_0);
							forecast_screen_0 = nullptr;
						}
						forecast_screen_0 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_0[i] = screenarraybyte[i];
						}
						break;
					case 1:
						if (forecast_screen_1 != nullptr)
						{
							free(forecast_screen_1);
							forecast_screen_1 = nullptr;
						}
						forecast_screen_1 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_1[i] = screenarraybyte[i];
						}
						break;
					case 2:
						if (forecast_screen_2 != nullptr)
						{
							free(forecast_screen_2);
							forecast_screen_2 = nullptr;
						}
						forecast_screen_2 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_2[i] = screenarraybyte[i];
						}
						break;
					case 3:
						if (forecast_screen_3 != nullptr)
						{
							free(forecast_screen_3);
							forecast_screen_3 = nullptr;
						}
						forecast_screen_3 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_3[i] = screenarraybyte[i];
						}
						break;
					case 4:
						if (forecast_screen_4 != nullptr)
						{
							free(forecast_screen_4);
							forecast_screen_4 = nullptr;
						}
						forecast_screen_4 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_4[i] = screenarraybyte[i];
						}
						break;
					case 5:
						if (forecast_screen_5 != nullptr)
						{
							free(forecast_screen_5);
							forecast_screen_5 = nullptr;
						}
						forecast_screen_5 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_5[i] = screenarraybyte[i];
						}
						break;
					case 6:
						if (forecast_screen_6 != nullptr)
						{
							free(forecast_screen_6);
							forecast_screen_6 = nullptr;
						}
						forecast_screen_6 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_6[i] = screenarraybyte[i];
						}
						break;
					case 7:
						if (forecast_screen_7 != nullptr)
						{
							free(forecast_screen_7);
							forecast_screen_7 = nullptr;
						}
						forecast_screen_7 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_7[i] = screenarraybyte[i];
						}
						break;
					case 8:
						if (forecast_screen_8 != nullptr)
						{
							free(forecast_screen_8);
							forecast_screen_8 = nullptr;
						}
						forecast_screen_8 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_8[i] = screenarraybyte[i];
						}
						break;
					case 9:
						if (forecast_screen_9 != nullptr)
						{
							free(forecast_screen_9);
							forecast_screen_9 = nullptr;
						}
						forecast_screen_9 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_9[i] = screenarraybyte[i];
						}
						break;
					case 10:
						if (forecast_screen_10 != nullptr)
						{
							free(forecast_screen_10);
							forecast_screen_10 = nullptr;
						}
						forecast_screen_10 = (uint8_t *)malloc(4096 * sizeof(uint8_t));
						for (int i = 0; i < 4096; i++)
						{
							forecast_screen_10[i] = screenarraybyte[i];
						}
						break;
					}
					forecast_selector++;
				}
				if (type == "logo")
				{
					logo_screen = (uint8_t *)malloc(4096 * sizeof(uint8_t));
					for (int i = 0; i < 4096; i++)
					{
						logo_screen[i] = screenarraybyte[i];
					}
					logos[2] = cfg_logo_custom;
					if (has_logo == false)
					{
						has_logo = true;
					}
				}
				if (type == "alert")
				{
					if (alert_screen != nullptr)
					{
						free(alert_screen);
						alert_screen = nullptr;
					}
					alert_screen = (uint8_t *)malloc(4096 * sizeof(uint8_t));
					for (int i = 0; i < 4096; i++)
					{
						alert_screen[i] = screenarraybyte[i];
					}
				}
			}
			else
			{
				// Debug.print(F("deserializeJson() failed: "));
				// Debug.println(error.c_str());
				debug_outln_info(F("deserializeJson() failed: "), error.c_str());
				if (type == "mod")
				{
					switch (screen)
					{
					case 0:
						if (forecast_screen_0 != nullptr)
						{
							free(forecast_screen_0);
							forecast_screen_0 = nullptr;
						}
						break;
					case 1:
						if (forecast_screen_1 != nullptr)
						{
							free(forecast_screen_1);
							forecast_screen_1 = nullptr;
						}
						break;
					case 2:
						if (forecast_screen_2 != nullptr)
						{
							free(forecast_screen_2);
							forecast_screen_2 = nullptr;
						}
						break;
					case 3:
						if (forecast_screen_3 != nullptr)
						{
							free(forecast_screen_3);
							forecast_screen_3 = nullptr;
						}
						break;
					case 4:
						if (forecast_screen_4 != nullptr)
						{
							free(forecast_screen_4);
							forecast_screen_4 = nullptr;
						}
						break;
					case 5:
						if (forecast_screen_5 != nullptr)
						{
							free(forecast_screen_5);
							forecast_screen_5 = nullptr;
						}
						break;
					case 6:
						if (forecast_screen_6 != nullptr)
						{
							free(forecast_screen_6);
							forecast_screen_6 = nullptr;
						}
						break;
					case 7:
						if (forecast_screen_7 != nullptr)
						{
							free(forecast_screen_7);
							forecast_screen_7 = nullptr;
						}
						break;
					case 8:
						if (forecast_screen_8 != nullptr)
						{
							free(forecast_screen_6);
							forecast_screen_8 = nullptr;
						}
						break;
					case 9:
						if (forecast_screen_9 != nullptr)
						{
							free(forecast_screen_9);
							forecast_screen_9 = nullptr;
						}
						break;
					case 10:
						if (forecast_screen_10 != nullptr)
						{
							free(forecast_screen_10);
							forecast_screen_10 = nullptr;
						}
						break;
					}
				}
				if (type == "logo")
				{
					logos[2] = 0;
				}
				if (type == "alert")
				{
					// alert_screen = nullptr;
					// ON NE FAIT RIEN ON GARDE l'ECRAN
					if (alert_screen != nullptr)
					{
						free(alert_screen);
						alert_screen = nullptr;
					}
				}
			}
		}
	}
	else
	{
		debug_outln_info(F("Failed connecting to AirCarto with error code:"), String(httpResponseCode));
		if (type == "mod")
		{
			switch (screen)
			{
			case 0:
				if (forecast_screen_0 != nullptr)
				{
					free(forecast_screen_0);
					forecast_screen_0 = nullptr;
				}
				break;
			case 1:
				if (forecast_screen_1 != nullptr)
				{
					free(forecast_screen_1);
					forecast_screen_1 = nullptr;
				}
				break;
			case 2:
				if (forecast_screen_2 != nullptr)
				{
					free(forecast_screen_2);
					forecast_screen_2 = nullptr;
				}
				break;
			case 3:
				if (forecast_screen_3 != nullptr)
				{
					free(forecast_screen_3);
					forecast_screen_3 = nullptr;
				}
				break;
			case 4:
				if (forecast_screen_4 != nullptr)
				{
					free(forecast_screen_4);
					forecast_screen_4 = nullptr;
				}
				break;
			case 5:
				if (forecast_screen_5 != nullptr)
				{
					free(forecast_screen_5);
					forecast_screen_5 = nullptr;
				}
				break;
			case 6:
				if (forecast_screen_6 != nullptr)
				{
					free(forecast_screen_6);
					forecast_screen_6 = nullptr;
				}
				break;
			case 7:
				if (forecast_screen_7 != nullptr)
				{
					free(forecast_screen_7);
					forecast_screen_7 = nullptr;
				}
				break;
			case 8:
				if (forecast_screen_8 != nullptr)
				{
					free(forecast_screen_6);
					forecast_screen_8 = nullptr;
				}
				break;
			case 9:
				if (forecast_screen_9 != nullptr)
				{
					free(forecast_screen_9);
					forecast_screen_9 = nullptr;
				}
				break;
			case 10:
				if (forecast_screen_10 != nullptr)
				{
					free(forecast_screen_10);
					forecast_screen_10 = nullptr;
				}
				break;
			}
		}
		if (type == "logo")
		{
			logos[2] = 0;
		}
		if (type == "alert")
		{
			//alert_screen = nullptr;
			// ON NE FAIT RIEN ON GARDE l'ECRAN
			if (alert_screen != nullptr)
			{
				free(alert_screen);
				alert_screen = nullptr;
			}
		}
	}
	//screenarray.~BasicJsonDocument();
	http.end();
	return;
}

/*****************************************************************
 * WiFi auto connecting script                                   *
 *****************************************************************/

static WiFiEventId_t disconnectEventHandler;
static WiFiEventId_t connectEventHandler;
static WiFiEventId_t STAstartEventHandler;
static WiFiEventId_t STAstopEventHandler;

static void connectWifi()
{
	if (cfg::has_matrix)
	{
		display.fillScreen(myBLACK);
		display.setTextColor(myWHITE);
		display.setFont(NULL);
		display.setTextSize(1);
		display.setCursor(1, 0);
		display.print(INTL_CONNECTION);
		display.setCursor(1, 11);
		display.print(INTL_TRY);
		for (int i = 0; i < 5; i++)
		{
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop1);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop2);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop3);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop4);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop5);
			delay(200);
		}
		display.fillScreen(myBLACK);
		display_update_enable(false);
	}

	display_debug(F("Connecting to"), String(cfg::wlanssid));

	disconnectEventHandler = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
										  {
											  if (!wifi_connection_lost)
											  {
												  debug_outln_info(F("Event disconnect"));
												  wifi_connection_lost = true;
											  }
										  },
										  WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

	connectEventHandler = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
									   {
										   if (wifi_connection_lost)
										   {
											   debug_outln_info(F("Event connect"));
											   wifi_connection_lost = false;
										   }
									   },
									   WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);

	STAstartEventHandler = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
										{ debug_outln_info(F("STA start")); },
										WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_START);

	STAstopEventHandler = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
									   { debug_outln_info(F("STA stop")); },
									   WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_STOP);

	if (WiFi.getAutoConnect())
	{
		WiFi.setAutoConnect(false);
	}

	WiFi.setAutoReconnect(false);

	// Use 13 channels for connect to known AP
	wifi_country_t wifi;
	wifi.policy = WIFI_COUNTRY_POLICY_MANUAL;
	strcpy(wifi.cc, INTL_LANG);
	wifi.nchan = 13;
	wifi.schan = 1;

	WiFi.mode(WIFI_STA);
	WiFi.setHostname(cfg::fs_ssid);
	WiFi.begin(cfg::wlanssid, cfg::wlanpwd); // Start WiFI

	debug_outln_info(FPSTR(DBG_TXT_CONNECTING_TO), cfg::wlanssid);

	waitForWifiToConnect(20); //diminur ici ???  //40
	debug_outln_info(emptyString);

	//if (WiFi.status() != WL_CONNECTED) //Waitforwifistatus ?
	if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) //Waitforwifistatus ?
	{
		debug_outln_info(F("Force change WiFi config"));
		wifi_connection_lost = true;
		cfg::has_wifi = false;

		if (cfg::has_matrix)
		{
			display_update_enable(true);
		}
		wifiConfig();
	}
	else
	{
		wifi_connection_lost = false;
		if (cfg::display_logo)
		{
			debug_outln_info(F("Get custom logo...")); //only once!
			getScreenAircarto(0, "logo", esp_chipid);
		}
	}

	debug_outln_info(F("WiFi connected, IP is: "), WiFi.localIP().toString());
	last_signal_strength = WiFi.RSSI(); //RSSI ICI!!!!

	if (MDNS.begin(cfg::fs_ssid))
	{
		MDNS.addService("http", "tcp", 80);
		MDNS.addServiceTxt("http", "tcp", "PATH", "/config");
	}

	if (cfg::has_matrix)
	{
		display_update_enable(true); //reactivate matrix

		if (calcWiFiSignalQuality(last_signal_strength) == 0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myWHITE);
			display.setFont(NULL);
			display.setTextSize(1);
			display.setCursor(1, 0);
			display.print(INTL_CONFIGURE);
			display.setCursor(1, 11);
			display.print(INTL_THE_WIFI);
			display.setCursor(1, 22);
			display.print("3 min.");
			for (int i = 0; i < 5; i++)
			{
				display.fillRect(47, 15, 16, 16, myBLACK);
				drawImage(47, 15, 16, 16, wifiloop1);
				delay(200);
				display.fillRect(47, 15, 16, 16, myBLACK);
				drawImage(47, 15, 16, 16, wifiloop2);
				delay(200);
				display.fillRect(47, 15, 16, 16, myBLACK);
				drawImage(47, 15, 16, 16, wifiloop3);
				delay(200);
				display.fillRect(47, 15, 16, 16, myBLACK);
				drawImage(47, 15, 16, 16, wifiloop4);
				delay(200);
				display.fillRect(47, 15, 16, 16, myBLACK);
				drawImage(47, 15, 16, 16, wifiloop5);
				delay(200);
			}
			display.fillScreen(myBLACK);
		}
		else
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myWHITE);
			display.setFont(NULL);
			display.setTextSize(1);
			display.setCursor(1, 0);
			display.print(INTL_CONNECTION);
			display.setCursor(1, 11);
#if defined(INTL_EN)
			display.print(INTL_DONE);
#endif
#if defined(INTL_FR)
			display.write(130);
			display.print("tablie");
#endif
			display.setCursor(1, 22);
			display.print(String(calcWiFiSignalQuality(WiFi.RSSI())));
			display.print("%");

			for (int i = 0; i < 5; i++)
			{
				display.fillRect(47, 15, 16, 16, myBLACK);
				drawImage(47, 15, 16, 16, wifiloop1);
				delay(200);
				display.fillRect(47, 15, 16, 16, myBLACK);
				drawImage(47, 15, 16, 16, wifiloop2);
				delay(200);
				display.fillRect(47, 15, 16, 16, myBLACK);
				drawImage(47, 15, 16, 16, wifiloop3);
				delay(200);
				display.fillRect(47, 15, 16, 16, myBLACK);
				drawImage(47, 15, 16, 16, wifiloop4);
				delay(200);
				display.fillRect(47, 15, 16, 16, myBLACK);
				drawImage(47, 15, 16, 16, wifiloop5);
				delay(200);
			}
			display.fillScreen(myBLACK);
		}
	}
}

// static void reConnectWifi()
// {
// 	if (cfg::has_matrix)
// 	{
// 		display_update_enable(false);
// 	}

// 	display_debug(F("Connecting to"), String(cfg::wlanssid));

// 	disconnectEventHandler = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
// 										  {
// 											  if (!wifi_connection_lost){
// 												Debug.println(F("Event disconnect"));
// 											  wifi_connection_lost = true;
// 											  }
// 										  },
// 										  WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

// 	connectEventHandler = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
// 										  {
// 											if (wifi_connection_lost){
// 												Debug.println(F("Event connect"));
// 											 wifi_connection_lost = false;
// 											}
// 										  },
// 										  WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);

// 	STAstartEventHandler = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
// 										  {
// 											Debug.println(F("STA start"));
// 										  },
// 										  WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_START);

// 	STAstopEventHandler = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
// 										  {
// 											Debug.println(F("STA stop"));
// 										  },
// 										  WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_STOP);

// 	if (WiFi.getAutoConnect())
// 	{
// 		WiFi.setAutoConnect(false);
// 	}

// 	WiFi.setAutoReconnect(false);

// 	// Use 13 channels for connect to known AP
// 	wifi_country_t wifi;
// 	wifi.policy = WIFI_COUNTRY_POLICY_MANUAL;
// 	strcpy(wifi.cc, INTL_LANG);
// 	wifi.nchan = 13;
// 	wifi.schan = 1;

// 	WiFi.mode(WIFI_STA);
// 	WiFi.setHostname(cfg::fs_ssid);
// 	WiFi.begin(cfg::wlanssid, cfg::wlanpwd); // Start WiFI

// 	debug_outln_info(FPSTR(DBG_TXT_CONNECTING_TO), cfg::wlanssid);

// 	waitForWifiToConnect(40);
// 	debug_outln_info(emptyString);

// 	if (WiFi.waitForConnectResult(10000) != WL_CONNECTED)
// 	{
// 		Debug.println(F("Can't restart!"));
// 		// sensor_restart();
// 		wifi_connection_lost = true;
// 	}
// 	else
// 	{
// 		wifi_connection_lost = false;
// 	}

// 	debug_outln_info(F("WiFi connected, IP is: "), WiFi.localIP().toString());
// 	last_signal_strength = WiFi.RSSI();

// 	if (MDNS.begin(cfg::fs_ssid))
// 	{
// 		MDNS.addService("http", "tcp", 80);
// 		MDNS.addServiceTxt("http", "tcp", "PATH", "/config");
// 	}

// 	if (cfg::has_matrix)
// 	{
// 		display_update_enable(true); //reactivate matrix
// 	}
// }

static WiFiClient *getNewLoggerWiFiClient(const LoggerEntry logger)
{
	WiFiClient *_client;
	_client = new WiFiClient;
	return _client;
}

static WiFiClientSecure *getNewLoggerWiFiClientSecure(const LoggerEntry logger)
{
	WiFiClientSecure *_client;
	_client = new WiFiClientSecure;
	return _client;
}

/*****************************************************************
 * send data to rest api                                         *
 *****************************************************************/
static unsigned long sendData(const LoggerEntry logger, const String &data, const int pin, const char *host, const char *url, bool ssl)
{
	unsigned long start_send = millis();
	const __FlashStringHelper *contentType;
	int result = 0;

	String s_Host(FPSTR(host));
	String s_url(FPSTR(url));

	switch (logger)
	{
	case LoggerSensorCommunity:
		debug_outln_info(F("LoggerSensorCommunity https: "), String(ssl));
		contentType = FPSTR(TXT_CONTENT_TYPE_JSON);
		break;
	case LoggerMadavi:
		debug_outln_info(F("LoggerMadavi https: "), String(ssl));
		contentType = FPSTR(TXT_CONTENT_TYPE_JSON);
		break;
	case LoggerCustom:
		debug_outln_info(F("LoggerAirCarto https: "), String(ssl));
		contentType = FPSTR(TXT_CONTENT_TYPE_JSON);
		break;
	case LoggerCustom2:
		debug_outln_info(F("LoggerAtmoSud https: "), String(ssl));
		contentType = FPSTR(TXT_CONTENT_TYPE_JSON);
		break;
	default:
		contentType = FPSTR(TXT_CONTENT_TYPE_JSON);
		break;
	}

	if (!ssl)
	{
		std::unique_ptr<WiFiClient> client(getNewLoggerWiFiClient(logger));

		HTTPClient http;
		http.setTimeout(20 * 1000);
		http.setUserAgent(SOFTWARE_VERSION + '/' + esp_chipid);
		http.setReuse(false);
		bool send_success = false;

		if (logger == LoggerCustom && (*cfg::user_custom || *cfg::pwd_custom))
		{
			http.setAuthorization(cfg::user_custom, cfg::pwd_custom);
		}

		if (logger == LoggerCustom2 && (*cfg::user_custom2 || *cfg::pwd_custom2))
		{
			http.setAuthorization(cfg::user_custom2, cfg::pwd_custom2);
		}

		if (http.begin(*client, s_Host, loggerConfigs[logger].destport, s_url, !!loggerConfigs[logger].session))
		{
			http.addHeader(F("Content-Type"), contentType);
			http.addHeader(F("X-Sensor"), String(F(SENSOR_BASENAME)) + esp_chipid);
			// http.addHeader(F("X-MAC-ID"), String(F(SENSOR_BASENAME)) + esp_mac_id);
			if (pin)
			{
				http.addHeader(F("X-PIN"), String(pin));
			}

			result = http.POST(data);

			if (result >= HTTP_CODE_OK && result <= HTTP_CODE_ALREADY_REPORTED)
			{
				debug_outln_info(F("Succeeded http - "), s_Host);
				send_success = true;
			}
			else if (result >= HTTP_CODE_BAD_REQUEST)
			{
				debug_outln_info(F("Request http failed with error: "), String(result));
				debug_outln_info(F("Details:"), http.getString());
			}
			http.end();
		}
		else
		{
			debug_outln_info(F("Failed connecting to "), s_Host);
		}
		if (!send_success && result != 0)
		{
			loggerConfigs[logger].errors++;
			last_sendData_returncode = result;
		}

		return millis() - start_send;
	}
	else
	{
		std::unique_ptr<WiFiClientSecure> clientSecure(getNewLoggerWiFiClientSecure(logger));

		switch (logger)
		{
		case LoggerSensorCommunity:
			clientSecure->setCACert(dst_root_ca_x3);
			break;
		case LoggerMadavi:
			clientSecure->setCACert(dst_root_ca_x3);
			break;
		case LoggerCustom:
			clientSecure->setCACert(ca_aircarto);
			break;
		case LoggerCustom2:
			clientSecure->setCACert(ca_atmo);
			break;
		}

		HTTPClient https;
		https.setTimeout(20 * 1000);
		https.setUserAgent(SOFTWARE_VERSION + '/' + esp_chipid);
		https.setReuse(false);
		bool send_success = false;
		if (logger == LoggerCustom && (*cfg::user_custom || *cfg::pwd_custom))
		{
			https.setAuthorization(cfg::user_custom, cfg::pwd_custom);
		}
		if (logger == LoggerCustom2 && (*cfg::user_custom2 || *cfg::pwd_custom2))
		{
			https.setAuthorization(cfg::user_custom2, cfg::pwd_custom2);
		}

		if (https.begin(*clientSecure, s_Host, loggerConfigs[logger].destport, s_url, !!loggerConfigs[logger].session))
		{
			https.addHeader(F("Content-Type"), contentType);
			https.addHeader(F("X-Sensor"), String(F(SENSOR_BASENAME)) + esp_chipid);
			if (pin)
			{
				https.addHeader(F("X-PIN"), String(pin));
			}

			result = https.POST(data);

			if (result >= HTTP_CODE_OK && result <= HTTP_CODE_ALREADY_REPORTED)
			{
				debug_outln_info(F("Succeeded https - "), s_Host);
				send_success = true;
			}
			else if (result >= HTTP_CODE_BAD_REQUEST)
			{
				debug_outln_info(F("Request https failed with error: "), String(result));
				debug_outln_info(F("Details:"), https.getString());
			}
			https.end();
		}
		else
		{
			debug_outln_info(F("Failed connecting to "), s_Host);
		}
		if (!send_success && result != 0)
		{
			loggerConfigs[logger].errors++;
			last_sendData_returncode = result;
		}

		return millis() - start_send;
	}
}

/*****************************************************************
 * send single sensor data to sensor.community api                *
 *****************************************************************/
static unsigned long sendSensorCommunity(const String &data, const int pin, const __FlashStringHelper *sensorname, const char *replace_str)
{
	unsigned long sum_send_time = 0;

	if (cfg::send2dusti && data.length())
	{
		RESERVE_STRING(data_sensorcommunity, LARGE_STR);
		data_sensorcommunity = FPSTR(data_first_part);

		debug_outln_info(F("## Sending to sensor.community - "), sensorname);
		data_sensorcommunity += data;
		data_sensorcommunity.remove(data_sensorcommunity.length() - 1);
		data_sensorcommunity.replace(replace_str, emptyString);
		data_sensorcommunity += "]}";
		debug_outln_info(data_sensorcommunity);
		sum_send_time = sendData(LoggerSensorCommunity, data_sensorcommunity, pin, HOST_SENSORCOMMUNITY, URL_SENSORCOMMUNITY, cfg::ssl_dusti);
	}

	return sum_send_time;
}

/*****************************************************************
 * send data as csv to serial out                                *
 *****************************************************************/
static void send_csv(const String &data)
{
	DynamicJsonDocument json2data(JSON_BUFFER_SIZE);
	DeserializationError err = deserializeJson(json2data, data);
	debug_outln_info(F("CSV Output: "), data);
	if (!err)
	{
		String headline = F("Timestamp_ms;");
		String valueline(act_milli);
		valueline += ';';
		for (JsonObject measurement : json2data[FPSTR(JSON_SENSOR_DATA_VALUES)].as<JsonArray>())
		{
			headline += measurement["value_type"].as<const char *>();
			headline += ';';
			valueline += measurement["value"].as<const char *>();
			valueline += ';';
		}
		static bool first_csv_line = true;
		if (first_csv_line)
		{
			if (headline.length() > 0)
			{
				headline.remove(headline.length() - 1);
			}
			debug_outln_info(headline);
			first_csv_line = false;
		}
		if (valueline.length() > 0)
		{
			valueline.remove(valueline.length() - 1);
		}
		debug_outln_info(valueline);
	}
	else
	{
		debug_outln_error(FPSTR(DBG_TXT_DATA_READ_FAILED));
	}
}

/*****************************************************************
 * read BMP280/BME280 sensor values                              *
 *****************************************************************/
static void fetchSensorBMX280(String &s)
{
	const char *const sensor_name = (bmx280.sensorID() == BME280_SENSOR_ID) ? SENSORS_BME280 : SENSORS_BMP280;
	debug_outln_verbose(FPSTR(DBG_TXT_START_READING), FPSTR(sensor_name));

	bmx280.takeForcedMeasurement();
	const auto t = bmx280.readTemperature();
	const auto p = bmx280.readPressure();
	const auto h = bmx280.readHumidity();
	if (isnan(t) || isnan(p))
	{
		last_value_BMX280_T = -128.0;
		last_value_BMX280_P = -1.0;
		last_value_BME280_H = -1.0;
		debug_outln_error(F("BMP/BME280 read failed"));
	}
	else
	{
		last_value_BMX280_T = t;
		last_value_BMX280_P = p;
		if (bmx280.sensorID() == BME280_SENSOR_ID)
		{
			add_Value2Json(s, F("BME280_temperature"), FPSTR(DBG_TXT_TEMPERATURE), last_value_BMX280_T);
			add_Value2Json(s, F("BME280_pressure"), FPSTR(DBG_TXT_PRESSURE), last_value_BMX280_P);
			last_value_BME280_H = h;
			add_Value2Json(s, F("BME280_humidity"), FPSTR(DBG_TXT_HUMIDITY), last_value_BME280_H);
		}
		else
		{
			add_Value2Json(s, F("BMP280_pressure"), FPSTR(DBG_TXT_PRESSURE), last_value_BMX280_P);
			add_Value2Json(s, F("BMP280_temperature"), FPSTR(DBG_TXT_TEMPERATURE), last_value_BMX280_T);
		}
	}
	debug_outln_info(FPSTR(DBG_TXT_SEP));
	debug_outln_verbose(FPSTR(DBG_TXT_END_READING), FPSTR(sensor_name));
}

/*****************************************************************
 * read MHZ16 sensor values                              *
 *****************************************************************/
static void fetchSensorMHZ16(String &s)
{
	const char *const sensor_name = SENSORS_MHZ16;
	debug_outln_verbose(FPSTR(DBG_TXT_START_READING), FPSTR(sensor_name));

	int value = mhz16.getPPM();

	if (isnan(value))
	{
		debug_outln_error(F("MHZ16 read failed"));
	}
	else
	{
		int value = mhz16.getPPM();
		mhz16_sum += value;
		mhz16_val_count++;
		debug_outln(String(mhz16_val_count), DEBUG_MAX_INFO);
	}

	if (send_now && cfg::sending_intervall_ms >= 120000)
	{
		last_value_MHZ16 = -1.0f;

		if (mhz16_val_count == 12)
		{
			last_value_MHZ16 = float(mhz16_sum / mhz16_val_count);
			add_Value2Json(s, F("MHZ16_CO2"), FPSTR(DBG_TXT_CO2PPM), last_value_MHZ16);
			debug_outln_info(FPSTR(DBG_TXT_SEP));
		}
		else
		{
			MHZ16_error_count++;
		}

		mhz16_sum = 0;
		mhz16_val_count = 0;
	}

	debug_outln_info(FPSTR(DBG_TXT_SEP));
	debug_outln_verbose(FPSTR(DBG_TXT_END_READING), FPSTR(sensor_name));
}

/*****************************************************************
 * read MHZ19 sensor values                              *
 *****************************************************************/
static void fetchSensorMHZ19(String &s)
{
	const char *const sensor_name = SENSORS_MHZ19;
	debug_outln_verbose(FPSTR(DBG_TXT_START_READING), FPSTR(sensor_name));

	int value;

	value = mhz19.getCO2();

	if (isnan(value))
	{
		debug_outln_error(F("MHZ19 read failed"));
	}
	else
	{
		mhz19_sum += value;
		mhz19_val_count++;
		debug_outln(String(mhz19_val_count), DEBUG_MAX_INFO);
	}

	if (send_now && cfg::sending_intervall_ms == 120000)
	{
		last_value_MHZ19 = -1.0f;

		if (mhz19_val_count >= 12)
		{
			last_value_MHZ19 = float(mhz19_sum / mhz19_val_count);
			add_Value2Json(s, F("MHZ19_CO2"), FPSTR(DBG_TXT_CO2PPM), last_value_MHZ19);
			debug_outln_info(FPSTR(DBG_TXT_SEP));
		}
		else
		{
			MHZ19_error_count++;
		}

		mhz19_sum = 0;
		mhz19_val_count = 0;
	}

	debug_outln_info(FPSTR(DBG_TXT_SEP));
	debug_outln_verbose(FPSTR(DBG_TXT_END_READING), FPSTR(sensor_name));
}

/*****************************************************************
 * read CCS811 sensor values                              *
 *****************************************************************/
static void fetchSensorCCS811(String &s)
{
	const char *const sensor_name = SENSORS_CCS811;
	debug_outln_verbose(FPSTR(DBG_TXT_START_READING), FPSTR(sensor_name));

	uint16_t etvoc, errstat;
	ccs811.read(NULL, &etvoc, &errstat, NULL);

	if (errstat == CCS811_ERRSTAT_OK)
	{

		ccs811_sum += etvoc;
		ccs811_val_count++;
		debug_outln(String(ccs811_val_count), DEBUG_MAX_INFO);
	}
	else if (errstat == CCS811_ERRSTAT_OK_NODATA)
	{
		debug_outln_info(F("CCS811: waiting for (new) data"));
	}
	else if (errstat & CCS811_ERRSTAT_I2CFAIL)
	{
		debug_outln_info(F("CCS811: I2C error"));
	}
	else
	{
		Debug.print(F("CCS811: errstat="));
		Debug.print(F("errstat,HEX"));
		Debug.print(F("="));
		debug_outln_info(ccs811.errstat_str(errstat));
	}

	if (send_now && cfg::sending_intervall_ms == 120000)
	{
		last_value_CCS811 = -1.0f;

		if (ccs811_val_count >= 12)
		{
			last_value_CCS811 = float(ccs811_sum / ccs811_val_count);
			add_Value2Json(s, F("CCS811_VOC"), FPSTR(DBG_TXT_VOCPPB), last_value_CCS811);
			debug_outln_info(FPSTR(DBG_TXT_SEP));
		}
		else
		{
			CCS811_error_count++;
		}

		ccs811_sum = 0;
		ccs811_val_count = 0;
	}

	debug_outln_info(FPSTR(DBG_TXT_SEP));
	debug_outln_verbose(FPSTR(DBG_TXT_END_READING), FPSTR(sensor_name));
}

/*****************************************************************
 * read SDS011 sensor values                                     *
 *****************************************************************/
static void fetchSensorSDS(String &s)
{
	if (cfg::sending_intervall_ms > (WARMUPTIME_SDS_MS + READINGTIME_SDS_MS) &&
		msSince(starttime) < (cfg::sending_intervall_ms - (WARMUPTIME_SDS_MS + READINGTIME_SDS_MS)))
	{
		if (is_SDS_running)
		{
			is_SDS_running = SDS_cmd(PmSensorCmd::Stop);
		}
	}
	else
	{
		if (!is_SDS_running)
		{
			is_SDS_running = SDS_cmd(PmSensorCmd::Start);
			SDS_waiting_for = SDS_REPLY_HDR;
		}

		while (serialSDS.available() >= SDS_waiting_for)
		{
			const uint8_t constexpr hdr_measurement[2] = {0xAA, 0xC0};
			uint8_t data[8];

			switch (SDS_waiting_for)
			{
			case SDS_REPLY_HDR:
				if (serialSDS.find(hdr_measurement, sizeof(hdr_measurement)))
					SDS_waiting_for = SDS_REPLY_BODY;
				break;
			case SDS_REPLY_BODY:
				debug_outln_verbose(FPSTR(DBG_TXT_START_READING), FPSTR(SENSORS_SDS011));
				if (serialSDS.readBytes(data, sizeof(data)) == sizeof(data) && SDS_checksum_valid(data))
				{
					uint32_t pm25_serial = data[0] | (data[1] << 8);
					uint32_t pm10_serial = data[2] | (data[3] << 8);

					if (msSince(starttime) > (cfg::sending_intervall_ms - READINGTIME_SDS_MS))
					{
						sds_pm10_sum += pm10_serial;
						sds_pm25_sum += pm25_serial;
						UPDATE_MIN_MAX(sds_pm10_min, sds_pm10_max, pm10_serial);
						UPDATE_MIN_MAX(sds_pm25_min, sds_pm25_max, pm25_serial);
						debug_outln_verbose(F("PM10 (sec.) : "), String(pm10_serial / 10.0f));
						debug_outln_verbose(F("PM2.5 (sec.): "), String(pm25_serial / 10.0f));
						sds_val_count++;
					}
				}
				debug_outln_verbose(FPSTR(DBG_TXT_END_READING), FPSTR(SENSORS_SDS011));
				SDS_waiting_for = SDS_REPLY_HDR;
				break;
			}
		}
	}
	if (send_now && cfg::sending_intervall_ms >= 120000)
	{
		last_value_SDS_P1 = -1;
		last_value_SDS_P2 = -1;
		if (sds_val_count > 2)
		{
			sds_pm10_sum = sds_pm10_sum - sds_pm10_min - sds_pm10_max;
			sds_pm25_sum = sds_pm25_sum - sds_pm25_min - sds_pm25_max;
			sds_val_count = sds_val_count - 2;
		}
		if (sds_val_count > 0)
		{
			last_value_SDS_P1 = float(sds_pm10_sum) / (sds_val_count * 10.0f);
			last_value_SDS_P2 = float(sds_pm25_sum) / (sds_val_count * 10.0f);
			add_Value2Json(s, F("SDS_P1"), F("PM10:  "), last_value_SDS_P1);
			add_Value2Json(s, F("SDS_P2"), F("PM2.5: "), last_value_SDS_P2);
			debug_outln_info(FPSTR(DBG_TXT_SEP));
			if (sds_val_count < 3)
			{
				SDS_error_count++;
			}
		}
		else
		{
			SDS_error_count++;
		}
		sds_pm10_sum = 0;
		sds_pm25_sum = 0;
		sds_val_count = 0;
		sds_pm10_max = 0;
		sds_pm10_min = 20000;
		sds_pm25_max = 0;
		sds_pm25_min = 20000;
		if ((cfg::sending_intervall_ms > (WARMUPTIME_SDS_MS + READINGTIME_SDS_MS)))
		{

			if (is_SDS_running)
			{
				is_SDS_running = SDS_cmd(PmSensorCmd::Stop);
			}
		}
	}
}

/*****************************************************************
 * read Tera Sensor Next PM sensor sensor values                 *
 *****************************************************************/
static void fetchSensorNPM(String &s)
{

	NPM_waiting_for_16 = NPM_REPLY_HEADER_16;

	debug_outln_info(F("Concentration NPM..."));
	NPM_cmd(PmSensorCmd2::Concentration);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_16)
	{
		const uint8_t constexpr header[2] = {0x81, 0x12};
		uint8_t state[1];
		uint8_t data[12];
		uint8_t checksum[1];
		uint8_t test[16];
		uint16_t N1_serial;
		uint16_t N25_serial;
		uint16_t N10_serial;
		uint16_t pm1_serial;
		uint16_t pm25_serial;
		uint16_t pm10_serial;

		switch (NPM_waiting_for_16)
		{
		case NPM_REPLY_HEADER_16:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_16 = NPM_REPLY_STATE_16;
			break;
		case NPM_REPLY_STATE_16:
			serialNPM.readBytes(state, sizeof(state));
			current_state_npm = NPM_state(state[0]);
			NPM_waiting_for_16 = NPM_REPLY_BODY_16;
			break;
		case NPM_REPLY_BODY_16:
			if (serialNPM.readBytes(data, sizeof(data)) == sizeof(data))
			{
				NPM_data_reader(data, 12);
				N1_serial = word(data[0], data[1]);
				N25_serial = word(data[2], data[3]);
				N10_serial = word(data[4], data[5]);

				pm1_serial = word(data[6], data[7]);
				pm25_serial = word(data[8], data[9]);
				pm10_serial = word(data[10], data[11]);

				debug_outln_info(F("Next PM Measure..."));

				debug_outln_verbose(F("PM1 (μg/m3) : "), String(pm1_serial / 10.0f));
				debug_outln_verbose(F("PM2.5 (μg/m3): "), String(pm25_serial / 10.0f));
				debug_outln_verbose(F("PM10 (μg/m3) : "), String(pm10_serial / 10.0f));

				debug_outln_verbose(F("PM1 (pcs/L) : "), String(N1_serial));
				debug_outln_verbose(F("PM2.5 (pcs/L): "), String(N25_serial));
				debug_outln_verbose(F("PM10 (pcs/L) : "), String(N10_serial));
			}
			NPM_waiting_for_16 = NPM_REPLY_CHECKSUM_16;
			break;
		case NPM_REPLY_CHECKSUM_16:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			NPM_data_reader(test, 16);
			if (NPM_checksum_valid_16(test))
			{
				debug_outln_info(F("Checksum OK..."));

				npm_pm1_sum += pm1_serial;
				npm_pm25_sum += pm25_serial;
				npm_pm10_sum += pm10_serial;

				npm_pm1_sum_pcs += N1_serial;
				npm_pm25_sum_pcs += N25_serial;
				npm_pm10_sum_pcs += N10_serial;
				npm_val_count++;
				debug_outln(String(npm_val_count), DEBUG_MAX_INFO);
			}
			NPM_waiting_for_16 = NPM_REPLY_HEADER_16;
			break;
		}
	}

	if (send_now && cfg::sending_intervall_ms >= 120000)
	{
		last_value_NPM_P0 = -1.0f;
		last_value_NPM_P1 = -1.0f;
		last_value_NPM_P2 = -1.0f;
		last_value_NPM_N1 = -1.0f;
		last_value_NPM_N10 = -1.0f;
		last_value_NPM_N25 = -1.0f;

		if (npm_val_count == 2)
		{
			last_value_NPM_P0 = float(npm_pm1_sum) / (npm_val_count * 10.0f);
			last_value_NPM_P1 = float(npm_pm10_sum) / (npm_val_count * 10.0f);
			last_value_NPM_P2 = float(npm_pm25_sum) / (npm_val_count * 10.0f);

			last_value_NPM_N1 = float(npm_pm1_sum_pcs) / (npm_val_count * 1000.0f);
			last_value_NPM_N10 = float(npm_pm10_sum_pcs) / (npm_val_count * 1000.0f);
			last_value_NPM_N25 = float(npm_pm25_sum_pcs) / (npm_val_count * 1000.0f);

			add_Value2Json(s, F("NPM_P0"), F("PM1: "), last_value_NPM_P0);
			add_Value2Json(s, F("NPM_P1"), F("PM10:  "), last_value_NPM_P1);
			add_Value2Json(s, F("NPM_P2"), F("PM2.5: "), last_value_NPM_P2);

			add_Value2Json(s, F("NPM_N1"), F("NC1.0: "), last_value_NPM_N1);
			add_Value2Json(s, F("NPM_N10"), F("NC10:  "), last_value_NPM_N10);
			add_Value2Json(s, F("NPM_N25"), F("NC2.5: "), last_value_NPM_N25);

			debug_outln_info(FPSTR(DBG_TXT_SEP));
		}
		else
		{
			NPM_error_count++;
		}

		npm_pm1_sum = 0;
		npm_pm10_sum = 0;
		npm_pm25_sum = 0;

		npm_val_count = 0;

		npm_pm1_sum_pcs = 0;
		npm_pm10_sum_pcs = 0;
		npm_pm25_sum_pcs = 0;

		debug_outln_info(F("Temperature and humidity in NPM after measure..."));
		current_th_npm = NPM_temp_humi();
	}
}

/*****************************************************************
 * display values                                                *
 *****************************************************************/
static void display_values_oled() //COMPLETER LES ECRANS
{
	float t_value = -128.0;
	float h_value = -1.0;
	float p_value = -1.0;
	String t_sensor, h_sensor, p_sensor;
	float pm01_value = -1.0;
	float pm25_value = -1.0;
	float pm10_value = -1.0;
	String pm01_sensor;
	String pm10_sensor;
	String pm25_sensor;
	float nc010_value = -1.0;
	float nc025_value = -1.0;
	float nc100_value = -1.0;

	String co2_sensor;
	String cov_sensor;

	float co2_value = -1.0;
	float cov_value = -1.0;

	double lat_value = -200.0;
	double lon_value = -200.0;
	double alt_value = -1000.0;
	String display_header;
	String display_lines[3] = {"", "", ""};
	uint8_t screen_count = 0;
	uint8_t screens[12];
	int line_count = 0;
	debug_outln_info(F("output values to display..."));

	if (cfg::npm_read)
	{
		pm01_value = last_value_NPM_P0;
		pm10_value = last_value_NPM_P1;
		pm25_value = last_value_NPM_P2;
		pm01_sensor = FPSTR(SENSORS_NPM);
		pm10_sensor = FPSTR(SENSORS_NPM);
		pm25_sensor = FPSTR(SENSORS_NPM);
		nc010_value = last_value_NPM_N1;
		nc100_value = last_value_NPM_N10;
		nc025_value = last_value_NPM_N25;
	}

	if (cfg::sds_read)
	{
		pm10_sensor = FPSTR(SENSORS_SDS011);
		pm25_sensor = FPSTR(SENSORS_SDS011);
		pm10_value = last_value_SDS_P1;
		pm25_value = last_value_SDS_P2;
	}

	if (cfg::bmx280_read)
	{
		t_sensor = p_sensor = FPSTR(SENSORS_BMP280);
		t_value = last_value_BMX280_T;
		p_value = last_value_BMX280_P;
		if (bmx280.sensorID() == BME280_SENSOR_ID)
		{
			h_sensor = t_sensor = FPSTR(SENSORS_BME280);
			h_value = last_value_BME280_H;
		}
	}

	if (cfg::mhz16_read)
	{
		co2_value = last_value_MHZ16;
		co2_sensor = FPSTR(SENSORS_MHZ16);
	}

	if (cfg::mhz19_read)
	{
		co2_value = last_value_MHZ19;
		co2_sensor = FPSTR(SENSORS_MHZ19);
	}

	if (cfg::ccs811_read)
	{
		cov_value = last_value_CCS811;
		cov_sensor = FPSTR(SENSORS_CCS811);
	}

	if (cfg::sds_read && cfg::display_measure)
	{
		screens[screen_count++] = 0;
	}
	if (cfg::npm_read && cfg::display_measure)
	{
		screens[screen_count++] = 1;
	}
	if (cfg::bmx280_read && cfg::display_measure)
	{
		screens[screen_count++] = 2;
	}

	if (cfg::mhz16_read && cfg::display_measure)
	{
		screens[screen_count++] = 3;
	}
	if (cfg::mhz19_read && cfg::display_measure)
	{
		screens[screen_count++] = 4;
	}
	if (cfg::ccs811_read && cfg::display_measure)
	{
		screens[screen_count++] = 5;
	}
	if (cfg::display_forecast)
	{
		screens[screen_count++] = 6; // Atmo Sud forecast
	}
	if (cfg::display_wifi_info && cfg::has_wifi)
	{
		screens[screen_count++] = 7; // Wifi info
	}
	if (cfg::display_device_info)
	{
		screens[screen_count++] = 8; // chipID, firmware and count of measurements
		screens[screen_count++] = 9; // Coordinates
		if (cfg::npm_read && cfg::display_measure)
		{
			screens[screen_count++] = 10; // info NPM
		}
	}

	switch (screens[next_display_count % screen_count])
	{
	case 0:
		display_header = FPSTR(SENSORS_SDS011);
		display_lines[0] = std::move(tmpl(F("PM2.5: {v} µg/m³"), check_display_value(pm25_value, -1, 1, 6)));
		display_lines[1] = std::move(tmpl(F("PM10: {v} µg/m³"), check_display_value(pm10_value, -1, 1, 6)));
		display_lines[2] = emptyString;
		break;
	case 1:
		display_header = FPSTR(SENSORS_NPM);
		display_lines[0] = std::move(tmpl(F("PM1: {v} µg/m³"), check_display_value(pm01_value, -1, 1, 6)));
		display_lines[1] = std::move(tmpl(F("PM2.5: {v} µg/m³"), check_display_value(pm25_value, -1, 1, 6)));
		display_lines[2] = std::move(tmpl(F("PM10: {v} µg/m³"), check_display_value(pm10_value, -1, 1, 6)));
		break;
	case 2:
		display_header = t_sensor;
		if (t_sensor != "")
		{
			display_lines[line_count] = "Temp.: ";
			display_lines[line_count] += check_display_value(t_value, -128, 1, 6);
			display_lines[line_count++] += " °C";
		}
		if (h_sensor != "")
		{
			display_lines[line_count] = "Hum.:  ";
			display_lines[line_count] += check_display_value(h_value, -1, 1, 6);
			display_lines[line_count++] += " %";
		}
		if (p_sensor != "")
		{
			display_lines[line_count] = "Pres.: ";
			display_lines[line_count] += check_display_value(p_value / 100, (-1 / 100.0), 1, 6);
			display_lines[line_count++] += " hPa";
		}
		while (line_count < 3)
		{
			display_lines[line_count++] = emptyString;
		}
		break;
	case 3:
		display_header = FPSTR(SENSORS_MHZ16);
		display_lines[0] = std::move(tmpl(F("CO2: {v} ppm"), check_display_value(co2_value, -1, 1, 6)));
		break;
	case 4:
		display_header = FPSTR(SENSORS_MHZ19);
		display_lines[0] = std::move(tmpl(F("CO2: {v} ppm"), check_display_value(co2_value, -1, 1, 6)));
		break;
	case 5:
		display_header = FPSTR(SENSORS_CCS811);
		display_lines[0] = std::move(tmpl(F("COV: {v} ppb"), check_display_value(cov_value, -1, 1, 6)));
		break;
	case 6:
		display_header = F("Forecast");
		break;
	case 7:
		display_header = F("Wifi info");
		display_lines[0] = "IP: ";
		display_lines[0] += WiFi.localIP().toString();
		display_lines[1] = "SSID: ";
		display_lines[1] += WiFi.SSID();
		display_lines[2] = std::move(tmpl(F("Signal: {v} %"), String(calcWiFiSignalQuality(last_signal_strength))));
		break;
	case 8:
		display_header = F("Device Info");
		display_lines[0] = "ID: ";
		display_lines[0] += esp_chipid;
		display_lines[1] = "FW: ";
		display_lines[1] += SOFTWARE_VERSION;
		display_lines[2] = F("Measurements: ");
		display_lines[2] += String(count_sends);
		break;
	case 9:
		display_header = F("Coordinates");
		display_lines[0] = "ID: ";
		display_lines[0] += esp_chipid;
		display_lines[1] = "FW: ";
		display_lines[1] += SOFTWARE_VERSION;
		display_lines[2] = F("Measurements: ");
		display_lines[2] += String(count_sends);
		break;
	case 10:
		display_header = FPSTR(SENSORS_NPM);
		display_lines[0] = current_state_npm;
		display_lines[1] = F("T_NPM / RH_NPM");
		display_lines[2] = current_th_npm;
		break;
	}

	oled_ssd1306->clear();
	oled_ssd1306->displayOn();
	oled_ssd1306->setTextAlignment(TEXT_ALIGN_CENTER);
	oled_ssd1306->drawString(64, 1, display_header);
	oled_ssd1306->setTextAlignment(TEXT_ALIGN_LEFT);
	oled_ssd1306->drawString(0, 16, display_lines[0]);
	oled_ssd1306->drawString(0, 28, display_lines[1]);
	oled_ssd1306->drawString(0, 40, display_lines[2]);
	oled_ssd1306->setTextAlignment(TEXT_ALIGN_CENTER);
	oled_ssd1306->drawString(64, 52, displayGenerateFooter(screen_count));
	oled_ssd1306->display();

	yield();
	next_display_count++;
}

static void display_values_matrix()
{
	float t_value = -128.0;
	float h_value = -1.0;
	float p_value = -1.0;
	String t_sensor, h_sensor, p_sensor;
	float pm01_value = -1.0;
	float pm25_value = -1.0;
	float pm10_value = -1.0;
	String pm01_sensor;
	String pm10_sensor;
	String pm25_sensor;
	float nc010_value = -1.0;
	float nc025_value = -1.0;
	float nc100_value = -1.0;

	String co2_sensor;
	String cov_sensor;

	float co2_value = -1.0;
	float cov_value = -1.0;

	double lat_value = -200.0;
	double lon_value = -200.0;
	double alt_value = -1000.0;
	uint8_t screen_count = 0;
	uint8_t screens[31];
	int line_count = 0;

	if (cfg::npm_read)
	{
		pm01_value = last_value_NPM_P0;
		pm10_value = last_value_NPM_P1;
		pm25_value = last_value_NPM_P2;
		pm01_sensor = FPSTR(SENSORS_NPM);
		pm10_sensor = FPSTR(SENSORS_NPM);
		pm25_sensor = FPSTR(SENSORS_NPM);
		nc010_value = last_value_NPM_N1;
		nc100_value = last_value_NPM_N10;
		nc025_value = last_value_NPM_N25;
	}

	if (cfg::sds_read)
	{
		pm10_sensor = FPSTR(SENSORS_SDS011);
		pm25_sensor = FPSTR(SENSORS_SDS011);
		pm10_value = last_value_SDS_P1;
		pm25_value = last_value_SDS_P2;
	}

	if (cfg::bmx280_read)
	{
		t_sensor = p_sensor = FPSTR(SENSORS_BMP280);
		t_value = last_value_BMX280_T;
		p_value = last_value_BMX280_P;
		if (bmx280.sensorID() == BME280_SENSOR_ID)
		{
			h_sensor = t_sensor = FPSTR(SENSORS_BME280);
			h_value = last_value_BME280_H;
		}
	}

	if (cfg::mhz16_read)
	{
		co2_value = last_value_MHZ16;
		co2_sensor = FPSTR(SENSORS_MHZ16);
	}

	if (cfg::mhz19_read)
	{
		co2_value = last_value_MHZ19;
		co2_sensor = FPSTR(SENSORS_MHZ19);
	}

	if (cfg::ccs811_read)
	{
		cov_value = last_value_CCS811;
		cov_sensor = FPSTR(SENSORS_CCS811);
	}

	if ((cfg::sds_read || cfg::npm_read || cfg::bmx280_read || cfg::mhz16_read || cfg::mhz19_read || cfg::ccs811_read) && cfg::display_measure)
	{
		screens[screen_count++] = 0; //Air intérieur
	}

	if (cfg::mhz16_read && cfg::display_measure)
	{
		if (cfg_screen_co2)
			screens[screen_count++] = 1;
	}
	if (cfg::mhz19_read && cfg::display_measure)
	{
		if (cfg_screen_co2)
			screens[screen_count++] = 2;
	}

	if (cfg::sds_read && cfg::display_measure)
	{

		if (cfg_screen_pm10)
			screens[screen_count++] = 3; //PM10
		if (cfg_screen_pm25)
			screens[screen_count++] = 4; //PM2.5
	}
	if (cfg::npm_read && cfg::display_measure)
	{
		if (cfg_screen_pm10)
			screens[screen_count++] = 5; //PM10
		if (cfg_screen_pm25)
			screens[screen_count++] = 6; //PM2.5
		if (cfg_screen_pm01)
			screens[screen_count++] = 7; //PM1
	}

	if (cfg::ccs811_read && cfg::display_measure)
	{
		if (cfg_screen_cov)
			screens[screen_count++] = 8;
	}

	if (cfg::bmx280_read && cfg::display_measure)
	{
		if (cfg_screen_temp)
			screens[screen_count++] = 9; //T
		if (cfg_screen_humi)
			screens[screen_count++] = 10; //H
		if (cfg_screen_press)
			screens[screen_count++] = 11; //P
	}

	if (cfg::display_forecast)
	{
		screens[screen_count++] = 12; // Air exterieur
		screens[screen_count++] = 13;
		screens[screen_count++] = 14;
		screens[screen_count++] = 15;
		screens[screen_count++] = 16;
		screens[screen_count++] = 17;
		screens[screen_count++] = 18;
		screens[screen_count++] = 19;
		screens[screen_count++] = 20;
		screens[screen_count++] = 21;
		screens[screen_count++] = 22;
		screens[screen_count++] = 23;
	}

	if (cfg::display_wifi_info && cfg::has_wifi)
	{
		screens[screen_count++] = 24; // Wifi info
	}
	if (cfg::display_device_info)
	{
		screens[screen_count++] = 25; // chipID, firmware and count of measurements
		screens[screen_count++] = 26; // Latitude, longitude, altitude
		if (cfg::npm_read && cfg::display_measure)
		{
			screens[screen_count++] = 27; // info NPM
		}
	}

	if (cfg::display_alert)
	{
		screens[screen_count++] = 28; // Alert
		screens[screen_count++] = 29; // Alert
	}

	screens[screen_count++] = 30; // Logos

	switch (screens[next_display_count % screen_count])
	{
	case 0:
		if (pm10_value != -1.0 || pm25_value != -1.0 || pm01_value != -1.0 || t_value != -128.0 || h_value != -1.0 || p_value != -1.0 || co2_value != -1.0 || cov_value != -1.0)
		{
			if (!cfg::has_wifi || (cfg::has_wifi && wifi_connection_lost))
			{
				drawImage(0, 0, 32, 64, interieur_no_connection);
			}
			if (cfg::has_wifi && !wifi_connection_lost)
			{
				drawImage(0, 0, 32, 64, interieur_wifi);
			}
		}
		else
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myWHITE);
			display.setFont(NULL);
			display.setTextSize(1);
			display.setCursor(1, 0);
#if defined(INTL_EN)
			display.print(INTL_FIRST);
#endif
#if defined(INTL_FR)
			display.print("Premi");
			display.write(138);
			display.print("res");
#endif
			display.setCursor(1, 11);
			display.print(INTL_MEASURES);
			display.setCursor(1, 22);
			display.print("2 min.");

			int div_entiere = (millis() - time_end_setup) / 17142;

			switch (div_entiere)
			{
			case 0:
				drawImage(47, 15, 16, 16, sablierloop1);
				break;
			case 1:
				drawImage(47, 15, 16, 16, sablierloop2);
				break;
			case 2:
				drawImage(47, 15, 16, 16, sablierloop3);
				break;
			case 3:
				drawImage(47, 15, 16, 16, sablierloop4);
				break;
			case 4:
				drawImage(47, 15, 16, 16, sablierloop5);
				break;
			case 5:
				drawImage(47, 15, 16, 16, sablierloop6);
				break;
			case 6:
				drawImage(47, 15, 16, 16, sablierloop7);
				break;
			case 7:
				drawImage(47, 15, 16, 16, sablierloop7);
				break;
			}
		}
		// else
		// {
		// 	act_milli += 5000;
		// }
		break;
	case 3: //SDS
		if (pm10_value != -1.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
			display.print("PM10");
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7); //Decaler vers le bas?
			display.write(181);
			display.print("g/m");
			display.write(179);
			drawImage(55, 0, 7, 9, maison);
			displayColor = interpolateint(pm10_value, 15, 30, 75, gamma_correction);
			myCUSTOM = display.color565(displayColor.R, displayColor.G, displayColor.B);
			display.fillRect(50, 9, 14, 14, myCUSTOM);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(pm10_value, 0), 0, 9, 14);
			display.setTextColor(myCUSTOM);
			messager1(pm10_value, 15, 30, 75);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 4:
		if (pm25_value != -1.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
			display.print("PM2.5");
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7);
			display.write(181);
			display.print("g/m");
			display.write(179);
			drawImage(55, 0, 7, 9, maison);
			displayColor = interpolateint(pm25_value, 10, 20, 50, gamma_correction);
			myCUSTOM = display.color565(displayColor.R, displayColor.G, displayColor.B);
			display.fillRect(50, 9, 14, 14, myCUSTOM);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(pm25_value, 0), 0, 9, 14);
			display.setTextColor(myCUSTOM);
			messager1(pm25_value, 10, 20, 50);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 5: //NPM
		if (pm10_value != -1.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
			display.print("PM10");
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7);
			display.write(181);
			display.print("g/m");
			display.write(179);
			drawImage(55, 0, 7, 9, maison);
			displayColor = interpolateint(pm10_value, 15, 30, 75, gamma_correction);
			myCUSTOM = display.color565(displayColor.R, displayColor.G, displayColor.B);
			display.fillRect(50, 9, 14, 14, myCUSTOM);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(pm10_value, 0), 0, 9, 14);
			display.setTextColor(myCUSTOM);
			messager1(pm10_value, 15, 30, 75);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 6:
		if (pm25_value != -1.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
			display.print("PM2.5");
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7);
			display.write(181);
			display.print("g/m");
			display.write(179);
			drawImage(55, 0, 7, 9, maison);
			displayColor = interpolateint(pm25_value, 10, 20, 50, gamma_correction);
			myCUSTOM = display.color565(displayColor.R, displayColor.G, displayColor.B);
			display.fillRect(50, 9, 14, 14, myCUSTOM);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(pm25_value, 0), 0, 9, 14);
			display.setTextColor(myCUSTOM);
			messager1(pm25_value, 10, 20, 50);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 7:
		if (pm01_value != -1.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
			display.print("PM1");
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7);
			display.write(181);
			display.print("g/m");
			display.write(179);
			drawImage(55, 0, 7, 9, maison);
			displayColor = interpolateint(pm01_value, 10, 20, 50, gamma_correction);
			myCUSTOM = display.color565(displayColor.R, displayColor.G, displayColor.B);
			display.fillRect(50, 9, 14, 14, myCUSTOM);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(pm01_value, 0), 0, 9, 14);
			display.setTextColor(myCUSTOM);
			messager1(pm01_value, 10, 20, 50);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 1:
		if (co2_value != -1.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
			display.print("CO");
			display.write(250);
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7);
			display.print("ppm");
			drawImage(55, 0, 7, 9, maison);
			displayColor = interpolateint2(co2_value, 800, 1500, gamma_correction);
			myCUSTOM = display.color565(displayColor.R, displayColor.G, displayColor.B);
			display.fillRect(50, 9, 14, 14, myCUSTOM);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(co2_value, 0), 0, 9, 14);
			display.setTextColor(myCUSTOM);
			messager2(co2_value, 800, 1500);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 2:
		if (co2_value != -1.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
			display.print("C0");
			display.write(250);
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7);
			display.print("ppm");
			drawImage(55, 0, 7, 9, maison);
			displayColor = interpolateint2(co2_value, 800, 1500, gamma_correction);
			myCUSTOM = display.color565(displayColor.R, displayColor.G, displayColor.B);
			display.fillRect(50, 9, 14, 14, myCUSTOM);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(co2_value, 0), 0, 9, 14);
			display.setTextColor(myCUSTOM);
			messager2(co2_value, 800, 1500);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 8:
		if (cov_value != -1.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
			display.print(INTL_COV);
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7);
			display.print("ppb");
			drawImage(55, 0, 7, 9, maison);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(cov_value, 0), 0, 9, 0);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 9:
		if (t_value != -128.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
			display.print(INTL_TEMP);
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7);
			display.write(176);
			display.print("C");
			drawImage(55, 0, 7, 9, maison);
			displayColor = interpolateint4(t_value, 19, 28, gamma_correction);
			myCUSTOM = display.color565(displayColor.R, displayColor.G, displayColor.B);
			display.fillRect(50, 9, 14, 14, myCUSTOM);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(t_value, 1), 0, 9, 14);
			display.setTextColor(myCUSTOM);
			messager4(t_value, 19, 28);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 10:
		if (h_value != -1.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
#if defined(INTL_EN)
			display.print(INTL_HUMI);
#endif
#if defined(INTL_FR)
			display.print("Humidit");
			display.write(130);
#endif
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7);
			display.write(37);
			drawImage(55, 0, 7, 9, maison);
			displayColor = interpolateint3(h_value, 40, 60, gamma_correction);
			myCUSTOM = display.color565(displayColor.R, displayColor.G, displayColor.B);
			display.fillRect(50, 9, 14, 14, myCUSTOM);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(h_value, 0), 0, 9, 14);
			display.setTextColor(myCUSTOM);
			messager3(h_value, 40, 60);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 11:
		if (p_value != -1.0)
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myCYAN);
			display.setFont(NULL);
			display.setCursor(1, 0);
			display.setTextSize(1);
			display.print(INTL_PRESS);
			display.setFont(&Font4x7Fixed);
			display.setCursor(display.getCursorX() + 2, 7);
			display.print("hPa");
			drawImage(55, 0, 7, 9, maison);
			display.setFont(NULL);
			display.setTextSize(2);
			display.setTextColor(myWHITE);
			drawCentreString(String(pressure_at_sealevel(t_value, p_value) / 100, 0), 0, 9, 0);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 12:
		if (forecast_screen_0 != nullptr || forecast_screen_1 != nullptr || forecast_screen_2 != nullptr || forecast_screen_3 != nullptr || forecast_screen_4 != nullptr || forecast_screen_5 != nullptr || forecast_screen_6 != nullptr || forecast_screen_7 != nullptr || forecast_screen_8 != nullptr || forecast_screen_9 != nullptr || forecast_screen_10 != nullptr)
		{
			drawImage(0, 0, 32, 64, exterieur);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 13:
		if (forecast_screen_0 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_0);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 14:
		if (forecast_screen_1 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_1);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 15:
		if (forecast_screen_2 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_2);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 16:
		if (forecast_screen_3 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_3);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 17:
		if (forecast_screen_4 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_4);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 18:
		if (forecast_screen_5 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_5);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 19:
		if (forecast_screen_6 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_6);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 20:
		if (forecast_screen_7 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_7);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 21:
		if (forecast_screen_8 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_8);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 22:
		if (forecast_screen_9 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_9);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 23:
		if (forecast_screen_10 != nullptr)
		{
			drawImage2(0, 0, 32, 64, forecast_screen_10);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 24:
		display.fillScreen(myBLACK);
		display.setTextColor(myWHITE);
		display.setFont(&Font4x5Fixed);
		display.setTextSize(1);
		display.setCursor(0, 4);
		display.print("Wifi Info");
		display.setCursor(0, 10);
		display.print("IP:");
		display.print(WiFi.localIP().toString());
		display.setCursor(0, 16);
		display.print("SSID:");
		display.print(WiFi.SSID());
		display.setCursor(0, 22);
		display.print("Signal:");
		display.print(String(calcWiFiSignalQuality(last_signal_strength)));
		break;
	case 25:
		display.fillScreen(myBLACK);
		display.setTextColor(myWHITE);
		display.setFont(&Font4x5Fixed);
		display.setCursor(0, 4);
		display.print("Device Info");
		display.setCursor(0, 10);
		display.print("ID:");
		display.print(esp_chipid);
		display.setCursor(0, 16);
		display.print("FW:");
		display.print(SOFTWARE_VERSION_SHORT);
		display.setCursor(0, 22);
		display.print("Meas.:");
		display.print(String(count_sends));
		break;
	case 26:
		display.fillScreen(myBLACK);
		display.setTextColor(myWHITE);
		display.setFont(&Font4x5Fixed);
		display.setCursor(0, 4);
		display.print("Position");
		display.setCursor(0, 10);
		display.print("Hauteur:");
		display.print(cfg::height_above_sealevel);
		break;
	case 27:
		if ((pm10_value != -1.0 || pm25_value != -1.0 || pm01_value != -1.0))
		{
			display.fillScreen(myBLACK);
			display.setTextColor(myWHITE);
			display.setFont(&Font4x7Fixed);
			display.setCursor(0, 6);
			display.print("Next PM permanent");
			display.setCursor(0, 14);
			display.print("T_NPM / RH_NPM");
			display.setCursor(0, 22);
			display.print(current_th_npm);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 28:
		if (cfg::display_alert && alert_screen != nullptr)
		{
			drawImage(0, 0, 32, 64, alert_ecran1);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 29:
		if (alert_screen != nullptr)
		{
			drawImage2(0, 0, 32, 64, alert_screen);
		}
		else
		{
			act_milli += 5000;
		}
		break;
	case 30:
		if (has_logo && logo_index < 3)
		{
			if (logos[logo_index] != 0)
			{
				switch (logos[logo_index])
				{
				case cfg_logo_moduleair:
					drawImage(0, 0, 32, 64, logo_moduleair);
					break;
				case cfg_logo_aircarto:
					drawImage(0, 0, 32, 64, logo_aircarto);
					break;
				case cfg_logo_custom:
					drawImage2(0, 0, 32, 64, logo_screen);
					break;
				}
			}
			else
			{
				act_milli += 5000;
			}
			logo_index++;
		}
		else
		{
			logo_index = 0;
			act_milli += 5000;
		}
		break;
	}

	yield();
	next_display_count++;
}

/*****************************************************************
 * Init matrix                                         *
 *****************************************************************/
static void init_matrix()
{
	timer = timerBegin(0, 80, true); //init timer once only
	display.begin(16);
	display.setDriverChip(SHIFT);  // SHIFT ou FM6124 ou FM6126A
	display.setColorOrder(RRGGBB); // ATTENTION à changer en fonction de l'écran !!!! Small Matrix (160x80mm) is RRBBGG and Big Matrix (192x96mm) is RRGGBB
	display_update_enable(true);
	display.setFont(NULL); //Default font

	if (cfg_logo_moduleair == 1)
	{
	}

	if (cfg_logo_moduleair == 2)
	{
		logos[0] = 0;
	}

	if (cfg_logo_moduleair == 0)
	{
		logos[0] = 0;
	}

	switch (cfg_logo_moduleair)
	{
	case 1:
		logos[0] = 1;
		display.fillScreen(myBLACK); //display.clearDisplay(); produces a flash
		drawImage(0, 0, 32, 64, logo_moduleair);
		delay(5000);
		break;
	case 2:
		logos[0] = 2;
		break;
	case 0:
		logos[0] = 0;
		break;
	}

	switch (cfg_logo_aircarto)
	{
	case 1:
		logos[1] = 1;
		display.fillScreen(myBLACK); //display.clearDisplay(); produces a flash
		drawImage(0, 0, 32, 64, logo_aircarto);
		delay(5000);
		break;
	case 2:
		logos[1] = 2;
		break;
	case 0:
		logos[1] = 0;
		break;
	}

	if (logos[0] == 1 || logos[1] == 1)
	{
		has_logo = true;
	}
	else
	{
		has_logo = false;
	}

	// for (int i = 1; i < 2; i++)
	// {

	// 	if (i == cfg_logo_moduleair)
	// 	{
	// 		display.fillScreen(myBLACK); //display.clearDisplay(); produces a flash
	// 		drawImage(0, 0, 32, 64, logo_moduleair);
	// 		logo_index++;
	// 		logos[logo_index] = i;
	// 		delay(5000);
	// 	}
	// 	if (i == cfg_logo_aircarto)
	// 	{
	// 		display.fillScreen(myBLACK); //display.clearDisplay(); produces a flash
	// 		drawImage(0, 0, 32, 64, logo_aircarto);
	// 		logo_index++;
	// 		logos[logo_index] = i;
	// 		delay(5000);
	// 	}
	// }

	// if (logo_index != -1)
	// {
	// 	has_logo = true;
	// 	logo_index = -1;
	// }
	// else
	// {
	// 	has_logo = false;
	// }
}

/*****************************************************************
 * Init LCD/OLED display                                         *
 *****************************************************************/
static void init_display()
{
	if (cfg::has_ssd1306)

	{

#if defined(ARDUINO_ESP32_DEV) and defined(KIT_V1)
		oled_ssd1306 = new SSD1306Wire(0x3c, I2C_PIN_SDA, I2C_PIN_SCL);
#endif

#if defined(ARDUINO_ESP32_DEV) and defined(KIT_C)
		oled_ssd1306 = new SSD1306Wire(0x3c, I2C_PIN_SDA, I2C_PIN_SCL);
#endif

		oled_ssd1306->init();
		oled_ssd1306->flipScreenVertically(); // ENLEVER ???
		oled_ssd1306->clear();
		oled_ssd1306->displayOn();
		oled_ssd1306->setTextAlignment(TEXT_ALIGN_CENTER);
		oled_ssd1306->drawString(64, 1, "START");
		oled_ssd1306->display();

		// reset back to 100k as the OLEDDisplay initialization is
		// modifying the I2C speed to 400k, which overwhelms some of the
		// sensors.
		Wire.setClock(100000);
		// Wire.setClockStretchLimit(150000);
	}
}

/*****************************************************************
 * Init BMP280/BME280                                            *
 *****************************************************************/
static bool initBMX280(char addr)
{
	debug_out(String(F("Trying BMx280 sensor on ")) + String(addr, HEX), DEBUG_MIN_INFO);

	if (bmx280.begin(addr))
	{
		debug_outln_info(FPSTR(DBG_TXT_FOUND));
		bmx280.setSampling(
			BMX280::MODE_FORCED,
			BMX280::SAMPLING_X1,
			BMX280::SAMPLING_X1,
			BMX280::SAMPLING_X1);
		return true;
	}
	else
	{
		debug_outln_info(FPSTR(DBG_TXT_NOT_FOUND));
		return false;
	}
}

/*****************************************************************
 * Init CCS811                                            *
 *****************************************************************/
static bool initCCS811()
{

	debug_out(String(F("Trying CCS811 sensor: ")), DEBUG_MIN_INFO);

	if (!ccs811.begin())
	{
		debug_out(String(F("CCS811 begin FAILED")), DEBUG_MIN_INFO);
		return false;
	}
	else
	{
		// Print CCS811 versions
		debug_outln_info(F("hardware version: "), ccs811.hardware_version());
		debug_outln_info(F("bootloader version: "), ccs811.bootloader_version());
		debug_outln_info(F("application version: "), ccs811.application_version());

		if (!ccs811.start(CCS811_MODE_1SEC))
		{
			debug_out(String(F("CCS811 start FAILED")), DEBUG_MIN_INFO);
			return false;
		}
		else
		{
			debug_out(String(F("CCS811 OK")), DEBUG_MIN_INFO);
			Debug.printf("\n");
			return true;
		}
	}
}

/*****************************************************************
   Functions
 *****************************************************************/
static void powerOnTestSensors()
{

	if (cfg::has_matrix)
	{
		display.fillScreen(myBLACK);
		// drawImage(31, 1, 30, 30, engrenage);
		display.setTextColor(myWHITE);
		display.setFont(NULL);
		display.setTextSize(1);
		display.setCursor(1, 0);
		display.print(INTL_ACTIVATION);
		display.setCursor(1, 11);
		display.print(INTL_PROBES);
	}

	if (cfg::sds_read)
	{
		debug_outln_info(F("Read SDS...: "), SDS_version_date());
		SDS_cmd(PmSensorCmd::ContinuousMode);
		delay(100);
		debug_outln_info(F("Stopping SDS011..."));
		is_SDS_running = SDS_cmd(PmSensorCmd::Stop);
	}

	if (cfg::npm_read)
	{
		int8_t test_state;
		//delay(15000); // wait a bit to be sure Next PM is ready to receive instructions.

		for (size_t i = 0; i < 30; ++i)
		{
			display.fillRect(i, 22, 1, 4, myWHITE);
			delay(500);
		}

		test_state = NPM_get_state();
		if (test_state == -1)
		{
			debug_outln_info(F("NPM not connected"));
			nextpmconnected = false;
		}
		else
		{
			nextpmconnected = true;
			if (test_state == 0x00)
			{
				debug_outln_info(F("NPM already started..."));
				nextpmconnected = true;
			}
			else if (test_state == 0x01)
			{
				debug_outln_info(F("Force start NPM...")); // to read the firmware version
				is_NPM_running = NPM_start_stop();
			}
			else
			{
				if (bitRead(test_state, 1) == 1)
				{
					debug_outln_info(F("Degraded state"));
				}
				else
				{
					debug_outln_info(F("Default state"));
				}
				if (bitRead(test_state, 2) == 1)
				{
					debug_outln_info(F("Not ready"));
				}
				if (bitRead(test_state, 3) == 1)
				{
					debug_outln_info(F("Heat error"));
				}
				if (bitRead(test_state, 4) == 1)
				{
					debug_outln_info(F("T/RH error"));
				}
				if (bitRead(test_state, 5) == 1)
				{
					debug_outln_info(F("Fan error"));

					// if (bitRead(test_state, 0) == 1){
					// 	debug_outln_info(F("Force start NPM..."));
					// 	is_NPM_running = NPM_start_stop();
					// 	delay(5000);
					// }
					// NPM_fan_speed();
					// delay(5000);
				}
				if (bitRead(test_state, 6) == 1)
				{
					debug_outln_info(F("Memory error"));
				}
				if (bitRead(test_state, 7) == 1)
				{
					debug_outln_info(F("Laser error"));
				}
				if (bitRead(test_state, 0) == 0)
				{
					debug_outln_info(F("NPM already started..."));
					is_NPM_running = true;
				}
				else
				{
					debug_outln_info(F("Force start NPM..."));
					is_NPM_running = NPM_start_stop();
				}
			}
		}

		if (nextpmconnected)
		{
			// delay(15000);

			for (size_t i = 30; i < 54; ++i)
			{
				display.fillRect(i, 22, 1, 4, myWHITE);
				delay(500);
			}
			NPM_version_date();

			for (size_t i = 54; i < 60; ++i)
			{
				display.fillRect(i, 22, 1, 4, myWHITE);
				delay(500);
			}
			NPM_temp_humi();
			for (size_t i = 60; i < 64; ++i)
			{
				display.fillRect(i, 22, 1, 4, myWHITE);
				delay(500);
			}
		}
	}

	if (cfg::bmx280_read)
	{
		debug_outln_info(F("Read BMxE280..."));
		if (!initBMX280(0x76) && !initBMX280(0x77))
		{
			debug_outln_error(F("Check BMx280 wiring"));
			bmx280_init_failed = true;
		}
	}

	if (cfg::ccs811_read)
	{
		debug_outln_info(F("Read CCS811..."));
		if (!initCCS811())
		{
			debug_outln_error(F("Check CCS811 wiring"));
			ccs811_init_failed = true;
		}
	}
}

static void logEnabledAPIs()
{
	debug_outln_info(F("Send to :"));
	if (cfg::send2dusti)
	{
		debug_outln_info(F("sensor.community"));
	}

	if (cfg::send2madavi)
	{
		debug_outln_info(F("Madavi.de"));
	}

	if (cfg::send2csv)
	{
		debug_outln_info(F("Serial as CSV"));
	}

	if (cfg::send2custom)
	{
		debug_outln_info(F("AirCarto API"));
	}
	if (cfg::send2custom2)
	{
		debug_outln_info(F("Atmosud API"));
	}
}

static void logEnabledDisplays()
{
	if (cfg::has_ssd1306)

	{
		debug_outln_info(F("Show on OLED..."));
	}
}

static void setupNetworkTime()
{
	// server name ptrs must be persisted after the call to configTime because internally
	// the pointers are stored see implementation of lwip sntp_setservername()
	static char ntpServer1[18], ntpServer2[18];
	strcpy_P(ntpServer1, NTP_SERVER_1);
	strcpy_P(ntpServer2, NTP_SERVER_2);
	configTime(0, 0, ntpServer1, ntpServer2);
}

static unsigned long sendDataToOptionalApis(const String &data)
{
	unsigned long sum_send_time = 0;

	debug_outln_info(data);

	if (cfg::send2madavi)
	{
		debug_outln_info(FPSTR(DBG_TXT_SENDING_TO), F("madavi.de: "));
		sum_send_time += sendData(LoggerMadavi, data, 0, HOST_MADAVI, URL_MADAVI, cfg::ssl_madavi);
	}

	if (cfg::send2custom)
	{
		String data_to_send = data;
		data_to_send.remove(0, 1);
		String data_4_custom(F("{\"moduleairid\": \""));
		data_4_custom += esp_chipid;
		data_4_custom += "\", ";
		data_4_custom += data_to_send;
		debug_outln_info(FPSTR(DBG_TXT_SENDING_TO), F("aircarto api: "));
		sum_send_time += sendData(LoggerCustom, data_4_custom, 0, cfg::host_custom, cfg::url_custom, cfg::ssl_custom);
	}

	if (cfg::send2custom2)
	{
		String data_to_send = data;
		data_to_send.remove(0, 1);
		String data_4_custom(F("{\"moduleairid\": \""));
		data_4_custom += esp_chipid;
		data_4_custom += "\", ";
		data_4_custom += data_to_send;
		debug_outln_info(FPSTR(DBG_TXT_SENDING_TO), F("atmosud api: "));
		sum_send_time += sendData(LoggerCustom2, data_4_custom, 0, cfg::host_custom2, cfg::url_custom2, cfg::ssl_custom2);
	}

	if (cfg::send2csv)
	{
		debug_outln_info(F("## Sending as csv: "));
		send_csv(data);
	}

	return sum_send_time;
}

/*****************************************************************
 * Check stack                                                    *
 *****************************************************************/
void *StackPtrAtStart;
void *StackPtrEnd;
UBaseType_t watermarkStart;

/*****************************************************************
 * The Setup                                                     *
 *****************************************************************/
void setup()
{
	void *SpStart = NULL;
	StackPtrAtStart = (void *)&SpStart;
	watermarkStart = uxTaskGetStackHighWaterMark(NULL);
	StackPtrEnd = StackPtrAtStart - watermarkStart;

	Debug.begin(115200); // Output to Serial at 115200 baud
	debug_outln_info(F("Starting"));

	Debug.printf("\r\n\r\nAddress of Stackpointer near start is:  %p \r\n", (void *)StackPtrAtStart);
	Debug.printf("End of Stack is near: %p \r\n", (void *)StackPtrEnd);
	Debug.printf("Free Stack at start of setup() is:  %d \r\n", (uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd);

	esp_chipid = String((uint16_t)(ESP.getEfuseMac() >> 32), HEX); // for esp32
	esp_chipid += String((uint32_t)ESP.getEfuseMac(), HEX);
	esp_chipid.toUpperCase();
	cfg::initNonTrivials(esp_chipid.c_str());
	WiFi.persistent(false);

	debug_outln_info(F("ModuleAirV2: " SOFTWARE_VERSION_STR "/"), String(CURRENT_LANG));

	init_config();

	spiffs_matrix = cfg::has_matrix; //save the spiffs state on start
	debug_outln_info(F("spiffs_matrix: "), String(spiffs_matrix));

	if (cfg::has_matrix)
	{
		init_matrix();
	}

	Wire.begin(I2C_PIN_SDA, I2C_PIN_SCL);

	if (cfg::npm_read)
	{
		serialNPM.begin(115200, SERIAL_8E1, PM_SERIAL_RX, PM_SERIAL_TX);
		debug_outln_info(F("Read Next PM... serialNPM 115200 8E1"));
		serialNPM.setTimeout(400);
	}

	if (cfg::sds_read)
	{
		serialSDS.begin(9600, SERIAL_8N1, PM_SERIAL_RX, PM_SERIAL_TX);
		debug_outln_info(F("No Next PM... serialSDS 9600 8N1"));
		serialSDS.setTimeout((4 * 12 * 1000) / 9600);
	}

	if (cfg::mhz16_read || cfg::mhz19_read)
	{
		//serialMHZ.begin(9600, SERIAL_8N1, CO2_SERIAL_RX, CO2_SERIAL_TX);
		debug_outln_info(F("serialMHZ 9600 8N1"));
		//serialMHZ.setTimeout((4 * 12 * 1000) / 9600); //VOIR ICI LE TIMEOUT

		if (cfg::mhz16_read)
		{
			mhz16.begin(CO2_SERIAL_RX, CO2_SERIAL_TX, 2);
			//mhz16.autoCalibration(false);
		}

		if (cfg::mhz19_read)
		{
			serialMHZ.begin(9600, SERIAL_8N1, CO2_SERIAL_RX, CO2_SERIAL_TX);
			mhz19.begin(serialMHZ);
			mhz19.autoCalibration(false);
		}
	}

	if (cfg::has_ssd1306)
	{
		init_display();
	}

	debug_outln_info(F("\nChipId: "), esp_chipid);

	if (cfg::has_matrix && cfg::has_wifi)
	{
		display.fillScreen(myBLACK);
		// drawImage(36, 6, 20, 27, wifiblue);
		display.setTextColor(myWHITE);
		display.setFont(NULL);
		display.setCursor(1, 0);
		display.setTextSize(1);
		display.print(INTL_ACTIVATION);
		display.setCursor(1, 11);
		display.print("WiFi");
		for (int i = 0; i < 5; i++)
		{
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop1);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop2);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop3);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop4);
			delay(200);
			display.fillRect(47, 15, 16, 16, myBLACK);
			drawImage(47, 15, 16, 16, wifiloop5);
			delay(200);
		}
		display.fillScreen(myBLACK);
	}

	if (cfg::has_wifi)
	{
		setupNetworkTime();
		connectWifi();
		setup_webserver();
	}
	else
	{
		wifiConfig();
	}

	createLoggerConfigs();
	logEnabledAPIs();
	powerOnTestSensors();
	logEnabledDisplays();

	delay(50);

	starttime = screentime = millis(); // store the start time
	time_point_device_start_ms = starttime;

	if (cfg::npm_read)
	{
		last_display_millis_oled = starttime_NPM = starttime;
		last_display_millis_matrix = starttime_NPM = starttime;
	}

	if (cfg::sds_read)
	{
		last_display_millis_oled = starttime_SDS = starttime;
		last_display_millis_matrix = starttime_SDS = starttime;
	}

	if (cfg::mhz16_read)
	{
		last_display_millis_oled = starttime_MHZ16 = starttime;
		last_display_millis_matrix = starttime_MHZ16 = starttime;
	}

	if (cfg::mhz19_read)
	{
		last_display_millis_oled = starttime_MHZ19 = starttime;
		last_display_millis_matrix = starttime_MHZ19 = starttime;
	}
	if (cfg::ccs811_read)
	{
		last_display_millis_oled = starttime_CCS811 = starttime;
		last_display_millis_matrix = starttime_CCS811 = starttime;
	}

	if (cfg::display_forecast)
	{
		forecast_selector = 0;
	}

	Debug.printf("End of void setup()\n");
	time_end_setup = millis();
}

/*****************************************************************
 * The Loop                                                     *
 *****************************************************************/
void loop()
{
	String result_SDS, result_NPM, result_MHZ16, result_MHZ19, result_CCS811;

	unsigned sum_send_time = 0;

	act_micro = micros();
	act_milli = millis();
	send_now = msSince(starttime) > cfg::sending_intervall_ms;
	screen_now = msSince(screentime) > cfg::screen_intervall_ms;

	// Wait at least 30s for each NTP server to sync

	if (cfg::has_wifi && !wifi_connection_lost)
	{
		if (!sntp_time_set && send_now && msSince(time_point_device_start_ms) < 1000 * 2 * 30 + 5000)
		{
			debug_outln_info(F("NTP sync not finished yet, skipping send"));
			send_now = false;
			starttime = screentime = act_milli;
		}
	}

	sample_count++;

	if (last_micro != 0)
	{
		unsigned long diff_micro = act_micro - last_micro;
		UPDATE_MIN_MAX(min_micro, max_micro, diff_micro);
	}
	last_micro = act_micro;

	if (cfg::npm_read)
	{
		if ((msSince(starttime_NPM) > SAMPLETIME_NPM_MS && npm_val_count == 0) || send_now)
		{
			starttime_NPM = act_milli;
			fetchSensorNPM(result_NPM);
		}
	}

	if (cfg::sds_read)
	{
		if ((msSince(starttime_SDS) > SAMPLETIME_SDS_MS) || send_now)
		{
			starttime_SDS = act_milli;
			fetchSensorSDS(result_SDS);
		}
	}

	if (cfg::mhz16_read)
	{
		if ((msSince(starttime_MHZ16) > SAMPLETIME_MHZ16_MS && mhz16_val_count < 11) || send_now)
		{
			starttime_MHZ16 = act_milli;
			fetchSensorMHZ16(result_MHZ16);
		}
	}

	if (cfg::mhz19_read)
	{
		if ((msSince(starttime_MHZ19) > SAMPLETIME_MHZ19_MS && mhz19_val_count < 11) || send_now)
		{
			starttime_MHZ19 = act_milli;
			fetchSensorMHZ19(result_MHZ19);
		}
	}

	if (cfg::ccs811_read && (!ccs811_init_failed))
	{
		if ((msSince(starttime_CCS811) > SAMPLETIME_CCS811_MS && ccs811_val_count < 11) || send_now)
		{
			starttime_CCS811 = act_milli;
			fetchSensorCCS811(result_CCS811);
		}
	}

	if ((msSince(last_display_millis_oled) > DISPLAY_UPDATE_INTERVAL_MS) && (cfg::has_ssd1306))
	{
		display_values_oled();
		last_display_millis_oled = act_milli;
	}

	if ((msSince(last_display_millis_matrix) > DISPLAY_UPDATE_INTERVAL_MS) && (cfg::has_matrix))
	{
		display_values_matrix();
		last_display_millis_matrix = act_milli;
	}

	//if (cfg::has_wifi && WiFi.waitForConnectResult(10000) == WL_CONNECTED)
	//if (cfg::has_wifi && !wifi_connection_lost)
	if (cfg::has_wifi && WiFi.status() == WL_CONNECTED)
	{
		server.handleClient();
		yield();
	}

	if (send_now && cfg::sending_intervall_ms >= 120000)
	{

		void *SpActual = NULL;
		Debug.printf("Free Stack at send_now is: %d \r\n", (uint32_t)&SpActual - (uint32_t)StackPtrEnd);

		if (cfg::has_wifi && !wifi_connection_lost)
		{
			last_signal_strength = WiFi.RSSI();
		}
		RESERVE_STRING(data, LARGE_STR);
		data = FPSTR(data_first_part);
		RESERVE_STRING(result, MED_STR);

		if (cfg::sds_read)
		{
			data += result_SDS;
			if (cfg::has_wifi && !wifi_connection_lost)
			{
				sum_send_time += sendSensorCommunity(result_SDS, SDS_API_PIN, FPSTR(SENSORS_SDS011), "SDS_");
			}
		}
		if (cfg::npm_read)
		{
			data += result_NPM;
			if (cfg::has_wifi && !wifi_connection_lost)
			{
				sum_send_time += sendSensorCommunity(result_NPM, NPM_API_PIN, FPSTR(SENSORS_NPM), "NPM_");
			}
		}

		if (cfg::bmx280_read && (!bmx280_init_failed))
		{
			fetchSensorBMX280(result);
			data += result;
			if (bmx280.sensorID() == BME280_SENSOR_ID)
			{
				if (cfg::has_wifi && !wifi_connection_lost)
				{
					sum_send_time += sendSensorCommunity(result, BME280_API_PIN, FPSTR(SENSORS_BME280), "BME280_");
				}
			}
			else
			{
				if (cfg::has_wifi && !wifi_connection_lost)
				{
					sum_send_time += sendSensorCommunity(result, BMP280_API_PIN, FPSTR(SENSORS_BMP280), "BMP280_");
				}
			}
			result = emptyString;
		}

		//These values are not sent because not configured in the SC API:

		if (cfg::mhz16_read)
		{
			data += result_MHZ16;
		}

		if (cfg::mhz19_read)
		{
			data += result_MHZ19;
		}

		if (cfg::ccs811_read && (!ccs811_init_failed))
		{
			data += result_CCS811;
		}

		add_Value2Json(data, F("samples"), String(sample_count));
		add_Value2Json(data, F("min_micro"), String(min_micro));
		add_Value2Json(data, F("max_micro"), String(max_micro));
		add_Value2Json(data, F("interval"), String(cfg::sending_intervall_ms));
		add_Value2Json(data, F("signal"), String(last_signal_strength));

		if ((unsigned)(data.lastIndexOf(',') + 1) == data.length())
		{
			data.remove(data.length() - 1);
		}
		data += "]}";

		yield();

		if (cfg::has_wifi && !wifi_connection_lost)
		{
			sum_send_time += sendDataToOptionalApis(data);

			//json example for WiFi transmission

			//{"software_version" : "ModuleAirV2-V1-122021", "sensordatavalues" :
			//[ {"value_type" : "NPM_P0", "value" : "1.84"},
			//{"value_type" : "NPM_P1", "value" : "2.80"},
			//{"value_type" : "NPM_P2", "value" : "2.06"},
			//{"value_type" : "NPM_N1", "value" : "27.25"},
			//{"value_type" : "NPM_N10", "value" : "27.75"},
			//{"value_type" : "NPM_N25", "value" : "27.50"},
			//{"value_type" : "BME280_temperature", "value" : "20.84"},
			//{"value_type" : "BME280_pressure", "value" : "99220.03"},
			//{"value_type" : "BME280_humidity", "value" : "61.66"},
			//{"value_type" : "samples", "value" : "138555"},
			//{"value_type" : "min_micro", "value" : "933"},
			//{"value_type" : "max_micro", "value" : "351024"},
			//{"value_type" : "interval", "value" : "145000"},
			//{"value_type" : "signal", "value" : "-71"}
			// ]}

			// https://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
			sending_time = (3 * sending_time + sum_send_time) / 4;

			if (sum_send_time > 0)
			{
				debug_outln_info(F("Time for Sending (ms): "), String(sending_time));
			}

			//RECONNECT ETAIT ICI
		}

		if ((WiFi.status() != WL_CONNECTED || sending_time > 30000 || wifi_connection_lost) && cfg::has_wifi)
		{
			debug_outln_info(F("Connection lost, reconnecting "));
			WiFi_error_count++;
			WiFi.reconnect();
			waitForWifiToConnect(20);

			if (wifi_connection_lost && WiFi.waitForConnectResult(10000) != WL_CONNECTED)
			{
				if (cfg::has_matrix)
				{
					display_update_enable(false);
				}

				debug_outln_info(F("Reconnect failed after WiFi.reconnect()"));

				WiFi.disconnect(true, true);
				// wifi_country_t wifi;
				// wifi.policy = WIFI_COUNTRY_POLICY_MANUAL;
				// strcpy(wifi.cc, INTL_LANG);
				// wifi.nchan = 13;
				// wifi.schan = 1;
				WiFi.mode(WIFI_STA);
				WiFi.setHostname(cfg::fs_ssid);
				WiFi.begin(cfg::wlanssid, cfg::wlanpwd); // Start WiFI again

				// if (MDNS.begin(cfg::fs_ssid))
				// {
				// 	MDNS.addService("http", "tcp", 80);
				// 	MDNS.addServiceTxt("http", "tcp", "PATH", "/config");
				// }

				//reConnectWifi();

				if (cfg::has_matrix)
				{
					display_update_enable(true);
				}
			}
			debug_outln_info(emptyString);
		}

		// only do a restart after finishing sending (Wifi). Befor Lora to avoid conflicts with the LMIC
		if (msSince(time_point_device_start_ms) > DURATION_BEFORE_FORCED_RESTART_MS)
		{
			sensor_restart();
		}

		// Resetting for next sampling
		last_data_string = std::move(data);
		sample_count = 0;
		last_micro = 0;
		min_micro = 1000000000;
		max_micro = 0;
		sum_send_time = 0;
		starttime = millis(); // store the start time
		count_sends++;
	}

	if (screen_now) // && cfg::screen_intervall_ms >= 300000) // LIMITE DE SECURITE?
	{
		if (cfg::display_forecast && cfg::has_wifi && !wifi_connection_lost)
		{
			getScreenAircarto(forecast_selector, "mod", esp_chipid);
		}
		else
		{
			switch (forecast_selector)
			{
			case 0:
				if (forecast_screen_0 != nullptr)
				{
					free(forecast_screen_0);
					forecast_screen_0 = nullptr;
				}
				break;
			case 1:
				if (forecast_screen_1 != nullptr)
				{
					free(forecast_screen_1);
					forecast_screen_1 = nullptr;
				}
				break;
			case 2:
				if (forecast_screen_2 != nullptr)
				{
					free(forecast_screen_2);
					forecast_screen_2 = nullptr;
				}
				break;
			case 3:
				if (forecast_screen_3 != nullptr)
				{
					free(forecast_screen_3);
					forecast_screen_3 = nullptr;
				}
				break;
			case 4:
				if (forecast_screen_4 != nullptr)
				{
					free(forecast_screen_4);
					forecast_screen_4 = nullptr;
				}
				break;
			case 5:
				if (forecast_screen_5 != nullptr)
				{
					free(forecast_screen_5);
					forecast_screen_5 = nullptr;
				}
				break;
			case 6:
				if (forecast_screen_6 != nullptr)
				{
					free(forecast_screen_6);
					forecast_screen_6 = nullptr;
				}
				break;
			case 7:
				if (forecast_screen_7 != nullptr)
				{
					free(forecast_screen_7);
					forecast_screen_7 = nullptr;
				}
				break;
			case 8:
				if (forecast_screen_8 != nullptr)
				{
					free(forecast_screen_6);
					forecast_screen_8 = nullptr;
				}
				break;
			case 9:
				if (forecast_screen_9 != nullptr)
				{
					free(forecast_screen_9);
					forecast_screen_9 = nullptr;
				}
				break;
			case 10:
				if (forecast_screen_10 != nullptr)
				{
					free(forecast_screen_10);
					forecast_screen_10 = nullptr;
				}
				break;
			}
		}

		if (cfg::display_alert && cfg::has_wifi && !wifi_connection_lost)
		{
			getScreenAircarto(0, "alert", esp_chipid);
		}
		else
		{
			if (alert_screen != nullptr)
			{
				free(alert_screen);
				alert_screen = nullptr;
			}
		}
	}

	if (sample_count % 500 == 0)
	{
		//		Serial.println(ESP.getFreeHeap(),DEC);
	}
}

/*****************************************************************
 * Gamma correction                                              *
 *****************************************************************/
const uint8_t PROGMEM gamma8[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
	2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
	5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
	10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
	17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
	25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
	37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
	51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
	69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
	90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
	115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
	144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
	177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
	215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255};
