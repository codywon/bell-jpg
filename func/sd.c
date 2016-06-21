#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <dirent.h>
#include <scsi/sg.h>
#include <linux/hdreg.h>
#include <semaphore.h>
#include "param.h"
#include "vbuf.h"
#include "abuf.h"
#include "mjpgbuf.h"
#include "protocol.h"
#include "debug.h"

/* BEGIN: Added by wupm, 2013/11/1 */
//extern TTT_DOORBELL bell_data;
//extern T_AVSTATE	bell_avstate;

pthread_mutex_t 	sdfilemutex = PTHREAD_MUTEX_INITIALIZER;
sem_t				semsd;
sem_t				semsda;
sem_t				semsdf;
sem_t				recordstart;
char					sdokflag = 0;
int					backuplen = 0;
long long				sdtotal = 0;
long long 			sdfree  = 0;
char					delfilecnt = 0;

short				timecnt = 0x00;
short				alarmcnt = 0x00;
char					sdstart = 0x00;

unsigned int			sdbackuplen = 0x00;
unsigned int			sdbackuplena = 0x00;
struct tm*              	 recordtm = NULL;
SDRECORD			timerrecord;

/* BEGIN: Added by wupm, 2013/6/27 */
BOOL	bCurrentNewFile = FALSE;

/* BEGIN: Added by wupm, 2013/4/11 */
pthread_mutex_t nMutex_GetStorageSpace = PTHREAD_MUTEX_INITIALIZER;
void Lock_GetStorageSpace()
{
    pthread_mutex_lock( &nMutex_GetStorageSpace );
}
void UnLock_GetStorageSpace()
{
    pthread_mutex_unlock( &nMutex_GetStorageSpace );
}
/* END:   Added by wupm, 2013/4/11 */
/* BEGIN: Added by wupm, 2013/1/31 */
pthread_mutex_t nMutex_GetRecordFile = PTHREAD_MUTEX_INITIALIZER;
void Lock_GetRecordFile()
{
    pthread_mutex_lock( &nMutex_GetRecordFile );
}
void UnLock_GetRecordFile()
{
    pthread_mutex_unlock( &nMutex_GetRecordFile );
}
/* END:   Added by wupm, 2013/1/31 */
pthread_mutex_t nMutex_GetRecordList = PTHREAD_MUTEX_INITIALIZER;
void Lock_GetRecordList()
{
    pthread_mutex_lock( &nMutex_GetRecordList );
}
void UnLock_GetRecordList()
{
    pthread_mutex_unlock( &nMutex_GetRecordList );
}

pthread_mutex_t sdmutex = PTHREAD_MUTEX_INITIALIZER;
void sdlock( void )
{
    pthread_mutex_lock( &sdmutex );
}
void sdunlock( void )
{
    pthread_mutex_unlock( &sdmutex );
}


/* BEGIN: Added by wupm, 2013/2/19 */

void CheckRecordStopStatus( void );
void SetRecordStop( void );
//lock
void sdfilelock( void )
{
    pthread_mutex_lock( &sdfilemutex );
}
void sdfileunlock( void )
{
    pthread_mutex_unlock( &sdfilemutex );
}

/* BEGIN: Added by Baggio.wu, 2013/7/6 */
void SDCard_dbg( char* pbuf, int cmd )
{

    char temp[256];
    struct tm* tm1;
    time_t     		curtime;

    memset( temp, 0x00, 256 );
    curtime = time( NULL );
    curtime = curtime - bparam.stDTimeParam.byTzSel;
    tm1 = localtime( &curtime );

    sprintf( temp, "%4d-%02d-%02d %02d:%02d:%02d %s\n",
             tm1->tm_year + 1900, tm1->tm_mon + 1, tm1->tm_mday,
             tm1->tm_hour, tm1->tm_min, tm1->tm_sec,
             pbuf );

    FILE* fp = NULL;

    if ( cmd == 0x00 )
    {
        fp = fopen( "/tmp/SDCard_dbg.txt", "ab+" );

        if ( fp == NULL )
        {
            printf( "SDCard_dbg file open failed\n" );
            return;
        }
    }

    else
    {
        fp = fopen( "/tmp/SDCard_dbg.txt", "wb" );

        if ( fp == NULL )
        {
            printf( "SDCard_dbg file open failed\n" );
            return;
        }
    }

    fwrite( temp, 1, strlen( temp ), fp );
    fclose( fp );
}

void Reboot_dbg( char* file, int line, char* func, char* pbuf, int cmd )
{

    char temp[256];

    memset( temp, 0x00, 256 );

    sprintf( temp, "%s, line %4d, %s(), %s\n",
             file, line, func,
             pbuf );

    FILE* fp = NULL;

    if ( cmd == 0x00 )
    {
        fp = fopen( "/param/reboot_dbg.txt", "ab+" );

        if ( fp == NULL )
        {
            printf( "Reboot_dbg file open failed\n" );
            return;
        }
    }

    else
    {
        fp = fopen( "/param/reboot_dbg.txt", "wb" );

        if ( fp == NULL )
        {
            printf( "Reboot_dbg file open failed\n" );
            return;
        }
    }

    fwrite( temp, 1, strlen( temp ), fp );
    fclose( fp );

    sleep( 2 );
}
/* END:   Added by Baggio.wu, 2013/7/6 */
//timer record
void InitTimerRecord( void )
{
    memset( &timerrecord, 0x00, sizeof( SDRECORD ) );
}
//write sd stream
void NoteSDUser( void )
{
	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
    //sem_post( &semsd );
}
//note sd write audio
void NoteSDUserAudio( void )
{
    sem_post( &semsda );
}
//sd debug
void sd_dbg( char* pbuf, int cmd )
{
    FILE* fp = NULL;

    if ( cmd == 0x00 )
    {
        fp = fopen( "/tmp/sddbug.txt", "ab+" );

        if ( fp == NULL )
        {
            printf( "sd dbg file open failed\n" );
            return;
        }
    }

    else
    {
        fp = fopen( "/tmp/sddbug.txt", "wb" );

        if ( fp == NULL )
        {
            printf( "sd dbg file open failed\n" );
            return;
        }
    }

    fwrite( pbuf, 1, strlen( pbuf ), fp );
    fclose( fp );
}
//sd debug
void sd_dbg1( char* pbuf, int cmd )
{
    FILE* fp = NULL;

    if ( cmd == 0x00 )
    {
        fp = fopen( "/tmp/sddbug1.txt", "ab+" );

        if ( fp == NULL )
        {
            printf( "sd dbg file open failed\n" );
            return;
        }
    }

    else
    {
        fp = fopen( "/tmp/sddbug1.txt", "wb" );

        if ( fp == NULL )
        {
            printf( "sd dbg file open failed\n" );
            return;
        }
    }

    fwrite( pbuf, 1, strlen( pbuf ), fp );
    fclose( fp );
}
//calute sd storage info
int GetStorageSpace( void )
{
    struct statfs fs;
    long long blocks, bfree;

    if ( statfs( "/mnt", &fs ) == -1 )
    {
        printf( "statfs failed for\n" );
        sdtotal = 0;
		sdfree = 0;
        return -1;
    }

    blocks = fs.f_blocks;
    bfree = fs.f_bfree;
    sdtotal = blocks * fs.f_bsize / ( 1024 * 1024 );
    sdfree = bfree * fs.f_bsize / ( 1024 * 1024 );

	Textout("SD Size = %ld / %ld, blocks = %ld, free = %ld, f_bsize = %ld", sdfree, sdtotal, fs.f_blocks, fs.f_bfree, fs.f_bsize );

    //fix usb disk error =  2m
    if ( sdtotal == 2 )
    {
        sdtotal = 0x00;
    }

//    printf( "sdtotal %lld sdfree %lld\n", sdtotal, sdfree );
#ifdef GET_RECORDLIST_PAGE_BY_PAGE
#endif
#if 0
    printf( "bsize %ld\n", fs.f_bsize ); /* optimal transfer block size */
    printf( "block %ld\n", fs.f_blocks ); /* total data blocks in file system */
    printf( "bfree %ld\n", fs.f_bfree ); /* free blocks in fs */
    printf( "bavail %ld\n", fs.f_bavail ); /* free blocks avail to non-superuser */
    printf( "files %ld\n", fs.f_files ); /* total file nodes in file system */
    printf( "free %ld\n", fs.f_ffree ); /* free file nodes in fs */
    printf( "sid %d\n", fs.f_fsid );  /* file system id */
    printf( "namelen %ld\n", fs.f_namelen ); /* maximum length of filenames */
    blocks = fs.f_blocks;
    bfree = fs.f_bfree;
    printf( "blocks %lld bfree %lld\n", blocks, bfree );
    printf( "Total size of / is %lld Mbyte\n", blocks * fs.f_bsize / ( 1024 * 1024 ) );
    printf( "Free size of / is %lld Mbyte\n", bfree * fs.f_bsize / ( 1024 * 1024 ) );
#endif
#if 0
    printf( "blocks %d free %d frsize %d\n", statFS.f_blocks, statFS.f_bfree, statFS.f_frsize );
    sdtotal = ( statFS.f_blocks / 1024 ) * ( statFS.f_frsize / 1024 );
    sdfree = ( statFS.f_bfree / 1024 ) * ( statFS.f_frsize / 1024 );
    printf( "sdtotal %ld sdfree %ld\n", sdtotal, sdfree );
#endif
    return 0;
}

//create record file name
void CreateFileName( char timer, char motion, char gpio )
{
    struct tm*       recordtm = NULL;
    int 			temptime;
    struct stat     stStat;

   //timer record
    if ( timerrecord.start == 0x01 )
    {
        if ( timerrecord.flag == 0x00 )
        {
            timerrecord.flag = 0x01;
            temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
            recordtm = localtime( &temptime );
            sdfilelock();

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE

            /* BEGIN: Added by wupm, 2013/2/21 */
            if ( timerrecord.filename[0] != 0 && stat( timerrecord.filename, &stStat ) >= 0 )
            {
                Textout( "Append a Record File = [%s], Size = %d", timerrecord.filename, ( int )stStat.st_size );
                Append2RecordFileList( timerrecord.filename + 5, ( int )stStat.st_size );
            }

            timerrecord.filename[0] = 0;
            /* END:   Added by wupm, 2013/2/21 */
#endif

#if 1
            sprintf( timerrecord.filename, "/mnt/%04d%02d%02d%02d%02d%02d_%d%d%d.h260",
                     recordtm->tm_year + 1900, recordtm->tm_mon + 1, recordtm->tm_mday,
                     recordtm->tm_hour, recordtm->tm_min, recordtm->tm_sec, timer, motion, gpio );
#else
            sprintf( timerrecord.filename, "/mnt/%04d%02d%02d%02d%02d%02d.h264",
                     recordtm->tm_year + 1900, recordtm->tm_mon + 1, recordtm->tm_mday,
                     recordtm->tm_hour, recordtm->tm_min, recordtm->tm_sec );
#endif
            sdfileunlock();
            printf( "record file is update:%s\n", timerrecord.filename );

            if ( stat( timerrecord.filename, &stStat ) >= 0 )
            {
                char cmd[64];
                memset( cmd, 0x00, 64 );
                sprintf( cmd, "rm -f %s", timerrecord.filename );
                printf( "%s\n", cmd );
                DoSystem( cmd );
            }

            /* BEGIN: Added by wupm, 2013/6/27 */
            bCurrentNewFile = TRUE;

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE
#else
            GetStorageSpace();
#endif
        }
    }
}

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE
void DiskDelFile( void )
{
	/* BEGIN: Deleted by wupm, 2013/11/1 */
    //if ( bparam.stRecordSet.recordover == 0x00 )

	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
    //if ( bell_data.record_cover == 0x01 )
    if ( 1 )
    {
        CheckRecordStopStatus();
        printf( "sd recordover isn't open" );
        return;
    }

    else
    {
        Lock_GetRecordFile();
        DeleteRecordFileList();
        UnLock_GetRecordFile();
    }
}

#else
//disk del file
void DiskDelFile( void )
{
    int 			iRet;
    char 		temp[128];
    unsigned char times = 0;

    int i = 0;
    int nRet = 0;

	/* BEGIN: Deleted by wupm, 2013/11/1 */
    //if ( bparam.stRecordSet.recordover == 0x00 )

    /* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
    //if ( bell_data.record_cover == 0x01 )
    if ( 1 )
    {
        CheckRecordStopStatus();
        printf( "sd recordover isn't open" );
        return;
    }

    else
    {
        FILE* fp = NULL;
        char    temp1[40];
        DoSystem( "ls /mnt/*.h260 > /tmp/getfile.txt " );
        fp = fopen( "/tmp/getfile.txt", "r" );

        if ( fp == NULL )
        {
            printf( "fopen getfile failed and stop\n" );
            printf( "query sd failed" );
            CheckRecordStopStatus();
            return;
        }

        /* BEGIN: Modified by wupm, 2013/4/3 */
#if	1
        fseek( fp, 0L, SEEK_END );

        for ( i = 0; i < 5; i++ )
        {
            nRet = fseek( fp, -29L, SEEK_CUR );

            if ( nRet == -1 )
            {
                Textout( "fseek Error" );
                break;
            }

            fread( temp1, 1, 29, fp );
            temp1[28] = 0;
            Textout( "%d, [%s]", i, temp1 );
            fseek( fp, -29L, SEEK_CUR );

            memset( temp, 0x00, 128 );
            sprintf( temp, "rm -f %s", temp1 );
            DoSystem( temp );
            Textout( "disk del name:%s\n", temp1 );

            iRet = access( temp1, F_OK );

            if ( iRet == 0x00 )
            {
                DoSystem( "mount -o rw,remount /mnt" );
                printf( "del filename failed\n" );
                break;
            }
        }

#else

        while ( !feof( fp ) )
        {
            memset( temp1, 0x00, 40 );
            fgets( temp1, 40, fp );
            memset( temp, 0x00, 128 );
            sprintf( temp, "rm -f %s", temp1 );
            DoSystem( temp );
            printf( "disk del name:%s\n", temp1 );
            iRet = access( temp1, F_OK );

            if ( iRet == 0x00 )
            {
                DoSystem( "mount -o rw,remount /mnt" );
                SetRecordStop();
                printf( "del filename failed\n" );
                break;
            }

            //del 5 file
            times++;

            if ( times >= 5 )
            {
                times = 0;
                break;
            }
        }

#endif

        fclose( fp );
    }
}
#endif


#ifdef	GET_RECORDLIST_PAGE_BY_PAGE

static int nDelFileFailCnt=0;
/* BEGIN: Added by wupm, 2013/3/9 */
#define	CHECK_STOREAGE_TIME	10
void* CheckStoreageProc( void* p )
{
    int iRet = 0;

    while ( 1 )
    {
        sleep( CHECK_STOREAGE_TIME );

        if ( sdokflag )
        {
            /* BEGIN: Added by wupm, 2013/4/11 */
            Lock_GetStorageSpace();

            iRet = GetStorageSpace();

            if ( iRet == 0 )
            {
                //sdtotal, sdfree
                if ( sdfree * 100 < sdtotal )
                {
//                  Textout( "sdtotal %lld sdfree %lld, Call Delete", sdtotal, sdfree );
                    nDelFileFailCnt++;
                    DiskDelFile();
                }
                else
                {
                    nDelFileFailCnt = 0;
                }

                if(nDelFileFailCnt >= 10)
                {
					Textout("software Reboot....");
                    SetRebootCgi();
                }
            }

            UnLock_GetStorageSpace();
        }
    }
}
#endif

//check sd
int CheckSd( void )
{
    char temp1[128];
    FILE* fp = NULL;
    char*	 pdst = NULL;
    char flag = 0x00;
    fp = fopen( "/proc/partitions", "rb" );

    if ( fp == NULL )
    {
		Textout("Not Found /proc/partitions, Return 0");
        return 0;
    }

    while ( !feof( fp ) )
    {
        memset( temp1, 0x00, 128 );
        fgets( temp1, 128, fp );
        pdst = strstr( temp1, "sda" );

        if ( pdst == NULL )
        {
            continue;
        }

        else
        {
			Textout("Found sda, Return 1");
            flag = 0x01;
            break;
        }
    }

    fclose( fp );
	if ( flag == 0 )
	{
		Textout("Not Found sda, Return 0");
	}
    return flag;
}

/* BEGIN: Added by wupm, 2013/5/24 */
void CheckRecord( void )
{
    int iRet;
    struct stat     stStat;
    int             iMode;
    iRet = lstat( "/system/www/record", &stStat );
    iMode = S_ISLNK( stStat.st_mode );

    if ( ( iMode == 0 ) || ( iRet ) )
    {
        /* BEGIN: Modified by wupm, 2013/1/7 */
        DoSystem( "rm /system/www/record" );
        DoSystem( "ln -s /mnt/ /system/www/record" );
        printf( "record is create\n" );
    }
}

/* BEGIN: Added by wupm, 2013/6/16 */
#ifdef	ENABLE_TAKE_PICTURE
BOOL GetSDState()
{
    return sdokflag;
}
#endif

#ifdef TAKE_PHOTO_2_SDCARD
BOOL GetSDState()
{
    return sdokflag;
}
#endif

//sd check proc
void* SdCheckProc( void* p )
{
    char 	cmd[128];
    int   		iRet = -1;
    char 	flag = 0x00;
    static int nCheckCount = 0;

    while ( 1 )
    {
        sleep( 10 );

        if ( flag == 0x00 )
        {
            iRet = CheckSd();
            /* BEGIN: Modified by wupm, 2012/12/26 */
            nCheckCount ++;
            //Textout2( "==========checksd========== %d \n", nCheckCount );

            if ( iRet == 0x01 )
            {
                sleep( 1 );
                printf( "=========mount start===========\n" );
                memset( cmd, 0x00, 128 );
                sprintf( cmd, "mount -t vfat /dev/sda  /mnt" );
                iRet = DoSystem( cmd );
                printf( "=========mount end===========mount() Return = %d\n", iRet );
                /* BEGIN: Modified by wupm, 2012/12/26 */
                sleep( 10 );
                //sleep(1);

                if ( iRet == 0x00 )
                {
					Textout("mount Success");
#ifdef	GET_RECORDLIST_PAGE_BY_PAGE
#else
                    GetStorageSpace();
					Textout("SD Size = %ld / %ld", sdfree, sdtotal );
#endif
                    bparam.stRecordSet.sdstatus = 0x01;
					Textout("Set SD Status = 1");
                    sdokflag = 0x01;
                    CheckRecord();

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE

                    /* BEGIN: Added by wupm, 2013/2/21 */
                    while ( 1 )
                    {
                        int bMotoMoving = GetMotoMoving();

                        if ( bMotoMoving == 0 )
                        {
                            //CreateLargeFiles();
                            InitRecordFileList();
                            Textout( "Init OK, RecordCount=%d", GetRecordFileCount() );
                            break;
                        }

                        sleep( 1 );
                    }

                    /* END:   Added by wupm, 2013/2/21 */
#endif
                }

                else
                {
					Textout("mount Fail");
                    bparam.stRecordSet.sdstatus = 0x00;
					Textout("Set SD Status = 0");
                    //GetStorageSpace();
                    sdokflag = 0x00;
                    //sdokflag = 0x01;	//TEST
                }
            }

            /* BEGIN: Added by wupm, 2012/12/26 */
            else
            {
				Textout("Set SD Status = 0");
                bparam.stRecordSet.sdstatus = 0x00;
            }

            /* END:   Added by wupm, 2012/12/26 */

            /* BEGIN: Modified by wupm, 2012/12/26 */
            //flag = 0x01;//stop check
            if ( bparam.stRecordSet.sdstatus == 0x01 || nCheckCount >= 10 )
            {
                flag = 0x01;//stop check

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE
                /* BEGIN: Added by wupm, 2013/3/6 */
                SetRecordFileListInited( TRUE );
#endif
            }

            /* END:   Modified by wupm, 2012/12/26 */
        }
    }
}
//write sd data
unsigned int WriteSdData( char* name, unsigned char* pbuf, unsigned int len )
{
    unsigned int  iRet = 0;
    FILE*    fp = NULL;

    /* BEGIN: Added by Baggio.wu, 2013/7/18 */
    int bChange=0;
    struct timeval time;
    iRet = access( name, F_OK );
    if ( iRet != 0 )
    {
        gettimeofday(&time, NULL);
        time.tv_sec -= bparam.stDTimeParam.byTzSel;
        settimeofday( &time, NULL );
        bChange = 1;
        //Textout("change system time ---1, time=%s", ctime( &( time.tv_sec ) ));
    }
    /* END:   Added by Baggio.wu, 2013/7/18 */

    fp = fopen( name, "ab+" );
    if ( fp )
    {
        //iRet = fwrite(&len, 1,4, fp);
        iRet = fwrite( pbuf, 1, len, fp );
        fclose( fp );
    }

    else
    {
       //printf( "write sd failed name:%s len %d\n", name, len );
        iRet = -1;

        /* BEGIN: Added by Baggio.wu, 2013/7/8 */
        DoSystem( "mount -o rw,remount /mnt" );
    }

    /* BEGIN: Added by Baggio.wu, 2013/7/18 */
    if(bChange)
    {
        gettimeofday(&time, NULL);
        time.tv_sec += bparam.stDTimeParam.byTzSel;
        settimeofday( &time, NULL );
        //Textout("change system time ---2, time=%s", ctime( &( time.tv_sec ) ));
    }
    /* END:   Added by Baggio.wu, 2013/7/18 */

    return iRet;
}
//write sd filename
int WriteSdFile( unsigned char* pbuf, int sdlen )
{
    unsigned int 	len;
    struct stat     stStat;
    int             	filelen = -1;
    int			iRet = 0;

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE

    /* BEGIN: Added by wupm, 2013/2/21 */
    if ( !RecordFileListInited() )
    {
        //Textout("!RecordFileListInited(), Donothing");
        return 0;
    }

#endif

    //timer record file
    if ( ( timerrecord.start == 0x01 ) && ( timerrecord.flag ) )
    {
        sd_dbg1( "wr0\n", 0 );
        sdfilelock();
        len = WriteSdData( timerrecord.filename, pbuf, sdlen );
        sdfileunlock();
        sd_dbg1( "wr1\n", 0 );

        /* BEGIN: Deleted by wupm, 2013/5/2 */
#ifdef	GET_RECORDLIST_PAGE_BY_PAGE
#else

        if ( stat( timerrecord.filename, &stStat ) < 0 )
        {
            printf( "stat is failed\n" );
        }

        else
        {
            filelen = stStat.st_size;
        }

        sd_dbg1( "wr2\n", 0 );

        //1->write file return 0  2->filelen == 0 3->filelen == sdbackuplen
        if ( ( len != sdlen ) || ( filelen == 0 ) || ( filelen == sdbackuplen ) )
        {
            sd_dbg1( "wr3\n", 0 );
            DiskDelFile();
            sd_dbg1( "wr4\n", 0 );
            GetStorageSpace();
            sd_dbg1( "wr5\n", 0 );
            delfilecnt++;
            printf( "sd write failed len=%d\n", len );
            printf( "len =%d wrlen=%d flen=%d backuplen=%d\n", len, sdlen, filelen, backuplen );
            sleep( 1 );
        }

        else
        {
            sdbackuplen = filelen;
            delfilecnt = 0x00;
        }

        //check del file count
        if ( delfilecnt++ >= 10 )
        {
            sd_dbg1( "wr6\n", 0 );
            GetStorageSpace();
            sd_dbg1( "wr7\n", 0 );
//            printf( "delfilecnt sdtotal %d sdfree %d\n", sdtotal, sdfree );
            DiskDelFile();
            sd_dbg1( "wr8\n", 0 );
            delfilecnt = 0x00;
            CheckRecordStopStatus();
            sd_dbg1( "wr9\n", 0 );
            printf( "delfile failed\n" );
            iRet = -1;
            //system("mount -o rw,remount /media/sd");
        }

#endif
    }

    return iRet;
}
void WriteSDStream( void )
{
    int                     	iRet=0;
    int				status=0;
    unsigned int           len     = 0;
    unsigned char*        pbuf   = NULL;
    LIVEHEAD               phead;
    unsigned char		 sendbuf[1024 * 256];
    static unsigned int     m_FrameNo=0;
    //printf("WriteSDStream backuplen %d prelen %d\n",sdbackuplen,encbufj.prelen);
    mjpglock();

    memcpy( &phead, encbufj.pbuf + encbufj.prelen, sizeof( LIVEHEAD ) );
    if(phead.startcode != STARTCODE)
    {
        mjpgunlock();
        return;
    }

    if(m_FrameNo + 1 > phead.frameno && phead.frameno != 0)
    {
        mjpgunlock();
        return;
    }

    m_FrameNo = phead.frameno;

    //read buffer or need copy buffer?
    pbuf = encbufj.pbuf + encbufj.prelen;
    len = phead.len + sizeof( LIVEHEAD );

    //printf("offset %d prelen %d len %d\n",userindm[usit].offset,encbufj.prelen,len);
    //printf("frameno %d type %d\n",frameno,phead.type);
    if ( ( len >= 1024 * 256 ) || ( len <= 0 ) )
    {
        status = -1;
    }

    else
    {
        memcpy( sendbuf, pbuf, len );
        status = 0;
    }

    mjpgunlock();

    //printf("sd h264 offset %d len:%d\n",userindsd.offset,len);
    if ( status == 0x00 )
    {
        iRet = WriteSdFile( sendbuf, len );
    }

    else
    {
        iRet = -1;
        printf( "sd write len >= 1024 * 256=%d, frameno=%d, type=%d\n", len, phead.frameno, phead.type);
    }
}

/* BEGIN: Modified by Baggio.wu, 2013/7/11 */
void WriteSDAudio( void )
{
    int                     	iRet;
    int				status;
    unsigned int           len     = 0;
    unsigned char*        pbuf   = NULL;
    LIVEHEAD               phead;
    unsigned char		 sendbuf[1024 * 128];
    static unsigned int     m_AFrameNo=0;

    audiolock();
    memcpy( &phead, encbufa.pbuf + encbufa.prelen, sizeof( LIVEHEAD ) );
    if(phead.startcode != STARTCODE)
    {
        audiounlock();
        return;
    }

    if(m_AFrameNo + 1 > phead.frameno && phead.frameno != 0)
    {
        audiounlock();
        return;
    }

    m_AFrameNo = phead.frameno;

    //read buffer or need copy buffer?
    pbuf = encbufa.pbuf + userindsd.aoffset;
    len = phead.len + sizeof( LIVEHEAD );
    if ( len >= 1024 * 128 )
    {
        printf( "=========read audio len failed=%d============", len );
        audiounlock();
        return;
    }

    memcpy( sendbuf, pbuf, len );
    audiounlock();

    WriteSdFile( sendbuf, len );
    Textout1("sd audio, aindex=%d, len=%d", userindsd.aindex, len);
}

/* END:   Modified by Baggio.wu, 2013/7/11 */

//sd write stream proc
void* SdWriteStreamProc( void* p )
{
    char temp[32];
    int   filenct = 0;

    if ( ( sem_init( &semsd, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

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

        sem_wait( &semsd );

        //printf("sdok %d start %d flag %d\n",sdokflag,timerrecord.start,timerrecord.flag);
        if ( sdokflag == 0x01 )
        {
            if ( ( timerrecord.start == 0x01 ) && ( timerrecord.flag ) )
            {
                /* BEGIN: Added by wupm, 2013/6/27 */
#ifdef	RECORD_VIDEO_VIA_MOTODETECT
                if ( bCurrentNewFile )
                {
                    Textout( "-------Write First Video Frame" );
                    sdlock();
                    WriteSDStream();
                    sdunlock();
                    bCurrentNewFile = FALSE;
                }

                else
                {
                    int iRet = MotionDetect();

                    /* BEGIN: Modified by wupm, 2013/7/1 */
                    if ( ( iRet & 0x02 ) == 0x02 )
                    {
                        Textout( "-------Motion Detect, Recording" );
                        sdlock();
                        WriteSDStream();
                        sdunlock();
                    }
                }

#else
                memset( temp, 0x00, 32 );
                sprintf( temp, "start write %d-", filenct++ );
                sd_dbg1( temp, 1 );
                sdlock();
                WriteSDStream();
                sdunlock();
                memset( temp, 0x00, 32 );
                sprintf( temp, "write end", filenct++ );
                sd_dbg1( temp, 0 );
#endif
            }
        }
    }
}
//sd write audio proc
void* SdWriteAudioProc( void* p )
{
    if ( ( sem_init( &semsda, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

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
        sem_wait( &semsda );

        if ( sdokflag == 0x01 )
        {
            if ( ( timerrecord.start == 0x01 ) && ( timerrecord.flag ) )
            {
                sdlock();
                WriteSDAudio();
                sdunlock();
            }
        }
    }
}
//note sd format
void NoteSDFormat( void )
{
	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
    //sem_post( &semsdf );
}
//sd format
int SDFatFormat( void )
{
    char cmd[128];
    printf( "start format...\n" );
    sdokflag = 0x00;
    sleep( 1 );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "umount /mnt" );
    DoSystem( cmd );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "mkdosfs -I  /dev/sda" );
    DoSystem( cmd );
    sleep( 1 );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "mount -t vfat /dev/sda /mnt" );
    DoSystem( cmd );
    sleep( 1 );

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE
    /* BEGIN: Deleted by wupm, 2013/3/9 */
    //GetStorageSpace();

    /* BEGIN: Added by wupm, 2013/2/22 */
    ZeroRecordFileList();
    /* END:   Added by wupm, 2013/2/22 */
#else
    GetStorageSpace();
#endif

    printf( "===end format===\n" );
    sdokflag = 0x01;

    /* BEGIN: Added by wupm, 2013/3/21 */
    return 0;
}
//sd format proc
void* SdFormatProc( void* p )
{
    if ( ( sem_init( &semsdf, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        sem_wait( &semsdf );
        updatelock();
        SDFatFormat();
        updateunlock();
    }
}
//create new record file
void SetRecordNewFile( void )
{
    timerrecord.timer1 = timerrecord.size;
}
//config record size
void ReadRecordSize( void )
{
	/* BEGIN: Deleted by wupm, 2013/11/1 */
	/*
    timerrecord.size = bparam.stRecordSet.timer * 60;

    if ( ( timerrecord.size < 5 * 60 ) || ( timerrecord.size > 180 * 60 ) )
    {
        timerrecord.size = 30 * 60;
    }
    */
    /* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
    //timerrecord.size = bell_data.record_size;
	if ( timerrecord.size < 30 || timerrecord.size > 300 )
	{
		timerrecord.size = 30;
	}

}
//set timer schdule start
void SetTimerRecordStart( void )
{
    if ( timerrecord.schdule == 0x00 )
    {
        timerrecord.schdule = 0x01;	//timer record
        ReadRecordSize();
        SetRecordNewFile();
        printf( "timer sd start is start...%d\n", timerrecord.timer1 );
    }
}
//clr timer schdule
void SetTimerRecordClr( void )
{
    timerrecord.schdule = 0x00;
}
//set motion start
/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
#if	1
void SetMotionRecordStart( void )
{
}
#else
void SetMotionRecordStart( void )
{
    if ( timerrecord.motion == 0x00 )
    {
        timerrecord.motion = 0x01;	//timer record
        ReadRecordSize();
        SetRecordNewFile();
        printf( "motion sd start is start...%d\n", timerrecord.timer1 );
    }
}
#endif
//motion clr
void SetMotionRecordClr( void )
{
    timerrecord.motion = 0x00;	//timer record

    /* BEGIN: Added by Baggio.wu, 2013/7/12 */
    //ReadRecordSize();
    //SetRecordNewFile();
    //SetDoorAlarm(FALSE);
}

//set alarm start
void SetAlarmRecordStart( void )
{
    if ( timerrecord.gpio == 0x00 )
    {
        timerrecord.gpio = 0x01;	//timer record
        ReadRecordSize();
        SetRecordNewFile();
        printf( "alarm sd start is start...%d\n", timerrecord.timer1 );
    }
}
//gpio clr
void SetAlarmRecordClr( void )
{
    timerrecord.gpio = 0x00;	//timer record
    /* BEGIN: Added by Baggio.wu, 2013/7/12 */
    //ReadRecordSize();
    //SetRecordNewFile();
}

//set record start
void CheckRecordStartStatus( void )
{
    if ( timerrecord.start == 0x00 )
    {
        if ( timerrecord.gpio || timerrecord.motion || timerrecord.schdule )
        {
            sdbackuplen = 0;
            sdbackuplena = 0;
            timerrecord.start = 0x01;
            printf( "sd record is start...\n" );

            /* BEGIN: Added by wupm, 2013/3/15 */
            if ( bparam.stRecordSet.sdstatus == 1 )
            {
                bparam.stRecordSet.sdstatus = 2;
            }
        }
    }
}
//set record stop
void CheckRecordStopStatus( void )
{
    struct stat     stStat;

    if ( timerrecord.start == 0x01 )
    {
        if ( ( timerrecord.gpio == 0x00 ) && ( timerrecord.motion == 0x00 ) && ( timerrecord.schdule == 0x00 ) )
        {
            timerrecord.start = 0x00;
            printf( "sd start is stop...\n" );
            /* BEGIN: Added by wupm, 2013/3/15 */
            if ( bparam.stRecordSet.sdstatus == 2 )
            {
                bparam.stRecordSet.sdstatus = 1;
            }

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE

            /* BEGIN: Added by wupm, 2013/2/21 */
            if ( timerrecord.filename[0] != 0 && stat( timerrecord.filename, &stStat ) >= 0 )
            {
                Append2RecordFileList( timerrecord.filename + 5, ( int )stStat.st_size );
            }

            timerrecord.filename[0] = 0;
            /* END:   Added by wupm, 2013/2/21 */
#endif
        }
    }
}
//set record file stop
void SetRecordStop( void )
{
    struct stat stStat;

    timerrecord.gpio = 0x00;
    timerrecord.motion = 0x00;
    timerrecord.schdule = 0x00;
    timerrecord.start = 0x00;
}
//check record file
void CheckRecordStatus( void )
{
    char temp[32];
    static int filecnt = 0;
    //check record event start
    CheckRecordStartStatus();
    //check record event stop
    CheckRecordStopStatus();

    if ( timerrecord.start == 0x01 )
    {
        //printf("timer %d size %d\n",timerrecord.timer1,timerrecord.size);
        timerrecord.timer1++;

        if ( timerrecord.timer1 >= timerrecord.size )
        {
            timerrecord.flag = 0x00;	//create a new recordfile
            timerrecord.timer1 = 0x00;
            memset( temp, 0x00, 32 );
            sprintf( temp, "create a file %d\n", filecnt++ );
            sd_dbg( temp, 1 );
            CreateFileName( timerrecord.schdule, timerrecord.motion, timerrecord.gpio );
            sd_dbg( "create file end\n", 0 );
            printf( "=========create filename=========\n" );
        }
    }
}
//note record start
void NoteRecordStart( void )
{
    sem_post( &recordstart );
}
//record check proc
void* RecordCheckProc( void* p )
{
    while ( 1 )
    {
        //check record status
        if ( sdokflag == 0x01 )
        {
            CheckRecordStatus();
        }

        sleep( 1 );
    }
}


void SdInit( void )
{
    pthread_t		Sdcheckthread;
    pthread_t		recordcheckthread;
    pthread_t		Sdwritethread;
    pthread_t		Sdwritethread1;
    pthread_t		Sdwritethread2;

    SdInitUser();
    InitTimerRecord();
    pthread_create( &Sdcheckthread, 0, &SdCheckProc, NULL );
    pthread_create( &Sdwritethread, 0, &SdWriteStreamProc, NULL );
    pthread_create( &Sdwritethread1, 0, &SdWriteAudioProc, NULL );
    pthread_create( &Sdwritethread2, 0, &SdFormatProc, NULL );
    pthread_create( &recordcheckthread, 0, &RecordCheckProc, NULL );

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE

    if ( 1 )
    {
        pthread_t		CheckStoreageThread;
        pthread_create( &CheckStoreageThread, 0, &CheckStoreageProc, NULL );
    }

#endif
}

