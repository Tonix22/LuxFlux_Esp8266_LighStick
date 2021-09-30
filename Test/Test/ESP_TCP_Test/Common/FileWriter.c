#include "FileWriter.h"

FILE *filePointer; 

void open_file(char * file_name){
   
    filePointer = fopen(file_name, "w") ; 
}
void close_file(){
    fclose(filePointer); 
}

void write_chunk(char * buffer){
    fputs(buffer,filePointer);
}