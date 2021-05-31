#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include "FileReader.h"
#include  <unistd.h>

#define MAX 80 
#define PORT 3333 
#define SA struct sockaddr 

// Function designed for chat between client and server. 
void func(int sockfd) 
{ 
	char buff[MAX]; 
	int n, valid_msg=0; 
	
	// infinite loop for chat 
	bzero(buff, MAX); 
	// read the message from client and copy it in buffer 
	read(sockfd, buff, sizeof(buff)); 

	//SYNC MESSAGE
	printf("From client: %s\t To client : ", buff); 
	if(strcmp(buff,"SYNC\n") == 0){
		write(sockfd, "READY TO SYNC",16); 
		printf("READY TO SYNC"); 
		valid_msg = 1;
	}else if(strcmp(buff,"FOTA\n") == 0){
		write(sockfd, "READY TO FOTA",16); 
		valid_msg = 1;
	}else{
		write(sockfd, "NACK",5); 
	}
	open_file();

	while (valid_msg) { 
		//RECEIVE FILE
		bzero(buff, MAX); 
		read(sockfd, buff, sizeof(buff)); 
		printf("From client: %s\n", buff);
		
		bzero(buff, MAX);
		int eof = get_chunk(buff,sizeof(buff));
		printf("To client: %s\n", buff);
		if(eof < 0){
			break;
		}
		printf("CHUNK: %s\n",buff );
		write(sockfd, buff, sizeof(buff)); 
	} 
	//IDLE
	close_file();
	write(sockfd, "EOF\n", 5); 
	usleep(1000000);

	//RITH
	bzero(buff, MAX); 
	read(sockfd, buff, sizeof(buff)); 
	printf("From client: %s\n", buff);
	write(sockfd, "EOF\n", 5);
	usleep(1000000);

	//CIRC
	bzero(buff, MAX); 
	read(sockfd, buff, sizeof(buff)); 
	printf("From client: %s\n", buff); 
	write(sockfd, "EOF\n", 5); 
	usleep(1000000);

	//LINEAR
	bzero(buff, MAX); 
	read(sockfd, buff, sizeof(buff)); 
	printf("From client: %s\n", buff); 
	write(sockfd, "EOF\n", 5); 
	usleep(1000000); 
 
} 

// Driver function 
int main() 
{ 
	
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
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
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 1)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, &len); 
	if (connfd < 0) { 
		printf("server acccept failed...\n"); 
		exit(0); 
	} 
	else
		printf("server acccept the client...\n"); 

	// Function for chatting between client and server 
	func(connfd); 

	// After chatting close the socket 
	close(sockfd); 
} 
