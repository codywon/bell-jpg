#ifndef __REGISTER_H__
#define __REGISTER_H__

#include "network.h"

typedef struct Hd_Register_Thread_S
{
    unsigned char	byRegType;
    unsigned char	byIsStoped;
    unsigned short	nPort;

    char		szIpAddr[20];

    int			iSocket;

    unsigned short	nSessionID;
    unsigned char	bySendHBCount;
    unsigned char	byIsRegisterOK;

    unsigned char	byFrameRate[8];

    unsigned int	dwVideoBitRate[8];

    unsigned int	dwAuditBitRate[4];

    pthread_t		threadID;

    struct sockaddr_in	saddr;

    cb_NotifyConnected	NotifyConnected;
    cb_NotifyClose	NotifyClose;
    cb_RecvMessage	RecvMessage;

} HD_REGISTER_THREAD_S, *PHD_REGISTER_THREAD_S;

int HD_NET_REG_Start( PHD_REGISTER_THREAD_S pstRegisterThread );
int HD_NET_REG_Stop( PHD_REGISTER_THREAD_S pstRegisterThread );

int HD_NET_REG_AlarmNotify( PHD_REGISTER_THREAD_S pstRegisterThread, int iEncChanID, int iAlarmType );

int HD_STREAM_SendVideoData(
    PHD_REGISTER_THREAD_S 	pstRegisterThread,
    int 			iEncChanID,
    unsigned char 		byFrameType,
    unsigned int		dwFrameNo,
    const char*		 pszVideoData,
    int 			iDataLen );

int HD_STREAM_SendAudioData(
    PHD_REGISTER_THREAD_S 	pstRegisterThread,
    int			iEncChanID,
    unsigned int		dwFrameNo,
    const char*		 pszAudioData,
    int 			iDataLen );

int HD_STREAM_SendAVData(
    PHD_REGISTER_THREAD_S 	pstRegisterThread,
    int 			iEncChanID,
    unsigned char 		byFrameType,
    unsigned int		dwFrameNo,
    const char*		 pszVideoData,
    int 			iVDataLen,
    const char*		 pszAudioData,
    int 			iADataLen );

int HD_STREAM_SendOsdData(
    PHD_REGISTER_THREAD_S 	pstRegisterThread,
    int 			iEncChanID,
    const char*		pszOsdData,
    int			iOsdDataLen );

int HD_ENC_ModifyParam( int iEncChanID, int bIsMainStream );

#endif //__REGISTER_H__
