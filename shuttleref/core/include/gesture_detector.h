#ifndef GESTURE_DETECTOR_H
#define GESTURE_DETECTOR_H
#include <stdbool.h>
#include <stdint.h>

typedef enum { GD_NONE = 0, GD_DOUBLE_TAP, GD_LONG_PRESS, GD_TRIPLE_TAP } gd_gesture_t;

typedef struct {
    uint32_t double_tap_window_ms;   /* 两击最大间隔 */
    uint32_t long_press_ms;          /* 长按时长 */
    uint32_t jitter_quiet_ms;        /* 触摸后电容稳定静默期 */
} gd_config_t;

typedef struct {
    gd_config_t cfg;
    uint8_t  taps;              /* 窗口内击数 */
    uint32_t last_touch_ms;
    bool     pressing;
    gd_gesture_t pending;       /* poll 已产生、待取走的手势 */
} gd_t;

void gd_init(gd_t *g, gd_config_t cfg);
void gd_touch(gd_t *g, uint32_t now_ms, bool cap_stable);
gd_gesture_t gd_poll(gd_t *g, uint32_t now_ms);
#endif
