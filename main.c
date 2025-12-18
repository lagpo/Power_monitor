#include <Arduino.h>

// --- HARDWARE ---
const int POT_PIN = 34;
const int BUTTON_PIN = 13;
const int RED_LED_PIN = 25;

// --- RTOS OBJECTS ---
QueueHandle_t sensorQueue;
SemaphoreHandle_t emergencySemaphore;
SemaphoreHandle_t serialMutex; // <--- NEW: The Key

// ==========================================
// HELPER: SAFE PRINTING FUNCTION
// ==========================================
void Serial_SafePrint(const char *msg) {
  // Try to take the mutex (wait forever if needed)
  if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
    Serial.println(msg);
    // IMPORTANT: Always give it back!
    xSemaphoreGive(serialMutex);
  }
}

// ==========================================
// INTERRUPT SERVICE ROUTINE
// ==========================================
void IRAM_ATTR ISR_ButtonPress() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(emergencySemaphore, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// ==========================================
// TASK: SAFETY (Priority 2 - HIGH)
// ==========================================
void Task_Safety(void *parameter) {
  while(1) {
    if (xSemaphoreTake(emergencySemaphore, portMAX_DELAY) == pdTRUE) {
      
      // We need to print, so we take the Mutex first
      if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
        Serial.println("\n!!! EMERGENCY STOP TRIGGERED !!!");
        xSemaphoreGive(serialMutex);
      }

      digitalWrite(RED_LED_PIN, HIGH);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      
      if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
        Serial.println("System Resetting...");
        xSemaphoreGive(serialMutex);
      }
      digitalWrite(RED_LED_PIN, LOW);
    }
  }
}

// ==========================================
// TASK: SENSOR (Producer)
// ==========================================
void Task_Sensor(void *parameter) {
  int rawValue;
  while(1) {
    rawValue = analogRead(POT_PIN);
    xQueueSend(sensorQueue, &rawValue, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// ==========================================
// TASK: PROCESS (Consumer)
// ==========================================
void Task_Process(void *parameter) {
  int receivedValue;
  float voltage;
  char buffer[50]; // Buffer for formatting strings

  while(1) {
    if (xQueueReceive(sensorQueue, &receivedValue, portMAX_DELAY) == pdTRUE) {
      voltage = (receivedValue / 4095.0) * 3.3;

      // Format the string first
      sprintf(buffer, "Voltage: %.2f V", voltage);

      // Now print safely using Mutex
      if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
        Serial.println(buffer);
        xSemaphoreGive(serialMutex);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(POT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);

  // 1. Create Objects
  sensorQueue = xQueueCreate(10, sizeof(int));
  emergencySemaphore = xSemaphoreCreateBinary();
  serialMutex = xSemaphoreCreateMutex(); // Create the Mutex

  // 2. Attach Interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ISR_ButtonPress, FALLING);

  // 3. Create Tasks
  xTaskCreate(Task_Safety, "Safety", 2048, NULL, 2, NULL);
  xTaskCreate(Task_Sensor, "Sensor", 2048, NULL, 1, NULL);
  xTaskCreate(Task_Process, "Process", 2048, NULL, 1, NULL);
}

void loop() {}
