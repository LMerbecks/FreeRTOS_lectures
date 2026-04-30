#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint16_t time_divider = 8; // timer runs at 10MHz now
static const uint64_t time_period = 1000000; 
static const TickType_t task_delay = 1000 / portTICK_PERIOD_MS;

static hw_timer_t *timer = NULL;
static volatile int isr_counter = 0;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer(){
  portENTER_CRITICAL_ISR(&spinlock);
  isr_counter++;
  portEXIT_CRITICAL_ISR(&spinlock);
}

void printValue(void *parameter){
  while(1){
    while(isr_counter > 0){
      Serial.println(isr_counter);
      portENTER_CRITICAL(&spinlock);
      isr_counter--;
      portEXIT_CRITICAL(&spinlock);
    }
    vTaskDelay(task_delay);
  }

}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println("Timer interrupt demo");

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
