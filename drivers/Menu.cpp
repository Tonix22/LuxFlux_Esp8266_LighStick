#include <iostream>
#include "Menu.h"

using namespace std;

Node* Rooth = NULL;
Node* END   = NULL; // helper pointer
Node* aux   = NULL;

Node menu[ARRAYSIZE] = {{idle_func,&(menu[RITH])},
                        {rith_func,&(menu[CIRC])},
                        {circ_func,&(menu[LEVEL])},
                        {leve_func,&(menu[WIFI])}, 
                        {wifi_func,&(menu[SYNC])},
                        {sync_func,&(menu[IDLE])},

                       };


Node* curr_screen = &(menu[IDLE]);

void idle_func(){
    cout<<"idle_func"<<endl;
}

void rith_func(){
    cout<<"rith_func"<<endl;
}

void circ_func(){
    cout<<"circ_func"<<endl;
}

void leve_func(){
    cout<<"leve_func"<<endl;
}

void wifi_func(){
    cout<<"wifi_func"<<endl;
}

void sync_func(){
    cout<<"sync_func"<<endl;
}



void Menu()
{
    
    curr_screen->foo();
    curr_screen++;

    
}