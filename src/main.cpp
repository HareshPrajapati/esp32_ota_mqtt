#include "defines.h"

void setup() {
    setup_app();
    start_mqtt_ota_task();
}

void loop() {
    if(toggle_led) {
        digitalWrite(5,!digitalRead(5));
        delay(led_delay);
    }
    // Serial.println("Hello");
    // delay(1000);
}
