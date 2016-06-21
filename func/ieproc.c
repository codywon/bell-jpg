#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <time.h>
#include <memory.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>


#include "init.h"
#include "alarm.h"
#include "param.h"
#include "protocol.h"
#include "video.h"
#include "network.h"


#define WIFI_MAX_RESULT 	10
#define LOG_SIZE 			8
#define LOG_CNT				8
#define TRUE     				1
#define FALSE    				0
pthread_mutex_t                 wdtmutex = PTHREAD_MUTEX_INITIALIZER;

void wdtlock( void )
{
    pthread_mutex_lock( &wdtmutex );
}

void wdtunlock( void )
{
    pthread_mutex_unlock( &wdtmutex );
}


void watchdog_init( void )
{
#if 0
    fd_wdt = open( "/dev/watchdog", O_WRONLY );

    if ( fd_wdt == -1 )
    {
        printf( "Watchdog device not enabled.\n" );
    }

#endif
}

void watchdog_handle( int state )
{
#if 0
    int options;

    if ( fd_wdt == -1 )
    {
        printf( "Watchdog device not enabled.\n" );
        return;
    }

#if 0

    if ( state == WATCHDOG_ON )
    {
        options = WDIOS_ENABLECARD;
        ioctl( fd_wdt, WDIOC_SETOPTIONS, &options );
    }

    else if ( state == WATCHDOG_OFF )
    {
        options = WDIOS_DISABLECARD;
        ioctl( fd_wdt, WDIOC_SETOPTIONS, &options );
    }

#endif
#endif
}
void
watchdog_restart( void )
{
#if 0
    int dummy;
    wdtlock();

    if ( fd_wdt == -1 )
    {
        wdtunlock();
        return;
    }

    ioctl( fd_wdt, WDIOC_KEEPALIVE, &dummy );
    wdtunlock();
#endif
}


void ReadDeviceID( char* deviceid )
{
#if 0
    FILE*    fp = NULL;
    fp = fopen( "/param/device.bin", "rb" );

    if ( fp )
    {
        fread( deviceid, 1, 7, fp );
        fclose( fp );
    }

    else
    {
        deviceid[0] = 0x00;
        deviceid[1] = 0x00;
        deviceid[2] = 0x00;
        deviceid[3] = 0x00;
        deviceid[4] = 0x00;
        deviceid[5] = 0x00;
        deviceid[6] = 0x00;
    }

#endif
}

void WriteDeviceID( char* deviceid )
{
#if 0
    FILE*    fp = NULL;
    fp = fopen( "/param/device.bin", "wb" );

    if ( fp )
    {
        fwrite( deviceid, 1, 7, fp );
        fclose( fp );
    }

    else
    {
        deviceid[0] = 0x00;
        deviceid[1] = 0x00;
        deviceid[2] = 0x00;
        deviceid[3] = 0x00;
        deviceid[4] = 0x00;
        deviceid[5] = 0x00;
        deviceid[6] = 0x00;
    }

#endif
}
int LIVE_ALARM_SEND( char type, char value )
{
#if 0
    char                    szBuffer[1024];
    int                     i, iBufferSize, j;
    PCOMMONRESPONSE         pstResponse;
    int                     iSocket = 0;
    unsigned short          alarmon;
    PLIVEHEAD               pLiveHead;
    struct timeval          tv;
    struct timezone         tz;
    char                    size = sizeof( LIVEHEAD );
    //printf("send video start\n");
    avlivelock();
    gettimeofday( &tv, &tz );
    printf( "alarm is  start\n" );

    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        if ( streamuser[i].socket < 1 )
        {
            continue;
        }

        iSocket = streamuser[i].socket;
        memset( szBuffer, 0, sizeof( szBuffer ) );
        pLiveHead = ( PLIVEHEAD )szBuffer;

        if ( value == 1 )
        {
            pLiveHead->type         = 0x05;		//motion detect
        }

        else
        {
            pLiveHead->type         = 0x06;		//gpio detect
        }

        pLiveHead->streamid     = bparam.stVencParam.bysize[0];
        pLiveHead->militime     = tv.tv_usec / 1000;
        pLiveHead->sectime      = tv.tv_sec;
        pLiveHead->len          = 0;
        pLiveHead->frameno      = 0;
        LIVE_SVR_Send( streamuser[i].socket, szBuffer, sizeof( LIVEHEAD ) );
    }

    avliveunlock();
#endif
}

