#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG_MENU  (0)

#define RIGHT_STICK (0)
#define LEFT_STICK  (1)

#define DEVELOMENT  (1) //GPIO adapat to DEV board
#define RELEASE     (0) // change on GPIO definition

#define IMU_TEST    (0)

#define PIXEL_TEST  (0)

#define AP_PASS      "DiaboloSticks"
#define MAX_STA_CONN       4

//STATION CONFIG
#define ESP_WIFI_SSID      "Tonix"
#define ESP_WIFI_PASS      "typewriter"
#define ESP_MAXIMUM_RETRY  10
#define AUTOCONNECT (0)

//This defines changes depending on upper defines
#if !RIGHT_STICK && !LEFT_STICK
    #error "MUST SELECT A LEDSTICK"
#endif

#if RIGHT_STICK
    #define AP_SSID      "LuxFLUX_RIGHT"
    #define CHANGE_DCHP  // avoid IP colision
#endif

#if LEFT_STICK
    #define AP_SSID      "LuxFLUX_LEFT"
#endif


#endif