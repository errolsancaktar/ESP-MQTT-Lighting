
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

  r = 0;
  g = 0;
  b = 0;
  w = 255;
  if(brightness >= 0){
    brightness = brightness;
  }else{
    brightness = 100;
  }
  StaticJsonDocument<400> jsonpl;
  deserializeJson(jsonpl, payload, length);
  JsonObject data = jsonpl.as<JsonObject>();
  if(data.containsKey("state")){
    (String)state = data["state"].as<String>();
  }
  if(data.containsKey("brightness")){
    brightness = data["brightness"];
    checkState();
  }
  if(data.containsKey("rgb")){
    r = data["rgb"][0];
    g = data["rgb"][1];
    b = data["rgb"][2];
    checkState();
  }
  if(data.containsKey("white_value")){
    w = data["white_value"];
    checkState();
  }
if(state == "ON"){
    if (setColor(r, g, b, w, brightness)) {
      mqtt.publish(baseTopic + "brightness", String(brightness), true);
      mqtt.publish(baseTopic + "white_value", String(w), true);
      String rgbcolor = "[" + String(r) + "," + String(g) + "," + String(b) + "]";
      mqtt.publish(baseTopic + "rgb", rgbcolor, true);
    }
  }else{
    turnOff();
  }
}

boolean reconnect() {
  if (client.connect(clientName, MQTT_USER, MQTT_PASS, statusTopic, 1, 1, "offline")) {

    mqtt.publish(baseTopic + "status", "online", true);
    mqtt.publish(baseTopic + "ip", ipAdd, true);
    mqtt.publish(baseTopic + "mac", macAdd, true);
    lastReconnectAttempt = 0;
    return client.connected();
  }
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
    state = "ON";
    return true;
}

void turnOff() {
  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 0);
  analogWrite(WHITEPIN, 0);
}


void handleLight() {
  if (server.arg("color") == "") {   //Parameter not found
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
    mqtt.publish(baseTopic + "state", "ON", true);
    state = "ON";
  } else {
    mqtt.publish(baseTopic + "state", "OFF", true);
    state = "OFF";
  }
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
  strcat(statusTopic, "status");

  // Initial MQTT Setup
  client.connect(clientName, MQTT_USER, MQTT_PASS, statusTopic, 1, 1, "offline");
  mqtt.publish(baseTopic + "status", "online", true);
  mqtt.publish(baseTopic + "ip", ipAdd, true);
  mqtt.publish(baseTopic + "mac", macAdd, true);

  // Define Callback func
  client.setCallback(callback);


  // Subscribe to MQTT Set Topic
  client.subscribe("home/Cabinetlights/set");
}

void loop() {

  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
      String UTtopic = baseTopic + "uptime";
      uptime = millis();
      itoa(uptime, msg, 10);
      UTtopic.toCharArray(charBuf, 50);
      client.publish(charBuf, msg);
    }
  } else {
    // Client connected
    client.loop();
  }



  webota.handle();
  server.handleClient();
  MDNS.update();
  checkState();

  if (uptime > 4000000000) {
    topic = baseTopic + "/INFO";
    topic.toCharArray(charBuf, 50);
    client.publish(charBuf, "Restart", true);
    ESP.restart();
  }
}
