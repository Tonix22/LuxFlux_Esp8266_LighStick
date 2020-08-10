#include <iostream>
#include <vector>
#include <list>
#include "Menu.h"
#include "FreeRTOS_wrapper.h"
#include "IO_driver.h"
#include "imu6050.h"
#include "Light_effects.h"
#include "memory_admin.h"

using namespace std;

DispMenu Menu;
gpio_num_t io_num;
int pressed = 0;

extern xQueueHandle imu_cntrl_queue;
extern xQueueHandle Light_event;
extern EventGroupHandle_t Flash_status;
extern EventGroupHandle_t Light_status;

void calib_and_cmd(IMU_msgID action)
{
    IMU_msgID msg = IMU_START_CALIBRATION;
    xQueueSend(imu_cntrl_queue, &msg, 10/ portTICK_RATE_MS);

    /*Hold calibration done*/
    while(get_calibration_status() == IMU_CALIBRATION)
    {
        vTaskDelay(100 / portTICK_RATE_MS);
    }
    if(get_calibration_status() != IMU_ABORT_CALIBRATION)
    {
        xQueueSend(imu_cntrl_queue, &action, portMAX_DELAY);
    }
}
void abort_if_needed()
{
    IMU_msgID msg;
    if(get_calibration_status() == IMU_CALIBRATION)
    {
        msg = IMU_ABORT_CALIBRATION;
        xQueueSend(imu_cntrl_queue, &msg, 10/ portTICK_RATE_MS);
    }
}

// functions
void idle_subtask(void *arg)
{
    while(Light_event == NULL){vTaskDelay(50 / portTICK_RATE_MS);}
    
    file_exist(IDLE_feature);
    EventBits_t bits = xEventGroupGetBits(Flash_status);
    
    if(bits & EMPTYFILE)
    {
        printf("EMPTY FILE IDLE DONE\r\n");
    }
    else
    {
        while(!(bits & BAD_FORMAT) && !(bits & ABORT))
        {
            file_read(IDLE_feature);
            bits = xEventGroupGetBits(Flash_status);

            if(bits & READ_OK)
                IDLE_light();

            bits = xEventGroupClearBits(Flash_status,READ_OK);
        }
    }

    xEventGroupClearBits(Flash_status,BAD_FORMAT|READ_OK|ABORT);
    vTaskDelete(NULL);
}

void rith_subtask(void *arg)
{
    input_IO_enable_isr(GPIO_SDD2, GPIO_INTR_NEGEDGE);
    cout<<"rith_subtask"<<endl;
    vTaskDelete(NULL);
}

void circular_subtask(void *arg)
{
    cout<<"circular_subtask"<<endl;
    
    //input_IO_disable_isr(GPIO_SDD2); // disable last sound ISR
    
    //calib_and_cmd(IMU_CIRCULAR_DRAW);

    vTaskDelete(NULL);
}

void level_subtask(void *arg)
{
    cout<<"level_subtask"<<endl;

    //abort_if_needed();

    //calib_and_cmd(IMU_LINEAR_DRAW);

    vTaskDelete(NULL);
}

void wifi_subtask(void *arg)
{
    cout<<"wifi_subtask"<<endl;

    //abort_if_needed();
    wifi_init_softap();

    vTaskDelete(NULL);
}

void sync_subtask(void *arg)
{
    input_IO_disable_isr(GPIO_SDD2);
    cout<<"sync_subtask"<<endl;
    vTaskDelete(NULL);
}

/* 
This is a way to acces menu directly by index, 
used to branch to a specific menu function. 
For example when a button is pressed by 3
seconds, the fucntion call will be directly like this.
*/
void sync_action (TimerHandle_t xTimer )
{
        Menu[SYNC];// Acces directly to function with enum
        Menu++;
}

void Menu_func()
{
    Menu(); // Jump to function like this
    Menu++; // Incremt position like this
}
