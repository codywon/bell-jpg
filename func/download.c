#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "debug.h"

#ifdef  AUTO_DOWNLOAD_FIRMWARE
#include "download.h"

#if 0
#define TiNetTextout(Fmt, args...)
#else
#define TiNetTextout Textout
#endif

///////////////////////////////////////////////////////////////////////////
//
//  Library
//
///////////////////////////////////////////////////////////////////////////

static TiNetHandler tiNetHandler;

Int32 TiNetUdpSend( char *server_ip, Int8* data, UInt32 len )
{
    int iRet = 0;
    struct sockaddr_in serveraddr;

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons( REQUEST_SERVER_PORT );
    serveraddr.sin_addr.s_addr = inet_addr(server_ip/*REQUEST_SERVER_IP_ADDRESS*/);

    iRet = sendto( tiNetHandler.socket, data, len, 0, ( struct sockaddr* )&serveraddr, sizeof( struct sockaddr_in )  );

    return iRet;
}

static Int32 TiNetUdpRecv( Int32 sockfd, Int8* data, Int32 len )
{
    int iRet = 0;
    struct sockaddr_in serveraddr;
    int addr_len = sizeof(struct sockaddr_in);

    iRet = recvfrom( sockfd, data, len, 0, ( struct sockaddr* )&serveraddr, &addr_len );
    if ( iRet <= 0 )
    {
        return -1;
    }

    //strcpy(pHead->srcIP, inet_ntoa(serveraddr.sin_addr));
    //pHead->srcPort = ntohs(serveraddr.sin_port);
    TiNetTextout("client ipaddr=%s, port=%d", inet_ntoa(serveraddr.sin_addr), ntohs(serveraddr.sin_port) );
    return iRet;
}

Void TiNetSocketClose( Int32 handler)
{
    if ( handler == -1 )
    {
        TiNetTextout("The handler of socket maybe closed");
        return;
    }

    TiNetTextout("Close socket=%d",handler);
    shutdown( handler, 2 );
    close( handler );
}


static Int32 TiNetSocketCreate( UInt16 port )
{
    Int32 iRet = 0;
    Int32 tiNetSocket = -1;
    struct sockaddr_in addr;

    tiNetSocket = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( tiNetSocket == -1 )
    {
        TiNetTextout( "Create socket fail, port=%d", port );
        return tiNetSocket;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons( port );
    addr.sin_addr.s_addr = INADDR_ANY;
    iRet = bind( tiNetSocket, ( struct sockaddr* )&addr, sizeof( addr ) );
    if ( iRet < 0 )
    {
        TiNetTextout( "Bind socket fail, port=%d", port);
        TiNetSocketClose( tiNetSocket);
        return -1;
    }

    TiNetTextout( "Bind socket(port=%d, socket=%d) success", port, tiNetSocket);
    return tiNetSocket;
}

Void* TiNetProc( Void* p )
{
    Int32 iRet = 0;
    Int8 buffer[MAX_TI_NET_PACKET_SIZE];

    Int32 maxFd = 0;
    fd_set readSet;
    struct timeval tv;

    while ( 1 )
    {
        FD_ZERO(&readSet);

        if(tiNetHandler.port > 0)
        {
            maxFd = tiNetHandler.socket;
            FD_SET(tiNetHandler.socket,&readSet);
        }

        tv.tv_sec = REQUEST_TIMEOUT;
        tv.tv_usec = 0;
        iRet = select(maxFd+1,&readSet,NULL,NULL,&tv);
        switch(iRet)
        {
            case -1:
                perror("select error");
                break;
            case 0:
                break;
            default:
                if(tiNetHandler.port > 0)
                {
                    if(FD_ISSET(tiNetHandler.socket, &readSet))
                    {
                        iRet = TiNetUdpRecv(tiNetHandler.socket, buffer, MAX_TI_NET_PACKET_SIZE);
                        if(iRet < 0)
                            break;
                        if(tiNetHandler.callback != NULL)
                            tiNetHandler.callback(buffer, iRet);
                    }
                }
                break;
        }
    }
}

Int32 TiNetAddListener(UInt16 port, TiNetCallback callback)
{
    Int32 iRet = 0;

    memset(&tiNetHandler, 0, sizeof(TiNetHandler));
    iRet = TiNetSocketCreate(port);
    tiNetHandler.port = port;
    tiNetHandler.socket = iRet;
    tiNetHandler.callback = callback;

    pthread_t           TiNetThread;
    iRet = pthread_create( &TiNetThread, 0, TiNetProc, NULL );
    if(iRet != 0)
    {
        TiNetTextout("Create TiNetProc fail, iRet=%d", iRet);
    }
    else
    {
        TiNetTextout("Create TiNetProc Success!!!");
    }

    return iRet;
}

///////////////////////////////////////////////////////////////////////////
//
//  Application
//
///////////////////////////////////////////////////////////////////////////
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include "param.h"

void ProcessDownloadFile(int nTotalSize)
{
	FILE *f = fopen("/tmp/download.bin", "rb");
	if ( f == NULL )
	{
		Textout("Not Found download.bin");
		return;
	}

	/////////////////////////////////////////////////////
	//
	//Process .....Copy or mv and etc.
	//
	/////////////////////////////////////////////////////

	TDownloadFileHead head;
	fread(&head, 1, sizeof(TDownloadFileHead), f);
	if ( head.nStartByte != REQUEST_START_BYTE || head.nTotalSize != nTotalSize )
	{
		Textout("download.bin Format ERROR");
		fclose(f);
		DoSystem("rm /tmp/download.bin");
		return;
	}

	int i = 0, k = 0;
	int nItemCount = head.nFileItemCount;
	int nPos1 = sizeof(TDownloadFileHead);
	int nPos2 = sizeof(TDownloadFileHead) + nItemCount * sizeof(TDownloadFileItem);
	char command[128] = {0};
	TDownloadFileItem item;	
	FILE *f2 = NULL;
	for ( i = 0; i < nItemCount; i++, nPos1 += sizeof(TDownloadFileItem) )
	{
		fseek( f, nPos1, SEEK_SET );
		fread(&item, 1, sizeof(TDownloadFileItem), f);
		switch(item.nOperator)
		{
			case DOP_KILL:		//killall:
				sprintf(command, "killall %s", item.szPath);
				Textout("Do-->[%s]", command);
				DoSystem(command);
				break;
			case DOP_MOVE:		//rm -rf 
				sprintf(command, "rm -rf %s", item.szPath);
				Textout("Do-->[%s]", command);
				DoSystem(command);
				break;
			case DOP_MKDIR:		//mkdir -p 
				sprintf(command, "mkdir -p %s", item.szPath);
				Textout("Do-->[%s]", command);
				DoSystem(command);
				break;
			case DOP_BACKUP:	//copy this file to /tmp/filename, then restore it
				sprintf(command, "cp %s /tmp/backup.%d.bak", item.szPath, item.nFileSize);
				Textout("Do-->[%s]", command);
				DoSystem(command);
				break;
			case DOP_RESTORE:	//restore it
				sprintf(command, "mv -f /tmp/backup.%d.bak %s", item.nFileSize, item.szPath);
				Textout("Do-->[%s]", command);
				DoSystem(command);
				break;

			case DOP_REBOOT:
				goto REBOOT_NOW;
				
			case DOP_REPLACE:	//replace current file, then chmod a+x
			case DOP_REPLACEMOD:
				fseek( f, nPos2, SEEK_SET );
				nPos2 += item.nFileSize;
				f2 = fopen("/tmp/tempfile", "wb");
				if ( f2 != NULL )
				{
					int nWriteBytes = 0;
					int nTotalWriteBytes = 0;
					char pbuffer[1024];
					while ( nTotalWriteBytes < item.nFileSize )
				    {
				        nWriteBytes = fread( pbuffer, 1, 1024, f );
				        fwrite( pbuffer, 1, nWriteBytes, f2 );
				        nTotalWriteBytes += nWriteBytes;
				    }
					fclose(f2);
					sprintf(command, "mv -f /tmp/tempfile %s", item.szPath);
					Textout("Do-->[%s]", command);
					DoSystem(command);

					if ( item.nOperator == DOP_REPLACEMOD )
					{
						sprintf(command, "chmod a+x %s", item.szPath);
						Textout("Do-->[%s]", command);
						DoSystem(command);
					}
				}
				else
				{
					Textout("Replace File [%s] ERROR", item.szPath);
					fclose(f);
					DoSystem("rm /tmp/download.bin");
					return;
				}
				break;
		}
	}
	
	/////////////////////////////////////////////////////
	//
	//Process Over.....
	//
	/////////////////////////////////////////////////////
	
REBOOT_NOW:

	fclose(f);
	Textout("Download Over, Success...");
	
	DoSystem("cp /tmp/gps.txt /system/www/gps.txt");	//Silice Reboot
	
	sleep(1);
	Textout("Download Success, So Reboot....");
    SetRebootCgi();
}

int DownloadFirmware(char *server_ip, int nPort, char *file_path)
{
    struct  sockaddr_in daddr;
    int     sock;
    int     iRet = 0;
    //char  szBuffer[1024] = {"GET /FM/system/TH-sys-48.2.64.178.zip HTTP/1.1\r\nHost:cd.gocam.so\r\nConnection:Close\r\n\r\n"};
    char    szBuffer[1024] = {0};
    struct  timeval    TimeOut;
    int nFlag;

    FILE* file = NULL;
    int recvlen = 0;

    sprintf( szBuffer, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection:Close\r\n\r\n",
             file_path,
             server_ip );
    Textout( "szBuffer = [%s]", szBuffer );

    sock = socket( AF_INET, SOCK_STREAM, 0 );
    bzero( &daddr, sizeof( struct sockaddr_in ) );

    nFlag = 1;
    setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, ( void* )&nFlag, sizeof( int ) );
	
    TimeOut.tv_sec = 30;
    TimeOut.tv_usec = 0;
    setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, ( char* )&TimeOut, sizeof( TimeOut ) );


    if ( iRet != 0 )
    {
        CloseSocket( sock );
        return -1;
    }

    daddr.sin_family        = AF_INET;
    daddr.sin_port          = htons( nPort );
    daddr.sin_addr.s_addr   = inet_addr( server_ip );

    if ( connect( sock, ( struct sockaddr* )&daddr, sizeof( struct sockaddr ) ) == -1 )
    {
        Textout( "connect download server failed\n" );
        CloseSocket( sock );
        return -1;
    }

    send( sock, szBuffer, strlen( szBuffer ), 0 );

    DoSystem( "rm -f /tmp/download.bin" );
    file = fopen( "/tmp/download.bin", "wb" );

    if ( file == NULL )
    {
        printf( "dbg file open failed\n" );
        return -1;
    }

    while ( 1 )
    {
        iRet = recv( sock, szBuffer, 1024, 0 );

        if ( iRet > 0 )
        {
            recvlen += iRet;
            fwrite( szBuffer, 1, iRet, file );
        }

        else
        {
            break;
        }
    }

    printf( "download file recv len=%d\n", recvlen );
    fclose( file );

    CloseSocket( sock );

	ProcessDownloadFile(recvlen);

	return 0;
}

Int32 OnRequestACK(Int8* buffer, UInt32 len)
{
    if ( len != sizeof(pTRequestACK) )
    {
        return -1;
    }

    pTRequestACK p = (pTRequestACK)buffer;
    if ( p->nStartByte == REQUEST_START_BYTE )
    {
        if ( p->szDownloadPath[0] != 0 )
        {
            //Will Download
            DownloadFirmware(p->szDownloadServer, p->nDownloadPort, p->szDownloadPath);
        }
    }
    return 0;
}

void GetDownloadServerIPAddress(char *server_ip)
{
	FILE *f = fopen("/system/system/bin/SERVER","rb");
	if ( f == NULL )
	{
		strcpy(server_ip, REQUEST_SERVER_IP_ADDRESS);
	}
	else
	{
		fread(server_ip, 1, 32, f);
		fclose(f);
	}
}

void *RequestProc(void *p)
{
    TRequest request;
    memset(&request, 0, sizeof(TRequest));
    request.nStartByte = REQUEST_START_BYTE;
    request.nMajorVersion = GetMajorVersion();
    request.nMinorVersion = GetMinorVersion();
    request.nCustomerID = CUSTOMER_ID;
    if ( bparam.stIEBaseParam.dwDeviceID[0] != 0 )
        strcpy(request.szID, bparam.stIEBaseParam.dwDeviceID);

	char server_ip[32] = {0};
	GetDownloadServerIPAddress(server_ip);

    while(1)
    {
        sleep(REQUEST_FREQUENCY);   //ONE HOUR Request Once
        TiNetUdpSend( server_ip, (Int8 *)&request, sizeof(TRequest));
    }
}

///////////////////////////////////////////////////////////////////////////
//
//  Application EnterPoint
//
///////////////////////////////////////////////////////////////////////////

void InitDownloadModule()
{
    pthread_t   thread_id;
    pthread_create( &thread_id, 0, &RequestProc, NULL );
    pthread_detach(thread_id);

    TiNetAddListener(REQUEST_LOCAL_PORT, OnRequestACK);
}

#endif

