/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// =============================================================================
// INCLUDES
// =============================================================================

#include <string.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "config.h"
#include "tcp_server.h"
#include "file_system.h"
#include "memory_admin.h"

extern char  File_names[MAX_features][MAX_NAME_SIZE];

// =============================================================================
// DEFINES
// =============================================================================
#define SEND_NACK message_response(&sock,"NACK\n")
#define SEND_ACK message_response(&sock,"ACK\n")
#define MAX_SIZE 128
#define PIXELS 8

static const char *TAG = "SERVER";

bool message_response(int* sock, const char* msg){
    int err = send(*sock, msg, (strlen(msg)+1), 0);
    if (err < 0) {
        ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
        return false;
    }
    return true;
}


//Chat between client ans server
void comunication(int* sock){
    char rx_buffer [MAX_SIZE];
    Block* chunk    =  malloc(sizeof(Block));
    uint32_t time   = 0;
    char pixels_cnt = 0;
    bool valid_msg  = false;
    char tempbuff [MAX_SIZE];
    int num_pixels = get_pixels();

    memset(rx_buffer,0,MAX_SIZE);// flush buffer
    int len = recv(*sock, rx_buffer, sizeof(rx_buffer),0);
    printf("From client: %s \r\n", rx_buffer); 
    //memset(rx_buffer+(len-2),0,3);

    if(strcmp(rx_buffer,"SYNC\n") == 0){
		message_response(sock,"READY TO SYNC\n");
		printf("To client: READY TO SYNC\r\n"); 
		valid_msg = true;
	}else if(strcmp(rx_buffer,"FOTA\n") == 0){
		message_response(sock,"READY TO FOTA");
		valid_msg = true;
	}else{
		message_response(sock,"NACK"); 
	}
    
    memset(rx_buffer,0,MAX_SIZE);// flush buffer
    len = recv(*sock, rx_buffer, sizeof(rx_buffer),0);
    //memset(rx_buffer+(len-2),0,3);
    if (strcmp(rx_buffer,"NACK\n") == 0){
        printf("READY TO SYNC FAILED\r\n");
        return;
    }
    printf("From client: %s \r\n", rx_buffer); 

    int i;
    while (valid_msg){

        for (i=0; i < MAX_features;i++){
            memset(rx_buffer,0,MAX_SIZE);// flush buffer
            recv(*sock, rx_buffer, sizeof(rx_buffer),0);//Receive File Name

            printf("From client: %s \r\n", rx_buffer); 

            file_open(READ,File_names[i]);

            memset(rx_buffer,0,MAX_SIZE);// flush buffer
            while(valid_msg)
            {
                
                 while (pixels_cnt < num_pixels  && valid_msg)
                {
                    valid_msg = read_chunk(chunk,sizeof(Block),1);
                    sprintf(tempbuff,"%d(%d,%d,%d),",chunk->pixels, chunk->color.RED, chunk->color.GREEN, chunk->color.BLUE);
                    strcat(rx_buffer,tempbuff);
                    pixels_cnt+=chunk->pixels;
                 }
                 if (!valid_msg){
                     break;
                 }

                valid_msg = read_chunk(&time,sizeof(uint32_t),1);
                sprintf(tempbuff,"%d\n",time);
                strcat(rx_buffer,tempbuff);
                printf("To client: %s\r\n",rx_buffer);
                message_response(sock,rx_buffer);

                memset(rx_buffer,0,MAX_SIZE);// flush buffer
                recv(*sock, rx_buffer, sizeof(rx_buffer),0);
                printf("From client: %s \r\n", rx_buffer); 

                time = 0;
                pixels_cnt = 0;
                memset(tempbuff,0,MAX_SIZE);// flush buffer
                memset(rx_buffer,0,MAX_SIZE);// flush buffer

            }

            message_response(sock,"EOF\n");
            close_file();
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }

    free(chunk);

}

/**
 * @brief 
 * 1. Prepare socket
 * 2. Socket Bind,listen and accept
 * 3. Wait First Message
 * 4. Send Response
 * 5. Loop comunication
 * @param pvParameters 
 */

static void tcp_server_task(void *pvParameters)
{
    // =============================================================================
    // 1. Prepare socket
    // =============================================================================
    int  addr_family;
    int  ip_protocol;
    

    while (1) {

        struct sockaddr_in destAddr = {
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        };
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        //inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);


        int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (listen_sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        // =============================================================================
        // 2. Socket Bind,listen and accept
        // =============================================================================

        int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket binded");

        err = listen(listen_sock, 1);
        if (err != 0) {
            ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket listening");


        struct sockaddr_in sourceAddr;

        uint addrLen = sizeof(sourceAddr);
        int sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket accepted");

        comunication(&sock);

        
        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
    
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    }
    vTaskDelete(NULL);
}


void server_init()
{
    xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);    
}
