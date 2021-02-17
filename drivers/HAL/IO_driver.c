#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
// =============================================================================
// includes
// =============================================================================
#include "esp_log.h"
#include "esp_system.h"
#include "IO_driver.h"
#include "Light_effects.h"
#include "Menu.h"

// =============================================================================
// Queues
// =============================================================================

extern xQueueHandle Light_event;
static xQueueHandle gpio_evt_queue = NULL;
Light_MessageID send = SOUND;

// =============================================================================
// Prototypes
// =============================================================================

static void gpio_menu_task(void *arg);

/**
 * @brief 
 * Recieve ISR IO signal and send to corresponding queue event
 * SOUND as GPIO_SDD2
 * ELSE MENU BUTTON
 */
static void gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;

    if(gpio_num == GPIO_SDD2)
    {
        input_IO_disable_isr(GPIO_SDD2); // debounce sound timer, avoid samples saturation
        xQueueSendFromISR(Light_event, &send, NULL);
    }
    else
    {
        xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    }
}

/**
 * @brief 
 * Create Thread GPIO task
 */
void Thread_safety_GPIO_config(void)
{
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_menu_task, "gpio_menu_task", 2048, NULL, 10, NULL);

}
/**
 * @brief 
 * Control Menu task Gpio ISR movement
 * If button is pressed 3 seconds it enters to SYNC menu
 * sync_timer is used to count if button is pressed
 * 
 * ISR of button is GPIO_INTR_ANYEDGE so, to caputres button 
 * pressed and released
 * 
 * Each Menu is an indpendent task to avoid blocking GPIO task
 */
static void gpio_menu_task(void *arg)
{
    uint32_t io_num;
    TimerHandle_t sync_timer;

    Menu_func(); // Start IDLE subtask
    sync_timer = xTimerCreate("Sync", 3000/ portTICK_RATE_MS, pdFALSE, 0, sync_action);
    
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)){ // wait for ISR signal
            
            input_IO_disable_isr(GPIO_D3);                  // Debounce  
            vTaskDelay(DEBOUNCE_TIME/ portTICK_RATE_MS);    // Filter noice produced
            input_IO_enable_isr(GPIO_D3, GPIO_INTR_ANYEDGE);// by shaking hand

            if(gpio_get_level(io_num) == PRESSED){
                xTimerStart(sync_timer,0); // Prepare timer in case button still presed
            }else{
                if(xTimerIsTimerActive(sync_timer) != DORMANT) // Abort timer only if is active
                {
                    xTimerStop(sync_timer,0); // Abort timer execution
                    Menu_func(); // Call a subtask only if timer wasn't call
                }
            }
        }
    }
}

/**
 * @brief 
 * Configure 
 * GPIO 5
 * GPIO 16
 * Output
 * PUll-up enable
 */

void Output_LED_config(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO0/16
    io_conf.pin_bit_mask =  GPIO_OUT_CONFIG;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}
/**
 * @brief 
 * Configure 
 * GPIO 0 --> Menu button
 * GPIO 4 --> Sound Input
 * Input
 * Install ISR  
 */

void input_IO_config(void)
{
    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_IN_CONFIG;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO_D3,   GPIO_INTR_ANYEDGE);//Menu button
    gpio_set_intr_type(GPIO_SDD2, GPIO_INTR_DISABLE);//audio
    //install gpio isr service

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_D3, gpio_isr_handler,  GPIO_D3);
    gpio_isr_handler_add(GPIO_SDD2, gpio_isr_handler,GPIO_SDD2);
}
/**
 * @brief 
 * Setup generic input ISR
 */

void input_IO_enable_isr(uint32_t GPIO,int EDGE )
{   
    //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO, EDGE);
}
/**
 * @brief 
 * Disable generic input ISR
 */
void input_IO_disable_isr(uint32_t GPIO)
{
    gpio_set_intr_type(GPIO,GPIO_INTR_DISABLE);
}
