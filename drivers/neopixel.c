#include <stdio.h>
#include "IO_driver.h"
#include "FreeRTOS_wrapper.h"


#define T0L_TIME 1
#define T1H_TIME 1
uint8_t H_counter = 0;
uint8_t L_counter = 0;
int delay = 10;


void Logical_0 (void)
{
    gpio_set_level(GPIO_S3, 1);
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    gpio_set_level(GPIO_S3, 0);
    for(L_counter = 0; L_counter < T0L_TIME; L_counter++)
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
}
void Logical_1 (void)
{
    gpio_set_level(GPIO_S3, 1);
    for(H_counter = 0; H_counter < T1H_TIME; H_counter++)
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
    __asm("NOP");
    __asm("NOP");

    gpio_set_level(GPIO_S3, 0);

    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
}
//50us delay
void interpacket_reset(void)
{
    gpio_set_level(GPIO_S3, 0);
    for(int i=0;i < 1000;i++) 
    {
        __asm("NOP");
    }
}
/*internally the led strip uses GREEN, RED, BLUE*/
void Color_Frame(uint8_t R,uint8_t G, uint8_t B)
{
    uint32_t cluster = (G << 16) | ( R << 8) | B;  
    uint32_t bit_value = 0;

    //printf("color frame: %06X\r\n",cluster);

    interpacket_reset();
    for(uint8_t i = 23; i != 255; i--)
    {
       bit_value =  cluster & (1<<i);
       if(bit_value == 0)
       {
           Logical_0();
       }
       else
       {
           Logical_1();
       }
    }
     //printf("color mask: %06X\r\n",bit_value);
   
}
