#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

void remainingStack(){
  Serial.print("Watermark of Stack (byte): ");
  Serial.println(4*uxTaskGetStackHighWaterMark(NULL));
}



void memoryDestroyer(void *parameter){
  while (1)
  {
    int a_variable = 1;
    int an_array[100];
    for(size_t i = 0; i < 100; i++){
      an_array[i] = a_variable++ + i;
    }
    Serial.println(an_array[0]);
    remainingStack();
    Serial.print("Free Heap before malloc (bytes) ");
    Serial.println(xPortGetFreeHeapSize());

    int *pointer = (int*) pvPortMalloc(1024 * sizeof(int));

    if(pointer == NULL) {
      Serial.println("Not enough heap");
    }else{
      for(size_t i = 0; i < 1024; i++) {
        pointer[i] = 3;
      }
    }

    Serial.print("Free heap after malloc (bytes) ");
    Serial.println(xPortGetFreeHeapSize());

    vPortFree(pointer);

    vTaskDelay(10/portTICK_PERIOD_MS);
  }

}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  xTaskCreatePinnedToCore(
    memoryDestroyer,
    "A Task that is memory menace",
    1500,
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
