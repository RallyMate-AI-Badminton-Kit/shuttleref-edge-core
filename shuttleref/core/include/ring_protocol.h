#ifndef RING_PROTOCOL_H
#define RING_PROTOCOL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RP_PACKET_LEN 8
#define RP_MAX_PAYLOAD 16

typedef enum {
    RP_EVT_SCORE = 0x01, RP_EVT_UNDO = 0x02, RP_EVT_REPLAY = 0x03,
    RP_EVT_CHECKIN_RED = 0x04, RP_EVT_CHECKIN_BLUE = 0x05, RP_EVT_ALIVE = 0x06
} rp_event_t;

typedef struct {
    uint16_t ring_id;
    uint8_t  event;
    uint16_t seq;
    uint8_t  battery;
} rp_packet_t;

size_t   rp_encode(const rp_packet_t *pkt, uint8_t *out, size_t cap);
bool     rp_decode(const uint8_t *buf, size_t len, rp_packet_t *out);
uint8_t  rp_crc8(const uint8_t *buf, size_t len);

typedef struct { uint16_t ring_id; uint16_t last_seq; bool seen; } rp_dedup_t;
bool rp_dedup_accept(rp_dedup_t *d, const rp_packet_t *pkt);
#endif
