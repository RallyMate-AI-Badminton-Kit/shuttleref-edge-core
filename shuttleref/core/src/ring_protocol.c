#include "ring_protocol.h"

uint8_t rp_crc8(const uint8_t *buf, size_t len) {   /* CRC-8/MAXIM 多项式 0x8C */
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (int b = 0; b < 8; b++)
            crc = (uint8_t)((crc >> 1) ^ (crc & 1 ? 0x8C : 0));
    }
    return crc;
}

size_t rp_encode(const rp_packet_t *pkt, uint8_t *out, size_t cap) {
    if (cap < RP_PACKET_LEN) return 0;
    out[0] = (uint8_t)(pkt->ring_id >> 8);
    out[1] = (uint8_t)(pkt->ring_id);
    out[2] = pkt->event;
    out[3] = (uint8_t)(pkt->seq >> 8);
    out[4] = (uint8_t)(pkt->seq);
    out[5] = pkt->battery;
    out[6] = 0xA5;                                   /* 固定填充字节 */
    out[7] = rp_crc8(out, RP_PACKET_LEN - 1);
    return RP_PACKET_LEN;
}

bool rp_decode(const uint8_t *buf, size_t len, rp_packet_t *out) {
    if (len != RP_PACKET_LEN || buf[6] != 0xA5) return false;
    if (rp_crc8(buf, RP_PACKET_LEN - 1) != buf[RP_PACKET_LEN - 1]) return false;
    out->ring_id = (uint16_t)((buf[0] << 8) | buf[1]);
    out->event   = buf[2];
    out->seq     = (uint16_t)((buf[3] << 8) | buf[4]);
    out->battery = buf[5];
    return true;
}

bool rp_dedup_accept(rp_dedup_t *d, const rp_packet_t *pkt) {
    if (d->seen && d->ring_id == pkt->ring_id && d->last_seq == pkt->seq)
        return false;                                 /* 同 (ring,seq) 重复 */
    d->ring_id = pkt->ring_id; d->last_seq = pkt->seq; d->seen = true;
    return true;
}
