#include "FileReader.h"

FILE *filePointer; 

void open_file(){
   
    filePointer = fopen("color_patern.txt", "r") ; 
}
void close_file(){
    fclose(filePointer); 
}

int get_chunk(char * buffer, size_t size){
    
    memset(buffer,0,size);
    return getline(&buffer,&size,filePointer);

}
/*
int main () 
{ 
    char dataToBeRead[80]; 

    open_file();
    while(get_chunk(dataToBeRead,sizeof(dataToBeRead)) >= 0){
  
        printf("%s\r\n",dataToBeRead);

    }

    close_file();
 
   
    // Declare the file pointer 
    FILE *filePointer ; 
      
    // Declare the variable for the data to be read from file 
  
    // Open the existing file GfgTest.c using fopen() 
    // in read mode using "r" attribute 
    filePointer = fopen("color_patern.txt", "r") ; 
      
    // Check if this filePointer is null 
    // which maybe if the file does not exist 
    if ( filePointer == NULL ) 
    { 
        printf( "color_patern.txt file failed to open." ) ; 
    } 
    else
    { 
          
        printf("The file is now opened.\n") ; 
          
        // Read the dataToBeRead from the file 
        // using fgets() method 
        while( fgets ( dataToBeRead, 50, filePointer ) != NULL ) 
        { 
          
            // Print the dataToBeRead  
            printf( "%s" , dataToBeRead ) ; 
         } 
          
        // Closing the file using fclose() 
        fclose(filePointer) ; 
          
        printf("\nData successfully read from file color_patern.txt\n"); 
        printf("The file is now closed.\n") ; 
    } 
    return 0; 
} 
*/