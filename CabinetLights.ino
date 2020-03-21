
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
#include <ArduinoOTA.h>
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
  htmlmsg += "<H1>Current effect: </H1>";
  htmlmsg += effect;
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
  brightness = data["brightness"];
  r = data["rgb_color"][0];
  g = data["rgb_color"][1];
  b = data["rgb_color"][2];
  w = data["white_value"];
  String effect = data["effect"];

  if (setColor(r, g, b, w, brightness)) {
    mqtt.publish(baseTopic + "brightness", String(brightness), true);
    mqtt.publish(baseTopic + "effect", effect, true);
    mqtt.publish(baseTopic + "white_value", String(w), true);
    String rgbcolor = "[" + String(r) + "," + String(g) + "," + String(b) + "]";
    mqtt.publish(baseTopic + "rgb_color", rgbcolor, true);
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
    return true;
}

void white() {
  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 0);
  analogWrite(WHITEPIN, 255);
}

void red() {
  analogWrite(REDPIN, 255);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 0);
  analogWrite(WHITEPIN, 0);
}

void green() {
  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 255);
  analogWrite(BLUEPIN, 0);
  analogWrite(WHITEPIN, 0);
}

void blue() {
  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 255);
  analogWrite(WHITEPIN, 0);
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


    int test = server.arg("color").toInt();
    server.send(200, "text/plain", server.arg("color"));
    switch (test) {
      case 1:
        white();
        break;
      case 2:
        red();
        break;
      case 3:
        green();
        break;
      case 4:
        blue();
        break;
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
  } else {
    mqtt.publish(baseTopic + "state", "OFF", true);
  }
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


  //init OTA
  // Port defaults to 8266
  ArduinoOTA.setPort(8080);

 // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("CabinetLights-L");

 // No authentication by default
  ArduinoOTA.setPassword("admin");

 // Password can be set with it's md5 value as well
 // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
 // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

 ArduinoOTA.onStart([]() {
   String type;
   if (ArduinoOTA.getCommand() == U_FLASH) {
     type = "sketch";
   } else { // U_FS
     type = "filesystem";
   }

   // NOTE: if updating FS this would be the place to unmount FS using FS.end()
   Serial.println("Start updating " + type);
 });
 ArduinoOTA.onEnd([]() {
   Serial.println("\nEnd");
 });
 ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
 });
 ArduinoOTA.onError([](ota_error_t error) {
   Serial.printf("Error[%u]: ", error);
   if (error == OTA_AUTH_ERROR) {
     Serial.println("Auth Failed");
   } else if (error == OTA_BEGIN_ERROR) {
     Serial.println("Begin Failed");
   } else if (error == OTA_CONNECT_ERROR) {
     Serial.println("Connect Failed");
   } else if (error == OTA_RECEIVE_ERROR) {
     Serial.println("Receive Failed");
   } else if (error == OTA_END_ERROR) {
     Serial.println("End Failed");
   }
 });
 ArduinoOTA.begin();
 Serial.println("Ready");
 Serial.print("IP address: ");
 Serial.println(WiFi.localIP());
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



  ArduinoOTA.handle();
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
