// Config File
// Errol Sancaktar
// 2020-03-24

// MQTT configuration
#define MQTT_USER ""
#define MQTT_PASS ""
#define MQTT_SERVER ""

//Base of device
String baseTopic = "";

//Topic to post status
char statusTopic[50];

//This clients name
char clientName[20] = "";
char mqttSet[30] = "";











//------------\You shouldnt need to Modify any of this------------------//

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
char buffer[512];
StaticJsonDocument<200> doc;
String ip;
String mac;
String topic;
long lastReconnectAttempt = 0;
long lastUpdate = 0;
uint8_t color;
int brightness;
int r;
int g;
int b;
int w;
String state;
String status;
String avail;
#define FADESPEED 500
#define REDPIN    12
#define GREENPIN    13
#define BLUEPIN  15
#define WHITEPIN   14



//Function Definitions
bool setColor(int r = 0, int g = 0, int b = 0, int w = 255, int brightness = 100);
