/*
  config.h - RGBW LED CONTROLLER
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

#ifndef _config_h
#define _config_h
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <PubSubClient.h>
#include <PubSubClientTools.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <ESPAsyncWiFiManager.h>  
#include "LittleFS.h"
#include <stdlib.h>
#include <Ticker.h>
#include <SD.h>
#include <EEPROM.h>

#define FADESPEED 500
#define REDPIN    12
#define GREENPIN    13
#define BLUEPIN  15
#define WHITEPIN   14

//Parameters

//extern const char* PARAM_MESSAGE;


//EEPROM Setup
#define cfgStart 0
#define version "SufetLight"



struct persistData{
    char MQTT_USER[100];
    char MQTT_PASS[100];
    char MQTT_SERVER[100];
    char baseTopic[512];
    char clientName[50];
    char mqttenable[4];
    char lastColor[7];
    int brightness;

};

extern persistData settings;




//Topic to post status
extern char statusTopic[512];
extern char mqttSet[30];

// Instantiate Variables
extern const char *filename;
extern int distance;
extern char msg[30];
extern char charBuf[50];
extern char nameBuf[50];
extern char ipAdd[20];
extern char macAdd[30];
extern int previousTime;
extern int uptime;
extern char buffer[512];
extern StaticJsonDocument<200> doc;
extern String ip;
extern String mac;
extern String topic;
extern long lastReconnectAttempt;
extern long lastUpdate;
extern int brightness;
extern int r;
extern int g;
extern int b;
extern int w;
extern String state;
extern String status;
extern String avail;
extern int color;
extern DNSServer dns;
extern PubSubClient client;
extern PubSubClientTools mqtt;
extern int rgb[3];
extern FSInfo fs_info;
extern uint32 chipID;
extern Ticker ticker;
extern int LED;



//Function Definitions
void saveConfig();
void loadConfig();
void configModeCallback(AsyncWiFiManager*);
extern AsyncWiFiManager wifiManager;
extern AsyncWebServer server;
bool setColor(int r = 0, int g = 0, int b = 0, int w = 255, int brightness = 100);
String getColor();
void callback(char* topic, byte* payload, unsigned int length);
bool publishStatus(String topic, String pubStatus, String pubState, int pubR, int pubG, int pubB, int pubW, int pubBrightness);
void blinkLed(unsigned int interval);
void hexToRgb(String hex);
void turnOff();
void handleNotFound();
bool checkState();
void resetWifi();
String processor(const String& var);
boolean reconnect();





#endif