#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "yinetwork.h"
#include "param.h"
#include "baAudioPlay.h"
int yisocketnet 	     = -1;
unsigned short  clientport = 0;
int				clientip   = 0;

extern void OnDoorOpenEx();
extern void SaveNewUUID(char *szNewUUID);
extern void SetRebootCgi( void );
extern void StartAudioPlay(unsigned int index, unsigned int count, BaAudioPlayCallback callback);
extern void BaAudioWifiConfigCallback(void);
int LockControl(char locknum,char value)
{

	Textout("locknum:%d,value:%d",locknum,value);

	OnDoorOpenEx();
#ifdef NEW_BRAOD_AES
	OnDoor2OpenEx();
#endif
	char buf[1] = {0x01};
	NotifyToClient(eLockControl,eWrite,buf,1);

	return 0;
}

int SetupDeviceID(char *deviceId)
{
	StartAudioPlay( WF_CONFIGOK, 1, BaAudioWifiConfigCallback);
	SaveNewUUID(deviceId);
	char buf[1] = {0x01};
	NotifyToClient(eSetDeviceId,eWrite,buf,1);
	sleep(2);
	SetRebootCgi();
	return 0;
}

int Setupwifi(char *data)
{
 	int  iRet = 0;
	char temp[128];
	
	memset( temp, 0x00, 128 );
	
    printf( "wifi data: %s\n", data );
    iRet = GetStrParamValue( data, "ssid", temp, 63 ); //bparam.stWifiParam.szSSID);

    if ( iRet == 0x00 )
    {
        memset( bparam.stWifiParam.szSSID, 0x00, 64 );
        strcpy( bparam.stWifiParam.szSSID, temp );
    }
	iRet = GetStrParamValue( data, "pwd", temp, 63 ); //bparam.stWifiParam.szSSID);

    if ( iRet == 0x00 )
    {
        memset( bparam.stWifiParam.szShareKey, 0x00, 64 );
        strcpy( bparam.stWifiParam.szShareKey, temp );
    }
	NoteSaveSem();
	StartAudioPlay( WF_CONFIGOK, 1, BaAudioWifiConfigCallback);
}
int YiCloseSocket( int iSocket )		// close socket
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

int YINetSocketInit( void )
{
    //create socket
    int iRet;
    struct sockaddr_in addr;
	int 		nReuseAddr = 1;
	int 	nFlag = 1;

    yisocketnet = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( yisocketnet == -1 )
    {
        printf( "create IPC socket failure\n" );
        return -1;
    }
	/*
	setsockopt( yisocketnet, IPPROTO_TCP, TCP_NODELAY, ( void* )&nFlag, sizeof( int ) );
	if ( iRet == -1 )
	{
		printf( "setsockopt TCP_NODELAY is failed\n" );
		shutdown( yisocketnet, 2 );
		close( yisocketnet );
		return -1;
	}
	*/
	iRet = setsockopt( yisocketnet, SOL_SOCKET, SO_REUSEADDR, ( void* )&nReuseAddr, sizeof( int ) );
	
	if ( iRet == -1 )
	{
		printf( "setsockopt SO_REUSEADDR is failed\n" );
		shutdown( yisocketnet, 2 );
		close( yisocketnet );
		return -1;
	}
	//setsockopt( yisocketnet, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout1, sizeof( struct timeval ) );

    //build connection address
    addr.sin_family = AF_INET;
    addr.sin_port = htons( YI_RECV_PORT );
    addr.sin_addr.s_addr = htonl( INADDR_ANY );
    iRet = bind( yisocketnet, ( struct sockaddr* )&addr, sizeof( addr ) );

    if ( iRet < 0 )
    {
        printf( "bind failured" );
        YiCloseSocket( yisocketnet );
        return -1;
    }

    return yisocketnet;
}

int YINetRead( char* param, int len )
{
    int iRet=-1;
    struct sockaddr_in 	addr;
    socklen_t addrLen=sizeof( struct sockaddr_in );;
    iRet = recvfrom( yisocketnet, param, len, 0, ( struct sockaddr* )&addr, &addrLen );
    if ( iRet <= 0 )
    {
        return -1;
    }
	clientport    = addr.sin_port;
    clientip      = addr.sin_addr.s_addr;
    return iRet;
}

int NotifyToClient( eYiCmd cmd, eYiNetWorkSubCmd subcmd, char* pdata, int len)
{
	printf("NotifyToClient\n");
    int 					iRet;
    struct sockaddr_in 	addr;
    
    YIPARAMCMD      param;
    
    param.startcode = STARTCODE;
    param.cmd 		= cmd;
    param.subcmd    = subcmd;
    param.datalen   = len;

    if(len>0)
    {
		memcpy(param.buffer, pdata, len);
	}
		
        
    addr.sin_family 		= AF_INET;
    addr.sin_port 			= clientport;
    addr.sin_addr.s_addr 	= clientip;
    iRet = sendto( yisocketnet, &param, sizeof(YIPARAMCMD), 0, ( struct sockaddr* )&addr, sizeof( addr ) );
    return iRet;
}

void* YINetThreadproc( void* p )
{
    int iRet = 0;
    YIPARAMCMD	param;
    
    while ( 1 )
    {
        iRet = YINetRead( (char*)&param, sizeof( YIPARAMCMD ));

        if ( ( iRet <= 0x00 ) || ( param.startcode != STARTCODE ) )
        {
            usleep( 20*1000 );
            continue;
        }

        switch ( param.cmd )
        {
            case eGetLink:		//send bparam
            case eEthLink:	//network
            case eWifiLink:	//wireless
                break;
            case eReadParam:
                break;
            case eWriteParam:
                break;
            case eToApMode:
            case eToStaMode:
            case eToStaReconn:
                break;
            case eSmartConnection:
                //if( param.subcmd == eOK ) StartAudioPlay(WF_CONFIGOK, 1, NULL);
                //else if(param.subcmd == eFail ) StartAudioPlay(WF_CONFIGFAIL, 1, NULL);

                break;
            case eWifiScan:
                
                break;
            case eFactoryParam:

                break;
            case eFactoryDDNS:
                
                break;
            case eOnline:
                
                break;
            case eWifiStatus:
                break;
            case eUsers:

                break;
            case eRootUsers:
                break;

			case eLockControl:
				LockControl(param.buffer[0],param.buffer[1]);
				break;

			case eSetDeviceId:
				SetupDeviceID(param.buffer);
				break;

			case eSetWifi:
				Setupwifi(param.buffer);
				break;

			case eEnabneIr:
				Textout("eEnabneIr");
				EnableIR(0);
				break;

			case eSetAutoRebootMins:
				SetAutoRbootMinsValue(param.buffer[0],param.buffer[1]);
				break;
            default:
                Textout( "cmd isn't support=%x\n", param.cmd );
                break;
        }
    }
}

void YiNetWorkInit( void )
{
    printf("ojtp2p ipc init, port=%d\n",YI_RECV_PORT);
    system( "ifconfig lo 127.0.0.1" );
    YINetSocketInit();
}

void YiNetWorkThread( void )
{
    pthread_t       	netthread;

	YiNetWorkInit();
    pthread_create( &netthread, 0, YINetThreadproc, NULL );
}






