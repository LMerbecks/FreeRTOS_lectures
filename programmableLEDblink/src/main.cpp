#include <Arduino.h>
#include <stdlib.h>

#if CONFIG_FREERTOS_UNICORE 
static const BaseType_t app_cpu = 0;
#else 
static const BaseType_t app_cpu = 1;
#endif

static const int buffer_len = 30;
static int led_pin = GPIO_NUM_23;

int blink_period_ms = 1000;
int duty_cycle_percent = 50;


void blinkLED(void *parameter){
  while(1){
    digitalWrite(led_pin, HIGH);
    vTaskDelay(blink_period_ms/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(blink_period_ms/portTICK_PERIOD_MS);
  }
}

void getBlinkPeriod(void *parameter){
  char buffer[buffer_len];
  char current_char;
  size_t buffer_index = 0;
  
  memset(buffer,0,buffer_len);

  Serial.println("Enter new led period");
  while(1) {
    if(Serial.available() > 0){
      current_char = Serial.read();
      if(current_char == '\n') {
        blink_period_ms = atoi(buffer);
        Serial.print("Blink period set to ");
        Serial.print(blink_period_ms);

        memset(buffer, 0, buffer_len);
        buffer_index = 0;
      }
      else{
        if(buffer_index < buffer_len) {
          buffer[buffer_index] = current_char;
          buffer_index++;
        }
      }
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(led_pin, OUTPUT);

  vTaskDelay(3000/portTICK_PERIOD_MS);
  Serial.println("LED blink period Demo");

  xTaskCreatePinnedToCore(
    blinkLED,
    "Blink the LED",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );
  xTaskCreatePinnedToCore(
    getBlinkPeriod,
    "Blink period CLI",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );

  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
