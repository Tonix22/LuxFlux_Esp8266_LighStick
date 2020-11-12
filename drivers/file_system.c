/* SPIFFS filesystem
   This code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// =============================================================================
// includes
// =============================================================================

#include "file_system.h"
#include "structs.h"

// =============================================================================
// local variables
// =============================================================================
static const char *TAG = "example";

fpos_t current_pos;
FILE* f;

/**
 * @brief Init Flash driver system
 */
void file_system_init()
{
    esp_vfs_spiffs_conf_t conf = 
    {
      .base_path = BASE_PATH,
      .partition_label = NULL,
      .max_files = MAX_FILES,
      .format_if_mount_failed = true
    };
    
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) 
    {
        if (ret == ESP_FAIL) 
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } 
        else if (ret == ESP_ERR_NOT_FOUND) 
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } 
        else 
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
       ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

/**
 * @brief Deinit Flash driver system
 */
void deinit()
{
    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
}

/**
 * @brief 
 * 
 * @param name --> file name 
 * @return true --> file exist
 * @return false --> file don't exist
 */
bool check_file_exist(const char* name)
{
    struct stat st;
    bool state = false;
    if (stat(name, &st) == 0) 
    {
        state = true;
    }
    //printf("file exist: %s \r\n",state?("Yes"):("NO"));
    return state;
}
/**
 * @brief Delete file
 * This Api shall be call if file exist
 * @param name 
 */
void delete_file(const char* name)
{
    unlink(name);
}
/**
 * @brief Open file
 * This Api shall be call if file exist
 * @param r_w 
 * @param name 
 */
void file_open(File_action r_w, const char* name)
{
    if(f == NULL) // file pointer is not taken
    {
        if(r_w == READ && check_file_exist(name)) // file must existe to read
        {
            
            f = fopen(name, "r"); // read    
        }
        else if(r_w == WRITE)//if file doensnt exist it creates it. 
        {
            f = fopen(name, "w"); // apend
        }
    }
}
/**
 * @brief 
 * This API shall be call only if file is open
 * @param data generic data pointer
 * @param data_type_size size of type
 * @param data_size size of data
 */
void write_chunck(void* data, char data_type_size, int data_size)
{   
    if(f !=NULL)
    {
        fwrite (data , data_type_size, data_size, f);
    }
}

/**
 * @brief 
 * 
 * @param data 
 * @param data_type_size type size
 * @param num_of_elements numero of elements to store
 * @return true --> read Ok
 * @return false --> end of file detected
 */

bool read_chunk(void* data, char data_type_size,char num_of_elements)
{
    bool success_reading = true;
    int val = 0;
    if(f !=NULL)
    {
        load_curren_pos();
        val = fread(data,data_type_size,num_of_elements,f);
    }
    if(val != num_of_elements)
    {
        success_reading = false; // end of file
        current_pos = 0;
    }
    else
    {
        save_current_pos();
    }

    return success_reading;
}
/**
 * @brief writ to file as printf does
 * 
 * @param format 
 * @param ... 
 */
void write_format_string(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(f, format,args);
    va_end(args);
}
/**
 * @brief close file.
 * Only call if the file was open before
 */
void close_file()
{
    fclose(f);
    f = NULL;
}
/**
 * @brief Move cursor the beging of file
 * 
 */
void clear_file_cursor()
{
    current_pos = BEGING_OF_FILE;
}
/**
 * @brief Save curren cursor pos before close file
 * it is usefull as checkopint to continue reading
 */
void save_current_pos()
{
    fgetpos(f,&current_pos);
}
/**
 * @brief Set cursor at last position set before open the file
 * 
 */
void load_curren_pos()
{
    fsetpos(f,&current_pos);
}

// =============================================================================
/*
████████╗███████╗███████╗████████╗
╚══██╔══╝██╔════╝██╔════╝╚══██╔══╝
   ██║   █████╗  ███████╗   ██║
   ██║   ██╔══╝  ╚════██║   ██║
   ██║   ███████╗███████║   ██║
   ╚═╝   ╚══════╝╚══════╝   ╚═╝
*/
// =============================================================================

#define WRITE_BLOCK(P,R,G,B) chunk->pixels = P;\
                             chunk->color.RED   = R;\
                             chunk->color.GREEN = G;\
                             chunk->color.BLUE  = B;\
                             write_chunck(chunk,sizeof(Block),1);\
                             
#define WRITE_TIME(t)       time = t;\
                            write_chunck(&time,sizeof(uint32_t),1);\

#define RGB_8_40ms(R,G,B)  WRITE_BLOCK(8,R,G,B);\
                           WRITE_TIME(40);\


void write_to_rith()
{
    Block* chunk   =  malloc(sizeof(Block));
    uint32_t time = 0;
    if(check_file_exist(RITH_FILE))
    {
        delete_file(RITH_FILE);
    }
    file_open(WRITE, RITH_FILE);

    RGB_8_40ms(255,0,0);
    RGB_8_40ms(255,127,0);
    RGB_8_40ms(255,255,0);
    RGB_8_40ms(127,255,0);
    RGB_8_40ms(0,255,0);
    RGB_8_40ms(0,255,127);
    RGB_8_40ms(0,255,255); 
    RGB_8_40ms(0,127,255);
    RGB_8_40ms(0,0,255);
    RGB_8_40ms(127,0,255);
    RGB_8_40ms(255,0,255);
    RGB_8_40ms(255,0,127);
    close_file();
    printf("write end\r\n");
    free(chunk);
}

#define write_me
void write_to_idle()
{
    #ifdef write_me
    Block* chunk   =  malloc(sizeof(Block));
    uint32_t time = 0;
    if(check_file_exist(IDLE_FILE))
    {
        delete_file(IDLE_FILE);
    }
    file_open(WRITE, IDLE_FILE);
    for(int i=1;i<=8;i++)
    {
        WRITE_BLOCK(i,0,0,255);
        if(i !=8)
        {
            WRITE_BLOCK(8-i,0,0,0);
        }
        WRITE_TIME(30);
    }
    for(int i=1;i<=8;i++)
    {
        WRITE_BLOCK(i,0,0,0);
        if(i !=8)
        {
            WRITE_BLOCK(8-i,0,0,255);
        }
        WRITE_TIME(30);
    }
    close_file();
    printf("write end\r\n");
    free(chunk);
    #else
    Block* chunk = malloc(sizeof(Block));
    bool valid = true;
    char pixels_cnt = 0;
    uint32_t time; 
    file_open(READ, IDLE_FILE);

    while(valid)
    {
        printf("[");
        while (pixels_cnt < 8 && valid)
        {
            valid = read_chunk(chunk,sizeof(Block),1);
            printf("%d",chunk->pixels);
            printf("(%d,",chunk->color.RED);
            printf(" %d,",chunk->color.GREEN);
            printf(" %d),",chunk->color.BLUE);
            pixels_cnt+=chunk->pixels;
        }
        printf("]");
        valid = read_chunk(&time,sizeof(uint32_t),1);
        printf("%d\r\n",time);
        time = 0;
        pixels_cnt = 0;
    }
    
    close_file();
    
    #endif
}
