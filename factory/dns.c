#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <semaphore.h>

#include "init.h"
#include "alarm.h"
#include "record.h"
#include "capture.h"
#include "public.h"
#include "param.h"
#include "cmdhead.h"
#include "debug.h"

unsigned char		factorystatus = 0;
unsigned int		factorytimes = 0;

int GetFactoryStatus( void )
{
    return factorystatus;
}

void DnsAlarmInit( char* pdst, char alarmtype )
{
    char temp[32];
	/* BEGIN: Modified by wupm, 2013/6/14 */
    sprintf( pdst, "GET /api/alarm.asp?username=%s&userpwd=%s&rea=%d&io=0&uid=%s HTTP/1.1\r\n"
    		"Host: %s\r\n"
    		"\r\n",
             bparam.stDdnsParam.serveruser,
             bparam.stDdnsParam.serverpwd,
             alarmtype,
             bparam.stIEBaseParam.dwDeviceID,
             bparam.stDdnsParam.serversvr);
}

void DnsAlarmInit_vstar( char* pdst, char alarmtype, char* pStrHome )
{
    char temp[32];
    /* BEGIN: Modified by wupm, 2013/6/14 */
    sprintf( pdst, "GET /api/%s?username=%s&userpwd=%s&rea=%d&io=0&uid=%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "\r\n",
             pStrHome,
             bparam.stDdnsParam.serveruser,
             bparam.stDdnsParam.serverpwd,
             alarmtype,
             bparam.stIEBaseParam.dwDeviceID,
             bparam.stDdnsParam.serversvr );
}

void DnsSendAlarm( char alarmtype )
{
    unsigned int            times = 6000;
    char                    ipaddr[16];
    int                     socket = -1;
    struct sockaddr_in      daddr;
    int                     iRet;
    char                    heatcont[1024];
    printf( "start alarm to server...\n" );
    //get dns param
    memset( ipaddr, 0x00, 16 );
    iRet = GetDnsIp( bparam.stDdnsParam.serversvr, ipaddr );

    if ( iRet != 0 )
    {
        return;
    }

    socket = InitSocket( 0, 1, NULL );
    //send heat param
    bzero( &daddr, sizeof( struct sockaddr_in ) );
    daddr.sin_family        = AF_INET;
    daddr.sin_port          = htons( bparam.stDdnsParam.serverport );
    daddr.sin_addr.s_addr   = inet_addr( ipaddr );

    if ( connect( socket, ( struct sockaddr* )&daddr, sizeof( struct sockaddr ) ) == -1 )
    {
        printf( "connect server is failed\n" );
        CloseSocket( socket );
        return;
    }

    //config heat param
    memset( heatcont, 0x00, 1024 );
    DnsAlarmInit( heatcont, alarmtype );
    //send
    iRet = send( socket, heatcont, strlen( heatcont ), 0 );

    if ( iRet == strlen( heatcont ) )
    {
        printf( "send alarm server=%d\n", iRet );
    }

    else
    {
        printf( "send alarm error iRet=%d\n", iRet );
    }

    //recive
    memset( heatcont, 0x00, 512 );
    iRet = recv( socket, heatcont, 512, 0 );
    CloseSocket( socket );
}

/* BEGIN: Added by Baggio.wu, 2013/7/10 */
static int m_DnsAlarmFlag = 0;
static int m_AlarmType = 0;
void setDnsSendAlarmFlag( int flag, int alarmType )
{
    m_DnsAlarmFlag = flag;
    m_AlarmType = alarmType;
}
void* DnsSendAlarmProc( void* p )
{
    unsigned int            times = 6000;
    char                    ipaddr[16];
    int                     socket = -1;
    struct sockaddr_in      daddr;
    int                     iRet = 0;
    char                    heatcont[1024];
    char alarmtype = 0;
    char strServer[64];
    char strHome[32];


    Textout( "start alarm to DNS server...\n" );

    while ( 1 )
    {
        if ( m_DnsAlarmFlag == 0 )
        {
            sleep( 1 );
            continue;
        }

        m_DnsAlarmFlag = 0;
        alarmtype = ( char )m_AlarmType;

        Textout("Alarm type(1--MOTION, 2--GPIO)=%d", alarmtype);
        //get dns param
        memset( ipaddr, 0x00, 16 );
        #ifdef CHANGE_DDNS_ALARM_SERVER
        /*{
            int i=0;
            if(bparam.stDdnsParam.serversvr[0] == 0)
            {
                Textout("DDNS Server is not set");
                continue;
            }
            int strLen = strlen(bparam.stDdnsParam.serversvr);
            if(strLen == 0 || strLen == 1)
            {
                Textout("DDNS Server is not set");
                continue;
            }
            char* pStr = (char *)bparam.stDdnsParam.serversvr;
            char strServer[64];
            memset(strServer, 0, 64);
            strcpy(strServer, "alarm");
            for(i=0; i<strLen; i++ )
            {
                if(pStr[i] == '.')
                {
                    break;
                }
            }

            strcat(strServer, &pStr[i]);
            Textout("DDNS Server=%s, Alarm Server=%s", bparam.stDdnsParam.serversvr, strServer);
            iRet = GetDnsIp( strServer, ipaddr );
            if ( iRet != 0 )
            {
                Textout("Not found Alarm Server, check DDNS Server");
                iRet = GetDnsIp( bparam.stDdnsParam.serversvr, ipaddr );
            }
        }*/
        {
            int i=0;
            int bUseDnsServer=0;
            if(vstarparam.bEnable)
            {
                bUseDnsServer = 0;
                if(vstarparam.alarm_svr[0] == 0)
                {
                    Textout("vstarparam.alarm_svr[0] == 0");
                    bUseDnsServer = 1;
                }
                else
                {
                    int strLen = strlen(vstarparam.alarm_svr);
                    if(strLen == 0 || strLen == 1)
                    {
                        bUseDnsServer = 1;
                    }
                    else
                    {
                        //alarm.gocam.so/api/alarm.jsp
                        char* pStr = (char*)vstarparam.alarm_svr;
                        memset(strHome, 0, 32);
                        memset(strServer, 0, 64);

                        //find server
                        for(i=0; i<strLen; i++)
                        {
                            if(pStr[i] == '/')
                            {
                                break;
                            }
                        }

                        memcpy(strServer, pStr, i);
                        Textout("Alarm server:%s", strServer);

                        //find home
                        int j=0;
                        for(j=i+1; j<strLen; j++)
                        {
                            if(pStr[j] == '/')
                            {
                                break;
                            }
                        }

                        strcpy(strHome, pStr+j+1);
                        Textout("Alarm server home:%s", strHome);
                        iRet = GetDnsIp( strServer, ipaddr );
                        if(iRet != 0)
                        {
                            bUseDnsServer = 1;
                        }
                    }


                }
            }
            else
            {
                bUseDnsServer = 1;
            }

            if ( bUseDnsServer == 1)
            {
                Textout("Not found Alarm Server, check DDNS Server");
                iRet = GetDnsIp( bparam.stDdnsParam.serversvr, ipaddr );
            }
        }
        #else
        iRet = GetDnsIp( bparam.stDdnsParam.serversvr, ipaddr );
        #endif
        if ( iRet != 0 )
        {
            Textout("Get dns ip fail, server=%s", bparam.stDdnsParam.serversvr);
            continue;
        }

        socket = InitSocket( 0, 1, NULL );
        //send heat param
        bzero( &daddr, sizeof( struct sockaddr_in ) );
        daddr.sin_family        = AF_INET;
        daddr.sin_port          = htons( bparam.stDdnsParam.serverport );
        daddr.sin_addr.s_addr   = inet_addr( ipaddr );

        if ( connect( socket, ( struct sockaddr* )&daddr, sizeof( struct sockaddr ) ) == -1 )
        {
            Textout( "connect server is failed\n" );
            CloseSocket( socket );
            continue;
        }

        //config heat param
        memset( heatcont, 0x00, 1024 );

        #ifdef CHANGE_DDNS_ALARM_SERVER
        DnsAlarmInit_vstar( heatcont, alarmtype, strHome );
        #else
        DnsAlarmInit( heatcont, alarmtype );
        #endif

        Textout("Http Request:%s", heatcont);
        //send
        iRet = send( socket, heatcont, strlen( heatcont ), 0 );

        if ( iRet == strlen( heatcont ) )
        {
            Textout( "send alarm server=%d\n", iRet );
        }

        else
        {
            Textout( "send alarm error iRet=%d\n", iRet );
        }

        //recive
        memset( heatcont, 0x00, 512 );
        iRet = recv( socket, heatcont, 512, 0 );
        CloseSocket( socket );
        socket = 0;
    }
}

int GetDnsServerIp( char* ipaddr )
{
    int iRet = 0;

    switch ( bparam.stDdnsParam.factoryversion )
    {
        case 10:		//vstarcam
            GetDnsIp( bparam.stDdnsParam.serversvr, ipaddr );
            break;

        case 11:		//xinhuaan  viporg
            GetDnsIp( bparam.stDdnsParam.serversvr, ipaddr );
            break;

        case 12:		//xinhuaan 88safe
            GetDnsIp( bparam.stDdnsParam.serversvr, ipaddr );
            break;

        case 13:		//smarteye
            GetDnsIp( bparam.stDdnsParam.serversvr, ipaddr );
            break;

            /* BEGIN: Added by wupm, 2013/3/23 */
        case 21:		//PSD
            GetDnsIp( bparam.stDdnsParam.serversvr, ipaddr );
            Textout1( "PSD Server=[%s], IP=[%s]", bparam.stDdnsParam.serversvr, ipaddr );
            break;

		/* BEGIN: Added by wupm, 2013/4/19 */
		//NEO
		case 22:
            //GetDnsIp( bparam.stDdnsParam.serversvr, ipaddr );
            //GetDnsIp( "www.9299.org", ipaddr );
            //GetIpByHostName( "www.baidu.com", ipaddr );
            //GetIpByHostName( "www.9299.org", ipaddr );
            GetIpByHostName( "www.365home.org", ipaddr );
			Textout( "NEO Server=[%s], IP=[%s]", bparam.stDdnsParam.serversvr, ipaddr );
            break;

		/* BEGIN: Added by wupm, 2013/4/25 */
		case 23:
            GetIpByHostName( "www.ipupnp.hk", ipaddr );
            Textout( "NEO Server=[%s], IP=[%s]", bparam.stDdnsParam.serversvr, ipaddr );
            break;
		case 24:
			//GetIpByHostName( "www.ibabycam.net", ipaddr );
			GetIpByHostName( "www.ulife.info", ipaddr );
            Textout( "NEO Server=[%s], IP=[%s]", bparam.stDdnsParam.serversvr, ipaddr );
            break;

		case 25:
			GetIpByHostName( "www.ipcpnp.com", ipaddr );
            Textout( "NEO Server=[%s], IP=[%s]", bparam.stDdnsParam.serversvr, ipaddr );
            break;
		case 26:
			GetIpByHostName( "mytenvis.org", ipaddr );
            Textout( "NEO Server=[%s], IP=[%s]", bparam.stDdnsParam.serversvr, ipaddr );
			break;
		default:
            iRet = -1;
            break;
    }

    return iRet;
}


void FactoryRegisterInit( char*  pdst )
{
    int iRet = 0;

    switch ( bparam.stDdnsParam.factoryversion )
    {
        case 10:		//vstarcam
        	/* BEGIN: Modified by wupm, 2013/6/14 */
            sprintf( pdst, "GET /api/userip.asp?username=%s&userpwd=%s&vertype=924&language=1&dtype=0&tcpport=%d&lanip=%s&cmdport=0&rtspport=0&dataport=0&uid=%s\r\nHTTP/1.1\r\n\r\n",
                     bparam.stDdnsParam.serveruser,
                     bparam.stDdnsParam.serverpwd,
                     bparam.stNetParam.nPort,
                     bparam.stNetParam.szIpAddr,
                     bparam.stIEBaseParam.dwDeviceID );
            break;

        case 11:		//xinhuaan  viporg
            sprintf( pdst, "GET http://%s/upgengxin.asp?username=%s&userpwd=%s&userdomain=9299.org&userport=%d&userip=%s&usermac=00-00-00-00-00-00&mod=%d HTTP/1.1\r\nHost: www.9299.org\r\n\r\n",
                     bparam.stDdnsParam.serversvr,
                     bparam.stDdnsParam.serveruser,
                     bparam.stDdnsParam.serverpwd,
                     bparam.stNetParam.nPort,
                     bparam.stNetParam.szIpAddr,
                     bparam.stDdnsParam.factorymode );
            //printf("buf:%s\n",pdst);
            break;

        case 12:		//xinhuaan 88safe
        	/* BEGIN: Modified by wupm, 2013/6/14 */
            sprintf( pdst, "GET http://%s/vipddns/upgengxin.asp?username=%s&userpwd=%s HTTP/1.1\r\n\r\n", bparam.stDdnsParam.serversvr,
                     bparam.stDdnsParam.serveruser,
                     bparam.stDdnsParam.serverpwd );
            break;

        case 13:
			/* BEGIN: Modified by wupm, 2013/6/14 */
            sprintf( pdst, "GET /api/userip.asp?username=%s&userpwd=%s&vertype=913&language=0&dtype=0&tcpport=%d&lanip=%s& HTTP/1.1\r\n\r\n",
                     bparam.stDdnsParam.serveruser,
                     bparam.stDdnsParam.serverpwd,
                     bparam.stNetParam.nPort,
                     bparam.stNetParam.szIpAddr );
            break;

            /* BEGIN: Added by wupm, 2013/3/23 */
        case 21:	//PSD
            /* BEGIN: Modified by wupm, 2013/6/14 */
            //Modify by liangdong on 2013-8-27 16:31
            sprintf( pdst, "GET /api/userip.asp?username=%s&userpwd=%s&vertype=940&language=1&dtype=0&tcpport=%d&lanip=%s&rtspport=%d& HTTP/1.1\r\nHost: user.easyn.hk\r\n\r\n",
                     bparam.stDdnsParam.serveruser,
                     bparam.stDdnsParam.serverpwd,
                     bparam.stNetParam.nPort,
                     bparam.stNetParam.szIpAddr,
                     0 );
            break;

            /* BEGIN: Added by wupm, 2013/4/19 */
            //NEO
        case 22:
            if ( 1 )
            {
                int i;
                char ip[24];
                char mac[24];
                char host[256];
                char server[256];
                memset( mac, 0, 24 );
                memcpy( mac, bparam.stIEBaseParam.szMac, 17 );

                for ( i = 0; i < strlen( mac ); i++ )
                {
                    if ( mac[i] == ':' )
                    {
                        mac[i] = '-';
                    }
                }

                Textout( "MAC = [%s]", mac );

				memset(ip, 0, 24);
				GetExternIp3(ip);
				Textout("Extern IP = [%s]", ip);

				/* BEGIN: Modified by wupm, 2013/4/19 */
				#if	0
				//GET %s?username=%s&userpwd=%s&userdomain=%s&userport=%d&userip=%s&usermac=%s&mod=%d
				//HTTP/1.1\r\nHost:%s\r\n\r\n
				{
					char *pdst = NULL;
					memset(host, 0, 256);
					memset(server, 0, 256);
					strcpy(host, bparam.stDdnsParam.szProxySvr);
					pdst = strstr(host, "/");
					strcpy(server, pdst);
					host[pdst-host] = 0;
				}

				sprintf( pdst, "GET %s?"
					"username=%s&"
					"userpwd=%s&"
					"userdomain=%s&"
					"userport=%d&"
					"userip=%s&"
					"usermac=%s&"
					"mod=%d "
					"HTTP/1.1\r\nHost:%s\r\n\r\n",
					server,	//bparam.stDdnsParam.szProxySvr,
					bparam.stDdnsParam.szUserName,
					bparam.stDdnsParam.szPassword,
					bparam.stDdnsParam.szDdnsName,
					bparam.stNetParam.nPort,
					(ip[0] == 0) ? bparam.stNetParam.szIpAddr : ip,
					mac,
					bparam.stDdnsParam.byMode,
					host
					);
				#else
				//GET /upgengxin.asp?
				//username=yb1011&userpwd=yb1321&
				//userdomain=9299.org&
				//userport=8968&userip=112.95.178.97&usermac=00-00-00-00-00-00&
				//mod=2 HTTP/1.1\r\nHost: www.9299.org\r\n\r\n

				sprintf( pdst,
					"GET http://www.365home.org/upgengxin.asp?"
					//"GET http://www.9299.org/upgengxin.asp?"
					"username=%s&userpwd=%s&"
					"userdomain=%s&"
					"userport=%d&"
					"userip=%s&usermac=%s&"
					//"userip=128.0.0.1&usermac=00-00-00-00-00-00&"
					"mod=%d HTTP/1.1\r\nHost: www.365home.org\r\n\r\n",
					//"mod=%d HTTP/1.1\r\nHost: www.9299.org\r\n\r\n",
					bparam.stDdnsParam.serveruser,
					bparam.stDdnsParam.serverpwd,
					bparam.stDdnsParam.serversvr,
					bparam.stNetParam.nPort,
					(ip[0] == 0) ? bparam.stNetParam.szIpAddr : ip,
					mac,
					bparam.stDdnsParam.factorymode);
				#endif
                Textout( "HTTP = [%s]", pdst );
            }
			break;

            /* BEGIN: Added by wupm, 2013/4/25 */
        case 23:
            //http://www.ipupnp.hk/upgengxin.asp?
            //username=1234567&userpwd=1234567&
            //userdomain=ipupnp.hk&userport=8182&mod=1
            sprintf( pdst,
                     "GET http://www.ipupnp.hk/upgengxin.asp?"
                     //"GET http://www.9299.org/upgengxin.asp?"
                     "username=%s&userpwd=%s&"
                     "userdomain=ipupnp.hk&"
                     "userport=%d&"
                     //"userip=%s&usermac=%s&"
                     "userip=128.0.0.1&usermac=00-00-00-00-00-00&"
                     "mod=%d "
                     "HTTP/1.1\r\nHost: www.ipupnp.hk\r\n\r\n",
                     //"mod=%d HTTP/1.1\r\nHost: www.9299.org\r\n\r\n",
                     bparam.stDdnsParam.serveruser,
                     bparam.stDdnsParam.serverpwd,
                     //bparam.stDdnsParam.serversvr,
                     bparam.stNetParam.nPort,
                     //(ip[0] == 0) ? bparam.stNetParam.szIpAddr : ip,
                     //mac,
                     bparam.stDdnsParam.factorymode );
            Textout( "HTTP = [%s]", pdst );
            break;
			case 24:
				 sprintf( pdst, "GET http://%s?username=%s&userpwd=%s&userdomain=%s&mod=%d&userport=%d&userip=128.0.0.1&usermac=00-00-00-00-00-00  HTTP/1.1\r\nHost: www.ulife.info\r\n\r\n",
            //sprintf( pdst, "GET http://%s?username=%s&userpwd=%s&userdomain=%s&mod=%d&userport=%d&userip=128.0.0.1&usermac=00-00-00-00-00-00  HTTP/1.1\r\nHost: www.ibabycam.net\r\n\r\n",
                     //"www.ibabycam.net/upgengxin.asp",
                     "www.ulife.info/upgengxin.asp",
                     bparam.stDdnsParam.serveruser,
                     bparam.stDdnsParam.serverpwd,
                     bparam.stDdnsParam.serversvr,
                     bparam.stDdnsParam.factorymode,
                     bparam.stNetParam.nPort
                   );
            Textout( "HTTP = [%s]", pdst );
            break;
			case 25:
				 //sprintf( pdst, "GET http://%s?username=%s&userpwd=%s&userdomain=%s&mod=%d&userport=%d&userip=128.0.0.1&usermac=00-00-00-00-00-00  HTTP/1.1\r\nHost: www.9299.org\r\n\r\n",
            sprintf( pdst, "GET http://%s?username=%s&userpwd=%s&userdomain=%s&mod=%d&userport=%d&userip=128.0.0.1&usermac=00-00-00-00-00-00  HTTP/1.1\r\nHost: www.ipcpnp.com\r\n\r\n",
                     "www.ipcpnp.com/upgengxin.asp",
                     bparam.stDdnsParam.serveruser,
                     bparam.stDdnsParam.serverpwd,
                     bparam.stDdnsParam.serversvr,
                     bparam.stDdnsParam.factorymode,
                     bparam.stNetParam.nPort
                   );
            Textout( "HTTP = [%s]", pdst );
            break;
			case 26:
			{
				sprintf( pdst,"GET /twupdate.aspx?user=%s&pwd=%s&acod=AALL HTTP/1.1\r\nHost: mytenvis.org\r\n\r\n",
				//sprintf( pdst, "GET %s?username=%s&userpwd=%s&userport=%d&mod=2&userdomain=ipcpnp.com&usermac=00-00-00-00-00-00&userip=128.0.0.1 HTTP/1.1\r\nHost: www.ipcpnp.com\r\n\r\n",
						// bparam.stDdnsParam.szProxySvr,
						 bparam.stDdnsParam.serveruser,
						 bparam.stDdnsParam.serverpwd
						 //bparam.stNetParam.nPort
					   );
			   Textout( "HTTP = [%s]", pdst );
			}
			break;

        default:
            iRet = -1;
            break;
    }

	/* BEGIN: Modified by wupm, 2013/7/1 */
    //return iRet;
}

void FactoryResultParase( char* pbuffer )
{
    char* pdst = NULL;
    char* pdst1 = NULL;
    int iRet = 0;

    switch ( bparam.stDdnsParam.factoryversion )
    {
        case 10:		//vstarcam
            pdst = strstr( pbuffer + 32, "Update-OK" );

            if ( pdst != NULL )
            {
                factorystatus = 22;
                factorytimes = 0;
            }

            pdst = strstr( pbuffer + 32, "ERRIDS_SERVER_NOAUTH" );

            if ( pdst != NULL )
            {
                factorystatus = 23;
            }

            pdst = strstr( pbuffer + 32, "ERRIDS_SERVER_NOID" );

            if ( pdst != NULL )
            {
                factorystatus = 24;
            }

            pdst = strstr( pbuffer + 32, "ERRIDS_SERVER_OVER" );

            if ( pdst != NULL )
            {
                factorystatus = 25;
            }

            pdst = strstr( pbuffer + 32, "ERRIDS_SERVER_ERR_IDDISABLE" );

            if ( pdst != NULL )
            {
                factorystatus = 26;
            }

            pdst = strstr( pbuffer + 32, "ERRIDS_SERVER_ERR_PARAM" );

            if ( pdst != NULL )
            {
                factorystatus = 27;
            }

            break;

        case 11:		//xinhuaan  viporg
            pdst1 = strstr( pbuffer, "UP" );
            pdst = strstr( pbuffer, "OK" );

            if ( ( pdst == NULL ) && ( pdst1 == NULL ) )
            {
                factorystatus = 19;			//ok
                factorytimes = 0;
            }

            else
            {
                factorystatus = 18;			//failed
            }

            break;

        case 13:		//smarteye ddns
            factorystatus = 18;
            pdst = strstr( pbuffer + 32, "Update-OK" );

            if ( pdst != NULL )
            {
                factorystatus = 19;
                factorytimes = 0;
            }

            break;

            /* BEGIN: Added by wupm, 2013/3/23 */
        case 21:	//PSD
            Textout1( "PSD Response = [%s]", pbuffer );
            pdst = strstr( pbuffer, "Update-OK" );

            if ( pdst != NULL )
            {
                factorystatus = 22;
                factorytimes = 0;
            }

            pdst = strstr( pbuffer, "ERRIDS_SERVER_NOAUTH" );

            if ( pdst != NULL )
            {
                factorystatus = 23;
            }

            pdst = strstr( pbuffer, "ERRIDS_SERVER_NOID" );

            if ( pdst != NULL )
            {
                factorystatus = 24;
            }

            pdst = strstr( pbuffer, "ERRIDS_SERVER_OVER" );

            if ( pdst != NULL )
            {
                factorystatus = 25;
            }

            pdst = strstr( pbuffer, "ERRIDS_SERVER_ERR_IDDISABLE" );

            if ( pdst != NULL )
            {
                factorystatus = 26;
            }

            pdst = strstr( pbuffer, "ERRIDS_SERVER_ERR_PARAM" );

            if ( pdst != NULL )
            {
                factorystatus = 27;
            }

            Textout1( "factorystatus = %d", factorystatus );
            break;

		/* BEGIN: Added by wupm, 2013/4/19 */
		//NEO
		case 22:

		/* BEGIN: Added by wupm, 2013/4/25 */
		//EYESIGHT
		case 23:
			if ( 1 )
			{
                char* pdst = NULL;
                char* psrc = NULL;
                Textout( "DDNS Server Response = [%s]", pbuffer );
                factorystatus = 45;
                psrc = strstr( pbuffer, "Cache-control:" );

                if ( psrc != NULL )
                {
                    pdst = strstr( psrc + 22, "OK" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 40;
                		factorytimes = 0;
                    }

                    pdst = strstr( psrc + 22, "UP" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 40;
                		factorytimes = 0;
                    }

                    pdst = strstr( psrc + 22, "ER" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 41;
                    }

                    pdst = strstr( psrc + 22, "DA" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 42;
                    }

                    pdst = strstr( psrc + 22, "SNE" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 43;
                    }

                    pdst = strstr( psrc + 22, "NE" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 44;
                    }
                }

                Textout( "factorystatus = %d", factorystatus );
                //bparam.stDdnsParam.dnsstatus = third_status;
            }
         	break;
		case 24:
            if(1)
    		{
    			char* 	pdst = NULL;
    			char*    psrc = NULL;
                Textout( "DDNS Server Response = [%s]", pbuffer);
                factorystatus = 45;
    			psrc = strstr( pbuffer, "Cache-control:" );

                if ( psrc != NULL )
                {
                    pdst = strstr( psrc + 22, "OK" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 40;
						factorytimes = 0;
                    }

                    pdst = strstr( psrc + 22, "UP" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 40;
						factorytimes = 0;
                    }

                    pdst = strstr( psrc + 22, "ER" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 41;
                    }

                    pdst = strstr( psrc + 22, "DA" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 42;
                    }

                    pdst = strstr( psrc + 22, "SNE" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 43;
                    }

                    pdst = strstr( psrc + 22, "NE" );

                    if ( pdst != NULL )
                    {
                        factorystatus = 44;
                    }
                }
    		}
			break;
		case 25:
			if(1)
			{
				char*	pdst = NULL;
				char*	 psrc = NULL;
				Textout( "DDNS Server Response = [%s]", pbuffer);
				factorystatus = 45;
				psrc = strstr( pbuffer, "Cache-control:" );
				if ( psrc != NULL )
				{
					pdst = strstr( psrc + 22, "OK" );
					if ( pdst != NULL )
					{
						factorystatus = 40;
						factorytimes = 0;
					}
					pdst = strstr( psrc + 22, "UP" );
					if ( pdst != NULL )
					{
						factorystatus = 40;
						factorytimes = 0;
					}
					pdst = strstr( psrc + 22, "ER" );
					if ( pdst != NULL )
					{
						factorystatus = 41;
					}
					pdst = strstr( psrc + 22, "DA" );
					if ( pdst != NULL )
					{
						factorystatus = 42;
					}
					pdst = strstr( psrc + 22, "SNE" );
					if ( pdst != NULL )
					{
						factorystatus = 43;
					}
					pdst = strstr( psrc + 22, "NE" );
					if ( pdst != NULL )
					{
						factorystatus = 44;
					}
				}
			}
			break;
		case 26:		//xinhuaan	viporg
				//pdst1 = strstr( pbuffer, "UP" );
				Textout("%s", pbuffer);
				pdst = strstr( pbuffer, "900" );


				if ( ( pdst == NULL ) /*&& ( pdst1 == NULL ) */)
				{
					factorystatus = 19; 		//ok
					Textout("DDNS  FAILD!!!!!!");
				}

				else
				{
					factorystatus = 18; 		//failed
					factorytimes = 0;
					Textout("DDNS  OK!!!!!!");
				}

				break;

        default:
            iRet = -1;
            break;
    }

    /* BEGIN: Modified by wupm, 2013/7/1 */
    //return iRet;
}

#ifdef TENVIS
static sem_t			semStartFatoryDDNS;
void NoteTenvisFatoryDdns()
{
    sem_post( &semStartFatoryDDNS);
}
#endif

void* FactoryRegisterProc( void* p )
{
    /* BEGIN: Deleted by wupm, 2013/3/27 */
    //unsigned int            	times = 180 * 25;
    char                    		ipaddr[16];
    int                     		socket = -1;
    struct sockaddr_in      daddr;
    int                     		iRet = 0;
    char					heatcont[512];
    struct timeval 		TimeOut;
    short				i;
    int 					nFlag;

    #ifdef TENVIS
    if ( ( sem_init( &semStartFatoryDDNS, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    while(1)
    {
        sem_wait( &semStartFatoryDDNS);
    #else
        sleep( 5 );
    #endif

        Textout( "Start FactoryRegisterProc" );
        factorytimes = bparam.stDdnsParam.serverbeat;

        while ( 1 )
        {
            sleep( 1 );

            #ifdef TENVIS
            if(tenvisparam.nEnable == 0)
            {
                Textout("Stop Tenvis fatory ddns");
                break;
            }
            #endif

            if ( bparam.stDdnsParam.factoryversion < 10 )
            {
                continue;
            }

        //printf("times %d\n",times);
        if ( strlen( bparam.stDdnsParam.serversvr ) == 0 )
        {
            factorystatus = 0x00;
            continue;
        }

        if ( strlen( bparam.stDdnsParam.serveruser ) == 0 )
        {
            factorystatus = 0x00;
            continue;
        }

        /* BEGIN: Modified by wupm, 2013/3/27 */
#if	1

        if ( factorytimes++ < bparam.stDdnsParam.serverbeat )
        {
            continue;
        }

#else
        /* BEGIN: Added by wupm, 2013/3/19 */
#ifdef	VSTARCAM

        if ( factorytimes++ < bparam.stDdnsParam.serverbeat )
        {
            continue;
        }

#else

        if ( times++ < bparam.stDdnsParam.serverbeat )
        {
            continue;
        }

#endif
#endif
        factorytimes = bparam.stDdnsParam.serverbeat;
        //Textout( "factorytimes %d\n", factorytimes );
        //times = 0;
        factorystatus = 1;
#if 0
        printf( "dns name:%s\n", bparam.stDdnsParam.serversvr );
        printf( "dns user:%s\n", bparam.stDdnsParam.serveruser );
        printf( "dns pass:%s\n", bparam.stDdnsParam.serverpwd );
        printf( "dns port:%d\n", bparam.stDdnsParam.serverport );
        printf( "dns heat:%d\n", bparam.stDdnsParam.serverbeat );
#endif
        //init socket
        socket = InitSocket( 0, 1, NULL );             //tcp
        nFlag = 1;
        setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, ( void* )&nFlag, sizeof( int ) );
        TimeOut.tv_sec = 30;
        TimeOut.tv_usec = 0;
        setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, ( char* )&TimeOut, sizeof( TimeOut ) );
        TimeOut.tv_sec = 30;
        TimeOut.tv_usec = 0;
        setsockopt( socket, SOL_SOCKET, SO_SNDTIMEO, ( char* )&TimeOut, sizeof( TimeOut ) );
        //get dns server ipaddr
        memset( ipaddr, 0x00, 16 );
        iRet = GetDnsServerIp( ipaddr );

        if ( iRet != 0 )
        {
            Textout( "=========Get dns Ip failed============\n" );
            factorystatus = 2;
            sleep( 10 );
            /* BEGIN: Deleted by wupm, 2013/3/27 */
            //times = bparam.stDdnsParam.serverbeat;
            continue;
        }

        //printf( "get ipaddr:%s\n", ipaddr );
        //send heat param
        bzero( &daddr, sizeof( struct sockaddr_in ) );
        daddr.sin_family        = AF_INET;
        daddr.sin_port          = htons( bparam.stDdnsParam.serverport );
        daddr.sin_addr.s_addr   = inet_addr( ipaddr );

        if ( connect( socket, ( struct sockaddr* )&daddr, sizeof( struct sockaddr ) ) == -1 )
        {
            Textout( "=========connect failed============\n" );
            factorystatus = 2;
            CloseSocket( socket );
            sleep( 10 );
            /* BEGIN: Deleted by wupm, 2013/3/27 */
            //times  = bparam.stDdnsParam.serverbeat;
            continue;
        }

        //config heat param
        memset( heatcont, 0, 512 );
        FactoryRegisterInit( heatcont );
        iRet = send( socket, heatcont, strlen( heatcont ), 0 );

        if ( iRet > 0 )
        {
            memset( heatcont, 0x00, 512 );
            iRet = recv( socket, heatcont, 511, 0 );
            //printf( "recive len:%d buf:%s\n", iRet, heatcont );
            FactoryResultParase( heatcont );
        }

        else
        {
            Textout( "send failed to ddns\n" );
        }

            CloseSocket( socket );
        }

    #ifdef TENVIS
    }
    #endif
}
void FactoryThread( void )
{
    pthread_t               Factorythread;
    /* BEGIN: Added by Baggio.wu, 2013/7/10 */
    pthread_t               DnsSendAlarmThread;

	/* BEGIN: Deleted by wupm, 2013/11/1 */
    //pthread_create( &DnsSendAlarmThread, 0, &DnsSendAlarmProc, NULL );
    //pthread_create( &Factorythread, 0, &FactoryRegisterProc, NULL );

}
