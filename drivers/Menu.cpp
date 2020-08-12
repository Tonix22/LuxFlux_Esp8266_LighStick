	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

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
EventGroupHandle_t Menu_status;

extern xQueueHandle imu_cntrl_queue;
extern xQueueHandle Light_event;

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
void abort_last_menu()
{
    xEventGroupSetBits(Menu_status, ABORT);
    
    EventBits_t kill = xEventGroupWaitBits(Menu_status,TASKDEATH,
            pdTRUE,// Clear Flag
            pdFALSE,portMAX_DELAY); // wait until IDLE Task is death
}

inline void DispMenu::first_time()
{
    static bool fts = true;
    if(fts)//first call of all
    {
        Menu_status = xEventGroupCreate();
        while(Light_event == NULL){vTaskDelay(50 / portTICK_RATE_MS);} // wait light task to be ready
        fts = false;// only acces once
    }
}

void Process_feature(FeatureBehaviour* feature)
{
    if(feature->file_found == CLEAR_BIT)
    {
        do{
            feature->load_feature_file();
            if(feature->load_status == CLEAR_BIT)
            {
                feature->run_feature_read();
            }
        }while(feature->cyclic == CLEAR_BIT);
    }
    DEBUG_EMPTY_FILE();

    delete(feature);
}

// functions
void idle_subtask(void *arg)
{
    Menu.first_time(); // wait to light task init
    IDLE_Light* idle_proc = new IDLE_Light(IDLE_feature);
    
    Process_feature(idle_proc);

    xEventGroupSetBits(Menu_status,TASKDEATH);
    vTaskDelete(NULL);
}

void rith_subtask(void *arg)
{
    abort_last_menu();
    
    Riht_Light* rith_proc = new Riht_Light(RITH_feature);
    Process_feature(rith_proc);
    
    cout<<"rith_subtask"<<endl;
    
    vTaskDelete(NULL);
}

void circular_subtask(void *arg)
{
    cout<<"circular_subtask"<<endl;
    
    input_IO_disable_isr(GPIO_SDD2); // disable last sound ISR
    
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
