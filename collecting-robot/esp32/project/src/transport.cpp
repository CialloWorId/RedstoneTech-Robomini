#include "Arduino.h"
#include "net.h"

#define BUF_SIZE 512

void Task_SerialtoTCP(void *argument)
{
    uint8_t buf[BUF_SIZE];

    for (;;)
    {
        if (!tcp_connected)
        {
            // tcp未连接时清空串口缓冲区
            while (Serial1.available() > 0)
            {
                Serial1.read(); 
            }

            // 降低轮询频率，等待tcp连接
            vTaskDelay(pdMS_TO_TICKS(50));
            continue; 
        }

        int available = Serial1.available();
        if (available > 0)
        {
            // 限制单次最大读取量，防止数组越界
            int read_len = std::min(available, BUF_SIZE);
            int len = Serial1.read(buf, read_len);

            if (len > 0)
            {
                if (xSemaphoreTake(ClientMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    // 再次确认连接
                    if (client.connected())
                    {
                        client.write(buf, len);
                    }
                    xSemaphoreGive(ClientMutex);
                }
            }
        }
        else
        {
            // 没数据时让出 CPU
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

void Task_TCPtoSerial(void *argument)
{
    uint8_t buf[BUF_SIZE];

    for (;;)
    {
        // tcp未连接时轮询等待
        if (!tcp_connected)
        {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // 从网络读数据前先看串口缓冲区剩余空间
        int space = Serial1.availableForWrite();
        
        // 串口缓冲区满了就跳过本轮
        if (space <= 0) 
        {
            vTaskDelay(pdMS_TO_TICKS(5));
            continue;
        }

        // 限制单次最大读取量，防止数组越界
        int read_len = std::min(space, BUF_SIZE);
        int len = 0;

        if (xSemaphoreTake(ClientMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            if (client.connected() && client.available() > 0)
            {
                len = client.read(buf, read_len);
            }
            xSemaphoreGive(ClientMutex);
        }

        // 限制后数据长度小于缓冲区长度，可以直接写
        if (len > 0)
        {
            Serial1.write(buf, len);
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}