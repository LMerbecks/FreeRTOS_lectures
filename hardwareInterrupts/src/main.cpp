#include <Arduino.h>

static const uint16_t time_divider = 80; // timer runs at 1MHz now
static const uint64_t time_period = 1000000; 

static const int led_pin = GPIO_NUM_23;

static hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer(){
  int LED_state = digitalRead(led_pin);
  digitalWrite(led_pin, !LED_state);
}

void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(1152000);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println("Timer interrupt demo");
  
  timer = timerBegin(0, time_divider, true);
  

  timerAttachInterrupt(timer, &onTimer, true);

  timerAlarmWrite(timer, time_period, true);

  timerAlarmEnable(timer);
}

void loop() {
  // put your main code here, to run repeatedly:
}
