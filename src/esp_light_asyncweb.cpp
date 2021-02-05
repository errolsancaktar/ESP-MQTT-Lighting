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




void loadConfig() {
  // Open file for reading
  File file = LittleFS.open("/config.json", "r+");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  const char* MQTT_USER = doc["MQTT_USER"];
  const char* MQTT_PASS = doc["MQTT_PASS"];
  const char* MQTT_SERVER = doc["MQTT_SERVER"];
  const char* baseTopic = doc["baseTopic"];
  const char* clientName = doc["clientName"];
  const char* mqttenable = doc["mqttenable"];
  const char* lastColor = doc["lastColor"];
  file.close();

  strcpy(settings.MQTT_USER, const_cast<char*>(MQTT_USER) );
  strcpy(settings.MQTT_PASS, const_cast<char*>(MQTT_PASS) );
  strcpy(settings.MQTT_SERVER, const_cast<char*>(MQTT_SERVER));
  strcpy(settings.baseTopic, const_cast<char*>(baseTopic) );
  strcpy(settings.clientName, const_cast<char*>(clientName));
  strcpy(settings.mqttenable, const_cast<char*>(mqttenable));
  strcpy(settings.lastColor, const_cast<char*>(lastColor));

}

// Saves the configuration to a file
void saveConfig() {

  // Open file for writing
  File file = LittleFS.open("/config.json", "w+");

  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<256> doc;

  // Set the values in the document
  doc["MQTT_USER"] = settings.MQTT_USER;
  doc["MQTT_PASS"] = settings.MQTT_PASS;
  doc["MQTT_SERVER"] = settings.MQTT_SERVER;
  doc["baseTopic"] = settings.baseTopic;
  doc["clientName"] = settings.clientName;
  doc["mqttenable"] = settings.mqttenable;
  doc["lastColor"] = settings.lastColor;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
}


// void loadConfig() {
// // Loads configuration from Littlefs into RAM
//   EEPROM.begin(512);
//   EEPROM.get(64, settings);
//   Serial.println("Loading Data");
//   EEPROM.end();
//   Serial.println(mqttenable);
// }


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
    Serial.print(settings.MQTT_PASS);
    return String(settings.MQTT_PASS);  
  }else if (var == "MQTT"){
    return String(settings.mqttenable);
  }else if (var == "LCOLOR"){
    if(strcmp(settings.lastColor, "FFFFFF") == 0){
      return settings.lastColor;
    }else{
      return "FFFFFF";
    }
  }
    return String();
}
#endif