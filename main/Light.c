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
#include "neopixel.h"
#include "wifi.h"
void app_main(void)
{
    esp_set_cpu_freq(ESP_CPU_FREQ_160M);
    vTaskDelay(1000 / portTICK_RATE_MS);
    printf("app_main\r\n");
    Output_LED_config();
    
    //Input_LED_config();
    //Thread_safety_GPIO_config();
    //wifi_init_sta();
    //wifi_init_softap();

    for(;;)
    {
        Pixel_rainbow();
    }
    
}
