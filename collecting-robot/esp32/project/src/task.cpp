#include "Arduino.h"
#include "WiFi.h"
#include "ESPmDNS.h"

SemaphoreHandle_t ClientMutex;

void Task_LED(void *argument)
{
    for(;;)
    {

    }
}

void Task_NetManager(void *argument)
{
    for(;;)
    {
        
    }
}

void Task_SerialtoTCP(void *argument)
{
    for(;;)
    {
        
    }
}

void Task_TCPtoSerial(void *argument)
{
    for(;;)
    {
        
    }
}

void TaskCreateChecked(TaskFunction_t taskCode, const char *name, uint32_t stackDepth, UBaseType_t priority, BaseType_t core)
{
    BaseType_t result = xTaskCreatePinnedToCore(taskCode, name, stackDepth, NULL, priority, NULL, core);
    if (result == pdPASS)
    {
        Serial.printf("[TaskCreate] 任务%s创建成功\n", name);
    }
    else
    {
        Serial.printf("[TaskCreate] 任务%s创建失败\n", name);
        delay(10000);
        ESP.restart();
    }
}

void Task_TaskCreate(void *argument)
{
    Serial.printf("[setup] 任务TaskCreate创建成功\n");
    TaskCreateChecked(Task_LED, "LED", 2048, 1, 1);
    TaskCreateChecked(Task_NetManager, "NetManager", 4096, 5, 1);
    TaskCreateChecked(Task_SerialtoTCP, "SerialtoTCP", 4096, 3, 1);
    TaskCreateChecked(Task_TCPtoSerial, "TCPtoSerial", 4096, 3, 1);
    vTaskDelete(NULL);
}