#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include  <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include  "FileWriter.h"

#define MAX 128
#define PORT 3333
#define SA struct sockaddr 

int sockfd;
char File_names[MAX_features][MAX_NAME_SIZE] = {{"IDLE_FILE.txt"},
                                                {"RITH_FILE.txt"},
                                                {"CIRC_FILE.txt"},
                                                {"LINE_FILE.txt"}};

int send_msg(const char * msg){
    int err = write(sockfd, msg, strlen(msg));
    if (err < 0) {
        printf("ERROR OCURRED DURING SENDING");
    }
    return err;
}

int recv_msg(char* msg, int lenght){
    int len = read(sockfd, msg, lenght-1);
        // Error occured during receiving
    if (len < 0) {
        printf("RECV FAILED");
    }
    return len;
}

void func(int sockfd) 
{ 
    char buff[MAX]; 

    bzero(buff, sizeof(buff)); 

    while (send_msg("APP\n") < 0) {
        usleep(1000000);
    }
    int len = recv_msg(buff,sizeof(buff));
    memset(buff+(len-2),0,3);
    printf("From Server : %s***\n", buff);

    if (strcmp(buff,"READY TO WRITE") != 0){
        send_msg("NACK\n");
        printf("RIP CLIENT :P\n");
        return;
    }else{
        send_msg("ACK\n");
    }
    send_msg("/spiffs/IDLE.bin\n");
    usleep(1000000);
    send_msg("4(0,255,255),4(255,0,0),1000\n");
    usleep(1000000);
    send_msg("4(255,0,0),4(0,255,255),1000\n");
    usleep(1000000);
    send_msg("EOF\n");

} 
  
int main() 
{ 
    int  connfd; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("192.168.5.1"); 
    servaddr.sin_port = htons(PORT); 
  
    // connect the client socket to server socket 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("connected to the server..\n"); 
  
    // function for chat 
    func(sockfd); 
  
    // close the socket 
    close(sockfd); 
} 