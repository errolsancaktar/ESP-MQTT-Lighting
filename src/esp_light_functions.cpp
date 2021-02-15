/*
  esp_light_functions.cpp - RGBW LED CONTROLLER
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


#ifndef _esp_light_func
#define _esp_light_func
#include "config.h"

int r;
int g;
int b;
int w;
int rgb[3];
unsigned long previousMillis = 0;
bool ledState = LOW;



// Light Controls
void turnOff() {
  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 0);
  analogWrite(WHITEPIN, 0);
  brightness = 0;
  checkState();
}

void blinkLed(unsigned int interval){
  if(interval > 0){
  unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }

      // set the LED with the ledState of the variable:
      digitalWrite(LED_BUILTIN, ledState);
    }  
  }
}

bool setColor(int r, int g, int b, int w, int brightness) {
  Serial.println("IN SET COLOR");
  // Convert for Lights in RGB
  int red = r * brightness / 100;
  int green = g * brightness / 100;
  int blue = b * brightness / 100;
  int white = w * brightness / 100;
  analogWrite(REDPIN, red);
  analogWrite(GREENPIN, green);
  analogWrite(BLUEPIN, blue);
  analogWrite(WHITEPIN, white);

  saveConfig();

  return true;
}
String getColor(){
  Serial.println("IN GET COLOR");
  if(strcmp(settings.lastColor, "FFFFFF") == 0){
    return settings.lastColor;
  }else{
    return "FFFFFF";
  }
}

bool checkState() {
  if (r > 0 || g > 0 || b > 0 || w > 0) {
    state = "ON";
  } else {
    state = "OFF";
  }
  return true;
}

void hexToRgb(String hex){
  // lastColor = hex;
  hex.toCharArray(settings.lastColor,7);
  saveConfig();
  int number = (int) strtol( &hex[0], NULL, 16);
      rgb[0] = number >> 16;
      rgb[1] = number >> 8 & 0xFF;
      rgb[2] = number & 0xFF;
   Serial.println(rgb[0]);
   Serial.println(rgb[1]);
   Serial.println(rgb[2]);
}

#endif
