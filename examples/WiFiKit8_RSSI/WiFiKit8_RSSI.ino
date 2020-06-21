// WiFiKit8_RSSI
// 20 Jun 2020
// The Grand Poohbah

// Display the signal strength of every SSID on the network.

// Acceptable Signal Strengths
// RSSI stands for Received Signal Strength Indicator
// Signal Strength	TL;DR	 	Required for
// -30 dBm	Amazing		Max achievable signal strength. The client can only be a few feet from the AP to achieve this. Not typical or desirable in the real world.	N/A
// -67 dBm	Very Good	Minimum signal strength for applications that require very reliable, timely delivery of data packets.	VoIP/VoWiFi, streaming video
// -70 dBm	Okay		Minimum signal strength for reliable packet delivery.	Email, web
// -80 dBm	Not Good	Minimum signal strength for basic connectivity. Packet delivery may be unreliable.	N/A
// -90 dBm	Unusable	Approaching or drowning in the noise floor. Any functionality is highly unlikely.	N/A
// From https://www.metageek.com/training/resources/understanding-rssi.html

#include "ESP8266WiFi.h"
#include <heltec.h>

#ifndef DISPLAY_WIDTH
#define DISPLAY_WIDTH 128
#endif

// The RSSI range is -90 to -10
const int kMinAbsRSSI = 10;
const int kMaxAbsRSSI = 90;

// Timer
unsigned long gTimer = 0;
unsigned long kTimeInterval = 2000; // 2 seconds

void setup() {
  Serial.begin(115200);
  // Initialize OLED
  Heltec.begin(true /*DisplayEnable Enable*/, false /*Serial Enable*/);
  // Set WiFi to station mode
  // and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println();
  Serial.println("Scan Networks");
  drawStatus("Scan Networks");
  // Start right away
  gTimer = 0;
}

void loop() {
  if(millis() >= gTimer) {
    // Time for an update. Reset timer.
    gTimer = millis() + kTimeInterval;
    int n = WiFi.scanNetworks();
    for(int i=0; i<n; i++) {
      // Display on Serial Monitor
      Serial.print(WiFi.SSID(i));
      Serial.print(" ");
      Serial.println(WiFi.RSSI(i));
      // Display on OLED
      drawBar(WiFi.RSSI(i));
      scrollLeft(WiFi.SSID(i), WiFi.RSSI(i));
    }
  }
}

// OLED Methods

// Draw the RSSI as a bar on the top line. Stronger is longer.
void drawBar(int rssi) {
  // Clear display
  Heltec.display->clear();
  // Draw new bar
  Heltec.display->setColor(WHITE);
  // Make sure rssi is within our bounds.
  int absRSSI = abs(rssi);
  if(absRSSI < kMinAbsRSSI) { absRSSI = kMinAbsRSSI; }
  else if(absRSSI > kMaxAbsRSSI) { absRSSI = kMaxAbsRSSI; }
  // Scale the rssi to the width of the display.
  // A large negative value is weaker, a small negative value is stronger.
  // Reverse the values so that smaller values have longer bars.
  int barWidth = map(absRSSI, kMinAbsRSSI, kMaxAbsRSSI, DISPLAY_WIDTH, 0);
  Heltec.display->fillRect(0, 0, barWidth, 8);
  Heltec.display->display();
  // Draw segment markers every 20 units.
  Heltec.display->setColor(BLACK);
  for(int x=20; x<DISPLAY_WIDTH; x+=20) {
    Heltec.display->drawLine(x, 0, x, 8);
    Heltec.display->display();
  }  
  // Restore white for text
  Heltec.display->setColor(WHITE);
}

// Draw theStatus centered on the top line
void drawStatus(String theStatus) {
  // Clear display
  Heltec.display->clear();
  // Small font for status display
  Heltec.display->setFont(ArialMT_Plain_10);
  // Scroll left and leave the RSSI.
  int stringWidth = Heltec.display->getStringWidth(theStatus);
  int x = (DISPLAY_WIDTH - stringWidth) / 2;
  Heltec.display->drawString(x, 0, theStatus);
  Heltec.display->display();
}

// Scroll theSSID and theRSSI from right to left on the bottom line
void scrollLeft(String theSSID, int theRSSI) {
  String theMessage = theSSID + " " + String(theRSSI);
  // Large font
  Heltec.display->setFont(ArialMT_Plain_24);
  // Scroll far enough to leave the RSSI showing at the end of the scroll.
  int xEnd = -Heltec.display->getStringWidth(theSSID);
  for(int x=DISPLAY_WIDTH; x>=xEnd; x--){
    Heltec.display->drawString(x, 5, theMessage);
    Heltec.display->display();
    delay(10);
    yield();
    // Erase the text that we just drew.
    Heltec.display->setColor(BLACK);
    Heltec.display->drawString(x, 5, theMessage);
    Heltec.display->setColor(WHITE);
  }
}
