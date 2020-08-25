#ifndef MENU_H
#define MENU_H

#include "FreeRTOS_wrapper.h"


#ifdef __cplusplus
extern "C" {

#endif

void Menu_func();
void sync_action(TimerHandle_t xTimer );

#ifdef __cplusplus
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

#define ABORT BIT0
#define TASKDEATH BIT1
#define IDLE_BUFFER_END BIT2

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


extern EventGroupHandle_t Flash_status;



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

//Prototypes

void abort_calib();
void abort_last_menu();

void idle_subtask(void *arg);
void rith_subtask(void *arg);
void circular_subtask(void *arg);
void level_subtask(void *arg);
void wifi_subtask(void *arg);
void sync_subtask(void *arg);

class FeatureBehaviour 
{ 
    public:
    feature_t   effect;
    EventBits_t file_mask;
    bool file_found;
    bool load_status;
    bool cyclic;

    virtual void run_feature_read()   = 0;

    void check_file(feature_t& effect)
    {
        this->effect = effect;
        file_mask  = file_exist(this->effect);
        file_found = (file_mask & EMPTYFILE);
        clear_file_cursor();

    }
    void load_feature_file()
    {
        file_read(effect);
        file_mask   = xEventGroupGetBits(Flash_status);
        load_status = (file_mask & BAD_FORMAT);
    }

    virtual ~FeatureBehaviour()
    {
        clear_file_cursor();
        Pixels_OFF();
        xEventGroupClearBits(Flash_status,BAD_FORMAT|EMPTYFILE);
    }
};
class IDLE_Light : public FeatureBehaviour
{   
    public:
    EventBits_t menu_mask = 0;
    IDLE_Light(feature_t mode)
    {
        printf("IDLE\r\n");
        check_file(mode);
    }
    void run_feature_read()
    {
        menu_mask = IDLE_light();
        cyclic    = (menu_mask & ABORT);
    }
};
class Riht_Light : public FeatureBehaviour
{
    public: 
    Riht_Light(feature_t mode)
    {
        Set_Frames_buffer(20);
        check_file(mode);
    }
    void run_feature_read()
    {
        input_IO_enable_isr(GPIO_SDD2, GPIO_INTR_NEGEDGE);
        cyclic = SET_BIT;
    }
    ~Riht_Light(){Set_Frames_buffer(10);}
};

class Circular_Light: public FeatureBehaviour
{
    public:
    Circular_Light(feature_t mode)
    {
        check_file(mode);
    }
    void run_feature_read()
    {

    }
};
typedef enum
{
    STOP,
    RUN,
}Service_steps;
class Channel
{
    public:
    Service_steps state = RUN;
    virtual void clear_last_menu()   = 0;
    virtual void init_wifi_service() = 0;
    virtual void init_comm_service() = 0;
    virtual void set_state()         = 0;
    virtual ~Channel(){}
};
class Reciever: public Channel
{
    public:
    Reciever()
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
        //server_init();
    }
    void set_state()
    {

    }
    ~Reciever()
    {
        wifi_deint();
    }
};
class Transmiter: public  Channel
{
    public:
    Transmiter()
    {

    }
    void clear_last_menu()
    {
        input_IO_disable_isr(GPIO_SDD2);
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
    void set_state()
    {

    }
    ~Transmiter()
    {
        esp_restart();
    }

};


class DispMenu
{
    public:
    Node screens[ARRAYSIZE] = 
                        { 
                            {idle_subtask,    &(screens[RITH] ), IDLE },
                            {rith_subtask,    &(screens[CIRC] ), RITH },
                            {circular_subtask,&(screens[LEVEL]), CIRC },
                            {level_subtask,   &(screens[WIFI] ), LEVEL}, 
                            {wifi_subtask,    &(screens[IDLE] ), WIFI },
                            {sync_subtask,    &(screens[IDLE] ), SYNC },
                        };             
    const char names[ARRAYSIZE][15] = 
    {
        "idle_subtask",
        "rith_subtask",
        "circ_subtask",
        "level_subtask",
        "wifi_subtask",
        "sync_subtask",
    };
    
    Node* screen = &(screens[IDLE]);

    DispMenu& operator++(int)
    {
        screen = screen->next;
        return *this;
    }
    //a way to acces to a function by index
    void operator[](std::size_t idx)
    {
        this->screen = &(screens[idx]);
        xTaskCreate(this->screen->val, names[idx], 2048, NULL, 5, NULL);
        //screens[idx].val(); //go to screen
    }
    void operator ( ) () // Menu()
    {
        //this->screen->val();
        xTaskCreate(this->screen->val, names[this->screen->id], 2048, NULL, 5, NULL);
    }
    void first_time();


};

}
#endif
#endif