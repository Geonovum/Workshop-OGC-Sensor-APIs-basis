#pragma once

#define LED_BUILTIN D2

void setupLogging()
{
  Serial.begin(115200);
  while (!Serial);
  delay(50);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("[SYS ] Booting...");
}
