
//Libs
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <PubSubClientTools.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <WebOTA.h>
#include "config.h"






// Setup WifiClient

ESP8266WebServer server(80);

void handleRoot() {
  String htmlmsg = "";
  htmlmsg += "<HTML><BODY><H1>Current Brightness: </H1>";
  htmlmsg += brightness;
  htmlmsg += "<BR>";
  htmlmsg += "<H1>Current colors: </H1>";
  htmlmsg += "Red: " + String(r) + "<BR>Green: " + String(g) + "<BR>Blue: " + String(b) + "<BR>White: " + String(w);
  htmlmsg += "</BODY></HTML>";
  htmlmsg += "<BR>";
  htmlmsg += "State: " + (String)state;
  htmlmsg += "<BR>";
  htmlmsg += "</BODY></HTML>";
  htmlmsg += "<BR>";

  server.send(200, "text/html", htmlmsg);
  Serial.println(htmlmsg);

}


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


WiFiClient espClient;
PubSubClient client(MQTT_SERVER, 1883, callback, espClient);
PubSubClientTools mqtt(client);


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
  if(data.containsKey("rgb")){
    r = data["rgb"][0];
    g = data["rgb"][1];
    b = data["rgb"][2];
    if(white.isNull()){
      w = 0;
    }
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
    publishStatus(baseTopic + "info","online","OFF",0,0,0,0,0);
  }else if(state == "ON"){
    if (setColor(r, g, b, w, brightness)) {
      //mqtt.publish(baseTopic + "brightness", String(brightness), true);
      //mqtt.publish(baseTopic + "white_value", String(w), true);
      //String rgbcolor = "[" + String(r) + "," + String(g) + "," + String(b) + "]";
      //mqtt.publish(baseTopic + "rgb", rgbcolor, true);
      publishStatus(baseTopic + "info","online",state,r,g,b,w,brightness);
    }
  }
}

boolean reconnect() {
  if (client.connect(clientName, MQTT_USER, MQTT_PASS, statusTopic, 1, 1, "offline")) {

    mqtt.publish(baseTopic + "ip", ipAdd, true);
    mqtt.publish(baseTopic + "mac", macAdd, true);
    mqtt.publish(baseTopic = "check", "Still Here");
    client.subscribe("home/Cabinetlights/set");
    lastReconnectAttempt = 0;
    return client.connected();
  }
  return false;

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
  return true;
}

void turnOff() {
  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 0);
  analogWrite(WHITEPIN, 0);
  brightness = 0;
  checkState();
}


void handleLight() {
  if (server.arg("c") == "") {   //Parameter not found
    server.send(200, "text/plain", "Nothing");
  } else {    //Parameter found


    int test = server.arg("c").toInt();
    server.send(200, "text/plain", server.arg("c"));
    switch (test) {
      case 5:
      turnOff();
      break;
    }
    server.sendHeader("Location", "/");
  }


}

bool checkState() {
  if (r > 0 || g > 0 || b > 0 || w > 0) {
    if(brightness == 0){
      state = "OFF";
    }
    state = "ON";
  } else {
    state = "OFF";
  }
  return true;
}

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

//##//##//##//##  Setup

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("starting Setup");

  //Setup LED Pins
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(WHITEPIN, OUTPUT);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();


  //if you get here you have connected to the WiFi
  Serial.println("connected...winning :)");
  ip = WiFi.localIP().toString();
  ip.toCharArray(ipAdd, 20);
  mac = WiFi.macAddress();
  mac.toCharArray(macAdd, 30);


  //init webota
  webota.init(8080, "/update");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
    server.on("/cmd", handleLight);
    server.on("/", handleRoot);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
  }

  // set LWT
  baseTopic.toCharArray(statusTopic,30);
  strcat(statusTopic, "info");

  // Initial MQTT Setup
  client.connect(clientName, MQTT_USER, MQTT_PASS, statusTopic, 1, 1, "offline");
//  mqtt.publish(baseTopic + "status", "online", true);
  mqtt.publish(baseTopic + "ip", ipAdd, true);
  mqtt.publish(baseTopic + "mac", macAdd, true);

  // Define Callback func
  client.setCallback(callback);

  // Subscribe to MQTT Set Topic
  client.subscribe("home/Cabinetlights/set");
  state = "OFF";
  publishStatus(baseTopic + "info", "online", state,0,0,0,0,0);
  uptime = millis();
  mqtt.publish(baseTopic + "uptime", (String)uptime);
  mqtt.publish(baseTopic + "status","{\"status\": \"online\"}",true);

}

void loop() {
  long now = millis();
  if (!client.connected()) {
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        mqtt.publish(baseTopic = "check", "reconnected");
        lastReconnectAttempt = 0;
      }else{
        mqtt.publish(baseTopic = "check", "problem with reconnect");
      }
    }
  }
  client.loop();
  if(now - lastUpdate > 30000){
    uptime = millis();
    mqtt.publish(baseTopic + "uptime", (String)uptime);
    checkState();
    publishStatus(baseTopic + "info", "online",state,r,g,b,w,brightness);
    mqtt.publish(baseTopic + "status","{\"status\": \"online\"}",true);
    lastUpdate = now;
  }

  webota.handle();
  server.handleClient();
  MDNS.update();
  checkState();

  if (uptime > 4000000000) {
    mqtt.publish(baseTopic + "info", "restart", true);
    ESP.restart();
  }
}
