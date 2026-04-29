#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Problem: Create two tasks that communicate with each
// other via two queues. The queues should enable full
// duplex communications.

// Task A: Prints any new messages from Queue 2, Reads
// serial input from user, echos back input to serial
// terminal, on input `delay xxx` sends xxx as number to
// Queue 1

// Task B: Updates time period with any new value from Queue
// 1, Blinks LED with time period delay, sends "Blinked"
// string to Queue2 after 100 times blinked, bonus: also
// send the number of times the LED blinked.

static const int blink_count_trigger = 100;
static const uint8_t led_pin = GPIO_NUM_23;

static const uint8_t command_length = 6;
static const char command[command_length+1] = "delay ";

static const uint8_t delay_queue_length = 10;
static const uint8_t message_queue_length = 10;

static const uint8_t message_length = 255;

static QueueHandle_t delay_queue;
static QueueHandle_t message_queue;

static const uint8_t buffer_length = 255;
static char input_buffer[buffer_length];

void printMessage(const char* message){
  if(xQueueReceive(message_queue, (void *)message, 10)==pdTRUE){
    Serial.println(message);
  }
}

void sendDelay(char* buffer, uint8_t& buffer_index){
  if(memcmp(buffer, command, command_length) == 0){
    char* command_tail = buffer + command_length;
    int led_delay = atoi(command_tail);
    led_delay = abs(led_delay);
    xQueueSend(delay_queue, (void*) &led_delay, 10);
  }
}

void readAndEchoSerial(char& current_char, char* buffer, uint8_t& buffer_index){
  if(Serial.available() > 0){
    current_char = Serial.read();
    if(buffer_index < buffer_length){
      buffer[buffer_index] = current_char;
      buffer_index++;
    }
    if(current_char == '\n'){
      buffer[buffer_index - 1] = '\0';
      sendDelay(buffer, buffer_index);
      memset(buffer, '\0', buffer_length);
      buffer_index = 0;
      Serial.print('\n');
    }
    else 
    {
      Serial.print(current_char);
    }
  }
}


void TaskA(void *parameter){
  char current_char;
  char message[message_length];
  memset(message, '\0', message_length);
  char buffer[buffer_length];
  memset(buffer, '\0', buffer_length);
  uint8_t buffer_index = 0;
  while(1){
    printMessage(message);
    readAndEchoSerial(current_char, buffer, buffer_index);
  }
}

void updateTimePeriod(int &blink_time){
  xQueueReceive(delay_queue, (void*) &blink_time, 0);
}

void blinkLED(int blink_time){
  digitalWrite(led_pin, HIGH);
  vTaskDelay(blink_time/portTICK_PERIOD_MS);
  digitalWrite(led_pin, LOW);
  vTaskDelay(blink_time/portTICK_PERIOD_MS);
}

void sendBlinkMessage(int &blink_count){
  if(blink_count > blink_count_trigger){
    xQueueSend(message_queue, (void *) &"Blinked", 0);
    blink_count = 0;
  }
}

void TaskB(void *parameter){
  int blink_time = 50;
  int blink_count = 0;
  while(1){
    updateTimePeriod(blink_time);
    blinkLED(blink_time);
    blink_count++;
    sendBlinkMessage(blink_count);
  }
}

void setupSerial(){
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println("Two Queues demo");
}

void setupTasks(){
   xTaskCreatePinnedToCore(
    TaskA,
    "Serial interface",
    2048,
    NULL,
    1,
    NULL,
    app_cpu
  );

  xTaskCreatePinnedToCore(
    TaskB,
    "LED interface",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );
}

void setupQueues(){
  delay_queue = xQueueCreate(delay_queue_length, sizeof(int));
  message_queue = xQueueCreate(message_queue_length, message_length * sizeof(char));
}

void setup() {
  pinMode(led_pin, OUTPUT);
  setupSerial();
  setupQueues();
  setupTasks();

  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}