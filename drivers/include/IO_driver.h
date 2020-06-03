#ifndef IO_DRIVER_H
#define IO_DRIVER_H

#include "driver/gpio.h"
#define GPIO_DO 16
#define GPIO_D1 5
#define GPIO_OUT_CONFIG (1ULL << GPIO_DO) | (1ULL << GPIO_D1)

void Output_LED_config(void);
void Input_LED_config(void);
void Thread_safety_GPIO_config(void);
#endif