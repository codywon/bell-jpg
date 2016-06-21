#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "cmdhead.h"
#include "param.h"

char	upnpflag = 0x01;
int upnpp2pport = -1;
int bupnpp2pport = -1;

sem_t	p2pportsem;

void PnpStop( void )
{
    bparam.stUpnpParam.byEnable = 0x00;
    upnpflag = 0x00;
}

void* PnpProc( void* p )
{
    int iRet = 0;
    unsigned short times = 0;

    while ( 1 )
    {
        //printf("upnpflag=%d  enable = %d\n",upnpflag,bparam.stUpnpParam.byEnable);
        if ( upnpflag == 0x01 )
        {
            if ( bparam.stUpnpParam.byEnable == 0x01 )
            {
                iRet = UPnPStart();
                printf( "iRet %d\n", iRet );

                if ( iRet == 0x00 )
                {
                    times = 0x00;
                }
            }
        }

        sleep( 30 );
        //printf("upnp times:%d\n",times);
        times++;

        if ( times >= 60 * 10 )
        {
            times = 0;
            upnpflag = 0x00;
        }
    }
}



int UPnPStart( void )
{
    int iRet = -1;

    if ( bparam.stUpnpParam.byEnable == 0x01 )
    {
        char temp[256];
        //printf("start upnp web...\n");
        memset( temp, 0x00, 256 );
        sprintf( temp, "/system/system/bin/upnpc-static -a %s %d %d TCP", bparam.stNetParam.szIpAddr, bparam.stNetParam.nPort,
                 bparam.stNetParam.nPort );
        iRet = DoSystem( temp );
        printf( "iRet %d upnp:%s\n", iRet, temp );

        if ( iRet == 0 )
        {
            upnpflag = 0x00;
            bparam.stStatusParam.upnpstat = 0x01;
        }

        else
        {
            bparam.stStatusParam.upnpstat = 0x03;
        }

#if 0
        memset( temp, 0x00, 256 );
        sprintf( temp, "/system/system/bin/upnpc-static -d %d UDP", bupnpp2pport );
        iRet = DoSystem( temp );
        memset( temp, 0x00, 256 );
        sprintf( temp, "/system/system/bin/upnpc-static -a %s %d %d UDP", bparam.stNetParam.szIpAddr, upnpp2pport, upnpp2pport );
        iRet = DoSystem( temp );
#endif
#if 0
        //printf("start upnp cmd...\n");
        memset( temp, 0x00, 256 );
        sprintf( temp, "upnpc-static -a %s %d %d TCP 1\n", bparam.stNetParam.szIpAddr, bparam.stNetParam.CmdPort,
                 bparam.stNetParam.CmdPort );
        iRet += DoSystem( temp );
        //printf("cmd:%s\n",temp);
        //printf("start upnp data...\n");
        memset( temp, 0x00, 256 );
        sprintf( temp, "upnpc-static -a %s %d %d TCP 2\n", bparam.stNetParam.szIpAddr, bparam.stNetParam.DatPort,
                 bparam.stNetParam.DatPort );
        iRet += DoSystem( temp );
        memset( temp, 0x00, 256 );
        sprintf( temp, "upnpc-static -a %s %d %d TCP\n", bparam.stNetParam.szIpAddr, bparam.stNetParam.rtspport,
                 bparam.stNetParam.rtspport );
        iRet += DoSystem( temp );
        myWriteLogIp( 0x7f000001, "UPnP is start..." );
        upnpflag = 0x01;
#endif
        //printf("data:%s\n",temp);
    }

    return iRet;
}

void NoteP2pPort( void )
{
    sem_post( &p2pportsem );
}

void* PnpP2pProc( void* p )
{
    int iRet = 0;
    char temp[256];

    if ( ( sem_init( &p2pportsem, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        //printf("upnpflag=%d  enable = %d\n",upnpflag,bparam.stUpnpParam.byEnable);
        sem_wait( &p2pportsem );

        if ( bupnpp2pport > 0 )
        {
            memset( temp, 0x00, 256 );
            sprintf( temp, "/system/system/bin/upnpc-static -d %d UDP", bupnpp2pport );
            iRet = DoSystem( temp );
        }

        if ( upnpp2pport > 0 )
        {
            memset( temp, 0x00, 256 );
            sprintf( temp, "/system/system/bin/upnpc-static -a %s %d %d UDP", bparam.stNetParam.szIpAddr, upnpp2pport, upnpp2pport );
            iRet = DoSystem( temp );
        }
    }
}


void UpnpInit( void )
{
}

void UpnpThread( void )
{
    pthread_t       pnpthread;
    pthread_t       pnpthread1;

    if ( bparam.stIEBaseParam.sysmode == 0x00 )
    {
        upnpflag = 0x01;
    }

    bparam.stUpnpParam.byEnable = 0x01;
//    pthread_create( &pnpthread, 0, &PnpProc, NULL );
//   pthread_create( &pnpthread, 0, &PnpP2pProc, NULL );
}

