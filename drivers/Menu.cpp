	
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
extern EventGroupHandle_t calib_flags;

extern xQueueHandle imu_cntrl_queue;
extern xQueueHandle Light_event;

bool calib_and_cmd(IMU_msgID action)
{
    bool procced = true;
    IMU_msgID msg = IMU_START_CALIBRATION;
    xQueueSend(imu_cntrl_queue, &msg, 10/ portTICK_RATE_MS);

    /*Hold calibration done*/
    EventBits_t hold_calib = xEventGroupWaitBits(calib_flags,
        SUCESS_CALIB|TERMINATED,
        pdFALSE,// Clear Flag
        pdFALSE,portMAX_DELAY); // wait until IDLE Task is death
    
    xEventGroupClearBits(calib_flags,INPROGRESS_CALIB);

    if(hold_calib & SUCESS_CALIB)
    {
        xQueueSend(imu_cntrl_queue, &action, portMAX_DELAY);
    }
    if(hold_calib & TERMINATED)
    {
        procced = false;
        xEventGroupClearBits(calib_flags,TERMINATED);
    }
    return procced;
}
void abort_calib()
{
    EventBits_t status = xEventGroupGetBits(calib_flags);
    if(status & INPROGRESS_CALIB)
    {   
        xEventGroupSetBits(calib_flags,ABORT_CALIB);
    }
}
void abort_last_menu()
{
    xEventGroupSetBits(Menu_status, ABORT);
    
    EventBits_t kill = xEventGroupWaitBits(Menu_status,TASKDEATH,
            pdTRUE,// Clear Flag
            pdFALSE,
            portMAX_DELAY); // wait until IDLE Task is death
    xEventGroupClearBits(Menu_status, ABORT);
}

void DispMenu::first_time()
{
    static bool fts = true;
    if(fts)//first call of all
    {
        Menu_status = xEventGroupCreate();
        while(Light_event == NULL){vTaskDelay(50 / portTICK_RATE_MS);} // wait light task to be ready
        while(calib_flags == NULL){vTaskDelay(50 / portTICK_RATE_MS);} // wait IMU to init
        EventBits_t IMU_status = xEventGroupWaitBits(calib_flags,
        IMU_HAS_CONECTION|IMU_NOT_CONNECT,
        pdFALSE,// Clear Flag
        pdFALSE,portMAX_DELAY); // wait until IDLE Task is death
        if(IMU_status & IMU_NOT_CONNECT)
        {
            ESP_LOGE("IMU STATUS", "IMU IS NOT CONNECTED");
        }
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
        }while(feature->cyclic == CLEAR_BIT );
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
    cout<<"rith_subtask"<<endl;
    abort_last_menu();
    
    Riht_Light* rith_proc = new Riht_Light(RITH_feature);
    Process_feature(rith_proc);

    vTaskDelete(NULL);
}

void circular_subtask(void *arg)
{
    cout<<"circular_subtask"<<endl;
    input_IO_disable_isr(GPIO_SDD2);
    if(calib_and_cmd(IMU_CIRCULAR_DRAW))
    {
        Circular_Light* circ_proc = new Circular_Light(CIRCULAR_feature);
        //Process_feature(circ_proc);
    }

    xEventGroupSetBits(Menu_status,TASKDEATH);
    
    vTaskDelete(NULL);
}

void level_subtask(void *arg)
{
    cout<<"level_subtask"<<endl;
    abort_calib();
    abort_last_menu();

    if(calib_and_cmd(IMU_LINEAR_DRAW))
    {

    }

    xEventGroupSetBits(Menu_status,TASKDEATH);
    
    vTaskDelete(NULL);
}

void wifi_subtask(void *arg)
{
    cout<<"wifi_subtask"<<endl;
    abort_calib();
    
    //wifi_init_softap();

    vTaskDelete(NULL);
}

void sync_subtask(void *arg)
{
    cout<<"sync_subtask"<<endl;
    input_IO_disable_isr(GPIO_SDD2);
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
