#include "net_message.h"

void hton_header(void *header, int type, int size) {
    struct net_header *nh = (struct net_header *) header;
    
    // Convert to network byte order
    nh->type = (uint16_t) type;
    nh->length = htons((uint16_t) size);
}

void ntoh_header(void *header) {
    struct net_header *nh = (struct net_header *) header;

    // Convert to host byte order
    nh->length = ntohs(nh->length);
}
