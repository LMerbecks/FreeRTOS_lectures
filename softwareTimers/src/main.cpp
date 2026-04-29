#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static TimerHandle_t one_shot_timer = NULL; 
static TimerHandle_t auto_reload_timer = NULL; 

void timerCallback(TimerHandle_t timer){
  if((int)pvTimerGetTimerID(timer) == 0){
    Serial.println("One shot timer expired");
  }
  if((int)pvTimerGetTimerID(timer) == 1){
    Serial.println("Auto reload timer expired");
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(3000/portTICK_PERIOD_MS);
  Serial.println("Software timer demo");

  one_shot_timer = xTimerCreate(
    "One shot timer",
    2000 / portTICK_PERIOD_MS,
    pdFALSE,
    (void*)0,
    timerCallback
  );

  auto_reload_timer = xTimerCreate(
    "Auto reload timer",
    1000 / portTICK_PERIOD_MS,
    pdTRUE,
    (void*)1,
    timerCallback
  );

  vTaskDelay(1000/portTICK_PERIOD_MS);
  if(one_shot_timer == NULL || auto_reload_timer == NULL) {
    Serial.println("Could not create timer");
  }else{
    Serial.println("Starting timer");
    xTimerStart(one_shot_timer, portMAX_DELAY);
    xTimerStart(auto_reload_timer, portMAX_DELAY);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
