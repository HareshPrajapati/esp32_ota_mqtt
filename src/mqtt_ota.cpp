#include "defines.h"

WiFiClient client;
PubSubClient mqtt(client);
bool update_done = false;
uint32_t start_time = 0, stop_time = 0;
static uint32_t total_len = 0;

uint32_t led_delay = 0;
static int led_brightness = 0;
static uint32_t rgb_value = 0;
bool toggle_led = false;

const int ledChannel = 0; 			// LEDC channel (0-15)
const int freq = 5000; 				// Frequency in Hertz
const int resolution = 8; 			// Resolution (1-16 bits)



CRGB leds[NUM_LEDS];

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
	if (_topic.equals(RGB_LED_TOPIC) == 1 && (response.indexOf("RGB") >= 0)) {
		int startIndex = response.indexOf(' ');
		String numericValue = response.substring(startIndex);
		// Serial.print("numericValue ");
		// Serial.println(numericValue);
		rgb_value = atoll((char *)numericValue.c_str());
		Serial.print("rgb_value is: ");
		Serial.println(rgb_value);
		unsigned int red = (rgb_value >> 16) & 0xFF;
    	unsigned int green = (rgb_value >> 8) & 0xFF;
    	unsigned int blue = (rgb_value >> 0) & 0xFF;
		CRGB color;
		color.r = red;
		color.g = green;
		color.b = blue;
		leds[0] = color;
  		FastLED.show();
		mqtt.publish(UPDATE_RESPONSE_TOPIC, "RGB OK");
	}
	if (_topic.equals(LED_DATA_TOPIC) == 1 && response.equals("ON")) {
		Serial.println("LED ON");
		mqtt.publish(UPDATE_RESPONSE_TOPIC, "LED ON OK");
		toggle_led = false;
		led_brightness = 0;
		pinMode(5,OUTPUT);
		digitalWrite(5,HIGH);
	}

	if (_topic.equals(LED_DATA_TOPIC) == 1 && response.equals("OFF")) {
		Serial.println("LED OFF");
		mqtt.publish(UPDATE_RESPONSE_TOPIC, "LED OFF OK");
		toggle_led = false;
		led_brightness = 0;
		pinMode(5,OUTPUT);
		digitalWrite(5,LOW);
	}

	if (_topic.equals(LED_DATA_TOPIC) == 1 && (response.indexOf("DELAY") >= 0)) {
		int startIndex = response.indexOf(' ');
		String numericValue = response.substring(startIndex);
		led_delay = numericValue.toInt();
		Serial.print("Delay is: ");
		Serial.println(led_delay);
		pinMode(5,OUTPUT);
		led_brightness = 0;
		if(led_delay > 0) {
			mqtt.publish(UPDATE_RESPONSE_TOPIC, "LED BLINK OK");
			toggle_led = true;
		}
	}

	if (_topic.equals(LED_DATA_TOPIC) == 1 && (response.indexOf("BRIGHTNESS") >= 0)) {
		int startIndex = response.indexOf(' ');
		String numericValue = response.substring(startIndex);
		led_brightness = numericValue.toInt();
		Serial.print("led_brightness is: ");
		Serial.println(led_brightness);
		ledcSetup(ledChannel, freq, resolution);
    	ledcAttachPin(5, ledChannel);
		if(led_brightness >= 0) {
			mqtt.publish(UPDATE_RESPONSE_TOPIC, "LED FADING OK");
			toggle_led = false;
			ledcWrite(ledChannel, led_brightness);
		}
	}

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
		Update.abort();
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
			mqtt.subscribe(LED_DATA_TOPIC);
			mqtt.subscribe(RGB_LED_TOPIC);
		} else {
			Serial.print("failed, rc=");
			Serial.print(mqtt.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}
// 


void setup_app() {
	Serial.begin(115200);
	pinMode(5,OUTPUT);
	ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(5, ledChannel);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
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
