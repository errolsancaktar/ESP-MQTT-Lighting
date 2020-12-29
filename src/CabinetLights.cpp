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


//EEPROM Stuff
void saveConfig() {
  // Save configuration from RAM into EEPROM
  EEPROM.begin(4095);
  EEPROM.put( cfgStart, settings );
  delay(200);
  EEPROM.commit();                      // Only needed for ESP8266 to get data written
  EEPROM.end();                         // Free RAM copy of structure
}

void loadConfig() {
  // Loads configuration from EEPROM into RAM
  Serial.println("Loading EEPROM config");
  persistData load;
  EEPROM.begin(4095);
  EEPROM.get( cfgStart, load);
  EEPROM.end();
  settings = load;
};



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

  //WiFiManager
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  AsyncWiFiManager wifiManager(&server,&dns);
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
  ip = WiFi.localIP().toString();
  ip.toCharArray(ipAdd, 20);
  mac = WiFi.macAddress();
  mac.toCharArray(macAdd, 30);


  //init webota
  //webota.init(8080, "/update");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }




  // Set up LittleFS
  Serial.println("Mount LittleFS");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }



// Web Server Stuff

  server.begin();
  Serial.println("HTTP server started");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false, processor);
  });

  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
    String message;
    if (request->hasParam("mqttstatus", true)){
      settings.mqttenable = request->getParam("mqttstatus", true)->value();
    }
    if(settings.mqttenable == "on"){}
      if (request->hasParam("svr", true)) {
          message = request->getParam("svr", true)->value();
          message.toCharArray(*settings.MQTT_SERVER, 100);
      } else {
          message = "No message sent";
      }
    if(request->hasParam("color", true)){
      setcolor(color[0], color[1], color[2], color[3], 255); //set color from array sent over post data

    }
    Serial.println(message);
    request->redirect("/");
  });

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


  if(settings.mqttenable != "on"){ 
    // set LWT
    settings.baseTopic.toCharArray(statusTopic,30);
    strcat(statusTopic, "info");

    // Initial MQTT Setup
    client.connect(*settings.clientName, *settings.MQTT_USER, *settings.MQTT_PASS, statusTopic, 1, 1, "offline");
  //  mqtt.publish(baseTopic + "status", "online", true);
    mqtt.publish(settings.baseTopic + "ip", ipAdd, true);
    mqtt.publish(settings.baseTopic + "mac", macAdd, true);

    // Define Callback func
    client.setCallback(callback);

    // Subscribe to MQTT Set Topic
    settings.baseTopic.toCharArray(charBuf,30);
    strcat(charBuf, "set");
    client.subscribe(charBuf);
    state = "OFF";
    publishStatus(settings.baseTopic + "info", "online", state,0,0,0,0,0);
    uptime = millis();
    mqtt.publish(settings.baseTopic + "uptime", (String)uptime);
  }
}

void loop() {
  if(settings.mqttenable != "on"){  
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
      mqtt.publish(settings.baseTopic + "uptime", (String)uptime);
      checkState();
      publishStatus(settings.baseTopic + "info", "online",state,r,g,b,w,brightness);
      lastUpdate = now;
    }
  }
  checkState();

  if (uptime > 2000000000) {
    *settings.MQTT_SERVER != NULL ? mqtt.publish(settings.baseTopic + "info", "restart", true): NULL ;
    ESP.restart();
  }
}