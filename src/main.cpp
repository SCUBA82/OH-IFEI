#include <Arduino.h>



/*
####### Stand alone DCS-BIOS IFEI simulation using a ZX7D00CE01S esp32-s3 7" touchscreen 
Notes:
While using active wifi, I had screen glitches with the default TFT clk frequency provided in the PANELAN library
Change frequency in PanelLan_esp32_arduino/src/board/sc05/sc05.cpp to
#define PanelLan_RGB_CLK_FREQ           (16000000)
*/


#include <fonts.h> //Custom Fonts
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "SPIFFS.h"



#include <PanelLan.h>



#define DCSBIOS_LAN    // LAN Access
#define DCSBIOS_ESP32  // ESP32 microcontroller

#define SERIAL_LOG 1
#include  <DcsBios.h>


#define SCK 40 
#define MISO 42
#define MOSI 2
#define CS 41



SPIClass SDCARD_SPI = SPIClass(HSPI);

 



// MISC Variables




char ssid[] = "Freudenhaus2";
char passwd[] = "aquarius";


//Clock
String TC_H = "00";
String TC_Dd1 = ":";
String TC_M = "00";
String TC_Dd2 = ":";
String TC_S = "00";

String LC_H = "00";
String LC_Dd1 = ":";
String LC_M = "00";
String LC_Dd2 = ":";
String LC_S = "00";

// IFEI COLOR MODE
String NOZL_PATH = "White";
int ifeiCol;
unsigned int ifei_color = 0xFFFFFFU;
const unsigned int color_day = 0xFFFFFFU;
const unsigned int color_NIGHT = 0x1CDD2AU;


// Create tft
PanelLan tft(BOARD_SC05);


// Sprites 
LGFX_Sprite THREED(&tft);
LGFX_Sprite LABELS(&tft);
LGFX_Sprite ffT(&tft);
LGFX_Sprite Fuel(&tft);
LGFX_Sprite CLOCK(&tft);
LGFX_Sprite NOZL_IMAGE(&tft);
LGFX_Sprite SMALL_BLOCK(&tft);

static const lgfx::U8g2font segments26( segments );
static const lgfx::U8g2font segmentsa42( bdf_font );
static const lgfx::U8g2font clock28_font( clock28 );
static const lgfx::U8g2font nirmala_font( nirmala );


struct display_element{
  int sprite_width;
  int sprite_hight;
  int pos_x;
  int pos_y;
  int textalign;
  LGFX_Sprite* sprite;
  const char* value;
};

// Drawing positions in pixel
// RPM

enum Display_Name{
  RPML,
  RPMR,
  RPMT,
  TMPL,
  TMPR,
  TMPT,
  FFL,
  FFR,
  FFT,
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

display_element display_elements[24]= {
  //width, hight, posx, posy, textalign
  {110, 56, 58, 20,2,&THREED,"012"}, //RPML
  {110, 56,250, 20,0,&THREED,"345"}, //RPMR
  { 55, 70,180, 20,1,&LABELS,"RPM"}, //RPMT
  {110, 56, 58, 85,2,&THREED,"678"}, //TMPL
  {110, 56,250, 85,0,&THREED,"0"}, //TMPR
  { 55, 70,180, 85,1,&LABELS,"TEMP"}, //TMPT
  {110, 56, 58,160,2,&THREED,"0"}, //FFL
  {110, 56,250,160,2,&THREED,"0"}, //FFR
  { 55, 70,180,160,1,&LABELS," FF \nX100"}, //FFT
  {110, 56, 58,400,2,&THREED,"0"}, //OILL
  {110, 56,250,400,0,&THREED,"0"}, //OILR
  { 55, 70,180,400,1,&LABELS,"OIL"}, //OILT
  {150,153, 58,230,0,&NOZL_IMAGE,"L0.bmp"}, //NOZL
  {150,153,211,230,0,&NOZL_IMAGE,"R0.bmp"}, //NOZR
  { 55, 70,180,300,1,&LABELS,"NOZ"}, //NOZT
  {190, 56,550, 30,2,&CLOCK,"0"}, //FUELU
  {190, 56,550, 85,2,&CLOCK,"0"}, //FUELL
  {190, 56,550,215,2,&CLOCK,"0"}, //BINGO
  { 55, 56,580,185,1,&LABELS,"BINGO"}, //BINGOT
  {190, 56,550,310,0,&CLOCK,"0"}, //CLOCKU
  {190, 56,550,375,0,&CLOCK,"0"}, //CLOCKL
  { 30, 40,740,350,1,&SMALL_BLOCK,"Z"}, //ZULU
  { 30, 40,740, 70,1,&SMALL_BLOCK,"L"}, //L
  { 30, 40,740,131,1,&SMALL_BLOCK,"R"}, //R
};

void create_display_elements(){
//RPM
 
// Create Sprites
  THREED.createSprite(display_elements[RPML].sprite_width, display_elements[RPML].sprite_hight);
  THREED.setFont(&segmentsa42);
  THREED.setColorDepth(24);
  THREED.setTextWrap(false);
  THREED.setTextColor(ifei_color);
  
  LABELS.createSprite(display_elements[RPMT].sprite_width, display_elements[RPMT].sprite_hight);
  LABELS.setFont(&nirmala_font);
  //LABELS.setTextSize(0.5);
  LABELS.setColorDepth(24);
  LABELS.setTextColor(ifei_color);

  CLOCK.createSprite(display_elements[CLOCKU].sprite_width, display_elements[CLOCKU].sprite_hight);
  CLOCK.setFont(&segmentsa42);
  CLOCK.setColorDepth(24);
  CLOCK.setTextWrap(false);
  CLOCK.setTextColor(ifei_color);

  NOZL_IMAGE.createSprite(display_elements[NOZL].sprite_width, display_elements[NOZL].sprite_hight);
  NOZL_IMAGE.fillScreen(ifei_color);
  NOZL_IMAGE.setColorDepth(24);
  
  SMALL_BLOCK.createSprite(20, 20);
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
   Serial.printf("Listing directory: %s\r\n", dirname);

   File root = fs.open(dirname);
   if(!root){
      Serial.println("− failed to open directory");
      return;
   }
   if(!root.isDirectory()){
      Serial.println(" − not a directory");
      return;
   }

   File file = root.openNextFile();
   while(file){
      if(file.isDirectory()){
         Serial.print("  DIR : ");
         Serial.println(file.name());
         if(levels){
            listDir(fs, file.name(), levels -1);
         }
      } else {
         Serial.print("  FILE: ");
         Serial.print(file.name());
         Serial.print("\tSIZE: ");
         Serial.println(file.size());
      }
      file = root.openNextFile();
   }
}

int set_textalignment(int element,int alignment, bool is_label){
  //alignment 0=left; 1=middle; 2=right
  int fontwidth=0;
  int sprite_width=0;
  for (const char* ptr = display_elements[element].value; *ptr != '\0'; ++ptr){
    if ( element != 15 || element != 16){
      if (!std::isspace(static_cast<unsigned char>(*ptr))) {
        if (element == RPMT || element == TMPT || element == FFT ||  element == NOZT || element == OILT  ){
          fontwidth += (segments26.max_char_width() + 2)/2;    
        }else{
          fontwidth += segmentsa42.max_char_width() + 2;
        }  
      }
    }else{
        fontwidth += segmentsa42.max_char_width();
    }
  }

  if (alignment == 2){
    return display_elements[element].sprite_width - fontwidth;
  }else if (alignment == 1){
    return (display_elements[element].sprite_width - fontwidth)/2;
  }else{
    return 0;
  }
}
/*
void update_element(const char* value,int element){
  
  
  THREED.clear();
  THREED.setCursor(1,2);
  //display_elements[element].sprite.setTextColor(ifei_color);
  THREED.print(value);
  THREED.pushSprite(display_elements[element].pos_x,display_elements[element].pos_x);
}
*/

void update_element(int element){
  int x1 = set_textalignment(element, display_elements[element].textalign, false);
  
  display_elements[element].sprite->clear();
  display_elements[element].sprite->setCursor(x1,1);
  if ( element == FUELU || element == FUELL || element == BINGO ){
    display_elements[element].sprite->setFont(&segmentsa42);
  }
  display_elements[element].sprite->setTextColor(ifei_color);
  display_elements[element].sprite->print(display_elements[element].value);
  display_elements[element].sprite->pushSprite(display_elements[element].pos_x,display_elements[element].pos_y);
  
}

/*
void update_label(const char* value,int element){
  int x1 = set_textalignment(LABELS, value,1, true);
  LABELS.clear();
  LABELS.setCursor(x1,1);
  LABELS.print(value);
  LABELS.pushSprite(display_elements[element].pos_x,display_elements[element].pos_y);
  

}
void update_ffT(const char* value1,const char* value2,int x,int y){
  int x1 = set_textalignment(ffT, value1,1, true);
  ffT.clear();
  ffT.setCursor(x1,1);
  ffT.print(value1);
  int x2 = (ffT, value2,"middle", true);
  ffT.setCursor(x2,17);
  ffT.print(value2);
  ffT.pushSprite(x,y);
}

void update_Fuel(const char* value, int element){
  int x1 = set_textalignment(Fuel, value,2, false);
  CLOCK.clear();
  CLOCK.setCursor(x1,1);
  CLOCK.print(value);
  CLOCK.pushSprite(x,y);
}

void update_Bingo(const char* value,int x,int y){
  int x1 = set_textalignment(CLOCK, value,2, false);
  CLOCK.clear();
  CLOCK.setCursor(x1,1);
  CLOCK.print(value);
  CLOCK.pushSprite(x,y);
}
*/
void update_Clock(int element){
  String TIME;
  if (element == 19){
     TIME = TC_H + TC_Dd1 + TC_M + TC_Dd2 + TC_S;
  }else{
     TIME = LC_H + LC_Dd1 + LC_M + LC_Dd2 + LC_S;
  }
 display_elements[element].value = TIME.c_str();
  
  display_elements[element].sprite->clear();
  display_elements[element].sprite->setCursor(1,1);
  display_elements[element].sprite->setFont(&clock28_font);
  display_elements[element].sprite->print(TIME);
  display_elements[element].sprite->pushSprite(display_elements[element].pos_x,display_elements[element].pos_y);
}

void update_nozzel(int element){
  String filename = "/" + NOZL_PATH + "/" + display_elements[element].value;
  //NOZL_IMAGE.drawBmp(SD, filename.c_str(), 0, 0);
  
  NOZL_IMAGE.drawBmp(SPIFFS, filename.c_str(), 0, 0);
  NOZL_IMAGE.pushSprite(display_elements[element].pos_x,display_elements[element].pos_y);
  update_element(NOZT);
}


  

//################## RPM  ##################Y
void onIfeiRpmLChange(char* newValue) {
  display_elements[RPML].value = newValue;
  update_element(RPML);
 
}
DcsBios::StringBuffer<3> ifeiRpmLBuffer(0x749e, onIfeiRpmLChange);

void onIfeiRpmRChange(char* newValue) {
  display_elements[RPMR].value = newValue;
  update_element(RPMR);

}
DcsBios::StringBuffer<3> ifeiRpmRBuffer(0x74a2, onIfeiRpmRChange);

void onIfeiRpmTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0) {
    display_elements[RPMT].value = "RPM";
    update_element(RPMT);
  }
  else if (strcmp(newValue, "0") == 0) {
    display_elements[RPMT].value = "   ";
    update_element(RPMT);
  }
}
DcsBios::StringBuffer<1> ifeiRpmTextureBuffer(0x74bc, onIfeiRpmTextureChange);

//################## TEMP  ##################Y
void onIfeiTempLChange(char* newValue) {
  display_elements[TMPL].value = newValue;
  update_element(TMPL);
   
}
DcsBios::StringBuffer<3> ifeiTempLBuffer(0x74a6, onIfeiTempLChange);

void onIfeiTempRChange(char* newValue) {
  display_elements[TMPR].value = newValue;
  update_element(TMPR);
}
DcsBios::StringBuffer<3> ifeiTempRBuffer(0x74aa, onIfeiTempRChange);

void onIfeiTempTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0) {
    display_elements[TMPT].value = "TEMP";
    update_element(TMPT);
  }
  else if (strcmp(newValue, "0") == 0) {
    display_elements[TMPT].value = "    ";
    update_element(TMPT);
  }
}
DcsBios::StringBuffer<1> ifeiTempTextureBuffer(0x74be, onIfeiTempTextureChange);

//################## FUEL FLOW LEFT ##################Y
void onIfeiFfLChange(char* newValue) {
  display_elements[FFL].value = newValue;
  update_element(FFL);
}
DcsBios::StringBuffer<3> ifeiFfLBuffer(0x7482, onIfeiFfLChange);
//################## FUEL FLOW RIGHT ##################Y
void onIfeiFfRChange(char* newValue) {
  display_elements[FFR].value = newValue;
  update_element(FFR);
}
DcsBios::StringBuffer<3> ifeiFfRBuffer(0x7486, onIfeiFfRChange);
//################## FUEL FLOW TEXTURE ##################Y
void onIfeiFfTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0) {
    display_elements[FFT].value = " FF \nX100";
    update_element(FFT);
  }
  else if (strcmp(newValue, "0") == 0) {
    display_elements[FFT].value = "    \n    ";
    update_element(FFT);
  }
}
DcsBios::StringBuffer<1> ifeiFfTextureBuffer(0x74c0, onIfeiFfTextureChange);

//################## OIL ##################Y

void onIfeiOilPressLChange(char* newValue) {
  display_elements[OILL].value = newValue;
  update_element(OILL);
}
DcsBios::StringBuffer<3> ifeiOilPressLBuffer(0x7496, onIfeiOilPressLChange);

void onIfeiOilPressRChange(char* newValue) {
  display_elements[OILR].value = newValue;
  update_element(OILR);
}
DcsBios::StringBuffer<3> ifeiOilPressRBuffer(0x749a, onIfeiOilPressRChange);

void onIfeiOilTextureChange(char* newValue) {
  
  if (strcmp(newValue, "1") == 0) {
    display_elements[OILT].value ="OIL";
    display_elements[NOZT].value ="NOZ";
   
  }
  else if (strcmp(newValue, "0") == 0) {
    display_elements[OILT].value ="   ";
    display_elements[NOZT].value ="   ";
  }
  update_element(OILT);
  update_element(NOZT);
}
DcsBios::StringBuffer<1> ifeiOilTextureBuffer(0x74c4, onIfeiOilTextureChange);


//################## NOZZL ##################
//LEFT
int NOZL_v = 0;
int NOZL_v_OLD = 0;

void onExtNozzlePosLChange(unsigned int newValue) {
   NOZL_v = map(newValue, 0, 65535, 0, 100);
   if (NOZL_v != NOZL_v_OLD){
    NOZL_v_OLD = NOZL_v;
    switch (NOZL_v) { // NOZ LEFT POSITION IFEI
      case 0 ... 4:    display_elements[NOZL].value = "L0.bmp"; update_nozzel(NOZL); break;
      case 5 ... 14:  display_elements[NOZL].value = "L10.bmp"; update_nozzel(NOZL); break;
      case 15 ... 24:  display_elements[NOZL].value = "L20.bmp"; update_nozzel(NOZL); break;
      case 25 ... 34:  display_elements[NOZL].value = "L30.bmp"; update_nozzel(NOZL); break;
      case 35 ... 44:  display_elements[NOZL].value = "L40.bmp"; update_nozzel(NOZL); break;
      case 45 ... 54:  display_elements[NOZL].value = "L50.bmp"; update_nozzel(NOZL); break;
      case 55 ... 64:  display_elements[NOZL].value = "L60.bmp"; update_nozzel(NOZL); break;
      case 65 ... 74:  display_elements[NOZL].value = "L70.bmp"; update_nozzel(NOZL); break;
      case 75 ... 84:  display_elements[NOZL].value = "L80.bmp"; update_nozzel(NOZL); break;
      case 85 ... 94:  display_elements[NOZL].value = "L90.bmp"; update_nozzel(NOZL); break;
      case 95 ... 100: display_elements[NOZL].value = "L100.bmp"; update_nozzel(NOZL); break;
    }
   }
}



DcsBios::IntegerBuffer extNozzlePosLBuffer(0x757a, 0xffff, 0, onExtNozzlePosLChange);

//RIGHT
int NOZR_v = 0;
int NOZR_v_OLD = 0;
void onExtNozzlePosRChange(unsigned int newValue) {
   NOZR_v = map(newValue, 0, 65535, 0, 100);
  // if (NOZR_v != NOZR_v_OLD){
   // NOZR_v_OLD = NOZR_v;
    Serial.print("NOZ: ");Serial.println(NOZR_v);
    switch (NOZR_v) { // NOZ RIGHT POSITION IFEI
      case 0 ... 4:    display_elements[NOZR].value = "R0.bmp"; update_nozzel(NOZR); break;
      case 5 ... 14:  display_elements[NOZR].value = "R10.bmp"; update_nozzel(NOZR); break;
      case 15 ... 24:  display_elements[NOZR].value = "R20.bmp"; update_nozzel(NOZR); break;
      case 25 ... 34:  display_elements[NOZR].value = "R30.bmp"; update_nozzel(NOZR); break;
      case 35 ... 44:  display_elements[NOZR].value = "R40.bmp"; update_nozzel(NOZR); break;
      case 45 ... 54:  display_elements[NOZR].value = "R50.bmp"; update_nozzel(NOZR); break;
      case 55 ... 64:  display_elements[NOZR].value = "R60.bmp"; update_nozzel(NOZR); break;
      case 65 ... 74:  display_elements[NOZR].value = "R70.bmp"; update_nozzel(NOZR); break;
      case 75 ... 84:  display_elements[NOZR].value = "R80.bmp"; update_nozzel(NOZR); break;
      case 85 ... 94:  display_elements[NOZR].value = "R90.bmp"; update_nozzel(NOZR); break;
      case 95 ... 100: display_elements[NOZR].value = "R100.bmp"; update_nozzel(NOZR); break;
    }
   //}
}

DcsBios::IntegerBuffer extNozzlePosRBuffer(0x7578, 0xffff, 0, onExtNozzlePosRChange);

// NOZZLE POINTER 
/*
void onIfeiRpointerTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0) {
    update_nozzel("LScale.bmp",NOZL_X,NOZL_Y);
    update_nozzel("RScale.bmp",NOZR_X,NOZR_Y);
  }
}
DcsBios::StringBuffer<1> ifeiRpointerTextureBuffer(0x74da, onIfeiRpointerTextureChange);
*/

///////////// IFEI COLOUR TEXT GREN OR WHITE ///////////////////////

void onCockkpitLightModeSwChange(unsigned int newValue) {
 
  ifeiCol = newValue;
  if (ifeiCol != 0) {
    NOZL_PATH = "Green";
    ifei_color = color_NIGHT;
    
    
    /*
    threeD.setTextColor(0x1CDD2AU);
    labels.setTextColor(0x1CDD2AU);
    ffT.setTextColor(0x1CDD2AU);
    Bingo.setTextColor(0x1CDD2AU);
    CLOCK.setTextColor(0x1CDD2AU);
    */
  }
  if (ifeiCol == 0) {
    NOZL_PATH = "White";
    ifei_color = color_day;
     
    /*
    threeD.setTextColor(0xFFFFFFU);
    labels.setTextColor(0xFFFFFFU);
    ffT.setTextColor(0xFFFFFFU);
    Bingo.setTextColor(0xFFFFFFU);
    CLOCK.setTextColor(0xFFFFFFU);
    */
  }

  for ( int i = 0; i < 24; i++ ){
      if ( i == NOZL || i == NOZR){
        update_nozzel(i);
      }else{
        update_element(i);
      }
  }

  update_Clock(CLOCKU);
  update_Clock(CLOCKL);
}
 
DcsBios::IntegerBuffer cockkpitLightModeSwBuffer(0x74c8, 0x0600, 9, onCockkpitLightModeSwChange);

//################## FUEL LOWER ##################

void onIfeiFuelDownChange(char* newValue) {
  display_elements[FUELL].value = newValue;
  update_element(FUELL);
}
DcsBios::StringBuffer<6> ifeiFuelDownBuffer(0x748a, onIfeiFuelDownChange);

void onIfeiTimeSetModeChange(char* newValue) {
  display_elements[FUELL].value = newValue;
  update_element(FUELL);
}
DcsBios::StringBuffer<6> ifeiTimeSetModeBuffer(0x74b6, onIfeiTimeSetModeChange);

//################# FUEL UPPER ##################
void onIfeiFuelUpChange(char* newValue) {
  display_elements[FUELU].value = newValue;
   update_element(FUELU);
}
DcsBios::StringBuffer<6> ifeiFuelUpBuffer(0x7490, onIfeiFuelUpChange);

/*void onIfeiTChange(char* newValue) {
  display_elements[FUELU].value = newValue;
  update_element(FUELU);
}
DcsBios::StringBuffer<6> ifeiTBuffer(0x757c, onIfeiTChange);
*/
//################## BINGO ################## Y
void onIfeiBingoChange(char* newValue) {
  display_elements[BINGO].value = newValue;
  update_element(BINGO);
}
DcsBios::StringBuffer<5> ifeiBingoBuffer(0x7468, onIfeiBingoChange);

//################## TOP CLOCK ##################
//Hours
void onIfeiClockHChange(char* newValue) {
  TC_H = String(newValue);
  update_Clock(CLOCKU);
}
DcsBios::StringBuffer<2> ifeiClockHBuffer(0x746e, onIfeiClockHChange);
//":"" 1 Hours
void onIfeiDd1Change(char* newValue) {
    TC_Dd1 = *newValue;
    update_Clock(CLOCKU);
}
DcsBios::StringBuffer<1> ifeiDd1Buffer(0x747a, onIfeiDd1Change);
// Minutes
void onIfeiClockMChange(char* newValue) {
  TC_M = String(newValue);
  update_Clock(CLOCKU);
}
DcsBios::StringBuffer<2> ifeiClockMBuffer(0x7470, onIfeiClockMChange);
// ":" 2 Minutes
void onIfeiDd2Change(char* newValue) {
    TC_Dd2 = *newValue;
    update_Clock(CLOCKU);
}
DcsBios::StringBuffer<1> ifeiDd2Buffer(0x747c, onIfeiDd2Change);
//Seconds
void onIfeiClockSChange(char* newValue) {
  TC_S = String(newValue);
  update_Clock(CLOCKU);
}
DcsBios::StringBuffer<2> ifeiClockSBuffer(0x7472, onIfeiClockSChange);

///////////// ZULU Texture ///////////////////////
void onIfeiZTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0) {
    display_elements[ZULU].value = "Z";
    update_element(ZULU);
  }
  else if (strcmp(newValue, "0") == 0) {
    display_elements[ZULU].value = " ";
    update_element(ZULU);
  }
}
DcsBios::StringBuffer<1> ifeiZTextureBuffer(0x74dc, onIfeiZTextureChange);

//################# TIMER CLOCK ########################
void onIfeiTimerHChange(char* newValue) {
  LC_H = String(newValue);
  update_Clock(CLOCKL);
}
DcsBios::StringBuffer<2> ifeiTimerHBuffer(0x7474, onIfeiTimerHChange);

// ":"

void onIfeiDd3Change(char* newValue) {
  LC_Dd1 = *newValue;
  update_Clock(CLOCKL);
}
DcsBios::StringBuffer<1> ifeiDd3Buffer(0x747e, onIfeiDd3Change);

void onIfeiTimerMChange(char* newValue) {
  LC_M = String(newValue);
  update_Clock(CLOCKL);
}
DcsBios::StringBuffer<2> ifeiTimerMBuffer(0x7476, onIfeiTimerMChange);

// ":"

void onIfeiDd4Change(char* newValue) {
  LC_Dd2 = *newValue;
  update_Clock(CLOCKL);
}
DcsBios::StringBuffer<1> ifeiDd4Buffer(0x7480, onIfeiDd4Change);

void onIfeiTimerSChange(char* newValue) {
  LC_S = String(newValue);
  update_Clock(CLOCKL);
}
DcsBios::StringBuffer<2> ifeiTimerSBuffer(0x7478, onIfeiTimerSChange);

// Brightness
void onIfeiBreightnessChange(unsigned int newValue) {
    tft.setBrightness(map(newValue, 0, 65535, 0, 255));
}
DcsBios::IntegerBuffer ifeiBrightness(0x74de, 0xffff, 0, onIfeiBreightnessChange);




void setup(void) {
  DcsBios::setup(ssid,passwd);
  //DcsBios::setup();
  tft.begin();
  create_display_elements();


  Serial.begin(115200);

  //SDCARD_SPI.begin(SCK, MISO, MOSI, CS);
  //SD.begin(CS,SDCARD_SPI,40000000);
  
if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }


  tft.setColorDepth(24);
  //tft.fillScreen(0x000000U);
  tft.fillScreen(tft.color888(141,76,71));
  
  


 for (int i=0; i < 19; i++){
    if ( i == 12 || i == 13 ){
      update_nozzel(i);
    }else{
      update_element(i);
    }
 }
 
  update_Clock(CLOCKU);
  update_Clock(CLOCKL);
  listDir(SPIFFS, "/", 0);
}





void loop() {
  DcsBios::loop();
}
