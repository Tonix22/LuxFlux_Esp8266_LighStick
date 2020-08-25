#ifndef WIFI_H
#define WIFI_H
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


void wifi_init_sta(void);
void wifi_init_softap(void);
void wifi_deint(void);
void WIFI_ON(void);
void WIFI_OFF(void);
#endif