#include <Stepper.h>
#define STEPS 100

Stepper small_stepper(STEPS, 2, 4, 3, 5);
int  Steps2Take;


int ledPin = 13; // choose the pin for the LED
int inPin = 7;   // choose the input pin (for a pushbutton)
int val = 0;     // variable for reading the pin status



void setup()  
{
   pinMode(ledPin, OUTPUT);  // declare LED as output
   pinMode(inPin, INPUT);    // declare pushbutton as input
  Serial.begin(9600);
}

void loop()  
{
  small_stepper.setSpeed(50);  
  Steps2Take  =  2048;  

  if (true) {
  
    // Rotate CW For a complete rotation
  // Change this number for change the degrees of the rotation
  small_stepper.step(Steps2Take);
  delay(2000);
  small_stepper.setSpeed(50);  

  // Change this for change the speed
  Steps2Take  =  -2048;  // Rotate CCW
  small_stepper.step(Steps2Take);
  delay(2000);
  
  }
  
   //val = digitalRead(inPin);  // read input value
  
}

