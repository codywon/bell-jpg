

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>

#include "debug.h"
#include "param.h"
#include "BaAudioPlay.h"

#if	0
#define	BaAudioTextout(Fmt, args...)
#else
#define	BaAudioTextout	Textout
#endif


static sem_t semBaAudioPlayStart;
static sem_t semBaAudioPlayStop; 
#ifdef SUPPORT_FM34
static sem_t semAudioWaitReboot;
#endif
static volatile unsigned int uAudioRebootFlag = 0;
static unsigned int nIndex = 0xFF;
static unsigned int nCount = 0xFF;
static BaAudioPlayCallback fnBaAudioPlayCallback = NULL;
const char szWaveFile[WF_COUNT][64] =
{
	{"/system/Wireless/startup.wav"},
	{"/system/Wireless/config.wav"},
	{"/system/Wireless/reboot.wav"},
	{"/system/Wireless/configok.wav"},
	{"/system/Wireless/open.wav"},
	{"/system/Wireless/welcome.wav"},
	{"/system/Wireless/call-timeout.wav"},
	{"/system/Wireless/calling.wav"},
	#ifdef FEEDDOG
	{"/system/Wireless/talk-over.wav"},
	#else
	{"/system/Wireless/talk-timeout.wav"},
	#endif
	{"/system/Wireless/talk-over.wav"},
	{"/system/Wireless/Beep.wav"},
	{"/system/Wireless/Beep2.wav"},
	{"/system/Wireless/reset-ok.wav"},
	/* BEGIN: KONX */
	/*        Added by wupm(2073111@qq.com), 2014/10/25 */
	{"/system/Wireless/alarm.wav"},
	{"/system/Wireless/alarm.wav"},
    {"/system/Wireless/wps.wav"}, /* WPS PLAYER*/
    {"/system/Wireless/mute.wav"}
};


void StopAudioPlay()
{
#ifdef ENABLE_AUDIO_PLAY
    if(GetPlayAlarmAudioState())
    {
        BaAudioTextout("Stop audio play...");
        sem_post( &semBaAudioPlayStop);
        SetPlayAlarmAudioState(FALSE);
    }
    else
    {
        BaAudioTextout("Audio Play already exit");
    }
#endif    
}


/*
*index: 0~WF_COUNT-the index of wave file; 0xAA-audio data
*count: play times
*
**/
void StartAudioPlay(unsigned int index, unsigned int count, BaAudioPlayCallback callback)
{	
	Textout("StartAudioPlay the index is %u,the count is %u\n",index,count);
#ifdef ENABLE_AUDIO_PLAY

#else
    BaAudioTextout("ENABLE_AUDIO_PLAY undefine");
    if(callback != NULL)
        callback();
    return;
#endif

    StopAudioPlay();

#ifdef  FEEDDOG
    if ( bparam.stBell.bell_audio == FALSE && (nIndex != WF_STARTUP) && (nIndex != WF_CONFIG))
#else
    if(bparam.stBell.bell_audio == FALSE)
#endif
    {
        Textout("bparam.stBell.bell_audio == FALSE");
        if(callback != NULL)
            callback();    
        return;
    }

#ifdef  KONX
        
    if(index == WF_SCARE)
    {
        SetKonxScareing(TRUE);
    }
        
#endif

    nIndex = index;
    nCount = count;
    fnBaAudioPlayCallback = callback;
    SetPlayAlarmAudioState(TRUE);
    sem_post( &semBaAudioPlayStart);
    Textout("post semBaAudioPlayStart");
}
void* BaAudioPlayProc(void* p)
{
	FILE *f = NULL;
	char szStereoBuffer[WAV_FILE_FRAME_SIZE*2 + 1];
	//char szBuffer[WAV_FILE_FRAME_SIZE+1];
	char* pszBuffer = NULL;
	int nBufferOffset = 0;
	int iRet = 0;
	WAVEFILE_HEADER head;

	unsigned long nFileSize = 0;
    unsigned int index = 0xFF;
    unsigned int count = 0xFF;
    unsigned int times = 0;

    int audioHandler = 0;
    
    while ( 1 )
    {
        sem_wait( &semBaAudioPlayStart );
        index = nIndex;
        count = nCount;

        if(index >= WF_STARTUP && index < WF_COUNT)
        {
            nFileSize = GetFileSize(szWaveFile[index]);
            if(nFileSize == 0)
            {
                BaAudioTextout("Not found ALARM_AUDIO_FILE:%s",szWaveFile[index]);
            
                SetPlayAlarmAudioState(FALSE);
				if(fnBaAudioPlayCallback != NULL)
		        {
		            fnBaAudioPlayCallback();
		        }
                continue;
            }
            
            f = fopen(szWaveFile[index], "rb");
            if ( f == NULL )
            {
                BaAudioTextout("Not found ALARM_AUDIO_FILE");
            
                SetPlayAlarmAudioState(FALSE);
                continue;
            }
            
            iRet = fread(&head, 1, sizeof(WAVEFILE_HEADER), f); 
            //Textout("SampleRate=%d, bits=%d, ch=%d, byterate=%d", head.SampleRate, head.BitsPerSample, head.NumChannels, head.ByteRate);        
            if( iRet != sizeof(WAVEFILE_HEADER) )
            {
                BaAudioTextout("Read Wave file head ERROR");
                fclose(f);
                SetPlayAlarmAudioState(FALSE);
                continue;         
            }

            pszBuffer = (char*)malloc(nFileSize);
            if(pszBuffer == NULL)
            {
                BaAudioTextout("malloc fail(len=%d)", nFileSize);
                fclose(f);
                SetPlayAlarmAudioState(FALSE);
                continue;
            }

            nFileSize = nFileSize/WAV_FILE_FRAME_SIZE;
            nFileSize = nFileSize*WAV_FILE_FRAME_SIZE;
            iRet = fread(pszBuffer, 1, nFileSize, f); 
            if(iRet < 0)
            {
                BaAudioTextout("Read Wave file data ERROR");
                free(pszBuffer);
                pszBuffer = NULL;
                fclose(f);
                SetPlayAlarmAudioState(FALSE);
                continue;
            }
            else if( iRet != nFileSize)
            {
                Textout("Not read full file iRet=%d(%d)", iRet, nFileSize);
                nFileSize = iRet;
            }

            /*The times of play initial*/
            times = 0;
            nBufferOffset = 0;
            fclose(f);

            
        }
        else
        {
            continue;
        }

		#ifdef SUPPORT_FM34
		uAudioRebootFlag = 1;
		sem_post(&semAudioWaitReboot);
		#endif
        while(1)
        {           
            
            memset(szStereoBuffer, 0, WAV_FILE_FRAME_SIZE*2 + 1);
            //if(index >= WF_STARTUP && index < WF_COUNT)
            if(1)
            {
                if ( count > 0 && times >= count )
                {
                    Textout("Wave Play Over, Stop");            
                    SetPlayAlarmAudioState(FALSE);
                    break;
                }

                if(nBufferOffset+WAV_FILE_FRAME_SIZE <= nFileSize)
                {
                    ComplexAudio( pszBuffer+nBufferOffset, szStereoBuffer, WAV_FILE_FRAME_SIZE/2 );
                    nBufferOffset += WAV_FILE_FRAME_SIZE;
                }

                if(nBufferOffset >= nFileSize)
                {
                    times++;
                    nBufferOffset=0;
                }
            }
            else if(index == 0xAA)
            {
                //HD_AudioOutGetDataNew(szStereoBuffer);
            }

            /*Audio Play*/
            audioHandler = BaGetAudioHandler();
            if(audioHandler == -1) 
                break;

            ioctl( audioHandler, 4, szStereoBuffer); //I2S_PUT_AUDIO

			#ifdef SUPPORT_FM34
			uAudioRebootFlag ++;
        	#endif

            /*Whether recved stop event or not*/
            iRet = sem_trywait( &semBaAudioPlayStop );
            if(iRet == 0)
            {
                Textout("Stop audio play OK, index=%d, count=%d", index, count);
				break;   
            }
            else
            {
                //Textout("Can not wait to stop information");
            }

            if(index == WF_CALLING && DoorBellIsTalking())
            {
                Textout("In talking states, stop play calling");
                SetPlayAlarmAudioState(FALSE);
                break;
            }
            //Textout("iRet:%d",iRet);
            
        }

		uAudioRebootFlag = 0;
		
        free(pszBuffer);
        pszBuffer = NULL;

        #ifdef  KONX
        if ( index == WF_SCARE)
        {
            SetKonxScareing(FALSE);
        }
        #endif

        if(fnBaAudioPlayCallback != NULL)
        {
            fnBaAudioPlayCallback();
        }

    }
}

void *AudioWaitRebootProc(void *p)
{
#ifdef SUPPORT_FM34
    int count  = 0;
    int waitTime =100;
	unsigned int preAudioCount = 0;
    unsigned int value = 0;

    while( 1 )
    {
        //Textout("***********************AudioWaitRebootProc***********************");
        sem_wait( &semAudioWaitReboot);
        count = 0;
		preAudioCount = 0;
        
        while(1)
        {
            usleep(100*1000);
            //Textout("preAudioCount = %d, uAudioRebootFlag = %d",preAudioCount,uAudioRebootFlag);
            if( preAudioCount  != uAudioRebootFlag )
            {
            	preAudioCount = uAudioRebootFlag;
				count = 0;
            }
			else if( 0 == uAudioRebootFlag)
			{
				break;
			}
			else
			{
				count ++;
			}

            if( count > waitTime)
            {
            	if( DoorBellIsIdle() == TRUE )
            	{
					Textout("************************Play Audio Error! Reboot The FM34**************************");
					setMotoActionFlag(FALSE);
					fm_Init();
					setMotoActionFlag(TRUE);
					count = 0;
					break;
				}               
            }
        }
    }
#endif
}

//sem_wait( &p2psem );
void BaAudioInit()
{
    pthread_t BaAudioPlayThread;
    pthread_t AudioWaitRebootThread;
    
    if ( ( sem_init( &semBaAudioPlayStart, 0, 0 ) ) != 0 )
    {
        Textout( "semBaAudioPlayStart init failed" );
        return;
    }

    if ( ( sem_init( &semBaAudioPlayStop, 0, 0 ) ) != 0 )
    {
        Textout( "semBaAudioPlayStop init failed" );
        return;
    }


    pthread_create( &BaAudioPlayThread, 0, &BaAudioPlayProc, NULL );
    pthread_detach(BaAudioPlayThread);

	#ifdef SUPPORT_FM34
	
    if( ( sem_init( &semAudioWaitReboot,0,0) ) != 0)
    {
        Textout( "semAudioWaitReboot init failed" );
        return;
    }
	
    pthread_create( &AudioWaitRebootThread,0,&AudioWaitRebootProc,NULL);
    pthread_detach( AudioWaitRebootThread);
	#endif
    
}


