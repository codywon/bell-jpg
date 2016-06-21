
#ifndef _VIDEO_BUFFER_H_
#define _VIDEO_BUFFER_H_

#include "param.h"
#include "init.h"

extern ENCBUF           encbufm;
extern ENCBUFIND       	userindm[MAX_CONN_USER_NUM];
extern ENCBUFIND       	userindp2p[MAX_CONN_USER_NUM];

extern ENCBUFIND       	userindsd;
extern ENCBUFIND        userindonvif;
//=================Extern api===========================
/*
*Parameter
*In:
*out:
*Init Video Buffer
*/
void EncMBbufInit( void );

/*
*Parameter
*In:
*pbuf: video data
*streamid: 0
*type: 0-IFrame, 1-other frame
*NALU Type:0x67,0x68, 0x06, 0x65, 0x61
*size: 0-VGA, 1-QVGA, 3-720P
*len: the length of video data
*frameno: > 1, 1,2,3,4,5,......
*Out:
*Calling this API, Push video Frame to this buffer. P2P AND WEB lib Pop it by themsel
*/
//void PushVMencData( unsigned char* pbuf, char type, char size, int len, unsigned int frameno );
void PushVMencData( unsigned char* pbuf, char streamid, char type, char size, int len, unsigned int frameno );

//=================P2P AND WEB LIB CALL THEM===========================

void mainLock( void );
void mainUnlock( void );

void SdInitUser( void );

unsigned char GetMainFrameRate();

#endif

