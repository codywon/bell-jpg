

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <dirent.h>
#include <dirent.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "param.h"

#include "wpslib.h"
#include "debug.h"

#ifdef SUPPORT_WPS


extern char wifiscanflag;

extern void SetWPSState(BOOL state);

static BOOL bRecheckWpsResult = FALSE;
static int nTotalWpsCheckTimes = 0;
#define	MAX_WPS_CHECK_TIMES	120	// 2 Minuters

void SetWifiCheckFunc(int status);

void SetWifiSelect( char *pszSsid, char *pszKey, int pnAuthType )
{
    int i = 0;
    int iRet = 0;
    for ( i = 0; i < wificnt; i++ )
    {
        if ( i > 31 )
        {
            break;
        }

        if ( strcmp(WifiResult[i].ssid, pszSsid) == 0 )
        {
            Textout("Find SSID(%s) in scanning list", pszSsid);
            Textout("channel=%d", WifiResult[i].channel);
            Textout("Authtype=%d", WifiResult[i].Authtype);
            Textout("modetype=%d", WifiResult[i].modetype);
            break;
        }

    }
    
    bparam.stWifiParam.byEnable = 1;
    
    memcpy( bparam.stWifiParam.szSSID, pszSsid, 64);
    bparam.stWifiParam.szSSID[63]=0;
    memcpy( bparam.stWifiParam.szShareKey, pszKey, 64);
    bparam.stWifiParam.szShareKey[63] = 0;
    bparam.stWifiParam.byWifiMode = 0;

    //Channel=11
    bparam.stWifiParam.channel = WifiResult[i].channel;
    
    bparam.stWifiParam.byEncrypt = 0;

    bparam.stWifiParam.byAuthType = pnAuthType;
    bparam.stWifiParam.byKeyFormat = 0;

    bparam.stWifiParam.byDefKeyType = 1;
    bparam.stWifiParam.byKey1Bits = 0;
    bparam.stWifiParam.byKey2Bits = 0;
    bparam.stWifiParam.byKey3Bits = 0;
    bparam.stWifiParam.byKey4Bits = 0;
    bparam.stIEBaseParam.sysmode = 0x01;

    NoteSaveSem();
    
	Textout("Do WPS Func success...");
}

void NoteWpsStart( )
{
    char    temp[128];
    char    ssid[8];
    memset(ssid, 0, 8);
    memset( temp, 0x00, 128 );

    sprintf( temp, "iwpriv ra0 set SSID=\"%s\"", ssid );
    DoSystem( temp );

    //----------------------------
    //stop WifiCheckProc
    netok = 1;
    SetWifiCheckFunc(0);
    //----------------------------
    
	Textout("StartWPS");    
	StartWPS();

	bRecheckWpsResult = TRUE;
	nTotalWpsCheckTimes = 0;
}

void* WPSThreadProc( void* p )
{
	char szSSID[64];
	char szKey[64];
	int nAuthType;
	BOOL bRet = FALSE;

    while ( 1 )
    {
		if ( !bRecheckWpsResult )
		{
			sleep(1);
			continue;
		}
		else
		{
			//Textout("get wps nTotalWpsCheckTimes=%d", nTotalWpsCheckTimes);
			if ( nTotalWpsCheckTimes >= MAX_WPS_CHECK_TIMES )
			{
				Textout("CheckWPSTimes >= 120s, Fail..., Stop Check");
				bRecheckWpsResult = FALSE;
                DoSystem("iwpriv ra0 wsc_stop");
                StartVideoCaptureEx();
                SetWifiCheckFunc(1);
                SetWPSState(FALSE);

                IpcStaMode();

                #ifdef ZILINK
                /*Close LED D2*/
                ControlIO(BELL_WPS_LED, TRUE);
                #endif

                continue;
			}

			bRet = GetWPSResult(szSSID, szKey, &nAuthType);
			if ( bRet && strlen(szSSID) != 0 )
			{
				//Textout("ConfigWSCProfile Success...");
				//Textout("szSSID = [%s], szKey = [%s], nAuthType = %d", szSSID, szKey, nAuthType);

                #ifdef ZILINK
                /*Close LED D2*/
                ControlIO(BELL_WPS_LED, TRUE);
                #endif
                
            
				StartAudioPlay(WF_CONFIGOK, 1, NULL);
			
				
				bRecheckWpsResult = FALSE;
                DoSystem("iwpriv ra0 wsc_stop");

                //==================================
                //wifi scanning
                cgiwaittimeout( 1 );
                wifiscanflag = 0x01;
                cgiwaittimeout( 4 );

                //==================================
                //set wifi
                SetWifiSelect(szSSID, szKey, nAuthType);

                IpcStaModeReconn();
                
                StartVideoCaptureEx();
                SetWifiCheckFunc(1);
                SetWPSState(FALSE);
                
				continue;
			}

			nTotalWpsCheckTimes += 5;
			sleep(5);
			continue;
		}
	}
}

void InitWps()
{
    int iRet = 0;
	pthread_t netthread = 0;
    iRet = pthread_create( &netthread, 0, &WPSThreadProc, NULL );
    if(iRet < 0)
    {
        Textout("ERROR Create WPSThreadProc fail");
    }
}

#endif

