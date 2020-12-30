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


persistData settings = {
  "",
  ""
};

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

// Dynamic Callbacks
String processor(const String& var){
  Serial.println(var);
  if(var == "SERVER"){\
    Serial.print(*settings.MQTT_SERVER);
    return *settings.MQTT_SERVER;
  }else if (var == "BASETOPIC"){
    Serial.print(settings.baseTopic);
    return settings.baseTopic;   
  }else if (var == "USERNAME"){
    Serial.print(*settings.MQTT_USER);
    return *settings.MQTT_USER;   
  }else if (var == "PASS"){
    Serial.print(*settings.MQTT_PASS);
    return *settings.MQTT_PASS;  
  }else if (var == "MQTTENABLE"){
    return settings.mqttenable;
  }else if (var == "CURRENT_COLOR"){
    return settings.mqttenable;
  }
    return String();
}


#endif