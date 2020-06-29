#include <iostream>
#include "Menu.h"
using namespace std;

 DispMenu Menu;

//class declaration


// functions
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



void Menu_func()
{
    /* 
    This is a way to acces menu directly by index, 
    used to branch to a specific menu function. 
    For example when a button is pressed by 3
    seconds, the fucntion call will be directly like this.
    */
    //Menu[SYNC];// Acces directly to function
    
    Menu(); // Jump to function like this
    Menu++; // Incremt position like this
       
}