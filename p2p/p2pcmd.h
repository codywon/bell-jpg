#ifndef __P2PCMD_H__
#define __P2PCMD_H__

//#include "debug.h"

#define MAX_P2P_CONNECT 	8

#define P2P_CMDCHANNEL  	0
#define P2P_VIDEOCHANNEL   	1
#define P2P_AUDIOCHANNEL   	2
#define P2P_TALKCHANNEL 	3
#define P2P_PLAYBACK    	4
#define P2P_ALARMCHANNEL    5

/* BEGIN: Change CMD_HEAD, because Snapshot Image Size maybe > 0xFFFF=64K */
/*        Modified by wupm(2073111@qq.com), 2014/9/5 */

typedef struct __CMDHEAD
{
    short 	startcode;
    short	cmd;
    unsigned int len;
    //short	version;
} CMDHEAD, *PCMDHEAD;

typedef struct __CMDHEADOLD
{
    short 	startcode;
    short	cmd;
    short	len;
    short	version;
} CMDHEADOLD, *PCMDHEADOLD;

#endif

