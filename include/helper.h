#ifndef HELPER_H
#define HELPER_H
#include <Arduino.h>
#include <display_driver.h>


//DCS BIOS Forks
#ifdef DCSBIOS_WIFI_FORK 
    #define DCSBIOS_LAN    // LAN Access
    #define DCSBIOS_ESP32  // ESP32 microcontroller
    #define SERIAL_LOG 0   //Set to 1 to enable serial debugging 
#endif

#ifdef DCSBIOS_DEFAULT
    #define DCSBIOS_DEFAULT_SERIAL
#endif 

// InputPins
#ifdef ENABLE_DCS_BIOS_INPUTS
  #define BRIGTHNESS_PIN GPIO_NUM_2
  #define MODE_SELECTOR_MAN GPIO_NUM_1
  #define MODE_SELECTOR_AUTO GPIO_NUM_42
  #define SELECT_HUD_PIN GPIO_NUM_4
  #define SELECT_LDDI_PIN GPIO_NUM_40
  #define MODE_AW9523_PIN 0  
  #define QTY_AW9523_PIN  1
  #define ARROW_UP_AW9523_PIN 2
  #define ARROW_DOWN_AW9523_PIN 3
  #define ZONE_AW9523_PIN 4
  #define ET_AW9523_PIN 5
#endif 

//Variables

//Clock
String TC_H = "00";  //TOP clock hours
String TC_Dd1 = ":"; //TOP clock colon 1 
String TC_M = "00";  //TOP clock minutes
String TC_Dd2 = ":"; //TOP clock colon 2
String TC_S = "00";  //TOP clock seconds

String LC_H = "0 ";   //Lower clock hours
String LC_Dd1 = ":"; //Lower clock colon 1
String LC_M = "00";  //Lower clock minutes
String LC_Dd2 = ":"; //Lower clock colon 2
String LC_S = "00";  //Lower clock seconds

// IFEI COLOR MODE

unsigned int ifei_color = 0xFFFFFFU;
const unsigned int color_day = 0xFFFFFFU;
const unsigned int color_NIGHT = 0x1CDD2AU;

// NOZL Gauge elements visibility
bool NOZL_pointer_visible = true;
bool NOZR_pointer_visible = true;
bool NOZL_scale_visible = true;
bool NOZR_scale_visible = true;
bool NOZL_numbers_visible = true;
bool NOZR_numbers_visible = true;

// lighting test switch active
bool test_switch_enabled = false;


//Aircraft 
bool ishornet = false;
// demo variables
unsigned long nozzle_update = 0;
bool demo_forward = true;
bool reset = false;
int demo_counter = 0;


// Create tft screen 
LGFX tft;




// Create sprites 

LGFX_Sprite TWOD(&tft);   // Data digits sprite (RPM/OIL)
LGFX_Sprite THREED(&tft); // Data digits sprite (TEMP/FF)
LGFX_Sprite LABELS(&tft); // Text label sprite 
LGFX_Sprite ffT(&tft);    // Special text sprite label for fuel flow
LGFX_Sprite Fuel(&tft);   //Fuel sprite
LGFX_Sprite CLOCK(&tft);  //Clock sprite 

LGFX_Sprite NOZL_IMAGE[27] //Left nozzel gauge sprites
{ LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft)
  };
  
LGFX_Sprite NOZR_IMAGE[27] //Right nozzel gauge sprites
{
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft),
  LGFX_Sprite(&tft)
};

LGFX_Sprite TAG(&tft); //Special Tags sprite (L/R/Z)



// Create data structure for display elements
struct display_element{
  int sprite_width;
  int sprite_hight;
  int pos_x;
  int pos_y;
  int textalign;
  LGFX_Sprite* sprite;
  const char* value;
  const char* old_value;
};

// Enumeration of display elements
enum Display_Name{
  RPML,
  RPMR,
  RPMT,
  TMPL,
  TMPR,
  TMPT,
  FFL,
  FFR,
  FFTU,
  FFTL,
  OILL,
  OILR,
  OILT,
  NOZL,
  NOZR,
  NOZT,
  FUELU,
  FUELL,
  BINGO,
  BINGOT,
  CLOCKU,
  CLOCKL,
  ZULU,
  L,
  R
};

// Functions
char* remove_trailing_spaces(const char* str) {
    if (str == nullptr || *str == '\0') {
        return nullptr; 
    }

    int length = strlen(str);
    while (length > 0 && isspace(str[length - 1])) {
        length--;
    }

    char* result = new char[length + 1];
    strncpy(result, str, length);
    result[length] = '\0';

    return result;
}
#endif

