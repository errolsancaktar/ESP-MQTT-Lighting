#ifndef _esp_light_wifi
#define _esp_light_wifi
#include "config.h"


AsyncWebServer server(80);

WiFiClient espClient;

// Setup Wifi and AP Mode
void configModeCallback (AsyncWiFiManager *wifiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(wifiManager->getConfigPortalSSID());
}


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