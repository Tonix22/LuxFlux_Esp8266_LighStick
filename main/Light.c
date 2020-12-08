// =============================================================================
// includes
// =============================================================================

#include <stdio.h>
#include <string.h>
#include "FreeRTOS_wrapper.h"
#include "esp_system.h"
#include "IO_driver.h"
#include "Light_effects.h"
#include "imu6050.h"
#include "file_system.h"
#include "config.h"
#include "lwip/init.h"


// =============================================================================
//  VERSION INFO !!!IMPORTANT!!
// =============================================================================

/*
This repo works correctly with the following conditions 

+ ESP8266 RTOS SDK commit 9f37d6900f (HEAD -> master)
+ LWIP ffd1059
+ mbedtls 9ef92c55
+ cJSON v1.7.12
to fix the problem type in your terminal -> git checkout <commit_id>
**/
void check_version()
{
    if(strcmp(esp_get_idf_version(),VALID_SYSTEM_VERSION) != 0)
    {
        printf("\x1B[0;33mW !!!WARNING!!! RTOS VERSION IS NOT g9f37d690\r\n");
        printf("\x1B[0;33mW !!!WARNING!!! UNEXPECTED BEHAVIOR\r\n");
    }
    if(strcmp(LWIP_VERSION_STRING,LWIP_VALID_VERSION)!=0)
    {   
        printf("\x1B[0;31mE WIFI MAY FAIL LWIP VERSION INCORRECT\r\n");
    }
}

// =============================================================================
// external testing variables
// =============================================================================
extern xQueueHandle Light_event;
extern xQueueHandle imu_cntrl_queue;

/**
 * @brief 
 * 1. CPU Frecuency will be 160MHZ, because neopixels requires fast speed.
 * 2. GPIO setup configuration
 * 3. Thread_safety_GPIO_config makes a task where GPIO ISR messages
 *    are handled. It will be hardware bridge to communicate Menu button with 
 *    features. IDLE, RITH, CIRCULAR, LINEAR,WIFI SYNC
 * 4. File System: Flash driver to save color patterns used in different feature effects
 * 5. Neopixel Ledstick init
 * 6. IMU driver init.
 */

void app_main(void)
{
    //check_version();//VERSION INFO !!!IMPORTANT!!

    esp_set_cpu_freq(ESP_CPU_FREQ_160M); // 1
    // ================================
    // IO config
    // ================================
    Output_LED_config();// 2
    input_IO_config();
    Thread_safety_GPIO_config(); // --> Here Menu is initialized //3
    // ================================
    // Filse System -> Flash Driver
    // ================================
    file_system_init(); // 4
   // write_to_rith();//TODO testing purpse
    write_to_idle();//TODO testing purpse
    // ================================
    // Tasks for Ledstick and IMU
    // ================================
    Ligth_init(); // 5
    imu_init();   // 6
    // ================================
    // IDLE TASK SHALL EVERY EXIST
    // ================================

    #if IMU_TEST
    IMU_msgID msg = IMU_START_CALIBRATION;
    vTaskDelay(1000/ portTICK_RATE_MS);
    xQueueSend(imu_cntrl_queue, &msg, 10/ portTICK_RATE_MS);
    Position(10000);
    #endif

    for(;;)
    {

        vTaskDelay(5000/ portTICK_RATE_MS);
        #if PIXEL_TEST
        Pixel_rainbow();
        #endif
    }
    
}
