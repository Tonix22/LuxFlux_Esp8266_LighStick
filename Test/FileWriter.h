#ifndef FILEWRITER_H
#define FILEWRITER_H
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#define MAX_features 4
#define MAX_NAME_SIZE 17

void open_file();
void close_file();
void write_chunk(char * buffer);


#endif