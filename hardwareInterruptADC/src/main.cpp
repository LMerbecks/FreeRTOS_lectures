#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const size_t str_buffer_size = 255;
static const size_t command_size = 4;
static const char average_command[command_size] = "avg";

static const int input_pin = A6;

static const uint16_t timer_divider = 8; // tick at 10MHz
static const uint64_t timer_period = 1000000;
static hw_timer_t *adc_timer = NULL;

static volatile float average = 0.0;

static const size_t BUFFER_LENGTH = 10;
static volatile uint16_t buffer_0[BUFFER_LENGTH];
static volatile uint16_t buffer_1[BUFFER_LENGTH];
static volatile uint16_t *read_from = buffer_0;
static volatile uint16_t *write_to = buffer_1;
static volatile uint8_t buffer_overrun = 0; 

static SemaphoreHandle_t reading_done_semaphore = NULL;
static TaskHandle_t processing_task = NULL;

static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
static QueueHandle_t error_queue;
static const uint16_t error_queue_length = 20;

// create hardware timer that samples from ADC in an ISR.
// Samples shall be placed in a buffer. Once ten samples
// have been created wake up task a and compute average.
// Store average in float. This is not atomic! Task b shall
// echo the serial input and print the average if avg\n is
// the input.

void swap_buffer(){
  volatile uint16_t *temp_ptr = write_to;
  write_to = read_from;
  read_from = write_to;
}

void IRAM_ATTR readADC(){
  static uint16_t index = 0; // only initialize this once (static)
  BaseType_t task_woken = pdFALSE;

  if((index < BUFFER_LENGTH) && (buffer_overrun == 0)){ // if the buffer is not full and not overrun
    write_to[index] = analogRead(input_pin);
    index++;
  }
  if(index >= BUFFER_LENGTH){
    if(xSemaphoreTakeFromISR(reading_done_semaphore, &task_woken) == pdFALSE){ // if reading the buffer is not done
      buffer_overrun = 1; // the buffer is still read and we cannot add more to the buffer
    }

    if(buffer_overrun == 0) {
      index = 0; // reset the index
      swap_buffer(); // swap read and write buffer

      vTaskNotifyGiveFromISR(processing_task, &task_woken); //notify the processing task, that data can be read (and processed)
    }

  }

  if(task_woken){
    portYIELD_FROM_ISR();
  }

}


void serialInterface(void *parameter){
  char current_char;
  char str_buffer[str_buffer_size];
  size_t str_buffer_index = 0;

  memset(str_buffer, 0, str_buffer_size);
  char message[str_buffer_size];
  while(1){
    if(xQueueReceive(error_queue, (void *)&message, 0) == pdTRUE) // if there was an error 
    {
      Serial.println(message);
    }

    if(Serial.available()>0){
      current_char = Serial.read();

      if(str_buffer_index < str_buffer_size-1){
          str_buffer[str_buffer_index] = current_char;
          str_buffer_index++;
      }

      if(current_char == '\n' || current_char == '\r') {
        Serial.print("\r\n");
        str_buffer[str_buffer_index-1] = '\0';

        if(strcmp(str_buffer, average_command)==0){
          Serial.print("Average: ");
          Serial.println(average);
        }

        memset(str_buffer, 0, str_buffer_size);
        str_buffer_index = 0;
      }else{
        Serial.print(current_char);
      }
    }
    vTaskDelay(10);
  }
}

void calculateAverage(void *parameter){
  char message[str_buffer_size];
  float local_average;
  while(1){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // wait until we are notified from the ISR that 10 samples are ready
    local_average = 0.0;
    for(int index = 0; index < BUFFER_LENGTH; index++){
      local_average += (float) read_from[index];
    }
    local_average /= BUFFER_LENGTH;

    // the float transfer might take more than an operation
    // i.e. the ISR could interrupt us here so we make it a
    // critical section
    portENTER_CRITICAL(&spinlock); 
    average = local_average;
    portEXIT_CRITICAL(&spinlock);

    if(buffer_overrun == 1) // we took to long to process the samples
    {
      strcpy(message, "Error: processing took to long, dropped samples");
      xQueueSend(error_queue, (void*) &message, 10);
    }
    portENTER_CRITICAL(&spinlock);
    buffer_overrun = 0;
    xSemaphoreGive(reading_done_semaphore);
    portEXIT_CRITICAL(&spinlock);
  }
}

void setup() {
  pinMode(input_pin, ANALOG);
  // put your setup code here, to run once:
  Serial.begin(115200);
  vTaskDelay(3000/portTICK_PERIOD_MS);
  Serial.println("ADC with hardware timer");

  reading_done_semaphore = xSemaphoreCreateBinary();

  if(reading_done_semaphore == NULL){
    Serial.println("Error creating semaphore");
    ESP.restart();
  }

  xSemaphoreGive(reading_done_semaphore);

  error_queue = xQueueCreate(error_queue_length, str_buffer_size * sizeof(char));

  xTaskCreatePinnedToCore(
    serialInterface, 
    "Serial Interface",
    1500,
    NULL,
    2,
    NULL,
    app_cpu
  );

  xTaskCreatePinnedToCore(
    calculateAverage, 
    "Calculate average",
    1500,
    NULL,
    1,
    &processing_task,
    app_cpu
  );

  adc_timer = timerBegin(0, timer_divider, true);
  timerAttachInterrupt(adc_timer, &readADC, true);
  timerAlarmWrite(adc_timer, timer_period, true);
  timerAlarmEnable(adc_timer);

  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
