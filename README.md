# Standalone F/A18C IFEI Display controller for DCS-WORLD

This project is all about a DCS-WORLD compatible, stand-alone F/A18C IFEI display emulation. 
It's based on a ZX7d00XE01S 7" display, powered by an ESP32s3.

![ZX7d00XE01S](https://github.com/SCUBA82/OH-IFEI/blob/main/workdir/Documentation/pictures/Zx7d00ce01.png)

It relys on pure DCS-BIOS input without the need of an additnional HDMI connection to the display.


# Hardware Modifications

## USB Connection (Optional)
The ZX7d00XE01S is ment to be programmed through it's dedicated programming port "J2". The USB connection provides power only by default.
There is a dedicated programming adapter available but a commen USB2Serial adapter will do the job as well.

To use the USB connection as programming port, respectively as serial interface, the USB data lines have to connected from the USB port to the ESP32s3.
This is achieved by closing the jumper pins R40 an R39.

<add picture> 

## GPIO Pins (Optional)

The ZX7d00XE01S provides 18 GPIO pins through a dedicated connector "J5" as well as solderable PCB pads "J6". These pins can be used to connect external buttons, switches and potentiometers.
10 pins (marked as ESP_* in the list below) are connected directly to the ESP32s3.
8 pins  (marked as LS_* in the list below) are connected through an AW9523B I2C port expander. 

<add picture> 
  
There are some shortcomings: 
- ESP_19 and ESP20 are used for the USB connection. If you have connected R40 R39 (see USB Connection ) you cannot use those pins.
- IIC_SDA and IIC_SCL are dedicated to the I2C. So only use them for additional I2C periperhals.
- The AW9523B does not provide internal pullup resistors. So external pullup resitors have to be applied if those pins are used as inputs. 

### IO MAP


|Purpose        | IO MAP        |       |
| ------------- |:-------------:| -----:|
|Power  | 5V     | 5V    |
|Power  | 3,3V   | 3,3V  |
|Power  | GND    | GND   |
|Power  | GND    | GND   |
|ESP_GPIO | IIC_SDA    | IIC_SCL   |
|ESP_GPIO | ESP_02    | ESP_01   |
|ESP_GPIO | ESP_42    | ESP_04   |
|ESP_GPIO | ESP_40    | ESP_41   |
|ESP_GPIO | ESP_19    | ESP_20   |
| | NC    | NC  |
| | NC    | NC  |
|EXT_LS_IO | LS_P07    | LS_P06   |
|EXT_LS_IO | LS_P05    | LS_P04   |
|EXT_LS_IO | LS_P03    | LS_P02   |
|EXT_LS_IO | LS_P01    | LS_P00   |
| | NC    | NC  |
|Power  | GND    | GND   |
|Power  | GND    | GND   |
|Power  | 3,3V   | 3,3V  |
|Power  | 5V     | 5V    |

### External Pullup Resistors 
The AW9523B does not provide internal pullup resistors. So external pullup resitors have to be applied if those pins are used as inputs.
This can be easily acomplished by solerding the resitors directly to the solder pads on J6. 

<add picture> 

## Hard reset to programming mode (optional)
If something goes wrong the ESP32s3 can be forced into programming mode by shorting IO0 to GND while resetting the ESP32s3. 
So it may be a good idea to provide a switch or jumper cable to connect IO0 to GND

<add picture> 

# Installation 


## Arduino 

**Prerequisites**

Install Libraries
Libraries

# Fonts

This projects uses custom fonts directly inspired by the IFEI representation in DCS-WORLD. The [VLW Font](https://wiki.seeedstudio.com/Wio-Terminal-LCD-Anti-aliased-Fonts/) font format was chosen since it offers lightweight anti aliased font capabilities which looks way more clean than bitmap fonts.  

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

LoyvanGFX offers the possibility to draw one specific color within a sprite transparent. The color 0x000000 (pure black) was chosen to be transparent so no overlapping occurs when a pointer sprite is updated. 
In regions were this effect was not wanted (pointer movement region) a slightly different color 0x010000 was chosen, so this area is still printed in "black" but not transparent. 







