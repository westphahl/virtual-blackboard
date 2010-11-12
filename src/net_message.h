#ifndef NET_MESSAGE_H
#define NET_MESSAGE_H

#include <stdint.h>
#include <arpa/inet.h>

enum message_types {
    m_login,
    m_status,
    m_board,
    m_clear,
    m_shutdown,
    m_request,
    m_query,
    m_reply,
    m_error = 255
};

enum error_codes {e_message=0, e_fatal=1, e_login=10};

/* Avoid padding problems */
#pragma pack(1)
struct net_header {
    uint8_t type;
    uint16_t length;
};

struct net_login {
    struct net_header header;
    uint8_t role;
    char name[];
};

struct net_status {
    struct net_header header;
    uint8_t role;
    uint16_t cid;
    uint8_t write;
    uint8_t dcount;
    uint8_t tcount;
    uint16_t scount;
};

struct net_board {
    struct net_header header;
    char content[];
};

/* Ask server for write permissions */
struct net_request {
    struct net_header header;
    uint8_t write;
};

/* Query docent for write permissions */
struct net_query {
    struct net_header header;
    uint16_t cid;
    char name[];
};

/* Reply of docent */
struct net_reply {
    struct net_header header;
    uint8_t write;
    uint16_t cid;
};

struct net_error {
    struct net_header header;
    uint8_t ecode;
    char detail[];
};
#pragma pack(0)

void hton_header(void *header, int type, int size);
void ntoh_header(void *header);

#endif
