#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "cmdhead.h"
#include "param.h"
#include "alarm.h"
#include "debug.h"

pthread_mutex_t 	logmutex = PTHREAD_MUTEX_INITIALIZER;
unsigned char*		plogbuf;
#define LOG_SIZE 	128
#define LOG_CNT		8


void loglock( void )
{
    pthread_mutex_lock( &logmutex );
}

void logunlock( void )
{
    pthread_mutex_unlock( &logmutex );
}

void LogInit( void )
{
    plogbuf  = ( unsigned char* )malloc( 1024 * 128 );

    if ( plogbuf == NULL )
    {
        printf( "============logbuf is failed============\n" );
    }

	/* BEGIN: Modified by wupm, 2013/4/3 */
    //memset( plogbuf, 0x00, 128 );
    memset( plogbuf, 0x00, 1024 * 128 );
}

int ReadLogIndex( void )
{
    FILE*	 fp = NULL;
    int	index = 0;
    fp = fopen( "/param/systemindex.txt", "rb" );

    if ( fp == NULL )
    {
        return -1;
    }

    fread( &index, 1, sizeof( int ), fp );
    fclose( fp );

	/* BEGIN: Modified by wupm, 2013/4/3 */
    //if ( index > 1024 )
    if ( index >= 1024 )
    {
        index = 0;
    }

    //printf("read log index=%d\n",index);
    return index;
}

void WriteLogIndex( int index )
{
    FILE*    fp = NULL;

	/* BEGIN: Modified by wupm, 2013/4/3 */
    //if ( index > 1024 )
    if ( index >= 1024 )
    {
        DoSystem( "rm /param/systemlog.txt" );
        index = 0;
    }

    fp = fopen( "/param/systemindex.txt", "wb" );

    if ( fp == NULL )
    {
        return;
    }

    fwrite( &index, 1, sizeof( int ), fp );
    fclose( fp );
}

/* BEGIN: Modified by wupm, 2013/7/1 */
//void myWriteLog( char* psrc, char len )
void myWriteLog( char* psrc, int len )
{
    FILE*	 fp = NULL;
    int 	index = 0;
    char	temp[128];
    loglock();
    index = ReadLogIndex();

    if ( index == -1 )
    {
        WriteLogIndex( 0 );
        logunlock();
        return;
    }

    //printf("write index=%d\n",index);
    index++;
    fp = fopen( "/param/systemlog.txt", "ab+" );

    if ( fp == NULL )
    {
        logunlock();
        return;
    }

    if ( len > LOG_SIZE )
    {
        len = LOG_SIZE;
    }

    memset( temp, 0x00, LOG_SIZE );
    memcpy( temp, psrc, len );
    fwrite( temp, 1, LOG_SIZE, fp );
    fclose( fp );
    WriteLogIndex( index );
    logunlock();
}

void myWriteLogIp( int ip, char* psrc )
{
    char temp[128];
    struct tm* tm1;
    time_t     		curtime;

	memset( temp, 0x00, 128 );
    curtime = time( NULL );
    curtime = curtime - bparam.stDTimeParam.byTzSel;
    tm1 = localtime( &curtime );

/* BEGIN: Modified by wupm, 2013/6/14 */
#ifdef	MORE_LOG
	sprintf( temp, "%4d-%02d-%02d %02d:%02d:%02d %s",
             tm1->tm_year + 1900, tm1->tm_mon + 1, tm1->tm_mday,
             tm1->tm_hour, tm1->tm_min, tm1->tm_sec,
             psrc );
#else
    sprintf( temp, "%4d-%02d-%02d %02d:%02d:%02d  %d.%d.%d.%d %s",
             tm1->tm_year + 1900, tm1->tm_mon + 1, tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec,
             ( ip >> 24 ) & 0xff, ( ip >> 16 ) & 0xff, ( ip >> 8 ) & 0xff, ( ip ) & 0xff, psrc );
#endif
	//Textout("LOG = [%s]", temp);
    myWriteLog( temp, strlen( temp ) );
}


unsigned short myReadLog( unsigned char* pszBuffer )
{
    FILE*            fp     = NULL;
    unsigned char*   pdst   = NULL;
    unsigned short  logcnt  = 0;
    loglock();
    fp = fopen( "/param/systemlog.txt", "rb" );

    if ( fp == NULL )
    {
        printf( "read system log failed\n" );
        logunlock();
        return 0;
    }

    pdst = pszBuffer;

    while ( !feof( fp ) )
    {
        fread( pdst, 1, LOG_SIZE, fp );
        pdst += LOG_SIZE;
        logcnt++;

        if ( logcnt >= 1024 )
        {
            break;
        }
    }

    fclose( fp );
    logunlock();
    return logcnt;
}

unsigned int ReadAlarmLog( unsigned char* plog )
{
    FILE*   	 fp = NULL;
    struct stat     stStat;
    unsigned int    len;
    ALARMLOG        alarmlog;
    unsigned int	offset = 0;
    unsigned char*	pdst = plog;
    int             temptime;
    struct tm*       tmptm = NULL;

    /* BEGIN: Deleted by wupm, 2013/3/19 */
//	int nCount = 0;

    if ( stat( "/param/alarmlog.bin", &stStat ) >= 0 )
    {
        //read alarmlog 1
        len = stStat.st_size;

        if ( ( len & 0x03 ) == 0x00 )
        {
            fp = fopen( "/param/alarmlog.bin", "rb" );

            if ( fp )
            {
//				nCount = 0;
                while ( len > 0 )
                {
                    if ( offset >= 1024 * 16 )
                    {
                        break;
                    }

                    len -= sizeof( ALARMLOG );
                    fseek( fp, len, SEEK_SET );
                    fread( &alarmlog, 1, sizeof( ALARMLOG ), fp );

                    //printf("alarmlog.type %d\n",alarmlog.type);
                    if ( alarmlog.type == MOTION_ALARM )
                    {
                        offset += sprintf( pdst + offset, "log_text+=\"motion alarm %04d-%02d-%02d %02d:%02d:%02d\\n\";\r\n",
                                           alarmlog.year, alarmlog.mon, alarmlog.day, alarmlog.hour, alarmlog.min, alarmlog.sec );
                    }

                    else
                    {
                        offset += sprintf( pdst + offset, "log_text+=\"gpio alarm %04d-%02d-%02d %02d:%02d:%02d\\n\";\r\n",
                                           alarmlog.year, alarmlog.mon, alarmlog.day, alarmlog.hour, alarmlog.min, alarmlog.sec );
                    }

                    /*
                    nCount ++;
                    if ( nCount >= 128 )
                    {
                    	break;
                    }
                    */
                }

                fclose( fp );
            }
        }
    }

    if ( stat( "/param/alarmlog1.bin", &stStat ) >= 0 )
    {
        //read alarmlog 2
        len = stStat.st_size;

        if ( ( len & 0x03 ) == 0x00 )
        {
            fp = fopen( "/param/alarmlog1.bin", "rb" );

            if ( fp )
            {
                while ( len > 0 )
                {
                    if ( offset >= 1024 * 16 )
                    {
                        break;
                    }

                    len -= sizeof( ALARMLOG );
                    fseek( fp, len, SEEK_SET );
                    fread( &alarmlog, 1, sizeof( ALARMLOG ), fp );

                    if ( alarmlog.type == MOTION_ALARM )
                    {
                        offset += sprintf( pdst + offset, "log_text+=\"motion alarm %04d-%02d-%02d %02d:%02d:%02d\\n\";\r\n",
                                           alarmlog.year, alarmlog.mon, alarmlog.day, alarmlog.hour, alarmlog.min, alarmlog.sec );
                    }

                    else
                    {
                        offset += sprintf( pdst + offset, "log_text+=\"gpio alarm %04d-%02d-%02d %02d:%02d:%02d\\n\";\r\n",
                                           alarmlog.year, alarmlog.mon, alarmlog.day, alarmlog.hour, alarmlog.min, alarmlog.sec );
                    }

                    /*
                    nCount ++;
                    if ( nCount >= 128 )
                    {
                    	break;
                    }
                    */
                }

                fclose( fp );
            }
        }
    }

    Textout( "read alarm log, len = %d\n", offset );
    return offset;
}

void WriteAlarmLog( int alarmtype )
{
    FILE*   	 fp = NULL;
    struct stat     stStat;
    unsigned int	len;
    ALARMLOG	alarmlog;
    int             temptime;
    struct tm*       tmptm = NULL;
    temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
    tmptm = localtime( &temptime );

    if ( stat( "/param/alarmlog.bin", &stStat ) >= 0 )
    {
        len = stStat.st_size;

        if ( len >= 1024 * 8 )
        {
            DoSystem( "cp -f /param/alarmlog.bin /param/alarmlog1.bin" );
            DoSystem( "rm -f /param/alarmlog.bin" );
        }
    }

    fp = fopen( "/param/alarmlog.bin", "ab+" );

    if ( fp == NULL )
    {
        return;
    }

    memset( &alarmlog, 0x00, sizeof( ALARMLOG ) );
    alarmlog.type = alarmtype;
    alarmlog.year =  tmptm->tm_year + 1900;
    alarmlog.mon  =  tmptm->tm_mon + 1;
    alarmlog.day  =  tmptm->tm_mday;
    alarmlog.hour =  tmptm->tm_hour;
    alarmlog.min  =  tmptm->tm_min;
    alarmlog.sec  = tmptm->tm_sec;
    fwrite( &alarmlog, 1, sizeof( ALARMLOG ), fp );
    //printf("alarmtype %d\n",alarmtype);
    fclose( fp );
}
