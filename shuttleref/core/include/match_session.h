#ifndef MATCH_SESSION_H
#define MATCH_SESSION_H
#include "score_engine.h"
typedef struct {
    char id[33], token[9];
    char state; /* i idle, a active, p paused */
    unsigned code;
    uint32_t watermark, point_watermark;
    char recent[8][65];
    unsigned next;
} match_session_t;
/* Expand the 32-byte DP110 wire command into the internal text protocol. */
bool match_command_decode(const char *, char *, size_t);
/* true means lifecycle command consumed, including a rejected request. */
bool match_session_apply(match_session_t *, se_state_t *, const char *);
/* Local touch scoring preserves phone command acknowledgement and sequence. */
se_err_t match_session_score_local(const match_session_t *, se_state_t *, se_side_t);
void match_session_report(const match_session_t *, const se_state_t *, char *, size_t);
#endif
