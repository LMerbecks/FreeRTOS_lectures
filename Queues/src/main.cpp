#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE 
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint8_t queue_length = 5;

static QueueHandle_t message_queue;

void printMessage(void *parameter){
  int item = 0;
  while(1) {
    if(xQueueReceive(message_queue, (void *) &item, 0) == pdTRUE) {
    }
    Serial.println(item);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void setupSerial(){
  Serial.begin(115200);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  Serial.println("Queue demo");
}

void setup() {
  // put your setup code here, to run once:
  setupSerial();
  message_queue = xQueueCreate(queue_length, sizeof(int));
  xTaskCreatePinnedToCore(
    printMessage,
    "print messages",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );

}

void loop() {
  // put your main code here, to run repeatedly:
  static int number = 0;
  if(xQueueSend(message_queue, (void*) &number, 10) != pdTRUE) {
    Serial.println("Queue full");
  }
  vTaskDelay(2000/portTICK_PERIOD_MS);
  number++;
}