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

