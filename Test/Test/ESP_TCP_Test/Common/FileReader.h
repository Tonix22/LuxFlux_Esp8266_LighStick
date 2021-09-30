#ifndef FILEREADER_H
#define FILEREADER_H
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
void open_file();
void close_file();
int get_chunk(char * buffer, size_t size);

#endif
