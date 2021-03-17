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

#include "FreeRTOS_wrapper.h"
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
#include "Light_effects.h"


// =============================================================================
// DEFINES
// =============================================================================
#define SEND_NACK message_response(&sock,"NACK\n")
#define SEND_ACK message_response(&sock,"ACK\n")
#define MAX_SIZE 128

// =============================================================================
// External VARIABLES
// =============================================================================

    extern char  File_names[MAX_features][MAX_NAME_SIZE];
    extern xQueueHandle Light_event;
// =============================================================================
// LOCAL VARIABLES
// =============================================================================
    static const char *TAG = "SERVER";  
    char rx_buffer [MAX_SIZE];
    char tempbuff [MAX_SIZE];
    int len = 0;
    uint32_t tcp_to_led_msg = 0;

    xQueueHandle tcp_light_event;

void filter_clr_cls()
{
    char* chr = (strchr(rx_buffer,'\n'));
    if(chr !=NULL){ *chr = 0;}
    chr = (strchr(rx_buffer,'\r'));
    if(chr !=NULL){ *chr = 0;}
}


/**
* @brief send message function
* @param sock socket
* @param msg string message to send
* @return true  <- if the message was send correctly
          false <- if an error ocured while sending the message
**/

bool message_response(int* sock, char* msg){
    int err = send(*sock, msg, (strlen(msg)+1), 0);
    if (err < 0) {
        ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
        return false;
    }
    return true;
}

/**
* @brief Fuction syncronization between both ledsticks
        1.  Send to client READY TO SYNC
        2.  Receive from client ACK
        3.  Receive from client File name
        4.  Open file
        5.  Read file
        6.  Change format of chunk
        7.  Send chunk to client
        8.  Receive ACK from client
        9.  Repeat 5 to 8 until EOF
        10. Send EOF
        11. Close file
        12. Repeat 3 to 11 until all files have been read
* @param sock socket
**/

void sync_func(int*sock){
    Block* chunk    =  malloc(sizeof(Block));
    uint32_t frame_time   = 0;
    char pixels_cnt = 0;
    bool valid_msg  = false;
    int num_pixels = get_pixels();

    message_response(sock,"READY TO SYNC\n");
	printf("To client: READY TO SYNC\r\n"); 
	valid_msg = true;
    memset(rx_buffer,0,MAX_SIZE);// flush buffer
    len = recv(*sock, rx_buffer, sizeof(rx_buffer),0); // WAIT AN ACK
    filter_clr_cls();

    if (strcmp(rx_buffer,"NACK") == 0){
        printf("READY TO SYNC FAILED\r\n");
        return;
    }
    printf("From client: %s \r\n", rx_buffer); 
    
    while (valid_msg){
        int i;
        for (i=0; i < MAX_features;i++){
            memset(rx_buffer,0,MAX_SIZE);// flush buffer
            recv(*sock, rx_buffer, sizeof(rx_buffer),0);//Receive File Name

            printf("From client: %s \r\n", rx_buffer);

            file_open(READ,File_names[i]);
            memset(rx_buffer,0,MAX_SIZE);// flush buffer
            
            // Read data From memory in order to sent data to client, in syncronization 
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
                valid_msg = read_chunk(&frame_time,sizeof(uint32_t),1);
                sprintf(tempbuff,"%d\n",frame_time);
                strcat(rx_buffer,tempbuff);
                printf("To client: %s\r\n",rx_buffer);

                //TODO QueueSend SYNC fade()
                tcp_to_led_msg = TCP_SYNC;
                if(!xQueueSend(Light_event, &tcp_to_led_msg, 0)){                
                     printf(" message failed 2\r\n");    
                 }
                 
                //Chunk send
                message_response(sock,rx_buffer);
                memset(rx_buffer,0,MAX_SIZE);// flush buffer
                recv(*sock, rx_buffer, sizeof(rx_buffer),0);
                printf("From client: %s \r\n", rx_buffer); 
                frame_time = 0;
                pixels_cnt = 0;
                memset(tempbuff,0,MAX_SIZE);// flush buffer
                memset(rx_buffer,0,MAX_SIZE);// flush buffer

                xQueueReceive(tcp_light_event, &tcp_to_led_msg, portMAX_DELAY);//TCP_ACK light event
            }

            message_response(sock,"EOF\n");
            close_file();
        }

    }
    free(chunk);

}

/**
* @brief Function FOTA (update function)
* @param sock socket
**/

void fota_func(int *sock){
    
}

/**
* @brief Syncronization between mobile app and ledstick
        1. Send to client READY TO WRITE
        2. Receive from client ACK
        3. Receive file name
        4. Open file
        5. Receive chunk from client
        6. Parse and write chunk
        7. Receive NEXT/EXIT from client
        8. if NEXT repeat 3 to 7 else close communication
* @param sock socket
**/

void app_func(int *sock){
    
    bool valid_name_file = false;
    bool valid_frame = true;
    message_response(sock, "READY TO WRITE\n");
    memset(rx_buffer,0,MAX_SIZE);// flush buffer
    
    //Receive ACK from client
    len = recv(*sock, rx_buffer, sizeof(rx_buffer),0);
    printf("From client: %s\r\n",rx_buffer);

    filter_clr_cls();

    if (strcmp(rx_buffer,"NACK") == 0){
            printf("READY TO WRITE FAILED\r\n");
            return;
    }
    memset(rx_buffer,0,MAX_SIZE);// flush buffer
    //Receive file name
    len = recv(*sock, rx_buffer, sizeof(rx_buffer),0);
    printf("From client: %s\r\n",rx_buffer);
    filter_clr_cls();

    if(check_file_exist(rx_buffer)){
        delete_file(rx_buffer);
    }else{ // check if its a valid name
        
        for(uint8_t i=0;i<MAX_features;i++)
        {
            if(strcmp(rx_buffer,File_names[i]) == 0)
            {
                valid_name_file = true;
            }
            if(valid_name_file == false)
            {
                printf("invalid name file\r\n");
                return;
            }
        }

    }
    file_open(WRITE,rx_buffer);
    
    //Receive chunk 1
    memset(rx_buffer,0,MAX_SIZE);// flush buffer
    recv(*sock, rx_buffer, sizeof(rx_buffer),0);
    filter_clr_cls();

    do
    {
        valid_frame  = parse_chunk(rx_buffer);
        valid_frame &= strcmp(rx_buffer,"EOF") ;
        recv(*sock, rx_buffer, sizeof(rx_buffer),0);
        filter_clr_cls();
    }while(valid_frame);


    close_file();
}



/**
* @brief Communication between server and client
* @param sock socket
**/
void comunication(int* sock){
    

    memset(rx_buffer,0,MAX_SIZE);// flush buffer
    //receives message from client
    len = recv(*sock, rx_buffer, sizeof(rx_buffer),0);
    filter_clr_cls();
    printf("From client: %s \r\n", rx_buffer);
    if(strcmp(rx_buffer,"SYNC") == 0){
		sync_func(sock);

	}else if(strcmp(rx_buffer,"FOTA") == 0){
		fota_func(sock);

	}else if(strcmp(rx_buffer,"APP") == 0){
        app_func(sock);        
    
    }else{
		message_response(sock,"NACK"); 
	}

    vTaskDelay(500 / portTICK_PERIOD_MS);

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
    struct sockaddr_in sourceAddr;
    uint8_t addrLen = sizeof(sourceAddr);
    tcp_light_event = xQueueCreate(10, sizeof(uint32_t));
    
    //TODO QueueSend flash()
    tcp_to_led_msg = TCP_LOAD;
    if(!xQueueSend(Light_event, &tcp_to_led_msg, 0)){                
        printf(" message failed 2\r\n");    
    }


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

        // =============================================================================
        // 3. Socket Listen
        // =============================================================================
        err = listen(listen_sock, 1);
        if (err != 0) {
            ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket listening");

        


        // =============================================================================
        // 4. Socket Accept
        // =============================================================================
        int sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket accepted");



        //TODO QueueSend OFF
        tcp_to_led_msg = OFF;
        if(!xQueueSend(Light_event, &tcp_to_led_msg, 0)){                
            printf(" message failed 2\r\n");    
        }
        
        
        comunication(&sock);
        //This line of code will be reached once the communication is over

        //TODO QueueSend OFF
        tcp_to_led_msg = OFF;
        if(!xQueueSend(Light_event, &tcp_to_led_msg, 0)){                
            printf(" message failed 2\r\n");    
        }
        
        
        ESP_LOGI(TAG, "Shutting down socket and restarting...");
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
