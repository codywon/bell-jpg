#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <asm/byteorder.h>
#include <semaphore.h>
#include <pthread.h>

#include "vbuf.h"
#include "protocol.h"
#include "param.h"
#include "debug.h"

#include "BaAudioPlay.h"

#if	0
#define	AudioTextout(Fmt, args...)
#else
#define	AudioTextout	Textout
#endif

#define AUDIOOUT_BUF  1024 * 32

int		audiofd	= -1;


unsigned char   audoutbuf[AUDIOOUT_BUF];
unsigned char   szpcmwrite[1024 * 8];
unsigned char   rdpcmdata[1024 * 16];
unsigned int      s_iRecvABufLen = 0;
int             		s_iSendABufLen = 0;
int             		s_iAudOutRLen = 0;
unsigned int           audoffset = 0;
int				audinoffset = 0;
char				rtspaudio = 0;
char				rtspaudio1 = 0;


void ComplexAudio( unsigned char* pszSrc, unsigned char* pszDst, unsigned int dwAudioLen );
void deComplexAudio( unsigned char* pszSrc, unsigned char* pszDst, unsigned int dwAudioLen );

int AudioCodecInit( void )
{
    audiofd = open( "/dev/i2s0", O_RDWR );;

    if ( audiofd < 0 )
    {
        AudioTextout( "Open I2S failed" );
        return -1;
    }

    AudioTextout("Open I2S SUCCESS");
    return 0;
}
void AudioCodecOpen()
{
    ioctl( audiofd, 6, NULL );	//I2S_RX_ENABLE
    ioctl( audiofd, 2, NULL );	//I2S_TX_ENABLE
}
int AudioInRelease( void )
{
    ioctl( audiofd, 7, NULL );	//I2S_RX_DISABLE
    ioctl( audiofd, 3, NULL );	//I2S_TX_DISABLE
    close(audiofd);
    audiofd = -1;
    return 0;
}
/* add begin by yiqing, 2015-07-16, Ô­Òò: */
void AudioCodecClose()
{
    if(audiofd != -1)
    {
        close(audiofd);
        audiofd = -1;
    }
    
}
/* add end by yiqing, 2015-07-16 */

int BaGetAudioHandler()
{
    return audiofd;
}


static BOOL bStartPlayAlarmAudio = FALSE;
pthread_mutex_t mutexPlayAlarmAudioState = PTHREAD_MUTEX_INITIALIZER;
BOOL GetPlayAlarmAudioState()
{
    BOOL bRet = FALSE;
    pthread_mutex_lock(&mutexPlayAlarmAudioState);
    bRet = bStartPlayAlarmAudio;
    pthread_mutex_unlock(&mutexPlayAlarmAudioState);
    return bRet;
}
void SetPlayAlarmAudioState(BOOL b)
{   
    SetAudioGongFang(b);
    pthread_mutex_lock(&mutexPlayAlarmAudioState);
    Textout("Set bStartPlayAlarmAudio = %s", b==0?"FALSE":"TRUE");    
    bStartPlayAlarmAudio = b;
    pthread_mutex_unlock(&mutexPlayAlarmAudioState);
}



#ifdef  KONX
pthread_mutex_t mutexKonxScarePlay = PTHREAD_MUTEX_INITIALIZER;
static BOOL bKonxScarePlaying = FALSE;
BOOL bKonxScareing()
{
    BOOL bRet = FALSE;
    pthread_mutex_lock(&mutexKonxScarePlay);
    bRet = bKonxScarePlaying;
    pthread_mutex_unlock(&mutexKonxScarePlay);
    return bRet;
}
void SetKonxScareing(BOOL b)
{
    pthread_mutex_lock(&mutexKonxScarePlay);   
    AudioTextout("Set bKonxScarePlaying = %s", b==0?"FALSE":"TRUE");
    bKonxScarePlaying = b;
    pthread_mutex_unlock(&mutexKonxScarePlay);
}
#endif


static int nWriteOffset = 0;
static int nReadOffset = 0;
static char NewAoBuffer[AUDIOOUT_BUF];
pthread_mutex_t aolock = PTHREAD_MUTEX_INITIALIZER;
void faolock( void )
{
    pthread_mutex_lock( &aolock );
}
void faounlock( void )
{
    pthread_mutex_unlock( &aolock );
}

int HD_AudioOutGetDataNew(unsigned char* audioBuff)
{
    int iRet = 0;
    faolock();
    
    if( (nReadOffset + WAV_FILE_FRAME_SIZE) <= nWriteOffset)
    {
        ComplexAudio( NewAoBuffer + nReadOffset, audioBuff, WAV_FILE_FRAME_SIZE/2 );
    
        nReadOffset += WAV_FILE_FRAME_SIZE;
        iRet = 0;
    }
    else
    {
        iRet = -1;
    }
    faounlock();

    return iRet;
}

int bRecordAudioStatus = 0;
int iAudioRecordNameNo = 0;
int bPlayAudioStatus = 0;
FILE *audiofp;

void setRecordAudioStatus(int status)
{
	bRecordAudioStatus = status;
	char audioName[256] = {0};
	
	sprintf(audioName,"/tmp/%d.pcm",iAudioRecordNameNo);
	if(1 == status)
	{
		audiofp = fopen(audioName,"ab+");
		if(audiofp == NULL)
		{
			Textout("open %s error",audioName);
		}
		else
		{
			Textout("open %s succedd",audioName);
		}
		bPlayAudioStatus = 0;
	}
	else
	{
		//fclose(audiofp);
		bPlayAudioStatus = 1;
		unsigned long nFileSize = 0;
		char* pszBuffer = NULL;
		int nBufferOffset = 0;
		int audioHandler = 0;

		nFileSize = GetFileSize(audioName);
		
		pszBuffer = (char*)malloc(nFileSize);
        if(pszBuffer == NULL)
        {
            Textout("malloc fail(len=%d)", nFileSize);
            fclose(audiofp);
            return;
        }
		

		char szStereoBuffer[WAV_FILE_FRAME_SIZE*2 + 1];

		int iRet = fread(pszBuffer, 1, nFileSize, audiofp);
		while(1)
        {           
            memset(szStereoBuffer, 0, WAV_FILE_FRAME_SIZE*2 + 1);

			if(nBufferOffset+WAV_FILE_FRAME_SIZE <= nFileSize)
			{
				memcpy(  szStereoBuffer,pszBuffer+nBufferOffset, WAV_FILE_FRAME_SIZE*2+1 );
				nBufferOffset += WAV_FILE_FRAME_SIZE*2+1;
			}
			else
			{
				break;
			}
			
            ioctl( audiofd, 4, szStereoBuffer); //I2S_PUT_AUDIO

			if(nBufferOffset >= nFileSize)
			{
				nBufferOffset=0;
				fclose(audiofp);
				break;
			}

		}
		
		//Textout("close %s ",audioName);
		iAudioRecordNameNo++;
	}
}

int HD_AudioOutSendDataNew( unsigned char* pszAudio, int iDataLen)
{
    if ( pszAudio && ( iDataLen > 0 ) )
    {
        faolock();

        if ( nWriteOffset <= AUDIOOUT_BUF - WAV_FILE_FRAME_SIZE )
        {
            DecoderClr( 0, 0 );
            ADPCMDecode( pszAudio, iDataLen, NewAoBuffer + nWriteOffset, 0 );
			/*
			if(1 == bRecordAudioStatus && audiofp != NULL)
			{
				fwrite(NewAoBuffer + nWriteOffset,1,iDataLen * 4,audiofp);
			}
			*/
            nWriteOffset += iDataLen * 4;

			
        }

        //Textout("WBefore,nReadOffset=%d, nWriteOffset=%d", nReadOffset, nWriteOffset);
        if ( ( AUDIOOUT_BUF - nWriteOffset ) < WAV_FILE_FRAME_SIZE && nReadOffset > 0 )
        {
            memcpy( NewAoBuffer, NewAoBuffer + nReadOffset, nWriteOffset - nReadOffset );
            nWriteOffset -= nReadOffset;
            nReadOffset = 0;
        }

        //Textout("WAfter,nReadOffset=%d, nWriteOffset=%d", nReadOffset, nWriteOffset);

        faounlock();
    }

    return 0;
}

int OutputAudioData( unsigned char* pszAudio, int iDataLen)
{
    HD_AudioOutSendDataNew(pszAudio, iDataLen);
}

int bEnableAudioIn = 1;
void SetAudioInStatus(int status)
{
	bEnableAudioIn = status;
    Textout("setbEnableAudioIn = %d ",status);
}

void* AudioCapture( void* p )
{
    unsigned int aframeno = 0;
    unsigned int aframeno1 = 0;
    int	i = 0;
    unsigned char temp[1024 * 4];
    unsigned int    offset = 0;
    int	iRet;
    printf( "audio capture:%d\n", getpid() );

    while ( 1 )
    {
        
        if(bEnableAudioIn == 0)
        {
            usleep( 1000*1000 );
            continue;
        }
        
        if(audiofd == -1) break;        
        memset(temp, 0, 4*1024);
        iRet = ioctl( audiofd, 5, temp );	//I2S_GET_AUDIO

		/*
		if(1 == bRecordAudioStatus && audiofp != NULL)
		{
			fwrite(temp,1,WAV_FILE_FRAME_SIZE*4,audiofp);
		}
		*/
		
        deComplexAudio( rdpcmdata + offset, temp, WAV_FILE_FRAME_SIZE / 2 );

        offset += WAV_FILE_FRAME_SIZE;

        if ( offset  >= WAV_FILE_FRAME_SIZE * 2 )
        {
            EncoderClr(0,0);

			//if(bEnableAudioIn)
			{
	            ADPCMEncode( rdpcmdata, WAV_FILE_FRAME_SIZE * 2, temp );
	            PushAencData( temp, WAV_FILE_FRAME_SIZE / 2, aframeno++ );
	            NoteP2pAudio();
	            NoteAudioSend();
			}

            offset = 0;
            usleep( 10000 );
        }
    }
}

void BaAudioTalkProc(void* p)
{
    int iRet = -1;
    unsigned char szAudioTalk[WAV_FILE_FRAME_SIZE*2];
		
    while(1)
    {         
        if(audiofd == -1) break;
        if(GetPlayAlarmAudioState())
        {
            usleep(40*1000);
            continue;
        }
        
        memset(szAudioTalk, 0, WAV_FILE_FRAME_SIZE*2);
        iRet = HD_AudioOutGetDataNew(szAudioTalk);
        if(iRet != 0){
            //usleep(10*1000);
            usleep(5*1000);
        }

		/*
		if(1 == bRecordAudioStatus && audiofp != NULL)
		{
			fwrite(szAudioTalk,1,WAV_FILE_FRAME_SIZE*2,audiofp);
		}
		*/
		

		ioctl( audiofd, 4, szAudioTalk );

    }
}


int AudioStart( void )
{
    int 		iRet = 0;
    pthread_t audinPid;
    pthread_t BaAudioTalkThread;
    
    EncABufInit();
    BaAudioInit();
    iRet = pthread_create( &audinPid, 0, &AudioCapture, NULL );
	pthread_detach(audinPid);
    pthread_create( &BaAudioTalkThread, 0, &BaAudioTalkProc, NULL );
    pthread_detach(BaAudioTalkThread);

    return iRet;
}

void ComplexAudio( unsigned char* pszSrc, unsigned char* pszDst, unsigned int dwAudioLen )
{
    int			i;
    unsigned short*	 pnSrc = ( unsigned short* )pszSrc;
    unsigned short*	 pnDst = ( unsigned short* )pszDst;

    for ( i = 0; i < dwAudioLen; i++ )
    {
        pnDst[i * 2] 	= pnSrc[i];
        pnDst[i * 2 + 1] = pnSrc[i];
    }
}

void deComplexAudio( unsigned char* pdst, unsigned char* psrc, unsigned int len )
{
    int                 i;
    unsigned short*      pnSrc = ( unsigned short* )psrc;
    unsigned short*      pnDst = ( unsigned short* )pdst;

    for ( i = 0; i < len; i++ )
    {
        pnDst[i]    = pnSrc[i * 2];
    }
}


