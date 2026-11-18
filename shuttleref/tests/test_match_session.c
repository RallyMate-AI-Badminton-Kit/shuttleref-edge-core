#include "match_session.h"
#include <assert.h>
#include <string.h>
int main(void) {
 se_state_t s;se_init(&s);match_session_t m={0};char buf[65];
 assert(match_command_decode("QhERERERERERERERERERERESNFZ4AAAA",buf,sizeof(buf)));assert(!strcmp(buf,"B:11111111111111111111111111111111:12345678"));
 assert(match_command_decode("UBERERERERERERERERERERESNFZ4AAAA",buf,sizeof(buf)));assert(!strcmp(buf,"P:11111111111111111111111111111111:12345678"));
 assert(match_command_decode("UhERERERERERERERERERERESNFZ4DAkF",buf,sizeof(buf)));assert(!strcmp(buf,"R:11111111111111111111111111111111:12345678:12,9,1,0,1"));
 assert(match_command_decode("QRERERERERERERERERERERESNFZ4AAAA",buf,sizeof(buf)));assert(!strcmp(buf,"A:11111111111111111111111111111111:12345678"));
 assert(match_command_decode("URERERERERERERERERERERESNFZ4AAAA",buf,sizeof(buf)));assert(!strcmp(buf,"Q:11111111111111111111111111111111:12345678"));
 assert(match_command_decode("VRERERERERERERERERERERESNFZ4AAAA",buf,sizeof(buf)));assert(!strcmp(buf,"U:11111111111111111111111111111111:12345678"));
 assert(match_command_decode("chERERERERERERERERERERESNFZ4AAAA",buf,sizeof(buf)));assert(!strcmp(buf,"point:red:11111111111111111111111111111111_12345678"));
 assert(match_command_decode("YhERERERERERERERERERERESNFZ4AAAA",buf,sizeof(buf)));assert(!strcmp(buf,"point:blue:11111111111111111111111111111111_12345678"));
 assert(!match_command_decode("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",buf,sizeof(buf)));
 assert(match_session_apply(&m,&s,"B:11111111111111111111111111111111:12345678"));
 assert(!m.code && m.state=='a');se_score_side(&s,SE_SIDE_RED);
 match_session_apply(&m,&s,"P:11111111111111111111111111111111:12345679");assert(m.state=='p');
 match_session_apply(&m,&s,"R:11111111111111111111111111111111:12345680:12,9,1,0,1");
 assert(m.state=='a' && s.score_red==12 && s.server==SE_SIDE_BLUE && s.games_red==1);
 assert(se_undo(&s)==SE_ERR_NOT_UNDOABLE);
 se_score_side(&s,SE_SIDE_BLUE);assert(se_undo(&s)==SE_OK && s.score_blue==9 && s.games_red==1);
 se_score_side(&s,SE_SIDE_RED);
 match_session_apply(&m,&s,"R:11111111111111111111111111111111:12345680:12,9,1,0,1");assert(s.score_red==13);
 match_session_apply(&m,&s,"R:22222222222222222222222222222222:12345681:2,1,0,0,0");assert(m.code==2 && s.score_red==13);
 match_session_apply(&m,&s,"A:22222222222222222222222222222222:12345682");assert(m.code==2 && s.score_red==13);
 match_session_apply(&m,&s,"P:11111111111111111111111111111111:12345683");
 match_session_apply(&m,&s,"R:11111111111111111111111111111111:12345684:21,1,0,0,0");assert(m.code==1 && m.state=='p');
 match_session_report(&m,&s,buf,sizeof(buf));assert(strlen(buf)<=64);
 match_session_apply(&m,&s,"A:11111111111111111111111111111111:12345685");assert(m.state=='i' && s.score_red==0);
 match_session_apply(&m,&s,"Q:11111111111111111111111111111111:12345700");
 match_session_apply(&m,&s,"R:11111111111111111111111111111111:12345686:2,1,0,0,0");
 assert(m.code==3 && s.score_red==0 && m.state=='i');
 se_init(&s); memset(&m,0,sizeof(m));
 assert(match_session_score_local(&m,&s,SE_SIDE_RED)==SE_OK && s.score_red==1);
 assert(match_session_score_local(&m,&s,SE_SIDE_BLUE)==SE_OK && s.score_blue==1);
 strcpy(m.id,"11111111111111111111111111111111");m.state='a';
 m.watermark=42;m.point_watermark=42;strcpy(m.token,"0000002a");
 match_session_t before=m;
 assert(match_session_score_local(&m,&s,SE_SIDE_RED)==SE_OK && s.score_red==2);
 assert(!memcmp(&m,&before,sizeof(m)));
 m.state='p';assert(match_session_score_local(&m,&s,SE_SIDE_RED)==SE_ERR_NOT_UNDOABLE && s.score_red==2);
 m.state='i';assert(match_session_score_local(&m,&s,SE_SIDE_BLUE)==SE_ERR_NOT_UNDOABLE && s.score_blue==1);
 m.state='a';s.match_over=true;assert(match_session_score_local(&m,&s,SE_SIDE_BLUE)==SE_ERR_MATCH_OVER);
 return 0;
}
