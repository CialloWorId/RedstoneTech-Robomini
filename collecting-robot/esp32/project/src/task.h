#ifndef __TASK_H__
#define __TASK_H__

extern SemaphoreHandle_t ClientMutex;
void Task_TaskCreate(void *argument);

#endif