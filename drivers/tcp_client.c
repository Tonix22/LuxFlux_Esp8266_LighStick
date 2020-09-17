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


 extern char  File_names[MAX_features][MAX_NAME_SIZE];
// =============================================================================
// Local variables
// =============================================================================

static const char *TAG = "example";
char *payload = "Message from ESP32 ";
int sock;

int send_msg(const char * msg){
    int err = send(sock, msg, strlen(msg), 0);
    if (err < 0) {
        ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
    }
    return err;
}

int recv_msg(char* msg, int lenght){
    int len = recv(sock, msg, lenght-1, 0);
        // Error occured during receiving
    if (len < 0) {
        ESP_LOGE(TAG, "recv failed: errno %d", errno);
    }
    return len;
}


/**
 * @brief 
 * 1. Prepare socket
 * 2. Socket Connect
 * 3. Send First Message
 * 4. Wait Response
 * 5. Request first chunk
 * 6. Loop comunication
 * 
 * @param pvParameters 
 */
static void tcp_client_task(void *pvParameters)
{
    // =============================================================================
    // 1. Prepare socket
    // =============================================================================
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        // =============================================================================
        // 2. Socket Connect
        // =============================================================================
        int err = connect(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        }
        ESP_LOGI(TAG, "Successfully connected");

        // =============================================================================
        // 2. Socket Connect
        // =============================================================================
        /************** SEND FIRST MESSAGE ******************/
        
        while(send_msg("SYNC\n")<0){
                vTaskDelay(1000/ portTICK_RATE_MS);    
        }
        // =============================================================================
        // 3. Send First Message
        // =============================================================================
        
        // =============================================================================
        // 4. Wait Response
        // =============================================================================
        int len = recv_msg(rx_buffer,sizeof(rx_buffer));
        memset(rx_buffer+(len-2),0,3);
        int tries = 0;
        while(strcmp (rx_buffer, "READY TO SYNC") != 0 && tries < MAX_TRIES){
            send_msg("NACK\n");
            len = recv_msg(rx_buffer,sizeof(rx_buffer));
            if (len > 0) {
                memset(rx_buffer+(len-2),0,3);
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);
            }
            tries++;
            //printf("Try (%i)\r\n",tries);
        }

        // =============================================================================
        // 5. Request first chunk of File
        // =============================================================================
        
        //RECORRE TODOS LOS ARCHIVOS
        int i=0;
        for (; i < MAX_features;i++){
            send_msg(File_names[i]);
                    
            // Open file 
            file_open(WRITE,File_names[i]);
        // =============================================================================
        // 6. Loop comunication
        // =============================================================================
            while (1) {
                memset(rx_buffer,0,sizeof(rx_buffer)); //flush a rx_buffer
                //RECEIVE CHUNK :)
                len = recv_msg(rx_buffer,sizeof(rx_buffer));
                if ( len > 0) {
                    memset(rx_buffer+(len-2),0,3);
                    ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                    ESP_LOGI(TAG, "%s", rx_buffer);

                //CHECK IF IS EOF (go to? if EOF)
                if(strcmp(rx_buffer,"EOF") == 0){

                    //CLOSE FILE
                    close_file();
                
                    break;
                }
                
                //Open File
                if(check_file_exist(File_names[i])){
                    delete_file(File_names[i]);
                }
                file_open(WRITE,File_names[i]);

                //PARSE THE CHUNK
                parse_chunk(rx_buffer);
                
                close_file();

                }
            }
        

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void client_init()
{
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}
