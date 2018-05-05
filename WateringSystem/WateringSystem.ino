#define DEBUG false //NOTE: the serial monitor must be opened if debug is true

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <Process.h>
#include <Time.h>


String compiletime=__TIME__;
String compiledate=__DATE__;
String file=__FILE__;


Process p;		// Create a process and call it "p"


const int ledPin =  13;           // the number of the on board LED pin
const int wateringPin =  12;      // the number of the on board LED pin
const int waterSensingPin = 2;    // the number of the on board water level sensing pin


unsigned long previousMillis = 0;        // will store last time bridge data was checked
unsigned long now_;
const long interval = 200;           // interval at which to check updated bridge data

char frompython[33]; //data read from bridge that was put by python. temporary storage before it is compied to a string. Allocate space for a 32 character long rest command

String lastTime; //time when data was read from the bridge
String rest;
String time;


unsigned int waterIsNotEnough = 1;
unsigned int weekday_trigger = 6; //starts on. (sunday is 1) 
unsigned int hour_trigger = 8;
unsigned int minute_trigger = 0;
unsigned int weekday_range = 3;
unsigned long minute_previous = 0;        // will store last time bridge data was checked
unsigned long hour_previous = 0;        // will store last time bridge data was checked
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
tmElements_t tm;


int lowWaterEmailSent = 0;
YunServer server;




int dac[] = {0, 0, 0, 0, 0, 0, 0};
int digital_pin[] = {2, 4, 7, 8, 9, 10, 11, 12, 13};
int dac_pin[] = {3, 5, 6};



void setup() {
  
  pinMode(ledPin, OUTPUT);
  pinMode(wateringPin, OUTPUT);
  pinMode(waterSensingPin, INPUT);
  digitalWrite(wateringPin,LOW);


  lastTime="99:99:99";

 
  
  now_=millis();

  
  // Initialize Bridge
  Bridge.begin();


  if (DEBUG) {
    //this requires a serial cable between arduino and computer. 
    Serial.begin(9600);
    delay(25);
    // Wait until serial is opened. Blink fast while doing so
    digitalWrite(ledPin,LOW);
    while (!Serial) {
      digitalWrite(ledPin,1-digitalRead(ledPin));
      delay(25);
    }
    digitalWrite(ledPin,LOW);
  }
  
  
  printdebug(compiletime);
  printdebug(compiledate);
  printdebug(file);
  
  server.listenOnLocalhost();
  server.begin();
  
  if (getDate(__DATE__) && getTime(__TIME__)) {
    setTime( tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);
    printdebug("Date is: " + String(tm.Day) +","+String(tm.Month) + ","+ String(tm.Year));
  }
}

YunClient client;
char ans[256];

















void loop() {
  
  now_ = millis();
 
  if(now_ - previousMillis >= interval) {
    previousMillis = now_;   
    
     client = server.accept();

  // There is a new client?
  if (client) {
    
      rest = client.readString();
      rest.replace("\r\n", "");
      process(rest,time);
    
    // Close connection and free resources.
    client.stop();
  }

  
  }
  
 if (hour() - hour_previous > 0) {
   lowWaterEmailSent = 0;
   hour_previous = hour();
 } 
  
 if (weekday_trigger == weekday() && minute_trigger == minute() && hour_trigger == hour()) {
   weekday_trigger = (weekday_trigger + weekday_range) % 7;
   printdebug("watering!");
   pump(wateringPin,3000);
   sendWateringEmail();
 } else {
   if (minute() - minute_previous > 0) {
   String temp="Hour :" + String(hour()) + ": " + String(minute()) + " Day of the week: " + weekday();
   printdebug(temp);
   minute_previous = minute();
   }
   
   if (minute() == 20 || minute() == 40) {
        keepAlive();
   }
 }
 
 if (digitalRead(waterSensingPin)  == LOW) {
   waterIsNotEnough = 0;
 } else {
   waterIsNotEnough = 1;
   
   if (lowWaterEmailSent == 0) {
     
     lowWaterEmailSent = 1;
     printdebug("Sending email");
     Process email;
     email.begin("python");
     email.addParameter("wateringsystem_sendemail.py");
     email.addParameter("&2>1"); // pipe error output to stdout
     email.run();
     printdebug("Email sent");
     while(email.available()>0) {
        if (DEBUG) {
            Serial.print(email.read());
     }
     }
   }
 }
}

void sendWateringEmail() {
   Process email;
   email.begin("python");
   email.addParameter("wateringsystem_watering.py");
   email.addParameter("&2>1"); // pipe error output to stdout
   email.run();
}
void keepAlive() {
   Process email;
   email.begin("python");
   email.addParameter("wateringsystem_keepalive.py");
   email.addParameter("&2>1"); // pipe error output to stdout
   email.run();
}

void process(String rest, String time) {
  printdebug("----------");
  String temp="Processing ["+rest+"] at "+time;
  client.print(rest + ";len=" + rest.length() + "$");     
  printdebug(temp);
  // read the command
  String command = head(rest);
  rest=tail(rest);


  client.print(command + ";len=" + command.length() + "$");  
  client.print(rest + ";len=" + rest.length() + "$");

  
  // is "digital" command?
  if (command == "digital") {
    digitalCommand(rest);
  }
  else if (command == "status") {
    getOverallStatus();
  }
  // is "analog" command?
  else if (command == "analog") {
    analogCommand(rest);
  }
  // is "mode" command?
  else if (command == "mode") {
    modeCommand(rest);
  }
  else if (command == "pump") {
    pumpCommand(rest);
  }
  else if (command == "water") {
    waterCommand(rest);
  }
  else if (command == "time") {
    timeCommand(rest);
  }
  else {
    String answer="{\"command\":\"unsupported\",\"action\":\"unsupported\"}";
    client.print(answer);
  }
}
void timeCommand(String rest) {

  String command = head(rest); rest = tail(rest);
  String answer;

  if (command == "get") {
    int dayRemaining = weekday_trigger - weekday();
    if (dayRemaining < 0) {dayRemaining += 7;}  
    
    answer="Next watering IS between " + String(dayRemaining) + " days at " + String(hour_trigger) + ":" + String(minute_trigger) + " (now is day " + weekday() + " on " + hour() + ":" + minute() + ")" ;
  } 
  else if (command == "set") {

     int newWeekday=headNumber(rest); rest = tail(rest);    
     int newHours=headNumber(rest); rest = tail(rest);
     int newMins=headNumber(rest); 
     
     if (newWeekday != -1 && newHours != -1 && newMins != -1) {
       weekday_trigger = newWeekday;
       hour_trigger = newHours;
       minute_trigger = newMins;
     
    
    int dayRemaining = weekday_trigger - weekday();
    if (dayRemaining < 0) dayRemaining += 7;  
    answer="Next watering SET between " + String(dayRemaining) + " days at " + String(hour_trigger) + ":" + String(minute_trigger);
    } else {
      answer="{\"command\":\"time\",\"action\":\"unsupported\"}";
    }
  } else {
    answer="{\"command\":\"time\",\"action\":\"unsupported\"}";

  }
  client.print(answer);
  
}
void waterCommand(String rest) {
  
  String command = head(rest); rest = tail(rest);
  String answer;
  
  if (command == "get") {    
    if (waterIsNotEnough == 1) {
      answer="Water level is too low";
    } else {
      answer="Water level is OK";
    }
  } 
  else {
      answer="{\"command\":\"digital\",\"action\":\"unsupported\"}";    
  }
    Bridge.put("answer",answer);
    

}

void pumpCommand(String rest) {

  int pin, value;

  //parse pin number from rest string
  pin=headNumber(rest);
  rest=tail(rest); //the part of the rest following the number

  //parse value from rest string
  value=headNumber(rest);
  rest=tail(rest);
  
  printdebug("digital command pin: "+String(pin)+", value: "+String(value));

  if (pin!=-1 && value!=-1) {
    String answer="{\"command\":\"digital\",\"pin\":"+String(pin)+",\"value\":"+String(value)+",\"action\":\"write\"}";
    pump(pin,3000);
    Bridge.put("answer",answer);
  } else if (pin!=-1 && value==-1) {
    value=digitalRead(pin);
    String answer="{\"command\":\"digital\",\"pin\":"+String(pin)+",\"value\":"+String(value)+",\"action\":\"read\"}";
    Bridge.put("answer",answer);
  } else {
    String answer="{\"command\":\"digital\",\"action\":\"unsupported\"}";
    Bridge.put("answer",answer);  
   } 
}

void pump(int pin, int del) {
 digitalWrite(pin,HIGH);
 delay(del);
 digitalWrite(pin,LOW);
}


void digitalCommand(String rest) {

  int pin, value;

  //parse pin number from rest string
  pin=headNumber(rest);
  rest=tail(rest); //the part of the rest following the number

  //parse value from rest string
  value=headNumber(rest);
  rest=tail(rest);
  String answer;
  printdebug("digital command pin: "+String(pin)+", value: "+String(value));

  if (pin!=-1 && value!=-1) {
    digitalWrite(pin,value);
     answer="{\"command\":\"digital\",\"pin\":"+String(pin)+",\"value\":"+String(value)+",\"action\":\"write\"}";
    Bridge.put("answer",answer);
  } else if (pin!=-1 && value==-1) {
    value=digitalRead(pin);
     answer="{\"command\":\"digital\",\"pin\":"+String(pin)+",\"value\":"+String(value)+",\"action\":\"read\"}";
    Bridge.put("answer",answer);
  } else {
    answer="{\"command\":\"digital\",\"action\":\"unsupported\"}";
    Bridge.put("answer",answer);  
   } 
   answer = String("d,") + pin + "," + value;
   client.print(answer);
}

void analogCommand(String rest) {

  int pin, value;

  //parse pin number from rest string
  pin=headNumber(rest);
  rest=tail(rest); //the part of the rest following the number

  if (pin!=-1) {
    value=analogRead(pin);
    String answer="{\"command\":\"analog\",\"pin\":"+String(pin)+",\"value\":"+String(value)+",\"action\":\"read\"}";
    Bridge.put("answer",answer);
  } else {
    String answer="{\"command\":\"analog\",\"action\":\"unsupported\"}";
    Bridge.put("answer",answer);  
   } 
}

void modeCommand(String rest) {
  //not implemented yet. Needed if you want to change digital pins between input and output
    String answer="{\"command\":\"mode\",\"action\":\"unsupported\"}";
    Bridge.put("answer",answer);  
}

int headNumber(String s) {
  int number;
  String numberString=head(s);
  if (numberString.length()>0) {
    number=numberString.toInt();
  } else {
    number=-1;
  }
  return number;
}  









void getOverallStatus() {
  
int pin, value;
client.print("S");
for (int thisPin = 0; thisPin < 9; thisPin++) {
pin = digital_pin[thisPin];
value = digitalRead(pin);
client.print("#");
client.print(pin);
client.print("=");
client.print(value);
}
for (int thisPin = 0; thisPin < 3; thisPin++) {
pin = dac_pin[thisPin];
value = dac[pin];
client.print("#");
client.print(pin);
client.print("=");
client.print(value);
}
for (int thisPin = 0; thisPin < 6; thisPin++) {
value = analogRead(thisPin);
value = analogRead(thisPin);
client.print("#A");
client.print(thisPin);
client.print("=");
client.print(value);
}
client.println("");
}
























String head(String s) {
  
  
  //returns text after leading slash until next slash 
  if (s.charAt(0) == '/') 
    return head(s.substring(1));
  else
    return s.substring(0,s.indexOf('/'));
}

String tail(String s) {
  //returns text after second slash (if first char is a slash)
  
  if (s.charAt(s.length()-1) != '/')
    s = s + '/';
  
  if (s.charAt(0) == '/') 
    return tail(s.substring(1));
  else {
    int first = s.indexOf('/');
    return s.substring(first+1, s.lastIndexOf('/'));
  }
}

void printdebug(String s) {
  if (DEBUG) {
    Serial.print(s);
    Serial.println(char(194));
  }
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = Year;
  return true;
}

