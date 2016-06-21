#ifndef __AI_ENC_H__
#define __AI_ENC_H__

#include <semaphore.h>


typedef struct Hd_Aenc_Thread_S
{
    unsigned char	bIsStoped;
    unsigned char	byIs16KSample;
    unsigned short	nReserved;

    int 		bUseAEC;

    int 		iAiDevID;

    int 		iAiChnID;

    int 		iAENCChnID;

    pthread_t 		threadPid;
} HD_AENC_THREAD_S, *PHD_AENC_THREAD_S;
#if 0
int amr_init( void );
extern sem_t           amr_sem;
extern volatile int    		g_u32_dsp_aud_msg_flag;
extern unsigned char		g_aud_msgfrmdsp[64 * 2];
extern unsigned int            inaudiodcpt;
extern unsigned int            outaudiodcpt;
extern cal_mcu_buff_dcpt*       pinaudiodcpt;
extern cal_mcu_buff_dcpt*       poutaudiodcpt;

extern unsigned int    		inaudioaddr;
extern unsigned int    		outaudioaddr;
extern unsigned char*  		 pinaddr;
extern unsigned char*  		 poutaddr;

void audlock( void );
void audunlock( void );
#endif
#endif //__AI_ENC_H__
