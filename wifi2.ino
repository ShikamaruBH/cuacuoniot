#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

const char* ssid = "CUA CUON IOT VIPPRO";
const char* password = "quamonvidieukhien";

int LED = 16; // GPIO16 (D16)
bool isLedOn = false;
bool isDoorOpen = false;

AsyncWebServer server(80);

String rainSensor() {
  return String(random(1024));
}
String getLed() {
  return isLedOn ? "ON" : "OFF";
}
String getDoor() {
  return isDoorOpen ? "Open":"Close";
}

void setup(){
  Serial.begin(9600);
  pinMode(LED, OUTPUT);

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
      } else if(inputMessage == "doorSwitch") {
        isDoorOpen = !isDoorOpen;
      } else Serial.println("No match");
    }
    else {
      inputMessage = "No message sent";
    }
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
  
  Serial.print("Sorf-AP IP Address: "); // will IP address on Serial Monitor
  Serial.println(WiFi.softAPIP());
}

void loop(){}
