#ifndef _esp_light_mqtt
#define _esp_light_mqtt
#include "config.h"

PubSubClient client(*settings.MQTT_SERVER, 1883, callback, espClient);
PubSubClientTools mqtt(client);


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
  if(data.containsKey("rgb")){
    r = data["rgb"][0];
    g = data["rgb"][1];
    b = data["rgb"][2];
  }
  if(data.containsKey("white_value")){
    w = data["white_value"];
  }
  if(state == "OFF"){
    turnOff();
    publishStatus(settings.baseTopic + "info","online","OFF",0,0,0,0,0);
  }else if(state == "ON"){
    if (setColor(r, g, b, w, brightness)) {
      //mqtt.publish(baseTopic + "brightness", String(brightness), true);
      //mqtt.publish(baseTopic + "white_value", String(w), true);
      //String rgbcolor = "[" + String(r) + "," + String(g) + "," + String(b) + "]";
      //mqtt.publish(baseTopic + "rgb", rgbcolor, true);
      publishStatus(settings.baseTopic + "info","online",state,r,g,b,w,brightness);
    }
  }
}
boolean reconnect() {
  if (client.connect(*settings.clientName, *settings.MQTT_USER, *settings.MQTT_PASS, statusTopic, 1, 1, "offline")) {

    //mqtt.publish(baseTopic + "status", "online", true);
    mqtt.publish(settings.baseTopic + "ip", ipAdd, true);
    mqtt.publish(settings.baseTopic + "mac", macAdd, true);
    lastReconnectAttempt = 0;
    return client.connected();
  }
  return client.connected();
}









#endif