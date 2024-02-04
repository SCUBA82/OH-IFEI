/*
####### Stand alone DCS-BIOS IFEI simulation using a ZX7D00CE01S esp32-s3 7" touchscreen 
Notes:
- While using active wifi, I had screen glitches with the default TFT clk frequency provided in the PANELAN library
Change frequency in PanelLan_esp32_arduino/src/board/sc05/sc05.cpp to
#define PanelLan_RGB_CLK_FREQ           (16000000)
- Upload Filesystem image for Nozzel Pointer images as well as fonts to internal littlefs prior to programming

*/

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#define LITTLEFS LittleFS

//####################### Start User Settings ####################################
/*Which DCS-BIOS fork are you using? 
------------------------------------------------------------
DCSBIOS_DEFAULT = https://github.com/DCS-Skunkworks/dcs-bios-arduino-library
Using USB for data communication
------------------------------------------------------------
DCSBIOS_WIFI_FORK = https://github.com/pavidovich/dcs-bios-arduino-library
Using Wifi for data communication
*/

//#define DCSBIOS_DEFAULT   
#define DCSBIOS_WIFI_FORK   
#include <helper.h>


#include  <DcsBios.h>


// Wifi credentials if enabled
#ifdef DCSBIOS_WIFI_FORK
  char ssid[] = "Freudenhaus2";
  char passwd[] = "aquarius";
#endif 


//####################### END User Settings ####################################
//####################### Don't edit anything below this line ##################


//Configure Display emelments
//{width,hight, posx, posy, textalign, sprite, value} 
display_element display_elements[]= {
//{  w,  h, px, py,a, sprite,   v }
  {127, 38, 26, 20,2,&THREED,"012",""}, //RPML 
  {127, 38,260, 20,0,&THREED,"345",""}, //RPMR
  { 58, 18,180, 31,1,&LABELS,"RPM",""}, //RPMT
  { 127, 38, 26, 85,2,&THREED,"678",""}, //TMPL
  { 127, 38,260, 85,0,&THREED,"987",""}, //TMPR
  { 58, 18,180, 96,1,&LABELS,"TEMP",""}, //TMPT
  { 127, 38, 26,160,2,&THREED,"0",""}, //FFL
  { 127, 38,260,160,2,&THREED,"0",""}, //FFR
  { 58, 18,180,171,1,&LABELS,"FF",""}, //FFTU
  { 65, 18,180,188,1,&LABELS,"X100",""}, //FFTL
  { 95, 38, 58,400,2,&THREED,"0",""}, //OILL
  { 95, 38,260,400,0,&THREED,"0",""}, //OILR
  { 58, 18,180,415,1,&LABELS,"OIL",""}, //OILT
  {150,154, 58,230,0,&NOZL_IMAGE[0],"0",""}, //NOZL
  {150,154,211,230,0,&NOZR_IMAGE[0],"0",""}, //NOZR
  { 58, 18,180,300,1,&LABELS,"NOZ",""}, //NOZT
  {176, 38,560, 30,2,&Fuel,"0",""}, //FUELU
  {176, 38,560, 85,2,&Fuel,"0",""}, //FUELL
  {176, 38,560,215,2,&Fuel,"0",""}, //BINGO
  { 58, 18,625,185,1,&LABELS,"BINGO",""}, //BINGOT
  {176, 35,560,310,2,&CLOCK,"",""}, //CLOCKU
  {176, 35,560,375,2,&CLOCK,"",""}, //CLOCKL
  { 18, 18,736,327,1,&TAG,"Z",""}, //ZULU Tag
  { 18, 18,736, 50,1,&TAG,"L",""}, //L Tag
  { 18, 18,736,105,1,&TAG,"R",""}, //R Tag
};



//################ Create sprites ###############################


//Create a sprite for each possible pointer nozzel pointer position from an image located within LITTLEFS and store it in Psram
//additionl sprites for scale and scale numbers as well as a blank sprite
void create_image_sprite(){
  int j = 0;
  //Left Nozzel White 
  for (int i = 0; i <= 120; i += 10){
    String filename = "/White/L" + String(i) + ".bmp";
    NOZL_IMAGE[j].setPsram(true);
    NOZL_IMAGE[j].setColorDepth(24);
    NOZL_IMAGE[j].createFromBmp(LITTLEFS,filename.c_str());
    
    j++;
  }
  //Left Nozzel Green
  for (int i = 0; i <= 120; i += 10){
    String filename = "/Green/L" + String(i) + ".bmp";
    NOZL_IMAGE[j].setPsram(true);
    NOZL_IMAGE[j].setColorDepth(24);
    NOZL_IMAGE[j].createFromBmp(LITTLEFS,filename.c_str());
    j++;
  }
  NOZL_IMAGE[j].setPsram(true);
  NOZL_IMAGE[j].setColorDepth(24);
  NOZL_IMAGE[j].createSprite(display_elements[NOZL].sprite_width, display_elements[NOZL].sprite_hight);
  NOZL_IMAGE[j].fillScreen(0x000000U);
  
  j = 0;
  //Right Nozzel White 
  for (int k = 0; k <= 120; k += 10){
    String filename = "/White/R" + String(k) + ".bmp";
    NOZR_IMAGE[j].setPsram(true);
    NOZR_IMAGE[j].setColorDepth(24);
    NOZR_IMAGE[j].createFromBmp(LITTLEFS,filename.c_str());
    j++;
  }
  
  //Right Nozzel Green 
  for (int k = 0; k <= 120; k += 10){
    String filename = "/Green/R" + String(k) + ".bmp";
    NOZR_IMAGE[j].setPsram(true);
    NOZR_IMAGE[j].setColorDepth(24);
    NOZR_IMAGE[j].createFromBmp(LITTLEFS,filename.c_str());
    j++;
  }
  NOZR_IMAGE[j].setPsram(true);
  NOZR_IMAGE[j].setColorDepth(24);
  NOZR_IMAGE[j].createSprite(display_elements[NOZR].sprite_width, display_elements[NOZR].sprite_hight);
  NOZR_IMAGE[j].fillScreen(0x000000U);
  
}

//create sprites for digital display areas and text lables; Fonts loaded from littlefs
void create_display_elements(){
 create_image_sprite();
  THREED.createSprite(display_elements[RPML].sprite_width, display_elements[RPML].sprite_hight);
  THREED.loadFont(LITTLEFS,"/Fonts/IFEI-Data-36.vlw");
  THREED.setFont(THREED.getFont());
  THREED.setColorDepth(24);
  THREED.setTextWrap(false);
  THREED.setTextColor(ifei_color);
    
  LABELS.createSprite(display_elements[RPMT].sprite_width, display_elements[RPMT].sprite_hight);
  LABELS.loadFont(LITTLEFS,"/Fonts/IFEI-Labels-16.vlw");
  LABELS.setFont(LABELS.getFont());
  LABELS.setColorDepth(24);
  LABELS.setTextColor(ifei_color);
  
  CLOCK.createSprite(display_elements[CLOCKU].sprite_width, display_elements[CLOCKU].sprite_hight);
  CLOCK.loadFont(LITTLEFS,"/Fonts/IFEI-Data-32.vlw");
  CLOCK.setFont(CLOCK.getFont());
  CLOCK.setColorDepth(24);
  CLOCK.setTextWrap(false);
  CLOCK.setTextColor(ifei_color);
  
  TAG.createSprite(display_elements[ZULU].sprite_width, display_elements[ZULU].sprite_hight);
  TAG.loadFont(LITTLEFS,"/Fonts/IFEI-Labels-16.vlw");
  TAG.setFont(LABELS.getFont());
  TAG.print(display_elements[NOZT].value);


  Fuel.createSprite(display_elements[FUELU].sprite_width, display_elements[FUELU].sprite_hight);
  Fuel.loadFont(LITTLEFS,"/Fonts/IFEI-Data-36.vlw");
  Fuel.setFont(Fuel.getFont());
  Fuel.setColorDepth(24);
  Fuel.setTextWrap(false);
  Fuel.setTextColor(ifei_color);
}

//Align text within it's sprite left, middle or right   
int set_textalignment(int element,int alignment){
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


// Update digital and label sprites and print them on the screen
void update_element(int element){
  int x1;
  if (!test_switch_enabled && element == FFR){
    x1 = set_textalignment(element, display_elements[element].textalign) - 32; 
  }else{
    x1 = set_textalignment(element, display_elements[element].textalign);
  }
  display_elements[element].sprite->clear();
  display_elements[element].sprite->setCursor(x1,0);
  display_elements[element].sprite->setTextColor(ifei_color);
  display_elements[element].sprite->print(display_elements[element].value);
  display_elements[element].sprite->pushSprite(display_elements[element].pos_x,display_elements[element].pos_y);
  
}

// Update clock sprites and print them on the screem
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



//################## Start DCS-BIOS routines ####################


//Test switch enabled 
void onLightsTestSwChange(unsigned int newValue) {
  //Hack to display 4 digits for RPM/TEMP/FF in test mode (only 3 digits provided by DCS-BIOS)
    int d_elements[] = {TMPL,TMPR, FFL,FFR};
    if ( newValue == 1 ){
      test_switch_enabled = true;
      for (int i=0; i < 4; i++){
        display_elements[d_elements[i]].old_value = display_elements[d_elements[i]].value;
        display_elements[d_elements[i]].value = "1000";
        update_element(d_elements[i]);
      }
    }else{
      for (int i=0; i < 4; i++){
        display_elements[d_elements[i]].value = display_elements[d_elements[i]].old_value;
        update_element(d_elements[i]);
      }
      test_switch_enabled = false;

    }
}
DcsBios::IntegerBuffer lightsTestSwBuffer(0x74c8, 0x0800, 11, onLightsTestSwChange);

//################## RPM  ##################
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

// ################## TEMP  ##################
//Left
void onIfeiTempLChange(char* newValue) {
  if (!test_switch_enabled){
    display_elements[TMPL].value = remove_trailing_spaces(newValue) ;
    update_element(TMPL);
  }
   
}
DcsBios::StringBuffer<3> ifeiTempLBuffer(0x74a6, onIfeiTempLChange);
//Right
void onIfeiTempRChange(char* newValue) {
  if (!test_switch_enabled){
    display_elements[TMPR].value = remove_trailing_spaces(newValue);
    update_element(TMPR);
  }
}
DcsBios::StringBuffer<3> ifeiTempRBuffer(0x74aa, onIfeiTempRChange);

//Texture
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

// ################## SP CODES  ##################
//SP 
void onIfeiSpChange(char* newValue) {
  if (!test_switch_enabled){
    display_elements[TMPL].value = newValue;
    update_element(TMPL);
  }
}
DcsBios::StringBuffer<3> ifeiSpBuffer(0x74b2, onIfeiSpChange);

//Codes
void onIfeiCodesChange(char* newValue) {
  if (!test_switch_enabled){
    display_elements[TMPR].value = newValue;
    update_element(TMPR);
  }
}
DcsBios::StringBuffer<3> ifeiCodesBuffer(0x74ae, onIfeiCodesChange);

//################## FUEL FLOW  ##################
//LEFT
void onIfeiFfLChange(char* newValue) {
  if (!test_switch_enabled){
    display_elements[FFL].value = remove_trailing_spaces(newValue);
    update_element(FFL);
  }
}
DcsBios::StringBuffer<3> ifeiFfLBuffer(0x7482, onIfeiFfLChange);

//Right
void onIfeiFfRChange(char* newValue) {
  if (!test_switch_enabled){
    display_elements[FFR].value = remove_trailing_spaces(newValue);
    update_element(FFR);
  }
}
DcsBios::StringBuffer<3> ifeiFfRBuffer(0x7486, onIfeiFfRChange);
//Texture
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

//################## OIL  ##################
//Left
void onIfeiOilPressLChange(char* newValue) {
  display_elements[OILL].value = remove_trailing_spaces(newValue);
  update_element(OILL);
}
DcsBios::StringBuffer<3> ifeiOilPressLBuffer(0x7496, onIfeiOilPressLChange);
//Right
void onIfeiOilPressRChange(char* newValue) {
  display_elements[OILR].value = remove_trailing_spaces(newValue);
  update_element(OILR);
}
DcsBios::StringBuffer<3> ifeiOilPressRBuffer(0x749a, onIfeiOilPressRChange);
//Texture
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

//################## NOZZEL Gauges  ##################
//Left
//Pointer position
int colormode = 0;
void onExtNozzlePosLChange(unsigned int newValue) {
   
   int NOZL_v = map(newValue, 0, 65535, 0, 100);
    switch (NOZL_v) { 
      case 0 ... 4:    display_elements[NOZL].value= "0";
        break;
      case 5 ... 14:   display_elements[NOZL].value= "1";
        break;
      case 15 ... 24:  display_elements[NOZL].value= "2";
        break;
      case 25 ... 34:  display_elements[NOZL].value= "3";
        break;
      case 35 ... 44:  display_elements[NOZL].value= "4";
        break;
      case 45 ... 54:  display_elements[NOZL].value= "5";
        break;
      case 55 ... 64:  display_elements[NOZL].value= "6";
        break;
      case 65 ... 74:  display_elements[NOZL].value= "7";
        break;
      case 75 ... 84:  display_elements[NOZL].value= "8";
        break;
      case 85 ... 94:  display_elements[NOZL].value= "9";
        break;
      case 95 ... 100: display_elements[NOZL].value= "10";
        break;
    }
    display_elements[NOZL].sprite = &NOZL_IMAGE[atoi(display_elements[NOZL].value) + colormode];
    if (NOZL_pointer_visible){
         display_elements[NOZL].sprite->pushSprite(display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
    } 
}
DcsBios::IntegerBuffer extNozzlePosLBuffer(0x757a, 0xffff, 0, onExtNozzlePosLChange);

//Pointer visibility
void onIfeiLpointerTextureChange(char* newValue) {
    if (strcmp(newValue, "0") == 0) {
    NOZL_IMAGE[26].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
    NOZL_pointer_visible = false;
  }else{
    display_elements[NOZL].sprite->pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
    NOZL_pointer_visible = true;
  }
}
DcsBios::StringBuffer<1> ifeiLpointerTextureBuffer(0x74d8, onIfeiLpointerTextureChange);
//Scale visibility
void onIfeiLscaleTextureChange(char* newValue) {
    if (strcmp(newValue, "0") == 0) {
      NOZL_IMAGE[26].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
      NOZL_scale_visible = false;
    }else {
      int image_pos = 12;
      if ( ifei_color == color_NIGHT){
        image_pos = 25;
      }
      NOZL_IMAGE[image_pos].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
      NOZL_scale_visible = true;
    }    
}
DcsBios::StringBuffer<1> ifeiLscaleTextureBuffer(0x74c8, onIfeiLscaleTextureChange);
//Scale numbers visibility
void onIfeiL100TextureChange(char* newValue) {
    if (strcmp(newValue, "0") == 0) {
      NOZL_IMAGE[26].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y);
      NOZL_numbers_visible = false;
    }else{
      int image_pos = 11;
      if ( ifei_color == color_NIGHT){
        image_pos = 24;
      }
      NOZL_IMAGE[image_pos].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
      NOZL_numbers_visible = true;
    }
}
DcsBios::StringBuffer<1> ifeiL100TextureBuffer(0x74d4, onIfeiL100TextureChange);

//Right
//Pointer position
void onExtNozzlePosRChange(unsigned int newValue) {
   int NOZR_v = map(newValue, 0, 65535, 0, 100);
    
    switch (NOZR_v) { 
      case 0 ... 4:   display_elements[NOZR].value= "0";
        break;
      case 5 ... 14:  display_elements[NOZR].value= "1";
        break;
      case 15 ... 24: display_elements[NOZR].value= "2";
        break;
      case 25 ... 34: display_elements[NOZR].value= "3";
        break;
      case 35 ... 44: display_elements[NOZR].value= "4";
        break;
      case 45 ... 54: display_elements[NOZR].value= "5";
        break;
      case 55 ... 64: display_elements[NOZR].value= "6";
        break;
      case 65 ... 74: display_elements[NOZR].value= "7";
        break;
      case 75 ... 84: display_elements[NOZR].value= "8";
        break;
      case 85 ... 94: display_elements[NOZR].value= "9";
        break;
      case 95 ... 100: display_elements[NOZR].value= "10";
        break;
    }
    display_elements[NOZR].sprite = &NOZR_IMAGE[atoi(display_elements[NOZR].value) + colormode];
    if (NOZR_pointer_visible){
      display_elements[NOZR].sprite->pushSprite(display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
    }
}
DcsBios::IntegerBuffer extNozzlePosRBuffer(0x7578, 0xffff, 0, onExtNozzlePosRChange);

//Pointer visibility
void onIfeiRpointerTextureChange(char* newValue) {
  if (strcmp(newValue, "0") == 0) {
    NOZR_IMAGE[26].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
    NOZR_pointer_visible = false;
  }else{
    display_elements[NOZR].sprite->pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
    NOZR_pointer_visible = true;
  }
}
DcsBios::StringBuffer<1> ifeiRpointerTextureBuffer(0x74da, onIfeiRpointerTextureChange);

//Scale visibility
void onIfeiRscaleTextureChange(char* newValue) {
    if (strcmp(newValue, "0") == 0) {
      NOZR_IMAGE[26].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
      NOZR_scale_visible = false;
    }else{
      int image_pos = 12;
      if ( ifei_color == color_NIGHT){
        image_pos = 25;
      }
      NOZR_IMAGE[image_pos].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U); 
      NOZR_scale_visible = true;
    }
}
DcsBios::StringBuffer<1> ifeiRscaleTextureBuffer(0x74ca, onIfeiRscaleTextureChange);

//Scale numbers visibility
void onIfeiR100TextureChange(char* newValue) {
    if (strcmp(newValue, "0") == 0) {
      NOZR_IMAGE[26].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y);
      NOZR_numbers_visible = false;
    }else{
      int image_pos = 11;
      if ( ifei_color == color_NIGHT){
        image_pos = 24;
      }
      NOZR_IMAGE[image_pos].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
      NOZR_numbers_visible = true;
    }
}
DcsBios::StringBuffer<1> ifeiR100TextureBuffer(0x74d6, onIfeiR100TextureChange);

//################## Light Modes  ##################

void onCockkpitLightModeSwChange(unsigned int newValue) {
    if (newValue != 0) {
      colormode = 13;
      ifei_color = color_NIGHT;
    }
    if (newValue == 0) {
      colormode = 0;
      ifei_color = color_day;
    }

    for ( int i = 0; i < 24; i++ ){
        if ( i == NOZL ){
          display_elements[NOZL].sprite = &NOZL_IMAGE[atoi(display_elements[NOZL].value) + colormode];
          if (NOZL_pointer_visible) display_elements[NOZL].sprite->pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
          if (NOZL_scale_visible)   NOZL_IMAGE[12 + colormode].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
          if (NOZL_numbers_visible) NOZL_IMAGE[11 + colormode].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
        }else if (i == NOZR ){
          display_elements[NOZR].sprite = &NOZR_IMAGE[atoi(display_elements[NOZR].value) + colormode];
          if (NOZR_pointer_visible) display_elements[NOZR].sprite->pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
          if (NOZR_scale_visible)   NOZR_IMAGE[12 + colormode].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
          if (NOZR_numbers_visible) NOZR_IMAGE[11 + colormode].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
        }else{
          update_element(i);
        }
    }
    update_Clock(CLOCKU);
    update_Clock(CLOCKL);
}
 
DcsBios::IntegerBuffer cockkpitLightModeSwBuffer(0x74c8, 0x0600, 9, onCockkpitLightModeSwChange);

//################## FUEL  ##################
//Upper
void onIfeiFuelUpChange(char* newValue) {
  display_elements[FUELU].value = remove_trailing_spaces(newValue);
  update_element(FUELU);
}
DcsBios::StringBuffer<6> ifeiFuelUpBuffer(0x7490, onIfeiFuelUpChange);

//Time mode
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

//Tag L
void onIfeiLTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0){
    display_elements[L].value = "L";
  }else{
    display_elements[L].value = " ";
  }
  update_element(L);
}
DcsBios::StringBuffer<1> ifeiLTextureBuffer(0x7582, onIfeiLTextureChange);

//Lower
void onIfeiFuelDownChange(char* newValue) {
  display_elements[FUELL].value = remove_trailing_spaces(newValue);
  update_element(FUELL);
}
DcsBios::StringBuffer<6> ifeiFuelDownBuffer(0x748a, onIfeiFuelDownChange);
//Time mode
void onIfeiTimeSetModeChange(char* newValue) {
  display_elements[FUELL].value = remove_trailing_spaces(newValue);
  update_element(FUELL);
}
DcsBios::StringBuffer<6> ifeiTimeSetModeBuffer(0x74b6, onIfeiTimeSetModeChange);

//Tag R
void onIfeiRTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0){
    display_elements[R].value = "R";
  }else{
    display_elements[R].value = " ";
  }
  update_element(R);
}
DcsBios::StringBuffer<1> ifeiRTextureBuffer(0x7584, onIfeiRTextureChange);


//################## BINGO ################## 
//Texture
void onIfeiBingoTextureChange(char* newValue) {
  if (strcmp(newValue, "1") == 0){
    display_elements[BINGOT].value = "BINGO";
  }else{
    display_elements[BINGOT].value = "    ";
  }
  update_element(BINGOT);
}
DcsBios::StringBuffer<1> ifeiBingoTextureBuffer(0x74c6, onIfeiBingoTextureChange);

//Digits
void onIfeiBingoChange(char* newValue) {
  
    display_elements[BINGO].value = remove_trailing_spaces(newValue);
    update_element(BINGO);

}
DcsBios::StringBuffer<5> ifeiBingoBuffer(0x7468, onIfeiBingoChange);


//################## CLOCK ##################
//Upper
//Hours
void onIfeiClockHChange(char* newValue) {
  TC_H = String(newValue);
  update_Clock(CLOCKU);
}
DcsBios::StringBuffer<2> ifeiClockHBuffer(0x746e, onIfeiClockHChange);
//colon 1 
void onIfeiDd1Change(char* newValue) {
    TC_Dd1 = newValue;
    update_Clock(CLOCKU);
}
DcsBios::StringBuffer<1> ifeiDd1Buffer(0x747a, onIfeiDd1Change);
//Minutes
void onIfeiClockMChange(char* newValue) {
  
  TC_M = newValue;
  update_Clock(CLOCKU);
}
DcsBios::StringBuffer<2> ifeiClockMBuffer(0x7470, onIfeiClockMChange);
//colon 2 
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

//Tag Z
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

//Lower
//Hours
void onIfeiTimerHChange(char* newValue) {
  LC_H = String(newValue);
  update_Clock(CLOCKL);
}
DcsBios::StringBuffer<2> ifeiTimerHBuffer(0x7474, onIfeiTimerHChange);

//Colon 1
void onIfeiDd3Change(char* newValue) {
  LC_Dd1 = String(newValue);
  update_Clock(CLOCKL);
}
DcsBios::StringBuffer<1> ifeiDd3Buffer(0x747e, onIfeiDd3Change);
//Minutes
void onIfeiTimerMChange(char* newValue) {
  LC_M = String(newValue);
  update_Clock(CLOCKL);
}
DcsBios::StringBuffer<2> ifeiTimerMBuffer(0x7476, onIfeiTimerMChange);
//Colon 2
void onIfeiDd4Change(char* newValue) {
  LC_Dd2 = String(newValue);
  update_Clock(CLOCKL);
}
DcsBios::StringBuffer<1> ifeiDd4Buffer(0x7480, onIfeiDd4Change);
//Seconds
void onIfeiTimerSChange(char* newValue) {
  LC_S = String(newValue);
  update_Clock(CLOCKL);
}
DcsBios::StringBuffer<2> ifeiTimerSBuffer(0x7478, onIfeiTimerSChange);


//################## CLOCK ##################
//Only changes in night mode

void onIfeiDispIntLtChange(unsigned int newValue) {
    tft.setBrightness(map(newValue, 0, 65535, 0, 255));
}
DcsBios::IntegerBuffer ifeiDispIntLtBuffer(0x7564, 0xffff, 0, onIfeiDispIntLtChange);
//################## END DCS-BIOS ##################



  int i = 0;
unsigned long start = 0;

int a = 0;
void demo(){
    for (int i=0; i < 24; i++){
        if ( i != NOZL && i != NOZR ){
          update_element(i);
        }
    }
    
      update_Clock(CLOCKU);
      update_Clock(CLOCKL);
    
      NOZL_IMAGE[11].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
      NOZL_IMAGE[12].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);
      NOZR_IMAGE[11].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
      NOZR_IMAGE[12].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);
      
     if (millis() - nozzle_update > 1000){
      NOZL_IMAGE[i].pushSprite(&tft,display_elements[NOZL].pos_x,display_elements[NOZL].pos_y,0x000000U);  
      NOZR_IMAGE[i].pushSprite(&tft,display_elements[NOZR].pos_x,display_elements[NOZR].pos_y,0x000000U);    
      nozzle_update = millis();
      if ( demo_forward ){
        if ( i == 10){
          demo_forward = false;
          i--;
        }else {
          i++;
        }
      }else{
        if (i == 0){
          demo_forward = true;
          i++;
        }else{
          i--;
        }
      }
  }

}

//################## SETUP  ##################
void setup(void) {
#ifdef DCSBIOS_DEFAULT 
  DcsBios::setup();
#endif 
#ifdef DCSBIOS_WIFI_FORK
  DcsBios::setup(ssid,passwd);
#endif
  
  tft.begin();
  Serial.begin(115200);
  
if(!LITTLEFS.begin(true)){
    Serial.println("An Error has occurred while mounting LITTLEFS");
    return;
  }

  create_display_elements();
  tft.setColorDepth(24);
  //tft.fillScreen(0x000000U);
  tft.fillScreen(tft.color888(141,76,71));
}

void loop() {
  
  DcsBios::loop();
}
