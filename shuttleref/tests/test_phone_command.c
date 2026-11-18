#include <assert.h>
#include <string.h>
#include "phone_command.h"

static se_err_t apply(se_state_t *score, phone_command_state_t *history, const char *command)
{
    return phone_command_apply(score, history, command, strlen(command));
}

int main(void)
{
    se_state_t score;
    phone_command_state_t history = {0};
    se_init(&score);

    assert(apply(&score, &history, "point:red:abc1") == SE_OK);
    assert(apply(&score, &history, "point:red:abc1") == SE_ERR_DUP);
    assert(score.score_red == 1);
    assert(apply(&score, &history, "point:red:abc2") == SE_OK);
    assert(apply(&score, &history, "point:blue:abc3") == SE_OK);
    assert(score.score_red == 2 && score.score_blue == 1);
    assert(apply(&score, &history, "point:red:abc1") == SE_ERR_DUP);
    assert(score.score_red == 2);
    assert(apply(&score, &history, "undo") == SE_OK);
    assert(score.score_blue == 0);
    assert(apply(&score, &history, "point:blue:") != SE_OK);
    assert(apply(&score, &history, "point:red:bad!seq") != SE_OK);
    assert(apply(&score, &history, "other") != SE_OK);
    assert(score.score_red == 2 && score.score_blue == 0);
    assert(apply(&score, &history, "reset_match") == SE_OK);
    assert(score.score_red == 0 && score.score_blue == 0);

    char too_long[100];
    memset(too_long, 'a', sizeof(too_long));
    assert(phone_command_apply(&score, &history, too_long, sizeof(too_long)) != SE_OK);
    return 0;
}
