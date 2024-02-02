#include <Arduino.h>



/*
####### Stand alone DCS-BIOS IFEI simulation using a ZX7D00CE01S esp32-s3 7" touchscreen 
Notes:
While using active wifi, I had screen glitches with the default TFT clk frequency provided in the PANELAN library
Change frequency in PanelLan_esp32_arduino/src/board/sc05/sc05.cpp to
#define PanelLan_RGB_CLK_FREQ           (16000000)
*/


#include <FS.h>
#include <SPI.h>
//#include "SPIFFS.h"
#include <LittleFS.h>

#define SPIFFS LittleFS



#include <PanelLan.h>



#define DCSBIOS_LAN    // LAN Access
#define DCSBIOS_ESP32  // ESP32 microcontroller

#define SERIAL_LOG 1
#include  <DcsBios.h>


#define SCK 40 
#define MISO 42
#define MOSI 2
#define CS 41
 



// MISC Variables




char ssid[] = "Freudenhaus2";
char passwd[] = "aquarius";


//Clock
String TC_H = "00";
String TC_Dd1 = ":";
String TC_M = "00";
String TC_Dd2 = ":";
String TC_S = "00";

String LC_H = "0";
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

LGFX_Sprite NOZL_IMAGE[27];
  
LGFX_Sprite NOZR_IMAGE[27];
/*{
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
};*/
LGFX_Sprite SMALL_BLOCK(&tft);
LGFX_Sprite NOZT_SPRITE(&tft);
LGFX_Sprite POINTER(&tft);





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

display_element display_elements[]= {
  //{width, hight, posx, posy, textalign, sprite, value}
  { 95, 38, 58, 20,2,&THREED,"012"}, //RPML
  { 95, 38,260, 20,0,&THREED,"345"}, //RPMR
  { 58, 18,180, 31,1,&LABELS,"RPM"}, //RPMT
  { 95, 38, 58, 85,2,&THREED,"678"}, //TMPL
  { 95, 38,260, 85,0,&THREED,"987"}, //TMPR
  { 58, 18,180, 96,1,&LABELS,"TEMP"}, //TMPT
  { 95, 38, 58,160,2,&THREED,"0"}, //FFL
  { 95, 38,260,160,2,&THREED,"0"}, //FFR
  { 58, 18,180,171,1,&LABELS,"FF"}, //FFTU
  { 65, 18,180,188,1,&LABELS,"X100"}, //FFTL
  { 95, 38, 58,400,2,&THREED,"0"}, //OILL
  { 95, 38,260,400,0,&THREED,"0"}, //OILR
  { 58, 18,180,418,1,&LABELS,"OIL"}, //OILT
  {150,154, 58,230,0,&NOZL_IMAGE[0],"0"}, //NOZL
  {150,154,211,230,0,&NOZR_IMAGE[0],"0"}, //NOZR
  { 58, 18,180,300,1,&LABELS,"NOZ"}, //NOZT
  {176, 38,560, 30,2,&Fuel,"0"}, //FUELU
  {176, 38,560, 85,2,&Fuel,"0"}, //FUELL
  {176, 38,560,215,2,&Fuel,"0"}, //BINGO
  { 58, 18,625,185,1,&LABELS,"BINGO"}, //BINGOT
  {176, 35,560,310,2,&CLOCK,"0"}, //CLOCKU
  {176, 35,560,375,2,&CLOCK,"0"}, //CLOCKL
  { 18, 18,736,327,1,&SMALL_BLOCK,"Z"}, //ZULU
  { 18, 18,736, 50,1,&SMALL_BLOCK,"L"}, //L
  { 18, 18,736,105,1,&SMALL_BLOCK,"R"}, //R
};


void create_image_sprite(){
  int j = 0;
  //Left Nozzel White 
  for (int i = 0; i <= 120; i += 10){
    String filename = "/White/L" + String(i) + ".bmp";
    NOZL_IMAGE[j].setPsram(true);
    NOZL_IMAGE[j].createSprite(150,154);
    //NOZL_IMAGE[j].createFromBmp(SPIFFS,filename.c_str());
    NOZL_IMAGE[j].setColorDepth(24);
    NOZL_IMAGE[j].setPaletteColor(1, 0x000000U);
    NOZL_IMAGE[j].fillScreen(0xFF0000U);
    NOZL_IMAGE[j].drawBmpFile(SPIFFS,filename.c_str());
    
    
    
    j++;
  }
  //Left Nozzel Green
  for (int i = 0; i <= 120; i += 10){
    String filename = "/Green/L" + String(i) + ".bmp";
    NOZL_IMAGE[j].setPsram(true);
    NOZL_IMAGE[j].setColorDepth(24);
    NOZL_IMAGE[j].setPaletteColor(1, 0x000000U);
    NOZL_IMAGE[j].createFromBmp(SPIFFS,filename.c_str());
    j++;
  }
  NOZL_IMAGE[j].setPsram(true);
  NOZL_IMAGE[j].setColorDepth(24);
  NOZL_IMAGE[j].setPaletteColor(1, 0x000000U);
  NOZL_IMAGE[j].createSprite(display_elements[NOZL].sprite_width, display_elements[NOZL].sprite_hight);
  NOZL_IMAGE[j].fillScreen(0x000000U);
  
  j = 0;
  //Right Nozzel White 
  for (int k = 0; k <= 120; k += 10){
    String filename = "/White/R" + String(k) + ".bmp";
    NOZR_IMAGE[j].setPsram(true);
    NOZR_IMAGE[j].setColorDepth(24);
    NOZR_IMAGE[j].setPaletteColor(1, 0x000000U);
    NOZR_IMAGE[j].createFromBmp(SPIFFS,filename.c_str());
    j++;
  }
  
  //Right Nozzel Green 
  for (int k = 0; k <= 120; k += 10){
    String filename = "/Green/R" + String(k) + ".bmp";
    NOZR_IMAGE[j].setPsram(true);
    NOZR_IMAGE[j].setColorDepth(24);
    NOZR_IMAGE[j].setPaletteColor(1, 0x000000U);
    NOZR_IMAGE[j].createFromBmp(SPIFFS,filename.c_str());
    j++;
  }
  NOZR_IMAGE[j].setPsram(true);
  NOZR_IMAGE[j].setColorDepth(24);
  NOZR_IMAGE[j].setPaletteColor(1, 0x000000u);
  NOZR_IMAGE[j].createSprite(display_elements[NOZR].sprite_width, display_elements[NOZR].sprite_hight);
  NOZR_IMAGE[j].fillScreen(0x000000U);

  
}

void create_display_elements(){
//RPM
 create_image_sprite();
// Create Sprites
  THREED.createSprite(display_elements[RPML].sprite_width, display_elements[RPML].sprite_hight);
  THREED.loadFont(SPIFFS,"/Fonts/IFEI-Data-36.vlw");
  THREED.setFont(THREED.getFont());
  //THREED.setFont(&segmentsa42);
  THREED.setColorDepth(24);
  THREED.setTextWrap(false);
  THREED.setTextColor(ifei_color);
    
  LABELS.createSprite(display_elements[RPMT].sprite_width, display_elements[RPMT].sprite_hight);
  LABELS.loadFont(SPIFFS,"/Fonts/IFEI-Labels-16.vlw");
  LABELS.setFont(LABELS.getFont());
  LABELS.setColorDepth(24);
  LABELS.setTextColor(ifei_color);
  
  CLOCK.createSprite(display_elements[CLOCKU].sprite_width, display_elements[CLOCKU].sprite_hight);
  CLOCK.loadFont(SPIFFS,"/Fonts/IFEI-Data-32.vlw");
  CLOCK.setFont(CLOCK.getFont());
  CLOCK.setColorDepth(24);
  CLOCK.setTextWrap(false);
  CLOCK.setTextColor(ifei_color);

  
  SMALL_BLOCK.createSprite(display_elements[ZULU].sprite_width, display_elements[ZULU].sprite_hight);
  SMALL_BLOCK.loadFont(SPIFFS,"/Fonts/IFEI-Labels-16.vlw");
  SMALL_BLOCK.setFont(LABELS.getFont());
  //SMALL_BLOCK.setTextSize(0,5);
  SMALL_BLOCK.print(display_elements[NOZT].value);

  NOZT_SPRITE.createSprite(display_elements[NOZT].sprite_width, display_elements[NOZT].sprite_hight);
  NOZT_SPRITE.loadFont(SPIFFS,"/Fonts/IFEI-Labels-16.vlw");
  NOZT_SPRITE.setFont(LABELS.getFont());
  NOZT_SPRITE.print(display_elements[NOZT].value);

  Fuel.createSprite(display_elements[FUELU].sprite_width, display_elements[FUELU].sprite_hight);
  Fuel.loadFont(SPIFFS,"/Fonts/IFEI-Data-36.vlw");
  Fuel.setFont(Fuel.getFont());
  Fuel.setColorDepth(24);
  Fuel.setTextWrap(false);
  Fuel.setTextColor(ifei_color);
}



  
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

int set_textalignment(int element,int alignment, bool is_label){
  //alignment 0=left; 1=middle; 2=right
  int fontwidth=0;
  for (const char* ptr = display_elements[element].value; *ptr != '\0'; ++ptr){
            fontwidth += display_elements[element].sprite->fontWidth();
  }
  if (alignment == 2){
    return display_elements[element].sprite_width - display_elements[element].sprite->textWidth(display_elements[element].value);
  }else if (alignment == 1){
    return (display_elements[element].sprite_width - display_elements[element].sprite->textWidth(display_elements[element].value))/2;
  }else{
    return 0;
  }
}


void update_element(int element){
  int x1 = set_textalignment(element, display_elements[element].textalign, false);
  
  display_elements[element].sprite->clear();
  display_elements[element].sprite->setCursor(x1,0);
  display_elements[element].sprite->setTextColor(ifei_color);
  display_elements[element].sprite->print(display_elements[element].value);
  display_elements[element].sprite->pushSprite(display_elements[element].pos_x,display_elements[element].pos_y);
  
}


void update_Clock(int element){
  String H;
  String DP1;
  String M;
  String DP2;
  String S;
  int offset = 0;
  if (element == CLOCKU){
    H = TC_H;
    DP1 = TC_Dd1;
    M = TC_M;
    DP2 = TC_Dd2;
    S = TC_S;
  }else{
    if (LC_H[1] == 32){
      H = LC_H[0];
      offset = 28;

    }else {
      H = LC_H;
    }
    DP1 = LC_Dd1;
    M = LC_M;
    DP2 = LC_Dd2;
    S = LC_S;
  }

  display_elements[element].sprite->clear();
  display_elements[element].sprite->setTextColor(ifei_color);
  
  
  if ( H[0] == 32 ){
     display_elements[element].sprite->setCursor(57,1);
  }else {
    display_elements[element].sprite->setCursor(1 + offset,1);
    display_elements[element].sprite->print(H);  
  }
  if ( DP1[0] == 32 ){
    display_elements[element].sprite->setCursor(63,1);
  }else {
    display_elements[element].sprite->print(DP1);
  }
  if ( M[0] == 32 ){
    display_elements[element].sprite->setCursor(119,1);
  }else {
    display_elements[element].sprite->print(M);
  }
  if ( DP2[0] == 32 ){
    display_elements[element].sprite->setCursor(125,1);
  }else {
    display_elements[element].sprite->print(DP2);
  }
  if ( S[0] == 32 ){
    display_elements[element].sprite->setCursor(181,1);
  }else {
    display_elements[element].sprite->print(S);
  }

  display_elements[element].sprite->pushSprite(display_elements[element].pos_x,display_elements[element].pos_y);
}



  

//################## RPM  ##################Y
void onIfeiRpmLChange(char* newValue) {
  display_elements[RPML].value = remove_trailing_spaces(newValue);
  update_element(RPML);
 
}
DcsBios::StringBuffer<3> ifeiRpmLBuffer(0x749e, onIfeiRpmLChange);

void onIfeiRpmRChange(char* newValue) {
  display_elements[RPMR].value = remove_trailing_spaces(newValue);
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
  display_elements[TMPL].value = remove_trailing_spaces(newValue);
  update_element(TMPL);
   
}
DcsBios::StringBuffer<3> ifeiTempLBuffer(0x74a6, onIfeiTempLChange);

void onIfeiTempRChange(char* newValue) {
  display_elements[TMPR].value = remove_trailing_spaces(newValue);
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
bool SPBIT;
// ************** SP (CODES) ***************************
void onIfeiSpChange(char* newValue) {

  //if (display_elements[TMPL].value == "   ") { 
  display_elements[TMPL].value = newValue;
  update_element(TMPL);
  //}
}

DcsBios::StringBuffer<3> ifeiSpBuffer(0x74b2, onIfeiSpChange);

void onIfeiCodesChange(char* newValue) {
 // if (display_elements[TMPL].value == "   ") { 
    display_elements[TMPR].value = newValue;
    update_element(TMPR);
  //}
}
DcsBios::StringBuffer<3> ifeiCodesBuffer(0x74ae, onIfeiCodesChange);


//################## FUEL FLOW LEFT ##################Y
void onIfeiFfLChange(char* newValue) {
  display_elements[FFL].value = remove_trailing_spaces(newValue);
  update_element(FFL);
}
DcsBios::StringBuffer<3> ifeiFfLBuffer(0x7482, onIfeiFfLChange);
//################## FUEL FLOW RIGHT ##################Y
void onIfeiFfRChange(char* newValue) {
  display_elements[FFR].value = remove_trailing_spaces(newValue);
  update_element(FFR);
}
DcsBios::StringBuffer<3> ifeiFfRBuffer(0x7486, onIfeiFfRChange);
//################## FUEL FLOW TEXTURE ##################Y
void onIfeiFfTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0) {
    display_elements[FFTU].value = "FF";
    display_elements[FFTL].value = "X100";
    update_element(FFTU);
    update_element(FFTL);
  }
  else if (strcmp(newValue, "0") == 0) {
    display_elements[FFTU].value = "";
    display_elements[FFTL].value = "";
    update_element(FFTU);
    update_element(FFTL);
  }
}
DcsBios::StringBuffer<1> ifeiFfTextureBuffer(0x74c0, onIfeiFfTextureChange);

//################## OIL ##################Y

void onIfeiOilPressLChange(char* newValue) {
  display_elements[OILL].value = remove_trailing_spaces(newValue);
  update_element(OILL);
}
DcsBios::StringBuffer<3> ifeiOilPressLBuffer(0x7496, onIfeiOilPressLChange);

void onIfeiOilPressRChange(char* newValue) {
  display_elements[OILR].value = remove_trailing_spaces(newValue);
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


//################## NOZZEL ##################
//LEFT
int NOZL_v = 0;
int NOZL_v_OLD = 0;


int colormode = 0;
void onExtNozzlePosLChange(unsigned int newValue) {
   NOZL_v = map(newValue, 0, 65535, 0, 100);
   //if (NOZL_v != NOZL_v_OLD){
   // NOZL_v_OLD = NOZL_v;

   //Switch between green and white images
    
    switch (NOZL_v) { // NOZ LEFT POSITION IFEI
      
      case 0 ... 4: 
        //NOZL_IMAGE[0 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[0 + colormode];
        display_elements[NOZL].value= "0";
        break;
      case 5 ... 14:  
        //NOZL_IMAGE[1 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[1 + colormode];
        display_elements[NOZL].value= "1";
        break;
      case 15 ... 24:  
        //NOZL_IMAGE[2 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[2 + colormode];
        display_elements[NOZL].value= "2";
        break;
      case 25 ... 34:  
        //NOZL_IMAGE[3 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[3 + colormode];
        display_elements[NOZL].value= "3";
        break;
      case 35 ... 44:  
        //NOZL_IMAGE[4 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[4 + colormode];
        display_elements[NOZL].value= "4";
        break;
      case 45 ... 54:  
        //NOZL_IMAGE[5 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[5 + colormode];
        display_elements[NOZL].value= "5";
        break;
      case 55 ... 64:  
        //NOZL_IMAGE[6 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[6 + colormode];
        display_elements[NOZL].value= "6";
        break;
      case 65 ... 74:  
        //NOZL_IMAGE[7 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[7 + colormode];
        display_elements[NOZL].value= "7";
        break;
      case 75 ... 84:  
        //NOZL_IMAGE[8 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[8 + colormode];
        display_elements[NOZL].value= "8";
        break;
      case 85 ... 94:  
        //NOZL_IMAGE[9 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[9 + colormode];
        display_elements[NOZL].value= "9";
        break;
      case 95 ... 100: 
        //NOZL_IMAGE[10 + colormode].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
        display_elements[NOZL].sprite = &NOZL_IMAGE[10 + colormode];
        display_elements[NOZL].value= "10";
        break;
    }
    display_elements[NOZL].sprite->pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
    //display_elements[NOZT].sprite->pushSprite(display_elements[NOZT].pos_x,display_elements[NOZT].pos_y);

}



DcsBios::IntegerBuffer extNozzlePosLBuffer(0x757a, 0xffff, 0, onExtNozzlePosLChange);
/*
// NOZZLE POINTER Left
void onIfeiLpointerTextureChange(char* newValue) {
    if (strcmp(newValue, "0") == 0) {
    NOZL_IMAGE[25].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
  }else{
    display_elements[NOZL].sprite->pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
  }
}
DcsBios::StringBuffer<1> ifeiLpointerTextureBuffer(0x74d8, onIfeiLpointerTextureChange);


//Left Scale
void onIfeiLscaleTextureChange(char* newValue) {
    if (strcmp(newValue, "0") == 0) {
      NOZL_IMAGE[25].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
    }else {
      int image_pos = 12;
      if ( ifei_color == color_NIGHT){
        image_pos = 24;
      }
      NOZL_IMAGE[image_pos].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U); 
      }    
}
DcsBios::StringBuffer<1> ifeiLscaleTextureBuffer(0x74c8, onIfeiLscaleTextureChange);

void onIfeiL100TextureChange(char* newValue) {
    if (strcmp(newValue, "0") == 0) {
      NOZL_IMAGE[25].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
    }else{
      int image_pos = 11;
      if ( ifei_color == color_NIGHT){
        image_pos = 23;
      }
      NOZL_IMAGE[image_pos].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
      }
}
DcsBios::StringBuffer<1> ifeiL100TextureBuffer(0x74d4, onIfeiL100TextureChange);
*/
//RIGHT
int NOZR_v = 0;
int NOZR_v_OLD = 0;



void onExtNozzlePosRChange(unsigned int newValue) {
   NOZR_v = map(newValue, 0, 65535, 0, 100);
  // if (NOZR_v != NOZR_v_OLD){
   // NOZR_v_OLD = NOZR_v;

   //Switch between green and white images
    
    
    switch (NOZR_v) { // NOZ RIGHT POSITION IFEI
      case 0 ... 4:     
        //NOZR_IMAGE[0 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[0 + colormode];
        display_elements[NOZR].value= "0";
        break;
      case 5 ... 14:  
        //NOZR_IMAGE[1 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[1 + colormode];
        display_elements[NOZR].value= "1";
        break;
      case 15 ... 24:  
        //NOZR_IMAGE[2 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[2 + colormode];
        display_elements[NOZR].value= "2";
        break;
      case 25 ... 34:  
        //NOZR_IMAGE[3 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[3 + colormode];
        display_elements[NOZR].value= "3";
        break;
      case 35 ... 44:  
        //NOZR_IMAGE[4 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[4 + colormode];
        display_elements[NOZR].value= "4";
        break;
      case 45 ... 54:  
        //NOZR_IMAGE[5 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[5 + colormode];
        display_elements[NOZR].value= "5";
        break;
      case 55 ... 64:  
        //NOZR_IMAGE[6 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[6 + colormode];
        display_elements[NOZR].value= "6";
        break;
      case 65 ... 74:  
        //NOZR_IMAGE[7 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[7 + colormode];
        display_elements[NOZR].value= "7";
        break;
      case 75 ... 84:  
        //NOZR_IMAGE[8 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[8 + colormode];
        display_elements[NOZR].value= "8";
        break;
      case 85 ... 94:  
        //NOZR_IMAGE[9 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[9 + colormode];
        display_elements[NOZR].value= "9";
        break;
      case 95 ... 100: 
        //NOZR_IMAGE[10 + colormode].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
        display_elements[NOZR].sprite = &NOZR_IMAGE[10 + colormode];
        display_elements[NOZR].value= "10";
        break;
    }
    display_elements[NOZR].sprite->pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
    //display_elements[NOZT].sprite->pushSprite(display_elements[NOZT].pos_x,display_elements[NOZT].pos_y);
}

DcsBios::IntegerBuffer extNozzlePosRBuffer(0x7578, 0xffff, 0, onExtNozzlePosRChange);

int old_NOZZL_v;
int old_NOZZR_v;
// NOZZLE POINTER Right
/*
void onIfeiRpointerTextureChange(char* newValue) {
  if (strcmp(newValue, "0") == 0) {
    NOZR_IMAGE[25].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
  }else{
    display_elements[NOZR].sprite->pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
  }
}
DcsBios::StringBuffer<1> ifeiRpointerTextureBuffer(0x74da, onIfeiRpointerTextureChange);

//Right Scale
void onIfeiRscaleTextureChange(char* newValue) {
    if (strcmp(newValue, "0") == 0) {
      NOZR_IMAGE[25].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
    }else{
      int image_pos = 12;
      if ( ifei_color == color_NIGHT){
        image_pos = 24;
      }
      NOZR_IMAGE[image_pos].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U); 
    }
}
DcsBios::StringBuffer<1> ifeiRscaleTextureBuffer(0x74ca, onIfeiRscaleTextureChange);

void onIfeiR100TextureChange(char* newValue) {
    if (strcmp(newValue, "0") == 0) {
      NOZR_IMAGE[25].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
    }else{
      int image_pos = 11;
      if ( ifei_color == color_NIGHT){
        image_pos = 23;
      }
      NOZR_IMAGE[image_pos].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
    }
}
DcsBios::StringBuffer<1> ifeiR100TextureBuffer(0x74d6, onIfeiR100TextureChange);
*/
///////////// IFEI COLOUR TEXT GREN OR WHITE ///////////////////////

int old_ifeiCol = 0;
void onCockkpitLightModeSwChange(unsigned int newValue) {
  ifeiCol = newValue;
  if (old_ifeiCol != ifeiCol){
    Serial.print("Lightmode Change: ");Serial.println(newValue);
    old_ifeiCol = ifeiCol;
    if (ifeiCol != 0) {
      colormode = 12;
      NOZL_PATH = "Green";
      ifei_color = color_NIGHT;
    }
    if (ifeiCol == 0) {
      colormode = 0;
      NOZL_PATH = "White";
      ifei_color = color_day;
    }

    for ( int i = 0; i < 24; i++ ){
        if ( i == NOZL ){
          display_elements[NOZL].sprite = &NOZL_IMAGE[atoi(display_elements[NOZL].value) + colormode];
          display_elements[NOZL].sprite->pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
        }else if (i == NOZR ){
          display_elements[NOZR].sprite = &NOZR_IMAGE[atoi(display_elements[NOZR].value) + colormode];
          display_elements[NOZR].sprite->pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
        }else{
          update_element(i);
        }
    }
    update_Clock(CLOCKU);
    update_Clock(CLOCKL);
    
  }
}
 
DcsBios::IntegerBuffer cockkpitLightModeSwBuffer(0x74c8, 0x0600, 9, onCockkpitLightModeSwChange);

//################## FUEL LOWER ##################

void onIfeiFuelDownChange(char* newValue) {
  display_elements[FUELL].value = remove_trailing_spaces(newValue);
  update_element(FUELL);
}
DcsBios::StringBuffer<6> ifeiFuelDownBuffer(0x748a, onIfeiFuelDownChange);

void onIfeiTimeSetModeChange(char* newValue) {
  display_elements[FUELL].value = remove_trailing_spaces(newValue);
  update_element(FUELL);
}
DcsBios::StringBuffer<6> ifeiTimeSetModeBuffer(0x74b6, onIfeiTimeSetModeChange);

//################# R TEXTURE LOWER ##################
void onIfeiRTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0){
    display_elements[R].value = "R";
  }else{
    display_elements[R].value = " ";
  }
  update_element(R);
}
DcsBios::StringBuffer<1> ifeiRTextureBuffer(0x7584, onIfeiRTextureChange);

//################# FUEL UPPER ##################
void onIfeiFuelUpChange(char* newValue) {
  display_elements[FUELU].value = remove_trailing_spaces(newValue);
  update_element(FUELU);
}
DcsBios::StringBuffer<6> ifeiFuelUpBuffer(0x7490, onIfeiFuelUpChange);

void onIfeiTChange(char* newValue) {
  int spaces = 0;
  for (int i = 0; i < 6; i++){
    if (isSpace(newValue[i])){
      spaces++;
    }
  }
  if ( spaces != 6){
    display_elements[FUELU].value = remove_trailing_spaces(newValue);
    update_element(FUELU);
  }
}
DcsBios::StringBuffer<6> ifeiTBuffer(0x757c, onIfeiTChange);

//################# L TEXTURE UPPER ##################

void onIfeiLTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0){
    display_elements[L].value = "L";
  }else{
    display_elements[L].value = " ";
  }
  update_element(L);
}
DcsBios::StringBuffer<1> ifeiLTextureBuffer(0x7582, onIfeiLTextureChange);

//################## BINGO ################## 
void onIfeiBingoChange(char* newValue) {
  
    display_elements[BINGO].value = remove_trailing_spaces(newValue);
    update_element(BINGO);

}
DcsBios::StringBuffer<5> ifeiBingoBuffer(0x7468, onIfeiBingoChange);

//################## BINGO Texture ################## 

void onIfeiBingoTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0){
    display_elements[BINGOT].value = "BINGO";
  }else{
    display_elements[BINGOT].value = "    ";
  }
  update_element(BINGOT);
}
DcsBios::StringBuffer<1> ifeiBingoTextureBuffer(0x74c6, onIfeiBingoTextureChange);

//################## TOP CLOCK ##################
//Hours
void onIfeiClockHChange(char* newValue) {
  TC_H = String(newValue);
  update_Clock(CLOCKU);
}
DcsBios::StringBuffer<2> ifeiClockHBuffer(0x746e, onIfeiClockHChange);
//":"" 1 Hours
void onIfeiDd1Change(char* newValue) {
    TC_Dd1 = newValue;
    update_Clock(CLOCKU);
}
DcsBios::StringBuffer<1> ifeiDd1Buffer(0x747a, onIfeiDd1Change);
// Minutes
void onIfeiClockMChange(char* newValue) {
  
  TC_M = newValue;
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
  
  TC_S = newValue;
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
  LC_Dd1 = String(newValue);
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
  LC_Dd2 = String(newValue);
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


  Serial.begin(115200);

  
if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  create_display_elements();

  tft.setColorDepth(24);
  //tft.fillScreen(0x000000U);
  tft.fillScreen(tft.color888(141,76,71));
  
  


 for (int i=0; i < 20; i++){
    if ( i == NOZL || i == NOZR ){
      //update_nozzel(i);
    }else{
      update_element(i);
    }
 }
 
  update_Clock(CLOCKU);
  update_Clock(CLOCKL);

  NOZL_IMAGE[11].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
  NOZL_IMAGE[12].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
  NOZR_IMAGE[11].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
  NOZR_IMAGE[12].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
  //POINTER.setPivot(0,0);
}



  bool forward = true;
  int i = 0;
unsigned long start = 0;
unsigned long nozzle_update = 0;
int a = 0;
void loop() {
  
  DcsBios::loop();
  
  /*
  if (millis() - nozzle_update > 1000){
    start=millis();

     NOZL_IMAGE[i].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
    
      //NOZL_IMAGE[i].pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
      //NOZT_SPRITE.pushSprite(display_elements[NOZT].pos_x,display_elements[NOZT].pos_y);
      //NOZR_IMAGE[23].pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0xFF0000U);
      //NOZR_IMAGE[i].pushImage(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,150,154, 0x000000U);
      NOZR_IMAGE[i].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
      //update_element(NOZT);
      //NOZT_SPRITE.pushSprite(display_elements[NOZT].pos_x,display_elements[NOZT].pos_y);
      
      nozzle_update = millis();
      if ( forward ){
        if ( i == 10){
          forward = false;
          i--;
          a-=8;
        }else {
          i++;
          a+=8;
        }
      }else{
        if (i == 0){
          forward = true;
          i++;
          a+=8;

        }else{
          i--;
          a-=8;
        }
      }
        Serial.print("Draw Time: ");Serial.println(millis()-start);

  }
  */
}
