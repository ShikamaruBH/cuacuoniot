#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

int in3=9;
int in4=10;
int sensorDigital=3;
int sensorAnalog=6;
int button=2;
int lastPush=0;
int limitSwitchTop=4;
int limitSwitchBottom=5;
int LED = 16; // GPIO16 (D16)

bool isLedOn = false;
bool isDoorOpen = false;

const char* ssid = "CUA CUON IOT VIPPRO";
const char* password = "quamonvidieukhien";

AsyncWebServer server(80);

void buttonHandle() {
  if(millis()-lastPush > 500) switchDoor();
  lastPush = millis();
}
void switchDoor(){
  if (isDoorOpen) closeDoor();
  else openDoor();
}
void sensorHandle() {
  closeDoor();
}
String rainSensor() {
  int level = map(analogRead(sensorAnalog), 200, 512, 3, 1);
  if (level == 1) return "Mua nho";
  if (level == 2) return "Mua vua";
  if (level == 3) return "Mua to";
  return "Khong co mua";
}
String getLed() {
  return isLedOn ? "ON" : "OFF";
}
String getDoor() {
  return isDoorOpen ? "Open":"Close";
}

void setup(){
  Serial.begin(9600);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(sensor,INPUT);
  pinMode(button,INPUT_PULLUP);
  pinMode(limitSwitchTop,INPUT_PULLUP);
  pinMode(limitSwitchBottom,INPUT_PULLUP);
  attachInterrupt(0, buttonHandle, RISING);
  attachInterrupt(1, sensorHandle, FALLING);

  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  Serial.print("Setting AP...");
  WiFi.softAP(ssid, password);
  
  server.on("/",HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/update",HTTP_GET, [](AsyncWebServerRequest *request){
    String inputMessage;
    if (request->hasParam("component")) {
      inputMessage = request->getParam("component")->value();
      if(inputMessage == "ledSwitch") {
        isLedOn = !isLedOn;
        digitalWrite(LED,isLedOn);
      } 
      else if(inputMessage == "doorSwitch") switchDoor();
      else Serial.println("No match");
    }
    else inputMessage = "No message sent";
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  server.on("/rainsensor",HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", rainSensor().c_str());
  });
  server.on("/ledState",HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getLed().c_str());
  });
  server.on("/doorState",HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getDoor().c_str());
  });
  server.begin();
  Serial.println("Server started");
  
  Serial.print("Sorf-AP IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop(){
  isDoorOpen = (digitalRead(limitSwitchTop) == 0);
}
void openDoor(){
  while(digitalRead(limitSwitchTop) != 0) {
    digitalWrite(in3,HIGH);
    digitalWrite(in4,LOW);
  }
  stopDoor();
}
void closeDoor(){
  while(digitalRead(limitSwitchBottom) != 0) {
    digitalWrite(in3,LOW);
    digitalWrite(in4,HIGH);
  }
  stopDoor();
}
void stopDoor(){
  digitalWrite(in3,LOW);
  digitalWrite(in4,LOW);
}
