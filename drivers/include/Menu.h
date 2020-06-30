#ifndef MENU_H
#define MENU_H

#include "FreeRTOS_wrapper.h"


#ifdef __cplusplus
extern "C" {

#endif


void Menu_func();
void sync_action(TimerHandle_t xTimer );

#ifdef __cplusplus

typedef void (*foo_ptr)();
struct Node
{
    foo_ptr val;
    Node* next;
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
void idle_func();
void rith_func();
void circ_func();
void leve_func();
void wifi_func();
void sync_func();

class DispMenu
{
    Node screens[ARRAYSIZE] = 
                        { 
                            {idle_func,&(screens[RITH] ) },
                            {rith_func,&(screens[CIRC] ) },
                            {circ_func,&(screens[LEVEL]) },
                            {leve_func,&(screens[WIFI] ) }, 
                            {wifi_func,&(screens[IDLE] ) },
                            {sync_func,&(screens[IDLE] ) },
                        };
    Node* screen = &(screens[IDLE]);

    public:
    DispMenu& operator++(int)
    {
        screen = screen->next;
        return *this;
    }
    //a way to acces to a function by index
    void operator[](std::size_t idx)
    {
        this->screen = &(screens[idx]);//update screen
        screens[idx].val(); //go to screen
    }
    void operator ( ) () // Menu()
    {
        this->screen->val();
    }
};

}
#endif
#endif