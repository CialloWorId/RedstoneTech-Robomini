#include "Arduino.h"
#include "WiFi.h"
#include "ESPmDNS.h"
#include "config.h"

TaskHandle_t TaskHandle_LED;
SemaphoreHandle_t ClientMutex;
volatile bool wifi_connected = false;
volatile bool tcp_connected = false;
WiFiServer server(TCP_PORT);
WiFiClient client;

void Task_LED(void *argument)
{
    uint16_t interval = 200; // LED 闪烁间隔（ms）

    for (;;)
    {
        if (!wifi_connected)
        {
            // WiFi 未连接：200ms 快速闪烁
            interval = 200;
        }
        else if (!tcp_connected)
        {
            // WiFi 已连接但 TCP 未连接：400ms 慢速闪烁
            interval = 400;
        }
        else
        {
            // WiFi 和 TCP 都已连接：LED 常亮
            digitalWrite(PIN_LED, HIGH);
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            // 收到 NetManager 通知后，立即重新判断状态
            continue;
        }

        // 翻转 LED 并等待
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(interval));
    }
}

void Task_NetManager(void *argument)
{
    uint32_t connect_start = millis(); // WiFi 连接超时计时起点

    Serial.printf("[NetManager] WiFi连接中");

    for (;;)
    {
        // 检查WiFi 连接状态
        if (WiFi.status() != WL_CONNECTED)
        {
            // WiFi 断开（或尚未连接）
            if (wifi_connected)
            {
                // 从“已连接”变为“断开”，清理状态
                wifi_connected = false;
                tcp_connected = false;
                connect_start = millis();
                MDNS.end();

                // 显式关闭旧 TCP 客户端，释放 socket 资源
                if (xSemaphoreTake(ClientMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    client.stop();
                    xSemaphoreGive(ClientMutex);
                }

                // 通知 LED 任务恢复闪烁指示
                if (TaskHandle_LED != NULL)
                {
                    xTaskNotifyGive(TaskHandle_LED);
                }

                Serial.printf("[NetManager] %s连接已断开\n", SSID);
                Serial.printf("[NetManager] WiFi重连中");
            }

            // 连接超时：强制重启 WiFi 模块
            if (millis() - connect_start > TIMEOUT)
            {
                connect_start = millis();
                Serial.printf("\n[NetManager] WiFi连接超时, 强制重启 WiFi 模块\n");
                Serial.printf("[NetManager] WiFi连接中");
                WiFi.disconnect(true);
                WiFi.begin(SSID, PASS);
            }

            Serial.printf(".");
        }
        else
        {
            // WiFi 已连接
            if (!wifi_connected)
            {
                // 从“未连接”变为“已连接”，初始化网络服务
                wifi_connected = true;
                Serial.printf("\n[NetManager] %s已连接\n", SSID);
                Serial.printf("[NetManager] IP: %s\n",
                              WiFi.localIP().toString().c_str());

                // 通知 LED 任务改变闪烁状态
                if (TaskHandle_LED != NULL)
                {
                    xTaskNotifyGive(TaskHandle_LED);
                }

                if (MDNS.begin(MDNS_NAME))
                {
                    Serial.printf("[NetManager] mDNS启动成功, 域名: %s.local\n", MDNS_NAME);
                    MDNS.addService("uart", "tcp", TCP_PORT);
                }
                else
                {
                    Serial.printf("[NetManager] mDNS启动失败, 请使用IP访问\n");
                }

                server.begin();
            }
        }

        // 管理tcp客户端
        if (xSemaphoreTake(ClientMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // 当前无客户端时才尝试添加客户端
            if (!client.connected())
            {
                // 客户端从连接状态断开
                if (tcp_connected)
                {
                    tcp_connected = false;
                    Serial.printf("[NetManager] TCP客户端已断开\n");

                    // 通知stm32停止（等通信协议定好后加上）

                    // 通知 LED 任务恢复闪烁指示
                    if (TaskHandle_LED != NULL)
                    {
                        xTaskNotifyGive(TaskHandle_LED);
                    }
                }

                // 确保释放旧连接资源
                client.stop();

                // 尝试接受新客户端
                WiFiClient newClient = server.available();
                if (newClient)
                {
                    client = newClient;
                    tcp_connected = true;
                    Serial.printf("[NetManager] 新客户端连入, IP: %s:%u\n",
                                  client.remoteIP().toString().c_str(),
                                  (unsigned)client.remotePort());

                    // 客户端已连接，同步标志位
                    tcp_connected = true;

                    // 通知 LED 任务改变闪烁状态
                    if (TaskHandle_LED != NULL)
                    {
                        xTaskNotifyGive(TaskHandle_LED);
                    }
                }
            }

            xSemaphoreGive(ClientMutex);
        }

        //TODO: 心跳保活

        // 1 秒轮询周期
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}