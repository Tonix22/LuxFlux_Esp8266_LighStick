#include <stdio.h>
#include <vector>
#include <list>
#include "FreeRTOS_wrapper.h"
#include "memory_admin.h"
#include "Light_effects.h"
#include "structs.h"


/*Defines*/


/*Function prototypes*/
inline void rd_flash_wr_class(feature_t feature);

/*Variables*/
extern Light* LedStick;
EventGroupHandle_t Flash_status;

char File_names[MAX_features][MAX_NAME_SIZE] = {{IDLE_FILE},
                                                {RITH_FILE},
                                                {CIRC_FILE},
                                                {LINE_FILE}};

void init_status_Group(void)
{
    Flash_status = xEventGroupCreate();
}

void file_exist(feature_t feature)
{
    if(!check_file_exist(File_names[feature]))
    {
        xEventGroupSetBits(Flash_status, EMPTYFILE);
    }
}

void file_read(feature_t feature)
{
    file_open(READ, File_names[feature]);
    rd_flash_wr_class(feature);
    close_file();
}

/*
Reads from flash , and write data into a linked list.

*/
inline void rd_flash_wr_class(feature_t feature)
{
    bool valid      = true;
    int  frames_cnt = 0;
    char pixels_cnt = 0;
    Block* chunk    = new(Block);
    Frame* frame    = new(Frame);

    while(valid && frames_cnt < LedStick->max_frames)
    {
        while(pixels_cnt < LedStick->pixels)
        {
            valid = read_chunk(chunk, sizeof(Block),1);
            if(valid)
            {
                pixels_cnt+=chunk->pixels;
                frame->group.push_back(*chunk);
            }
            else
            {
                goto Flush;
            }
        }
        if(pixels_cnt == LedStick->pixels)//apend time of block
        {
            pixels_cnt = 0;
            valid = read_chunk(&(frame->time), sizeof(uint32_t),1);
        }
        else
        {
            xEventGroupSetBits(Flash_status, BAD_FORMAT);
            goto Terminate;
        }
        //Frame is Done now push it to sequence
        if(valid)
        {
            LedStick->sequence.push_back(*frame);
            frame->group.clear();
            frames_cnt++;
        }
    }
    Flush:
    xEventGroupSetBits(Flash_status, READ_OK);
    Terminate:
    frame->group.clear();
    delete(chunk);
    delete(frame);
}