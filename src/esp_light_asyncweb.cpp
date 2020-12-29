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


AsyncWebServer server(80);

// Dynamic Callbacks
String processor(const String& var){
  Serial.println(var);
  if(var == "SERVER"){
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
  }else if (var == "HEXCOLOR"){
    if(hexcolor == NULL){
      return "000000";
    }else{
      return hexcolor;
    }
    return String();
  }
}


#endif