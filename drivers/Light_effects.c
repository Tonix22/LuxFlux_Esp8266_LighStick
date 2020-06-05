#include <stdio.h>
#include "esp_system.h"
#include "Light_effects.h"
#include "FreeRTOS_wrapper.h"
#include "neopixel.h"

uint8_t rainbow_delay = 10;
uint8_t pixels        = 8;
int flash_times       = 1000;
int fade_cycles       = 1000;

xQueueHandle Light_event = NULL;

static TimerHandle_t hold_next_sound;
static bool sound_released = true;
void sound_action (TimerHandle_t xTimer );

void Light_task(void *arg)
{
    Light_event = xQueueCreate(10, sizeof(uint32_t));
    hold_next_sound = xTimerCreate("Sound", 100/ portTICK_RATE_MS, pdFALSE, 0, sound_action);
    MessageID msg;
    printf("light_stask init\r\n");
    for(;;)
    {
        if (xQueueReceive(Light_event, &msg, portMAX_DELAY))
        {
            if(msg == SOUND && sound_released == true)
            {
                sound_released = false;
                xTimerStart(hold_next_sound, 0 );
            }
        }
    }
}

void Ligth_init(void)
{
    xTaskCreate(Light_task, "Light_task", 2048, NULL, 10, NULL);
}


void sound_action (TimerHandle_t xTimer )
{
    static char toggle = 0;
    toggle^=1;
    static bool frame = 0;
    if(toggle)
    {
        for(int i=0 ; i < pixels ; i++)
        {
            Color_Frame(0,0,0);
        }
    }
    else
    {
        for(int i=0 ; i < pixels ; i++)
        {
            if(frame)
            {
                 Color_Frame(255,128,0);
            }
            else
            {
                Color_Frame(255,0,0);
            }
            
        }
        frame^=1;
    }
    sound_released = true;
}


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



