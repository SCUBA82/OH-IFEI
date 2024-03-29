# Standalone F/A18C IFEI Display controller for DCS-WORLD

This project is all about a DCS-WORLD compatible, stand-alone F/A18C IFEI display emulation. 
It's based on a ZX7d00XE01S 7" display, powered by an ESP32s3.

![ZX7d00XE01S](https://github.com/SCUBA82/OH-IFEI/blob/main/workdir/Documentation/pictures/Zx7d00ce01.png)

It relys on pure DCS-BIOS input without the need of an additnional HDMI connection to the display.


# Hardware 

## USB Connection (Optional)
The ZX7d00XE01S is ment to be programmed through it's dedicated programming port "J2". The USB connection provides power only by default.
There is a dedicated programming adapter available but a commen USB2Serial adapter will do the job as well.

To use the USB connection as programming port, respectively as serial interface, the USB data lines have to connected from the USB port to the ESP32s3.
This is achieved by closing the jumper pins R40 an R39.

<add picture> 

## GPIO Pins 

The ZX7d00XE01S provides 18 GPIO pins through a dedicated connector "J5" as well as on solderpads "J6". These pins can be used to connect external buttons, switches and potentiometers.
The predefined pin mapping can be found in the "IO MAP" or customized within helper.h.

10 pins (marked as ESP_* in the list below) are connected directly to the ESP32s3.
8 pins  (marked as LS_* in the list below) are connected through an AW9523B I2C port expander. 

<add picture> 
  
There are some shortcomings: 
- ESP_19 and ESP20 are used for the USB connection. If you have shorted R40 and R39 for USB connection. you cannot use those pins.
- IIC_SDA and IIC_SCL are dedicated to I2C. So only use them for additional I2C periperhals.
- The AW9523B does not provide internal pullup resistors. So external pullup resitors have to be applied.

### IO MAP


|    fuction left    |       left        |   right    | function right |
| ------------- |:-------------:| ------|------:|
|Power  | 5V     | 5V    |Power |
|Power  | 3,3V   | 3,3V  |Power|
|Power  | GND    | GND   |Power|
|Power  | GND    | GND   |Power|
|I2C | IIC_SDA    | IIC_SCL |I2C|
|brightness pot | ESP_02    | ESP_01   | mode switch MAN |
|mode switch AUTO | ESP_42    | ESP_04   |  select switch HUD |
|select switch LDDI | ESP_40    | ESP_41   | - |
|USBdata+ | ESP_19    | ESP_20   | USBdata-|
| | NC    | NC  ||
| | NC    | NC  ||
|- | LS_P07    | LS_P06   |-|
|ET button | LS_P05    | LS_P04   | zone button |
|arrow down button | LS_P03    | LS_P02   | arrow up button |
|qty button | LS_P01    | LS_P00   | mode button |
| | NC    | NC  ||
|Power  | GND    | GND   |Power|
|Power  | GND    | GND   |Power|
|Power  | 3,3V   | 3,3V  |Power|
|Power  | 5V     | 5V    |Power|

### External Pullup Resistors 
The AW9523B does not provide internal pullup resistors. So external pullup resitors have to be applied if those pins are used as inputs.
This can be easily acomplished by solerding 10k resistors between the desired pin and +3,3V an J6.

<add picture> 

## Reset to programming mode (optional)
If something goes wrong the ESP32s3 can be forced into programming mode by shorting IO0 to GND while resetting the ESP32s3. 
So it may be a good idea to provide a switch or jumper cable to connect IO0 to GND

<add picture> 

# Installation and Configuration 

This project is ment to be used along with platformIO but can be easily ported to Arduino IDE. 

##PlatformIO settings



## Configuration
There are some configurable options within config.h

### DCS Fork
The projects allows to use different forks of DCS-BIOS

#### DCS-BIOS Skunkworks Fork
[DCS-BIOS Skunkworks Fork](https://github.com/DCS-Skunkworks/dcs-bios-arduino-library) uses the serial protocoll to communicate with DCS. 
#define DCSBIOS_DEFAULT
//#define DCSBIOS_WIFI

#### DCS-BIOS Wifi Fork



# Fonts

The fonts used in this project were directly inspired by the IFEI representation in DCS-WORLD. The [VLW Font](https://wiki.seeedstudio.com/Wio-Terminal-LCD-Anti-aliased-Fonts/)  format was chosen since it offers lightweight anti aliased font capabilities which looks way more clean than bitmap fonts.  

## How fonts were created
- DCS-WORLD textures were used as templates. 
- [Vector graphics](https://github.com/SCUBA82/OH-IFEI/tree/main/workdir/Fonts/Vectors) were created by tracing the outlines of the font elements.

<img src="https://github.com/SCUBA82/OH-IFEI/blob/main/workdir/Documentation/pictures/vector-Trace.png" width="400">

- Vector graphics were imported in ["font forge"](https://fontforge.org/en-US/)

<img src="https://github.com/SCUBA82/OH-IFEI/blob/main/workdir/Documentation/pictures/Fontforge-IFEI-DATA.png" width="400">

- The [finished fonts](https://github.com/SCUBA82/OH-IFEI/tree/main/workdir/Fonts/Fonts/TrueType) were exported as ["True Type fonts"](https://github.com/SCUBA82/OH-IFEI/tree/main/workdir/Fonts/Fonts/TrueType) and installed in windows.
- The TTF Fonts were converted to [VLW Font](https://wiki.seeedstudio.com/Wio-Terminal-LCD-Anti-aliased-Fonts/) format using [processing](https://processing.org/) with the "smooth" option

  <img src="https://github.com/SCUBA82/OH-IFEI/blob/main/workdir/Documentation/pictures/Processing.png" width="400">

- The converted VLW fonts are stored on the esp32s3 in littlefs


# Image Sprites

To display the nozzel gauges, [individual images](https://github.com/SCUBA82/OH-IFEI/tree/main/workdir/Images) for each possible pointer position and color were created.
Again DCS-WORLD textures were used as templates to outline custom vector graphics.

To overcome some sprite overlapping issues, the color "0x000000 (pure black)" was defined to be transparent within LoyvanGFX.
Areas which are intendet to have a black background were defined with the color "0x010000" which still looks pure black. 


![Transparent black](https://github.com/SCUBA82/OH-IFEI/blob/main/workdir/Documentation/pictures/transparent.png)





