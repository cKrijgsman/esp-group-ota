#include <WiFi.h> //WIFI setup
#include <ArduinoHttpClient.h> //http download
#include <AsyncUDP.h> //sending/receiving UDP packets (commands)
#include <ArduinoOTA.h> // only for InternalStorage
#include <EEPROM.h>

void setup() {
  // Set up serial
  Serial.begin(115200);
  while (!Serial);

  // Set up OTA
  otaSetup("Ziggo9794608","r76Jfaxnhtje",4000);
}

void loop() {
  // Just used for demonstation.
  Serial.println("Just printing and waiting!");
  delay(1000);
}
