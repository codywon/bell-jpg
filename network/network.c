
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#include "vbuf.h"
#include "network.h"
#include "param.h"
#include "debug.h"

/* Use in modeSta/Ap.c */
WifiCont WifiResult[MAX_WIFI_ITEM_COUNT];
char     wificnt = 0;
char	 wifiok = 0x00;

void WifiResultInit( void )
{
    memset( &WifiResult, 0x00, sizeof( WifiCont ) * 32 );
}

int GetWifiScan()
{
    if(geWifiMode == eStaMode) StaModeGetWifiScan();
    else ApModeGetWifiScan();
}

/* Use in modeSta/Ap.c */

eWifiMode geWifiMode = eStaMode;

pthread_mutex_t wifimutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cmdmutex = PTHREAD_MUTEX_INITIALIZER;

char				netok    = -1;
int				externwifistatus = -1;

void wifilock( void )
{
    pthread_mutex_lock( &wifimutex );
}

void wifiunlock( void )
{
    pthread_mutex_unlock( &wifimutex );
}

void cmdlock( void )
{
    pthread_mutex_lock( &cmdmutex );
}

void cmdunlock( void )
{
    pthread_mutex_unlock( &cmdmutex );
}


int CheckIPok( char* ipaddr )
{
    int             iRet;
    struct in_addr  inp;
    iRet = inet_aton( ipaddr, &inp );

    if ( iRet )
    {
        return 0;
    }

    return -1;
}

int DoSystem( char* pcmd )
{
    cmdlock();
    CmdWrite( pcmd, strlen( pcmd ) );
    cmdunlock();
	return 0;
}

int CloseSocket( int iSocket )		// close socket
{
    int 	iRet = -1;

    if ( iSocket == -1 )
    {
        return iRet;
    }

    shutdown( iSocket, 2 );
    close( iSocket );
    return 0;
}

int InitSocket( unsigned char byIsServer, unsigned char byIsTcp, struct sockaddr_in* saddr )
{
    int		iSocket;
    int		iRet = -1;
    int		iBufSize;
    int 		type;
    int		protocol = 0;
    int 		nReuseAddr = 1;
    int		nFlag = 1;
    socklen_t 	size;

    size = sizeof( struct sockaddr_in );

    if ( byIsTcp )
    {
        type 		= SOCK_STREAM;
        protocol 	= IPPROTO_TCP;
    }

    else
    {
        type 		= SOCK_DGRAM;
        protocol 	= IPPROTO_UDP;
    }

    iSocket = socket( PF_INET, type, protocol );

    if ( iSocket == -1 )
    {
        return iRet;
    }

#if 0
    iBufSize = 32768;
    setsockopt( iSocket, SOL_SOCKET, SO_RCVBUF, ( void* )&iBufSize, sizeof( int ) );
    setsockopt( iSocket, SOL_SOCKET, SO_SNDBUF, ( void* )&iBufSize, sizeof( int ) );
#endif
    setsockopt( iSocket, SOL_SOCKET, SO_REUSEADDR, ( void* )&nReuseAddr, sizeof( int ) );
    setsockopt( iSocket, IPPROTO_TCP, TCP_NODELAY, ( char* )&nFlag, sizeof( int ) );

    if ( byIsServer )
    {
        if ( bind( iSocket, ( struct sockaddr* )saddr, size ) == -1 )
        {
            CloseSocket( iSocket );
            return iRet;
        }
    }

    return iSocket;
}

int GetExternIp3( char* pipaddr ) 		//get extern ipaddr of ipcam
{
    struct 	sockaddr_in daddr;
    char	ipaddr[16];
    char	ipaddr1[16];
    int 	sock;
    int 	iRet;
    char 	szBuffer[1024] = {"GET /ic.asp HTTP/1.1\r\nHost:iframe.ip138.com\r\nConnection:Close\r\n\r\n"};
    char*	psrc1 = NULL;
    char*	psrc2 = NULL;
    struct timeval          TimeOut;
    char	times = 0;
    int	nFlag;

    sock = socket( AF_INET, SOCK_STREAM, 0 );
    bzero( &daddr, sizeof( struct sockaddr_in ) );
    memset( ipaddr, 0x00, 16 );

    nFlag = 1;
    setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, ( void* )&nFlag, sizeof( int ) );
    TimeOut.tv_sec = 30;
    TimeOut.tv_usec = 0;
    setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, ( char* )&TimeOut, sizeof( TimeOut ) );

    memset( ipaddr, 0x00, 16 );
    iRet = GetDnsIp( "iframe.ip138.com", ipaddr );

    if ( iRet != 0 )
    {
        CloseSocket( sock );
        return -1;
    }

    Textout( "get ipaddr:%s\n", ipaddr );

    daddr.sin_family        = AF_INET;
    daddr.sin_port          = htons( 80 );
    daddr.sin_addr.s_addr   = inet_addr( ipaddr );

    if ( connect( sock, ( struct sockaddr* )&daddr, sizeof( struct sockaddr ) ) == -1 )
    {
        printf( "connect failed\n" );
        CloseSocket( sock );
        return -1;
    }

    send( sock, szBuffer, strlen( szBuffer ), 0 );

    while ( 1 )
    {
        times++;

        if ( times > 3 )
        {
            return -1;
        }

        recv( sock, szBuffer, 1024, 0 );
        printf( "===%s===\n", szBuffer );
        psrc1 = strstr( szBuffer, "[" );
        psrc2 = strstr( szBuffer, "]" );

        if ( ( psrc1 == NULL ) || ( psrc2 == NULL ) )
        {
            printf( "this is NULL\n" );
            continue;
        }

        else
        {
            break;
        }
    }

    //printf("len %d\n",psrc2-psrc1);
    memcpy( pipaddr, psrc1 + 1, psrc2 - psrc1 - 1 );
    printf( "local ip:%d=%s\n", psrc2 - psrc1, pipaddr );
    CloseSocket( sock );
    return 0x00;
}

int GetExternIp1( char* pipaddr ) 		//get extern ipaddr of ipcam
{
    struct 	sockaddr_in daddr;
    char	ipaddr[16];
    char	ipaddr1[16];
    int 	sock;
    int 	iRet;
    char 	szBuffer[1024] = {"GET /ip2city.asp HTTP/1.1\r\nHost:www.ip138.com\r\nConnection:Close\r\n\r\n"};
    char*	psrc1 = NULL;
    char*	psrc2 = NULL;
    struct timeval          TimeOut;
    char	times = 0;
    int	nFlag;

    sock = socket( AF_INET, SOCK_STREAM, 0 );
    bzero( &daddr, sizeof( struct sockaddr_in ) );
    memset( ipaddr, 0x00, 16 );

    nFlag = 1;
    setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, ( void* )&nFlag, sizeof( int ) );
    TimeOut.tv_sec = 30;
    TimeOut.tv_usec = 0;
    setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, ( char* )&TimeOut, sizeof( TimeOut ) );

    memset( ipaddr, 0x00, 16 );
    iRet = GetDnsIp( "www.ip138.com", ipaddr );

    if ( iRet != 0 )
    {
        CloseSocket( sock );
        return -1;
    }

    //printf("get ipaddr:%s\n",ipaddr);

    daddr.sin_family        = AF_INET;
    daddr.sin_port          = htons( 80 );
    daddr.sin_addr.s_addr   = inet_addr( ipaddr );

    if ( connect( sock, ( struct sockaddr* )&daddr, sizeof( struct sockaddr ) ) == -1 )
    {
        printf( "connect failed\n" );
        CloseSocket( sock );
        return -1;
    }

    send( sock, szBuffer, strlen( szBuffer ), 0 );

    while ( 1 )
    {
        times++;

        if ( times > 3 )
        {
            return -1;
        }

        recv( sock, szBuffer, 1024, 0 );
        //printf("===%s===\n",szBuffer);
        psrc1 = strstr( szBuffer, "[" );
        psrc2 = strstr( szBuffer, "]" );

        if ( ( psrc1 == NULL ) || ( psrc2 == NULL ) )
        {
            //printf("this is NULL\n");
            continue;
        }

        else
        {
            break;
        }
    }

    memcpy( pipaddr, psrc1 + 1, psrc2 - psrc1 - 1 );
    CloseSocket( sock );
    return 0x00;
}


BOOL GetIpByHostName( const char* lpszHost, char* lpszIp )
{
    if ( ( NULL == lpszHost ) || ( NULL == lpszIp ) )
    {
		Textout("1");
        return FALSE;
    }

    struct hostent* pHostInfo = NULL;

    char* lpszTemp = NULL;

    pHostInfo = gethostbyname( lpszHost );

    if ( NULL == pHostInfo )
    {
		Textout("2");
        return FALSE;
    }

    lpszTemp = inet_ntoa( *( struct in_addr* ) * ( pHostInfo->h_addr_list ) );

    if ( NULL == lpszTemp )
    {
		Textout("3");
        return FALSE;
    }

	Textout("4, lpszTemp = [%s]", lpszTemp);
    //strncpy( lpszIp, lpszTemp, MAX_PATH - 1 );
    strcpy( lpszIp, lpszTemp);
    return TRUE;
}

int GetDnsIp( char* dnsname, char* ipaddr ) // get ipaddr of name
{
    struct hostent* ht;
    struct in_addr in;
    struct sockaddr_in local_addr;
    dnslock();

    if ( ( ht = gethostbyname( dnsname ) ) == NULL )
    {
        Textout("cann't get dns=[%s] ipaddr", dnsname);
        dnsunlock();
        return -1;
    }

    dnsunlock();
    memcpy( &local_addr.sin_addr.s_addr, ht->h_addr, 4 );
    in.s_addr = local_addr.sin_addr.s_addr;
    sprintf( ipaddr, "%s", inet_ntoa( in ) );
    return 0;
}


#define NdisMediaStateConnected                 		1
#define NdisMediaStateDisconnected              		0
#define OID_GEN_MEDIA_CONNECT_STATUS                0x060B
#define	OID_802_11_SSID								0x0509

#define RT_PRIV_IOCTL                   				(SIOCIWFIRSTPRIV + 0x01)

typedef struct _NDIS_802_11_SSID {
	unsigned int SsidLength;	/* length of SSID field below, in bytes; */
	/* this can be zero. */
	unsigned char Ssid[32];	/* SSID information field */
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;


int OidQueryInformation( unsigned long OidQueryCode, int socket_id, char* DeviceName, void* ptr, unsigned long PtrLength )
{
    struct iwreq wrq;

    strcpy( wrq.ifr_name, DeviceName );
    wrq.u.data.length = PtrLength;
    wrq.u.data.pointer = ( caddr_t ) ptr;
    wrq.u.data.flags = OidQueryCode;

    return ( ioctl( socket_id, RT_PRIV_IOCTL, &wrq ) );
}

int StaModeGetWifiStatus( )
{
    int                 s = 0;
    unsigned int		ConnectStatus = NdisMediaStateDisconnected;

	if ( bparam.stWifiParam.byEnable == 0 )
	{
		externwifistatus = 0x01;
		return 0;
	}

    s = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( OidQueryInformation( OID_GEN_MEDIA_CONNECT_STATUS, s, "ra0", &ConnectStatus, sizeof( ConnectStatus ) ) < 0 )
    {
        printf( "========Query OID_GEN_MEDIA_CONNECT_STATUS failed!========\n" );
        close( s );
        return -1;
    }

    close( s );
    Textout( "OID_GEN_MEDIA_CONNECT_STATUS:%d(Succ=1,Fail=0)", ConnectStatus );

    if ( ConnectStatus == NdisMediaStateConnected )
    {
        //Succ
        externwifistatus = 0x00;
    }

    else
    {
        //Fail
        externwifistatus = 0x01;
    }

    return externwifistatus;
}

int StaModeGetWifiSSID( PNDIS_802_11_SSID ssid)
{
    int                 s = 0;

    s = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( OidQueryInformation( OID_802_11_SSID, s, "ra0", ssid, sizeof( NDIS_802_11_SSID ) ) < 0 )
    {
        printf( "========Query OID_802_11_SSID failed!========\n" );
        close( s );
        return -1;
    }

    close( s );

    Textout("ssid->Ssid=%s", ssid->Ssid);
    return 0;
}


static int bWifiCheckEn = 1;
void SetWifiCheckFunc(int status)
{
    bWifiCheckEn = status;
}


#define WIFITIMEOUT 60
void* WfiCheckProc( void* p )
{
    int iRet = 0;
    char cmd[128];
    unsigned int times = WIFITIMEOUT;
    NDIS_802_11_SSID ssid;
    while ( 1 )
    {
        if(times > 0) 
			times--;
        else if(times == 0)
        {
        
			//printf("netok=%d,geWifiMode=%d,byEnable=%d,bWifiCheckEn=%d\n",netok,geWifiMode,bparam.stWifiParam.byEnable,bWifiCheckEn);
            if(geWifiMode == eApMode)
            {
                times = WIFITIMEOUT*10;
				
				#ifdef BELINK
				ControlIO(BELL_NET_LED, 1);
				#endif
            }
            else if(netok == 0 && geWifiMode == eStaMode && bparam.stWifiParam.byEnable && bWifiCheckEn)
            {
                
                iRet = StaModeGetWifiStatus();
                if(iRet == 1)
                {
                	times = WIFITIMEOUT/10;
                    if(bparam.stWifiParam.szSSID[0] != 0)
                    {
                        sprintf(cmd, "iwpriv ra0 set SSID=\"%s\"", bparam.stWifiParam.szSSID );
                        DoSystem(cmd);
                    }
					
					#ifdef BELINK
					Textout("CB_FLASH_SLOW");
					ControlBellLED(CB_FLASH_SLOW);
					#endif
					
                }
                else 
                {
                	times = WIFITIMEOUT;
                    if(bparam.stWifiParam.szSSID[0] != 0)
                    {
                        StaModeGetWifiSSID(&ssid);
                        if( strcmp(ssid.Ssid, bparam.stWifiParam.szSSID))
                        {
                            Textout("Cur ssid=%s, should be=%s", ssid.Ssid, bparam.stWifiParam.szSSID);
                            IpcStaModeReconn();
                        }
                    }
					#ifdef BELINK
					Textout("CB_CLOSE");
					ControlBellLED(CB_CLOSE);
					#endif
					
                }
				#ifdef BELINK
				ControlIO(BELL_NET_LED, 1);
				#endif
            }
			else if(netok == 1)
			{
				
				#ifdef BELINK
				ControlBellLED(CB_CLOSE);
				ControlIO(BELL_NET_LED, 0);
				#endif
			}
			
        }
        
        sleep(1);
    }
}

void Networkhread( void )
{
    pthread_t netthread1;
    pthread_create( &netthread1, 0, &WfiCheckProc, NULL );
}


