#define DEBUG false //NOTE: the serial monitor must be opened if debug is true

#include "helpers.h"
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <Process.h>
#include <Time.h>
#include <LiquidCrystal.h>
#include "pump.h"
#include "Thermistor.h"
#include "Photoresistor.h"



/*********** HARDWARE RELATED VARIABLES ********/
const int LED_PIN = 13;             // the number of the on board LED pin
const int WATERING_PIN =  13;       // the number of the on board LED pin
const int WATER_SENSING_PIN = 0;    // the number of the on board water level sensing pin: ANALOG
const int WATER_SENSING_THRESHOLD = 800;
const int TEMPERATURE_SENSING_PIN = 2;
const int INT_TEMPERATURE_SENSING_PIN = 3;
const int LIGHT_SENSING_PIN = 1;
/**********************************************/


/*********** REST RELATED VARIABLES ***********/
unsigned long previousServerAcceptTemptative = 0;        // will store last time bridge data was checked
unsigned long previousStatRefresh = 0;
unsigned long statRefreshRate = 4000;
unsigned long timestamp = 0;       
unsigned long now_;
long deltat;
const long interval = 200;           // interval at which to check updated bridge data

char frompython[33]; //data read from bridge that was put by python. temporary storage before it is compied to a string. Allocate space for a 32 character long rest command

Process p;		// Create a process and call it "p"

String lastTime; //time when data was read from the bridge
String rest;
String time;


YunServer server;

YunClient client;
char ans[256];
/**********************************************/




/*********** WATERING LOGIC RELATED VARIABLES ***********/
PumpSwitch pump1;
int waterIsNotEnough;

Thermistor temp(TEMPERATURE_SENSING_PIN);
Thermistor intTemp(INT_TEMPERATURE_SENSING_PIN);
Photoresistor light(LIGHT_SENSING_PIN);
double lightValue = 0;
double tempValue = 0;
double intTempValue = 0;

/**********************************************/


/*********** LCD RELATED VARIABLES ***********/
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int showWateringStats = 0;
unsigned long showWateringStatsTime = 0;
unsigned long showWateringStatsDisplayTime = 5000;

/**********************************************/


void setup() {

  pinMode(LED_PIN, OUTPUT);
  pinMode(WATERING_PIN, OUTPUT);
  digitalWrite(WATERING_PIN, LOW);
  

  initPump(pump1);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Shui System On!");


  // Initialize Bridge
  lastTime = "99:99:99";
  now_ = millis();
  timestamp = now_;
  previousServerAcceptTemptative = now_;
  deltat = 0;

  Bridge.begin();

  if (DEBUG) {
    //this requires a serial cable between arduino and computer.
    Serial.begin(9600);
    delay(25);
    // Wait until serial is opened. Blink fast while doing so
    digitalWrite(LED_PIN, LOW);
    while (!Serial) {
      digitalWrite(LED_PIN, 1 - digitalRead(LED_PIN));
      delay(25);
    }
    digitalWrite(LED_PIN, LOW);
  }

  server.listenOnLocalhost();
  server.begin();
  
  
}












int waterSensing() {
  if (analogRead(WATER_SENSING_PIN)  < WATER_SENSING_THRESHOLD) {
       waterIsNotEnough = 0;
  } else {
       waterIsNotEnough = 1;
  }
}




void loop() {

  now_ = millis();

  if (millis() - previousServerAcceptTemptative >= interval) {
    previousServerAcceptTemptative = now_;
    client = server.accept();
    // Is there a new client?
    if (client) {
      rest = client.readString();
      rest.replace("\r\n", "");
      process(rest, time);
      client.stop(); // Close connection and free resources.
    }
  }

  waterSensing();
  deltat = millis() - timestamp;
  updateSwitchSM(pump1,deltat);

  
   if (!showWateringStats) {
    
    if (millis() - previousStatRefresh >= statRefreshRate) {
       
      previousStatRefresh = now_;
      updateSensors();
      if (waterIsNotEnough) {
          lcd.setCursor(0, 1);   
          lcd.print("Refill Water!       ");
      } else {
          lcd.setCursor(0, 1);   
          lcd.print("T:"+String(tempValue) + " L:"+String(lightValue) + "     ");
      }
      
    }
  } else {
    
   if (millis() - showWateringStatsTime >= showWateringStatsDisplayTime) {
     showWateringStats = 0;
   } 
   
  }

    timestamp = now_;

}


void process(String rest, String time) {
  printdebug("----------");
  String temp = "Processing [" + rest + "] at " + time;
  client.print(rest + ";len=" + rest.length() + "$");
  printdebug(temp);
  // read the command
  String command = head(rest);
  rest = tail(rest);

  client.print(command + ";len=" + command.length() + "$");
  client.print(rest + ";len=" + rest.length() + "$");

  // is "digital" command?
  if (command == "digital") {
    digitalCommand(rest);
  }
  // is "analog" command?
  else if (command == "analog") {
    analogCommand(rest);
  }
  else if (command == "pumpSwitch") {
    pumpSwitchCommand(rest);
  }
  else if (command == "water") {
    waterCommand(rest);
  }
  else if (command == "temp") {
    temperatureCommand(rest);
  }
  else if (command == "intTemp") {
    intTemperatureCommand(rest);
  }
  else if (command == "lux") {
    lightCommand(rest);
  }
  else {
    String answer = "{\"command\":\"unsupported\",\"action\":\"unsupported\"}";
    client.print(answer);
  }
}

void waterCommand(String rest) {

  String command = head(rest); rest = tail(rest);
  String answer;

  if (command == "get") {
    if (waterIsNotEnough == 1) {
      answer = "KO";
      client.print(answer);
    } else {
      answer = "OK";
      client.print(answer); 
    }
  }
  else {
    answer = "{\"command\":\"digital\",\"action\":\"unsupported\"}";
  }
  Bridge.put("answer", answer);

}


void pumpSwitchCommandExtended(String rest) { 
    //pumpSwitch/[pin1]/[pin2]/[1-2]/[millsecs]
    // sarebbe bello...
//  //parse pin number from rest string
//  pin1 = headNumber(rest);
//  rest = tail(rest); //the part of the rest following the number
//  pin2 = headNumber(rest);
//  rest = tail(rest); //the part of the rest following the number
//  switchPosition = headNumber(rest);
//  rest = tail(rest); //the part of the rest following the number
//  switchTiming = headNumber(rest);
//  rest = tail(rest); //the part of the rest following the number
//
//  PumpSwitch pump = { pin1, pin2, switchPosition, switchTiming };

}

//pumpSwitch/[1-2]/[millsecs]
void pumpSwitchCommand(String rest) {

  long wateringTime;
  int switchPosition;
  pump_setpoint_t switchPositionCurr, switchPositionNext;

  //parse pin number from rest string
  switchPosition = headNumber(rest);
  rest = tail(rest); //the part of the rest following the number
  wateringTime = headNumber(rest);
  rest = tail(rest); //the part of the rest following the number

  pump1.pumpStatus = MOVING;

  switchPositionCurr = pump1.switchSetpoint;
  
  if (switchPosition == 1) 
    pump1.switchSetpoint = POSITION_A;
  if (switchPosition == 2)
    pump1.switchSetpoint = POSITION_B;

  if (switchPosition != 1 && switchPosition != 2) {    
  } else {
  pump1.wateringTime = wateringTime;
  pump1.wateringTimeElapsed = 0;

  printdebug("Pump go from position " + String(switchPositionCurr) +"to position [" + String(switchPositionNext) + " watering for " + String(wateringTime));
  }
}














void digitalCommand(String rest) {

  int pin, value;

  //parse pin number from rest string
  pin = headNumber(rest);
  rest = tail(rest); //the part of the rest following the number

  //parse value from rest string
  value = headNumber(rest);
  rest = tail(rest);
  String answer;
  printdebug("digital command pin: " + String(pin) + ", value: " + String(value));

  if (pin != -1 && value != -1) {
    digitalWrite(pin, value);
    answer = "{\"command\":\"digital\",\"pin\":" + String(pin) + ",\"value\":" + String(value) + ",\"action\":\"write\"}";
    Bridge.put("answer", answer);
  } else if (pin != -1 && value == -1) {
    value = digitalRead(pin);
    answer = "{\"command\":\"digital\",\"pin\":" + String(pin) + ",\"value\":" + String(value) + ",\"action\":\"read\"}";
    Bridge.put("answer", answer);
  } else {
    answer = "{\"command\":\"digital\",\"action\":\"unsupported\"}";
    Bridge.put("answer", answer);
  }
  answer = String("d,") + pin + "," + value;
  client.print(answer);
}

void analogCommand(String rest) {

  int pin, value;

  //parse pin number from rest string
  pin = headNumber(rest);
  rest = tail(rest); //the part of the rest following the number

  if (pin != -1) {
    value = analogRead(pin);
    String answer = "{\"command\":\"analog\",\"pin\":" + String(pin) + ",\"value\":" + String(value) + ",\"action\":\"read\"}";
    Bridge.put("answer", answer);
      client.print(answer);

  } else {
    String answer = "{\"command\":\"analog\",\"action\":\"unsupported\"}";
    Bridge.put("answer", answer);
      client.print(answer);

  }

}

void temperatureCommand(String rest) {
  
    String answer = "{\"command\":\"analog\",\"pin\":" + String(2) + ",\"value\":" + String(tempValue) + "celsius,\"action\":\"read\"}";
    Bridge.put("answer", answer);
    client.print(answer);

}

void intTemperatureCommand(String rest) {
  
    String answer = "{\"command\":\"analog\",\"pin\":" + String(2) + ",\"value\":" + String(tempValue) + "celsius,\"action\":\"read\"}";
    Bridge.put("answer", answer);
    client.print(answer);

}

void lightCommand(String rest) {
  
    String answer = "{\"command\":\"analog\",\"pin\":" + String(1) + ",\"value\":" + String(lightValue) + " lux,\"action\":\"read\"}";
    Bridge.put("answer", answer);
    client.print(answer);

}



void updateSensors() {
  lightValue = 0.75*lightValue + 0.25*light.getLux();
  tempValue = 0.75*tempValue + 0.25*temp.getTemp();
  intTempValue = 0.75*intTempValue + 0.25*intTemp.getTemp();
}



