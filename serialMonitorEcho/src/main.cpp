#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const int buffer_length = 255;
static volatile bool newMessage = false;
static char* transfer_array = NULL;

// objective: create two tasks that pass messages over heap
// Task A listens for input from serial monitor, on newline
// writes all chars in heap, notifies B of message
// Task B waits for notification from Task A, prints message
// in heap memory to serial monitor, frees the memory

void serialListener(void *parameter){
  char buffer[buffer_length];
  char current_char;
  size_t received_chars = 0;

  memset(buffer, 0, buffer_length);

  while(1){
    if(Serial.available() > 0) {
      current_char = Serial.read();

      if(received_chars < buffer_length){
        buffer[received_chars] = current_char;
        received_chars++;
      }

      if(current_char == '\n') {
        buffer[received_chars - 1] = '\0';
        if(newMessage == false){
          transfer_array = (char*) pvPortMalloc(received_chars * sizeof(char));
          
          configASSERT(transfer_array);
          memcpy(transfer_array, buffer, received_chars);
          newMessage = true;
        }

        memset(buffer, 0, buffer_length);
        received_chars = 0;
      }
    }
  }
}

void serialPrinter(void *parameter){
  while(1){
    if(newMessage){
      Serial.println(transfer_array);
      vPortFree(transfer_array);
      transfer_array = NULL;
      newMessage = false;
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  xTaskCreatePinnedToCore(
    serialListener,
    "Serial Listener",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );
  xTaskCreatePinnedToCore(
    serialPrinter,
    "Serial Printer",
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
