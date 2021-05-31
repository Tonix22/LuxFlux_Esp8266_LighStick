// =============================================================================
// Includes
// =============================================================================

#include <stdio.h>
#include <ctype.h>
#include <vector>
#include <list>
#include "FreeRTOS_wrapper.h"
#include "memory_admin.h"
#include "Light_effects.h"
#include "structs.h"
#include "file_system.h"


// =============================================================================
// Function prototypes
// =============================================================================
inline void rd_flash_wr_class(feature_t feature);


// =============================================================================
// Variables
// =============================================================================
extern Light* LedStick;
EventGroupHandle_t Flash_status;

char File_names[MAX_features][MAX_NAME_SIZE] = {{IDLE_FILE},
                                                {RITH_FILE},
                                                {CIRC_FILE},
                                                {LINE_FILE}};

// =============================================================================
// Functions
// =============================================================================

/** 
 * @brief
 * Allocate Flash Flag status
*/

void init_flash_status_group(void)
{
    Flash_status = xEventGroupCreate();
}

/** 
 * @brief
 * Check if file exist and 
 * update event Group Flash status.
*/

EventBits_t file_exist(feature_t feature)
{
    if(!check_file_exist(File_names[feature]))
    {
        xEventGroupSetBits(Flash_status, EMPTYFILE);
    }
    return xEventGroupGetBits(Flash_status);
}

/** 
 * @brief
 * Read secuencue and update data
 * 
*/

void file_read(feature_t feature)
{
    file_open(READ, File_names[feature]);
    rd_flash_wr_class(feature);
    close_file();
}

/*
Reads from flash , and write data into a linked list.
+-----+-------+------+
| Red | Green | Blue | --> RGB
+-----+-------+------+

+--------+------+
| Pixels | RGB  | --> BLOCK
+--------+------+

+-------+-------+----+-------------+
| time  | group | .. | list<Block> | --> Frame
+-------+-------+----+-------------+

+--------+---------+-----+--------------+
| Frame1 | Frame2  | ... | list<Frames> | --> Sequence
+--------+---------+-----+--------------+

*/
inline void rd_flash_wr_class(feature_t feature)
{
    bool valid      = true;
    int  frames_cnt = 0;
    char pixels_cnt = 0;
    Block* chunk    = new(Block); 
    Frame* frame    = new(Frame);

    while(valid && frames_cnt < LedStick->max_frames) // Read until buffer is full
    {
        while(pixels_cnt < LedStick->pixels) // Read untill all pixels in frame are ready
        {
            valid = read_chunk(chunk, sizeof(Block),1);
            if(valid)
            {
                pixels_cnt+=chunk->pixels;
                frame->group.push_back(*chunk); // push pixels in a group until fill ledstick
                //DEBUG :)
               // printf("CHUNK: PIXELS %d, R= %d,G= %d,B= %d \r\n",chunk->pixels,chunk->color.RED,chunk->color.GREEN,chunk->color.BLUE);
            }
            else {goto Terminate;} // frame is invalid or file is and its end
        }
        if(pixels_cnt == LedStick->pixels)//apend time of block
        {
            pixels_cnt = 0;
            valid = read_chunk(&(frame->time), sizeof(uint32_t),1); // get Frame time
            //DEBUG :)
           // printf("TIME: %d",frame->time);
        }
        else // invalid Time format, last data is ever time
        {
            xEventGroupSetBits(Flash_status, BAD_FORMAT);
            goto Terminate;
        }
        //Frame is Done now push it to sequence
        if(valid)
        {
            LedStick->sequence.push_back(*frame); // push frame to light sequence
            frame->group.clear(); // Free memory of group allocated
            frames_cnt++; // incremente frames count
        }
    }
    Terminate: // goto Label here goes when something fail or when reading is finished
    frame->group.clear();
    delete(chunk);
    delete(frame);
}

/** 
 * @brief
 * 
 * 
*/
bool parse_chunk(char * msg){
    int pixel_cnt=0, time_frame=0;
    Block *group = new (Block);
    char * ptr;
      
    //printf ("Splitting string \"%s\" into tokens:\n",msg[i]);
    //PRIMER BLOQUE
    ptr = strtok (msg,"(,)");
    
    while(pixel_cnt < 8 && ptr!=NULL){

        if(isdigit(*ptr)){

            group->pixels = atoi(ptr);
            
            pixel_cnt += group->pixels;

            PARSE_COLOR(RED); 
            PARSE_COLOR(GREEN);
            PARSE_COLOR(BLUE);
            TOKEN();

            //frame.group.push_back(group); // insert group in block
           // printf("%d(%d,%d,%d),",group->pixels, group->color.RED, group->color.GREEN, group->color.BLUE); // ()

            write_chunck(group,sizeof(Block),1);
        }else{
            delete group;
            ptr = NULL;
            return false;
        }
    }

    if(pixel_cnt != 8){
        return false;
    }
    time_frame = atoi(ptr);
    
   // printf("%d\r\n",time_frame);
    write_chunck(&time_frame,sizeof(uint32_t),1);

    delete group;
    ptr = NULL;
    group = NULL;
    return true;
}

int get_pixels (){
    return LedStick->pixels;
}