#ifndef SHUTTLEREF_PHONE_COMMAND_H
#define SHUTTLEREF_PHONE_COMMAND_H

#include "score_engine.h"

#define PHONE_COMMAND_SEQ_MAX 48
#define PHONE_COMMAND_HISTORY 8

typedef struct {
    char recent[PHONE_COMMAND_HISTORY][PHONE_COMMAND_SEQ_MAX + 1];
    unsigned next;
} phone_command_state_t;

/* Apply a DP 110 command. The input is length-delimited and need not be NUL-terminated. */
se_err_t phone_command_apply(se_state_t *score, phone_command_state_t *history,
                             const char *command, size_t length);

#endif
