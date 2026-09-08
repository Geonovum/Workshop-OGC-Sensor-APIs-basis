#pragma once

#include <TinyGPSPlus.h> // https://github.com/mikalhart/TinyGPSPlus

// Reference coordinates for Amersfoort, Netherlands
static const double AMERSFOORT_LAT = 52.1561113;
static const double AMERSFOORT_LON = 5.3878266;

// Updated pin definitions for XIAO ESP32 C3
#define RXPin D7          // GPS module TX connects to ESP32 D7 (RX)
#define TXPin D6          // GPS module RX connects to ESP32 D6 (TX) - optional
const uint32_t GNSSBaud = 9600;

#define SERIALGNSS Serial1

double lat;
double lng;

TinyGPSPlus gnss;

// GPS timeout variables
unsigned long lastGNSSUpdate = 0;    // Timestamp of last GPS data update
bool gnssTimeoutReported = false;

void setupGNSS()
{
  Serial.println("[GNSS] Initializing...");
  SERIALGNSS.begin(GNSSBaud, SERIAL_8N1, RXPin, TXPin);
  Serial.println("[GNSS] Initialized");
}

void loopGNSS()
{
  while (SERIALGNSS.available() > 0) {
    if (gnss.encode(SERIALGNSS.read())) {
      // GPS data was successfully parsed
      if (gnss.location.isUpdated()) {
        lat = gnss.location.lat();
        lng = gnss.location.lng();
        lastGNSSUpdate = millis();
        gnssTimeoutReported = false; // Reset timeout flag
        
        Serial.printf("[GNSS] Lat=%.6f, Lng=%.6f\n", lat, lng);
      }
    }
  }
  
  // Check for GPS timeout (30 seconds without update)
  if (!gnssTimeoutReported && (millis() - lastGNSSUpdate > 30000)) {
    Serial.println("[GNSS] Timeout - using default coordinates");
    gnssTimeoutReported = true;
  }
}

/*
 * Helper functions for GPS data display
 * 
 * These functions provide formatted output for GPS data values,
 * handling invalid data gracefully by displaying asterisks.
 */

/**
 * Prints a float value with proper formatting and validation
 * 
 * @param val The float value to print
 * @param valid Whether the value is valid
 * @param len Total field width for formatting
 * @param prec Number of decimal places to display
 */
static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    // Print asterisks for invalid data
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    // Print the valid float value
    Serial.print(val, prec);
    
    // Calculate the actual length of the printed number
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // Account for decimal point and minus sign
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    
    // Add padding spaces to maintain column alignment
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
}

/**
 * Prints an integer value with proper formatting and validation
 * 
 * @param val The unsigned long value to print
 * @param valid Whether the value is valid
 * @param len Total field width for formatting
 */
static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  
  // Add padding spaces to maintain column alignment
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
}

/**
 * Prints date and time from GPS with validation
 * 
 * @param d TinyGPSDate object containing date information
 * @param t TinyGPSTime object containing time information
 */
static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  // Print date (MM/DD/YY format)
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  // Print time (HH:MM:SS format)
  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  // Print age of the date/time fix
  printInt(d.age(), d.isValid(), 5);
}

/**
 * Prints a string with fixed-width formatting
 * 
 * @param str The string to print
 * @param len The desired field width
 */
static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
}

/**
 * Custom delay function that ensures GPS data is continuously processed
 * This prevents the GPS buffer from overflowing and ensures fresh data
 * 
 * @param ms Number of milliseconds to delay
 */
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    // Process any available GPS data while waiting
    while (Serial1.available()) {
      gnss.encode(Serial1.read());
    }
  } while (millis() - start < ms);
}
