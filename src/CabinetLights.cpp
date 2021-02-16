/*
  CabinetLights.cpp - RGBW LED CONTROLLER
  Copyright (C) 2020  Errol Sancaktar
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


//Libs
#include "config.h"
#include <stdlib.h>





// Definitions
char charBuf[50];
int brightness;
String state;
int uptime;
bool shouldReboot;
persistData settings;
char statusTopic[512];
uint32 chipID = system_get_chip_id();



//  Setup Debugging
#define DEBUG true  //set to true for debug output, false for no debug ouput
#ifdef DEBUG
  #define Serial Serial
#else
  #define Serial {...}
#endif
//##//##//##//##  Setup

void setup() {
  // Set up Serial Logging
  Serial.begin(115200);
  Serial.println("Starting Setup Func");


 

  //Setup LED Pins
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(WHITEPIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Set up LittleFS
  Serial.println("Mount LittleFS");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }


 // Load Previous Settings
  loadConfig();
  if(strlen(settings.clientName) < 1){
    sprintf(settings.clientName,"%u", chipID);
    Serial.println(settings.clientName);
  }


  //WiFiManager
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  if(strlen(settings.clientName) > 0){
    
    wifi_station_set_hostname(settings.clientName);
  }else{
    wifi_station_set_hostname("RGBW_LED");
  }

  AsyncWiFiManagerParameter custom_text("<p>RGBW LED Controller</p>");
  wifiManager.addParameter(&custom_text);
  wifiManager.setRemoveDuplicateAPs(true);
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.autoConnect("LEDController");
 if(!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }


  //if you get here you have connected to the WiFi
  Serial.println("connected...winning");
  ticker.detach();
  digitalWrite(LED, HIGH);
  String ip = WiFi.localIP().toString();
  ip.toCharArray(ipAdd, 20);
  String mac = WiFi.macAddress();
  mac.toCharArray(macAdd, 30);


  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN,LOW);
    delay(500);
    digitalWrite(LED_BUILTIN,HIGH);

    Serial.printf(".");
  }



// Web Server Stuff

  server.begin();
  Serial.println("HTTP server started");

// ---------- Root  ---------- //

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false, processor);
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/favicon.ico", "image/png");
  });

// ---------- Get Color  ---------- //

   server.on("/getColor", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", settings.lastColor);
    Serial.print("Current Color: ");
    Serial.println(settings.lastColor);
  });

// ---------- POST DATA  ---------- //

   server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
    String message;
    Serial.println("got post data");
     if(DEBUG){ 
      int args = request->args();
      for(int i=0;i<args;i++){
        Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
      }
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isFile()){ //p->isPost() is also true
          Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
        } else if(p->isPost()){
          Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        } else {
          Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
    }

     if (request->hasParam("MQTT", true)){
      (request->getParam("MQTT", true)->value()).toCharArray(settings.mqttenable,3);
      Serial.println(settings.mqttenable);
      saveConfig();
    }
    if(strcmp(settings.mqttenable, "on") == 0)
    {
      if (request->hasParam("SERVER", true)) {
          Serial.println("IN SERVER PARAM");
          message = request->getParam("SERVER", true)->value();
          Serial.println(message);
          if(message.length() > 0){
            message.toCharArray(settings.MQTT_SERVER, 100);
            saveConfig();
          }
      }
       else {
          message = "No Server message sent";
      }
      if (request->hasParam("BASETOPIC", true)) {
          Serial.println("IN BASETOPIC PARAM");
          if((request->getParam("BASETOPIC", true)->value()).length() > 0){
            (request->getParam("BASETOPIC", true)->value()).toCharArray(settings.baseTopic,512);
            saveConfig();
            Serial.println(settings.baseTopic);
          }
      }
       else {
          message = "No Basetopic message sent";
      }
      if (request->hasParam("USERNAME", true)) {
          Serial.println("IN USERNAME PARAM");
          message = request->getParam("USERNAME", true)->value();
          Serial.println(message);
          Serial.println(message.length());
          if(message.length() > 0){
            message.toCharArray(settings.MQTT_USER, 100);
            saveConfig();
          }
      }
       else {
          message = "No USERNAME message sent";
      }
    }
      if (request->hasParam("BRIGHTNESS", true)) {
          Serial.println("IN BRIGHTNESS PARAM");
          message = request->getParam("BRIGHTNESS", true)->value();
          Serial.println(message);
          Serial.println(message.length());
          if(message.length() > 0){
            settings.brightness = message.toInt();
            saveConfig();
          }
      }
       else {
          message = "No BRIGHTNESS message sent";
      }
      if (request->hasParam("HOSTNAME", true)) {
          Serial.println("IN HOSTNAME PARAM");
          message = request->getParam("HOSTNAME", true)->value();
          Serial.println(message);
          Serial.println(message.length());
          if(message.length() > 0){
            message.toCharArray(settings.clientName, 100);
            saveConfig();
          }
      }
       else {
          message = "No Hostname message sent";
      }
      if (request->hasParam("PASS", true)) {
          Serial.println("IN PASS PARAM");
          message = request->getParam("PASS", true)->value();
          Serial.println(message);
          if(message.length() > 0){
            message.toCharArray(settings.MQTT_PASS, 100);
            saveConfig();
          }
      }
       else {
          message = "No PASS message sent";
      }
    if (request->hasParam("wifiReset", true)) {
          Serial.println("Wifi Reset Called");
          if((request->getParam("wifiReset", true)->value()) == "true"){
            resetWifi();
          }
      }
    if(request->hasParam("color", true)){
      Serial.println("IN COLOR POST DATA");
      message = request->getParam("color", true)->value();
      if(message == "FFFFFF"){
        message.toCharArray(settings.lastColor,7);
        setColor();
      }else{
      hexToRgb(message);
      // Serial.println(message);
      // for(int i = 0; i<3; i++){
      //   Serial.println(rgb[i]);
      // }
      setColor(rgb[0], rgb[1], rgb[2], 0, settings.brightness);
      
      }
    }
     Serial.println(message);
    request->redirect("/");
   
  });

// ---------- Firmaware Update  ---------- //
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        Serial.printf("Update Success: %uB\n", index+len);
      } else {
        Update.printError(Serial);
      }
    }
  });
// ---------- CSS  ---------- //

  server.on("/main.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/main.css", "text/css");
  });


    server.onNotFound([](AsyncWebServerRequest *request){
      Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });

// MQTT Stuff
  if(strcmp(settings.mqttenable, "on") == 0){ 
    Serial.println("in Mqtt Stuff");
    // set LWT
     strncpy(statusTopic,settings.baseTopic,512);
     strcat(statusTopic,"/");

    // Initial MQTT Setup
    client.connect(settings.clientName, settings.MQTT_USER, settings.MQTT_PASS, statusTopic, 1, 1, "offline");
  //  mqtt.publish(baseTopic + "status", "online", true);
    mqtt.publish(String(settings.baseTopic) + "ip", ipAdd, true);
    mqtt.publish(String(settings.baseTopic) + "mac", macAdd, true);

    // Define Callback func
    client.setCallback(callback);

    // Subscribe to MQTT Set Topic
    //settings.baseTopic.toCharArray(charBuf,30);
    strncpy(charBuf,settings.baseTopic,50);
    strcat(charBuf, "set");
    client.subscribe(charBuf);
    state = "OFF";
    publishStatus(String(settings.baseTopic) + "info", "online", state,0,0,0,0,0);
    uptime = millis();
    mqtt.publish(String(settings.baseTopic) + "uptime", (String)uptime);
  }
}
//
void loop() {
  if(shouldReboot){
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }
  // MQTT Enabled
  if(strcmp(settings.mqttenable, "on") == 0){  
    long now = millis();
    if (!client.connected()) {
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        // Attempt to reconnect
        if (reconnect()) {
          lastReconnectAttempt = 0;
        }
      }
    } else {
      // Client connected
      client.loop();
    }
    if(now - lastUpdate > 30000){
      uptime = millis();
      mqtt.publish(String(settings.baseTopic) + "uptime", (String)uptime);
      checkState();
      publishStatus(String(settings.baseTopic) + "info", "online",state,r,g,b,w,brightness);
      lastUpdate = now;
    }
  }
  // Check light state
  checkState();

//Handle reboot if needed and post about it
  if (uptime > 1000000000) {
    if(sizeof(settings.MQTT_SERVER) > 0){
      mqtt.publish(String(settings.baseTopic) + "info", "restart", true);
    }
    ESP.restart();
  }
}