/*
  esp_light_mqtt.cpp - RGBW LED CONTROLLER
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

#ifndef _esp_light_mqtt
#define _esp_light_mqtt
#include "config.h"


long lastReconnectAttempt = 0;
long lastUpdate = 0;
WiFiClient espClient;
PubSubClient client(settings.MQTT_SERVER, 1883, callback, espClient);
PubSubClientTools mqtt(client);
char ipAdd[20];
char buffer[512];
char macAdd[30];
StaticJsonDocument<200> doc;



bool publishStatus(String topic, String pubStatus, String pubState, int pubR, int pubG, int pubB, int pubW, int pubBrightness){
  // Build Json MQTT Message
  JsonObject root = doc.to<JsonObject>();
  root["status"] = pubStatus.c_str();
  root["state"] = pubState.c_str();
  root["brightness"] = pubBrightness;
  root["white_value"] = pubW;
  JsonArray data = root.createNestedArray("rgb");
  data.add(pubR);
  data.add(pubG);
  data.add(pubB);
  serializeJson(doc, buffer);
  mqtt.publish(topic,buffer);
  return true;
}
void callback(char* topic, byte* payload, unsigned int length) {

  StaticJsonDocument<400> jsonpl;
  deserializeJson(jsonpl, payload, length);
  JsonObject data = jsonpl.as<JsonObject>();
  JsonVariant white = data["white_value"];
  if(data.containsKey("state")){
    state = data["state"].as<String>();
    if(state == "ON"){
      r = 0;
      g = 0;
      b = 0;
      w = 255;
      brightness = 100;
    }
  }
  if(data.containsKey("brightness")){
    brightness = data["brightness"];
  }
  if(data.containsKey("color")){
    r = data["color"]["r"];
    g = data["color"]["g"];
    b = data["color"]["b"];
    if(white.isNull()){
      w = 0;
    }
  }

  if(data.containsKey("white_value")){
    w = data["white_value"];
  }
  if(state == "OFF"){
    turnOff();
  }else if(state == "ON"){
    if (setColor(r, g, b, w, brightness)) {
      publishStatus(String(settings.baseTopic) + "info","online",state,r,g,b,w,brightness);
    }
  }
}
boolean reconnect() {
  strncpy(statusTopic,settings.baseTopic,512);
  strcat(statusTopic,"info");
  if (client.connect(settings.clientName, settings.MQTT_USER, settings.MQTT_PASS, statusTopic, 1, 1, "offline")) {
    mqtt.publish(String(settings.baseTopic) + "ip", ipAdd, true);
    mqtt.publish(String(settings.baseTopic) + "mac", macAdd, true);
    lastReconnectAttempt = 0;
    return client.connected();
  }
  return client.connected();
}









#endif