#include <iostream>
#include "Menu.h"
#include "FreeRTOS_wrapper.h"
#include "IO_driver.h"
#include "imu6050.h"
using namespace std;

DispMenu Menu;
gpio_num_t io_num;
int pressed = 0;
xQueueHandle sync_lock;

extern xQueueHandle imu_cntrl_queue;

void menu_init()
{
    sync_lock = xQueueCreate(5,sizeof(bool));
    if( sync_lock == NULL )
    {
        cout<<"sync_lock was not created"<<endl;
    }
}

// functions
void idle_func(){
    cout<<"idle_func"<<endl;
}

void rith_func()
{
    input_IO_enable_isr(GPIO_SDD2, GPIO_INTR_NEGEDGE);
    cout<<"rith_func"<<endl;
}

void circ_func()
{
    static bool first_time = true;
    IMU_msgID msg;
    cout<<"circ_func"<<endl;

    input_IO_disable_isr(GPIO_SDD2);

    if(first_time == true) // TODO Check if we calibrate every time we are enter or only once
    {
        first_time = false;
        msg        = IMU_START_CALIBRATION;
        xQueueSend(imu_cntrl_queue, &msg, 10/ portTICK_RATE_MS);
    }
}

void level_func(){
    IMU_msgID msg;
    cout<<"level_func"<<endl;
    if(get_calibration_status() == IMU_CALIBRATION)
    {
        msg = IMU_ABORT_CALIBRATION;
        xQueueSend(imu_cntrl_queue, &msg, 10/ portTICK_RATE_MS);
    }
    
}

void wifi_func(){
    cout<<"wifi_func"<<endl;
}

void sync_func(){
    input_IO_disable_isr(GPIO_SDD2);
    cout<<"sync_func"<<endl;
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
