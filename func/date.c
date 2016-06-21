#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <pthread.h>
#include "cmdhead.h"
#include "param.h"
#include <semaphore.h>

#include "debug.h"

pthread_mutex_t         datemutex = PTHREAD_MUTEX_INITIALIZER;
static char		ntpflag  = 1;
static unsigned short  	ntptimes = 0;


void datelock( void )
{
    pthread_mutex_lock( &datemutex );
}

void dateunlock( void )
{
    pthread_mutex_unlock( &datemutex );
}

int ReadDate( void )
{
    FILE*    fp = NULL;
    int     curtime = 0;
    fp = fopen( "/param/date.bin", "rb" );

    if ( fp )
    {
        fread( &curtime, 1, sizeof( int ), fp );
        fclose( fp );
    }

    return curtime;
}

void WriteDate( int curtime )
{
    FILE*    fp = NULL;
    fp = fopen( "/param/date.bin", "wb" );

    if ( fp )
    {
        fwrite( &curtime, 1, sizeof( int ), fp );
        fclose( fp );
        printf( "write date ok\n" );
    }
}

void SetCurrentTime( unsigned int dwCurTime, int byzone )
{
    struct timeval		tv;
    struct timezone		tz;
    struct tm*		tm;
    tv.tv_sec		= dwCurTime; // + 28800;//28800 = 8 * 60 * 60 (beijing's time(8 timezone))
    tv.tv_usec		= 0;
    tz.tz_minuteswest	= 0;
    tz.tz_dsttime		= 0;
    settimeofday( &tv, NULL );
}

int GetCurrentTime()
{
    struct timeval		tv;
    struct timezone		tz;
    struct tm		mTime;
    tv.tv_sec		= mktime( &mTime );
    tv.tv_usec		= 0;
    tz.tz_minuteswest	= 0;
    tz.tz_dsttime		= 0;
    settimeofday( &tv, &tz );
    return 0;
}

int GetNtpTime( void )
{
    int iRet = -1;

	//Textout("bparam.stDTimeParam.byIsNTPServer = %d", bparam.stDTimeParam.byIsNTPServer);
	//Textout("bparam.stDTimeParam.szNtpSvr = %s", bparam.stDTimeParam.szNtpSvr);
    if ( bparam.stDTimeParam.byIsNTPServer )
    {
        iRet = NtpRun( bparam.stDTimeParam.szNtpSvr );
    }
    
    return iRet;
}

void SetNtpRestart( char flag )
{
    ntpflag = flag;
    ntptimes = 0x00;
}

//ntp thread

/*30 minutes Sync time*/
#define	NTP_REFRESH_MINUTES	(30*60)  

void* NtpThreadProc( void* p )
{
    int	iRet = 0x00;

    while ( 1 )
    {
        if(netok == -1)
        {
            sleep(1);
            continue;
        }
        
        if ( ntpflag == 1 )
        {
            iRet = GetNtpTime();
            if ( iRet == 0x00 )
            {
                ntpflag = 0;
            }
            else
            {
                sleep(10);
            }
        }
        else
        {
            sleep( 1 );
        
            ntptimes++;
            if ( ntptimes >= NTP_REFRESH_MINUTES )
            {
                ntptimes = 0x00;
                ntpflag = 1;
            }
        }
    }
}

void ConfigDate( void ) 	//read last time
{
    int curtime = 0;
    curtime = ReadDate();
    SetCurrentTime( curtime + 40, 0 );
    WriteDate( curtime + 40 );
}

/* BEGIN: Added by wupm, 2013/3/2 */
sem_t		sem_SetTime;
void* SetTimeThreadProc( void* p )
{
    if ( ( sem_init( &sem_SetTime, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        sem_wait( &sem_SetTime );
        SetCurrentTime( bparam.stDTimeParam.dwCurTime, 0 );
    }
}

void PcDateSync( void ) 	//pc sync time
{
    if ( bparam.stDTimeParam.byIsPCSync )
    {
        Textout( "Time zone is %d", bparam.stDTimeParam.byTzSel );
        sem_post( &sem_SetTime );
    }
}

void DateInit( void ) 	//date init
{
    ConfigDate();
}

void DateThread( void ) 	//date check thread is start
{
    pthread_t ntpthread2;
    pthread_t ntpthread;

    pthread_create( &ntpthread, 0, &NtpThreadProc, NULL );
	pthread_detach(ntpthread);
    pthread_create( &ntpthread2, 0, &SetTimeThreadProc, NULL );
	pthread_detach(ntpthread2);
}

unsigned char CheckData( unsigned char* pbuf, unsigned char len )
{
    unsigned char 	i;
    unsigned char	flag = 0;

    for ( i = 0; i < len; i++ )
    {
        flag = 0x00;

        if ( ( pbuf[i] >= 0x2e ) && ( pbuf[i] <= 0x39 ) )
        {
            flag = 0x01;
        }

        if ( ( pbuf[i] >= 0x61 ) && ( pbuf[i] <= 0x66 ) )
        {
            flag = 0x01;
        }

        if ( flag == 0x00 )
        {
            break;
        }
    }

    return i;
}


