#ifndef __DCSBIOS_FASTLEDS_H
#define __DCSBIOS_FASTLEDS_H

#include "Arduino.h"
#include "ExportStreamListener.h"
#include <FastLED.h>


namespace DcsBios {
  
struct Indicator {
    uint16_t led1;
    uint16_t led2;
    uint16_t led3;
    uint16_t led4;
    uint32_t color;
};

	class FASTLED : public Int16Buffer {
		private:
			unsigned int mask;
			Indicator indic;
			CRGB* ledArray;
			
		public:
			FASTLED(unsigned int address, unsigned int mask, Indicator indic, CRGB* ledArray  ) : Int16Buffer(address), mask(mask), indic(indic), ledArray(ledArray)  {}
			void update_color(uint32_t color){
				indic.color = color;
			}
			virtual void loop() {
				
				if (hasUpdatedData()) {
					bool state = getData() & mask;
					if ( state ){
					       ledArray[indic.led1].setColorCode(indic.color);
					       ledArray[indic.led2].setColorCode(indic.color);
					       ledArray[indic.led3].setColorCode(indic.color);
					       ledArray[indic.led4].setColorCode(indic.color);
						   
					}else {
						   ledArray[indic.led1].setRGB(0,0,0);
					       ledArray[indic.led2].setRGB(0,0,0);
					       ledArray[indic.led3].setRGB(0,0,0);
					       ledArray[indic.led4].setRGB(0,0,0);
					}
				}
			}
	};

}

#endif