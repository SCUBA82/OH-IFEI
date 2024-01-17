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


int RPML_v;
int RPMR_v;

int TMPL_v;
int TMPR_v;

int FFXL_v;
int FFXR_v;

int OILL_v;
int OILR_v;

const char* oilT_value = "OIL";
const char* nozT_value = "NOZ";

int BINGO_v;

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
static const lgfx::U8g2font segmentsa42( segments42 );


struct display_element{
  int sprite_width;
  int sprite_hight;
  int pos_x;
  int pos_y;
  int textalign;
  LGFX_Sprite* sprite;
};

// Drawing positions in pixel
// RPM

enum {
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
  {110, 56, 58, 20,2,&THREED}, //RPML
  {110, 56,250, 20,0,&THREED}, //RPMR
  { 45, 70,190, 20,1,&LABELS}, //RPMT
  {110, 56, 58, 85,2,&THREED}, //TMPL
  {110, 56,250, 85,0,&THREED}, //TMPR
  { 45, 70,190, 85,1,&LABELS}, //TMPT
  {110, 56, 85,160,2,&THREED}, //FFL
  {110, 56,250,160,0,&THREED}, //FFR
  { 45, 70,190,160,1,&LABELS}, //FFT
  {110, 56, 85,400,2,&THREED}, //OILL
  {110, 56,250,400,0,&THREED}, //OILR
  { 45, 70,190,400,1,&LABELS}, //OILT
  {150,153, 85,230,0,&NOZL_IMAGE}, //NOZL
  {150,153,211,230,0,&NOZL_IMAGE}, //NOZR
  { 45, 70,190,300,1,&LABELS}, //NOZT
  {100,100,540, 30,2,&CLOCK}, //FUELU
  {100,100,540, 85,2,&CLOCK}, //FUELL
  {230, 56,570,215,2,&CLOCK}, //BINGO
  {230, 56,570,195,1,&LABELS}, //BINGOT
  {230, 56,520,310,2,&CLOCK}, //CLOCKU
  {230, 56,520,375,2,&CLOCK}, //CLOCKL
  { 20, 20,700,360,1,&SMALL_BLOCK}, //ZULU
  { 20, 20,700, 60,1,&SMALL_BLOCK}, //L
  { 20, 20,700,110,1,&SMALL_BLOCK}, //R
};

void create_display_elements(){
//RPM
 
// Create Sprites
  THREED.createSprite(display_elements[RPML].sprite_width, display_elements[RPML].sprite_hight);
  THREED.setFont(&segmentsa42);
  THREED.setColorDepth(24);
  THREED.setTextColor(ifei_color);
  
  LABELS.createSprite(display_elements[RPMT].sprite_width, display_elements[RPMT].sprite_hight);
  //LABELS.setFont(&segments26);
  //LABELS.setTextSize(0.5);
  LABELS.setColorDepth(24);
  LABELS.setTextColor(ifei_color);

  CLOCK.createSprite(230, 56);
  CLOCK.setFont(&segmentsa42);
  CLOCK.setColorDepth(24);
  CLOCK.setTextColor(ifei_color);

  NOZL_IMAGE.createSprite(display_elements[NOZL].sprite_width, display_elements[NOZL].sprite_hight);
  NOZL_IMAGE.fillScreen(ifei_color);
  NOZL_IMAGE.setColorDepth(24);
  
  SMALL_BLOCK.createSprite(20, 20);
}



int set_textalignment(LGFX_Sprite sprite,const char* value, int alignment, bool is_label){
  //alignment 0=left; 1=middle; 2=right
  int fontwidth=0;
  int sprite_width=0;
  for (int i=0; i < strlen(value);i++){
    if (is_label){
      fontwidth += (segments26.max_char_width() + 2)/2;
      sprite_width = 45;
    }else{
      fontwidth += segmentsa42.max_char_width() + 2;
      sprite_width = 110; //&sprite.getWidth();
    }  
  }
   
  if (alignment == 0){
    return sprite_width - fontwidth;
  }else if (alignment == 1){
    return (sprite_width - fontwidth)/2;
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

void update_element(const char* value,int element){
  int x1 = set_textalignment(display_elements[element].sprite, value, display_elements[element].textalign, false);
  
  display_elements[element].sprite->clear();
  display_elements[element].sprite->setCursor(x1,2);
  //display_elements[element].sprite.setTextColor(ifei_color);
  display_elements[element].sprite->print(value);
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
 int x1 = set_textalignment(display_elements[element].sprite, TIME.c_str(),2, false);

  display_elements[element].sprite->clear();
  display_elements[element].sprite->setCursor(1,1);
  display_elements[element].sprite->print(TIME);
  display_elements[element].sprite->pushSprite(display_elements[element].pos_x,display_elements[element].pos_y);
}

void update_nozzel(String FILE,int NOZ_X,int NOZ_Y){
  String filename = "/" + NOZL_PATH + "/" + FILE;

  NOZL_IMAGE.drawBmp(SD, filename.c_str(), 0, 0);
  NOZL_IMAGE.pushSprite(NOZ_X,NOZ_Y);
  update_element(nozT_value,NOZT);
}

void update_SMALLBLOCK(const char* value,int x, int y){
  SMALL_BLOCK.clear();
  SMALL_BLOCK.setCursor(1,1);
  SMALL_BLOCK.print(value);
  SMALL_BLOCK.pushSprite(x,y);
}

  

//################## RPM  ##################Y
void onIfeiRpmLChange(char* newValue) {
  RPML_v = atol(newValue);
  update_element(newValue,RPML);
 
}
DcsBios::StringBuffer<3> ifeiRpmLBuffer(0x749e, onIfeiRpmLChange);

void onIfeiRpmRChange(char* newValue) {
  RPMR_v = atol(newValue);  
  update_element(newValue,RPMR);

}
DcsBios::StringBuffer<3> ifeiRpmRBuffer(0x74a2, onIfeiRpmRChange);

void onIfeiRpmTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0) {
    update_element("RPM",RPMT);
  }
  else if (strcmp(newValue, "0") == 0) {
    update_element("   ",RPMT);
  }
}
DcsBios::StringBuffer<1> ifeiRpmTextureBuffer(0x74bc, onIfeiRpmTextureChange);

//################## TEMP  ##################Y
void onIfeiTempLChange(char* newValue) {
  TMPL_v = atol(newValue);
  update_element(newValue,TMPL);
   
}
DcsBios::StringBuffer<3> ifeiTempLBuffer(0x74a6, onIfeiTempLChange);

void onIfeiTempRChange(char* newValue) {
  TMPR_v = atol(newValue);
  update_element(newValue,TMPR);
}
DcsBios::StringBuffer<3> ifeiTempRBuffer(0x74aa, onIfeiTempRChange);

void onIfeiTempTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0) {
    update_element("TEMP",TMPT);
  }
  else if (strcmp(newValue, "0") == 0) {
    update_element("    ",TMPT);
  }
}
DcsBios::StringBuffer<1> ifeiTempTextureBuffer(0x74be, onIfeiTempTextureChange);

//################## FUEL FLOW LEFT ##################Y
void onIfeiFfLChange(char* newValue) {
  FFXL_v = atol(newValue);
  update_element(newValue,FFL);
}
DcsBios::StringBuffer<3> ifeiFfLBuffer(0x7482, onIfeiFfLChange);
//################## FUEL FLOW RIGHT ##################Y
void onIfeiFfRChange(char* newValue) {
  FFXR_v = atol(newValue);
  update_element(newValue,FFR);
}
DcsBios::StringBuffer<3> ifeiFfRBuffer(0x7486, onIfeiFfRChange);
//################## FUEL FLOW TEXTURE ##################Y
void onIfeiFfTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0) {
      update_element(" FF \nX100",FFT);
  }
  else if (strcmp(newValue, "0") == 0) {
        update_element("  \n  ",FFT);
  }
}
DcsBios::StringBuffer<1> ifeiFfTextureBuffer(0x74c0, onIfeiFfTextureChange);

//################## OIL ##################Y

void onIfeiOilPressLChange(char* newValue) {
  OILL_v = atol(newValue);
  update_element(newValue,OILL);
}
DcsBios::StringBuffer<3> ifeiOilPressLBuffer(0x7496, onIfeiOilPressLChange);

void onIfeiOilPressRChange(char* newValue) {
  OILR_v = atol(newValue);
  update_element(newValue,OILR);
}
DcsBios::StringBuffer<3> ifeiOilPressRBuffer(0x749a, onIfeiOilPressRChange);

void onIfeiOilTextureChange(char* newValue) {
  
  if (strcmp(newValue, "1") == 0) {
    oilT_value="OIL";
    nozT_value="NOZ";
   
  }
  else if (strcmp(newValue, "0") == 0) {
    oilT_value="   ";
    nozT_value="   ";
  }
  update_element(oilT_value,OILT);
  update_element(nozT_value,NOZT);
}
DcsBios::StringBuffer<1> ifeiOilTextureBuffer(0x74c4, onIfeiOilTextureChange);


//################## NOZZL ##################
//LEFT
int NOZL_v = 0;
int NOZL_v_OLD = 0;

void onExtNozzlePosLChange(unsigned int newValue) {
   NOZL_v = map(newValue, 0, 65535, 0, 100);
   if (NOZL_v != NOZL_v_OLD){
    Serial.print("NOZL: ");Serial.println(NOZL);
    NOZL_v_OLD = NOZL_v;
    switch (NOZL) { // NOZ LEFT POSITION IFEI
      case 0 ... 9:    update_nozzel("L0.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
      case 10 ... 19:  update_nozzel("L10.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
      case 20 ... 29:  update_nozzel("L20.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
      case 30 ... 39:  update_nozzel("L30.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
      case 40 ... 49:  update_nozzel("L40.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
      case 50 ... 59:  update_nozzel("L50.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
      case 60 ... 69:  update_nozzel("L60.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
      case 70 ... 79:  update_nozzel("L70.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
      case 80 ... 89:  update_nozzel("L80.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
      case 90 ... 95:  update_nozzel("L90.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
      case 96 ... 100: update_nozzel("100.bmp",display_elements[NOZL].pos_x,display_elements[NOZL].pos_y); break;
    }
   }
}



DcsBios::IntegerBuffer extNozzlePosLBuffer(0x757a, 0xffff, 0, onExtNozzlePosLChange);

//RIGHT
int NOZR_v = 0;
int NOZR_v_OLD = 0;
void onExtNozzlePosRChange(unsigned int newValue) {
   NOZR_v = map(newValue, 0, 65535, 0, 100);
   if (NOZR_v != NOZR_v_OLD){
    NOZR_v_OLD = NOZR_v;
    switch (NOZR) { // NOZ RIGHT POSITION IFEI
      case 0 ... 9:    update_nozzel("R0.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
      case 10 ... 19:  update_nozzel("R10.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
      case 20 ... 29:  update_nozzel("R20.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
      case 30 ... 39:  update_nozzel("R30.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
      case 40 ... 49:  update_nozzel("R40.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
      case 50 ... 59:  update_nozzel("R50.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
      case 60 ... 69:  update_nozzel("R60.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
      case 70 ... 79:  update_nozzel("R70.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
      case 80 ... 89:  update_nozzel("R80.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
      case 90 ... 95:  update_nozzel("R90.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
      case 96 ... 100: update_nozzel("100.bmp",display_elements[NOZR].pos_x,display_elements[NOZR].pos_y); break;
    }
   }
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
}
 
DcsBios::IntegerBuffer cockkpitLightModeSwBuffer(0x74c8, 0x0600, 9, onCockkpitLightModeSwChange);

//################## FUEL LOWER ##################

void onIfeiFuelDownChange(char* newValue) {
  update_element(newValue,FUELL);
}
DcsBios::StringBuffer<6> ifeiFuelDownBuffer(0x748a, onIfeiFuelDownChange);

void onIfeiTimeSetModeChange(char* newValue) {
    update_element(newValue,FUELL);
}
DcsBios::StringBuffer<6> ifeiTimeSetModeBuffer(0x74b6, onIfeiTimeSetModeChange);

//################# FUEL UPPER ##################
void onIfeiFuelUpChange(char* newValue) {
   update_element(newValue,FUELU);
}
DcsBios::StringBuffer<6> ifeiFuelUpBuffer(0x7490, onIfeiFuelUpChange);

void onIfeiTChange(char* newValue) {
    update_element(newValue,FUELU);
}
DcsBios::StringBuffer<6> ifeiTBuffer(0x757c, onIfeiTChange);

//################## BINGO ################## Y
void onIfeiBingoChange(char* newValue) {
  BINGO_v = atol(newValue);
  update_element(newValue,BINGO);
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
    update_element("Z",ZULU);
  }
  else if (strcmp(newValue, "0") == 0) {
    update_element(" ",ZULU);
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
  //Serial.print("PanelLan_RGB_CLK_FREQ: ");Serial.println(PanelLan_RGB_CLK_FREQ);

  SDCARD_SPI.begin(SCK, MISO, MOSI, CS);
  SD.begin(CS,SDCARD_SPI,40000000);
  



  tft.setColorDepth(24);
  //tft.fillScreen(0x000000U);
  tft.fillScreen(tft.color888(141,76,71));
  
  


 
 //Display Demo values
  //RPM 
    update_element("0",RPML);
    update_element("0",RPMR);
    update_element("RPM",RPMT);
  //TEMP
  
    update_element("0",TMPL);
    update_element("0",TMPL);
    update_element("TEMP",TMPT);

  //Fuel flow
    update_element("0",FFL);
    update_element("0",FFR);
    update_element(" FF \nX100",FFT);
  // OIL
    update_element("0",OILL);
    update_element("0",OILR);
    update_element("OIL",OILT);
  // FUEL
    update_element("12345T",FUELU);
    update_element("54321I",FUELL);
  // Bingo
    update_element("12345", BINGO);
  //LGauge
  //print_LGauge(75,245);
  update_Clock(CLOCKU);
  update_Clock(CLOCKL);
  
}





void loop() {
  DcsBios::loop();
}
