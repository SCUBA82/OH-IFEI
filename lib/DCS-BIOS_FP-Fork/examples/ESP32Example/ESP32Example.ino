/*
 * Example using ESP32 mode
 */
 
#define DCSBIOS_LAN    // LAN Access
#define DCSBIOS_ESP32  // ESP32 microcontroller

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
