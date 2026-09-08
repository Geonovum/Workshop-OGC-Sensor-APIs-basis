#pragma once

void setupLogging()
{
  Serial.begin(115200);
  while (!Serial);
  delay(50);

  Serial.println("[SYS ] Booting...");
}
