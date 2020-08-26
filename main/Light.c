// =============================================================================
// includes
// =============================================================================

#include <stdio.h>
#include "FreeRTOS_wrapper.h"
#include "esp_system.h"
#include "IO_driver.h"
#include "Light_effects.h"
#include "imu6050.h"
#include "file_system.h"
#include "config.h"


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
    write_to_rith();//TODO testing purpse
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
