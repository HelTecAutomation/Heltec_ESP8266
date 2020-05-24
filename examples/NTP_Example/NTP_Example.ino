// WiFiKit8_NTP2
// 23 May 2020
// The Grand Poohbah

// NTP1 uses UDP to access NTP servers directly to get the time.
// NTP2 uses the NTPClient to get the time.

// Adapted to WiFi Kit 8 from NetworkTimeProtocol.ino
// From: https://www.hackster.io/hammadiqbal12/internet-clock-ntp-58ffd8 (17 May 2019)

// Dependent libraries (must install):
// NTP library: https://hacksterio.s3.amazonaws.com/uploads/attachments/893468/ntpclient_MSBZvSJiRa.rar
// Time Library: https://hacksterio.s3.amazonaws.com/uploads/attachments/893471/time_ltyS0W3HlF.rar

// Board: WiFi Kit 8
// Port:  /dev/cu.SLAB_USBtoUART

// Network Time Protocol (NTP) is a protocol used to synchronize computer clock times in a network.
// It belongs to and is one of the oldest parts of the TCP/IP protocol suite.
// The term NTP applies to both the protocol and the client-server programs that run on computers.

// NTP, which was developed by David Mills at the University of Delaware in 1981,
// is designed to be highly fault-tolerant and scalable.

// How does NTP work?
// The NTP client initiates a time-request exchange with the NTP server.
// As a result of this exchange, the client is able to calculate the link delay
// and its local offset, and adjust its local clock to match the clock at the server's computer.
// As a rule, six exchanges over a period of about 5 to 10 minutes are required
// to initially set the clock.

#include <Time.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <heltec.h>

const char *ssid = "WiFi SSID";
const char *pass = "WiFi Password";

// PDT
const long kTimeZone = 8;
int gDay, gMonth, gYear, gSeconds, gMinutes, gHours;
int gDayLast, gMonthLast, gYearLast, gSecondsLast, gMinutesLast, gHoursLast;
bool gAMlast;
long int gTimeNow;
const char *myMonthStr[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const char *myDowStr[] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Query interval
#define kNtpInterval 1000
// Set the timer to trigger immediately.
unsigned long gNtpTimer = 0;

void setup() {
  Serial.begin(115200);
  gDayLast = gMonthLast = gYearLast = gSecondsLast = gMinutesLast = gHoursLast = 0;
  gAMlast = true;
  // Initialize OLED
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Serial Enable*/);
 // We start by connecting to a WiFi network
  setupWiFi();
  drawStatus();
  // The timeClient does all the NTP work.
  timeClient.begin();
}

void loop() {
    // Check time
  if(millis() >= gNtpTimer) {
    gNtpTimer = millis() + kNtpInterval;
    timeClient.update();
    // Get epoch time and adjust it according to the local time zone.
    gTimeNow = timeClient.getEpochTime() + (kTimeZone * 3600);
    gDay = day(gTimeNow);
    gMonth = month(gTimeNow);
    gYear = year(gTimeNow);
    gHours = hourFormat12(gTimeNow);
    gMinutes = minute(gTimeNow);
    gSeconds = second(gTimeNow);
    drawDate();
    drawTime();
    /*
    Serial.print(gTimeNow);
    Serial.print(" ");
    Serial.print(gDay);
    Serial.print("/");
    Serial.print(gMonth);
    Serial.print("/");
    Serial.print(gYear);
    Serial.print(" ");
    Serial.print(gHours);
    Serial.print(":");
    Serial.print(gMinutes);
    Serial.print(":");
    Serial.print(gSeconds);
    Serial.println();
    */
  }
}

// OLED Methods

void drawStatus() {
  Heltec.display->clear();
    Heltec.display->drawString(0, 0, "Connected to " + String(ssid));
  Heltec.display->drawString(0, 10, "IP Address: " + WiFi.localIP().toString());
  Heltec.display->display();
  delay(5000);
  Heltec.display->clear();
}

void drawDate() {
  // Assemble date string.
  String dateStr = String(myDowStr[weekday(gTimeNow)]) + "  " + String(gDay) + " " + String(myMonthStr[gMonth]);
  // Small font for date display
  Heltec.display->setFont(ArialMT_Plain_10);
  if(gDay != gDayLast) {
    // New day. Clear the entire display.
    gDayLast = gDay;
    Heltec.display->clear();
  }
  Heltec.display->drawString(45, 0, dateStr);
  Heltec.display->display();
}

void drawTime() {
  // Large font for time display
  Heltec.display->setFont(ArialMT_Plain_24);
  // Erase last value if needed and set new last value.
  gHoursLast = eraseOldValueIfNeeded(5, gHoursLast, gHours);
  gMinutesLast = eraseOldValueIfNeeded(39, gMinutesLast, gMinutes);
  gSecondsLast = eraseOldValueIfNeeded(74, gSecondsLast, gSeconds);
  // Draw new time
  Heltec.display->setColor(WHITE);
  Heltec.display->drawString(5, 10, makeStringWithLeadingZeroIfNeeded(gHours));
  Heltec.display->drawString(31, 10, ":");
  Heltec.display->drawString(39, 10, makeStringWithLeadingZeroIfNeeded(gMinutes));
  Heltec.display->drawString(67, 10, ":");
  Heltec.display->drawString(74, 10, makeStringWithLeadingZeroIfNeeded(gSeconds));
  addAMorPM();
  Heltec.display->display();
}

void addAMorPM() {
    // Small font for AM/PM
  Heltec.display->setFont(ArialMT_Plain_10);
  if (isPM(gTimeNow)) {
    // PM
    if(gAMlast) {
      // Change from AM to PM, erase AM
      gAMlast = false;
      Heltec.display->setColor(BLACK);
      Heltec.display->drawString(105, 22, "AM");
      Heltec.display->display();
      Heltec.display->setColor(WHITE);
    }
    Heltec.display->drawString(105, 22, "PM");
  }
  else {
    // AM
    if(!gAMlast) {
      // Change from PM to AM, erase PM
      gAMlast = true;
      Heltec.display->setColor(BLACK);
      Heltec.display->drawString(105, 22, "PM");
      Heltec.display->display();
      Heltec.display->setColor(WHITE);
    }
    Heltec.display->drawString(105, 22, "AM");
  }
}

// Add a leading zero if the time is one digit.
String makeStringWithLeadingZeroIfNeeded(int theTime) {
  String timeStr;
  if (theTime < 10)
    timeStr = "0" + String(theTime);
  else
    timeStr = String(theTime);
  return timeStr;
}

// Draw the old value in black to erase it.
// This avoids the flicker of the entire screen caused by clear.
int eraseOldValueIfNeeded(int xLoc, int oldValue, int newValue) {
  if(newValue != oldValue) {
    // Value changed, erase old value by redrawing it in black.
    String oldValueStr = makeStringWithLeadingZeroIfNeeded(oldValue);
    Heltec.display->setColor(BLACK);
    Heltec.display->drawString(xLoc, 10, oldValueStr);
    Heltec.display->display();
  }
  return newValue;
}

void setupWiFi(void)
{
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.begin(ssid, pass);
  delay(100);
  Heltec.display->clear();

  byte count = 0;
  String connectingStr = "Connecting";
  while(WiFi.status() != WL_CONNECTED && count < 100)
  {
    count ++;
    delay(500);
    Heltec.display->drawString(0, 0, connectingStr);
    Heltec.display->display();
    // Display a dot for each attempt.
    connectingStr += ".";
  }
  //Heltec.display->clear();
  if(WiFi.status() != WL_CONNECTED)
  {
    Heltec.display->drawString(0, 9, "Failed");
    Heltec.display->display();
    delay(1000);
    Heltec.display->clear();
  }
}