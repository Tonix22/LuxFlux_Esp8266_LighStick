#ifndef STRUCTS_H
#define STRUCTS_H

#define MAX_RGB 10

typedef enum
{
    IDLE_feature,
    RITH_feature,
    CIRCULAR_feature,
    LINEAR_feature,
    MAX_features,
}feature_t;

typedef struct 
{
    uint8_t RED;
    uint8_t GREEN;
    uint8_t BLUE;
}RGB;


typedef struct
{
    char pixels;
    RGB  color;
}Block; // 4bytes

#ifdef __cplusplus
extern "C" 
{
#endif

    #ifdef __cplusplus
    #include <list>

    typedef struct
    {
        std::list<Block> group; 
        int32_t time;
    }Frame;
    //min size = 8 bytes
    //max size = 68 bytes
}
#endif


#endif