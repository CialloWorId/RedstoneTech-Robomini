#include "WiFi.h"
#include "ESPmDNS.h"
#include "src/config.h"
#include "src/task.h"

void setup()
{
  // put your setup code here, to run once:
  pinMode(PIN_LED, OUTPUT);

  Serial.begin(BAUD0);
  Serial1.begin(BAUD1, SERIAL_8N1, RX1, TX1);
  Serial.printf("[setup] UART1绑定引脚: RX = %d, TX = %d, 波特率: %d\n", RX1, TX1, BAUD1);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(SSID, PASS);

  if ((ClientMutex = xSemaphoreCreateMutex()) != NULL)
  {
    Serial.printf("[setup] 互斥信号量创建成功\n");
  }
  else
  {
    Serial.printf("[setup] 互斥信号量创建失败\n");
    delay(10000);
    ESP.restart();
  }
  if (xTaskCreatePinnedToCore(Task_TaskCreate, "TaskCreate", 8192, NULL, 7, NULL, 1) != pdPASS)
  {
    Serial.printf("[setup] 任务TaskCreate创建失败\n");
    delay(10000);
    ESP.restart();
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  vTaskDelete(NULL);
}