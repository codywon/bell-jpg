
#ifndef _MJPG_BUF_H_
#define _MJPG_BUF_H_

#include "param.h"

extern ENCBUF          encbufj;
extern ENCBUFIND       	userindj[MAX_CONN_USER_NUM];



//=================Extern api===========================
/*
*Parameter
*In:
*out:
*Init Video Buffer
*/
void EncJBufInit( void );

/*
*Parameter
*In:
*pbuf: video data
*streamid: 0x03
*type: 0x03
*size: 1
*len: the length of video data
*frameno: 0,1,2,3,4,5,......
*Out:
*Calling this API, Push video Frame to this buffer. P2P AND WEB lib Pop it by themself
*/
//void PushVJencData( unsigned char* pbuf, char type, char size, int len, unsigned int frameno );
void PushVJencData( unsigned char* pbuf, char streamid, char type, char size, int len, unsigned int frameno );

//=================P2P AND WEB LIB CALL THEM===========================

void mjpglock( void );
void mjpgunlock( void );
#endif

