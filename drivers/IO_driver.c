#include "FreeRTOS_wrapper.h"
#include "esp_log.h"
#include "esp_system.h"
#include "IO_driver.h"
#include "Light_effects.h"
#include "imu6050.h"

extern xQueueHandle Light_event;

static const char *TAG = "IO_DRIVER";
static xQueueHandle gpio_evt_queue = NULL;



static void gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    if(gpio_num == GPIO_SDD2){
        MessageID send = SOUND;
        xQueueSendFromISR(Light_event, &send, NULL);
    }else{
        xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    }
    
    
}

static void gpio_task_example(void *arg)
{
    uint32_t io_num;
    //int cnt =0;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            ESP_LOGI(TAG, "GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            //cnt^=1;
            //gpio_set_level(16, cnt);
        }
    }
}


void Output_LED_config(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask =  GPIO_OUT_CONFIG;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void input_IO_config(void)
{
    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins, use GPIO4 here
    io_conf.pin_bit_mask = GPIO_IN_CONFIG;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO_D3, GPIO_INTR_NEGEDGE);//TODO cambiar a RISING EDGE :)
    //install gpio isr service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_D3, gpio_isr_handler, (void *) 0);
    
}


void input_IO_enable_isr(void)
{   
    //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO_SDD2, GPIO_INTR_NEGEDGE);

    gpio_isr_handler_add(GPIO_SDD2, gpio_isr_handler, (void *) 0);
}

void input_IO_disable_isr(void)
{
    gpio_isr_handler_remove(GPIO_SDD2);
    
}

void Thread_safety_GPIO_config(void)
{
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

}