#include <stdio.h>
#include "IO_driver.h"
#include "FreeRTOS_wrapper.h"

#define T0H_TIME 0
#define T0L_TIME 9
#define T1H_TIME 28
#define T1L_TIME 15 

void Logical_0 (void)
{
    gpio_set_level(GPIO_D1, 1);
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    gpio_set_level(GPIO_D1, 0);
    for(int i=0; i < T0L_TIME; i++)
    {
        __asm("NOP");
    }
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
}
void Logical_1 (void)
{
    gpio_set_level(GPIO_D1, 1);
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    gpio_set_level(GPIO_D1, 0);
    for(int i=0; i < T0L_TIME; i++)
    {
        __asm("NOP");
    }
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
}

/*internally the led strip uses GREEN, RED, BLUE*/
void Color_Frame(uint8_t R,uint8_t G, uint8_t B)
{
    uint32_t cluster = (G << 16) | ( R << 8) | B;  
    uint8_t bit_value;
    gpio_set_level(GPIO_D1, 0);
    //printf("color frame: %06X\r\n",cluster);
    taskENTER_CRITICAL();
    
    for(uint8_t i = 0; i < 24;i++)
    {
       bit_value =  cluster & (1<<i);
       if(bit_value == 0)
       {
           Logical_0();
       }
       else
       {
           //Logical_1();
       }
    }
    
   taskEXIT_CRITICAL();
}