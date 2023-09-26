#ifndef DEFINE_H
#define DEFINE_H

#include "Arduino.h"
#include <WiFi.h>
#include <Update.h>
#include <PubSubClient.h>
#include "mqtt_ota.h"

#define WIFI_SSID                   "Your ssid"
#define WIFI_PASS                   "Your password"
#define MQTT_SERVER                 "Your IP"
#define MQTT_PORT                   1883  // by Default 1883

#define UPDATE_START_TOPIC          "/update/start/"
#define UPDATE_WRITE_TOPIC          "/update/write/"
#define UPDATE_STOP_TOPIC           "/update/stop/"
#define UPDATE_RESPONSE_TOPIC       "/update/response/"
#define UPDATE_CANCEL_TOPIC         "/update/cancel/"

#define START_MSG                   "START"
#define STOP_MSG                    "STOP"
#define CANCEL_MSG                  "CANCEL"

#define START_OK_RES                "START OK"
#define START_FAILED_RES            "START FAILED"
#define WRITE_OK_RES                "WRITE OK"
#define WRITE_FAILED_RES            "WRITE FAILED"
#define STOP_OK_RES                 "STOP OK"
#define STOP_FAILED_RES             "STOP FAILED"
#define CANCEL_OK_RES               "CANCEL OK"
#define CANCEL_FAILED_RES           "CANCEL FAILED"


extern WiFiClient client;
extern PubSubClient mqtt;
extern bool update_done;
extern uint32_t start_time, stop_time;

#endif