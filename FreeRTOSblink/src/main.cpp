#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const int app_cpu = 1;
#endif

static const int led_pin = GPIO_NUM_1;


void toggleLED(void *parameter){
  while(1){
    digitalWrite(led_pin,HIGH);
    vTaskDelay(500/ portTICK_PERIOD_MS);
    digitalWrite(led_pin,LOW);
    vTaskDelay(500/ portTICK_PERIOD_MS);
  }
}

void toggleLEDfast(void *parameter){
  while(1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void setup() {
  pinMode(led_pin, OUTPUT);

  xTaskCreatePinnedToCore(
    toggleLED,
    "Blin LED 1s",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );

  xTaskCreatePinnedToCore(
    toggleLEDfast,
    "Blink LED 400ms",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );

}

void loop() {
  // put your main code here, to run repeatedly:
}
