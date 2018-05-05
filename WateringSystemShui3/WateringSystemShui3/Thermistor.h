/**********************************************************************************/
/* Thermistor -- manages a 10k Thermistor in the follwing circuit: 		  */
/*     										  */
/*	(Ground) ---- (10k-Resister) -------|------- (B57164K0473) ---- (+5v)	  */
/*                                          |					  */
/*                                      Analog Pin				  */
/*										  */
/**********************************************************************************/

#ifndef Thermistor_h
#define Thermistor_h

#include "Arduino.h"
#include "math.h"

class Thermistor {

	public:
		Thermistor(int pin);
		double getTemp();

	private:
		int _pin;

};

Thermistor::Thermistor(int pin) {
  _pin = pin;
}

double Thermistor::getTemp() {

  // Inputs ADC Value from Thermistor and outputs Temperature in Celsius

  int RawADC = analogRead(_pin);

  double Resistance;
  double Temp;


  // Assuming a 10k Thermistor.  Calculation is actually: Resistance = (1024/ADC)

  Resistance=((1024.0)/RawADC - 1)*8000;//((10240000/RawADC) - 10000);


  /**************************************************************/
  /*   Utilizes the Steinhart-Hart Thermistor Equation:		*/
  /*   Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]^3}	*/
  
  // A = 0.9291538594 * 10-4
  // B = 2.258551386 * 10-5
  // C = -0.03907565473 * 10-7
  
  // http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm

  /******************************************************************/

  double A = 0.9291538594*pow(10,-3);
  double B = 2.258551386*pow(10,-4);
  double C = -0.03907565473*pow(10,-7);
  
  double lr = log(Resistance);
  Temp = 1.0 / (A + (B * lr) + ( (lr * lr * lr) * C ));
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius

  return Temp;  // Return the Temperature

}

#endif
