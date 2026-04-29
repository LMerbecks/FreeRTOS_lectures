#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static int shared_counter = 0;

void incrementCounter(void *parameter){
  int local_counter;
  while(1) {
    local_counter = shared_counter;
    local_counter++;
    vTaskDelay(random(100,400)/portTICK_PERIOD_MS);
    shared_counter = local_counter;

    Serial.println(shared_counter);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);

  Serial.println("Race condition demo");

  xTaskCreatePinnedToCore(
    incrementCounter,
    "Task 1",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );

  xTaskCreatePinnedToCore(
    incrementCounter,
    "Task 2",
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

