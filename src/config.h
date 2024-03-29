#ifndef CONFIG_H
#define CONFIG_H

//#######################  User Settings ####################################
/*Which DCS-BIOS fork are you using? 
------------------------------------------------------------
DCSBIOS_DEFAULT = https://github.com/DCS-Skunkworks/dcs-bios-arduino-library
Using USB for data communication
------------------------------------------------------------
DCSBIOS_WIFI_FORK = https://github.com/SCUBA82/dcs-bios-arduino-library
Using Wifi for data communication
*/

//#define DCSBIOS_DEFAULT   
#define DCSBIOS_WIFI_FORK   

// Wifi credentials if Wifi is enabled
#ifdef DCSBIOS_WIFI_FORK
  char ssid[] = "Freudenhaus2";
  char passwd[] = "aquarius";
#endif 


/* Enable DCS BIOS input command forward
   Disable this option if no physical inputs are connected to the ZX7D00CE01S board.
*/
//#define ENABLE_DCS_BIOS_INPUTS

#endif