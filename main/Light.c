/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "FreeRTOS_wrapper.h"
#include "esp_system.h"
#include "IO_driver.h"
#include "Light_effects.h"
#include "wifi.h"
void app_main(void)
{
    esp_set_cpu_freq(ESP_CPU_FREQ_160M);
    vTaskDelay(1000 / portTICK_RATE_MS);
    printf("app_main\r\n");
    Ligth_init();

    Output_LED_config();
    input_IO_config();
    input_IO_enable_isr();
    //wifi_init_sta();
    //wifi_init_softap();

    for(;;)
    {
        vTaskDelay(100/ portTICK_RATE_MS);
        //for(int i=0;i<8;i++)
        //{
        //    Color_Frame(255,0,0);
        //}
        //Pixel_rainbow();
        //Flash_color(255,255,255,100);
        //Fade_color();
    }
    
}
