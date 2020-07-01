#ifndef CONFIG_H
#define CONFIG_H

#define RIGHT_STICK (1)
#define LEFT_STICK  (0)

#define DEVELOMENT  (1) //GPIO adapat to DEV board
#define RELEASE     (0) // change on GPIO definition


#define AP_PASS      "DiaboloSticks"
#define MAX_STA_CONN       4

//STATION CONFIG
#define ESP_WIFI_SSID      "IZZI-99CD"
#define ESP_WIFI_PASS      "704FB81799CD"
#define ESP_MAXIMUM_RETRY  10

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