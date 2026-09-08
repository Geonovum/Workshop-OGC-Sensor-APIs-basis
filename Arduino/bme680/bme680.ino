/*
 * IoT Sensor Project - Step 7: Sensor Data Collection
 * 
 * This Arduino sketch implements a comprehensive IoT sensor system that:
 * - Connects to WiFi for data transmission
 * - Collects GPS location data
 * - Reads sensor data
 * - Transmits sensor values to a remote server/cloud (Sensor Things API)
 * 
 * Hardware: XIAO ESP32 C3 with various sensors
 * Author: IoT Project
 * Date: aug 2025
 */

#include <Arduino_JSON.h>  // from https://github.com/arduino-libraries/Arduino_JSON

// Include helper libraries for different system components
#include "../arduino_secrets.h"  // WiFi credentials and secrets

#include "../helpers/logging.h"  // Serial communication and logging setup
#include "../helpers/wifi.h"     // WiFi connection management
#include "../helpers/stringFormat.h"

#include <HTTPClient.h>  // ships with ESP32 library, no extra lib needed

const char* serviceHost = "https://xxxx/FROST-Server";
const char* serviceVersion = "v1.1";

HTTPClient http;

// Function declaration for data transmission
void transmitValue(float value, char* UoM, uint datastreamId);

#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C

/**
 * Setup function - runs once at startup
 * Initializes all system components in the correct order
 */
void setup() {
  // Initialize system components
  setupLogging();  // Start serial communication at 115200 baud
  setupWiFi();     // Start WiFi connection process

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1)
      ;
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);  // 320*C for 150 ms
}

/**
 * Transmit sensor value to remote server/cloud
 * 
 * @param value The sensor reading to transmit
 * 
 * This function handles the data transmission logic. Currently it only
 * logs the value, but can be extended to send data via HTTP POST,
 * MQTT, or other protocols.
 */
void transmitValue(float value, char* UoM, uint datastreamId) {
  if (WiFi.status() == WL_CONNECTED) {
    // WiFi is connected - ready to transmit data
    Serial.printf("Value: %.2f%s (Datastream id: %d)\n", value, UoM, datastreamId);

    // Example implementations:
    // - HTTP POST
    // - MQTT publish

    /*
    JSONVar point;
    point["type"] = "Point";
    JSONVar coordinates;
    coordinates[0] = lat;
    coordinates[1] = lng;
    point["coordinates"] = coordinates;

    JSONVar featureOfInterest;
    featureOfInterest["name"] = "hier"; // TODO
    featureOfInterest["description"] = "iets meer naar ginder"; // TODO
    featureOfInterest["encodingType"] = "application/vnd.geo+json";
    featureOfInterest["feature"] = point;
*/

    JSONVar observation;
    //  observation["FeatureOfInterest"] = featureOfInterest;
    //  observation["phenomenonTime"] = getISO8601dateTime();
    observation["result"] = value;

    const String url = formatString("%s/%s/Datastreams(%d)/Observations", serviceHost, serviceVersion, datastreamId);
    auto body = JSON.stringify(observation);

    Serial.print("HTTP POST to ");
    Serial.print(url);
    Serial.print(" with body ");
    Serial.println(body);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    auto httpCode = http.POST(body);
    Serial.printf("[HTTP] Result of HTTP POST %d\n", httpCode);
    http.end();

  } else {
    // WiFi not connected - log the value for debugging
    Serial.printf("[WiFi] Not connected to Transmit. Value: %.2f%s (Datastream Id: %d)\n", value, UoM, datastreamId);
  }
}

/**
 * Main loop function - runs forever
 * 
 * This function implements a non-blocking architecture where each
 * component (WiFi, GPS, sensor) is checked and updated independently.
 * This prevents one component from blocking the others.
 */
void loop() {
  // Check and update WiFi connection status
  loopWifi();

  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);

  Serial.println(F("You can do other work during BME680 measurement."));
  delay(50); // This represents parallel work.
  // There's no need to delay() until millis() >= endTime: bme.endReading()
  // takes care of that. It's okay for parallel work to take longer than
  // BME680's measurement time.

  // Obtain measurement results from BME680. Note that this operation isn't
  // instantaneous even if milli() >= endTime due to I2C/SPI latency.
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }

  Serial.print(F("Reading completed at "));
  Serial.println(millis());

  transmitValue(bme.temperature, "*C", 10);
  transmitValue(bme.pressure / 100.0, "hPa", 10);
  transmitValue(bme.humidity, "%", 10);
  transmitValue(bme.readAltitude(SEALEVELPRESSURE_HPA), "m", 10);

  Serial.println();
  delay(2000);
}