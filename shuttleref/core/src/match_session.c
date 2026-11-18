#include "match_session.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
static bool hex(const char *s, unsigned n) {
    for (unsigned i=0;i<n;i++) if (!isxdigit((unsigned char)s[i])) return false;
    return true;
}
bool match_session_apply(match_session_t *m, se_state_t *s, const char *cmd) {
    if (!cmd || !strchr("BPRAQU",cmd[0]) || cmd[1]!=':') return false;
    size_t n=strlen(cmd);
    if (n<43 || n>64 || cmd[34]!=':' || !hex(cmd+2,32) || !hex(cmd+35,8)) return true;
    char id[33]; memcpy(id,cmd+2,32);id[32]=0;
    memcpy(m->token,cmd+35,8);m->token[8]=0;m->code=0;
    bool same=!strcmp(m->id,id), idle=!m->id[0] || m->state=='i';
    for (unsigned i=0;i<8;i++) if (!strcmp(m->recent[i],cmd)) return true;
    uint32_t seq=(uint32_t)strtoul(m->token,NULL,16);
    if (seq<=m->watermark) {snprintf(m->token,sizeof(m->token),"%08lx",(unsigned long)m->watermark);m->code=3;return true;}
    m->watermark=seq;m->point_watermark=seq;
    if (cmd[0]!='R' && n!=43) {m->code=1;return true;}
    if (cmd[0]=='Q') return true;
    if (!idle && !same && m->state!='p') {m->code=2;return true;}
    if (cmd[0]=='U') {
        if (!same || m->state!='a' || se_undo(s)!=SE_OK) {m->code=1;return true;}
    } else if (cmd[0]=='B') {
        if (!idle && same) return true;
        /* A new court may replace a saved/paused court, never a legacy live score. */
        if (idle && (s->score_red || s->score_blue || s->games_red || s->games_blue)) {m->code=2;return true;}
        se_init(s);strcpy(m->id,id);m->state='a';
    } else if (cmd[0]=='P') {
        if (!idle && !same) {m->code=2;return true;}
        strcpy(m->id,id);m->state='p';
    } else if (cmd[0]=='A') {
        if (!idle && !same) {m->code=2;return true;}
        /* Keep released ID in ack so the caller can verify the intended match. */
        strcpy(m->id,id);m->state='i';se_init(s);
    } else {
        unsigned r,b,gr,gb,server;int used=0;
        if (n<45 || cmd[43]!=':' || sscanf(cmd+44,"%u,%u,%u,%u,%u%n",&r,&b,&gr,&gb,&server,&used)!=5 || cmd[44+used]) {m->code=1;return true;}
        if (idle && !m->id[0] && (s->score_red || s->score_blue || s->games_red || s->games_blue)) {m->code=2;return true;}
        if (!idle && same && m->state=='a') {m->code=2;return true;}
        if (r>29 || b>29 || gr>1 || gb>1 || server>1 || ((r>=21 || b>=21) && (r>b?r-b:b-r)>=2)) {m->code=1;return true;}
        se_restore(s,r,b,gr,gb,(se_side_t)server);
        strcpy(m->id,id);m->state='a';
    }
    strcpy(m->recent[m->next],cmd);m->next=(m->next+1)%8;
    return true;
}
void match_session_report(const match_session_t *m,const se_state_t *s,char *buf,size_t cap) {
    snprintf(buf,cap,"@1:%s:%s:%c:%u,%u,%u,%u,%u:%u",m->id[0]?m->id:"00000000000000000000000000000000",m->token[0]?m->token:"00000000",m->state?m->state:'i',s->score_red,s->score_blue,s->games_red,s->games_blue,s->server,m->code);
}

/* Fixed-size base64 envelope keeps the complete UUID and sequence within DP110. */
bool match_command_decode(const char *wire,char *out,size_t cap) {
    static const char alphabet[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (!wire || strlen(wire)!=32 || cap<65) return false;
    unsigned char bytes[24];
    for (unsigned i=0,j=0;i<32;i+=4,j+=3) {
        unsigned n=0;
        for(unsigned k=0;k<4;k++) {const char *p=strchr(alphabet,wire[i+k]);if(!p)return false;n=(n<<6)|(unsigned)(p-alphabet);}
        bytes[j]=(unsigned char)(n>>16);bytes[j+1]=(unsigned char)(n>>8);bytes[j+2]=(unsigned char)n;
    }
    if(!bytes[0] || !strchr("BPRAQUrb",bytes[0]))return false;
    if(bytes[0]=='R') {if(bytes[21]>29 || bytes[22]>29 || bytes[23]>7)return false;}
    else if(bytes[21] || bytes[22] || bytes[23])return false;
    char id[33],token[9];
    for(unsigned i=0;i<16;i++)snprintf(id+i*2,3,"%02x",bytes[i+1]);
    for(unsigned i=0;i<4;i++)snprintf(token+i*2,3,"%02x",bytes[i+17]);
    if(bytes[0]=='r' || bytes[0]=='b')snprintf(out,cap,"point:%s:%s_%s",bytes[0]=='r'?"red":"blue",id,token);
    else if(bytes[0]=='R')snprintf(out,cap,"R:%s:%s:%u,%u,%u,%u,%u",id,token,bytes[21],bytes[22],bytes[23]&1,(bytes[23]>>1)&1,(bytes[23]>>2)&1);
    else snprintf(out,cap,"%c:%s:%s",bytes[0],id,token);
    return true;
}

se_err_t match_session_score_local(const match_session_t *m, se_state_t *s, se_side_t side) {
    if (m->state == 'p' || (m->id[0] && m->state != 'a')) return SE_ERR_NOT_UNDOABLE;
    return se_score_side(s, side);
}
