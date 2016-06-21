#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ipc.h"

int aisocketipc = -1;

int aiIPCSocketInit( void )
{
    //create socket
    int iRet;
    struct sockaddr_in addr;
    struct timeval timeout1 = {5, 0};
    aisocketipc = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( aisocketipc == -1 )
    {
        printf( "socket failure\n" );
        return -1;
    }

    printf( "socket fd=%d\n", aisocketipc );
#if 1
    //build connection address
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 6668 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = bind( aisocketipc, ( struct sockaddr* )&addr, sizeof( addr ) );

    if ( iRet < 0 )
    {
        printf( "bind failured" );
        CloseSocket( aisocketipc );
        return -1;
    }

#endif
    setsockopt( aisocketipc, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout1, sizeof( struct timeval ) );
    printf( "bind(6668) address successful!\n" );
    return aisocketipc;
}

int aiIPCRcvCmd( unsigned char* pbuf )
{
    int iRet;
    struct sockaddr_in addr;
    socklen_t len = sizeof( struct sockaddr_in );
    iRet = recvfrom( aisocketipc, pbuf, 16, 0, ( struct sockaddr* )&addr, &len );

    if ( iRet <= 0 )
    {
        return -1;
    }

    return iRet;
}

int aiIPCSendData( unsigned char* pbuf, unsigned int len )
{
    int iRet;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 6669 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = sendto( aisocketipc, pbuf, len, 0, ( struct sockaddr* )&addr, sizeof( addr ) );
    //printf("audio send %d return %d\n",len,iRet);
    return iRet;
}

int aiDeviceSend( unsigned char* pbuf, unsigned int len )
{
    int 	iRet;
    char 	buffer[16];
    //printf("device audioin send %d\n",len);
    iRet = aiIPCRcvCmd( buffer );

    if ( iRet < 0 )
    {
        printf( "ai device cmd recive:%d\n", iRet );
        return -1;
    }

    aiIPCSendData( pbuf, len );
    return 0;
}

