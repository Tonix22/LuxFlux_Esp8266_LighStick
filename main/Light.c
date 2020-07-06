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
#include "imu6050.h"
#include "neopixel.h"
#include "Menu.h"
#include "config.h"

//master
extern xQueueHandle Light_event;
extern xQueueHandle imu_cntrl_queue;

void app_main(void)
{
    //Light_MessageID test = FADE;
    esp_set_cpu_freq(ESP_CPU_FREQ_160M);
    vTaskDelay(1000 / portTICK_RATE_MS);
    printf("app_main\r\n");
  
    Ligth_init();
    imu_init();


    //Output_LED_config();
    //input_IO_config();
    //input_IO_enable_isr();


    //wifi_general_cfg();

    //Thread_safety_GPIO_config();
    IMU_msgID msg = IMU_START_CALIBRATION;
    vTaskDelay(1000/ portTICK_RATE_MS);

    xQueueSend(imu_cntrl_queue, &msg, 10/ portTICK_RATE_MS);

    for(;;)
    {
        //WIFI_OFF();
        vTaskDelay(5000/ portTICK_RATE_MS);
        //WIFI_ON();
        //vTaskDelay(5000/ portTICK_RATE_MS);

        //Pixel_rainbow();
        //Flash_color(255,255,255,100);
        //Fade_color();
        //Pixel_rainbow_Fade();
    }
    
}
