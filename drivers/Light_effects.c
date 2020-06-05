#include "esp_system.h"
#include "Light_effects.h"
#include "FreeRTOS_wrapper.h"
#include "neopixel.h"

uint8_t rainbow_delay = 10;
uint8_t pixels        = 8;
int flash_times       = 1000;
int fade_cycles       = 1000;

void Fade_color(void)
{
    uint8_t leds_num = 0;
    bool orange = false;
    while(1)
    {
        leds_num++;
        for(int j=0; j < leds_num ;j++)
        {
            if(orange == false)
            {
                Color_Frame(255,0,0);
            }
            else
            {
                Color_Frame(255,128,0);
            }
            
        }
        if(leds_num == pixels)
        {
            leds_num = 0;
            orange ^= 1;
        }
        vTaskDelay(100/ portTICK_RATE_MS);
    }
}

void Flash_color(uint8_t R, uint8_t G, uint8_t B, int ms_rate)
{ 
    for(int i=0;i < flash_times;i++)
    {
        for(int j=0; j < pixels; j++)
        {
            Color_Frame(R,G,B);
        }
        vTaskDelay(ms_rate/ portTICK_RATE_MS);
        for(int j=0; j < pixels; j++)
        {
            Color_Frame(0,0,0);
        }
        vTaskDelay(ms_rate/ portTICK_RATE_MS);
    }   
}


void Pixel_rainbow(void)
{
   
    int i=0;
    int j=0;
    int k=0;
    int led;

    for(i=0;i<255;i++)
    {
        vTaskDelay(rainbow_delay/ portTICK_RATE_MS);
        for(led = 0; led < pixels ;led++)
        {
            Color_Frame(i,j,k);
        }
    }
    for(j=0;j<255;j++)
    {
        vTaskDelay(rainbow_delay/ portTICK_RATE_MS);
        for(led = 0; led < pixels ;led++)
        {
            Color_Frame(i,j,k);
        }
    }
    for(i=255;i!=0;i--)
    {
        vTaskDelay(rainbow_delay/ portTICK_RATE_MS);
        for(led = 0; led < pixels ;led++)
        {
            Color_Frame(i,j,k);
        }
    }
    for(k=0;k<255;k++)
    {
        vTaskDelay(rainbow_delay/ portTICK_RATE_MS);
        for(led = 0; led < pixels ;led++)
        {
            Color_Frame(i,j,k);
        }
    }
    for(j=255;j!=0;j--)
    {
        vTaskDelay(rainbow_delay/ portTICK_RATE_MS);
        for(led = 0; led < pixels ;led++)
        {
            Color_Frame(i,j,k);
        }
    }

    for(k=255;k!=0;k--)
    {
        vTaskDelay(rainbow_delay/ portTICK_RATE_MS);
        for(led = 0; led < pixels ;led++)
        {
            Color_Frame(i,j,k);
        }
    }
}


