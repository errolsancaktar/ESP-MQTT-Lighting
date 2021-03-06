/*
  esp_light_wifi.cpp - RGBW LED CONTROLLER
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

#ifndef _esp_light_wifi
#define _esp_light_wifi
#include "config.h"
#include <ESPAsyncWiFiManager.h> 
#include "esp_light_wifi.h"
Ticker ticker;
int LED = LED_BUILTIN;

void tick()
{
  //toggle state
  digitalWrite(LED, !digitalRead(LED));     // set pin to the opposite state
}


// Setup Wifi and AP Mode
void configModeCallback (AsyncWiFiManager *wifiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(wifiManager->getConfigPortalSSID());
  ticker.attach(0.2, tick);
}
void resetWifi(){
  Serial.println("Resetting Wifi Credentials");
  wifiManager.resetSettings();
  Serial.println("Reset..");
  ESP.restart();
}
#endif