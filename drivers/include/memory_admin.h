#ifndef MEMORY_ADMIN_H
#define MEMORY_ADMIN_H


#define DEBUG_RGB() printf("[%d, ",(LedStick->feature_collection[feature]->back()).RED);\
                    printf(",%d, ",(LedStick->feature_collection[feature]->back()).GREEN);\
                    printf(",%d], ",(LedStick->feature_collection[feature]->back()).BLUE);

#define BAD_FORMAT BIT0
#define EMPTYFILE  BIT1

#ifdef __cplusplus
extern "C" 
{
#endif
    #include "structs.h"
    void file_exist(feature_t feature);
    void file_read(feature_t feature);
    void init_flash_status_group(void);
    #ifdef __cplusplus
    #include "file_system.h"
    
}
    #endif


#endif
