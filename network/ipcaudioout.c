#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ipc.h"

int aosocketipc = -1;

int aoIPCSocketInit( void )
{
    //create socket
    int iRet;
    struct sockaddr_in addr;
    struct timeval timeout1 = {5, 0};
    aosocketipc = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( aosocketipc == -1 )
    {
        printf( "socket failure\n" );
        return -1;
    }

    printf( "socket fd=%d\n", aosocketipc );
#if 1
    //build connection address
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 6670 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = bind( aosocketipc, ( struct sockaddr* )&addr, sizeof( addr ) );

    if ( iRet < 0 )
    {
        printf( "bind failured" );
        CloseSocket( aosocketipc );
        return -1;
    }

#endif
    //setsockopt(aosocketipc,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout1,sizeof(struct timeval));
    printf( "bind address successful!\n" );
    return aosocketipc;
}

int aoIPCRcvData( unsigned char* pbuf )
{
    int iRet;
    struct sockaddr_in addr;
    socklen_t len;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 6670 );
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    iRet = recvfrom( aosocketipc, pbuf, 640, 0, ( struct sockaddr* )&addr, &len );

    if ( iRet <= 0 )
    {
        return 0;
    }

    return iRet;
}

