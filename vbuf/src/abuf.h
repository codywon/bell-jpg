
#ifndef _A_BUF_H_
#define _A_BUF_H_

#define MAX_FRAME_CNT		240

#define EBUF_ASIZE			1024 * 256
#define EBUF_MAX_ALEN		1024 * 256 * 4

extern ENCBUF          encbufa;

/*
*Parameter
*In:
*out:
*Init Audio Buffer
*/
void EncABufInit( void );

/*
*Parameter
*In:
*pbuf: adpcm audio data
*len: the length of adpcm data; In RT5350, len = 512
*frameno: > 1, 1,2,3,4,5,......
*Out:
*Calling this API, Push video Frame to this buffer. P2P AND WEB lib Pop it by themsel
*/
void PushAencData( unsigned char* pbuf, int len, unsigned int frameno );

void audiolock( void );
void audiounlock( void );

//adpcm
int GetEncoderAduioPreSample();
int GetEncoderAudioIndex();


#endif



