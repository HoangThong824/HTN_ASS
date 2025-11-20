#include "global.h"
float glob_temperature = 0;
float glob_humidity = 0;
float glob_light = 0;
uint32_t publish_interval_ms = 10000;


String ssid = "ESP32-HT";
String password = "12345678";
String wifi_ssid = "ACLAB";
String wifi_password = "ACLAB2023";
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();