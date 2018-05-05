#include <Arduino.h>

/* 
PUMP HEADER

Switch Button 1 = pin 10
Switch Button 2 = pin 9
Motor Direction 1 = pin 8
Motor Direction 2 = pin 7
Pump = pin 13
  
  */

extern LiquidCrystal lcd;
extern int waterIsNotEnough;
extern int showWateringStats;
extern unsigned long showWateringStatsTime;


enum pump_status_t {STOPPED, MOVING, WATERING};
enum pump_setpoint_t {POSITION_A, POSITION_B};
struct PumpSwitch {

  int SB1;
  int SB2;
  int M1;
  int M2;
  int PUMP;
  pump_setpoint_t  switchSetpoint;
  pump_status_t    pumpStatus;
  long wateringTime;
  long wateringTimeElapsed;

};



void initPump(PumpSwitch& myPump) {
  myPump.SB1 = 10; //9
  myPump.SB2 = 9; //8
  myPump.M1 = 8; //7
  myPump.M2 = 7; //6
  myPump.PUMP = 13; //12
  pinMode(myPump.SB1, INPUT);
  pinMode(myPump.SB2, INPUT);
  pinMode(myPump.M1, OUTPUT);
  pinMode(myPump.M2, OUTPUT);
  myPump.pumpStatus = STOPPED;
  myPump.switchSetpoint = POSITION_A;
  myPump.wateringTime = 1;
  myPump.wateringTimeElapsed = 0;
}




void pump(int pin, long del) {
  digitalWrite(pin, HIGH);
  delay(del);
  digitalWrite(pin, LOW);
}

void pumpOn(int pin) {
  digitalWrite(pin, HIGH);
}

void pumpOff(int pin) {
  digitalWrite(pin, LOW);
}


void motor_stop(int pin1, int pin2) {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  delay(25);
}


void drive_forward(int pin1, int pin2) {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, HIGH);
}

void drive_backward(int pin1, int pin2) {
  digitalWrite(pin1, HIGH);
  digitalWrite(pin2, LOW);
  delay (25);
}



void interruptPumping(PumpSwitch& myPump) {
  myPump.pumpStatus = STOPPED;
  pumpOff(myPump.PUMP);
  motor_stop(myPump.M1,myPump.M2);
  
   if (waterIsNotEnough) {
    lcd.setCursor(0, 1);
    lcd.print("No more Water.        ");
  } else {
     lcd.setCursor(0, 1);
     lcd.print("Elapsed: "+ String(myPump.wateringTimeElapsed)+ "ms       ");
  }
  
  myPump.wateringTimeElapsed = 0;

}




void updateSwitchSM(PumpSwitch& myPump, long deltat) {
    
 if (myPump.pumpStatus == MOVING) {
   
   if (waterIsNotEnough) {
      interruptPumping(myPump);
      return;
   }

   if (myPump.switchSetpoint == POSITION_A) {
      if (!digitalRead(myPump.SB1) == LOW) {
        drive_forward(myPump.M1,myPump.M2);
        myPump.pumpStatus = MOVING;
      } else {
        motor_stop(myPump.M1,myPump.M2);
        myPump.pumpStatus = WATERING;
        myPump.wateringTimeElapsed -= deltat;        
        lcd.setCursor(0, 1); 
        lcd.print("Moving to P1!");
      }   
    } else {
      if (!digitalRead(myPump.SB2) == LOW) {
        drive_backward(myPump.M1,myPump.M2);
        myPump.pumpStatus = MOVING;  
      } else {
        motor_stop(myPump.M1,myPump.M2);
        myPump.pumpStatus = WATERING;
        myPump.wateringTimeElapsed -= deltat;
        lcd.setCursor(0, 1);   
        lcd.print("Moving to P2!");
      }    
    }
    
    
 }
 
  if (myPump.pumpStatus == WATERING) {

    if (waterIsNotEnough) {
      interruptPumping(myPump);
    } else {   
      if (myPump.wateringTimeElapsed < myPump.wateringTime) {
                myPump.pumpStatus = WATERING;
                pumpOn(myPump.PUMP);
                myPump.wateringTimeElapsed += deltat;
                
                lcd.setCursor(0, 1);
                if (myPump.switchSetpoint == POSITION_A) {
                   lcd.print("WateringP1 " + String(100.0*myPump.wateringTimeElapsed/(myPump.wateringTime)) + "%     ");
                } else {
                   lcd.print("WateringP2 " + String(100.0*myPump.wateringTimeElapsed/(myPump.wateringTime)) + "%     ");
                }
                
                if (showWateringStats == 0) {
                  showWateringStatsTime = millis();
                }
                showWateringStats = 1;

  
      } else {
        interruptPumping(myPump);
      }
    }
      
  }
  
  if (myPump.pumpStatus == STOPPED) {
      pumpOff(myPump.PUMP);
      motor_stop(myPump.M1,myPump.M2);
      myPump.wateringTimeElapsed = 0;
  }

}

