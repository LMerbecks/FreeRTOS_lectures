#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint16_t time_divider = 80; // timer runs at 10MHz now
static const uint64_t time_period = 1000000; 
static const TickType_t task_delay = 1000 / portTICK_PERIOD_MS;

static const int analog_pin = A6;

static hw_timer_t *timer = NULL;
static volatile uint16_t analog_val;
static SemaphoreHandle_t analog_read_semaphore = NULL;

void IRAM_ATTR onTimer(){
  BaseType_t task_woken = pdFALSE;
  analog_val = analogRead(analog_pin);

  xSemaphoreGiveFromISR(analog_read_semaphore, &task_woken);

  if(task_woken){
    portYIELD_FROM_ISR();
  }
}

void printValue(void *parameter){
  while(1){
    xSemaphoreTake(analog_read_semaphore, portMAX_DELAY);
    Serial.println(analog_val);
  }

}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println("Timer interrupt demo");

  analog_read_semaphore = xSemaphoreCreateBinary();

  if(analog_read_semaphore == NULL){
    Serial.println("Could not initialize semaphore");
    ESP.restart();
  }

  xTaskCreatePinnedToCore(
    printValue,
    "Print values",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );

  timer = timerBegin(0, time_divider, true);
  if(timer == NULL){
    Serial.println("Could not create timer");
  }

  timerAttachInterrupt(timer, &onTimer, true);

  timerAlarmWrite(timer, time_period, true);

  timerAlarmEnable(timer);
}

void loop() {
  // put your main code here, to run repeatedly:
}
