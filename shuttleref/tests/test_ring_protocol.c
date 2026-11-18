#include "test_harness.h"
#include "ring_protocol.h"

TEST(encode_decode_roundtrip) {
    rp_packet_t in = { .ring_id = 0x1234, .event = RP_EVT_SCORE,
                       .seq = 0x00B7, .battery = 87 };
    uint8_t buf[RP_MAX_PAYLOAD];
    size_t n = rp_encode(&in, buf, sizeof buf);
    ASSERT_EQ(n, 8);
    rp_packet_t out;
    ASSERT(rp_decode(buf, n, &out));
    ASSERT_EQ(out.ring_id, 0x1234);
    ASSERT_EQ(out.event, RP_EVT_SCORE);
    ASSERT_EQ(out.seq, 0x00B7);
    ASSERT_EQ(out.battery, 87);
}
TEST(decode_rejects_bad_crc) {
    rp_packet_t in = { .ring_id = 1, .event = RP_EVT_UNDO, .seq = 2, .battery = 50 };
    uint8_t buf[RP_MAX_PAYLOAD];
    size_t n = rp_encode(&in, buf, sizeof buf);
    buf[n - 1] ^= 0xFF;                      // 破坏 CRC 字节
    rp_packet_t out;
    ASSERT(!rp_decode(buf, n, &out));
}
TEST(decode_rejects_short_buffer) {
    rp_packet_t out;
    ASSERT(!rp_decode((uint8_t[]){1, 2, 3}, 3, &out));
}
TEST(dedup_accepts_once_then_rejects_duplicate) {
    rp_dedup_t d = { .ring_id = 0, .last_seq = 0, .seen = false };
    rp_packet_t p = { .ring_id = 9, .event = RP_EVT_SCORE, .seq = 5, .battery = 90 };
    ASSERT(rp_dedup_accept(&d, &p));         // 首次接受
    ASSERT(!rp_dedup_accept(&d, &p));        // 同 seq 重复拒绝
    p.seq = 6;
    ASSERT(rp_dedup_accept(&d, &p));         // 新 seq 接受
}

int main(void) {
    RUN(encode_decode_roundtrip);
    RUN(decode_rejects_bad_crc);
    RUN(decode_rejects_short_buffer);
    RUN(dedup_accepts_once_then_rejects_duplicate);
    TEST_MAIN_END();
}
