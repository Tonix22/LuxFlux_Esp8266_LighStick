#ifndef WIFI_H
#define WIFI_H

// =============================================================================
// INCLUDES
// =============================================================================

#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


// =============================================================================
// EVENT GROUPS BITS DEFINES
// =============================================================================

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// =============================================================================
// Functions Prototypes
// =============================================================================

void wifi_init_sta(void);
void wifi_init_softap(void);
void wifi_deint_ap(void);
#endif