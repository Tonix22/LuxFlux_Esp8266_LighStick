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
#include "tcp_client.h"
#include "file_system.h"
#include "memory_admin.h"

// =============================================================================
// DEFINES
// =============================================================================
#define SEND_NACK message_response(&sock,"NACK\n")
#define SEND_ACK message_response(&sock,"ACK\n")
#define MAX 128
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
    char rx_buffer [MAX];
    Block* chunk    =  malloc(sizeof(Block));
    uint32_t time   = 0;
    char pixels_cnt = 0;
    bool valid_msg  = false;
    char tempbuff [MAX];
    int num_pixels = get_pixels();

    memset(rx_buffer,0,MAX);// flush buffer
    recv(*sock, rx_buffer, sizeof(rx_buffer));
    printf("From client: %s \r\n", rx_buffer); 

    if(strcmp(rx_buffer,"SYNC\n") == 0){
		send(*sock, "READY TO SYNC", 16, 0);
		printf("READY TO SYNC\r\n"); 
		valid_msg = true;
	}else if(strcmp(rx_buffer,"FOTA\n") == 0){
		send(*sock, "READY TO FOTA", 16, 0);
		valid_msg = true;
	}else{
		send(*sock, "NACK", 5, 0); 
	}

    int i;
    for (i=0; i < MAX_features;i++){
        memset(rx_buffer,0,MAX);// flush buffer
        recv(*sock, rx_buffer, sizeof(rx_buffer));//Receive File Name
        printf("From client: %s \r\n", rx_buffer); 

        file_open(READ,File_names[i]);

       
        while(valid_msg)
        {
            
             while (pixels_cnt < num_pixels  && valid_msg)
            {
                valid_msg = read_chunk(chunk,sizeof(Block),1);
                sprintf(tempbuff,"%d(%d,%d,%d),",chunk->pixels, chunk->color.RED, chunk->color.GREEN, chunk->color.BLUE);
                strcat(rx_buffer,tempbuff);
                pixels_cnt+=chunk->pixels;
             }
        
            valid_msg = read_chunk(&time,sizeof(uint32_t),1);
            sprintf(tempbuff,"%d",time);
            strcat(rx_buffer,tempbuff);
            printf("To client: %s",rx_buffer);
            message_response(sock,rx_buffer);
            time = 0;
            pixels_cnt = 0;
            memset(tempbuff,0,MAX);// flush buffer
            
        }
        close_file();
    }



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
    char rx_buffer[128];
    char addr_str [128];
    int  addr_family;
    int  ip_protocol;
    bool first_msg = true;

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

       /* while (1) {

            // =============================================================================
            // 3. Wait First Message
            // =============================================================================


            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occured during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            }
            // Connection closed
            else if (len == 0) {
                ESP_LOGI(TAG, "Connection closed");
                break;
            }
            // Data received
            else {

                inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                // =============================================================================
                // 4. Send Response
                // =============================================================================
                //rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                char* ptr = strchr(rx_buffer,'\n');
                if(ptr !=NULL){
                    *ptr ='\0';
                }
                ptr = strchr(rx_buffer,'\r'); 
                if(ptr !=NULL){
                    *ptr ='\0';
                }
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);

                if(first_msg){
                    first_msg= false;
                    if(strcmp (rx_buffer, "SYNC") == 0){

                        message_response(&sock,"READY TO SYNC\n");

                    }else if (strcmp (rx_buffer, "FOTA") == 0){

                        message_response(&sock,"READY TO FOTA\n");

                    }else{
                        SEND_NACK;
                    }
                }else{
                    if (!SEND_ACK) {
                        break;
                    }
                }
            }
        }*/

        SEND_NACK;

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}


void server_init()
{
    xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);    
}
