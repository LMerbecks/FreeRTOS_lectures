#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif


const char phrase[] = "I am running around the world!";

void printSentence(void *parameter) {
  size_t phrase_length = strlen(phrase);
  while(1) {
    for(int i = 0; i<phrase_length; i++){
      Serial.print(phrase[i]);
    }
    Serial.println();
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void printAsterix(void *parameter) {
  while(1) {
    Serial.print('*');
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

static TaskHandle_t sentencePrinter = NULL;
static TaskHandle_t asterixPrinter = NULL;

void setup() {
  Serial.begin(300);
  
  vTaskDelay(3000/portTICK_PERIOD_MS);

  Serial.print("Running on core ");
  Serial.print(xPortGetCoreID());
  Serial.print(" at priority of ");
  Serial.println(uxTaskPriorityGet(NULL));
  
  xTaskCreatePinnedToCore(
    printSentence,
    "Print the phrase",
    1024,
    NULL,
    1,
    &sentencePrinter,
    app_cpu
  );

  xTaskCreatePinnedToCore(
    printAsterix,
    "Print an asterix",
    1024,
    NULL,
    2,
    &asterixPrinter,
    app_cpu
  );

}

void loop() {
  for(int i = 0; i<4; i++){
    vTaskSuspend(asterixPrinter);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    vTaskResume(asterixPrinter);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
  if(sentencePrinter != NULL) {
    vTaskDelete(sentencePrinter);
    sentencePrinter = NULL;
  }
}

