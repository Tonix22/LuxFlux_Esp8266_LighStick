#ifndef CONFIG_H
#define CONFIG_H
// =============================================================================
// Defines
// =============================================================================

/**
 * @brief this config file will se compilation parameters
 * 
 */

// =============================================================================
// SDK VERSION
// =============================================================================
#define VALID_SYSTEM_VERSION "v3.2-565-g9f37d690-dirty"
#define LWIP_VALID_VERSION   "2.1.3d"

// =============================================================================
// GPIO RULES
// =============================================================================
#define DEVELOMENT  (1) //GPIO adapat to DEV board
#define RELEASE     (0) // change on GPIO definition

// =============================================================================
// TESTING DEFINES
// =============================================================================

#define DEBUG_MENU  (0)
#define IMU_TEST    (0)
#define PIXEL_TEST  (0)
#define WIFI_TEST   (1)

// =============================================================================
// WIFI
// =============================================================================

#define RIGHT_STICK (0)
#define LEFT_STICK  (1)
#define WIFI_HOME   (1)

#define AP_PASS      "DiaboloSticks"
#define MAX_STA_CONN       4
// =============================================================================
// STATION
// =============================================================================
#if WIFI_HOME
#define ESP_WIFI_SSID      "DEPTO_201"
#define ESP_WIFI_PASS      "sxk!0363" 
#endif

#define ESP_MAXIMUM_RETRY  10
#define AUTOCONNECT (1)

// =============================================================================
// ACCES POINT
// =============================================================================

#define LEFT_STICK_IP "192.168.4.1"
#define RIGHT_STICK_IP "192.168.5.1"
#define LAP_IP "192.168.201.174"
#define PORT 3333

//This defines changes depending on upper defines
#if !RIGHT_STICK && !LEFT_STICK
    #error "MUST SELECT A LEDSTICK"
#endif

#if RIGHT_STICK
    #define AP_SSID      "LuxFLUX_RIGHT"
    #define CHANGE_DCHP  // avoid IP colision
    #define ESP_WIFI_SSID      "LuxFLUX_LEFT"
    #define ESP_WIFI_PASS      "DiaboloSticks"
    //TCP
    #define HOST_IP_ADDR LEFT_STICK_IP
#endif

#if LEFT_STICK

    #define AP_SSID      "LuxFLUX_LEFT"
    #if !WIFI_HOME
    
    #define ESP_WIFI_SSID      "LuxFLUX_RIGHT"
    #define ESP_WIFI_PASS      "DiaboloSticks"
    #endif
        //TCP
    #if WIFI_TEST

        #define HOST_IP_ADDR LAP_IP

    #else
    
         #define HOST_IP_ADDR RIGHT_STICK_IP

    #endif
    
#endif


#endif