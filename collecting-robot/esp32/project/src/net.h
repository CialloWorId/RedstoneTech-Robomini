#ifndef __NET_H__
#define __NET_H__

#include "WiFi.h"

extern TaskHandle_t TaskHandle_LED;
extern SemaphoreHandle_t ClientMutex;
extern volatile bool tcp_connected;
extern WiFiClient client;

void Task_LED(void *argument);
void Task_NetManager(void *argument);

#endif