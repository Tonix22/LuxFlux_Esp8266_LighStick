/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "config.h"
#include "wifi.h"
#include "FreeRTOS_wrapper.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"



static const char *TAG = "wifi Driver";

//ACESS POINT

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;



static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY) 
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } 
        else 
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}



void wifi_general_cfg(void)
{
    //Create flag group
    s_wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // REGISTER STATUS FLAGS
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    
    wifi_init_softap();
    wifi_init_sta();

    #if CHANGE_DCHP // change in production
        tcpip_adapter_ip_info_t val;
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP,&val);
        ESP_LOGI(TAG, "old dhcp:%s", ip4addr_ntoa(&val.ip));
        IP4_ADDR(&val.ip,192, 168 , 5, 1);
        IP4_ADDR(&val.gw,192, 168 , 5, 1);
        tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);

        tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP,&val);

        tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);

        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP,&val);

        ESP_LOGI(TAG, "new dhcp:%s", ip4addr_ntoa(&val.ip));
    #endif
}


void wifi_init_sta(void)
{
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    esp_wifi_stop();
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
}

void wifi_init_softap(void)
{
    wifi_config_t wifi_config;

    wifi_config.ap.ssid_len = strlen(AP_SSID)+1;
    memset(&wifi_config.ap.ssid,0,32);
    strcpy((char*)&wifi_config.ap.ssid,AP_SSID);
    memset(&wifi_config.ap.password,0,64);
    strcpy((char*)&wifi_config.ap.password,AP_PASS);
    wifi_config.ap.authmode = WIFI_AUTH_WPA_PSK;
    wifi_config.ap.max_connection = MAX_STA_CONN;
    wifi_config.ap.channel = 6;


    if (strlen(AP_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        printf("OPEN\r\n");
    }

    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    esp_wifi_stop();
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_softap finished. AP_SSID:%s password:%s",
             AP_SSID, AP_PASS);
}

void WIFI_ON(void)
{
    if(esp_wifi_start() == 0)
    {
        ESP_LOGI(TAG,"WIFI ON");
    }
}
void WIFI_OFF(void)
{
    if(esp_wifi_stop() == 0)
    {
        ESP_LOGI(TAG,"WIFI OFF");
    }
}