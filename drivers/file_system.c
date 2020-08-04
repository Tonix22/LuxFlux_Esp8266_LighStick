/* SPIFFS filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "file_system.h"

//static const char *TAG = "example";

FILE* f;
char line[64];

inline void file_system_init()
{
     //ESP_LOGI(TAG, "Initializing SPIFFS");
    
    esp_vfs_spiffs_conf_t conf = {
      .base_path = BASE_PATH,
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            //ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            //ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            //ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
       //ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        //ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}
inline void deinit()
{
    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
    //ESP_LOGI(TAG, "SPIFFS unmounted");
}
inline bool check_file_exist(const char* name)
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

void file_open(File_action r_w, char* name)
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
        //fprintf(file, "Hello World!\n");
        fwrite (data , data_type_size, data_size, f);
    }
}
void read_chunk(void* data, char data_type_size,char num_of_elements)
{
    if(f !=NULL)
    {
        fread(data,data_type_size,num_of_elements,f);
    }
}
void read_line()
{
    //memset(line,0,sizeof(line));
    fgets(line, sizeof(line), f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    //ESP_LOGI(TAG, "Read from file: '%s'", line);
}
void write_format_string(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(f, format,args);
    va_end(args);
}
inline void close_file()
{
    fclose(f);
    f = NULL;
}

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

/*
void app_main(void)
{
    file_system_init();

    Example_struct();
    
    deinit();

}
*/