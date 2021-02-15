

#ifndef _esp_light_wifi_h
#define _esp_light_wifi_h


DNSServer dns;
AsyncWebServer server(80);
AsyncWiFiManager wifiManager(&server,&dns);



#endif