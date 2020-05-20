#ifndef _HELTEC_H_
#define _HELTEC_H_

#if defined(ESP8266)

#include <Arduino.h>

#if defined( WIFI_Kit_8 )
#include <Wire.h>
#include "oled/SSD1306Wire.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32

#endif

class Heltec_ESP8266 {

 public:
    Heltec_ESP8266();
	~Heltec_ESP8266();

    void begin(bool DisplayEnable=true, bool SerialEnable=true);
    SSD1306Wire *display;
};

extern Heltec_ESP8266 Heltec;

#else
#error "This library only supports boards with ESP8266 processor."
#endif


#endif
