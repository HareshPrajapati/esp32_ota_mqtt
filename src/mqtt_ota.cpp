#include "defines.h"

WiFiClient client;
PubSubClient mqtt(client);
bool update_done = false;
uint32_t start_time = 0, stop_time = 0;
static uint32_t total_len = 0;

void mqtt_callback(char *topic, byte *payload, unsigned int length) {
	payload[length] = '\0';
	String _topic = String(topic);
	String response = String((char *)payload);
	// debug
	// Serial.print("payload");
	// Serial.println(_message);
	//   for (int i = 0; i < length; i++) {
	//     Serial.write((char)payload[i]);
	// }
	// Serial.println();
	if(_topic.equals(UPDATE_CANCEL_TOPIC) == 1 && response.equals(CANCEL_MSG)) {
		if(Update.isRunning()) {
			Serial.println("Update cancelled");
			Update.abort();
			mqtt.publish(UPDATE_RESPONSE_TOPIC, CANCEL_OK_RES);
		} else {
			mqtt.publish(UPDATE_RESPONSE_TOPIC, CANCEL_FAILED_RES);
		}
	}

	if (_topic.equals(UPDATE_START_TOPIC) == 1 && response.equals(START_MSG)) {
		if (Update.begin(UPDATE_SIZE_UNKNOWN)) {
			Serial.println("Update Start");
			start_time = millis();
			total_len = 0;
			mqtt.publish(UPDATE_RESPONSE_TOPIC, START_OK_RES);
		} else {
			mqtt.publish(UPDATE_RESPONSE_TOPIC, START_FAILED_RES);
		}
	}

	if (_topic.equals(UPDATE_WRITE_TOPIC) == 1) {
		if (Update.write(payload, length)) {
			total_len += length;
			Serial.print("Write [ ");
			Serial.print(total_len);
			Serial.println(" ]");
			mqtt.publish(UPDATE_RESPONSE_TOPIC, WRITE_OK_RES);
		} else {
			mqtt.publish(UPDATE_RESPONSE_TOPIC, WRITE_FAILED_RES);
		}
	}

	if (_topic.equals(UPDATE_STOP_TOPIC) == 1 && response.equals(STOP_MSG)) {
		if (Update.end(true)) {
			Serial.println("Update Done ");
			stop_time = millis();
			Serial.print("total Time to update : [ ");
			Serial.print((stop_time - start_time) / 1000);
			Serial.println(" ] ");
			mqtt.publish(UPDATE_RESPONSE_TOPIC, STOP_OK_RES);
			update_done = true;
		} else {
			mqtt.publish(UPDATE_RESPONSE_TOPIC, STOP_FAILED_RES);
		}
	}
}

void connect_to_wifi() {
	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(WIFI_SSID);
	WiFi.begin(WIFI_SSID, WIFI_PASS);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void reconnect_mqtt_server() {
	while (!mqtt.connected()) {
		Serial.print("Attempting MQTT connection...");
		if (mqtt.connect("ESP32Client")) {
			Serial.println("connected");
			mqtt.subscribe(UPDATE_START_TOPIC);
			mqtt.subscribe(UPDATE_WRITE_TOPIC);
			mqtt.subscribe(UPDATE_STOP_TOPIC);
			mqtt.subscribe(UPDATE_RESPONSE_TOPIC);
			mqtt.subscribe(UPDATE_CANCEL_TOPIC);
		} else {
			Serial.print("failed, rc=");
			Serial.print(mqtt.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void setup_app() {
	Serial.begin(115200);
    connect_to_wifi();
}

static void mqtt_ota_loop(void *parameter) {
	mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setBufferSize(4096);
    mqtt.setCallback(mqtt_callback);
	mqtt.setKeepAlive(60);

	while (1) {
		if (!mqtt.connected()) {
			reconnect_mqtt_server();
		}
		mqtt.loop();
		if (update_done) {
			Serial.println("Rebooting after 5 second....");
			delay(5000);
			ESP.restart();
		}
		vTaskDelay(pdMS_TO_TICKS(3));
	}

}

void start_mqtt_ota_task() {
	xTaskCreatePinnedToCore(mqtt_ota_loop, "mqtt_ota_task", 1024 * 8, NULL, 10, NULL, 1);
}
