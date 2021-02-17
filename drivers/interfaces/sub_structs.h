#ifndef SUB_STRUCTS_H
#define SUB_STRUCTS_H
// =============================================================================
// TYPE DEFINITION
// =============================================================================
typedef enum
{
    IDLE_feature,
    RITH_feature,
    CIRCULAR_feature,
    LINEAR_feature,
    MAX_features,
}feature_t;

    /*
    Next structures explained
    +-----+-------+------+
    | Red | Green | Blue | --> RGB
    +-----+-------+------+

    +--------+------+
    | Pixels | RGB  | --> BLOCK
    +--------+------+

    +-------+-------+----+-------------+
    | time  | group | .. | list<Block> | --> Frame
    +-------+-------+----+-------------+

    +--------+---------+-----+--------------+
    | Frame1 | Frame2  | ... | list<Frames> | --> Sequence
    +--------+---------+-----+--------------+

    */

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



#endif