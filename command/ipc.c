#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#include "ipc.h"
#include "cmdhead.h"

int socketipc = -1;

struct sockaddr_in 	ipcaddr;
socklen_t 		ipclen = sizeof( struct sockaddr_in );
char			updateflag = 0x00;
pthread_mutex_t         updatemutex = PTHREAD_MUTEX_INITIALIZER;
sem_t			reboot_sem;

int CloseSocket( int socketipc )          // close socket
{
    int     iRet = -1;

    if ( socketipc == -1 )
    {
        return iRet;
    }

    shutdown( socketipc, 2 );
    close( socketipc );
    return 0;
}


int IPCSocketInit( void )
{
    //create socket
    int iRet;
    struct sockaddr_in addr;
    struct timeval timeout1 = {10, 0};
    socketipc = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( socketipc == -1 )
    {
        printf( "socket failure\n" );
        return -1;
    }

    printf( "zqh socket fd=%d\n", socketipc );
    //build connection address
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 8831 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = bind( socketipc, ( struct sockaddr* )&addr, sizeof( addr ) );

    if ( iRet < 0 )
    {
        printf( "bind failured" );
        CloseSocket( socketipc );
        return -1;
    }

    setsockopt( socketipc, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout1, sizeof( struct timeval ) );
    printf( "zqh bind(6666) address successful!\n" );
    return socketipc;
}

int IPCRead( unsigned char* param )
{
    int iRet;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 8831 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = recvfrom( socketipc, param, 1024, 0, ( struct sockaddr* )&addr, &ipclen );

    if ( iRet <= 0 )
    {
        return -1;
    }

    return iRet;
}

int IPCWrite( unsigned char* param, short len )
{
    int iRet;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 8832 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = sendto( socketipc, param, len, 0, ( struct sockaddr* )&addr, sizeof( addr ) );
    return iRet;
}

void* IPCThreadProc( void* p )
{
    int 		iRet = 0;
    unsigned char   cmdbuf[1024];
    char		result[8];
    printf( "ie param ppid %d\n", getppid() );

    while ( 1 )
    {
        memset( cmdbuf, 0x00, 1024 );
        iRet = IPCRead( cmdbuf );

        //printf("IPC Read:%d\n",iRet);
        if ( iRet <= 0x00 )
        {
            sleep( 1 );
            continue;
        }

        system( cmdbuf );
        //printf("system len %d cmd:%s\n",strlen(cmdbuf),cmdbuf);
        iRet = 0x00;
        IPCWrite( &iRet, sizeof( int ) );
    }
}

void IPCInit( void )
{
    system( "ifconfig lo 127.0.0.1" );
    IPCSocketInit();
}
void IPCThread( void )
{
    pthread_t       ipcthreadID;
    pthread_t	rebootthread;
    pthread_create( &ipcthreadID, 0, &IPCThreadProc, NULL );
}



