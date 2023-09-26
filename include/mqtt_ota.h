#ifndef MQTT_OTA_H
#define MQTT_OTA_H
#include "defines.h"

void mqtt_callback(char* topic, byte* payload, unsigned int length);
void connect_to_wifi();
void reconnect_mqtt_server();
void setup_app();
void start_mqtt_ota_task();

#endif