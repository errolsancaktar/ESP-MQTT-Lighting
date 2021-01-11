/*
  esp_light_asyncweb.cpp - RGBW LED CONTROLLER
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

#ifndef _esp_light_web
#define _esp_light_web
#include "config.h"


//EEPROM Stuff
// Saves Configuration
void saveConfig() {
  EEPROM.begin(512);
  Serial.println("Saving Data");
  EEPROM.put(64, settings);
  EEPROM.end();
  Serial.println(settings.mqttenable);
}
void loadConfig() {
// Loads configuration from Littlefs into RAM
  EEPROM.begin(512);
  EEPROM.get(64, settings);
  Serial.println("Loading Data");
  EEPROM.end();
  Serial.println(settings.mqttenable);
  
}

// Dynamic Callbacks
String processor(const String& var){
  //Serial.println(var);
  if(var == "SERVER"){
    Serial.print(settings.MQTT_SERVER);
    return String(settings.MQTT_SERVER);
  }else if (var == "BASETOPIC"){
    Serial.print(settings.baseTopic);
    return String(settings.baseTopic);   
  }else if (var == "USERNAME"){
    Serial.print(settings.MQTT_USER);
    return String(settings.MQTT_USER);   
  }else if (var == "PASS"){
    Serial.print(*settings.MQTT_PASS);
    return String(settings.MQTT_PASS);  
  }else if (var == "MQTT"){
    return String(settings.mqttenable);
  }else if (var == "LCOLOR"){
    if((settings.lastColor).length() > 0){
      return settings.lastColor;
    }else{
      return "FFFFFF";
    }
  }
    return String();
}
#endif