#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
int dac[] = {0, 0, 0, 0, 0, 0, 0};
int DigitalPin[] = {2, 4, 7, 8, 9, 10, 11, 12, 13};
int DacPin[] = {3, 5, 6};
YunServer server;

void setup() {
pinMode(3,INPUT);
pinMode(4,INPUT);
pinMode(7,INPUT);
pinMode(8,OUTPUT);
pinMode(9,OUTPUT);
pinMode(10,OUTPUT);
pinMode(11,OUTPUT);
pinMode(12,OUTPUT);
pinMode(13,OUTPUT);
digitalWrite(13, LOW);
Bridge.begin();
digitalWrite(13, HIGH);
server.listenOnLocalhost();
server.begin();
Serial.begin(9600);

}
void loop() {
YunClient client = server.accept();
if (client) {
process(client);
client.stop();
}
delay(50);
}
void process(YunClient client) {
String command = client.readStringUntil('/');
  Serial.println(command);

if (command == "digital") {
digitalCommand(client);
}
if (command == "analog") {
analogCommand(client);
}
if (command == "dac") {
dacCommand(client);
}
if (command == "stato") {
statoCommand(client);
}
}
void digitalCommand(YunClient client) {
int pin, value;
pin = client.parseInt();
if (client.read() == '/') {
value = client.parseInt();
digitalWrite(pin, value);
}
else {
value = digitalRead(pin);
}
client.print(F("analog,"));
client.print(pin);
client.print(F(","));
client.println(value);
}
void dacCommand(YunClient client) {
int pin, value;
pin = client.parseInt();
if (client.read() == '/') {
value = client.parseInt();
dac[pin] = value;
analogWrite(pin, value);
}
else {
value = dac[pin];
}
client.print(F("dac,"));
client.print(pin);
client.print(F(","));
client.println(value);
}
void analogCommand(YunClient client) {
int pin, value;
pin = client.parseInt();
if (client.read() == '/') {
value = client.parseInt();
analogWrite(pin, value);
client.print(F("analog,"));
client.print(pin);
client.print(F(","));
client.println(value);
}
else {
value = analogRead(pin);
client.print(F("analog,"));
client.print(pin);
client.print(F(","));
client.println(value);
}
}
void statoCommand(YunClient client) {
int pin, value;
client.print(F("stato"));
for (int thisPin = 0; thisPin < 9; thisPin++) {
pin = DigitalPin[thisPin];
value = digitalRead(pin);
client.print(F("#"));
client.print(pin);
client.print(F("="));
client.print(value);
}
for (int thisPin = 0; thisPin < 3; thisPin++) {
pin = DacPin[thisPin];
value = dac[pin];
client.print(F("#"));
client.print(pin);
client.print(F("="));
client.print(value);
}
for (int thisPin = 0; thisPin < 6; thisPin++) {
value = analogRead(thisPin);
value = analogRead(thisPin);
client.print(F("#A"));
client.print(thisPin);
client.print(F("="));
client.print(value);
}
client.println("");
}
