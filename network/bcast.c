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

#define BROADCAST_RECV_PORT             	8600

#define MESSAGE_VERSION                 		0x10

#define GET_DEVICE           				0x0101
#define GET_DEVICE_RESPONSE          		0x0801

#define SET_DEVICE           				0x0102
#define SET_DEVICE_RESPONSE          		0x0802

#define GET_FACTORY           				0x0103
#define GET_FACTORY_RESPONSE          	0x0803


#define START_CODE						0x4844


unsigned char  			netmac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int 						iSocket = -1;
int 						RouteSocket = -1;
pthread_t               		bcasthread;
int                     			iRet, iBufferSize;
int                     			iAddrLen;
unsigned short			packindex = 0;
unsigned short          		backupport = 0;
int						backupip = 0;

typedef struct _stMessageHead
{
    unsigned char       byVersion;
    unsigned char       byHeadLen;
    unsigned short      nType;

    unsigned short      nSessionID;
    unsigned short      nMessageLen;

} MESSAGE_HEAD, *PMESSAGE_HEAD;

struct   sockaddr_in   	srv;

int GetIPAddress( unsigned int* pIpAddress )
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_in* sin;
    fd = socket( PF_INET, SOCK_DGRAM, 0 );
    memset( &ifr, 0, sizeof( ifr ) );
    strcpy( ifr.ifr_name, "eth0" );
    ioctl( fd, SIOCGIFADDR, &ifr );
    close( fd );
    sin = ( struct sockaddr_in* )&ifr.ifr_addr;
    *pIpAddress = sin->sin_addr.s_addr;
    return 0;
}

void GetSystemConfig( int iSocket, unsigned short nSessionID, unsigned short type )
{
    BCASTPARAM    		bcastparam;
    BCASTDDNS			ddnsparam;
    char            			szBuffer[1400];
    int             				iBufferSize;
    unsigned short  			nResult;
    struct sockaddr_in 		pClientAddr;
    int					ipaddr;
    int					iRet;
    BCASTPROTOCOL   	bcast;
    memset( &bcast, 0x00, sizeof( BCASTPROTOCOL ) );
    bcast.startcode = START_CODE;
    bcast.cmd = GET_DEVICE_RESPONSE;
    bcast.bcast.dhcp = ( bparam.stNetParam.byIsDhcp > 0 ) ? 1 : 0 ;
    memcpy( bcast.bcast.szIpAddr, bparam.stNetParam.szIpAddr, 16 );
    memcpy( bcast.bcast.szMask, bparam.stNetParam.szMask, 16 );
    memcpy( bcast.bcast.szGateway, bparam.stNetParam.szGateway, 16 );
    memcpy( bcast.bcast.szDns1, bparam.stNetParam.szDns1, 16 );
    memcpy( bcast.bcast.szDns2, bparam.stNetParam.szDns2, 16 );
    bcast.bcast.nPort = bparam.stNetParam.nPort;
    strcpy( bcast.bcast.szDevName, bparam.stIEBaseParam.szDevName );
	strcpy( bcast.bcast.dwDeviceID, bparam.stIEBaseParam.dwDeviceID );
	/* add begin by yiqing, 2016-05-17*/
	strcpy( bcast.bcast.dwApiLisense, bparam.stIEBaseParam.dwApiLisense );

	
    int ver_value;
    char temp[64] = {0};
    ver_value = bparam.stIEBaseParam.sys_ver;
    sprintf( temp, "%d.%d.%d.%d", ( ver_value >> 24 ) & 0xff, ( ver_value >> 16 ) & 0xff, ( ver_value >> 8 ) & 0xff, ver_value & 0xff );
    strcpy( bcast.bcast.sysver, temp );
    //printf("szDevName:%s\n",stParamList.szDevName);
    bcast.bcast.szMacAddr[0] = netmac[0];
    bcast.bcast.szMacAddr[1] = netmac[1];
    bcast.bcast.szMacAddr[2] = netmac[2];
    bcast.bcast.szMacAddr[3] = netmac[3];
    bcast.bcast.szMacAddr[4] = netmac[4];
    bcast.bcast.szMacAddr[5] = netmac[5];
    memcpy( &ddnsparam, &bparam.stDdnsParam, sizeof( DDNSPARAM ) );
    memcpy( szBuffer, &bcast, sizeof( BCASTPROTOCOL ) );
    memcpy( szBuffer + sizeof( BCASTPROTOCOL ), &ddnsparam, sizeof( DDNSPARAM ) );
    pClientAddr.sin_family         		= AF_INET;
    pClientAddr.sin_port           		= backupport;//htons(BROADCAST_SEND_PORT);
    pClientAddr.sin_addr.s_addr    	= backupip;
    iRet = sendto( iSocket, szBuffer, sizeof( BCASTPROTOCOL ) + sizeof( DDNSPARAM ), 0, ( struct sockaddr* )&pClientAddr, sizeof( struct sockaddr_in ) );
    pClientAddr.sin_family         		= AF_INET;
    pClientAddr.sin_port           		= backupport;//htons(BROADCAST_SEND_PORT);
    pClientAddr.sin_addr.s_addr    	= inet_addr( "255.255.255.255" );
    iRet = sendto( iSocket, szBuffer, sizeof( BCASTPROTOCOL ) + sizeof( DDNSPARAM ), 0, ( struct sockaddr* )&pClientAddr, sizeof( struct sockaddr_in ) );
    //printf("iRet=%d\n",iRet);
}

int ConfigSystem( int iSocket, const char* pszMessage, unsigned short nMessageLen, unsigned short nSessionID )
{
    BCASTPROTOCOL    	curbcast, remotebcast;
    char            	szBuffer[1400];
    struct sockaddr_in     	pClientAddr;
    int			iRet = 0;
    int			result;
    char			i;
    memcpy( &remotebcast, pszMessage, sizeof( BCASTPROTOCOL ) );

    //check mac
    for ( i = 0; i < 6; i++ )
    {
        if ( remotebcast.bcast.szMacAddr[i] != netmac[i] )
        {
#if 1
            printf( "remote mac is error:%02x-%02x-%02x-%02x-%02x-%02x\n", remotebcast.bcast.szMacAddr[0],
                    remotebcast.bcast.szMacAddr[1], remotebcast.bcast.szMacAddr[2], remotebcast.bcast.szMacAddr[3],
                    remotebcast.bcast.szMacAddr[4], remotebcast.bcast.szMacAddr[5] );
#endif

			/* BEGIN: Modified by wupm, 2013/7/1 */
            //return;
            return 0;
        }
    }

    printf( "user:%s pwd:%s\n", remotebcast.bcast.szUserName, remotebcast.bcast.szPassword );
    //check user or passwd
    iRet = GetUserPri( remotebcast.bcast.szUserName, remotebcast.bcast.szPassword );

    if ( iRet == 255 )
    {
        result = 0x8000;
    }

    else if ( iRet == 0 )
    {
        result = 0x8001;
    }

    else
    {
        result = 0x8002;
    }

    printf( "iRet %d\n", iRet );

    if ( iRet == 255 )
    {
        memcpy( &curbcast, &remotebcast, sizeof( BCASTPROTOCOL ) );
        memcpy( bparam.stNetParam.szIpAddr, remotebcast.bcast.szIpAddr, 16 );
        memcpy( bparam.stNetParam.szMask, remotebcast.bcast.szMask, 16 );
        memcpy( bparam.stNetParam.szGateway, remotebcast.bcast.szGateway, 16 );
        memcpy( bparam.stNetParam.szDns1, remotebcast.bcast.szDns1, 16 );
        memcpy( bparam.stNetParam.szDns2, remotebcast.bcast.szDns2, 16 );
        bparam.stNetParam.nPort   = remotebcast.bcast.nPort;
        bparam.stNetParam.byIsDhcp = remotebcast.bcast.dhcp;
        printf( "set ok\n" );
    }

    remotebcast.startcode = START_CODE;
    remotebcast.cmd       = SET_DEVICE_RESPONSE;
    memcpy( szBuffer, &remotebcast, sizeof( BCASTPROTOCOL ) );
    memcpy( szBuffer + sizeof( BCASTPROTOCOL ), &result, sizeof( int ) );
    printf( "%02x-%02x-%02x-%02x\n", szBuffer[276], szBuffer[277], szBuffer[278], szBuffer[279] );
    pClientAddr.sin_family         		= AF_INET;
    pClientAddr.sin_port           		= backupport;
    pClientAddr.sin_addr.s_addr    	= inet_addr( "255.255.255.255" );
    iRet = sendto( iSocket, szBuffer, sizeof( BCASTPROTOCOL ) + sizeof( int ), 0, ( struct sockaddr* )&pClientAddr, sizeof( struct sockaddr_in ) );
    printf( "config system %d\n", iRet );
    return result;
}

void BcastProcMessage( int iSocket, const char* pszBuffer, int iBufferSize )
{
	/* BEGIN: Modified by wupm, 2013/7/1 */
    //unsigned short 	nMessageLen, nType, nSessionID;
	unsigned short 	nMessageLen = 0, nType = 0, nSessionID = 0;

    const char*      pszMessage = NULL;
    int		i;
    BCASTPROTOCOL	bcast;
    memcpy( &bcast, pszBuffer, sizeof( BCASTPROTOCOL ) );

    if ( bcast.startcode != START_CODE )
    {
        //printf("start code error:%x\n",bcast.startcode);
        return;
    }

    //printf("cmd:%x\n",bcast.cmd);
    switch ( bcast.cmd )
    {
        case GET_DEVICE:
            GetSystemConfig( iSocket, nSessionID, GET_DEVICE_RESPONSE );
            //printf("get device\n");
            break;

        case SET_DEVICE:
        {
            int iRet;
            iRet = ConfigSystem( iSocket, pszBuffer, nMessageLen, nSessionID );

            if ( iRet == 0x8000 )
            {
                SaveSystemParam( &bparam );
                SetRebootCgi();
            }
        }
        break;

        case GET_FACTORY:
            break;

        default:
            break;
    }
}


unsigned char GetMacValue( unsigned char temp )
{
    unsigned char temp2 = 0;

    if ( ( temp >= 0x30 ) && ( temp <= 0x39 ) )
    {
        temp2 = temp - 0x30;
        //printf("temp20:%x\n",temp2);
    }

    else if ( ( temp >= 0x41 ) && ( temp <= 0x46 ) )
    {
        temp2 = temp - 0x37;
        //printf("temp21:%x\n",temp2);
    }

    else if ( ( temp >= 0x61 ) && ( temp <= 0x66 ) )
    {
        temp2 = temp - 0x57;
        //printf("temp22:%x\n",temp2);
    }

    return temp2;
}

void GetMacString( void )
{
    unsigned char temp[12];
    unsigned char temp1;
    unsigned char	temp2;
    char		i, j;
    //scanning mac
    j = 0;

    for ( i = 0; i < 17; i++ )
    {
        temp1 = bparam.stIEBaseParam.szMac[i];

        if ( temp1 == 0x3a ) 		//0x3a->:
        {
            continue;
        }

        temp[j] = temp1;
        j++;
    }

    //get mac
    printf( "=======mac=======\n" );

    for ( i = 0; i < 6; i++ )
    {
        temp1 = GetMacValue( temp[i * 2] );
        temp2 = GetMacValue( temp[i * 2 + 1] );
        //printf("temp:%x-%x\n",temp[i*2],temp[i*2+1]);
        //printf("%x-%x\n",temp1,temp2);
        netmac[i] = ( temp1 << 4 ) | temp2;
        printf( "%02x-", netmac[i] );
    }

    printf( "\n" );
}

int GetListenPort( void )
{
    unsigned short	p2pport = 0;
    unsigned char		value;
    p2pport = 12345 + netmac[0] + netmac[1] + netmac[2] + netmac[3] + netmac[4] + netmac[5];
    p2pport += rand() & 0xff;
    bupnpp2pport = upnpp2pport;
    upnpp2pport = p2pport;
    NoteP2pPort();
    return p2pport;
}

void* SearchProc( void* p )
{
    char                    szBuffer[1400];
    struct sockaddr_in     	pClientAddr;
#if 0
    memcpy( netmac, bparam.stIEBaseParam.szMac, 6 );
#else
    //GetMacString();
#endif
    sleep( 15 );

    while ( 1 )
    {
        memset( &pClientAddr, 0, sizeof( pClientAddr ) );
        memset( szBuffer, 0, sizeof( szBuffer ) );
        iBufferSize = recvfrom( iSocket, szBuffer, sizeof( szBuffer ), 0, ( struct sockaddr* )&pClientAddr, &iAddrLen );

        //printf("board recive %d\n",iBufferSize);
        if ( iBufferSize > 0 )
        {
            backupport = pClientAddr.sin_port;
            backupip      = pClientAddr.sin_addr.s_addr;
            Textout("ip %x port %d",pClientAddr.sin_addr.s_addr,backupport);
            BcastProcMessage( iSocket, szBuffer, iBufferSize );
        }
    }
}

void SearchStop( void )
{
    shutdown( iSocket, 2 );
    close( iSocket );
}

void SearchInit( void )
{
    const int               routenum = 255;
    const int               loopback = 1;
    struct ip_mreq          mreq;
    struct sockaddr_in      servaddr;
    int			boardcast = 1;
    int			nReuseAddr = 1;
    iSocket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( iSocket < 0 )
    {
        printf( "create socket error!\n" );
        return;
    }

    iRet = setsockopt( iSocket, SOL_SOCKET, SO_REUSEADDR, ( void* )&nReuseAddr, sizeof( int ) );

    if ( iRet == -1 )
    {
        printf( "setsockopt broadcast is failed\n" );
        shutdown( iSocket, 2 );
        close( iSocket );

		/* BEGIN: Modified by wupm, 2013/7/1 */
        //return -1;
        return;
    }

    iRet = setsockopt( iSocket, SOL_SOCKET, SO_BROADCAST, ( const char* )&boardcast, sizeof( boardcast ) );

    if ( iRet == -1 )
    {
        printf( "setsockopt broadcast is failed\n" );
        shutdown( iSocket, 2 );
        close( iSocket );

		/* BEGIN: Modified by wupm, 2013/7/1 */
        //return -1;
        return;
    }

    memset( &servaddr, 0, sizeof( servaddr ) );
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( BROADCAST_RECV_PORT );
    servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
    iRet = bind( iSocket, ( struct sockaddr* )&servaddr, sizeof( struct sockaddr_in ) );

    if ( iRet == -1 )
    {
        printf( "bind() errno %d\n", errno );
        shutdown( iSocket, 2 );
        close( iSocket );
        return;
    }

    iAddrLen = sizeof( struct sockaddr );
}

void SearchThread( void )
{
	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/7/10 */
//    pthread_create( &bcasthread, 0, &SearchProc, NULL );
}
