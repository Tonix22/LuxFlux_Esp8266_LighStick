#ifndef MEMORY_ADMIN_H
#define MEMORY_ADMIN_H


#define DEBUG_RGB() printf("[%d, ",(LedStick->feature_collection[feature]->back()).RED);\
                    printf(",%d, ",(LedStick->feature_collection[feature]->back()).GREEN);\
                    printf(",%d], ",(LedStick->feature_collection[feature]->back()).BLUE);

#define READ_OK    BIT0
#define BAD_FORMAT BIT1
#define ABORT      BIT2
#define EMPTYFILE  BIT3

#ifdef __cplusplus
extern "C" 
{
#endif
    #include "structs.h"
    void file_exist(feature_t feature);
    void file_read(feature_t feature);
    void init_status_Group(void);
    #ifdef __cplusplus
    #include "file_system.h"
    
}
    #endif


#endif
