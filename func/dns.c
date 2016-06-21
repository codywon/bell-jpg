#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <unistd.h>
#include <sys/time.h>

#include "cmdhead.h"
#include "param.h"
#include "network.h"

pthread_mutex_t dnsmutex = PTHREAD_MUTEX_INITIALIZER;
unsigned char	third_status = 0;
char 		ddnsflag = 0x00;


void dnslock( void )
{
    pthread_mutex_lock( &dnsmutex );
}

void dnsunlock( void )
{
    pthread_mutex_unlock( &dnsmutex );
}

static char table64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void base64Encode( char* intext, char* output )
{
    unsigned char	ibuf[3];
    unsigned char	obuf[4];
    int			i;
    int			inputparts;

    while ( *intext )
    {
        for ( i = inputparts = 0; i < 3; i++ )
        {
            if ( *intext )
            {
                inputparts++;
                ibuf[i] = *intext;
                intext++;
            }

            else
            {
                ibuf[i] = 0;
            }
        }

        obuf[0] = ( ibuf[0] & 0xFC ) >> 2;
        obuf[1] = ( ( ibuf[0] & 0x03 ) << 4 ) | ( ( ibuf[1] & 0xF0 ) >> 4 );
        obuf[2] = ( ( ibuf[1] & 0x0F ) << 2 ) | ( ( ibuf[2] & 0xC0 ) >> 6 );
        obuf[3] = ibuf[2] & 0x3F;

        switch ( inputparts )
        {
            case 1: /* only one byte read */
                sprintf( output, "%c%c==", table64[obuf[0]], table64[obuf[1]] );
                break;

            case 2: /* two bytes read */
                sprintf( output, "%c%c%c=", table64[obuf[0]], table64[obuf[1]], table64[obuf[2]] );
                break;

            default:
                sprintf( output, "%c%c%c%c", table64[obuf[0]], table64[obuf[1]], table64[obuf[2]], table64[obuf[3]] );
                break;
        }

        output += 4;
    }

    *output = 0;
}

void SetThirdStatus( void )
{
    third_status = 0;
}

void DnsRegisterInit( char index, char*  pdst )
{
    switch ( index )
    {
        case 2:
        {
            char    szAuth[512];
            char    auth[100];
            char    ipaddr[16];
            memset( ipaddr, 0x00, 16 );
            GetExternIp1( ipaddr );
            memset( auth, 0x00, 100 );
            sprintf( auth, "%s:%s", bparam.stDdnsParam.szUserName, bparam.stDdnsParam.szPassword );
            memset( szAuth, 0x00, 512 );
            base64Encode( auth, szAuth );
#if 1
            sprintf( pdst, "GET /nic/update?system=dyndns&hostname=%s&myip=%s&wildcard=NOCHG&mx=NOCHG&backmx=NOCHG HTTP/1.0\r\nHost: members.dyndns.org\r\nAuthorization: Basic %s\r\nUser-Agent: Company - Device - Version Number\r\n\r\n",
                     bparam.stDdnsParam.szDdnsName,
                     ipaddr,
                     szAuth );
#else
            sprintf( pdst, "GET /nic/update?system=dyndns&hostname=%s&myip=%s&wildcard=NOCHG&mx=NOCHG&backmx=NOCHG HTTP/1.0\r\nHost: techwatch.ariso.eu\r\nAuthorization: Basic %s\r\nUser-Agent: Company - Device - Version Number\r\n\r\n",
                     bparam.stDdnsParam.szDdnsName,
                     ipaddr,
                     szAuth );
#endif
        }
        break;

        case 3:
        {
            char    szAuth[512];
            char    auth[100];
            char    ipaddr[16];
            memset( ipaddr, 0x00, 16 );
            GetExternIp1( ipaddr );
            memset( auth, 0x00, 100 );
            sprintf( auth, "%s:%s", bparam.stDdnsParam.szUserName, bparam.stDdnsParam.szPassword );
            memset( szAuth, 0x00, 512 );
            base64Encode( auth, szAuth );
#if 1
            sprintf( pdst, "GET /nic/update?system=statdns&hostname=%s&myip=%s&wildcard=NOCHG&mx=NOCHG&backmx=NOCHG HTTP/1.0\r\nHost: members.dyndns.org\r\nAuthorization: Basic %s\r\nUser-Agent: Company - Device - Version Number\r\n\r\n",
                     bparam.stDdnsParam.szDdnsName,
                     ipaddr,
                     szAuth );
#else
            sprintf( pdst, "GET /nic/update?system=statdns&hostname=%s&myip=%s&wildcard=NOCHG&mx=NOCHG&backmx=NOCHG HTTP/1.0\r\nHost: techwatch.ariso.eu\r\nAuthorization: Basic %s\r\nUser-Agent: Company - Device - Version Number\r\n\r\n",
                     bparam.stDdnsParam.szDdnsName,
                     ipaddr,
                     szAuth );
#endif
        }
        break;

        case 4:
        {
            char    szAuth[512];
            char    auth[100];
            char    ipaddr[16];
            memset( ipaddr, 0x00, 16 );
            GetExternIp1( ipaddr );
            memset( auth, 0x00, 100 );
            sprintf( auth, "%s:%s", bparam.stDdnsParam.szUserName, bparam.stDdnsParam.szPassword );
            memset( szAuth, 0x00, 512 );
            base64Encode( auth, szAuth );
#if 1
            sprintf( pdst, "GET /nic/update?system=custom&hostname=%s&myip=%s&wildcard=NOCHG&mx=NOCHG&backmx=NOCHG HTTP/1.0\r\nHost: members.dyndns.org\r\nAuthorization: Basic %s\r\nUser-Agent: Company - Device - Version Number\r\n\r\n",
                     bparam.stDdnsParam.szDdnsName,
                     ipaddr,
                     szAuth );
#else
            sprintf( pdst, "GET /nic/update?system=custom&hostname=%s&myip=%s&wildcard=NOCHG&mx=NOCHG&backmx=NOCHG HTTP/1.0\r\nHost: techwatch.ariso.eu\r\nAuthorization: Basic %s\r\nUser-Agent: Company - Device - Version Number\r\n\r\n",
                     bparam.stDdnsParam.szDdnsName,
                     ipaddr,
                     szAuth );
#endif
        }
        break;

        case 8:
        {
            char	szAuth[512];
            char	auth[100];
            char	ipaddr[16];
            memset( ipaddr, 0x00, 16 );
            GetExternIp1( ipaddr );
            memset( auth, 0x00, 100 );
            sprintf( auth, "%s:%s", bparam.stDdnsParam.szUserName, bparam.stDdnsParam.szPassword );
            memset( szAuth, 0x00, 512 );
            base64Encode( auth, szAuth );
            sprintf( pdst, "GET /dyndns/update?system=dyndns&hostname=%s&myip=%s&wildcard=OFF&mx=&backmx=NO&offline=NO HTTP/1.1\r\nHost: members.3322.org\r\nAuthorization: Basic %s\r\nUser-Agent: myclient/1.0 me@null.net\r\n\r\n",
                     bparam.stDdnsParam.szDdnsName,
                     ipaddr,
                     szAuth );
        }
        break;

        case 9:
        {
            char    szAuth[512];
            char    auth[100];
            char    ipaddr[16];
            memset( ipaddr, 0x00, 16 );
            GetExternIp1( ipaddr );
            memset( auth, 0x00, 100 );
            sprintf( auth, "%s:%s", bparam.stDdnsParam.szUserName, bparam.stDdnsParam.szPassword );
            memset( szAuth, 0x00, 512 );
            base64Encode( auth, szAuth );
            sprintf( pdst, "GET /dyndns/update?system=statdns&hostname=%s&myip=%s&wildcard=OFF&mx=&backmx=NO&offline=NO HTTP/1.1\r\nHost: members.3322.org\r\nAuthorization: Basic %s\r\nUser-Agent: myclient/1.0 me@null.net\r\n\r\n",
                     bparam.stDdnsParam.szDdnsName,
                     ipaddr,
                     szAuth );
        }
        break;

        case 10:
        {
#if 0
            sprintf( pdst, "GET http://www.9299.org/upgengxin.asp?username=yb1011&userpwd=yb1321&userdomain=9299.org&userport=8968&userip=112.95.178.97&usermac=00-00-00-00-00-00&mod=2  HTTP/1.1\r\nHost: www.9299.org\r\n\r\n" );
#else
            sprintf( pdst, "GET http://%s?username=%s&userpwd=%s&userdomain=%s&mod=%d&userport=%d&userip=128.0.0.1&usermac=00-00-00-00-00-00  HTTP/1.1\r\nHost: www.9299.org\r\n\r\n",
                     bparam.stDdnsParam.szProxySvr,
                     bparam.stDdnsParam.szUserName,
                     bparam.stDdnsParam.szPassword,
                     bparam.stDdnsParam.szDdnsName,
                     bparam.stDdnsParam.byMode,
                     bparam.stNetParam.nPort
                   );
#endif
        }
        break;

        case 11:
        {
            sprintf( pdst, "GET /api/userip.asp?username=%s&userpwd=%s&vertype=913&language=0&dtype=0&tcpport=%d&lanip=%s&\n\n",
                     bparam.stDdnsParam.szUserName,
                     bparam.stDdnsParam.szPassword,
                     bparam.stNetParam.nPort,
                     bparam.stNetParam.szIpAddr );
        }
        break;

        case 12:
            sprintf( pdst, "GET /api/userip.asp?username=%s&userpwd=%s&vertype=913&language=0&dtype=0&tcpport=%d&lanip=%s&\n\n",
                     bparam.stDdnsParam.szUserName,
                     bparam.stDdnsParam.szPassword,
                     bparam.stNetParam.nPort,
                     bparam.stNetParam.szIpAddr );
            break;

        case 13:
        {
            sprintf( pdst, "GET http://%s?username=%s&userpwd=%s&userport=%d&mod=%d\n\n",
                     bparam.stDdnsParam.szProxySvr,
                     bparam.stDdnsParam.szUserName,
                     bparam.stDdnsParam.szPassword,
                     bparam.stNetParam.nPort,
                     bparam.stDdnsParam.byReserved + 1 );
        }
        break;

        case 14:
        {
            sprintf( pdst, "GET /api/userip.asp?username=%s&userpwd=%s&vertype=916&language=1&dtype=0&tcpport=%d&lanip=%s&rtspport=%d&\n\n\n",
                     bparam.stDdnsParam.szUserName,
                     bparam.stDdnsParam.szPassword,
                     bparam.stNetParam.nPort,
                     bparam.stNetParam.szIpAddr,
                     bparam.stNetParam.rtspport );
        }

        case 15:
            sprintf( pdst, "GET http://%s?username=%s&userpwd=%s&userport=%d&mod=%d&userdomain=vipcam.org&usermac=00-00-00-00-00-00&userip=128.0.0.1\n\n",
                     bparam.stDdnsParam.szProxySvr,
                     bparam.stDdnsParam.szUserName,
                     bparam.stDdnsParam.szPassword,
                     bparam.stNetParam.nPort,
                     bparam.stDdnsParam.byReserved + 1 );
            break;
    }

    //printf("buf:%s\n",pdst);
}

void DnsRegisterResult( char index, char* regcont )
{
    int third_status;

    //printf("index %d\n",index);
    switch ( index )
    {
        case 2:
        case 3:
        case 4:
        {
            char* pdst = NULL;
            third_status = 4;
            pdst = strstr( regcont, "good" );

            if ( pdst != NULL )
            {
                third_status = 3;
            }

            pdst = strstr( regcont, "nochg" );

            if ( pdst != NULL )
            {
                third_status = 3;
            }

            pdst = strstr( regcont, "error" );

            if ( pdst != NULL )
            {
                third_status = 4;
            }

            pdst = strstr( regcont, "badauth" );

            if ( pdst != NULL )
            {
                third_status = 5;
            }

            pdst = strstr( regcont, "donator" );

            if ( pdst != NULL )
            {
                third_status = 6;
            }

            pdst = strstr( regcont, "notfqdn" );

            if ( pdst != NULL )
            {
                third_status = 7;
            }

            pdst = strstr( regcont, "nohost" );

            if ( pdst != NULL )
            {
                third_status = 8;
            }

            pdst = strstr( regcont, "yours" );

            if ( pdst != NULL )
            {
                third_status = 9;
            }

            pdst = strstr( regcont, "numhost" );

            if ( pdst != NULL )
            {
                third_status = 10;
            }

            pdst = strstr( regcont, "abuse" );

            if ( pdst != NULL )
            {
                third_status = 11;
            }

            pdst = strstr( regcont, "good 127.0.0.1" );

            if ( pdst != NULL )
            {
                third_status = 12;
            }

            bparam.stDdnsParam.dnsstatus = third_status;
        }
        break;

        case 8:
        case 9:
        {
            char* pdst = NULL;
            third_status = 30;
            pdst = strstr( regcont, "good" );

            if ( pdst != NULL )
            {
                third_status = 29;
            }

            pdst = strstr( regcont, "nochg" );

            if ( pdst != NULL )
            {
                third_status = 29;
            }

            pdst = strstr( regcont, "error" );

            if ( pdst != NULL )
            {
                third_status = 30;
            }

            pdst = strstr( regcont, "badauth" );

            if ( pdst != NULL )
            {
                third_status = 31;
            }

            pdst = strstr( regcont, "donator" );

            if ( pdst != NULL )
            {
                third_status = 32;
            }

            pdst = strstr( regcont, "notfqdn" );

            if ( pdst != NULL )
            {
                third_status = 33;
            }

            pdst = strstr( regcont, "nohost" );

            if ( pdst != NULL )
            {
                third_status = 34;
            }

            pdst = strstr( regcont, "yours" );

            if ( pdst != NULL )
            {
                third_status = 35;
            }

            pdst = strstr( regcont, "numhost" );

            if ( pdst != NULL )
            {
                third_status = 36;
            }

            pdst = strstr( regcont, "abuse" );

            if ( pdst != NULL )
            {
                third_status = 37;
            }

            bparam.stDdnsParam.dnsstatus = third_status;
        }
        break;

        case 10:
        {
            char* pdst = NULL;
            char* psrc = NULL;
            third_status = 45;
            psrc = strstr( regcont, "Cache-control:" );

            if ( psrc != NULL )
            {
                pdst = strstr( psrc + 22, "OK" );

                if ( pdst != NULL )
                {
                    third_status = 40;
                }

                pdst = strstr( psrc + 22, "UP" );

                if ( pdst != NULL )
                {
                    third_status = 40;
                }

                pdst = strstr( psrc + 22, "ER" );

                if ( pdst != NULL )
                {
                    third_status = 41;
                }

                pdst = strstr( psrc + 22, "DA" );

                if ( pdst != NULL )
                {
                    third_status = 42;
                }

                pdst = strstr( psrc + 22, "SNE" );

                if ( pdst != NULL )
                {
                    third_status = 43;
                }

                pdst = strstr( psrc + 22, "NE" );

                if ( pdst != NULL )
                {
                    third_status = 44;
                }
            }

            bparam.stDdnsParam.dnsstatus = third_status;
        }

        case 11:
        {
            char* pdst = NULL;
            pdst = strstr( regcont, "Update-OK" );

            if ( pdst != NULL )
            {
                third_status = 22;
            }

            pdst = strstr( regcont, "ERRIDS_SERVER_NOAUTH" );

            if ( pdst != NULL )
            {
                third_status = 23;
            }

            pdst = strstr( regcont, "ERRIDS_SERVER_NOID" );

            if ( pdst != NULL )
            {
                third_status = 24;
            }

            pdst = strstr( regcont, "ERRIDS_SERVER_OVER" );

            if ( pdst != NULL )
            {
                third_status = 25;
            }

            pdst = strstr( regcont + 32, "ERRIDS_SERVER_ERR_IDDISABLE" );

            if ( pdst != NULL )
            {
                third_status = 26;
            }

            pdst = strstr( regcont + 32, "ERRIDS_SERVER_ERR_PARAM" );

            if ( pdst != NULL )
            {
                third_status = 27;
            }
        }
        break;

        case 12:
        {
            char* pdst = NULL;
            third_status = 1;
            pdst = strstr( regcont, "Update-OK" );

            if ( pdst != NULL )
            {
                third_status = 18;
            }

            else
            {
                third_status = 19;
            }
        }
        break;

        case 13:
        {
            char* pdst = NULL;
            char* pdst1 = NULL;
            pdst = strstr( regcont, "OK" );
            pdst1 = strstr( regcont, "UP" );

            if ( ( pdst != NULL ) || ( pdst1 != NULL ) )
            {
                third_status = 18;
            }

            else
            {
                third_status = 19;
            }
        }
        break;

        case 14:
        {
            char* pdst = NULL;
            pdst = strstr( regcont, "Update-OK" );

            if ( pdst != NULL )
            {
                third_status = 22;
            }

            pdst = strstr( regcont, "ERRIDS_SERVER_NOAUTH" );

            if ( pdst != NULL )
            {
                third_status = 23;
            }

            pdst = strstr( regcont, "ERRIDS_SERVER_NOID" );

            if ( pdst != NULL )
            {
                third_status = 24;
            }

            pdst = strstr( regcont, "ERRIDS_SERVER_OVER" );

            if ( pdst != NULL )
            {
                third_status = 25;
            }

            pdst = strstr( regcont + 32, "ERRIDS_SERVER_ERR_IDDISABLE" );

            if ( pdst != NULL )
            {
                third_status = 26;
            }

            pdst = strstr( regcont + 32, "ERRIDS_SERVER_ERR_PARAM" );

            if ( pdst != NULL )
            {
                third_status = 27;
            }
        }
        break;

        case 15:
        {
            char* pdst = NULL;
            char* pdst1 = NULL;
            pdst = strstr( regcont, "OK" );
            pdst1 = strstr( regcont, "UP" );

            if ( ( pdst != NULL ) || ( pdst1 != NULL ) )
            {
                third_status = 18;
            }

            else
            {
                third_status = 19;
            }
        }
        break;
    }
}

int GetDdnsName( char index, char* pbuf )
{
    char 	temp[64];
    char*	 ptemp = NULL;
    int	iRet = 0;

    switch ( index )
    {
        case 2:		//dyndns
        case 3:
        case 4:
        {
            strcpy( pbuf, "members.dyndns.org" );
            bparam.stDdnsParam.nProxyPort = 80;
        }
        break;

        case 8:		//3322
        case 9:
        {
            strcpy( pbuf, "www.3322.org" );
            bparam.stDdnsParam.nProxyPort = 80;
        }
        break;

        case 10:	//9299
        {
            //http:// = 7 find www.9299.org
            memset( temp, 0x00, 64 );
#if 0
            strcpy( temp, bparam.stDdnsParam.szProxySvr + 7 );
#else
            strcpy( temp, bparam.stDdnsParam.szProxySvr );
#endif
            ptemp = strstr( temp, "/" );

            if ( ptemp == NULL )
            {
                return -1;
            }

            printf( "buf:%s len %d", temp, ptemp - temp );
            memcpy( pbuf, temp, ptemp - temp );
        }
        break;

        case 11:	//factory
        {
            strcpy( pbuf, bparam.stDdnsParam.szProxySvr );
        }
        break;

        case 12:	//factory
            strcpy( pbuf, bparam.stDdnsParam.szProxySvr );
            break;

        case 13:	//factory
            memset( temp, 0x00, 64 );
            strcpy( temp, bparam.stDdnsParam.szProxySvr + 7 );
            ptemp = strstr( temp, "/" );

            if ( ptemp == NULL )
            {
                return -1;
            }

            //printf("buf:%s len %d",temp,ptemp - temp);
            memcpy( pbuf, temp, ptemp - temp );
            break;

        case 14:	//factory
            strcpy( pbuf, bparam.stDdnsParam.szProxySvr );
            break;

        case 15:	//factory
            memset( temp, 0x00, 64 );
            strcpy( temp, bparam.stDdnsParam.szProxySvr + 7 );
            ptemp = strstr( temp, "/" );

            if ( ptemp == NULL )
            {
                return -1;
            }

            //printf("buf:%s len %d",temp,ptemp - temp);
            memcpy( pbuf, temp, ptemp - temp );
            break;

        default:
            iRet = -1;
            break;
    }

    return iRet;
}

int ThirdDnsRegister( char index )
{
    char                    ipaddr[16];
    int                     rsocket = -1;
    struct sockaddr_in      daddr;
    int                     iRet;
    char                    regcont[1024];
    struct timeval          TimeOut;
    int 			nFlag;
    unsigned char		ddns_status = 255;
    char			ddnsname[64];
    ddns_status = 1;
    //get dnsserver namde
    //printf("get ddnsname of www type\n");
    memset( ddnsname, 0x00, 64 );
    iRet = GetDdnsName( index, ddnsname );

    if ( iRet )
    {
        //printf("dns server index is error\n");
        third_status = 0x02;
        return -1;
    }

    //get dnsserver ipaddr
    printf( "get dnsname ipaddr\n" );
    memset( ipaddr, 0x00, 16 );
    iRet = GetDnsIp( ddnsname, ipaddr );

    if ( iRet != 0 )
    {
        printf( "get failed:%s=%s\n", bparam.stDdnsParam.szProxySvr, ddnsname );
        CloseSocket( rsocket );
        third_status = 0x02;
        return -1;
    }

    printf( "get ipaddr:%s\n", ipaddr );
    //init register
    memset( regcont, 0x00, 1024 );
    DnsRegisterInit( index, regcont );
    //init socket
    rsocket = InitSocket( 0, 1, NULL );             //tcp
    nFlag = 1;
    setsockopt( rsocket, IPPROTO_TCP, TCP_NODELAY, ( void* )&nFlag, sizeof( int ) );
    TimeOut.tv_sec = 30;
    TimeOut.tv_usec = 0;
    setsockopt( rsocket, SOL_SOCKET, SO_RCVTIMEO, ( char* )&TimeOut, sizeof( TimeOut ) );
    TimeOut.tv_sec = 30;
    TimeOut.tv_usec = 0;
    setsockopt( rsocket, SOL_SOCKET, SO_SNDTIMEO, ( char* )&TimeOut, sizeof( TimeOut ) );
    bzero( &daddr, sizeof( struct sockaddr_in ) );
    daddr.sin_family        = AF_INET;
    daddr.sin_port          = htons( bparam.stDdnsParam.nProxyPort );
    daddr.sin_addr.s_addr   = inet_addr( ipaddr );
    //connect dnsserver ipaddr and port
    printf( "port:%d\n", bparam.stDdnsParam.nProxyPort );

    if ( connect( rsocket, ( struct sockaddr* )&daddr, sizeof( struct sockaddr ) ) == -1 )
    {
        printf( "connect server is failed\n" );
        CloseSocket( rsocket );
        third_status = 0x02;
        return -1;
    }

    //send register to server
    //printf("regcont:%s\n",regcont);
    iRet = send( rsocket, regcont, strlen( regcont ), 0 );

    if ( iRet == strlen( regcont ) )
    {
        printf( "send server=%d\n", iRet );
    }

    else
    {
        printf( "send error iRet=%d\n", iRet );
    }

    //recive result
    memset( regcont, 0x00, 512 );
    iRet = recv( rsocket, regcont, 512, 0 );
    //printf("recive len:%d buf:%s\n",iRet,regcont);
    DnsRegisterResult( index, regcont );
    //close socket
    CloseSocket( rsocket );
    return 0;
}

void SetDdnsStart( void )
{
    ddnsflag = 0x00;
}

//third ddns proc
void* ThirdDnsProc( void* p ) 	//ddns register
{
    unsigned short times = 0;
    int iRet;
    sleep( 6 );
    printf( "start run ddns\n" );

    while ( 1 )
    {
        if ( ddnsflag == 0x00 )
        {
            iRet = ThirdDnsRegister( bparam.stDdnsParam.byDdnsSvr );
            times = 0x00;

            if ( iRet != 0x00 )
            {
                ddnsflag = 0x00;
            }

            else
            {
                ddnsflag = 0x01;
            }
        }

        if ( times++ >= 60 )
        {
            ddnsflag = 0x00;
        }

        sleep( 10 );
    }

    return NULL;
}

void DnsInit( void )
{
}

void DnsThread( void )
{
    pthread_t               ddnsthread;
//    pthread_create( &ddnsthread, 0, ThirdDnsProc, NULL );
}

