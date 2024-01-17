/*
 * Example using ESP8266 mode
 */
 
#define DCSBIOS_LAN     // WiFi-LAN Access
#define DCSBIOS_ESP8266 // ESP8266 microcontroller

char ssid[]   = "SSID_LAN";
char passwd[] = "password";

#include "DcsBios.h"

void setup() {
  // put your setup code here, to run once:
  DcsBios::setup(ssid,passwd);
}

void loop() {
  // put your main code here, to run repeatedly:
  DcsBios::loop();
}
