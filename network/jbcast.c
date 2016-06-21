#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>;
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include "alarm.h"
#include "param.h"

#define BROADCAST_RECV_PORT_JPEG             	10000
#define GET_DEVICE_JPEG           				0x00
#define GET_DEVICE_JPEG_RESPONSE          	0x01
#define SET_DEVICE_JPEG           				0x02
#define SET_DEVICE_JPEG_RESPONSE          	0x03
#define START_CODE_JPEG					0x495f4f4d


int 					iSocketjpeg = -1;
int                  		iAddrLenjpeg;
int					backupportjpeg;

extern unsigned char  	netmac[6];
//get system config
void GetSystemConfigJpeg( int iSocket, unsigned short nSessionID, unsigned short type )
{
    char            	szBuffer[256];
    struct sockaddr_in 	pClientAddr;
    int			iRet;
    int			len;
//head
    int             	startcode;      //jpeg head
    short           	cmd;            //jpeg cmd
    char            	cmdother;          //other
    char            	cmdother1[8];      //other1
    int             	cmdlen;         //len
    int             	cmdother2;
//respone
    char			deviceid[13];
    char			devicename[21];
    int			ip;
    int			mask;
    int			gateway;
    int			dns;
    int			other;
    int			sysver = 0x1f021700; //0x0017021f;
    int			appver = 0x0c040900; //0x0009040c;
    short		port;
    char			other2 = 0x01;
    //head
    startcode = START_CODE_JPEG;
    cmd = GET_DEVICE_JPEG_RESPONSE;
    cmdlen = 65;
    //text
    memcpy( deviceid, bparam.stIEBaseParam.dwDeviceID, 13 );
    memcpy( devicename, bparam.stIEBaseParam.szDevName, 21 );
    ip = inet_addr( bparam.stNetParam.szIpAddr );
    mask = inet_addr( bparam.stNetParam.szMask );
    gateway = inet_addr( bparam.stNetParam.szGateway );
    dns = inet_addr( bparam.stNetParam.szDns1 );
    port = htons( bparam.stNetParam.nPort );
    //printf("ip:%x\n",ip);
    memset( szBuffer, 0x00, 128 );
    //startcode
    len = 0;
    memcpy( szBuffer, &startcode, 4 );
    len += 4;
    //cmd
    memcpy( szBuffer + len, &cmd, 2 );
    len += 2;
    //other
    len += 9;
    //cmdlen
    memcpy( szBuffer + len, &cmdlen, 4 );
    len += 4;
    //other2
    memcpy( szBuffer + len, &cmdlen, 4 );
    len += 4;
    //deivceid
    memcpy( szBuffer + len, deviceid, 13 );
    len += 13;
    //devicename
    memcpy( szBuffer + len, devicename, 21 );
    len += 21;
    //ip
    memcpy( szBuffer + len, &ip, 4 );
    len += 4;
    //mask
    memcpy( szBuffer + len, &mask, 4 );
    len += 4;
    //gateway
    memcpy( szBuffer + len, &gateway, 4 );
    len += 4;
    //dns
    memcpy( szBuffer + len, &dns, 4 );
    len += 4;
    //other
    len += 4;
    //sysver
    memcpy( szBuffer + len, &sysver, 4 );
    len += 4;
    //appver
    memcpy( szBuffer + len, &appver, 4 );
    len += 4;
    //port
    memcpy( szBuffer + len, &port, 2 );
    len += 2;
    //other
    memcpy( szBuffer + len, &other2, 1 );
    len += 1;
//send respone
    pClientAddr.sin_family         = AF_INET;
    pClientAddr.sin_port           = backupportjpeg;//htons(BROADCAST_SEND_PORT);
    pClientAddr.sin_addr.s_addr    = inet_addr( "255.255.255.255" );
    iRet = sendto( iSocket, szBuffer, len, 0, ( struct sockaddr* )&pClientAddr, sizeof( struct sockaddr_in ) );
    //printf("iRet=%d port %d\n",iRet,backupportjpeg);
}

unsigned char GetHexValue( const char* pszMacInfo )
{
    unsigned char byValue;

    if ( *pszMacInfo >= 'a' && *pszMacInfo <= 'f' )
    {
        byValue = ( *pszMacInfo - 'a' + 10 ) * 16;
    }

    else if ( *pszMacInfo >= 'A' && *pszMacInfo <= 'F' )
    {
        byValue = ( *pszMacInfo - 'A' + 10 ) * 16;
    }

    else
    {
        byValue = ( *pszMacInfo - 0x30 ) * 16;
    }

    if ( *( pszMacInfo + 1 ) >= 'a' && *( pszMacInfo + 1 ) <= 'f' )
    {
        byValue += ( *( pszMacInfo + 1 ) - 'a' + 10 );
    }

    else if ( *( pszMacInfo + 1 ) >= 'A' && *( pszMacInfo + 1 ) <= 'F' )
    {
        byValue += ( *( pszMacInfo + 1 ) - 'A' + 10 );
    }

    else
    {
        byValue += ( *( pszMacInfo + 1 ) - 0x30 );
    }

    return byValue;
}


//config system
void ConfigSystemJpeg( int iSocket, const char* pszMessage, unsigned short nMessageLen, unsigned short nSessionID )
{
    char                    szBuffer[256];
    struct sockaddr_in      pClientAddr;
    int                     iRet;
    int                     len;
    short			result;
//head
    int                     startcode;      //jpeg head
    short                   cmd;            //jpeg cmd
    char                    cmdother;          //other
    char                    cmdother1[8];      //other1
    int                     cmdlen;         //len
    int                     cmdother2;
//text
    int			checkid;
    char                    deviceid[13];
    char                    deviceuser[13];
    char			devicepasswd[13];
    int                     ip;
    int                     mask;
    int                     gateway;
    int                     dns;
    int                     other;
    short                   port;
    int i;
    unsigned char szDeviceid[6] = {0};
//get
    printf( "config ip\n" );
    //check user and passwd
    len = 23;			//head len
    //checkid
    memcpy( &checkid, pszMessage + len, 4 );
    len += 4;
    //deviceid
    memcpy( deviceid, pszMessage + len, 13 );
    len += 13;

    //printf("deviceid: %s\n", deviceid);
    //printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n", netmac[0], netmac[1], netmac[2], netmac[3], netmac[4], netmac[5]);

    for ( i = 0; i < 6; i++ )
    {
        szDeviceid[i] = GetHexValue( &deviceid[i * 2] );
        //printf("&deviceid[i*2]: %s\n", &deviceid[i*2]);
        // printf("szDeviceid[%d]: %d\n", i, szDeviceid[i]);
    }

    //printf("device ok: %02x:%02x:%02x:%02x:%02x:%02x\n", (unsigned char)szDeviceid[0],
    //    (unsigned char)szDeviceid[1], (unsigned char)szDeviceid[2], (unsigned char)szDeviceid[3],
    //    (unsigned char)szDeviceid[4], (unsigned char)szDeviceid[5]);

    for ( i = 0; i < 6; i++ )
    {
        if ( szDeviceid[i] != netmac[i] )
        {
            return;
        }
    }

    //user
    memcpy( deviceuser, pszMessage + len, 13 );
    len += 13;
    //pwd
    memcpy( devicepasswd, pszMessage + len, 13 );
    len += 13;
    //ip
    memcpy( &ip, pszMessage + len, 4 );
    len += 4;
    //mask
    memcpy( &mask, pszMessage + len, 4 );
    len += 4;
    //gateway
    memcpy( &gateway, pszMessage + len, 4 );
    len += 4;
    //dns
    memcpy( &dns, pszMessage + len, 4 );
    len += 4;
    //port
    memcpy( &port, pszMessage + len, 2 );
    //other
#if 0
    printf( "checkid:%x ip %x mask %x gateway %x dns %x port %d\n", checkid, ip, mask, gateway, dns, htons( port ) );
    printf( "user:%s pass:%s\n", deviceuser, devicepasswd );
#else
    sprintf( bparam.stNetParam.szIpAddr, "%d.%d.%d.%d", ip & 0xff, ( ip >> 8 ) & 0xff, ( ip >> 16 ) & 0xff, ( ip >> 24 ) & 0xff );
    sprintf( bparam.stNetParam.szMask, "%d.%d.%d.%d", mask & 0xff, ( mask >> 8 ) & 0xff, ( mask >> 16 ) & 0xff, ( mask >> 24 ) & 0xff );
    sprintf( bparam.stNetParam.szGateway, "%d.%d.%d.%d", gateway & 0xff, ( gateway >> 8 ) & 0xff, ( gateway >> 16 ) & 0xff, ( gateway >> 24 ) & 0xff );
    sprintf( bparam.stNetParam.szDns1, "%d.%d.%d.%d", dns & 0xff, ( dns >> 8 ) & 0xff, ( dns >> 16 ) & 0xff, ( dns >> 24 ) & 0xff );
    bparam.stNetParam.nPort = htons( port );
    SaveSystemParam( &bparam );
#endif
//=========send respone=========================
    //head
    startcode = START_CODE_JPEG;
    cmd = SET_DEVICE_JPEG_RESPONSE;
    cmdlen = 2;
    result = 0;
    memset( szBuffer, 0x00, 128 );
    //startcode
    len = 0;
    memcpy( szBuffer, &startcode, 4 );
    len += 4;
    //cmd
    memcpy( szBuffer + len, &cmd, 2 );
    len += 2;
    //other
    len += 9;
    //cmdlen
    memcpy( szBuffer + len, &cmdlen, 4 );
    len += 4;
    //other2
    len += 4;
    //status
    memcpy( szBuffer + len, &result, 2 );
    len += 2;
//send respone
    pClientAddr.sin_family         = AF_INET;
    pClientAddr.sin_port           = backupportjpeg;//htons(BROADCAST_SEND_PORT);
    pClientAddr.sin_addr.s_addr    = inet_addr( "255.255.255.255" );
    iRet = sendto( iSocket, szBuffer, len, 0, ( struct sockaddr* )&pClientAddr, sizeof( struct sockaddr_in ) );
    printf( "iRet=%d port %d\n", iRet, backupportjpeg );
}

void BcastProcMessageJpeg( int iSocket, const char* pszBuffer, int iBufferSize )
{
    int		len;
    BCASTJPEG	bcast;
    int		nSessionID = 0;
    memcpy( &bcast, pszBuffer, sizeof( BCASTJPEG ) );

    if ( bcast.startcode != START_CODE_JPEG )
    {
        //printf("start code error:%x\n",bcast.startcode);
        return;
    }

    len = ntohl( bcast.len );
    bcast.len = len;

    //printf("cmd:%x len %d\n",bcast.cmd,bcast.len);
    switch ( bcast.cmd )
    {
        case GET_DEVICE_JPEG:
            GetSystemConfigJpeg( iSocket, nSessionID, GET_DEVICE_JPEG_RESPONSE );
            //printf("get device\n");
            break;

        case SET_DEVICE_JPEG:
            ConfigSystemJpeg( iSocket, pszBuffer, 0, nSessionID );
            printf( "set device\n" );
            break;

        default:
            break;
    }
}

void* SearchProcJpeg( void* p )
{
    char                    szBuffer[1400];
    struct sockaddr_in     	pClientAddr;
    char			i;
    int			iBufferSize;
    //memcpy(netmac,bparam.stIEBaseParam.szMac,6);
    sleep( 15 );

    while ( 1 )
    {
        memset( &pClientAddr, 0, sizeof( pClientAddr ) );
        memset( szBuffer, 0, 1024 );
        iBufferSize = recvfrom( iSocketjpeg, szBuffer, 1024, 0, ( struct sockaddr* )&pClientAddr, &iAddrLenjpeg );
        //printf("board recive %d port %d\n",iBufferSize,pClientAddr.sin_port);
#if 0

        for ( i = 0; i < iBufferSize; i++ )
        {
            printf( "%02x-", szBuffer[i] );

            if ( ( i != 0x00 ) && ( ( i % 16 ) == 0 ) )
            {
                printf( "\n" );
            }
        }

        printf( "\n" );
#endif

        if ( iBufferSize > 0 )
        {
            backupportjpeg = pClientAddr.sin_port;
            //printf("ip %x port %d\n",pClientAddr.sin_addr.s_addr,backupportjpeg);
            BcastProcMessageJpeg( iSocketjpeg, szBuffer, iBufferSize );
            //printf("BcastProc\n");
        }
    }
}

void SearchStopJpeg( void )
{
    shutdown( iSocketjpeg, 2 );
    close( iSocketjpeg );
}

void SearchInitJpeg( void )
{
    const int               routenum = 255;
    const int               loopback = 1;
    struct ip_mreq          mreq;
    struct sockaddr_in      servaddr;
    int			boardcast = 1;
    int			nReuseAddr = 1;
    int			iRet;
    iSocketjpeg = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( iSocketjpeg < 0 )
    {
        printf( "create socket error!\n" );
        return;
    }

    iRet = setsockopt( iSocketjpeg, SOL_SOCKET, SO_REUSEADDR, ( void* )&nReuseAddr, sizeof( int ) );

    if ( iRet == -1 )
    {
        printf( "setsockopt broadcast is failed\n" );
        shutdown( iSocketjpeg, 2 );
        close( iSocketjpeg );

		/* BEGIN: Modified by wupm, 2013/7/1 */
        //return -1;
        return;
    }

    iRet = setsockopt( iSocketjpeg, SOL_SOCKET, SO_BROADCAST, ( const char* )&boardcast, sizeof( boardcast ) );

    if ( iRet == -1 )
    {
        printf( "setsockopt broadcast is failed\n" );
        shutdown( iSocketjpeg, 2 );
        close( iSocketjpeg );

		/* BEGIN: Modified by wupm, 2013/7/1 */
        //return -1;
        return;
    }

    memset( &servaddr, 0, sizeof( servaddr ) );
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( BROADCAST_RECV_PORT_JPEG );
    servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
    iRet = bind( iSocketjpeg, ( struct sockaddr* )&servaddr, sizeof( struct sockaddr_in ) );

    if ( iRet == -1 )
    {
        printf( "bind() errno %d\n", errno );
        shutdown( iSocketjpeg, 2 );
        close( iSocketjpeg );
        return;
    }

    iAddrLenjpeg = sizeof( struct sockaddr );
}

