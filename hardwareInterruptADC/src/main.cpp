#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const size_t str_buffer_size = 255;
static const size_t command_size = 4;
static const char average_command[command_size] = "avg";


// create hardware timer that samples from ADC in an ISR.
// Samples shall be placed in a buffer. Once ten samples
// have been created wake up task a and compute average.
// Store average in float. This is not atomic! Task b shall
// echo the serial input and print the average if avg\n is
// the input.


void serialInterface(void *parameter){
  char current_char;
  char str_buffer[str_buffer_size];
  size_t str_buffer_index = 0;
  memset(str_buffer, 0, str_buffer_size);
  while(1){
    if(Serial.available()>0){
      current_char = Serial.read();
      Serial.print(current_char);
      if(current_char == '\n' || current_char == '\r') {
        str_buffer[str_buffer_index] = '\0';
        if(strcmp(str_buffer, average_command)==0){
          Serial.print("Average: ");
          Serial.println();
        }
        memset(str_buffer, 0, str_buffer_size);
        str_buffer_index = 0;
      }else{
        if(str_buffer_index < str_buffer_size){
          str_buffer[str_buffer_index] = current_char;
          str_buffer_index++;
        }
      }
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vTaskDelay(3000/portTICK_PERIOD_MS);
  Serial.println("ADC with hardware timer");

  xTaskCreatePinnedToCore(
    serialInterface, 
    "Serial Interface",
    1500,
    NULL,
    1,
    NULL,
    app_cpu
  );
}

void loop() {
  // put your main code here, to run repeatedly:
}
