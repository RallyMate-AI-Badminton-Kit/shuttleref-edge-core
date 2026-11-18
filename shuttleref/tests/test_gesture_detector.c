#include "test_harness.h"
#include "gesture_detector.h"

static gd_config_t cfg(void) {
    return (gd_config_t){ .double_tap_window_ms = 350, .long_press_ms = 800,
                          .jitter_quiet_ms = 60 };
}
TEST(double_tap_detected) {
    gd_t g; gd_init(&g, cfg());
    gd_touch(&g, 1000, true);
    gd_touch(&g, 1200, true);
    ASSERT_EQ(gd_poll(&g, 1261), GD_DOUBLE_TAP);      /* 第二击后需等 jitter_quiet(60ms) 静默 */
    ASSERT_EQ(gd_poll(&g, 1300), GD_NONE);            /* 事件只发一次 */
}
TEST(triple_tap_beats_double) {
    gd_t g; gd_init(&g, cfg());
    gd_touch(&g, 100, true); gd_touch(&g, 200, true); gd_touch(&g, 300, true);
    ASSERT_EQ(gd_poll(&g, 300), GD_TRIPLE_TAP);
}
TEST(long_press_detected) {
    gd_t g; gd_init(&g, cfg());
    gd_touch(&g, 1000, true);
    ASSERT_EQ(gd_poll(&g, 1000 + 800), GD_LONG_PRESS);
}
TEST(unstable_touch_ignored_for_scoring) {
    gd_t g; gd_init(&g, cfg());
    gd_touch(&g, 1000, false);                          /* 运动中的触碰 → 丢弃 */
    gd_touch(&g, 1200, false);
    ASSERT_EQ(gd_poll(&g, 1261), GD_NONE);
}
TEST(slow_taps_not_double) {
    gd_t g; gd_init(&g, cfg());
    gd_touch(&g, 1000, true);
    gd_touch(&g, 2000, true);                           /* 超窗 → 两次单击 */
    ASSERT_EQ(gd_poll(&g, 2100), GD_NONE);              /* 单击无语义（V1） */
}

int main(void) {
    RUN(double_tap_detected);
    RUN(triple_tap_beats_double);
    RUN(long_press_detected);
    RUN(unstable_touch_ignored_for_scoring);
    RUN(slow_taps_not_double);
    TEST_MAIN_END();
}
