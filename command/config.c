#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "cmdhead.h"

int socketipc = -1;

struct sockaddr_in      ipcaddr;
socklen_t              	ipclen = sizeof( struct sockaddr_in );

int CloseSocket( void )          // close socket
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
    struct timeval timeout1 = {5, 0};
    socketipc = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( socketipc == -1 )
    {
        printf( "socket failure\n" );
        return -1;
    }

    //printf("socket fd=%d\n",socketipc);
    //build connection address
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 6667 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = bind( socketipc, ( struct sockaddr* )&addr, sizeof( addr ) );

    if ( iRet < 0 )
    {
        printf( "bind failured" );
        CloseSocket();
        return -1;
    }

    setsockopt( socketipc, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout1, sizeof( struct timeval ) );
    //printf("bind(6666) address successful!\n");
    return socketipc;
}

int IPCRead( unsigned char* param )
{
    int iRet;
    iRet = recvfrom( socketipc, param, 1024, 0, ( struct sockaddr* )&ipcaddr, &ipclen );

    if ( iRet <= 0 )
    {
        return -1;
    }

    return iRet;
}

int IPCWrite( unsigned char* param, short len )
{
    int iRet;
    ipcaddr.sin_family = AF_INET;
    ipcaddr.sin_port = htons( 6666 );
    ipcaddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = sendto( socketipc, param, len, 0, ( struct sockaddr* )&ipcaddr, ipclen );
    return iRet;
}

int IPCSendCmd( short cmd )
{
    IPCCMDPARAM     ipccmd;
    IPCSocketInit();
    ipccmd.ipccmd = cmd;
    ipccmd.index  = 0x00;
    ipccmd.result = 0x00;
    ipccmd.len    = 0x00;
    IPCWrite( &ipccmd, sizeof( IPCCMDPARAM ) );
    CloseSocket();
}
