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

// Light Controls
void turnOff() {
  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 0);
  analogWrite(WHITEPIN, 0);
  brightness = 0;
  checkState();
}

bool setColor(int r, int g, int b, int w, int brightness) {
  r = r * brightness / 100;
  g = g * brightness / 100;
  b = b * brightness / 100;
  w = w * brightness / 100;
  analogWrite(REDPIN, r);
  analogWrite(GREENPIN, g);
  analogWrite(BLUEPIN, b);
  analogWrite(WHITEPIN, w);
  // Store last color value
  settings.lastRed = r;
  settings.lastGreen = g;
  settings.lastBlue = b;
  settings.lastWhite = w;
  return true;
}

bool checkState() {
  if (r > 0 || g > 0 || b > 0 || w > 0) {
    state = "ON";
  } else {
    state = "OFF";
  }
  return true;
}

#endif
