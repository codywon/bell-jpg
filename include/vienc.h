#ifndef __VI_ENC_H__
#define __VI_ENC_H__

#include "hd_common.h"

typedef int ( *cb_VencStreamProc )( int iEncChnID, unsigned char FrameType, unsigned int FrameNo, int length, unsigned char* pstStream );

typedef struct Hd_Venc_Thread_S
{
    int		iEncChanID;
    pthread_t 	iGetVideoID;
    pthread_t      	iGetEncID;
    pthread_t      	iSendEncID;
    pthread_t      	iSendRtspID;
    pthread_t       iSendRtspID1;
    pthread_t       iUser0ID;
    pthread_t       iUser1ID;
    pthread_t       iUser2ID;
    pthread_t       iUser3ID;
} HD_VENC_THREAD_S, *PHD_VENC_THREAD_S;

int HD_H264Enc_Close( int iEncGrp, int iEncChn );

int HD_H264Enc_Start( PHD_VENC_THREAD_S pstVENCThread );

int HD_H264Enc_Stop( PHD_VENC_THREAD_S pstVENCThread );

int HD_H264Enc_RequestKeyFrame( int iEncChn );

extern char	byMotionStart;
extern char     byMotionHappen;

#endif //__VI_ENC_H__
