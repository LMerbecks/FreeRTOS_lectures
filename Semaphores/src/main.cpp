#include <Arduino.h>
#include <Arduino.h>
/**
 * FreeRTOS Mutex Challenge
 * 
 * Pass a parameter to a task using a mutex.
 * 
 * Date: January 20, 2021
 * Author: Shawn Hymel
 * License: 0BSD
 */

// You'll likely need this on vanilla FreeRTOS
//#include <semphr.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const int num_tasks = 5;

typedef struct Message {
  char body[30];
  uint8_t length;
} Message;

static SemaphoreHandle_t counting_semaphore; // use this to stop setup to free stack variable before it is copied by tasks
static SemaphoreHandle_t mutex; // mutex for the serial print function;

void printTask(void *parameter){
  
  Message local_message = *(Message *) parameter;
  xSemaphoreTake(counting_semaphore, 0);
  if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE){
    Serial.print("Received: ");
    Serial.print(local_message.body);
    Serial.print(" length: ");
    Serial.println(local_message.length);
    xSemaphoreGive(mutex);
  }

  vTaskDelay(1000/portTICK_PERIOD_MS);
  vTaskDelete(NULL);
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {
  Serial.begin(115200);
  vTaskDelay(3000/portTICK_PERIOD_MS);

  char task_name[] = "";

  char text[] = "I am an important string";
  Message parent_message;
  strcpy(parent_message.body, text);
  parent_message.length = strlen(text);

  counting_semaphore = xSemaphoreCreateCounting(num_tasks, 0);
  mutex = xSemaphoreCreateMutex();

  for(int i = 0; i < num_tasks; i++) {
    sprintf(task_name, "Task_%i", i);
    xTaskCreatePinnedToCore(printTask,
                            task_name,
                            1024,
                            (void *)&parent_message,
                            1,
                            NULL,
                            app_cpu);


  }

  // Start task 1

  vTaskDelay(portTICK_PERIOD_MS); // wait a single tick to make sure the other task actually gets the mutex.
  for(int i = 0; i < num_tasks; i++){
    xSemaphoreTake(counting_semaphore,portMAX_DELAY); // wait for all five tasks by taking the semaphore.
  }
  // Show that we accomplished our task of passing the stack-based argument
  Serial.println("All tasks done!");
}

void loop() {
  
  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}