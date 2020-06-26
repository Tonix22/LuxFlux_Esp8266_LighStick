#ifndef MENU_H
#define MENU_H




#ifdef __cplusplus
extern "C" {
#endif

void Menu();

#ifdef __cplusplus

struct Node
{
    void (*foo)();
    Node* next;
    Node& operator++()
    {
        *this = *(this->next);
        return *this;
    }
};

//Prototypes
void idle_func();
void rith_func();
void circ_func();
void leve_func();
void wifi_func();
void sync_func();



typedef enum{
    IDLE,
    RITH,
    CIRC,
    LEVEL,
    WIFI,
    SYNC,
    ARRAYSIZE,
    }Menu_t;

}
#endif
#endif