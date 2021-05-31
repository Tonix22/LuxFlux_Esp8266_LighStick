#ifndef STRUCTS_H
#define STRUCTS_H



#ifdef __cplusplus
extern "C" 
{
#endif

    #include "sub_structs.h"

    #ifdef __cplusplus
    #include <list>

    typedef struct
    {
        std::list<Block> group; 
        uint32_t time;
    }Frame;
    //min size = 8 bytes
    //max size = 68 bytes
}
#endif


#endif