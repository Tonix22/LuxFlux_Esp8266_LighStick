#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG_MENU  (0)

#define RIGHT_STICK (0)
#define LEFT_STICK  (1)
#define WIFI_HOME   (1)

#define DEVELOMENT  (1) //GPIO adapat to DEV board
#define RELEASE     (0) // change on GPIO definition

#define IMU_TEST    (0)

#define PIXEL_TEST  (0)

#define WIFI_TEST   (0)

#define AP_PASS      "DiaboloSticks"
#define MAX_STA_CONN       4

//STATION CONFIG
#if WIFI_HOME
#define ESP_WIFI_SSID      "IZZI-99CD"
#define ESP_WIFI_PASS      "704FB81799CD"
#endif

#define ESP_MAXIMUM_RETRY  10
#define AUTOCONNECT (1)


//This defines changes depending on upper defines
#if !RIGHT_STICK && !LEFT_STICK
    #error "MUST SELECT A LEDSTICK"
#endif

#define LEFT_STICK_IP "192.168.4.1"
#define RIGHT_STICK_IP "192.168.5.1"
#define PORT 3333

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
    #define HOST_IP_ADDR RIGHT_STICK_IP
    
#endif


#endif