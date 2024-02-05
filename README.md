# Standalone F/A18C IFEI Display controller for DCS-WORLD

This project uses a ZX7d00XE01S 7" display powered by an ESP32s3 as a standalone F/A18C IFEI display controller for
use with DCS-World

![ZX7d00XE01S](https://github.com/SCUBA82/OH-IFEI/blob/main/workdir/Documentation/pictures/Zx7d00ce01.png)

Despite a typical DCS-WORLD viewport export option, this display relys on pure DCS-BIOS input without the need of an additnional HDMI connection to the display.

It includes the following features

- Custom font for digital values as well as labels (DCS-WORLD inspired)
- Custom nozzel gauge representation (DCS-WORLD inspired)
- full DCS-BIOS integration
- Wifi support (optional through different [DCS-BIOS arduino-library fork](https://github.com/pavidovich/dcs-bios-arduino-library))
- DCS-BIOS input integration (optional, WIP)
- HID device integration (optional, WIP)
- Arduino IDE and PlatformIO compatible


# Fonts

This projects uses custom fonts directly inspired by the IFEI representation in DCS-WORLD. The [VLW Font](https://wiki.seeedstudio.com/Wio-Terminal-LCD-Anti-aliased-Fonts/) font format was chosen since it offers lightweight anti aliased font capabilities. 

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


