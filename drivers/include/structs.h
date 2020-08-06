#ifndef STRUCTS_H
#define STRUCTS_H

#define MAX_RGB 10

typedef enum
{
    IDLE_feature,
    RITH_feature,
    CIRCULAR_feature,
    LINEAR_feature,
    Features,
}feature_t;


typedef struct 
{
    uint8_t RED;
    uint8_t GREEN;
    uint8_t BLUE;
}RGB;

struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint32_t time_ms;
}typedef RGBT;

#endif