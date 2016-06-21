#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/vfs.h>
#include <semaphore.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "update.h"
#include "param.h"
#include "factory.h"

FILEINDEX 	fileindex;
FILEHEAD	filehead;
sem_t		appupdatesem;
sem_t		sysupdatesem;

void FileInit( void )
{
    memset( &fileindex, 0x00, sizeof( FILEINDEX ) );
    memset( &filehead, 0x00, sizeof( FILEHEAD ) );
}

int FileHeadCheck( void )
{
    int 		iRet;
    struct stat     stStat;
    char		factory;
    //check head is sys
    printf( "system:%2x-%2x-%2x\n", filehead.sys[0], filehead.sys[1], filehead.sys[2] );

    if ( ( filehead.sys[0] != 0x73 ) || ( filehead.sys[1] != 0x79 ) || ( filehead.sys[2] != 0x73 ) )
    {
        printf( "this isn't system file\n" );
        return -1;
    }

    //check factory
    factory = bparam.stIEBaseParam.factory;

    if ( filehead.factory != factory )
    {
        printf( "factory isn't ok\n" );
        return -1;
    }

    return 0;
}

void CheckDir( char* dir )
{
    char 	temp[68];
    char    temp1[80];
    char	i;
    memcpy( temp, dir, 68 );

    for ( i = 68; i > 0; i-- )
    {
        if ( temp[i - 1] == '/' )
        {
            temp[i - 1] = '\0';
            break;
        }

        temp[i - 1] = 0x00;
    }

    memset( temp1, 0x00, 80 );
    sprintf( temp1, "mkdir -p %s\n", temp );
    DoSystem( temp1 );
    //printf("dir:%s\n",temp);
}

int FileReadWrite( FILE* fp, char* dst, unsigned int len )
{
    FILE* fp1 = NULL;
    unsigned char temp[1024];
    char          temp1[72];
    int iRet;
    int iRet1;
    memset( temp1, 0x00, 72 );
    sprintf( temp1, "rm %s", dst );
    DoSystem( temp1 );
    CheckDir( dst );
    fp1 = fopen( dst, "wb" );

    if ( dst == NULL )
    {
        printf( "fopen %s failed\n", dst );
        return -1;
    }

    while ( len > 0 )
    {
        if ( len >= 1024 )
        {
            iRet = fread( temp, 1, 1024, fp );
#if 1
            iRet1 = fwrite( temp, 1, iRet, fp1 );

            if ( iRet1 != iRet )
            {
                printf( "write failed iRet1 %d iRet %d\n", iRet1, iRet );
                break;
            }

#endif
            len -= 1024;
            //printf("dst %s len %d\n",dst,len);
        }

        else
        {
            iRet = fread( temp, 1, len, fp );
#if 1
            iRet1 = fwrite( temp, 1, iRet, fp1 );

            if ( iRet1 != iRet )
            {
                printf( "write failed iRet1 %d iRet %d\n", iRet1, iRet );
                break;
            }

            len -= iRet1;
#else
            len -= iRet;
#endif
            //printf("dst1 %s len %d\n",dst,len);
        }
    }

    fclose( fp1 );
    return 0;
}


int Upgradesystem( char* path )
{
    char 		i;
    int 		iRet = 0;
    FILE*		 fp = NULL;
    unsigned int	cnt = 0;
    FileInit();
#if 0
    fp = fopen( "/tmp/system.tar", "rb" );

    if ( fp == NULL )
    {
        printf( "system.tar file open failed\n" );
        return -1;
    }

#else
    fp = fopen( path, "rb" );

    if ( fp == NULL )
    {
        printf( "%s file open failed\n", path );
        return -1;
    }

#endif
    iRet = fread( &filehead, 1, sizeof( FILEHEAD ), fp );

    if ( iRet != sizeof( FILEHEAD ) )
    {
        printf( "system.tar head is error return %d path %s\n", iRet, path );
        return -1;
    }

    iRet = FileHeadCheck();
    printf( "iRet %d\n", iRet );

    if ( iRet )
    {
        printf( "update failed...\n" );
        fclose( fp );
        return -1;
    }

    while ( !feof( fp ) )
    {
        FILE*		 fp1 = NULL;
        int		iRet1 = 0;
        char    	filename[68];
        unsigned int	len;
        iRet = fread( &fileindex, 1, sizeof( FILEINDEX ), fp );

        if ( iRet == 0x00 )
        {
            break;
        }

        if ( iRet != sizeof( FILEINDEX ) )
        {
            printf( "read fileindex failed=%d\n", iRet );
            break;
        }

        memset( filename, 0x00, 68 );
        sprintf( filename, "/system/", 8 );
        memcpy( filename + 8, fileindex.filename, 64 );
        len = fileindex.filelen;
        printf( "file1:%s len1 %d\n", filename, len );
        FileReadWrite( fp, filename, len );
        printf( "file:%s len %d\n", filename, len );
    }

    fclose( fp );
    DoSystem( "sync" );
    printf( "update ok...\n" );
    return 0;
}

void NoteAppUpdate( void )
{
    sem_post( &appupdatesem );
}

void NoteSysUpdate( void )
{
    sem_post( &sysupdatesem );
}
#if 0
void updateappsystem( void )
{
    int             iRet;
    unsigned int    len;
    unsigned int	len1;
    unsigned int	i = 0;
    unsigned int    offset;
    FILE*            fp = NULL;
    unsigned char*   pbuffer = NULL;
    unsigned char*   pstart  = NULL;
    unsigned char*   pend    = NULL;
    unsigned char*	plen    = NULL;
    unsigned char*	ptemp   = NULL;
    char		temp;
    fp = fopen( "/tmp/post1.bin", "rb" );

    if ( fp == NULL )
    {
        printf( "open post.bin failed\n" );
        return 0;
    }

    pbuffer = malloc( 1024 * 1024 * 3 / 2 );

    if ( pbuffer == NULL )
    {
        printf( "malloc memory failed\n" );
        fclose( fp );
        return 0;
    }

    //read post to buffer
    offset = 0x00;

    while ( !feof( fp ) )
    {
        offset += fread( pbuffer + offset, 1, 4096, fp );
    }

    fclose( fp );
    printf( "read filesize %d\n", offset );
    //find start
    printf( "find head or end\n" );
#if 0
    pstart = strstr( pbuffer, "ifi-camera-app-qazwsxedcrfvtgba" );
#else
    pstart = strstr( pbuffer, appfindstring );
#endif

    if ( pstart == NULL )
    {
        printf( "find app head is error\n" );
        return;
    }

    pstart--;

    while ( i < 128 )
    {
        ptemp = pbuffer + offset - 32 - i;
        pend   = strstr( ptemp, "ifi-camera-end-yhnujmzaqxswcdef" );

        if ( pend )
        {
            break;
        }

        i++;
    }

    if ( pend == NULL )
    {
        printf( "find app end is error\n" );
        return;
    }

    pend--;
    temp = *pend;

    if ( temp != 0x77 )
    {
        printf( "find app end1 is error\n" );
    }

    //find len
    printf( "find file len\n" );
    plen = pstart + 32;
    memcpy( &len, plen, sizeof( int ) );
    len1 = pend - pstart - 36;
    printf( "file len %d diff len %d\n", len, len1 );

    if ( len != len1 )
    {
        printf( "len isn't ok\n" );
        return;
    }

    //create file
    fp = fopen( "/tmp/www.zip", "wb" );

    if ( fp == NULL )
    {
        printf( "create failed\n" );
        free( pbuffer );
        return 0;
    }

    fwrite( pstart + 36, 1, len, fp );
    fclose( fp );
    //free buffer
    free( pbuffer );
    printf( "check buffer is ok\n" );
    //update file
    DoSystem( "rm -rf /system/www/" );
    DoSystem( "mkdir -p /system/www" );
    DoSystem( "unzip -o  /tmp/www.zip -d /system" );
    //rm temp file
    DoSystem( "rm /tmp/post1.bin" );
    DoSystem( "rm /tmp/www.zip" );
    DoSystem( "sync" );
    sleep( 2 );
    ClrUpdateFlag();
    //run next url
}
#endif

#if	0
void updateappsystem( void )
{
    int             iRet;
    unsigned int    len;
    unsigned int    len1;
    unsigned int    i = 0;
    unsigned int    offset;
    FILE*            fp = NULL;
    unsigned char*   pbuffer = NULL;
    unsigned char*   pstart  = NULL;
    unsigned char*   pend    = NULL;
    unsigned char*   plen    = NULL;
    unsigned char*   ptemp   = NULL;
    char            temp;
    fp = fopen( "/tmp/post1.bin", "rb" );

    if ( fp == NULL )
    {
        printf( "open post.bin failed\n" );
        return 0;
    }

    pbuffer = malloc( 1024 * 1024 * 3 / 2 );

    if ( pbuffer == NULL )
    {
        printf( "malloc memory failed\n" );
        fclose( fp );

        /* BEGIN: Modified by wupm, 2013/7/1 */
        //return 0;
        return;
    }

    //read post to buffer
    offset = 0x00;

    while ( !feof( fp ) )
    {
        offset += fread( pbuffer + offset, 1, 4096, fp );
    }

    fclose( fp );
    printf( "read filesize %d\n", offset );
    //find start
    printf( "find head or end\n" );
#if 1
    pstart = strstr( pbuffer, "ifi-camera-app-qazwsxedcrfvtgba" );
#else
    pstart = strstr( pbuffer, appfindstring );
#endif

    if ( pstart == NULL )
    {
        printf( "find app head is error\n" );
        return;
    }

    pstart--;

    while ( i < 128 )
    {
        ptemp = pbuffer + offset - 32 - i;
        pend   = strstr( ptemp, "ifi-camera-end-yhnujmzaqxswcdef" );

        if ( pend )
        {
            break;
        }

        i++;
    }

    if ( pend == NULL )
    {
        printf( "find app end is error\n" );
        return;
    }

    pend--;
    temp = *pend;

    if ( temp != 0x77 )
    {
        printf( "find app end1 is error\n" );
    }

    //find len
    printf( "find file len\n" );
    plen = pstart + 32;
    memcpy( &len, plen, sizeof( int ) );
    len1 = pend - pstart - 36;
    printf( "file len %d diff len %d\n", len, len1 );

    if ( len != len1 )
    {
        printf( "len isn't ok\n" );
        return;
    }

    //create file
    fp = fopen( "/tmp/www.zip", "wb" );

    if ( fp == NULL )
    {
        printf( "create failed\n" );
        free( pbuffer );

        /* BEGIN: Modified by wupm, 2013/7/1 */
        //return 0;
        return;
    }

    fwrite( pstart + 36, 1, len, fp );
    fclose( fp );
    //free buffer
    free( pbuffer );
    printf( "check buffer is ok\n" );
    //update file
    DoSystem( "cp /system/www/system.ini /tmp/system.ini" );
    DoSystem( "cp /system/www/system-b.ini /tmp/system-b.ini" );
    DoSystem( "rm -rf /system/www/" );
    DoSystem( "mkdir -p /system/www" );
    DoSystem( "cp /tmp/system.ini /system/www/system.ini" );
    DoSystem( "cp /tmp/system-b.ini /system/www/system-b.ini" );
    DoSystem( "unzip1 -o -P vstarcam!@#$%  /tmp/www.zip -d /system" );
    //rm temp file
    DoSystem( "rm /tmp/post1.bin" );
    DoSystem( "rm /tmp/www.zip" );
    DoSystem( "sync" );
    sleep( 2 );
    ClrUpdateFlag();
    //run next url
}


//update App  proc
void* UpdateAppProc( void* p )   //update app proc
{
    printf( "start app update thread\n" );

    if ( ( sem_init( &appupdatesem, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        sem_wait( &appupdatesem );
        updatelock();
        printf( "thread update app\n" );
        updateappsystem();
        sleep( 1 );
        updateunlock();
        MotoTimeStop();
        Es838Close();
		Textout("software Reboot....");
        DoSystem( "reboot" );

    }
}


void updatesyssystem( void )
{
    int             iRet;
    unsigned int    len;
    unsigned int    len1;
    unsigned int    i = 0;
    unsigned int    offset;
    FILE*            fp = NULL;
    unsigned char*   pbuffer = NULL;
    unsigned char*   pstart  = NULL;
    unsigned char*   pend    = NULL;
    unsigned char*   plen    = NULL;
    unsigned char*   ptemp   = NULL;
    char            temp;
    struct stat     stStat;
    fp = fopen( "/tmp/post1.bin", "rb" );

    if ( fp == NULL )
    {
        printf( "open post1.bin failed\n" );

        /* BEGIN: Modified by wupm, 2013/7/1 */
        //return 0;
        return;
    }

    pbuffer = malloc( 1024 * 1024 * 3 / 2 );

    if ( pbuffer == NULL )
    {
        printf( "malloc memory failed\n" );
        fclose( fp );

        /* BEGIN: Modified by wupm, 2013/7/1 */
        //return 0;
        return;
    }

    //read post to buffer
    offset = 0x00;

    while ( !feof( fp ) )
    {
        offset += fread( pbuffer + offset, 1, 4096, fp );
    }

    fclose( fp );
    printf( "read filesize %d\n", offset );
    //find start
    printf( "find head or end\n" );
    pstart = strstr( pbuffer, "wifi-camera-sys-qetyipadgjlzcbmn" );

    if ( pstart == NULL )
    {
        printf( "find sys head is error\n" );
        return;
    }

    while ( i < 128 )
    {
        ptemp = pbuffer + offset - 32 - i;
        pend   = strstr( ptemp, "ifi-camera-end-nvxkhfsouteqzhpo" );

        if ( pend )
        {
            break;
        }

        i++;
    }

    if ( pend == NULL )
    {
        printf( "find sys end is error\n" );
        return;
    }

    pend--;
    temp = *pend;

    if ( temp != 0x77 )
    {
        printf( "find sys end1 is error\n" );
    }

    //find len
    printf( "find file len\n" );
    plen = pstart + 32;
    memcpy( &len, plen, sizeof( int ) );
    len1 = pend - pstart - 36;
    printf( "file len %d diff len %d\n", len, len1 );

    if ( len != len1 )
    {
        printf( "len isn't ok\n" );
        return;
    }

    //create file
    fp = fopen( "/tmp/system.zip", "wb" );

    if ( fp == NULL )
    {
        printf( "create failed\n" );
        free( pbuffer );

        /* BEGIN: Modified by wupm, 2013/7/1 */
        //return 0;
        return;
    }

    fwrite( pstart + 36, 1, len, fp );
    fclose( fp );
    //free buffer
    free( pbuffer );
    printf( "check buffer is ok\n" );
#if 1
    DoSystem( "rm /tmp/post1.bin" );
    //update file
    DoSystem( "cp /system/system/bin/encoder /tmp" );
    DoSystem( "cp /system/system/bin/gmail_thread /tmp" );
    DoSystem( "cp /system/system/bin/daemon /tmp" );
    DoSystem( "cp /system/system/bin/cmd_thread /tmp" );
    DoSystem( "rm -rf /system/system/bin/encoder" );
    DoSystem( "rm -rf /system/system/bin/ftp_thread" );
//	system("rm -f /system/system/lib/*.so");
    DoSystem( "cp /tmp/encoder /system/system/bin" );
    DoSystem( "cp /tmp/gmail_thread /system/system/bin" );
    DoSystem( "cp /tmp/daemon /system/system/bin" );
    DoSystem( "cp /tmp/cmd_thread /system/system/bin" );
    DoSystem( "rm -rf /tmp/encoder" );
    DoSystem( "rm -rf /tmp/daemon" );
    DoSystem( "rm -rf /tmp/cmd_thread" );
    DoSystem( "unzip -o /tmp/system.zip -d /." );
    DoSystem( "chmod a+x /system/system/bin/*" );
    DoSystem( "chmod a+x /system/init/ipcam.sh" );
    DoSystem( "sync" );
    //rm temp file
    printf( "update ok\n" );
    DoSystem( "rm /tmp/system.zip" );
    printf( "clear temp file\n" );
    DoSystem( "sync" );
    sleep( 2 );
    ClrUpdateFlag();
#endif
    //run next url
}

//update Sys  proc
void* UpdateSysProc( void* p )   //update sys proc
{
    printf( "start sys update thread\n" );

    if ( ( sem_init( &sysupdatesem, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        sem_wait( &sysupdatesem );
        updatelock();
        printf( "thread update sys\n" );
        updatesyssystem();
        sleep( 1 );
        updateunlock();
        MotoTimeStop();
        Es838Close();
#ifdef REBOOT_DBG
        {
            Reboot_dbg( __FILE__, __LINE__, __FUNCTION__, "reboot", 0 );
        }
#endif
		Textout("software Reboot....");
        DoSystem( "reboot" );

    }
}
#endif

void UpdateStart( void )
{
    //pthread_t               updaeappthread;
    //pthread_t               updaesysthread;
    //pthread_create( &updaeappthread, 0, &UpdateAppProc, NULL );
    //pthread_create(&updaesysthread, 0,&UpdateSysProc, NULL);
}

