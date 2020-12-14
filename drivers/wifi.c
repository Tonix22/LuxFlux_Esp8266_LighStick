/* 
   This code is in the Public Domain (or CC0 licensed, at your option.
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// =============================================================================
// includes
// =============================================================================
#include <string.h>
#include "config.h"
#include "wifi.h"
#include "FreeRTOS_wrapper.h"
#include "esp_event_loop.h"

// =============================================================================
// Local variables
// =============================================================================

static const char *TAG = "wifi Driver";

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
static EventGroupHandle_t s_wifi_event_group;

//static int s_retry_num = 0; // Max retryes number counter

// =============================================================================
// Functions Prototypes
// =============================================================================
void DHCP_setup(void);

// =============================================================================
// 
// ██╗███████╗██████╗     ██╗  ██╗ █████╗ ███╗   ██╗██████╗ ██╗     ███████╗██████╗ ███████╗
// ██║██╔════╝██╔══██╗    ██║  ██║██╔══██╗████╗  ██║██╔══██╗██║     ██╔════╝██╔══██╗██╔════╝
// ██║███████╗██████╔╝    ███████║███████║██╔██╗ ██║██║  ██║██║     █████╗  ██████╔╝███████╗
// ██║╚════██║██╔══██╗    ██╔══██║██╔══██║██║╚██╗██║██║  ██║██║     ██╔══╝  ██╔══██╗╚════██║
// ██║███████║██║  ██║    ██║  ██║██║  ██║██║ ╚████║██████╔╝███████╗███████╗██║  ██║███████║
// ╚═╝╚══════╝╚═╝  ╚═╝    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝
// =============================================================================

/**
 * @brief Handler when ESP action happens
 * 
 * @param ctx 
 * @param event_base 
 */


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;
    
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);
        if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
            /*Switch to 802.11 bgn mode */
            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
        }
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
        ESP_LOGI(TAG, "assigned ip:%s",
                 ip4addr_ntoa(&event->event_info.ap_staipassigned.ip));
        break;
    default:
        break;
    }
    return ESP_OK;
}




// =============================================================================
// 
// ██╗    ██╗██╗███████╗██╗    ███████╗███████╗████████╗██╗   ██╗██████╗
// ██║    ██║██║██╔════╝██║    ██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
// ██║ █╗ ██║██║█████╗  ██║    ███████╗█████╗     ██║   ██║   ██║██████╔╝
// ██║███╗██║██║██╔══╝  ██║    ╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝
// ╚███╔███╔╝██║██║     ██║    ███████║███████╗   ██║   ╚██████╔╝██║
//  ╚══╝╚══╝ ╚═╝╚═╝     ╚═╝    ╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝
// =============================================================================

/**
 * @brief Generic TCP adapter init
 * this step is indepent of STA or AP
 */
void wifi_set(void)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

/**
 * @brief Init station driver
 * 
 */
void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    wifi_set();
    /*
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &station_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &station_event_handler, NULL));
    */
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS
        },
    };

    /* Setting a password implies station will connect to all security modes including WEP/WPA.
        * However these modes are deprecated and not advisable to be used. Incase your Access point
        * doesn't support WPA2, these mode can be enabled by commenting below line */

    if (strlen((char *)wifi_config.sta.password)) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    //#if AUTOCONNECT
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by station_event_handler() (see above) */
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

    vEventGroupDelete(s_wifi_event_group);
}

/**
 * @brief Init soft AP
 */

void wifi_init_softap(void)
{
    wifi_set();

    //ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &ap_event_handler, NULL));
    
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .password = AP_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(AP_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
             AP_SSID, AP_PASS);

    #ifdef CHANGE_DCHP // TODO change in production
    DHCP_setup();
    #endif

}
/**
 * @brief Clean all AP status, so AP is off 
 */
void wifi_deint_ap(void)
{
    esp_wifi_stop();
    //esp_event_loop_delete_default();
    esp_wifi_deinit();
    //esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &ap_event_handler);    
}
/**
 * @brief Change DHCP IPS
 */
void DHCP_setup(void)
{
    
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
   
}