#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint8_t led_pin = GPIO_NUM_23;

static TimerHandle_t backlight_timer = NULL;
static const int backlight_timeout = 2000;

void echoSerialLCD(void *parameter){
  char current_char = '\0';
  while(1){
    if(Serial.available()>0){
      current_char = Serial.read();
      Serial.print(current_char);
      digitalWrite(led_pin, HIGH);
      xTimerStart(backlight_timer, portMAX_DELAY);
    }
  }
}

void LCDbacklightDelay(TimerHandle_t timer){
  digitalWrite(led_pin,LOW);
}

void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(115200);
  vTaskDelay(3000/portTICK_PERIOD_MS);
  Serial.println("LCD backlight demo");

  xTaskCreatePinnedToCore(
    echoSerialLCD,
    "LCD char echo",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );

  backlight_timer = xTimerCreate(
    "LCD fadeout",
    backlight_timeout / portTICK_PERIOD_MS,
    pdFALSE,
    (void*) 0,
    LCDbacklightDelay
  );

  if(backlight_timer == NULL){
    Serial.println("Could not create timer");
  }else{
    xTimerStart(backlight_timer, portMAX_DELAY);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
