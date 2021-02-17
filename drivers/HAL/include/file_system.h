#ifndef FILEYSYSTEM_H
#define FILEYSYSTEM_H

// =============================================================================
// includes
// =============================================================================

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

// =============================================================================
// Defines
// =============================================================================

#define MAX_FILES 5
#define MAX_NAME_SIZE 17

#define BASE_PATH "/spiffs"

#define DIR(NAME) BASE_PATH"/"#NAME".bin"

#define IDLE_FILE DIR(IDLE) // /spiffs/IDLE.bin
#define RITH_FILE DIR(RITH) // /spiffs/RITH.bin
#define CIRC_FILE DIR(CIRC) // /spiffs/CIRC.bin
#define LINE_FILE DIR(LINE) // /spiffs/LINE.bin

#define BEGING_OF_FILE 0

// =============================================================================
// Typedefs
// =============================================================================

typedef enum
{
    WRITE,
    READ,
}File_action;
// =============================================================================
// Function prototypes
// =============================================================================

//CURSOR
void clear_file_cursor();
void save_current_pos();
void load_curren_pos();
//INIT/DENIT
void file_system_init();
void deinit();
//MANIPULATE FILE
bool check_file_exist(const char* name);
void delete_file(const char* name);
void file_open(File_action r_w, const char* name);
void write_chunck(void* data, char data_type_size, int data_size);
bool read_chunk(void* data, char data_type_size,char num_of_elements);
void write_format_string(const char* format, ...);
void close_file();
//TESTING
void write_to_idle();
void write_to_rith();
void write_to_circ();
#endif