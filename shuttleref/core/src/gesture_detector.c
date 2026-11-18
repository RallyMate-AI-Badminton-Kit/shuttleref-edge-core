#include "gesture_detector.h"

void gd_init(gd_t *g, gd_config_t cfg) {
    *g = (gd_t){0};
    g->cfg = cfg;
}

void gd_touch(gd_t *g, uint32_t now_ms, bool cap_stable) {
    if (!cap_stable) return;                    /* 运动抑制：不稳定触摸直接丢弃 */
    if (g->taps > 0 && now_ms - g->last_touch_ms > g->cfg.double_tap_window_ms)
        g->taps = 0;                            /* 超窗重置 */
    g->taps++;
    g->last_touch_ms = now_ms;
    g->pressing = true;
    if (g->taps >= 3) { g->pending = GD_TRIPLE_TAP; g->taps = 0; g->pressing = false; }
}

gd_gesture_t gd_poll(gd_t *g, uint32_t now_ms) {
    if (g->pending) { gd_gesture_t r = g->pending; g->pending = GD_NONE; return r; }
    if (g->pressing && g->taps == 1 &&
        now_ms - g->last_touch_ms >= g->cfg.long_press_ms) {
        g->pressing = false; g->taps = 0;
        return GD_LONG_PRESS;
    }
    if (g->taps == 2 && now_ms - g->last_touch_ms > g->cfg.jitter_quiet_ms) {
        g->taps = 0;                            /* 第二击后静默 → 判定双击 */
        return GD_DOUBLE_TAP;
    }
    if (g->taps == 1 && !g->pressing) g->taps = 0;
    return GD_NONE;
}
