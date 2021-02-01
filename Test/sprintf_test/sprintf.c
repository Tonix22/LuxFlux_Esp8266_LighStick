#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

int main (){
    char arr[100], buff[100];
    sprintf(arr,"This is a format and ");
    //printf("%s\n",arr);
    sprintf(arr,"maybe is accumulating ");
    //printf("%s\n",arr);
    sprintf(arr,"or maybe not :) ");
   // printf("%s\n",arr);
    sprintf(arr,"short");
    printf("%s\n",arr);
    strcat(buff,arr);
    printf("BUFF -> %s\n", buff);
    strcat(buff,arr);
    printf("BUFF -> %s\n", buff);
}
