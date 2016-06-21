
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


int ApModeScanWifi( void ) 	//run wifi scan and save wifiscan.txt
{
    int     iRet = 0;
    char    temp[256];
    FILE*   fp = NULL;
    
    memset( temp, 0x00, 256 );
    sprintf( temp, "iwpriv ra0 set SiteSurvey=1" );
    iRet = DoSystem( temp );

    memset( temp, 0x00, 256 );
    sprintf( temp, "iwpriv ra0 get_site_survey > /tmp/wifiscan.txt" );
    iRet = DoSystem( temp );

    DoSystem("sync");
    
    fp = fopen( "/tmp/wifiscan.txt", "ab+" );
    if ( fp == NULL )
    {
        printf( "fopen failed\n" );
        return -1;
    }

    fclose( fp );
    return iRet;
}

int GetFileLineEx(const char *pInputName, char *pOutputBuf, int cnt)  
{  
    FILE * fp = NULL;  
    int i=0;  
    char * line = NULL;  
    size_t len = 0;  
    ssize_t read;  
  
    fp = fopen(pInputName, "r");  
    if (fp == NULL)  
     return -1;  
  
    if(cnt<=0)  
         return -2;  
           
    while ((read = getline(&line, &len, fp)) != -1) {  
        ++i;  
        if(i>=cnt)  
            break;  
    }  

    fclose(fp);

    if (line && read > 1)  
    {  
        memcpy(pOutputBuf,line,read);  
        free(line);  
        return 0;   
    }  
  
    return -3;  
} 

int FindSpaceFromStart( char* pbuf, int startPos )
{
    int sit = 0;
    int iRet = -1;
    char* pos = pbuf + startPos;

    //find space
    while ( sit < (256 - startPos -1) )
    {
        if ( *pos == 0x20 )
        {
            iRet = 0;
            break;
        }

        pos++;
        sit++;
    }

    if ( iRet )
    {
        return -1;
    }

    else
    {
        return sit;
    }
}

int FindSpaceFromEnd( char* pbuf, int startpos, int endpos )
{
    int iRet = -1;
    char* pos = pbuf;

    //find space
    while ( endpos > startpos )
    {
        if ( *pos != 0x20 )
        {
            iRet = 0;
            break;
        }

        pos--;
        endpos--;
    }

    if ( iRet )
    {
        return -1;
    }

    else
    {
        endpos+=1;
        return endpos - startpos;
    }
}


int ApModeGetWifiResult( void )
{
    int iRet = 0;
    char            Authtype[32];   //auth
    char            channel[4];
    
    const char* rfile = "/tmp/wifiscan.txt";
    char  strline[256];
    int nline = 0;
    int nssid = 0;
    int nSpace = 0;

    char* pPos = NULL;
    int nPosCh = 0;
    int nPosSSID = 0;
    int nPosBSSID = 0;
    int nPosSecurity = 0;
    int nPosSignal = 0;
    

    memset(strline, 0, 256);
    nline = 2;
    if( (iRet = GetFileLineEx(rfile, strline, nline)) < 0)
    {
        return 0;
    }

    //Textout("Line 1: %s", strline);
    if( (pPos = strstr(strline, "Ch")) == NULL)
    {
        Textout("Not find Ch");
        return 0;
    }
    nPosCh = pPos - strline;
    
    if( (pPos = strstr(strline, "SSID")) == NULL)
    {
        Textout("Not find SSID");
        return 0;
    }
    nPosSSID = pPos - strline;
    
    if( (pPos = strstr(strline, "BSSID")) == NULL)
    {
        Textout("Not find BSSID");
        return 0;
    }
    nPosBSSID = pPos - strline;
    
    if( (pPos = strstr(strline, "Security")) == NULL)
    {
        Textout("Not find Security");
        return 0;
    }
    nPosSecurity = pPos -strline;
    
    if( (pPos = strstr(strline, "Siganl")) == NULL)
    {
        Textout("Not find Siganl");
        return 0;
    }
    nPosSignal = pPos - strline;

    //Textout("nPosCh=%d, nPosSSID=%d, nPosBSSID=%d, nPosSecurity=%d, nPosSignal=%d", nPosCh, nPosSSID, nPosBSSID, nPosSecurity, nPosSignal);


    memset(strline, 0, 256);
    nssid = 0;
    nline = 3;
    while( ( iRet = GetFileLineEx(rfile, strline, nline) ) == 0)
    {
        memset(&WifiResult[nssid], 0, sizeof(WifiCont));

        //Textout("Index=%d", nssid);
        //get channel
        nSpace = FindSpaceFromStart(strline, nPosCh);
        if(nSpace != -1)
        {
            memset(channel, 0, 4);
            memcpy(channel, strline+nPosCh, nSpace);
            WifiResult[nssid].channel = atoi(channel);
            //Textout("channel=%s(%d)", channel, WifiResult[nssid].channel);
        }
        
        //get ssid
        nSpace = FindSpaceFromEnd(strline + nPosBSSID - 1, nPosSSID, nPosBSSID - 1);
        if(nSpace != -1)
        {
            memcpy(WifiResult[nssid].ssid, strline + nPosSSID, nSpace);
            //Textout("ssid=%s", WifiResult[nssid].ssid);
        }
        else
        {
            Textout("Unsurpport the ssid, go next line");
            nline++;
            continue;
        }
        
        //get bssid
        nSpace = FindSpaceFromStart(strline, nPosBSSID);
        if(nSpace != -1)
        {
            memcpy(WifiResult[nssid].mac, strline + nPosBSSID, nSpace);
            //Textout("mac=%s", WifiResult[nssid].mac);
        }

        //get security
        nSpace = FindSpaceFromStart(strline, nPosSecurity);
        if(nSpace != -1)
        {
            int ency = 0;
            int key = 0;
            int auth = 0;
            char* pdst = NULL;
            memset(Authtype, 0, 32);
            memcpy(Authtype, strline + nPosSecurity, nSpace);

            //Textout("Security=%s", Authtype);
            pdst = strstr( Authtype, "NONE" );

            if ( pdst != NULL )
            {
                ency = 0x00;
                key == 0x00;
            }

            pdst = strstr( Authtype, "WEP" );

            if ( pdst != NULL )
            {
                ency = 0x00;
                key == 0x01;
            }

            pdst = strstr( Authtype, "WPAPSK" );

            if ( pdst != NULL )
            {
                auth = 0x01;
            }

            pdst = strstr( Authtype, "WPA2PSK" );

            if ( pdst != NULL )
            {
                auth = 0x02;
            }

            pdst = strstr( Authtype, "AES" );

            if ( pdst != NULL )
            {
                ency = 0x01;
            }

            pdst = strstr( Authtype, "TKIP" );

            if ( pdst != NULL )
            {
                ency = 0x02;
            }

            if ( ( ency == 1 ) && ( auth == 1 ) )
            {
                WifiResult[nssid].Authtype = 0x02;        //WPA-PSK AES
            }

            if ( ( ency == 1 ) && ( auth == 2 ) )
            {
                WifiResult[nssid].Authtype = 0x04;        //WPA2-PSK AES
            }

            if ( ( ency == 2 ) && ( auth == 1 ) )
            {
                WifiResult[nssid].Authtype = 0x03;        //WPA-PSK TKIP
            }

            if ( ( ency == 2 ) && ( auth == 2 ) )
            {
                WifiResult[nssid].Authtype = 0x05;        //WPA2-PSK TKIP
            }

            if ( ( ency == 0 ) && ( key == 1 ) )
            {
                WifiResult[nssid].Authtype = 0x01;        //WEP
            }

            if ( ( ency == 0 ) && ( key == 0 ) )
            {
                WifiResult[nssid].Authtype = 0x00;        //WEP-NONE
            }            
        }

        //get signal
        nSpace = FindSpaceFromStart(strline, nPosSignal);
        if(nSpace != -1)
        {
            memcpy(WifiResult[nssid].db0, strline + nPosSignal, nSpace);
            //Textout("db0=%s", WifiResult[nssid].db0);
        }

        nline++;
        nssid++;
        if(nssid > 31)
        {
            break;
        }

        memset(strline, 0, 256);
    }
    
    //Textout("nssid=%d", nssid);
    return nssid;
}


int ApModeGetWifiScan( void )
{
    int wifiPrevStatus = 0;
    int 	iRet = 0;

    wifilock();
    WifiResultInit();

    iRet = ApModeScanWifi();
    if ( iRet == 0 )
    {
        wificnt = ApModeGetWifiResult();
        Textout( "AP mode scan wifi cnt=%d", wificnt );
    }

    wifiunlock();

    return iRet;
}

void IpcApMode()
{
     NotifyToDaemon( 10, &bparam, sizeof( IEPARAMLIST ) );
     geWifiMode = eApMode;
}



