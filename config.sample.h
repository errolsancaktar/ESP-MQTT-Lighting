// Config File
// Errol Sancaktar
// 2020-03-13

// Instantiate Variables

long duration;
int distance;
char msg[30];
char charBuf[50];
char nameBuf[50];
char ipAdd[20];
char macAdd[30];
int previousTime;
int uptime;
String ip;
String mac;
String topic;
long lastReconnectAttempt = 0;
uint8_t color;
int brightness;
int r;
int g;
int b;
int w;
String effect = "NULL";
#define FADESPEED 500
#define REDPIN    15
#define GREENPIN    12
#define BLUEPIN  14
#define WHITEPIN   13
#define MQTT_USER "<USERNAME>"
#define MQTT_PASS "<PASSWORD>""
#define MQTT_SERVER "<SERVER>"
String baseTopic = "home/Cabinetlights/";
char cbaseTopic[50] = "home/Cabinetlights/";
char clientName[50] = "CabinetLights-L";
char mqttSet[30] = "home/Cabinetlights/set";
