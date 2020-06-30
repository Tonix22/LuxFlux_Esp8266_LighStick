#ifndef IO_DRIVER_H
#define IO_DRIVER_H

#include "driver/gpio.h"
#define DEVELOP
#define GPIO_DO 16
#define GPIO_D3 0

#define GPIO_SDD2 9
#ifdef DEVELOP
#define GPIO_SDD3 5
#else
#define GPIO_SDD3 10
#endif

#define GPIO_D4_SCL 2
#define GPIO_D5_SDA 14

#define GPIO_OUT_CONFIG (1ULL << GPIO_DO) | (1ULL << GPIO_SDD3)
#define GPIO_IN_CONFIG (1UL << GPIO_D3) | (1UL << GPIO_SDD2)

void Output_LED_config(void);
void input_IO_config(void);
void input_IO_disable_isr(uint32_t GPIO);
void input_IO_enable_isr(uint32_t GPIO,int EDGE);
void Thread_safety_GPIO_config(void);
#endif