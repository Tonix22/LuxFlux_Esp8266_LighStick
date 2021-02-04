// =============================================================================
// Delete Warning in compiler
// =============================================================================

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// =============================================================================
// includes
// =============================================================================

#include <iostream>
#include <vector>
#include <list>
#include "Menu.h"
#include "FreeRTOS_wrapper.h"
#include "IO_driver.h"
#include "imu6050.h"
#include "Light_effects.h"
#include "memory_admin.h"

// =============================================================================
// namespaces
// =============================================================================

using namespace std;

// =============================================================================
// Locals
// =============================================================================

DispMenu Menu; //Class to work with Menu actions and task

// =============================================================================
// EVENT GROUPS 
// =============================================================================

EventGroupHandle_t Menu_status;
extern EventGroupHandle_t calib_flags;

// =============================================================================
// QUEUE
// =============================================================================

extern xQueueHandle imu_cntrl_queue;
extern xQueueHandle Light_event;


/* ===============================================================================================

 ███╗   ███╗███████╗███╗   ██╗██╗   ██╗     █████╗  ██████╗████████╗██╗ ██████╗ ███╗   ██╗███████╗
 ████╗ ████║██╔════╝████╗  ██║██║   ██║    ██╔══██╗██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝
 ██╔████╔██║█████╗  ██╔██╗ ██║██║   ██║    ███████║██║        ██║   ██║██║   ██║██╔██╗ ██║███████╗
 ██║╚██╔╝██║██╔══╝  ██║╚██╗██║██║   ██║    ██╔══██║██║        ██║   ██║██║   ██║██║╚██╗██║╚════██║
 ██║ ╚═╝ ██║███████╗██║ ╚████║╚██████╔╝    ██║  ██║╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║███████║
 ╚═╝     ╚═╝╚══════╝╚═╝  ╚═══╝ ╚═════╝     ╚═╝  ╚═╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝
 ================================================================================================*/

// =============================================================================
// Functions
// =============================================================================

/**
 * @brief   
 * This function is called after a GPIO interrupt is set
 * it will initalize a task as parallel excution of menu action.
 * 
 * @return void
 */

void Menu_func()
{
    Menu(); // Create Task for spectific menu routine
    Menu++; // Prefetch next task routine
}

/**
 * @brief 
 * This is a way to acces menu directly by index, 
 * used to branch to a specific menu function. 
 * For example when a button is pressed by 3
 * seconds, the fucntion call will be directly like this.
 * 
 */

void sync_action (TimerHandle_t xTimer )
{
    Menu[SYNC];// Acces directly to function with enum
    Menu++;   // Prefetch next task routine
}

/**
 * @brief 
 * First time Setup for Menu
 * 1. Allocate Event Group Menu
 * 2. Wait IMU and Light to be done
 * 3. Get and Report IMU status conection
 */

void DispMenu::first_time()
{
    static bool fts = true;
    if(fts)//first call of all
    {
        Menu_status = xEventGroupCreate();

        while(Light_event == NULL){vTaskDelay(50 / portTICK_RATE_MS);} // wait light task to be ready
        while(calib_flags == NULL){vTaskDelay(50 / portTICK_RATE_MS);} // wait IMU to init
        /*GET IMU status*/

        EventBits_t IMU_status = xEventGroupWaitBits(calib_flags,
        IMU_HAS_CONECTION|IMU_NOT_CONNECT,
        pdFALSE,// Clear Flag
        pdFALSE,portMAX_DELAY); // wait until IDLE Task is death

        if(IMU_status & IMU_NOT_CONNECT)
        {
            ESP_LOGE("IMU STATUS", "IMU IS NOT CONNECTED"); // REPORT STATUS
        }
        fts = false;// set false to only acces once
    }
}

/**
 * @brief 
 * When a task is created it will set ABORT signal to the last task
 * wait bit TASKDEATH to proceed. 
 * Clear the Abort Flag
 */

void abort_last_menu()
{
    xEventGroupSetBits(Menu_status, ABORT);
    
    EventBits_t kill = xEventGroupWaitBits(Menu_status,TASKDEATH,
            pdTRUE,// Clear Flag
            pdFALSE,
            portMAX_DELAY); // wait until IDLE Task is death
    xEventGroupClearBits(Menu_status, ABORT);
}

/* ============================================================================
███████╗███████╗ █████╗ ████████╗██╗   ██╗██████╗ ███████╗███████╗
██╔════╝██╔════╝██╔══██╗╚══██╔══╝██║   ██║██╔══██╗██╔════╝██╔════╝
█████╗  █████╗  ███████║   ██║   ██║   ██║██████╔╝█████╗  ███████╗
██╔══╝  ██╔══╝  ██╔══██║   ██║   ██║   ██║██╔══██╗██╔══╝  ╚════██║
██║     ███████╗██║  ██║   ██║   ╚██████╔╝██║  ██║███████╗███████║
╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝
=============================================================================*/

/**
 * @brief 
 * Get polimorfic behavior of the classes
 * 1. Color File is found?
 * 2. Load Colors
 * 3. Execute Color actions, it will different for each class
 * 4. Continue to read? Y - Go to step 2 / N- Step 5
 * 5. Deallocate class it will not be used
 * 
 * @param FeatureBehaviour* feature abstract data class
 */

void Process_feature(FeatureBehaviour* feature)
{
    if(feature->file_found == CLEAR_BIT)// 1
    {
        do{
            feature->load_feature_file(); // 2
            if(feature->load_status == CLEAR_BIT)
            {
                feature->run_feature_read(); // 3
            }
        }while(feature->cyclic == CLEAR_BIT ); // 4
    }
    DEBUG_EMPTY_FILE();

    delete(feature); // 5
}
/* ============================================================================

                        ██╗██████╗ ██╗     ███████╗
                        ██║██╔══██╗██║     ██╔════╝
                        ██║██║  ██║██║     █████╗
                        ██║██║  ██║██║     ██╔══╝
                        ██║██████╔╝███████╗███████╗
                        ╚═╝╚═════╝ ╚══════╝╚══════╝
=============================================================================*/

/**
 * @brief 
 * 1. Run First time setup
 * 2. Create IDLE Class instance
 * 3. Procces -> It will load a personalized sequence
 * 4. Report Task Death
 * 5. Delete Task
 * @param arg 
 */
void idle_subtask(void *arg)
{
    Menu.first_time(); // 1
    IDLE_Light* idle_proc = new IDLE_Light(IDLE_feature);// 2
    Process_feature(idle_proc);// 3
    xEventGroupSetBits(Menu_status,TASKDEATH);// 4
    vTaskDelete(NULL); // 5
}

/* ============================================================================
                ███████╗ ██████╗ ██╗   ██╗███╗   ██╗██████╗
                ██╔════╝██╔═══██╗██║   ██║████╗  ██║██╔══██╗
                ███████╗██║   ██║██║   ██║██╔██╗ ██║██║  ██║
                ╚════██║██║   ██║██║   ██║██║╚██╗██║██║  ██║
                ███████║╚██████╔╝╚██████╔╝██║ ╚████║██████╔╝
                ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝╚═════╝
==============================================================================*/
/**
 * @brief 
 * 1. Abort IDLE 
 * 2. Create Riht_Light Class 
 * 3. Process Feature -> These feature will only setup interrupt
 * Sound will be managed as interrupts and it will comunicate directly with
 * LIGHT, so this task will NOT HAVE A CYCLE.
 * 4. Report Task Death
 * 5. Delete Task
 * @param arg 
 */
void rith_subtask(void *arg)
{
    cout<<"rith_subtask"<<endl;
    abort_last_menu(); // 1
    Riht_Light* rith_proc = new Riht_Light(RITH_feature);// 2
    Process_feature(rith_proc);// 3
    xEventGroupSetBits(Menu_status,TASKDEATH);// 4
    vTaskDelete(NULL);// 5
}

/* ============================================================================
                            ██╗███╗   ███╗██╗   ██╗
                            ██║████╗ ████║██║   ██║
                            ██║██╔████╔██║██║   ██║
                            ██║██║╚██╔╝██║██║   ██║
                            ██║██║ ╚═╝ ██║╚██████╔╝
                            ╚═╝╚═╝     ╚═╝ ╚═════╝
==============================================================================*/
/*******************************DESCRIPTION***********************************/                
/* ============================================================================*/
/**
 * @brief Each IMU task will consist in the following steps
 * 1. Calibrate the IMU -> Lights will show loading calibration
 * 2. Read an IMU sample and update a positon of the stick
 * 3. Each Position will update a color.
 * 
 * Note: calibration can be aborted to pass to next MENU
==============================================================================*/

// =============================================================================
// Functions
// =============================================================================

/**
 * @brief 
 * Used to pass to next menu and abort calibration if needed. 
 * Set ABORT_CALIB if INPROGRESS_CALIB is set.
 */
void abort_calib()
{
    EventBits_t status = xEventGroupGetBits(calib_flags);
    if(status & INPROGRESS_CALIB)   
        xEventGroupSetBits(calib_flags,ABORT_CALIB);
}

/**
 * @brief 
 * 1. Send start calibration to queue
 * 2. Wait to calibration Succed or Terminate
 * 3. Clear INPROGRESS_CALIB because calib is done
 * 4. If hold_calib SUCCESS start IMU sequence
 *    Else clear TERMINATED and set procced to false.
 * 
 * @param action Linear or Circular DRAW
 * @return true  Procced to IMU task
 * @return false IMU calib fail. Causes: ABORT,IMU DISCONNECT
 */
bool calib_and_cmd(IMU_msgID action)
{
    bool procced = true;
    IMU_msgID msg = IMU_START_CALIBRATION;
    xQueueSend(imu_cntrl_queue, &msg, 10/ portTICK_RATE_MS); // 1

    /*Hold calibration done*/
    EventBits_t hold_calib = xEventGroupWaitBits(calib_flags, // 2
        SUCESS_CALIB|TERMINATED,
        pdFALSE,// Clear Flag
        pdFALSE,portMAX_DELAY); // wait until IDLE Task is death
    
    xEventGroupClearBits(calib_flags,INPROGRESS_CALIB); // 3

    if(hold_calib & SUCESS_CALIB) // 4 a)
    {
        xQueueSend(imu_cntrl_queue, &action, portMAX_DELAY);
    }
    if(hold_calib & TERMINATED)   // 4 b)
    {
        procced = false;
        xEventGroupClearBits(calib_flags,TERMINATED);
    }
    return procced;
}

/**
 * @brief 
 * Draw a color in Ledstick for each angle.
 * 1. Disable sound ISR, last menu was SOUND
 * 2. Calibrate
 * 3. Execute
 * 4. Report Task Death
 * 5. Delete Task
 * @param arg 
 */

void circular_subtask(void *arg)
{
    cout<<"circular_subtask"<<endl;
    input_IO_disable_isr(GPIO_SDD2); // 1
    if(calib_and_cmd(IMU_CIRCULAR_DRAW)) //2
    {
        Circular_Light* circ_proc = new Circular_Light(CIRCULAR_feature);
        Process_feature(circ_proc); // 3
    }
    xEventGroupSetBits(Menu_status,TASKDEATH); // 4
    vTaskDelete(NULL);// 5
}
/**
 * @brief 
 * Draw a color in Ledstick for each angle.
 * 1. Abort circular draw calibration
 * 2. Calibrate
 * 3. Execute
 * 4. Report Task Death
 * 5. Delete Task
 * @param arg 
 */
void level_subtask(void *arg)
{
    cout<<"level_subtask"<<endl;
    abort_calib(); // 1
    abort_last_menu();

    if(calib_and_cmd(IMU_LINEAR_DRAW))
    {
        
    }

    xEventGroupSetBits(Menu_status,TASKDEATH);
    
    vTaskDelete(NULL);
}

/* ============================================================================
                            ██╗    ██╗██╗███████╗██╗
                            ██║    ██║██║██╔════╝██║
                            ██║ █╗ ██║██║█████╗  ██║
                            ██║███╗██║██║██╔══╝  ██║
                            ╚███╔███╔╝██║██║     ██║
                            ╚══╝╚══╝ ╚═╝╚═╝     ╚═╝
============================================================================*/
/*******************************DESCRIPTION***********************************/ 
/*============================================================================*/
/**
 * @brief 
 * WIFI Menus correspond to Channel abstrac class
 * This wifi features has this behavior in common
 * 1. clear_last_menu 
   2. init_wifi_service -> AP/STATION
   3. init_comm_service -> Client/Server
   4. Clean wifi_service
============================================================================*/

// =============================================================================
// Functions
// =============================================================================

/**
 * @brief TESTIG PORUPUSE COUNTER
 * 
 */
void wifi_hold()
{
    printf("Wait 10 secs\r\n");
    for(int i =0; i < 10; i++)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        printf("sec: %d\r\n",i);
    }
    printf("WIFI WILL DEATH\r\n");
}

/**
 * @brief Generic Wifi Sequence
 * 
 * @param feature Reciever or Transmiter
 */
void Process_Wifi_Comm(Channel* feature)
{   
    //printf("before?\r\n");
    feature->clear_last_menu();  //Clear last calls
    //printf("clear_last_menu-> \r\n");
    feature->init_wifi_service();//AP or Station
    feature->init_comm_service();//Client or server
    do
    {   
        vTaskDelay(2000/ portTICK_RATE_MS);
        //feature->state = STOP; //TODO testing porpuose
        //wifi_hold();

    }while(feature->state != STOP);
    
    delete(feature);
}


/*
PROBAR CLIENTE
    1.sacar IP de compu
    2. Ponersela al ejemplod el cliente
    3.Iniciarlizar Socket Test como server NOTA: ver mensajes que recibe el cliente

PROBAR AMBOS MICROS
*/

/**
 * @brief Reciever Task
 * 
 * @param arg 
 */
void wifi_subtask(void *arg)
{
    cout<<"wifi_subtask"<<endl;
    Reciever* protocol = new Reciever;
    Process_Wifi_Comm(protocol);
    vTaskDelete(NULL);
}
/**
 * @brief Transmiter Task
 * 
 * @param arg 
 */
void sync_subtask(void *arg)
{
    cout<<"sync_subtask"<<endl;
    Transmiter* protocol = new Transmiter;
    Process_Wifi_Comm(protocol);
    vTaskDelete(NULL);
}

/* ============================================================================
        ███████╗██╗██╗     ███████╗    ███████╗███╗   ██╗██████╗
        ██╔════╝██║██║     ██╔════╝    ██╔════╝████╗  ██║██╔══██╗
        █████╗  ██║██║     █████╗      █████╗  ██╔██╗ ██║██║  ██║
        ██╔══╝  ██║██║     ██╔══╝      ██╔══╝  ██║╚██╗██║██║  ██║
        ██║     ██║███████╗███████╗    ███████╗██║ ╚████║██████╔╝
        ╚═╝     ╚═╝╚══════╝╚══════╝    ╚══════╝╚═╝  ╚═══╝╚═════╝
============================================================================*/
