
#ifndef _SUB_BUF_H_
#define _SUB_BUF_H_

#include "param.h"

extern ENCBUF           encbufs;
extern ENCBUFIND       	userinds[MAX_CONN_USER_NUM];
extern ENCBUFIND       	userindsubp2p[MAX_CONN_USER_NUM];


//=================Extern api===========================
/*
*Parameter
*In:
*out:
*Init Video Buffer
*/
void EncSBbufInit( void );

/*
*Parameter
*In:
*pbuf: video data
*type: 0-IFrame, 1-other frame
*frametype:0x67,0x68, 0x06, 0x65, 0x61
*size: 0-VGA, 1-QVGA, 3-720P
*len: the length of video data
*frameno: > 1, 1,2,3,4,5,......
*Out:
*Calling this API, Push video Frame to this buffer. P2P AND WEB lib Pop it by themsel
*/
void PushVSencData( unsigned char* pbuf, char type, char size, int len, unsigned int frameno );

//=================P2P AND WEB LIB CALL THEM===========================

void subLock( void );
void subUnlock( void );

unsigned char GetSubFrameRate();

#endif

