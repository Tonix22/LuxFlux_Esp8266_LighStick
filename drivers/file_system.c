/* SPIFFS filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "file_system.h"
#include "structs.h"
static const char *TAG = "example";

fpos_t current_pos;

FILE* f;
char line[64];

void file_system_init()
{
    //ESP_LOGI(TAG, "Initializing SPIFFS");
    
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
void deinit()
{
    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
    //ESP_LOGI(TAG, "SPIFFS unmounted");
}
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

void delete_file(const char* name)
{
    unlink(name);
}

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

void write_chunck(void* data, char data_type_size, int data_size)
{   
    if(f !=NULL)
    {
        fwrite (data , data_type_size, data_size, f);
    }
}

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
void read_line()
{
    
    fgets(line, sizeof(line), f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    
}

void write_format_string(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(f, format,args);
    va_end(args);
}
void close_file()
{
    fclose(f);
    f = NULL;
}

void save_current_pos()
{
    fgetpos(f,&current_pos);
}

void load_curren_pos()
{
    fsetpos(f,&current_pos);
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
        chunk->pixels = i;
        chunk->color.RED   = 0;
        chunk->color.GREEN = 0;
        chunk->color.BLUE  = 255;
        write_chunck(chunk,sizeof(Block),1);
        if(i !=8)
        {
            chunk->pixels = 8-i;
            chunk->color.RED   = 0;
            chunk->color.GREEN = 0;
            chunk->color.BLUE  = 0;
            write_chunck(chunk,sizeof(Block),1);
        }
        time = 60;
        write_chunck(&time,sizeof(uint32_t),1);
    }
    for(int i=1;i<=8;i++)
    {
        chunk->pixels = i;
        chunk->color.RED   = 0;
        chunk->color.GREEN = 0;
        chunk->color.BLUE  = 0;
        write_chunck(chunk,sizeof(Block),1);
        if(i !=8)
        {
            chunk->pixels = 8-i;
            chunk->color.RED   = 0;
            chunk->color.GREEN = 0;
            chunk->color.BLUE  = 255;
            write_chunck(chunk,sizeof(Block),1);
        }
        time = 60;
        write_chunck(&time,sizeof(uint32_t),1);
    }
    close_file();
    printf("write end\r\n");
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
/*

void Example1()
{
    
    static char buffer[] = { 'x' , 'y' , 'z' , '\n'};
    static char rcv_buffer[64];

    file_open(WRITE, IDLE_FILE);// First create a file.
   
    write_chunck(buffer,sizeof(char),4);
    write_format_string("Mama soy un pez\n");
   
    close_file();
    //ESP_LOGI(TAG, "File written");
  
    // Open renamed file for reading

    file_open(READ, IDLE_FILE);

    read_chunk(rcv_buffer,sizeof(char),64);
    close_file();
}
 void Example_struct()
 {
    Data* chunk =  malloc(sizeof(Data));
    chunk->B = 30;
    chunk->G = 50;
    chunk->R = 240;
    
    file_open(WRITE, LINE_FILE);// First create a file.

    write_chunck(chunk,sizeof(Data),1);//only write on struct

    close_file();

    //clear chunk
    memset(chunk,0,sizeof(Data));
    //check if was written in flash 
    file_open(READ, LINE_FILE);

    read_chunk(chunk,sizeof(Data),1);
    printf("chunk->B: %d\r\n",chunk->B);
    printf("chunk->G: %d\r\n",chunk->G);
    printf("chunk->R: %d\r\n",chunk->R);

    free(chunk);
 }
*/
