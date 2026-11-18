#include "test_harness.h"
#include "score_engine.h"
#include <string.h>

TEST(single_point_scores) {
    se_state_t st; se_init(&st);
    ASSERT_EQ(se_score_side(&st, SE_SIDE_RED), SE_OK);
    ASSERT_EQ(st.score_red, 1); ASSERT_EQ(st.score_blue, 0);
}
TEST(win_game_at_21_with_two_lead) {
    se_state_t st; se_init(&st);
    for (int i = 0; i < 20; i++) se_score_side(&st, SE_SIDE_RED);   // 20:0
    for (int i = 0; i < 20; i++) se_score_side(&st, SE_SIDE_BLUE);  // 20:20
    se_score_side(&st, SE_SIDE_RED);                                 // 21:20 未结束
    ASSERT_EQ(st.games_red, 0);
    se_score_side(&st, SE_SIDE_RED);                                 // 22:20 胜局
    ASSERT_EQ(st.games_red, 1);
    ASSERT_EQ(st.score_red, 0); ASSERT_EQ(st.score_blue, 0);        // 新局清零
    ASSERT_EQ(st.current_game, 2);
    ASSERT(!st.match_over);
}
TEST(cap_at_30_after_29_all) {
    se_state_t st; se_init(&st);
    for (int i = 0; i < 29; i++) { se_score_side(&st, SE_SIDE_RED); se_score_side(&st, SE_SIDE_BLUE); }
    ASSERT_EQ(st.score_red, 29); ASSERT_EQ(st.score_blue, 29);
    se_score_side(&st, SE_SIDE_BLUE);                                // 30 封顶
    ASSERT_EQ(st.games_blue, 1);
}
TEST(match_two_games_wins) {
    se_state_t st; se_init(&st);
    for (int g = 0; g < 2; g++) {
        for (int i = 0; i < 21; i++) se_score_side(&st, SE_SIDE_RED);
        ASSERT_EQ(st.games_red, g + 1);
    }
    ASSERT(st.match_over);
    ASSERT_EQ(se_winner(&st), SE_SIDE_RED);
    ASSERT_EQ(se_score_side(&st, SE_SIDE_BLUE), SE_ERR_MATCH_OVER); // 赛后拒记
}

TEST(announce_court_by_score_parity) {
    se_state_t st; se_init(&st);
    se_score_side(&st, SE_SIDE_RED);                          /* 1:0 红发 */
    char buf[96];
    se_announce(&st, buf, sizeof buf);
    ASSERT(strstr(buf, "1比0"));
    ASSERT(strstr(buf, "红方发球"));
    ASSERT(strstr(buf, "左区"));                              /* 红方 1 分→奇数→左区 */
    se_score_side(&st, SE_SIDE_RED);                          /* 2:0 */
    se_announce(&st, buf, sizeof buf);
    ASSERT(strstr(buf, "右区"));                              /* 2 分→偶→右区 */
}
TEST(announce_flags) {
    se_state_t st; se_init(&st);
    for (int i = 0; i < 20; i++) se_score_side(&st, SE_SIDE_RED);
    char buf[96];
    se_announce(&st, buf, sizeof buf);
    ASSERT(strstr(buf, "局点"));                              /* 20:0 → 红局点 */
    se_score_side(&st, SE_SIDE_RED);                          /* 21:0 胜第一局 */
    for (int i = 0; i < 20; i++) se_score_side(&st, SE_SIDE_RED);
    se_announce(&st, buf, sizeof buf);
    ASSERT(strstr(buf, "场点"));                              /* 1 局在手 20:0 → 场点 */
    se_state_t d; se_init(&d);
    for (int i = 0; i < 20; i++) { se_score_side(&d, SE_SIDE_RED); se_score_side(&d, SE_SIDE_BLUE); }
    se_announce(&d, buf, sizeof buf);
    ASSERT(strstr(buf, "平分"));                              /* 20:20 */
}

TEST(undo_rolls_back_last_point) {
    se_state_t st; se_init(&st);
    se_score_side(&st, SE_SIDE_RED);
    se_score_side(&st, SE_SIDE_RED);                  /* 2:0 */
    ASSERT_EQ(se_undo(&st), SE_OK);
    ASSERT_EQ(st.score_red, 1); ASSERT_EQ(st.score_blue, 0);
    ASSERT_EQ(st.server, SE_SIDE_RED);                /* 重放恢复 1:0 时红发 */
}
TEST(undo_across_game_boundary) {
    se_state_t st; se_init(&st);
    for (int i = 0; i < 21; i++) se_score_side(&st, SE_SIDE_RED);   /* 胜第一局 */
    ASSERT_EQ(st.games_red, 1);
    ASSERT_EQ(se_undo(&st), SE_OK);                   /* 撤销局点 */
    ASSERT_EQ(st.games_red, 0);
    ASSERT_EQ(st.score_red, 20); ASSERT_EQ(st.score_blue, 0);
}
TEST(undo_empty_returns_error) {
    se_state_t st; se_init(&st);
    ASSERT_EQ(se_undo(&st), SE_ERR_NOT_UNDOABLE);
}
TEST(undo_after_match_over_restores_play) {
    se_state_t st; se_init(&st);
    for (int g = 0; g < 2; g++) for (int i = 0; i < 21; i++) se_score_side(&st, SE_SIDE_RED);
    ASSERT(st.match_over);
    ASSERT_EQ(se_undo(&st), SE_OK);                   /* 撤销赛点 */
    ASSERT(!st.match_over);
    ASSERT_EQ(st.games_red, 1); ASSERT_EQ(st.score_red, 20);
}

TEST(checkin_activates_and_scores) {
    se_state_t st; se_init(&st);
    ASSERT_EQ(se_checkin(&st, 0x1111, SE_SIDE_RED), SE_OK);
    ASSERT_EQ(se_checkin(&st, 0x2222, SE_SIDE_RED), SE_OK);
    ASSERT_EQ(se_checkin(&st, 0x3333, SE_SIDE_BLUE), SE_OK);
    ASSERT_EQ(se_checkin(&st, 0x4444, SE_SIDE_BLUE), SE_OK);
    ASSERT_EQ(se_score(&st, 0x1111), SE_OK);          /* 红队戒指得分 */
    ASSERT_EQ(st.score_red, 1);
    ASSERT_EQ(se_score(&st, 0x3333), SE_OK);          /* 蓝队戒指得分 */
    ASSERT_EQ(st.score_blue, 1);
}
TEST(inactive_ring_rejected) {
    se_state_t st; se_init(&st);
    ASSERT_EQ(se_checkin(&st, 0x1111, SE_SIDE_RED), SE_OK);
    ASSERT_EQ(se_score(&st, 0x9999), SE_ERR_RING_INACTIVE);  /* 未报到 */
}
TEST(side_full_and_duplicate_rejected) {
    se_state_t st; se_init(&st);
    se_checkin(&st, 1, SE_SIDE_RED); se_checkin(&st, 2, SE_SIDE_RED);
    ASSERT_EQ(se_checkin(&st, 3, SE_SIDE_RED), SE_ERR_SIDE_FULL);
    ASSERT_EQ(se_checkin(&st, 1, SE_SIDE_BLUE), SE_ERR_DUP);  /* 同戒指不可双报 */
}
TEST(rotation_invalidates_old_map) {
    se_state_t st; se_init(&st);
    se_checkin(&st, 1, SE_SIDE_RED); se_checkin(&st, 2, SE_SIDE_RED);
    se_checkin(&st, 3, SE_SIDE_BLUE); se_checkin(&st, 4, SE_SIDE_BLUE);
    se_score(&st, 1);
    se_reset_checkins(&st);                            /* 轮转 */
    ASSERT_EQ(se_score(&st, 1), SE_ERR_RING_INACTIVE); /* 旧映射失效 */
    ASSERT_EQ(st.score_red, 1);                        /* 比分不清零（局内换人不受影响） */
}

int main(void) {
    RUN(single_point_scores);
    RUN(win_game_at_21_with_two_lead);
    RUN(cap_at_30_after_29_all);
    RUN(match_two_games_wins);
    RUN(announce_court_by_score_parity);
    RUN(announce_flags);
    RUN(undo_rolls_back_last_point);
    RUN(undo_across_game_boundary);
    RUN(undo_empty_returns_error);
    RUN(undo_after_match_over_restores_play);
    RUN(checkin_activates_and_scores);
    RUN(inactive_ring_rejected);
    RUN(side_full_and_duplicate_rejected);
    RUN(rotation_invalidates_old_map);
    TEST_MAIN_END();
}
