

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

#include "cmdhead.h"
#include "param.h"
#include "debug.h"



void StaModeGetWifiMac( char* pdst, char sit ) 		//get wifi mac
{
    memset( WifiResult[sit].mac, 0x00, 20 );
	memcpy( WifiResult[sit].mac, pdst + 9, 18 );
}

void StaModeGetESSID( char* pdst, char sit ) 		//get ssid
{
    unsigned char i;
    memset( WifiResult[sit].ssid, 0x00, 64 );
    for ( i = 0; i < 64; i++ )
    {
        if ( pdst[i + 7] == 0x22 )
        {
            break;
        }

        WifiResult[sit].ssid[i] = pdst[i + 7];
    }
}

void StaModeGetWifiMode( char* pdst, char sit ) 		//get wifi Mode
{
    unsigned char i;

    memset( WifiResult[sit].mode, 0x00, 8 );
    for ( i = 0; i < 64; i++ )
    {
        if ( pdst[i + 5] < 0x30 )
        {
            break;
        }

        WifiResult[sit].mode[i] = pdst[i + 5];
    }

    //memcpy(WifiResult[sit].mode,pdst + 5,8);
    if ( WifiResult[sit].mode[0] == 0x4d )
    {
        WifiResult[sit].modetype = 0x00;
    }

    else
    {
        WifiResult[sit].modetype = 0x01;
    }
}

void StaModeGetQuantMode( char* pdst, char sit ) 		//get wifi signal quant
{
    unsigned char i;
    unsigned char j = 0;

    memset( WifiResult[sit].db0, 0x00, 4 );
    memset( WifiResult[sit].db1, 0x00, 4 );
    for ( i = 0; i < 4; i++ )
    {
        j++;

        if ( pdst[i + 8] < 0x30 )
        {
            break;
        }

        WifiResult[sit].db0[i] = pdst[i + 8];
    }

    for ( i = 0; i < 4; i++ )
    {
        if ( pdst[i + 8 + j] < 0x30 )
        {
            break;
        }

        WifiResult[sit].db1[i] = pdst[i + 8 + j];
    }
}


int StaModeParseWifiScan( char* pbuf, int sit ) 		//parse wifi scan result
{
    char* pdst = NULL;
    char auth = 0, ency = 0, key = 0;

    pdst = strstr( pbuf, "Address: " );

    if ( pdst != NULL )
    {
        StaModeGetWifiMac( pdst, sit );
    }

    pdst = strstr( pbuf, "ESSID:" );

    if ( pdst != NULL )
    {
        StaModeGetESSID( pdst, sit );
    }

    pdst = strstr( pbuf, "Mode:" );

    if ( pdst != NULL )
    {
        StaModeGetWifiMode( pdst, sit );
    }

    pdst = strstr( pbuf, "Quality=" );

    if ( pdst != NULL )
    {
        StaModeGetQuantMode( pdst, sit );
    }

    pdst = strstr( pbuf, "Encryption key:" );

    if ( pdst != NULL )
    {
        pdst += 16;

        if ( pdst[0] == 0x6e )
        {
            key = 1;
        }

        else
        {
            key = 0;
        }
    }

    pdst = strstr( pbuf, "WPA Version" );

    if ( pdst != NULL )
    {
        auth = 1;
    }

    pdst = strstr( pbuf, "WPA2 Version" );

    if ( pdst != NULL )
    {
        auth = 2;
    }

    pdst = strstr( pbuf, "CCMP" );

    if ( pdst != NULL )
    {
        ency = 1;
    }

    pdst = strstr( pbuf, "TKIP" );

    if ( pdst != NULL )
    {
        ency = 2;
    }

    if ( ( ency == 1 ) && ( auth == 1 ) )
    {
        WifiResult[sit].Authtype = 0x02;		//WPA-PSK AES
    }

    if ( ( ency == 1 ) && ( auth == 2 ) )
    {
        WifiResult[sit].Authtype = 0x04;        //WPA2-PSK AES
    }

    if ( ( ency == 2 ) && ( auth == 1 ) )
    {
        WifiResult[sit].Authtype = 0x03;        //WPA-PSK TKIP
    }

    if ( ( ency == 2 ) && ( auth == 2 ) )
    {
        WifiResult[sit].Authtype = 0x05;        //WPA2-PSK TKIP
    }

    if ( ( ency == 0 ) && ( key == 1 ) )
    {
        WifiResult[sit].Authtype = 0x01;        //WEP
    }

    if ( ( ency == 0 ) && ( key == 0 ) )
    {
        WifiResult[sit].Authtype = 0x00;        //WEP-NONE
    }

    return 0;
}


int StaModeGetWifiResult( void )
{
    FILE*    fp = NULL;
    char*    pdst = NULL;
    char    temp[256];
    int    sit = -1;
    char    cell[2048];
    char*    psrc = NULL;

    fp = fopen( "/tmp/wifiscan.txt", "r" );

    if ( fp == NULL )
    {
        printf( "fopen failed\n" );
        return -1;
    }

    fgets( temp, 256, fp );
    memset( temp, 0x00, 256 );
    psrc = cell;

    while ( !feof( fp ) )
    {
        memset( temp, 0x00, 256 );
        fgets( temp, 256, fp );
        pdst = strstr( temp, "Cell" );

        if ( pdst == NULL )
        {
            memcpy( psrc, temp, strlen( temp ) );
            psrc += strlen( temp );
            continue;
        }

        if ( sit >= 0 )
        {
            StaModeParseWifiScan( cell, sit );
        }

        memset( cell, 0x00, 2048 );
        psrc = cell;
        memcpy( psrc, temp, strlen( temp ) );
        psrc += strlen( temp );
        sit++;
        printf( "cell %d\n", sit );

        if ( sit >= 31 )
        {
            break;
        }
    }

    fclose( fp );

    if ( ( sit == -1 ) || ( sit == 255 ) )
    {
        sit = 0x00;
    }

    return sit;
}


int StaModeScanWifi( void ) 	//run wifi scan and save wifiscan.txt
{
    int     iRet = 0;
    char    temp[256];
    FILE*   fp = NULL;

    memset( temp, 0x00, 256 );
    sprintf( temp, "iwlist ra0 scanning > /tmp/wifiscan.txt" );
    iRet = DoSystem( temp );

    DoSystem("sync");
    
    fp = fopen( "/tmp/wifiscan.txt", "ab+" );

    if ( fp == NULL )
    {
        printf( "fopen failed\n" );
        return -1;
    }

    memset( temp, 0x00, 256 );
    sprintf( temp, "Cell\n" );
    fwrite( temp, 6, 1, fp );
    fclose( fp );
    return iRet;
}

int StaModeGetWifiScan( void )
{
    int wifiPrevStatus = 0;
    int 	iRet = 0;

    wifilock();
    WifiResultInit();

    Textout("Sta mode scan wifi start");
    iRet = StaModeScanWifi();

    Textout("Sta mode scan wifi end");
    Textout("Sta mode parse scan result...");
    if ( iRet == 0 )
    {
        wificnt = StaModeGetWifiResult();
        Textout( "Sta mode scan wifi cnt=%d", wificnt );
    }

    wifiunlock();

    return iRet;
}


void IpcStaMode()
{
	Textout("*************IpcStaMode***************************");
    NotifyToDaemon( 11, &bparam, sizeof( IEPARAMLIST ) );
    geWifiMode = eStaMode;
}

void IpcStaModeReconn()
{
	Textout("*************IpcStaMode***************************");
    NotifyToDaemon( 12, &bparam, sizeof( IEPARAMLIST ) );
    geWifiMode = eStaMode;
}


