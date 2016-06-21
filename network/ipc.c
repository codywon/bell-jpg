#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#include "ipc.h"
#include "cmdhead.h"
#include "param.h"
#include "debug.h"

int socketmail 	= -1;
int socketftp 		= -1;
int socketcmd 	= -1;
int socketnet 	= -1;

struct sockaddr_in 	ipcaddr;
socklen_t 			ipclen = sizeof( struct sockaddr_in );
pthread_mutex_t     updatemutex = PTHREAD_MUTEX_INITIALIZER;
sem_t				reboot_sem;


void updatelock( void )
{
    pthread_mutex_lock( &updatemutex );
}

void updateunlock( void )
{
    pthread_mutex_unlock( &updatemutex );
}

int MailSocketInit( void )
{
    //create socket
    int iRet;
    struct sockaddr_in addr;
    struct timeval timeout1 = {15, 0};
    socketmail = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( socketmail == -1 )
    {
        printf( "socket failure\n" );
        return -1;
    }

    printf( "zqh socket fd=%d\n", socketmail );
    //build connection address
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 8813 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = bind( socketmail, ( struct sockaddr* )&addr, sizeof( addr ) );

    if ( iRet < 0 )
    {
        printf( "bind failured" );
        CloseSocket( socketmail );
        return -1;
    }

    setsockopt( socketmail, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout1, sizeof( struct timeval ) );
    printf( "zqh bind(8813) address successful!\n" );
    return socketmail;
}
//email read
int EmailRead( unsigned char* param )
{
    int iRet;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    iRet = recvfrom( socketmail, param, 1024, 0, ( struct sockaddr* )&addr, &addr_len );
    printf( "email read:%d %02x-%02x-%02x-%02x\n", iRet, param[0], param[1], param[2], param[3] );

    if ( iRet <= 0 )
    {
        return -1;
    }

    return iRet;
}
//email write
int EmailWrite( unsigned char* param, short len )
{
    int iRet;
    struct sockaddr_in addr;
    char    temp[1024];

    addr.sin_family = AF_INET;
    addr.sin_port = htons( 8812 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = sendto( socketmail, param, len, 0, ( struct sockaddr* )&addr, sizeof( addr ) );
    memset( temp, 0x00, 1024 );
    EmailRead( temp );
    return iRet;
}
int FtpSocketInit( void )
{
    int iRet;
    struct sockaddr_in addr;
    struct timeval timeout1 = {15, 0};
    socketftp = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( socketftp == -1 )
    {
        printf( "ftp socket create failure\n" );
        return -1;
    }

    printf( "ftp socket=%d\n", socketftp );
    //build connection address
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 8822 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = bind( socketftp, ( struct sockaddr* )&addr, sizeof( addr ) );

    if ( iRet < 0 )
    {
        printf( "ftp socket bind failured\n" );
        CloseSocket( socketftp );
        return -1;
    }

    setsockopt( socketftp, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout1, sizeof( struct timeval ) );
    printf( "ftp socket bind(8822) success\n" );
    return socketftp;
}
//ftp write
int FtpWrite( unsigned char* param, short len )
{
    int iRet;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 8821 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = sendto( socketftp, param, len, 0, ( struct sockaddr* )&addr, sizeof( addr ) );
    printf( "ftp write %d\n", iRet );
    return iRet;
}

//cmd socket init
int CmdSocketInit( void )
{
    //create socket
    int iRet;
    struct sockaddr_in addr;
    struct timeval timeout1 = {30, 0};
    socketcmd = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( socketcmd == -1 )
    {
        printf( "cmd socket create failure\n" );
        return -1;
    }

    printf( "cmd socket=%d\n", socketcmd );

    addr.sin_family = AF_INET;
    addr.sin_port = htons( 8832 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = bind( socketcmd, ( struct sockaddr* )&addr, sizeof( addr ) );

    if ( iRet < 0 )
    {
        printf( "cmd socket bind failured\n" );
        CloseSocket( socketcmd );
        return -1;
    }

    setsockopt( socketcmd, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout1, sizeof( struct timeval ) );
    printf( "cmd socket bind(8832)success\n" );
    return socketcmd;
}
//cmd read
int CmdRead( char* param, int maxlen)
{
    int iRet;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    iRet = recvfrom( socketcmd, param, maxlen, 0, ( struct sockaddr* )&addr, &addr_len );
    if ( iRet <= 0 )
    {
        return -1;
    }

    return iRet;
}
//cmd write
int CmdWrite( unsigned char* param, short len )
{
    int iRet;
    struct sockaddr_in addr;
    char 	temp[1024];
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 8831 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = sendto( socketcmd, param, len, 0, ( struct sockaddr* )&addr, sizeof( addr ) );
    CmdRead( temp, 1024 );
    return iRet;
}
//net socket init
int NetSocketInit( void )
{
    //create socket
    int iRet;
    struct sockaddr_in addr;
    struct timeval timeout1 = {30, 0};
    socketnet = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( socketnet == -1 )
    {
        printf( "socket failure\n" );
        return -1;
    }

    printf( "zqh socket fd=%d\n", socketnet );
    //build connection address
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 9124 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = bind( socketnet, ( struct sockaddr* )&addr, sizeof( addr ) );

    if ( iRet < 0 )
    {
        printf( "bind failured" );
        CloseSocket( socketnet );
        return -1;
    }

    setsockopt( socketnet, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout1, sizeof( struct timeval ) );
    printf( "zqh bind(9124) address successful!\n" );
    return socketnet;
}
//net read
int NetRead( unsigned char* param, int len )
{
    int 					iRet;
    struct sockaddr_in 	addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    iRet = recvfrom( socketnet, param, len, 0, ( struct sockaddr* )&addr, &addr_len );

    if ( iRet <= 0 )
    {
        return -1;
    }

    return iRet;
}
//net write
int NotifyToDaemon( unsigned cmd, unsigned char* param, int  len )
{
    int 					iRet;
    struct sockaddr_in 	addr;
    bnetparam.startcode 	= 0x8019;
    bnetparam.cmd 		= cmd;
    memcpy( &bnetparam.bparam, param, len );
    addr.sin_family 		= AF_INET;
    addr.sin_port 			= htons( 9123 );
    addr.sin_addr.s_addr 	= inet_addr( "127.0.0.1" );
    iRet = sendto( socketnet, &bnetparam, len, 0, ( struct sockaddr* )&addr, sizeof( addr ) );
    return iRet;
}

void SystemBoot( void )
{
    int flag ;

    updatelock();
    flag = GetUpdateFlag();

    if ( flag == 0x00 )
    {
        Es838Close();
        AudioInRelease();

        DoSystem( "sync" );
        DoSystem( "reboot" );
    }

    updateunlock();
}

void SetRebootCgi( void )
{
    sem_post( &reboot_sem );
}

void* rebootproc( void* p )
{
    int iRet = 0;

    if ( ( sem_init( &reboot_sem, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        sem_wait( &reboot_sem );
        SystemBoot();
    }
}

void* NetThreadproc( void* p )
{
    int 				iRet = 0;
    IEPARAMCMD 		param;

    while ( 1 )
    {
        memset( &param, 0x00, sizeof( IEPARAMCMD ) );
        iRet = NetRead( &param, sizeof( IEPARAMCMD ) );

        if ( ( iRet <= 0x00 ) || ( param.startcode != 0x8019 ) )
        {
            sleep( 1 );
            continue;
        }

        switch ( param.cmd )
        {
            case 0:		//send bparam
                //memcpy net to bparam
                Textout("Recv Daemon Sync NetWork command(0)");
                memcpy( &bparam.stNetParam, &param.bparam.stNetParam, sizeof( NETPARAM ) );
                SaveSystemParam( &bparam );
                Textout( "Save network Parameters to flash and reboot\n" );
                SetRebootCgi();
                break;

            case 1:	//network
				bparam.stBell.status = param.bparam.stBell.status;
				Textout("Recv from Daemon, Eth Network Status = %d", bparam.stBell.status);
                netok = 1;
                memcpy( &bparam.stNetParam, &param.bparam.stNetParam, sizeof( NETPARAM ) );

                ConfigVersion();
                SaveSystemParam( &bparam );
                NetworkInitOK();
                break;

            case 2:	//wireless

				/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/30 */
				bparam.stBell.status = param.bparam.stBell.status;
				Textout("Recv from Daemon, Wifi Network Status = %d", bparam.stBell.status);

                netok = 0;
                memcpy( &bparam.stNetParam, &param.bparam.stNetParam, sizeof( NETPARAM ) );

                ConfigVersion();
                SaveSystemParam( &bparam );
                NetworkInitOK();
                break;

            default:
                printf( "cmd isn't support=%x\n", param.cmd );
                break;
        }
    }
}

void IPCInit( void )
{
    DoSystem( "ifconfig lo 127.0.0.1" );
    CmdSocketInit();
    MailSocketInit();
    FtpSocketInit();
    NetSocketInit();
}

void IPCThread( void )
{
    pthread_t		rebootthread;
    pthread_t       	netthread;
    pthread_create( &rebootthread, 0, rebootproc, NULL );
    pthread_create( &netthread, 0, NetThreadproc, NULL );
}


