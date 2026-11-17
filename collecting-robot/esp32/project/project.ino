#include "src/init.h"

void setup()
{
  // 创建优先级高于所有任务的初始化任务，防止任务创建过程中被打断
  if (xTaskCreatePinnedToCore(Task_Init, "Init", 8192, NULL, 7, NULL, 1) != pdPASS)
  {
    // 创建失败自动重启
    Serial.printf("[setup] 任务TaskCreate创建失败, 10s后自动重启\n");
    delay(10000);
    ESP.restart();
  }
}

void loop()
{
  vTaskDelete(NULL);
}