#include "Arduino.h"
#include <WiFi.h>
#include <Update.h>
#include <PubSubClient.h>
const char* WIFI_SSID = "MikroTroniks_You";
const char* WIFI_SECRET = "51251251";
const char* MQTT_SERVER = "192.168.1.72";
WiFiClient client;
PubSubClient mqtt(client);
static uint32_t total_len = 0;
bool update_done = false;
uint32_t start_time = 0, stop_time = 0;


void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    String _topic = String(topic);
    String response = String((char*)payload);
    // debug
    // Serial.print("payload");
    // Serial.println(_message);
    //   for (int i = 0; i < length; i++) {
    //     Serial.write((char)payload[i]);
    // }
    // Serial.println();
    if (_topic.equals("/update/start/") == 1 && response.equals("START")) {
        if(Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Serial.println("Update Start");
            start_time = millis();
            total_len = 0;
            mqtt.publish("/update/response/","START OK");
        } else {
            mqtt.publish("/update/response/","START FAILED");
        }
    }

    if (_topic.equals("/update/write/") == 1) {
        if(Update.write(payload,length)) {
                total_len += length;
                Serial.print("Write [ ");
                Serial.print(total_len);
                Serial.println(" ]");
                mqtt.publish("/update/response/","WRITE OK");
        } else {
                mqtt.publish("/update/response/","WRITE FAILED");
        }
    }

    if (_topic.equals("/update/stop/") == 1 && response.equals("STOP")) {
        if(Update.end(true)) {
            Serial.println("Update Done ");
            stop_time = millis();
            Serial.print("total Time to update : [ ");
            Serial.print((stop_time - start_time) / 1000);
            Serial.println(" ] ");
            mqtt.publish("/update/response/","STOP OK");
            update_done = true;
        } else {
            mqtt.publish("/update/response/","STOP FAILED");
        }
    }
}

void connectToWiFi() {
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_SECRET);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
//D:/esp32_ota_mqtt/.pio/build/esp32doit-devkit-v1/firmware.bin
void reconnect() {
    while (!mqtt.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqtt.connect("ESP32Client")) {
            Serial.println("connected");
            mqtt.subscribe("/update/start/");
            mqtt.subscribe("/update/write/");
            mqtt.subscribe("/update/stop/");
            mqtt.subscribe("");
        }
        else {
            Serial.print("failed, rc=");
            Serial.print(mqtt.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    connectToWiFi();
    mqtt.setServer(MQTT_SERVER, 8080);
    mqtt.setBufferSize(4096);
    mqtt.setCallback(callback);
}

void loop() {
    if (!mqtt.connected()) {
        reconnect();
    }
    mqtt.loop();
    if(update_done) {
        Serial.println("Rebooting after 5 second....");
        delay(5000);
        ESP.restart();
    }
}