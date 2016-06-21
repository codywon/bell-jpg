#include <sys/types.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <sys/time.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <sys/stat.h>

#include <semaphore.h>

#include "alarm.h"
#include "param.h"
#include "init.h"
#include "debug.h"

#include "sensorapi.h"

/* BEGIN: Added by wupm, 2013/11/1 */
//extern TTT_DOORBELL bell_data;

ALARMACTION	motionparam;
ALARMACTION	gpioparam;

char 			byMotionStart = 0;
char 			byGpioStart = 0;
char				recordflag = 0;
char				alarmftpflag = 0x00;
char				alarmemailflag = 0x00;

#define FILE_CAPTURE    "/tmp/photo.jpg"

//init alarm action param;
void InitAlarmAction( void )
{
    memset( &motionparam, 0x00, sizeof( ALARMACTION ) );
    memset( &gpioparam, 0x00, sizeof( ALARMACTION ) );
}
//alarm write log
void AlarmLog( char type )
{
    if ( type == MOTION_ALARM )
    {
        //write a log
    }

    if ( type == GPIO_ALARM )
    {
    }
}
//alarm not all user
void AlarmNote( char type )
{
    char alarmtype = 0;

    if ( type == MOTION_ALARM )
    {
        alarmtype = LIVE_MOTION;
    }

    else if ( type == GPIO_ALARM )
    {
        alarmtype = LIVE_GPIO;
    }

    else
    {
        printf( "alarm note type is error=%d\n", type );
        return;
    }

    //send live user
    LIVE_ALARM_Send( alarmtype );
    //cgi status
    bparam.stStatusParam.warnstat = type;
    printf( "===alarm status %d\n", bparam.stStatusParam.warnstat );
}
//alarm note all user ok
void AlarmNoteClear( char type )
{
    char alarmtype = 0;
    //cgi status
    bparam.stStatusParam.warnstat = 0x00;

    //send live user
    if ( type == MOTION_ALARM )
    {
        alarmtype = LIVE_MOTION_CLR;
    }

    else if ( type == GPIO_ALARM )
    {
        alarmtype = LIVE_GPIO_CLR;
    }

    else
    {
        printf( "alarm note type is error=%d\n", type );
        return;
    }

    LIVE_ALARM_Send( alarmtype );
}

//alarm record
void AlarmRecord( char type )
{
    //set motion record
    if ( bparam.stAlarmParam.byAlarmRecord == 0x01 )
    {
    }
}
//alarm link to out
//void AlarmLink( char type )

//alarm ptz
//void AlarmPTZ( char type )


#if	0	//def SEND_EMAIL_WITH_SIX_IMAGE
static char nAlarmType = 0;
static BOOL bAlarmEmailSending = FALSE;
static pthread_t	thread_sendmail = NULL;
void* AlarmEmailProc( void* p )
{
    char 	filename[6][256];
    int		temptime;
    struct 	tm*	tmptm = NULL;
    int		frameno = 0;
    int		iRet;
    int		i;

    while ( 1 )
    {
        if ( !bAlarmEmailSending )
        {
            usleep( 1000 );
            continue;
        }

        for ( i = 0; i < 6; i++ )
        {
            //filename mac+name+alarm+date+no
            memset( filename[i], 0x00, 256 );
            temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
            tmptm = localtime( &temptime );


            /* BEGIN: Modified by wupm, 2013/4/22 */
            //sprintf( filename, "/tmp/%s_%s_1_%04d%02d%02d%02d%02d%02d_%d.jpg",

            /* BEGIN: Modified by wupm, 2013/6/20 */
#if	0
            sprintf( filename[i], "/tmp/%c%c_%c%c_%c%c_%c%c_%c%c_%c%c_%s_1_%04d%02d%02d%02d%02d%02d_%d.jpg",
                     //bparam.stIEBaseParam.szMac,
                     bparam.stIEBaseParam.szMac[0],
                     bparam.stIEBaseParam.szMac[1],
                     bparam.stIEBaseParam.szMac[3],
                     bparam.stIEBaseParam.szMac[4],
                     bparam.stIEBaseParam.szMac[6],
                     bparam.stIEBaseParam.szMac[7],
                     bparam.stIEBaseParam.szMac[9],
                     bparam.stIEBaseParam.szMac[10],
                     bparam.stIEBaseParam.szMac[12],
                     bparam.stIEBaseParam.szMac[13],
                     bparam.stIEBaseParam.szMac[15],
                     bparam.stIEBaseParam.szMac[16],
                     bparam.stIEBaseParam.dwDeviceID,
                     tmptm->tm_year + 1900,
                     tmptm->tm_mon + 1,
                     tmptm->tm_mday,
                     tmptm->tm_hour,
                     tmptm->tm_min,
                     tmptm->tm_sec,
                     frameno );
#else
            sprintf( filename[i], "/tmp/%04d%02d%02d%02d%02d%02d-%d.jpg",
                     tmptm->tm_year + 1900,
                     tmptm->tm_mon + 1,
                     tmptm->tm_mday,
                     tmptm->tm_hour,
                     tmptm->tm_min,
                     tmptm->tm_sec,
                     i + 1 );
#endif

            iRet = CaptureJpeg( filename[i] );
            Textout( "CaptureJpeg [%s] Return %d", filename[i], iRet );

            if ( iRet )
            {
                Textout( "CaptureJPEG ERROR" );
                continue;
            }

            frameno++;

            if ( frameno >= 10 )
            {
                frameno = 0x00;
            }

            sleep( 1 );
        }

        printf( "========send 6 email===============\n" );
        EmailSendSixImage( nAlarmType,
                           filename[0],
                           filename[1],
                           filename[2],
                           filename[3],
                           filename[4],
                           filename[5] );

        /* BEGIN: Modified by wupm, 2013/3/5 */
        if ( 1 )
        {
            char temp[256] = {0};

            for ( i = 0; i < 6; i++ )
            {
                if ( filename[i][0] != 0 )
                {
                    sprintf( temp, "rm -rf %s", filename[i] );
                }

                DoSystem( temp );
            }
        }

        bAlarmEmailSending = FALSE;
    }
}
void StartAlarmEmail( char type )
{
    if ( thread_sendmail == NULL )
    {
        pthread_create( &thread_sendmail, 0, &AlarmEmailProc, NULL );
    }

    nAlarmType = type;
    bAlarmEmailSending = TRUE;
}
#endif

/* BEGIN: Added by wupm, 2013/6/16 */
#if	0	//def	ENABLE_TAKE_PICTURE
void AlarmMakePicture( char type )
{
    time_t          curTime;
    int             temptime;
    struct tm*       tmptm = NULL;
    int		frameno = 0;
    int		iRet;

    if ( GetSDState() == 0 )
    {
        return;
    }

    if ( bparam.stAlarmParam.enable_take_pic >= 0x01 )
    {
        char filename[256];
        //filename mac+name+alarm+date+no
        memset( filename, 0x00, 256 );
        temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
        tmptm = localtime( &temptime );

        /* BEGIN: Modified by wupm, 2013/4/22 */
        //sprintf( filename, "/tmp/%s_%s_1_%04d%02d%02d%02d%02d%02d_%d.jpg",
        sprintf( filename, "/mnt/%c%c_%c%c_%c%c_%c%c_%c%c_%c%c_%s_1_%04d%02d%02d%02d%02d%02d_%d.jpg",
                 //bparam.stIEBaseParam.szMac,
                 bparam.stIEBaseParam.szMac[0],
                 bparam.stIEBaseParam.szMac[1],
                 bparam.stIEBaseParam.szMac[3],
                 bparam.stIEBaseParam.szMac[4],
                 bparam.stIEBaseParam.szMac[6],
                 bparam.stIEBaseParam.szMac[7],
                 bparam.stIEBaseParam.szMac[9],
                 bparam.stIEBaseParam.szMac[10],
                 bparam.stIEBaseParam.szMac[12],
                 bparam.stIEBaseParam.szMac[13],
                 bparam.stIEBaseParam.szMac[15],
                 bparam.stIEBaseParam.szMac[16],
                 bparam.stIEBaseParam.dwDeviceID,
                 tmptm->tm_year + 1900,
                 tmptm->tm_mon + 1,
                 tmptm->tm_mday,
                 tmptm->tm_hour,
                 tmptm->tm_min,
                 tmptm->tm_sec,
                 frameno );
        //printf("filename:%s\n",filename);
        iRet = CaptureJpeg( filename );
    }
}
#endif

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/8/3 */
#if	0
//alarm email
void AlarmEmail( char type )
{
    time_t          curTime;
    int             temptime;
    struct tm*       tmptm = NULL;
    int		frameno = 0;
    int		iRet;

#ifdef SEND_EMAIL_WITH_SIX_IMAGE
    Textout( "bparam.stIEBaseParam.szMac=[%s]", bparam.stIEBaseParam.szMac );

    if ( bparam.stAlarmParam.byAlarmEmail >= 0x01 )
    {
        if ( !bAlarmEmailSending )
        {
            StartAlarmEmail( type );
            return;
        }
    }

#endif

    if ( bparam.stAlarmParam.byAlarmEmail >= 0x01 )
    {
        char filename[256];
        //filename mac+name+alarm+date+no
        memset( filename, 0x00, 256 );
        temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
        tmptm = localtime( &temptime );

        /* BEGIN: Modified by wupm, 2013/4/22 */
        //sprintf( filename, "/tmp/%s_%s_1_%04d%02d%02d%02d%02d%02d_%d.jpg",
        sprintf( filename, "/tmp/%c%c_%c%c_%c%c_%c%c_%c%c_%c%c_%s_1_%04d%02d%02d%02d%02d%02d_%d.jpg",
                 //bparam.stIEBaseParam.szMac,
                 bparam.stIEBaseParam.szMac[0],
                 bparam.stIEBaseParam.szMac[1],
                 bparam.stIEBaseParam.szMac[3],
                 bparam.stIEBaseParam.szMac[4],
                 bparam.stIEBaseParam.szMac[6],
                 bparam.stIEBaseParam.szMac[7],
                 bparam.stIEBaseParam.szMac[9],
                 bparam.stIEBaseParam.szMac[10],
                 bparam.stIEBaseParam.szMac[12],
                 bparam.stIEBaseParam.szMac[13],
                 bparam.stIEBaseParam.szMac[15],
                 bparam.stIEBaseParam.szMac[16],
                 bparam.stIEBaseParam.dwDeviceID,
                 tmptm->tm_year + 1900,
                 tmptm->tm_mon + 1,
                 tmptm->tm_mday,
                 tmptm->tm_hour,
                 tmptm->tm_min,
                 tmptm->tm_sec,
                 frameno );
        //printf("filename:%s\n",filename);
        iRet = CaptureJpeg( filename );

        if ( iRet )
        {
            return;
        }

        frameno++;

        if ( frameno >= 10 )
        {
            frameno = 0x00;
        }

        printf( "========send a email===============\n" );
        EmailSend( type, filename );

        /* BEGIN: Modified by wupm, 2013/3/5 */
        if ( 1 )
        {
            char temp[128] = {0};
            sprintf( temp, "rm -rf %s", filename );
            DoSystem( temp );
        }
    }
}
//alarm ftp
void AlarmFtp( char type )
{
    if ( bparam.stAlarmParam.byUploadInter >= 0x01 )
    {
        alarmftpflag = type;
        printf( "==alarmftpflag===%d\n", alarmftpflag );
    }
}
//alarm clear
void AlarmClear( char type )
{
    //alarm link out clear
    //if (bparam.stAlarmParam.byLinkEnable == 0x01){
    if ( bparam.stAlarmParam.byAlarmOutLevel )
    {
        //HD_ALARMOUT_SetAlarm(0x01,0);
    }

    else
    {
        //HD_ALARMOUT_SetAlarm(0x01,1);
    }

    //}
    //alarm record stop
    if ( bparam.stAlarmParam.byAlarmRecord == 0x01 )
    {
    }

    //alarm note all user alarm is clear
    AlarmNoteClear( type );
}
#endif

/* BEGIN: Added by wupm, 2013/6/16 */
#ifdef	SET_ALARM_RECORD_TIME_ONE_MINUTE
#ifdef	ALARM_TIMER_OVER
#undef	ALARM_TIMER_OVER
#endif
#define	ALARM_TIMER_OVER	60
#endif
/* END:   Added by wupm, 2013/6/16 */

#include "vbuf.h"
extern SDRECORD	timerrecord;

//alarm check over
void CheckAlarmTimer( void )
{
	#if	0
    if ( gpioparam.flag == 0x01 )
    {
        gpioparam.time++;

        //printf("gpio over timer %d\n",gpioparam.time);

		/* BEGIN: Deleted by wupm, 2013/11/1 */
        //if ( gpioparam.time >= ALARM_TIMER_OVER )
        if ( gpioparam.time >= bell_data.record_size )
        {
            AlarmClear( ALARM_GPIO );
            gpioparam.time = 0x00;
            gpioparam.ftptime = 0x00;
            gpioparam.mailcnt = 0x00;
            gpioparam.flag = 0x00;
            Textout( "30Sencods PAST========gpio is clr============%d\n", alarmftpflag );
            alarmftpflag   = 0x00;
            bparam.stStatusParam.warnstat = 0x00;

            /* BEGIN: Added by Baggio.wu, 2013/7/30 */
            Textout( "Past 30 Seconds, Stop GPIO Record" );
            SetAlarmRecordClr();
        }
    }
	#endif

    //if ( motionparam.flag == 0x01 )
    if ( motionparam.flag == 0x01 && timerrecord.motion == 0x01 )
    {
        motionparam.time++;

		/* BEGIN: Deleted by wupm, 2013/11/1 */
        //if ( motionparam.time >= ALARM_TIMER_OVER )

		/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
		#if	0
        if ( motionparam.time >= bell_data.record_size )
        {
            SetMotionDect( 0x00 );
            AlarmClear( ALARM_MOTION );
            motionparam.time = 0x00;
            motionparam.ftptime = 0x00;
            motionparam.mailcnt = 0x00;
            motionparam.flag = 0x00;
            alarmftpflag   = 0x00;
            Textout( "=======motion is clr=========%d\n", alarmftpflag );
            bparam.stStatusParam.warnstat = 0x00;
            /* BEGIN: Added by wupm, 2013/1/25   Version:158 */
            Textout( "Past 30 Seconds, Stop Motion Record" );
            SetMotionRecordClr();
        }
		#endif
    }
}

void AlarmDetect( void )
{
	time_t      curTime;
	curTime = time( NULL );

	/* BEGIN: Modified by wupm(2073111@qq.com), 2014/7/18 */
    bparam.stDTimeParam.dwCurTime = curTime;// - bparam.stDTimeParam.byTzSel;
    //bparam.stDTimeParam.dwCurTime = curTime - bparam.stDTimeParam.byTzSel;

    return 0;
}

void* GpioCheckProc( void* p ) 	//gpio check
{
    int iRet;
    char flag;
    char param[4];
    printf( "start gpio check\n" );

    while ( 1 )
    {

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE

        /* BEGIN: Added by wupm, 2013/2/23 */
        if ( !RecordFileListInited() )
        {
            sleep( 1 );
            continue;
        }

#endif

        usleep( 500000 );

        if ( byGpioStart != 0x01 )
        {
            usleep( 500000 );
            continue;
        }

        flag = gpioparam.flag;

        //printf("gpio check %d cmp value:%d\n",alarminhappen,bparam.stAlarmParam.byAlarmInLevel);
        //Textout("alarminhappen = %d, Define = %d, flag = %d(0=Ready, 1=Already)", alarminhappen, bparam.stAlarmParam.byAlarmInLevel, flag);

        /* BEGIN: Added by wupm, 2013/5/16 */
#ifdef	SET_DEFAULT_ALARM_IO_LOW

        if ( bparam.stAlarmParam.byAlarmInLevel == 0 )
        {
            usleep( 500000 );
            continue;
        }

#endif

		/* BEGIN: Deleted by wupm, 2013/12/31 */
		#if	0
        if ( ( bparam.stAlarmParam.byAlarmInLevel == alarminhappen ) && ( flag == 0x00 ) )
        {
            //Note Alarm
            Textout( "============gpio is action============\n" );
            AlarmLog( GPIO_ALARM );
            AlarmNote( GPIO_ALARM );
            AlarmRecord( GPIO_ALARM );
            AlarmLink( GPIO_ALARM );
            AlarmPTZ( GPIO_ALARM );
            AlarmEmail( GPIO_ALARM );
            #ifdef TAKE_PHOTO_2_SDCARD
            TakePhoto2SDCard(GPIO_ALARM);
            #else
            AlarmFtp( GPIO_ALARM );
            #endif

#ifdef	ENABLE_TAKE_PICTURE
            AlarmMakePicture( GPIO_ALARM );
#endif
            WriteAlarmLog( GPIO_ALARM );

            /* BEGIN: Deleted by Baggio.wu, 2013/7/10 */
            //SendAlarmHttp( GPIO_ALARM );
            setHttpSendAlarmFlag( 1, GPIO_ALARM );

            sendP2PAlarm( GPIO_ALARM );
            /* BEGIN: Deleted by Baggio.wu, 2013/7/10 */
            //DnsSendAlarm( GPIO_ALARM );
            setDnsSendAlarmFlag( 1, GPIO_ALARM );
#ifdef	P2P_SCC_LIB
            sccSendAlarm( GPIO_ALARM );
#endif

            /* BEGIN: Added by wupm, 2013/6/17 */
#ifdef	PLAY_AUDIO_FILE_ON_ALARM
            #ifdef DEFAULT_PLAY_AUDIO_FILE
            bparam.stAlarmParam.reserved |= 0x01;
            #endif
            if ( (bparam.stAlarmParam.reserved & 0x01) == 0x01 )
            {
                StartPlayAlarmAudio();
            }
#endif

#if 0
            param[0] = 0x01;
            param[1] = 0x02;
            param[2] = 0x03;
            param[3] = 0x04;
            FtpWrite( param, 4 );
#endif

            /* BEGIN: Added by Baggio.wu, 2013/8/16 */
            SetAlarmRecordStart();
            #ifdef CUSTOM_HTTP_ALARM
            SendCustomHttpAlarm23(MOTION_ALARM);
            #endif

            gpioparam.ftptime = bparam.stAlarmParam.byUploadInter;
            gpioparam.mailcnt = bparam.stAlarmParam.byAlarmEmail;
            gpioparam.time = 0x00;
            gpioparam.flag = 0x01;
            Textout( "gpio action is end\n" );
        }
		#endif
		/* END: Deleted by wupm, 2013/12/31 */
    }

    return NULL;
}

/* BEGIN: Added by wupm, 2013/3/2 */
static int bMotoMoving = 1;
static int nStopCount=80;
int GetMotoMoving()
{
    return bMotoMoving;
}

static int nMotionInterval=2;
void SetMotionInterval(int value)
{
    //nMotionInterval=value;
    nMotionInterval=2;
}

int GetMotionInterval()
{
    return nMotionInterval;
}

void AlarmMotionInit(int sensitivity)
{
    int motionvalue=0;
    int motionarea=0x0001 | 0x0002 | 0x0004 ////1, 2, 3
                    |0x0008 | 0x0010 | 0x0020  //4, 5, 6
                     | 0x0040 | 0x0080 | 0x0100;   //7, 8, 9

    switch(sensitivity)
	{
		case 1:
			sensitivity = 10;
			break;
		case 2:
			sensitivity = 8;
			break;
		case 3:
			sensitivity = 6;
			break;
		case 4:
			sensitivity = 4;
			break;
		case 5:
		default:
			sensitivity = 2;
			break;

	}                     
    
    motionvalue = MOTION_THRESHOLD_BASE<<(nMotionInterval - 1);
    Textout("motion interval=%d, threshold=0x%08x, sensitivity=%d", nMotionInterval, motionvalue, sensitivity);
    motionvalue *= sensitivity;

    H264SetMotionEnable(MOTION_ENABLE);
    ObjMotionInit( motionvalue, motionarea );
}


static sem_t sem_start_motion_check;
void NoteStartMotionCheck( void )
{
    Textout("post sem_start_motion_check");
    sem_post( &sem_start_motion_check );
}

void* MotionCheckProc( void* p ) 	//Motion check
{
    int 	iRet = 0;
    int mMotionArea = 0;
    if ( ( sem_init( &sem_start_motion_check, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        sem_wait( &sem_start_motion_check );
        Textout("====>Start motion check");
        break;
    }

    AlarmMotionInit(bparam.stBell.alarm_level);

    while ( 1 )
    {

		if(bparam.stBell.alarm_type == AT_MOTION)
		{
			iRet = MotionDetect(&mMotionArea);
			if ( iRet == 0x01 )
			{
				
				OnMotionDetected();
			}
		}
		else if( bparam.stBell.alarm_type == AT_PIR )
		{
			if(bPIRStatus == 1)
			{
				OnPIRDetected();	
			}
		}
		else if( bparam.stBell.alarm_type == AT_MOTION_PIR )
		{
			iRet = MotionDetect(&mMotionArea);
			if ( iRet == 0x01 && bPIRStatus == 1 )
			{
				OnPirAndMotionDetected();
			}
		}
		
        
	
        usleep( 500*1000 );
    }

    return NULL;
}

/* BEGIN: Added by wupm, 2013/11/1 */
void OnDoorRecord()
{
	SetMotionRecordStart();

    //motionparam.ftptime = bparam.stAlarmParam.byUploadInter;
    //motionparam.mailcnt = bparam.stAlarmParam.byAlarmEmail;
    motionparam.time = 0x00;
    motionparam.flag = 0x01;
}

void* AlarmProc( void* p ) 	//Alarm process
{
    int iRet;
    printf( "start alarm proc\n" );
    sleep( 5 );

    while ( 1 )
    {
        sleep( 1 );

#if	0	//def	GET_RECORDLIST_PAGE_BY_PAGE

        /* BEGIN: Added by wupm, 2013/2/23 */
        if ( !RecordFileListInited() )
        {
            sleep( 1 );
            continue;
        }

#endif
        AlarmDetect();
    }

    return NULL;
}

void* AlarmMailProc( void* p )
{
    while ( 1 )
    {
        sleep( 1 );
    }
}

void ftp_dbg( char* pbuf, int cmd )
{
#if 0
    FILE* fp = NULL;

    if ( cmd == 0x00 )
    {
        fp = fopen( "/tmp/ftp_dbg.txt", "ab+" );

        if ( fp == NULL )
        {
            printf( "dbg file open failed\n" );
            return;
        }
    }

    else
    {
        fp = fopen( "/tmp/ftp_dbg.txt", "wb" );

        if ( fp == NULL )
        {
            printf( "dbg file open failed\n" );
            return;
        }
    }

    fwrite( pbuf, 1, strlen( pbuf ), fp );
    fclose( fp );
#endif
}

void ftp_dbg1( char* pbuf, int cmd )
{
#if 1
    FILE* fp = NULL;

    if ( cmd == 0x00 )
    {
        fp = fopen( "/tmp/ftp_dbg1.txt", "ab+" );

        if ( fp == NULL )
        {
            printf( "dbg file open failed\n" );
            return;
        }
    }

    else
    {
        fp = fopen( "/tmp/ftp_dbg1.txt", "wb" );

        if ( fp == NULL )
        {
            printf( "dbg file open failed\n" );
            return;
        }
    }

    fwrite( pbuf, 1, strlen( pbuf ), fp );
    fclose( fp );
#endif
}

/* BEGIN: Added by Baggio.wu, 2013/7/12 */
unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0){
        return filesize;
    }else{
       filesize = statbuff.st_size;
   }
    return filesize;
}

/* BEGIN: Added by yiqing, 2015/4/10 */
static int bFtpCapture = -1;
static struct timeval stFtpTime;
void AlarmFtp( char type, struct timeval* alarmTime, int bCapture )
{
	#ifndef BAFANGDIANZI 
    if ( bparam.stAlarmParam.byUploadInter >= 0x01 )
	#endif
    {
        memcpy(&stFtpTime, alarmTime, sizeof(struct timeval));
        bFtpCapture = bCapture;
        alarmftpflag = type;
    }
}

void* AlarmFtpProc( void* p )
{
    char 		flag;
    int             	temptime;
    struct tm*       tmptm = NULL;
    char            	frameno = 0;
    int             	iRet;
    short		delay;
    unsigned int	testcnt = 0;

    while ( 1 )
    {
		#if 0
        if ( !RecordFileListInited() )
        {
            sleep( 1 );
            continue;
        }
		#endif
		
        if ( alarmftpflag == 0x00 )
        {
            sleep( 1 );
            continue;
        }
		alarmftpflag = 0;

		Textout("AlarmFtpProc");

		#ifndef BAFANGDIANZI
        if ( bparam.stAlarmParam.byUploadInter >= 0x00 )
		#endif
		{
            char filename[256];
            char mac[20];
            char capfilename[32];

            //filename mac+name+alarm+date+no
            memset( filename, 0x00, 256 );
            temptime = stFtpTime.tv_sec - bparam.stDTimeParam.byTzSel;

            tmptm = localtime( &temptime );

            memset( mac, 0x00, 20 );
            memcpy( mac, bparam.stIEBaseParam.szMac, 17 );
            mac[2] = 0x5f;	//"_"
            mac[5] = 0x5f;
            mac[8] = 0x5f;
            mac[11] = 0x5f;
            mac[14] = 0x5f;

            sprintf( filename, "/tmp/%s_%s_%d_%04d%02d%02d%02d%02d%02d_%d.jpg",
                     mac, bparam.stIEBaseParam.dwDeviceID, alarmftpflag,
                     tmptm->tm_year + 1900, tmptm->tm_mon + 1, tmptm->tm_mday,
                     tmptm->tm_hour, tmptm->tm_min, tmptm->tm_sec, frameno );

            memset( capfilename, 0x00, 32 );
            sprintf( capfilename, "/tmp/%d.jpg", alarmftpflag );

			Textout("******************************************");
            iRet = access( capfilename, F_OK );

            if ( iRet == 0 )
            {
                char cmd[128];
                memset( cmd, 0, 128 );
                sprintf( cmd, "rm -f %s", capfilename );
                DoSystem( cmd );
                DoSystem( "sync" );
                Textout( "File is exist, to remove it, capfilename=%s", capfilename );
            }
			

            if(bFtpCapture != -1)
            {
                char cmd[256];
				
                sprintf(cmd, "cp %s %s", FILE_CAPTURE, capfilename);
				
                DoSystem(cmd);
				
                bFtpCapture = -1;
				Textout("cp %s %s",FILE_CAPTURE,capfilename);
				
            }
            else
            {
                iRet = CaptureJpeg( capfilename );
				
				Textout("iRet:%d",iRet);
                if ( iRet )
                {
                    sleep( 1 );
                    continue;
                }
				
            }
			
            iRet = FtpsendAlarm( capfilename, filename, "/tmp" );

            /* BEGIN: Deleted by wupm, 2013/4/25 */
            /*
            Textout3("7");
            memset( temp, 0x00, 64 );
            sprintf( temp, "rm -rf %s", capfilename );
            DoSystem( temp );
            Textout3( "Delete Temp File\n", 0 );
            */
            frameno++;

            if ( frameno >= 10 )
            {
                frameno = 0x00;
            }

            delay = bparam.stAlarmParam.byUploadInter;

            if ( delay > 0 )
            {
                sleep( delay );
            }

        }
		#ifndef BAFANGDIANZI	
        else
        {
            sleep( 1 );
        }
		#endif
    }
}
int RecordFileListInited();
void* FtpTimerProc( void* p )
{
    char 		flag;
    int             	temptime;
    struct tm*       tmptm = NULL;
    char            	frameno = 0;
    int             	iRet;
    short		delay;
    char		temp[64];
    unsigned int	testcnt = 0;

    while ( 1 )
    {
		#ifdef	GET_RECORDLIST_PAGE_BY_PAGE
        /* BEGIN: Added by wupm, 2013/2/23 */
        if ( !RecordFileListInited() )
        {
            sleep( 1 );
            continue;
        }
		#endif
        if ( bparam.stFtpParam.nInterTime >= 0x01 )
        {
            char filename[256];
            // char temp[256];
            char mac[20];
            char capfilename[32];

            //filename mac+name+alarm+date+no
            memset( filename, 0x00, 256 );
            temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
            tmptm = localtime( &temptime );

            memset( mac, 0x00, 20 );
            memcpy( mac, bparam.stIEBaseParam.szMac, 17 );
            mac[2] = 0x5f;	//"_"
            mac[5] = 0x5f;
            mac[8] = 0x5f;
            mac[11] = 0x5f;
            mac[14] = 0x5f;
#if 0
            sprintf( filename, "/tmp/%s_(%s)_%d_%04d%02d%02d%02d%02d%02d_%d.jpg",
                     mac, bparam.stIEBaseParam.dwDeviceID, alarmftpflag,
                     tmptm->tm_year + 1900, tmptm->tm_mon + 1, tmptm->tm_mday,
                     tmptm->tm_hour, tmptm->tm_min, tmptm->tm_sec, frameno );
#else
            sprintf( filename, "/tmp/%s_%s_0_%04d%02d%02d%02d%02d%02d_%d.jpg",
                     mac, bparam.stIEBaseParam.dwDeviceID,
                     tmptm->tm_year + 1900, tmptm->tm_mon + 1, tmptm->tm_mday,
                     tmptm->tm_hour, tmptm->tm_min, tmptm->tm_sec, frameno );
#endif
            memset( capfilename, 0x00, 32 );
            sprintf( capfilename, "/tmp/12.jpg" );

            /* BEGIN: Added by Baggio.wu, 2013/7/19 */
            iRet = access( capfilename, F_OK );

            if ( iRet == 0 )
            {
                char cmd[128];
                memset( cmd, 0, 128 );
                sprintf( cmd, "rm -f %s", capfilename );
                DoSystem( cmd );
                DoSystem( "sync" );
                Textout( "File is exist, to remove it, capfilename=%s", capfilename );
            }

            /* END:   Added by Baggio.wu, 2013/7/19 */

            iRet = CaptureJpeg( capfilename );

            Textout( "Capture JPEG OK" );
            //sleep(60);

            if ( iRet )
            {
                sleep( 1 );
                continue;
            }

            iRet = FtpsendAlarm( capfilename, filename , "/tmp" );

            Textout( "FTP UPLOAD OK" );
            //sleep(60);

            /* BEGIN: Deleted by wupm, 2013/4/25 */
            //DoSystem( "rm -rf /tmp/12.jpg" ) ; //add by iven
            frameno++;

            if ( frameno >= 10 )
            {
                frameno = 0x00;
            }

            delay = bparam.stFtpParam.nInterTime;

            if ( delay > 0 )
            {
                sleep( delay );
            }

        }

        else
        {
            sleep( 1 );
        }
    }
}

/* END:   Added by yiqing, 2015/4/10 */
void AlarmThread( void )
{
    pthread_t               gpiothread;
    pthread_t               motionthread;
    pthread_t               alarmthread;
    pthread_t		alarmftpthread;
    pthread_t		alarmmailthread;
    pthread_t		ftptimerthread;

    /* BEGIN: Added by wupm, 2013/3/2 */
	/*
    if ( 1 )
    {
        pthread_t		thread2;
        pthread_create( &thread2, 0, &CheckMotoProc, NULL );
    }
    */

    /* BEGIN: Added by Baggio.wu, 2013/7/10 */
	/*
    if ( 1 )
    {
        pthread_t		sendalarmhttpthread;
        pthread_create( &sendalarmhttpthread, 0, &SendAlarmHttpProc, NULL );
    }
	*/

/* BEGIN: Deleted by wupm, 2013/11/1 */
//    pthread_create( &gpiothread, 0, &GpioCheckProc, NULL );

/* BEGIN: Modified by wupm(2073111@qq.com), 2014/7/10 */
    pthread_create( &motionthread, 0, &MotionCheckProc, NULL );
    pthread_create( &alarmthread, 0, &AlarmProc, NULL );

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/7/11 */
	pthread_detach(motionthread);
	pthread_detach(alarmthread);

    pthread_create( &alarmftpthread, 0, &AlarmFtpProc, NULL );
//    pthread_create( &alarmmailthread, 0, &AlarmMailProc, NULL );
//    pthread_create( &ftptimerthread, 0, &FtpTimerProc, NULL );
    /* BEGIN: Added by wupm, 2013/3/12 */

#if	0	//def	HONESTECH

    if ( 1 )
    {
        pthread_t	ht_thread;
        pthread_t	ht_thread2;
        pthread_create( &ht_thread, 0, &HT_MotionCheckProc, NULL );
        pthread_create( &ht_thread, 0, &HT_DDNSConnectProc, NULL );
    }

#endif
}


