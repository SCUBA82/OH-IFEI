/* 
A rough test for implementing the AW9523 I2C portexpander 
AW9523 code sinpptes from https://github.com/Isharathilina/AW9523_and_PCA9557_library_for_esp32/
*/
#ifndef __DCSBIOS_AW9523_H
#define __DCSBIOS_AW9523_H

#include <math.h>
#include <Wire.h>
#include "Arduino.h"

#define inputPort0 0x00
#define inputPort1 0x01

#define configPort0 0x04
#define configPort1 0x05


namespace DcsBios {
	template <unsigned long pollIntervalMs = POLL_EVERY_TIME>
	class Switch2PosAW9523T : PollingInput, public ResettableInput
	{
	private:
		const char* msg_;
		char pin_;
		char debounceSteadyState_;
		char lastState_;
		bool reverse_;
		unsigned long debounceDelay_;
		unsigned long lastDebounceTime = 0;
		bool is_initialised = false;
		
		int i2cAddress = 0x5B;    
		uint8_t pinMode = 0xff; // all as input (1= input)
		
		void resetState()
		{
			lastState_ = (lastState_==0)?-1:0;
		}

		void pollInput() {
			char state = AWdigitalRead(pin_);
			if (reverse_) state = !state;

			unsigned long now = millis();

			if (state != debounceSteadyState_) {
				lastDebounceTime = now;
				debounceSteadyState_ = state;
			}
			
			if ((now - lastDebounceTime) >= debounceDelay_) {
				if (debounceSteadyState_ != lastState_) {
					if (tryToSendDcsBiosMessage(msg_, state == HIGH ? "0" : "1")) {
						lastState_ = debounceSteadyState_;
					}
				}
			}			
		}
	public:
		Switch2PosAW9523T(const char* msg, char pin, bool reverse = false, unsigned long debounceDelay = 50) :
			PollingInput(pollIntervalMs)
		{				
			msg_ = msg;
			pin_ = pin;
			
			debounceDelay_ = debounceDelay;
			reverse_ = reverse;

			lastState_ = AWdigitalRead(pin_);
			if (reverse_) lastState_ = !lastState_;		
			
		}

		void init()
		{  			
			Wire.begin(GPIO_NUM_48,GPIO_NUM_47);
			Wire.beginTransmission(i2cAddress); 
			Wire.write(configPort0);       
			Wire.write(pinMode);        
			Wire.endTransmission();   

			Wire.beginTransmission(i2cAddress); 
			Wire.write(configPort1);       
			Wire.write(pinMode);        
			Wire.endTransmission();    
		}


		bool AWdigitalRead(char cpin)
		{
			int pin = atoi(&cpin);
			if(pin<8)
			{  //00h reg

				Wire.beginTransmission(i2cAddress);  
				Wire.write(inputPort0);       
				Wire.endTransmission();   

				uint8_t ReadDataP0;
				Wire.requestFrom(i2cAddress, 1);   
				while (Wire.available()){
					 ReadDataP0 = Wire.read(); 
				}
				return  (ReadDataP0 >> pin) & 1;


			}else
			{ // 01h reg

				Wire.beginTransmission(i2cAddress); 
				Wire.write(inputPort1);        
				Wire.endTransmission();    

				uint8_t ReadDataP1;
				Wire.requestFrom(i2cAddress, 1);   
				while (Wire.available()){
					 ReadDataP1 = Wire.read(); 
				}
				return  (ReadDataP1 >> pin-8) & 1;

			}
		}
				
		void SetControl( const char* msg )
		{
			msg_ = msg;
		}
        
		void resetThisState()
		{
			this->resetState();
		}

	};
	typedef Switch2PosAW9523T<> Switch2PosAW9523;
	
}

#endif	