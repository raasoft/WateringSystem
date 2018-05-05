/**********************************************************************************/
/* Photoresistor -- manages a 10k Photoresistor in the follwing circuit: 		  */
/*     										  */
/*	(Ground) ---- (10k-Resister) -------|------- (VT935G) ---- (+5v)	  */
/*                                          |					  */
/*                                      Analog Pin				  */
/*										  */
/**********************************************************************************/

#ifndef Photoresistor_h
#define Photoresistor_h

#include "Arduino.h"
#include "math.h"

class Photoresistor {

	public:
		Photoresistor(int pin);
		double getLux();

	private:
		int _pin;

};

Photoresistor::Photoresistor(int pin) {
  _pin = pin;
}

double Photoresistor::getLux() {

  int RawADC = analogRead(_pin);
// Calculating the voltage in the input of the ADC
		double voltage = 5.0 * ((double)RawADC / 1024.0);
 
		// Calculating the resistance of the photoresistor 
 
		// in the voltage divider
		double resistance = (8.0 * 5.0) / voltage - 8.0;
 
		// Calculating the intensity of light in lux
		double illuminance = 255.84 * pow(resistance, -10/9);
 
  return illuminance;  // Return the Temperature

}

#endif
