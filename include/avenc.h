#ifndef __AV_ENC_H__
#define __AV_ENC_H__

#include "hd_common.h"

typedef int ( *cb_AvencStreamProc )( int iAVChanID, int iAencChanID, int iVencChanID, /*AVENC_LIST_S*/unsigned char* pVideoNode, /*AVENC_LIST_S*/unsigned char* pAudioNode );

typedef struct Hd_AVenc_Thread_S
{
    int 		bIsStoped;

    int 		iAVencChanID;
    int 		iAiDevID;
    int 		iAiChanID;
    int 		iAencChanID;
    int			iVencChanID;

    pthread_t 		threadPid;

    int 		iTspErr;

    cb_AvencStreamProc	encStreamFunc;

} HD_AVENC_THREAD_S, *PHD_AVENC_THREAD_S;

int HD_AVEnc_Open( PHD_AVENC_THREAD_S pstAVEncThread );

int HD_AVEnc_Close( PHD_AVENC_THREAD_S pstAVEncThread );

int HD_AVEnc_Start( PHD_AVENC_THREAD_S pstAVEncThread );

int HD_AVEnc_Stop( PHD_AVENC_THREAD_S pstAVEncThread );

#endif //__AI_ENC_H__
