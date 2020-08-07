#include <stdio.h>
#include <vector>
#include <list>
#include "memory_admin.h"
#include "Light_effects.h"
#include "structs.h"
extern Light* LedStick;

template <typename T>
void write_to_class_gen(feature_t feature);

void check_files_and_read()
{
    if(check_file_exist(IDLE_FILE))
    {
        file_open(READ, IDLE_FILE);
        write_to_class_gen<RGBT>(IDLE_feature);
        close_file();
    }
    if(check_file_exist(RITH_FILE))
    {
        file_open(READ, RITH_FILE);
        write_to_class_gen<RGB>(RITH_feature);
        close_file();
    }
    if(check_file_exist(CIRC_FILE))
    {
        file_open(READ, CIRC_FILE);
        write_to_class_gen<RGB>(CIRCULAR_feature);
        close_file();
    }
    if(check_file_exist(LINE_FILE))
    {
        file_open(READ, LINE_FILE);
        write_to_class_gen<RGB>(LINEAR_feature);
        close_file();
    }
}


/*
    debug a color
    printf("[%d, ",(LedStick->feature_collection[feature]->back()).RED);
    printf(",%d, ",(LedStick->feature_collection[feature]->back()).GREEN);
    printf(",%d], ",(LedStick->feature_collection[feature]->back()).BLUE);
*/
void push_to_class(RGB val,feature_t feature)
{
    LedStick->feature_collection[feature]->push_back(val);
}
/*
    debug a color
    printf("[%d, ",(LedStick->idle_light.back()).RED);
    printf(",%d, ",(LedStick->idle_light.back()).GREEN);
    printf(",%d, ",(LedStick->idle_light.back()).BLUE);
    printf(",%d], ",(LedStick->idle_light.back()).time_ms);
*/

void push_to_class(RGBT val, feature_t feature)
{
    LedStick->idle_light.push_back(val);
}

/*

Reads from flash , and write data into a linked list.
Each element of the list is an RGB or RGBT struct.
Lists are found insde de class and should have a max size of 10.
If file is not yet in EOF 

*/
template <typename T>
void write_to_class_gen(feature_t feature)
{
    bool valid  = true;
    int counter = 0;
    T* chunk  = new(T);
    while(valid && counter < MAX_RGB)
    {
        valid = read_chunk(chunk, sizeof(T),1);
        push_to_class(*chunk,feature);
        counter++;
    }
    delete(chunk);
}