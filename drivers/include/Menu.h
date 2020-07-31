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
#include "tcp_server.h"
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
void idle_subtask(void *arg);
void rith_subtask(void *arg);
void circular_subtask(void *arg);
void level_subtask(void *arg);
void wifi_subtask(void *arg);
void sync_subtask(void *arg);

class DispMenu
{
    public:
    Node screens[ARRAYSIZE] = 
                        { 
                            {idle_subtask,    &(screens[RITH] ), IDLE },
                            {rith_subtask,    &(screens[CIRC] ), RITH },
                            {circular_subtask,&(screens[LEVEL]), CIRC },
                            {level_subtask,   &(screens[WIFI]),  LEVEL}, 
                            {wifi_subtask,    &(screens[IDLE] ), WIFI },
                            {sync_subtask,    &(screens[IDLE] ), SYNC },
                        };
                       
    char names[ARRAYSIZE][15] = 
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
        xTaskCreate(this->screen->val, names[idx], 1024, NULL, 5, NULL);
        //screens[idx].val(); //go to screen
    }
    void operator ( ) () // Menu()
    {
        //this->screen->val();
        xTaskCreate(this->screen->val, names[this->screen->id], 1024, NULL, 5, NULL);
    }
};

}
#endif
#endif