#ifndef FILEYSYSTEM_H
#define FILEYSYSTEM_H


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"



#define BASE_PATH "/spiffs"

#define DIR(NAME) BASE_PATH"/"#NAME".txt"

#define IDLE_FILE DIR(IDLE)
#define RITH_FILE DIR(RITH)
#define CIRC_FILE DIR(CIRC)
#define LINE_FILE DIR(LINE)


typedef enum
{
    WRITE,
    READ,
}File_action;

struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint32_t time_ms;
}typedef Data;


inline void file_system_init();
inline void deinit();
inline bool check_file_exist(const char* name);
void delete_file(const char* name);
void file_open(File_action r_w, char* name);
void write_chunck(void* data, char data_type_size, int data_size);
void read_chunk(void* data, char data_type_size,char num_of_elements);
void read_line();
void write_format_string(const char* format, ...);
inline void close_file();
void Example1();
void Example_struct();



#endif