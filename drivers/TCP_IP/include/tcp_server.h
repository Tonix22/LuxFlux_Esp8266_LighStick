#ifndef TCP_SERVER_H
#define TCP_SERVER_H
// =============================================================================
// C Function prototypes
// =============================================================================
typedef enum
{
   TCP_ACK,
   TCP_NACK
}TCP_msgID;

void server_init();

#endif