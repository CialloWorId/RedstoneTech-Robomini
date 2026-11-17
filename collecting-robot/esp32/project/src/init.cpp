#include "Arduino.h"
#include "WiFi.h"
#include "config.h"
#include "net.h"
#include "transport.h"

void TaskCreateChecked(TaskFunction_t taskCode, const char *name, uint32_t stackDepth,
                       UBaseType_t priority, TaskHandle_t *handle, BaseType_t core)
{
    BaseType_t result = xTaskCreatePinnedToCore(taskCode, name, stackDepth, NULL, priority, handle, core);

    // 判断任务是否创建成功
    if (result == pdPASS)
    {
        Serial.printf("[Init] 任务%s创建成功\n", name);
    }
    else
    {
        // 创建失败自动重启
        Serial.printf("[Init] 任务%s创建失败, 10s后自动重启\n", name);
        delay(10000);
        ESP.restart();
    }
}

void Task_Init(void *argument)
{
    Serial.printf("[Init] 任务Init创建成功\n");

    pinMode(PIN_LED, OUTPUT);

    Serial.begin(BAUD0);
    Serial1.setRxBufferSize(1024);  // 设置 uart1 缓冲区为1024字节
    Serial1.begin(BAUD1, SERIAL_8N1, RX1, TX1);
    Serial.printf("[Init] UART1绑定引脚: RX = %d, TX = %d, 波特率: %d\n", RX1, TX1, BAUD1);

    WiFi.persistent(false);      // 不把 SSID/密码写进 flash，减少磨损
    WiFi.mode(WIFI_STA);         // 设置WiFi模式：连路由器
    WiFi.setSleep(false);        // 关闭睡眠减少延迟
    WiFi.setAutoReconnect(true); // 掉线后自动重连
    WiFi.begin(SSID, PASS);

    if ((ClientMutex = xSemaphoreCreateMutex()) != NULL)
    {
        Serial.printf("[Init] 互斥信号量创建成功\n");
    }
    else
    {
        Serial.printf("[Init] 互斥信号量创建失败, 10s后自动重启\n");
        delay(10000);
        ESP.restart();
    }

    // 创建FreeRTOS任务
    TaskCreateChecked(Task_LED, "LED", 2048, 1, &TaskHandle_LED, 1);
    TaskCreateChecked(Task_NetManager, "NetManager", 4096, 5, NULL, 1);
    TaskCreateChecked(Task_SerialtoTCP, "SerialtoTCP", 4096, 3, NULL, 1);
    TaskCreateChecked(Task_TCPtoSerial, "TCPtoSerial", 4096, 3, NULL, 1);

    vTaskDelete(NULL);
}