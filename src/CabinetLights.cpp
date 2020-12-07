
//Libs
#include "config.h"


// Setup WifiClient
void configModeCallback (AsyncWiFiManager *wifiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(wifiManager->getConfigPortalSSID());
}
AsyncWebServer server(80);

WiFiClient espClient;
PubSubClient client(MQTT_SERVER, 1883, callback, espClient);
PubSubClientTools mqtt(client);

void turnOff() {
  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 0);
  analogWrite(WHITEPIN, 0);
  brightness = 0;
  checkState();
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

    //mqtt.publish(baseTopic + "status", "online", true);
    mqtt.publish(baseTopic + "ip", ipAdd, true);
    mqtt.publish(baseTopic + "mac", macAdd, true);
    lastReconnectAttempt = 0;
    return client.connected();
  }
  return client.connected();
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

bool checkState() {
  if (r > 0 || g > 0 || b > 0 || w > 0) {
    state = "ON";
  } else {
    state = "OFF";
  }
  return true;
}



//##//##//##//##  Setup

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("starting Setup Func");

  //Setup LED Pins
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(WHITEPIN, OUTPUT);

  //WiFiManager
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  AsyncWiFiManager wifiManager(&server,&dns);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect("LEDControl");
 if(!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }


  //if you get here you have connected to the WiFi
  Serial.println("connected...winning :)");
  ip = WiFi.localIP().toString();
  ip.toCharArray(ipAdd, 20);
  mac = WiFi.macAddress();
  mac.toCharArray(macAdd, 30);


  //init webota
  //webota.init(8080, "/update");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    server.onNotFound([](AsyncWebServerRequest *request){
      Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });

    server.begin();
    Serial.println("HTTP server started");

  // Set up LittleFS
  Serial.println("Mount LittleFS");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

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
  baseTopic.toCharArray(charBuf,30);
  strcat(charBuf, "set");
  client.subscribe(charBuf);
  state = "OFF";
  publishStatus(baseTopic + "info", "online", state,0,0,0,0,0);
  uptime = millis();
  mqtt.publish(baseTopic + "uptime", (String)uptime);
}

void loop() {
  long now = millis();
  if (!client.connected()) {
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    client.loop();
  }
  if(now - lastUpdate > 30000){
    uptime = millis();
    mqtt.publish(baseTopic + "uptime", (String)uptime);
    checkState();
    publishStatus(baseTopic + "info", "online",state,r,g,b,w,brightness);
    lastUpdate = now;
  }

  //webota.handle();
  //server.handleClient();
  checkState();

  if (uptime > 2000000000) {
    mqtt.publish(baseTopic + "info", "restart", true);
    ESP.restart();
  }
}