#ifndef MENU_H
#define MENU_H

#include "FreeRTOS_wrapper.h"


#ifdef __cplusplus
extern "C" {

#endif
// =============================================================================
// C Visible Function prototypes
// =============================================================================
void Menu_func();
void sync_action(TimerHandle_t xTimer );

#ifdef __cplusplus
// =============================================================================
// C++ Includes
// =============================================================================
#include "IO_driver.h"
#include "imu6050.h"
#include "FreeRTOS_wrapper.h"
#include "wifi.h"
#include "config.h"
#include "structs.h"
#include "memory_admin.h"
#include "Light_effects.h"
#include "tcp_server.h"
#include "tcp_client.h"

// =============================================================================
// EVENT GROUPS BITS DEFINES
// =============================================================================

#define ABORT BIT0
#define TASKDEATH BIT1
#define IDLE_BUFFER_END BIT2
// =============================================================================
// DEFINES
// =============================================================================
#define CLEAR_BIT 0
#define SET_BIT 1

#if DEBUG_MENU
#define DEBUG_EMPTY_FILE()  else\
                            {\
                                printf("EMPTY FILE IDLE DONE\r\n");\
                            }
#else
#define DEBUG_EMPTY_FILE()
#endif

// =============================================================================
// Extern
// =============================================================================

extern EventGroupHandle_t Flash_status;
extern xQueueHandle Light_event;
// =============================================================================
// Type definitions
// =============================================================================

typedef void (*foo_ptr)(void *);
struct Node
{
    foo_ptr val;
    Node* next;
    unsigned char id;
};
    //Definitions
typedef enum{
    IDLE,
    RITH,
    CIRC,
    LEVEL,
    WIFI,
    SYNC,
    ARRAYSIZE,
}Screen_t;


typedef enum
{
    STOP,
    RUN,
}Service_steps;

// =============================================================================
// C++ Function prototypes
// =============================================================================

void abort_calib();
void abort_last_menu();
void idle_subtask(void *arg);
void rith_subtask(void *arg);
void circular_subtask(void *arg);
void level_subtask(void *arg);
void wifi_subtask(void *arg);
void sync_subtask(void *arg);

/* =============================================================================
             ██████╗██╗      █████╗ ███████╗███████╗███████╗███████╗
            ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝
            ██║     ██║     ███████║███████╗███████╗█████╗  ███████╗
            ██║     ██║     ██╔══██║╚════██║╚════██║██╔══╝  ╚════██║
            ╚██████╗███████╗██║  ██║███████║███████║███████╗███████║
            ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚══════╝
  =============================================================================*/
/**
 * @brief 
 * Abstract and parent Class. It will be used by features that
 * need reding data from flash and take actions with
 * colors data
 */
class FeatureBehaviour 
{ 
    public:
    feature_t   effect;
    EventBits_t file_mask;
    bool file_found;
    bool load_status;
    bool cyclic;
    /**
     * @brief Each children class will specify this method
     */
    virtual void run_feature_read()   = 0;
    /**
     * @brief Open file and check if exist
     * @param effect Select File to open
     */
    void check_file(feature_t& effect)
    {
        this->effect = effect;
        file_mask  = file_exist(this->effect);
        file_found = (file_mask & EMPTYFILE);
        clear_file_cursor();

    }
    /**
     * @brief Read data form Flash is file succeed
     */
    void load_feature_file()
    {
        file_read(effect);
        file_mask   = xEventGroupGetBits(Flash_status);
        load_status = (file_mask & BAD_FORMAT);
    }
    /**
     * @brief Destroy the Feature Behaviour object
     * File cursor is set at the begging
     * light will be off
     * clear event flag bits
     */
    virtual ~FeatureBehaviour()
    {
        clear_file_cursor();
        Pixels_OFF();
        xEventGroupClearBits(Flash_status,BAD_FORMAT|EMPTYFILE);
    }
};
/**
 * @brief IDLE light specialization
 */
class IDLE_Light : public FeatureBehaviour
{   
    public:
    EventBits_t menu_mask = 0;
    IDLE_Light(feature_t mode)
    {
        printf("IDLE\r\n");
        check_file(mode);
    }
    /**
     * @brief IDLE Mask will abort only when
     * menu_mask bit is in abort
     */
    void run_feature_read()
    {
        menu_mask = IDLE_light();
        cyclic    = (menu_mask & ABORT);
    }
};
/**
 * @brief Ritmic especialization
 */
class Riht_Light : public FeatureBehaviour
{
    public: 
    Riht_Light(feature_t mode)
    {
        //expands buffer because it only will pass to read once
        Set_Frames_buffer(20);
        check_file(mode);
    }
    /**
     * @brief Ritmic only pass once and enables ISR
     * 
     */
    void run_feature_read()
    {
        input_IO_enable_isr(GPIO_SDD2, GPIO_INTR_NEGEDGE);
        cyclic = SET_BIT;
    }
    //set buffer to normal size
    ~Riht_Light(){Set_Frames_buffer(10);}
};
/**
 * @brief IMU circular especialization
 * 
 */
class Circular_Light: public FeatureBehaviour
{
    public:
    EventGroupHandle_t hold;
    Circular_Light(feature_t mode, EventGroupHandle_t& menu_stat)
    {
        Set_Frames_buffer(36);
        check_file(mode);
        this->hold = menu_stat;
    }
    void run_feature_read() //TODO
    {
        Light_MessageID imu_to_led_msg = CIRC_LOAD;
        xQueueSend(Light_event, &imu_to_led_msg, 0);
        EventBits_t kill = xEventGroupWaitBits(this->hold,TASKDEATH,
            pdTRUE,// Clear Flag
            pdFALSE,
            portMAX_DELAY); // wait until IDLE Task is death
            
    }
};

/**
 * @brief Abstract class for Wifi Routines
 */

class Channel
{
    public:
    Service_steps state = RUN;
    virtual void clear_last_menu()   = 0; // clear last task
    virtual void init_wifi_service() = 0; // AP or Statiton
    virtual void init_comm_service() = 0; // client or server
    virtual void set_state()         = 0; // END loop condition
    virtual ~Channel(){}
};
/**
 * @brief Class as AP and server 
 */
class Reciever: public Channel
{
    public:
    Reciever() // TODO
    {

    }
    void clear_last_menu()
    {
        abort_calib();
        abort_last_menu();
    }
    void init_wifi_service()
    {
        wifi_init_softap();
    }
    void init_comm_service()
    {
        server_init();
    }
    void set_state() // TODO insert logic here
    {

    }
    ~Reciever()
    {
        wifi_deint_ap();
    }
};
/**
 * @brief Class as station and client 
 */
class Transmiter: public  Channel
{
    public:
    Transmiter()
    {

    }
    void clear_last_menu()
    {
        input_IO_disable_isr(GPIO_SDD2);
        abort_last_menu();
        abort_calib();
    }
    void init_wifi_service()
    {
        wifi_init_sta();
    }
    void init_comm_service()
    {
        client_init();
    }
    void set_state()// TODO insert logic here
    {
    
    }
    ~Transmiter()
    {
        esp_restart();
    }

};
/**
 * @brief 
 * This class takes the Menu state behaviouir
 * works as static list which contains a task routin, 
 * and ENUM as index of the corresponding task
 * 
 * Also has a next pointer which means which will be the next menu
 */

class DispMenu
{
    public:
    Node screens[ARRAYSIZE] = 
                        { 
                            //Task routine    //Next Menu       //Enum
                            {idle_subtask,    &(screens[RITH] ), IDLE },
                            {rith_subtask,    &(screens[CIRC] ), RITH },
                            {circular_subtask,&(screens[LEVEL]), CIRC },
                            {level_subtask,   &(screens[WIFI] ), LEVEL}, 
                            {wifi_subtask,    &(screens[IDLE] ), WIFI },
                            {sync_subtask,    &(screens[IDLE] ), SYNC },
                        };
    /**
     * @brief 
     * Task Menu names table
     */
    const char names[ARRAYSIZE][15] = 
    {
        "idle_subtask",
        "rith_subtask",
        "circ_subtask",
        "level_subtask",
        "wifi_subtask",
        "sync_subtask",
    };
    
    Node* screen = &(screens[IDLE]); // pointer tu current menu state

    DispMenu& operator++(int) // overload ++ to fetch next menu routine
    {
        screen = screen->next;//go to next screen
        return *this;
    }
    //a way to acces to a function by index
    void operator[](std::size_t idx) // Take Menu enumeration as direct index of the screen array
    {
        this->screen = &(screens[idx]); // set up by index
        xTaskCreate(this->screen->val, names[idx], 2048, NULL, 5, NULL);
    }
    void operator ( ) () // Create task as a parallel excution, 
                         //routine is based on the screen pointer
    {
        xTaskCreate(this->screen->val, names[this->screen->id], 2048, NULL, 5, NULL);
    }
    void first_time();


};

}
#endif
#endif