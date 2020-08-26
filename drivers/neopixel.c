// =============================================================================
// includes
// =============================================================================
#include <stdio.h>
#include "IO_driver.h"
#include "FreeRTOS_wrapper.h"
#include "neopixel.h"

// =============================================================================
// Local variables
// =============================================================================

uint8_t H_counter = 0;
uint8_t L_counter = 0;
int delay = NEO_DELAY;

/**
 * @brief LOGICAL 0 driver to neopixel
 * NOP are used because 
 * HW timer peripheral cant reach Neopixel time requirments
 */
void Logical_0 (void)
{
    gpio_set_level(GPIO_SDD3, 1);
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    gpio_set_level(GPIO_SDD3, 0);
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
/**
 * @brief LOGICAL 1 driver to neopixel
 * NOP are used because 
 * HW timer peripheral cant reach Neopixel time requirments
 */
void Logical_1 (void)
{
    gpio_set_level(GPIO_SDD3, 1);
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

    gpio_set_level(GPIO_SDD3, 0);

    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
}
/**
 * @brief Ther is a must a 
 * 50us delay
 * between each data frame
 */
void interpacket_reset(void)
{
    gpio_set_level(GPIO_SDD3, 0);
    for(int i=0;i < 1000;i++) 
    {
        __asm("NOP");
    }
}

/**
 * @brief 
 * internally the led strip uses GREEN, RED, BLUE
 * char* is used to pass data as reference and do frames faster
 * @param R -> red 
 * @param G -> green
 * @param B -> blue
 */
void Color_Frame(uint8_t* R,uint8_t* G, uint8_t* B)
{
    uint32_t cluster = (*G << 16) | ( *R << 8) | *B; // set up mask
    uint32_t bit_value = 0;

    interpacket_reset();
    for(uint8_t i = 23; i != 255; i--)//pass 24 bits (8 bits per color)
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
}
