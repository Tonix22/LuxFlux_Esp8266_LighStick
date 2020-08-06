#include <stdio.h>
#include <vector>
#include "memory_admin.h"
#include "Light_effects.h"
#include "structs.h"
extern Light* LedStick;

void write_to_class(feature_t feature);

void check_files_and_read()
{
    if(check_file_exist(IDLE_FILE))
    {
        file_open(READ, IDLE_FILE);
        write_to_class(IDLE_feature);
        close_file();
    }
    if(check_file_exist(RITH_FILE))
    {
        file_open(READ, RITH_FILE);
        write_to_class(RITH_feature);
        close_file();
    }
    if(check_file_exist(CIRC_FILE))
    {
        file_open(READ, CIRC_FILE);
        write_to_class(CIRCULAR_feature);
        close_file();
    }
    if(check_file_exist(LINE_FILE))
    {
        file_open(READ, LINE_FILE);
        write_to_class(LINEAR_feature);
        close_file();
    }
}

/*
Feature_collection is an (array of iterators)
Each iteator points to a feature begin.
Each feature is a vector of RGB
*/
void write_to_class(feature_t feature)
{
    bool valid  = true;
    int counter = 0;
    char size   = sizeof(RGB);
    if(feature == IDLE_feature)
    {
        size = sizeof(RGBT);
    }        
    while(valid && counter < MAX_RGB)
    {
        valid = read_chunk(&*(LedStick->feature_collection[feature]), size,1);
        printf("[%d, ",LedStick->sound_light[counter].RED);
        printf("%d, ",LedStick->sound_light[counter].GREEN);
        printf("%d ]\r\n",LedStick->sound_light[counter].BLUE);
        LedStick->feature_collection[feature]++;
        counter++;
    }

}