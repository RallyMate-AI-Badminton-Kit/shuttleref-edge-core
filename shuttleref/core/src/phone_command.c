#include "phone_command.h"

#include <ctype.h>
#include <string.h>

static bool sequence_valid(const char *value, size_t length)
{
    if (length == 0 || length > PHONE_COMMAND_SEQ_MAX) return false;
    for (size_t i = 0; i < length; ++i) {
        unsigned char ch = (unsigned char)value[i];
        if (!isalnum(ch) && ch != '-' && ch != '_') return false;
    }
    return true;
}

se_err_t phone_command_apply(se_state_t *score, phone_command_state_t *history,
                             const char *command, size_t length)
{
    if (score == NULL || history == NULL || command == NULL || length == 0 || length > 64) {
        return SE_ERR_NOT_UNDOABLE;
    }
    if (length == 4 && memcmp(command, "undo", 4) == 0) return se_undo(score);
    if (length == 11 && memcmp(command, "reset_match", 11) == 0) {
        se_init(score);
        return SE_OK;
    }

    static const char red_prefix[] = "point:red:";
    static const char blue_prefix[] = "point:blue:";
    size_t prefix_length;
    se_side_t side;
    if (length > sizeof(red_prefix) - 1 &&
        memcmp(command, red_prefix, sizeof(red_prefix) - 1) == 0) {
        prefix_length = sizeof(red_prefix) - 1;
        side = SE_SIDE_RED;
    } else if (length > sizeof(blue_prefix) - 1 &&
               memcmp(command, blue_prefix, sizeof(blue_prefix) - 1) == 0) {
        prefix_length = sizeof(blue_prefix) - 1;
        side = SE_SIDE_BLUE;
    } else {
        return SE_ERR_NOT_UNDOABLE;
    }

    const char *sequence = command + prefix_length;
    size_t sequence_length = length - prefix_length;
    if (!sequence_valid(sequence, sequence_length)) return SE_ERR_NOT_UNDOABLE;
    for (unsigned i = 0; i < PHONE_COMMAND_HISTORY; ++i) {
        if (strlen(history->recent[i]) == sequence_length &&
            memcmp(history->recent[i], sequence, sequence_length) == 0) {
            return SE_ERR_DUP;
        }
    }

    se_err_t result = se_score_side(score, side);
    if (result != SE_OK) return result;
    char *slot = history->recent[history->next];
    memcpy(slot, sequence, sequence_length);
    slot[sequence_length] = '\0';
    history->next = (history->next + 1) % PHONE_COMMAND_HISTORY;
    return SE_OK;
}
