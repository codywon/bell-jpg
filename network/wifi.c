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

#define UCHAR unsigned char
#define USHORT  unsigned short
#define PACKED  __attribute__ ((packed))

#define RT_PRIV_IOCTL                   				(SIOCIWFIRSTPRIV + 0x01)

#define RTPRIV_IOCTL_WSC_PROFILE        			(SIOCIWFIRSTPRIV + 0x12)
#define RT_OID_SYNC_RT61                				0x0D010750
#define RT_OID_WSC_QUERY_STATUS         			((RT_OID_SYNC_RT61 + 0x01) & 0xffff)
#define RT_OID_802_11_WSC_QUERY_PROFILE         0x0750

#define NdisMediaStateConnected                 		1
#define NdisMediaStateDisconnected              		0
#define OID_GEN_MEDIA_CONNECT_STATUS                0x060B

WifiCont                WifiResult[32];
char                    	wificnt = 0;
char				wifiok = 0x00;

//zxh 2013-1-26 for apclient wps pbc
typedef struct  __attribute__( ( packed ) ) _WSC_CONFIGURED_VALUE
{
    unsigned short  WscConfigured; // 1 un-configured; 2 configured
    unsigned char   WscSsid[32 + 1];
    unsigned short  WscAuthMode; // mandatory, 0x01: open, 0x02: wpa-psk, 0x04: shared, 0x08:wpa, 0x10: wpa2, 0x
    unsigned short  WscEncrypType;  // 0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes
    unsigned char   DefaultKeyIdx;
    unsigned char   WscWPAKey[64 + 1];
} WSC_CONFIGURED_VALUE;

typedef struct PACKED _NDIS80211SSID
{
    unsigned int    SsidLength;   // length of SSID field below, in bytes;
    // this can be zero.
    unsigned char   Ssid[32]; // SSID information field
} NDIS80211SSID;

// WSC configured credential
typedef struct  _WSC_CREDENTIAL
{
    NDIS80211SSID    SSID;               // mandatory
    USHORT              AuthType;           // mandatory, 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa-psk2
    USHORT              EncrType;           // mandatory, 1: none, 2: wep, 4: tkip, 8: aes
    UCHAR               Key[64];            // mandatory, Maximum 64 byte
    USHORT              KeyLength;
    UCHAR               MacAddr[6];         // mandatory, AP MAC address
    UCHAR               KeyIndex;           // optional, default is 1
    UCHAR               Rsvd[3];            // Make alignment
} WSC_CREDENTIAL, *PWSC_CREDENTIAL;

#ifndef UINT
#define UINT    unsigned long
#endif

// WSC configured profiles
typedef struct  _WSC_PROFILE
{
    UINT                ProfileCnt;
    UINT                ApplyProfileIdx;  // add by johnli, fix WPS test plan 5.1.1
    WSC_CREDENTIAL      Profile[8];             // Support up to 8 profiles
} WSC_PROFILE;

/* BEGIN: Deleted by wupm, 2014/3/4 */
#if	0
static int CheckWPSOk( void )
{
	if ( ENABLE_AP )
	{
	    char* str;
	    char long_buf[4096];
	    FILE* fp;
	    memset( long_buf, 0, 4096 );

	    if ( !( fp = popen( "iwpriv apcli0 stat", "r" ) ) )
	    {
	        return 0;
	    }

	    fread( long_buf, 1, 4096, fp );
	    pclose( fp );

	    if ( !( str = strstr( long_buf, "WPS Profile Count" ) ) )
	    {
	        return 0;
	    }

	    if ( !( str = strchr( str, '=' ) ) )
	    {
	        return 0;
	    }

	    if ( atoi( str + 1 ) )
	    {
	        return 1;
	    }

	    return 0;
	}
	else
	{
		char* str;
	    char long_buf[4096];
	    FILE* fp;
	    memset( long_buf, 0, 4096 );

	    if ( !( fp = popen( "iwpriv ra0 stat", "r" ) ) )
	    {
	        return 0;
	    }

	    fread( long_buf, 1, 4096, fp );
	    pclose( fp );

	    if ( !( str = strstr( long_buf, "WPS Profile Count" ) ) )
	    {
	        return 0;
	    }

	    if ( !( str = strchr( str, '=' ) ) )
	    {
	        return 0;
	    }

	    if ( atoi( str + 1 ) )
	    {
	        return 1;
	    }

	    return 0;
	}
}

void FindKeyName( char* pdst, char* keyname )
{
    char* ptemp = NULL;

    ptemp = strstr( pdst, "= " );

    if ( ptemp )
    {
        printf( "ptemp:%s len %d\n", ptemp + 2, strlen( ptemp + 2 ) );
        memcpy( keyname, ptemp + 2, strlen( ptemp + 2 ) - 1 );
    }
}

void GetWscChannel( void )
{
    FILE* fp = NULL;
    char temp[256];
    char* pdst = NULL;
    char channel[4];

    DoSystem( "iwconfig apcli0 >/tmp/channel.txt" );
    fp = fopen( "/tmp/channel.txt", "r" );

    if ( fp == NULL )
    {
        return -1;
    }

    while ( !feof( fp ) )
    {
        memset( temp, 0x00, 256 );
        fgets( temp, 256, fp );
        //channel
        pdst = strstr( temp, "Channel=" );

        if ( pdst )
        {
            memset( channel, 0x00, 4 );
            channel[0] = pdst[8];

            if ( pdst[9] != 0x20 )
            {
                channel[1] = pdst[9];
            }

            bparam.stWifiParam.channel = atoi( channel );
            printf( "channel %s=%d\n", channel, bparam.stWifiParam.channel );
        }
    }

    fclose( fp );
}

int AddWscProfileAP( char* ssid, char* auth, char* enc, char* keyindex, char* key )
{
    int iRet = 0;
    int keytype;
    int authtype;
    int enctype;
    char authflag = 0x00;	//0->no 1->wep 2->wpapsk 3->wpa2psk
    char encflag = 0x00;	//0->no 1->share 2->aes 3->tkip
    char* ptemp = NULL;

    memset( bparam.stWifiParam.szSSID, 0x00, 64 );
    memset( bparam.stWifiParam.szShareKey, 0x00, 64 );
    memset( bparam.stWifiParam.szKey1, 0x00, 64 );
    memset( bparam.stWifiParam.szKey2, 0x00, 64 );
    memset( bparam.stWifiParam.szKey3, 0x00, 64 );
    memset( bparam.stWifiParam.szKey4, 0x00, 64 );
    //ssid
    strcpy( bparam.stWifiParam.szSSID, ssid );
    //auth type process for wpa,not for wep or no passwd
    ptemp = strstr( auth, "WPAPSK" );

    if ( ptemp )
    {
        authflag = 0x02;
    }

    ptemp = strstr( auth, "WPA2PSK" );

    if ( ptemp )
    {
        authflag = 0x03;
    }

    //enc aes or tkip,not for wep or no passwd
    ptemp = strstr( enc, "AES" );

    if ( ptemp )
    {
        encflag = 0x02;
    }

    ptemp = strstr( enc, "TKIP" );

    if ( ptemp )
    {
        encflag = 0x03;
    }

    printf( "authflag %d key %d \n", authflag, encflag );

    //wpapsk aes
    if ( ( authflag == 0x02 ) && ( encflag == 0x02 ) )
    {
        bparam.stWifiParam.byAuthType = 0x02;
        strcpy( bparam.stWifiParam.szShareKey, key );
    }

    //wpapsk tkip
    if ( ( authflag == 0x02 ) && ( encflag == 0x03 ) )
    {
        bparam.stWifiParam.byAuthType = 0x03;
        strcpy( bparam.stWifiParam.szShareKey, key );
    }

    //wpa2psk aes
    if ( ( authflag == 0x03 ) && ( encflag == 0x02 ) )
    {
        bparam.stWifiParam.byAuthType = 0x04;
        strcpy( bparam.stWifiParam.szShareKey, key );
    }

    //wpa2psk tkip
    if ( ( authflag == 0x03 ) && ( encflag == 0x03 ) )
    {
        bparam.stWifiParam.byAuthType = 0x05;
        strcpy( bparam.stWifiParam.szShareKey, key );
    }

    printf( "authflag %d key:%s\n", bparam.stWifiParam.byAuthType, bparam.stWifiParam.szShareKey );
    return iRet;
}

int AddWscProfile( WSC_PROFILE* WscProfile )
{
    int iRet = 0;

    memset( bparam.stWifiParam.szSSID, 0x00, 64 );
    memset( bparam.stWifiParam.szShareKey, 0x00, 64 );
    memset( bparam.stWifiParam.szKey1, 0x00, 64 );
    memset( bparam.stWifiParam.szKey2, 0x00, 64 );
    memset( bparam.stWifiParam.szKey3, 0x00, 64 );
    memset( bparam.stWifiParam.szKey4, 0x00, 64 );

    strcpy( bparam.stWifiParam.szSSID, WscProfile->Profile[0].SSID.Ssid );

    switch ( WscProfile->Profile[0].AuthType )
    {
        case 1:		//open
            printf( "add authtype open\n" );
            bparam.stWifiParam.byAuthType = 0;
            bparam.stWifiParam.byDefKeyType = 0;
            break;

        case 2:		//wpa-psk
            printf( "wpa-psk\n" );

            if ( WscProfile->Profile[0].EncrType == 8 )
            {
                bparam.stWifiParam.byAuthType = 2;
            }

            else
            {
                bparam.stWifiParam.byAuthType = 3;
            }

            strcpy( bparam.stWifiParam.szShareKey, WscProfile->Profile[0].Key );
            break;

        case 4:		//share
            bparam.stWifiParam.byAuthType = 1;

            if ( WscProfile->Profile[0].KeyLength == 5 || WscProfile->Profile[0].KeyLength == 13 )
            {
                bparam.stWifiParam.byKeyFormat = 1;
            }

            else if ( WscProfile->Profile[0].KeyLength == 10 || WscProfile->Profile[0].KeyLength == 26 )
            {
                bparam.stWifiParam.byKeyFormat = 0;
            }

            else
            {
                bparam.stWifiParam.byKeyFormat = 1;
            }

            if ( WscProfile->Profile[0].KeyIndex == 1 )
            {
                bparam.stWifiParam.byDefKeyType = 1;
                strcpy( bparam.stWifiParam.szKey1, WscProfile->Profile[0].Key );
            }

            else if ( WscProfile->Profile[0].KeyIndex == 2 )
            {
                bparam.stWifiParam.byDefKeyType = 2;
                strcpy( bparam.stWifiParam.szKey2, WscProfile->Profile[0].Key );
            }

            else if ( WscProfile->Profile[0].KeyIndex == 3 )
            {
                bparam.stWifiParam.byDefKeyType = 3;
                strcpy( bparam.stWifiParam.szKey3, WscProfile->Profile[0].Key );
            }

            else if ( WscProfile->Profile[0].KeyIndex == 4 )
            {
                bparam.stWifiParam.byDefKeyType = 4;
                strcpy( bparam.stWifiParam.szKey4, WscProfile->Profile[0].Key );
            }

            bparam.stWifiParam.byEncrypt = 0x01;
            break;

        case 8:		//wpa
            break;

        case 0x10:	//wpa2
            break;

        case 0x20:	//wpa-psk2
            printf( "wpa2-psk\n" );

            if ( WscProfile->Profile[0].EncrType == 8 )
            {
                bparam.stWifiParam.byAuthType = 4;
            }

            else
            {
                bparam.stWifiParam.byAuthType = 5;
            }

            strcpy( bparam.stWifiParam.szShareKey, WscProfile->Profile[0].Key );
            break;

        default:
            iRet = -1;
            break;
    }

    return iRet;
}

int getWscProfileAP( void )
{
    FILE* fp = NULL;
    char temp[256];
    char* pdst = NULL;
    char  ssid[32];
    char  auth[8];
    char  enc[8];
    char  keyindex[8];
    char  key[32];

    memset( ssid, 0x00, 32 );
    memset( auth, 0x00, 8 );
    memset( enc, 0x00, 8 );
    memset( keyindex, 0x00, 8 );
    memset( key, 0x00, 32 );

    DoSystem( "iwpriv apcli0 stat >/tmp/apcli0.txt" );
    fp = fopen( "/tmp/apcli0.txt", "r" );

    if ( fp == NULL )
    {
        return -1;
    }

    while ( !feof( fp ) )
    {
        memset( temp, 0x00, 256 );
        fgets( temp, 256, fp );
        //SSID
        pdst = strstr( temp, "SSID" );

        if ( pdst )
        {
            FindKeyName( pdst, ssid );
        }

        //AuthType
        pdst = strstr( temp, "AuthType" );

        if ( pdst )
        {
            FindKeyName( pdst, auth );
        }

        //EncrypType
        pdst = strstr( temp, "EncrypType" );

        if ( pdst )
        {
            FindKeyName( pdst, enc );
        }

        //KeyIndex
        pdst = strstr( temp, "KeyIndex" );

        if ( pdst )
        {
            FindKeyName( pdst, keyindex );
        }

        else
        {
            //Key
            pdst = strstr( temp, "Key" );

            if ( pdst )
            {
                FindKeyName( pdst, key );
            }
        }
    }

    fclose( fp );

    printf( "ssid:%s len %d\n", ssid, strlen( ssid ) );
    printf( "auth:%s len %d\n", auth, strlen( auth ) );
    printf( "enc:%s len %d\n", enc, strlen( enc ) );
    printf( "keyindex:%s len %d\n", keyindex, strlen( keyindex ) );
    printf( "key:%s len %d\n", key, strlen( key ) );

    //add ssid auth
    AddWscProfileAP( ssid, auth, enc, keyindex, key );
    //add channel
    GetWscChannel();
    //save param
    NoteSaveSem();
    return 0;
}

int getWscProfile( WSC_PROFILE* wsc_profile )
{
    int socket_id;
    struct iwreq wrq;

    socket_id = socket( AF_INET, SOCK_DGRAM, 0 );
    strcpy( wrq.ifr_name, "ra0" );
    wrq.u.data.length = sizeof( WSC_PROFILE );
    wrq.u.data.pointer = ( caddr_t ) wsc_profile;
    wrq.u.data.flags = RT_OID_802_11_WSC_QUERY_PROFILE;

    if ( ioctl( socket_id, RT_PRIV_IOCTL, &wrq ) == -1 )
    {
        printf( "ioctl error, getWscProfile:%s\n", strerror( errno ) );
        close( socket_id );
        return -1;
    }

    close( socket_id );
    return 0;
}


int ConfigWscProfile( void )
{
	if ( ENABLE_AP )
	{
	    int		iRet;

	    iRet = getWscProfileAP();

	    if ( iRet )
	    {
	        printf( "get wsc profile failed\n" );
	        return -1;
	    }

	    return iRet;
	}
	else
	{
		int		iRet;
	    WSC_PROFILE 	WscProfile;

	    iRet = getWscProfile( &WscProfile );

	    if ( iRet )
	    {
	        printf( "get wsc profile failed\n" );
	        return -1;
	    }

	    printf( "ssid:%s\n", WscProfile.Profile[0].SSID.Ssid );
	    printf( "authmode:%d\n", WscProfile.Profile[0].AuthType );
	    printf( "enctype:%d\n", WscProfile.Profile[0].EncrType );
	    printf( "defaultkey:%s\n", WscProfile.Profile[0].Key );
	    printf( "mac:%02x-%02x-%02x-%02x-%02x-%02x\n", WscProfile.Profile[0].MacAddr[0],
	            WscProfile.Profile[0].MacAddr[1],
	            WscProfile.Profile[0].MacAddr[2],
	            WscProfile.Profile[0].MacAddr[3],
	            WscProfile.Profile[0].MacAddr[4],
	            WscProfile.Profile[0].MacAddr[5] );
	    iRet = AddWscProfile( &WscProfile );

	    if ( iRet == 0x00 )
	    {
	        bparam.stWifiParam.byEnable = 0x01;
	        NoteSaveSem();
	    }

	    printf( "AddWscProfile %d\n", iRet );
	    return iRet;
	}
}

int getWscStatus( void )
{
	if ( ENABLE_AP )
	{
	    int socket_id;
	    struct iwreq wrq;
	    int data = 0;
	    socket_id = socket( AF_INET, SOCK_DGRAM, 0 );
	    strcpy( wrq.ifr_name, "apcli0" );
	    wrq.u.data.length = sizeof( data );
	    wrq.u.data.pointer = ( caddr_t ) &data;
	    wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;

	    if ( ioctl( socket_id, RT_PRIV_IOCTL, &wrq ) == -1 )
	    {
	        perror( "ioctl error" );
	        close( socket_id );
	        return -1;
	    }

	    close( socket_id );
	    return data;
	}
	else
	{
		int socket_id;
	    struct iwreq wrq;
	    int data = 0;
	    socket_id = socket( AF_INET, SOCK_DGRAM, 0 );
	    strcpy( wrq.ifr_name, "ra0" );
	    wrq.u.data.length = sizeof( data );
	    wrq.u.data.pointer = ( caddr_t ) &data;
	    wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;

	    if ( ioctl( socket_id, RT_PRIV_IOCTL, &wrq ) == -1 )
	    {
	        perror( "ioctl error" );
	        close( socket_id );
	        return -1;
	    }

	    close( socket_id );
	    return data;
	}
}

int GetWPSStatus( void )
{
	if ( ENABLE_AP )
	{
	    int iRet;
	    int status = 0;

	    iRet = getWscStatus();

	    switch ( iRet )
	    {
	        case 0: /* Not used*/
	        case 1: /* Idle */
	            status = 0x00;	//no action
	            break;

	        case 2: /* WSC Fail */
	            status = 1;	//wps failed
	            break;

	        case 34: /* Configured */
	            status = 2;	//wps ok
	            break;

	        case 0x109:
	            status = 3;	//wps overlap detection
	            break;

	        default:
	            if ( iRet == 3 || ( iRet >= 10 && iRet <= 26 ) )
	            {
	                status = 4;	//wps process
	            }

	            else
	            {
	                status = 5;	//wps undefine status
	            }

	            break;
	    }

	    return status;
	}
	else
	{
		int iRet;
	    int status = 0;

	    iRet = getWscStatus();

	    switch ( iRet )
	    {
	        case 0: /* Not used*/
	        case 1: /* Idle */
	            status = 0x00;	//no action
	            break;

	        case 2: /* WSC Fail */
	            status = 1;	//wps failed
	            break;

	        case 34: /* Configured */
	            status = 2;	//wps ok
	            break;

	        case 0x109:
	            status = 3;	//wps overlap detection
	            break;

	        default:
	            if ( iRet == 3 || ( iRet >= 10 && iRet <= 26 ) )
	            {
	                status = 4;	//wps process
	            }

	            else
	            {
	                status = 5;	//wps undefine status
	            }

	            break;
	    }

	    return status;
	}
}
#endif


//wifi config
#if	0
void WifiApConfig( void )
{
    FILE*    fp = NULL;
    char    temp[256];
    char     wifimac[16];
    int		len = 0;

    DoSystem( "mkdir -p /etc/Wireless/RT2860" );
    fp = fopen( "/etc/Wireless/RT2860/RT2860.dat", "wb" );

    if ( fp == NULL )
    {
        printf( "Wifi config failed\n" );
        return;
    }

    fwrite( "Default\n", 1, strlen( "Default\n" ), fp );
    fwrite( "CountryRegion=5\n", 1, strlen( "CountryRegion=5\n" ), fp );
    fwrite( "CountryRegionABand=7\n", 1, strlen( "CountryRegionABand=7\n" ), fp );
    fwrite( "CountryCode=\n", 1, strlen( "CountryCode=\n" ), fp );
    fwrite( "BssidNum=1\n", 1, strlen( "BssidNum=1\n" ), fp );

    if ( strlen( bparam.stWifiRoute.szSSID ) == 0x00 )
    {
        char mac[20];
        memset( mac, 0x00, 20 );
        mac[6] = bparam.stIEBaseParam.szWifiMac[16];
        mac[5] = bparam.stIEBaseParam.szWifiMac[15];
        mac[4] = bparam.stIEBaseParam.szWifiMac[13];
        mac[3] = bparam.stIEBaseParam.szWifiMac[12];
        mac[2] = 'M';
        mac[1] = 'A';
        mac[0] = 'C';
        //memcpy(mac,bparam.stIEBaseParam.szMac,17);
        sprintf( temp, "SSID1=%s\n", mac );
        fwrite( temp, 1, strlen( temp ), fp );
        sprintf( bparam.stWifiRoute.szSSID, "%s", mac );
    }

    else
    {
        sprintf( temp, "SSID1=%s\n", bparam.stWifiRoute.szSSID );
        fwrite( temp, 1, strlen( temp ), fp );
    }

    fwrite( "SSID2=\n", 1, strlen( "SSID2=\n" ), fp );
    fwrite( "SSID3=\n", 1, strlen( "SSID3=\n" ), fp );
    fwrite( "SSID4=\n", 1, strlen( "SSID4=\n" ), fp );
    fwrite( "SSID5=\n", 1, strlen( "SSID5=\n" ), fp );
    fwrite( "SSID6=\n", 1, strlen( "SSID6=\n" ), fp );
    fwrite( "SSID7=\n", 1, strlen( "SSID7=\n" ), fp );
    fwrite( "SSID8=\n", 1, strlen( "SSID8=\n" ), fp );
    fwrite( "WirelessMode=9\n", 1, strlen( "WirelessMode=9\n" ), fp );
    fwrite( "FixedTxMode=\n", 1, strlen( "FixedTxMode=\n" ), fp );
    fwrite( "TxRate=0\n", 1, strlen( "TxRate=0\n" ), fp );
    fwrite( "Channel=1\n", 1, strlen( "Channel=1\n" ), fp );
    fwrite( "BasicRate=15\n", 1, strlen( "BasicRate=15\n" ), fp );
    fwrite( "BeaconPeriod=100\n", 1, strlen( "BeaconPeriod=100\n" ), fp );
    fwrite( "DtimPeriod=1\n", 1, strlen( "DtimPeriod=1\n" ), fp );
    fwrite( "TxPower=100\n", 1, strlen( "TxPower=100\n" ), fp );
    fwrite( "DisableOLBC=0\n", 1, strlen( "DisableOLBC=0\n" ), fp );
    fwrite( "BGProtection=0\n", 1, strlen( "BGProtection=0\n" ), fp );
    fwrite( "TxAntenna=\n", 1, strlen( "TxAntenna=\n" ), fp );
    fwrite( "RxAntenna=\n", 1, strlen( "RxAntenna=\n" ), fp );
    fwrite( "TxPreamble=1\n", 1, strlen( "TxPreamble=1\n" ), fp );
    fwrite( "RTSThreshold=2347\n", 1, strlen( "RTSThreshold=2347\n" ), fp );
    fwrite( "FragThreshold=2346\n", 1, strlen( "FragThreshold=2346\n" ), fp );
    fwrite( "TxBurst=1\n", 1, strlen( "TxBurst=1\n" ), fp );
    fwrite( "PktAggregate=1\n", 1, strlen( "PktAggregate=1\n" ), fp );
    fwrite( "AutoProvisionEn=0\n", 1, strlen( "AutoProvisionEn=0\n" ), fp );
    fwrite( "FreqDelta=0\n", 1, strlen( "FreqDelta=0\n" ), fp );
    fwrite( "TurboRate=0\n", 1, strlen( "TurboRate=0\n" ), fp );
    fwrite( "WmmCapable=1\n", 1, strlen( "WmmCapable=1\n" ), fp );
    fwrite( "APAifsn=3;7;1;1\n", 1, strlen( "APAifsn=3;7;1;1\n" ), fp );
    fwrite( "APCwmin=4;4;3;2\n", 1, strlen( "APCwmin=4;4;3;2\n" ), fp );
    fwrite( "APCwmax=6;10;4;3\n", 1, strlen( "APCwmax=6;10;4;3\n" ), fp );
    fwrite( "APTxop=0;0;94;47\n", 1, strlen( "APTxop=0;0;94;47\n" ), fp );
    fwrite( "APACM=0;0;0;0\n", 1, strlen( "APACM=0;0;0;0\n" ), fp );
    fwrite( "BSSAifsn=3;7;2;2\n", 1, strlen( "BSSAifsn=3;7;2;2\n" ), fp );
    fwrite( "BSSCwmin=4;4;3;2\n", 1, strlen( "BSSCwmin=4;4;3;2\n" ), fp );
    fwrite( "BSSCwmax=10;10;4;3\n", 1, strlen( "BSSCwmax=10;10;4;3\n" ), fp );
    fwrite( "BSSTxop=0;0;94;47\n", 1, strlen( "BSSTxop=0;0;94;47\n" ), fp );
    fwrite( "BSSACM=0;0;0;0\n", 1, strlen( "BSSACM=0;0;0;0\n" ), fp );
    fwrite( "AckPolicy=0;0;0;0\n", 1, strlen( "AckPolicy=0;0;0;0\n" ), fp );
    fwrite( "APSDCapable=0\n", 1, strlen( "APSDCapable=0\n" ), fp );
    fwrite( "DLSCapable=0\n", 1, strlen( "DLSCapable=0\n" ), fp );
    fwrite( "NoForwarding=0\n", 1, strlen( "NoForwarding=0\n" ), fp );
    fwrite( "NoForwardingBTNBSSID=0\n", 1, strlen( "NoForwardingBTNBSSID=0\n" ), fp );
    fwrite( "HideSSID=0\n", 1, strlen( "HideSSID=0\n" ), fp );
    fwrite( "ShortSlot=1\n", 1, strlen( "ShortSlot=1\n" ), fp );
    fwrite( "AutoChannelSelect=0\n", 1, strlen( "AutoChannelSelect=0\n" ), fp );
    fwrite( "IEEE8021X=0\n", 1, strlen( "IEEE8021X=0\n" ), fp );
    fwrite( "IEEE80211H=0\n", 1, strlen( "IEEE80211H=0\n" ), fp );
    fwrite( "CarrierDetect=0\n", 1, strlen( "CarrierDetect=0\n" ), fp );
    fwrite( "ITxBfEn=0\n", 1, strlen( "ITxBfEn=0\n" ), fp );
    fwrite( "PreAntSwitch=\n", 1, strlen( "PreAntSwitch=\n" ), fp );
    fwrite( "PhyRateLimit=0\n", 1, strlen( "PhyRateLimit=0\n" ), fp );
    fwrite( "DebugFlags=0\n", 1, strlen( "DebugFlags=0\n" ), fp );
    fwrite( "ETxBfEnCond=0\n", 1, strlen( "ETxBfEnCond=0\n" ), fp );
    fwrite( "ITxBfTimeout=0\n", 1, strlen( "ITxBfTimeout=0\n" ), fp );
    fwrite( "ETxBfTimeout=0\n", 1, strlen( "ETxBfTimeout=0\n" ), fp );
    fwrite( "ETxBfNoncompress=0\n", 1, strlen( "ETxBfNoncompress=0\n" ), fp );
    fwrite( "ETxBfIncapable=0\n", 1, strlen( "ETxBfIncapable=0\n" ), fp );
    fwrite( "FineAGC=0\n", 1, strlen( "FineAGC=0\n" ), fp );
    fwrite( "StreamMode=0\n", 1, strlen( "StreamMode=0\n" ), fp );
    fwrite( "StreamModeMac0=\n", 1, strlen( "StreamModeMac0=\n" ), fp );
    fwrite( "StreamModeMac1=\n", 1, strlen( "StreamModeMac1=\n" ), fp );
    fwrite( "StreamModeMac2=\n", 1, strlen( "StreamModeMac2=\n" ), fp );
    fwrite( "StreamModeMac3=\n", 1, strlen( "StreamModeMac3=\n" ), fp );
    fwrite( "CSPeriod=6\n", 1, strlen( "CSPeriod=6\n" ), fp );
    fwrite( "RDRegion=\n", 1, strlen( "RDRegion=\n" ), fp );
    fwrite( "StationKeepAlive=0\n", 1, strlen( "StationKeepAlive=0\n" ), fp );
    fwrite( "DfsLowerLimit=0\n", 1, strlen( "DfsLowerLimit=0\n" ), fp );
    fwrite( "DfsUpperLimit=0\n", 1, strlen( "DfsUpperLimit=0\n" ), fp );
    fwrite( "DfsOutdoor=0\n", 1, strlen( "DfsOutdoor=0\n" ), fp );
    fwrite( "SymRoundFromCfg=0\n", 1, strlen( "SymRoundFromCfg=0\n" ), fp );
    fwrite( "BusyIdleFromCfg=0\n", 1, strlen( "BusyIdleFromCfg=0\n" ), fp );
    fwrite( "DfsRssiHighFromCfg=0\n", 1, strlen( "DfsRssiHighFromCfg=0\n" ), fp );
    fwrite( "DfsRssiLowFromCfg=0\n", 1, strlen( "DfsRssiLowFromCfg=0\n" ), fp );
    fwrite( "DFSParamFromConfig=0\n", 1, strlen( "DFSParamFromConfig=0\n" ), fp );
    fwrite( "FCCParamCh0=\n", 1, strlen( "FCCParamCh0=\n" ), fp );
    fwrite( "FCCParamCh1=\n", 1, strlen( "FCCParamCh1=\n" ), fp );
    fwrite( "FCCParamCh2=\n", 1, strlen( "FCCParamCh2=\n" ), fp );
    fwrite( "FCCParamCh3=\n", 1, strlen( "FCCParamCh3=\n" ), fp );
    fwrite( "CEParamCh0=\n", 1, strlen( "CEParamCh0=\n" ), fp );
    fwrite( "CEParamCh1=\n", 1, strlen( "CEParamCh1=\n" ), fp );
    fwrite( "CEParamCh2=\n", 1, strlen( "CEParamCh2=\n" ), fp );
    fwrite( "CEParamCh3=\n", 1, strlen( "CEParamCh3=\n" ), fp );
    fwrite( "JAPParamCh0=\n", 1, strlen( "JAPParamCh0=\n" ), fp );
    fwrite( "JAPParamCh1=\n", 1, strlen( "JAPParamCh1=\n" ), fp );
    fwrite( "JAPParamCh2=\n", 1, strlen( "JAPParamCh2=\n" ), fp );
    fwrite( "JAPParamCh3=\n", 1, strlen( "JAPParamCh3=\n" ), fp );
    fwrite( "JAPW53ParamCh0=\n", 1, strlen( "JAPW53ParamCh0=\n" ), fp );
    fwrite( "JAPW53ParamCh1=\n", 1, strlen( "JAPW53ParamCh1=\n" ), fp );
    fwrite( "JAPW53ParamCh2=\n", 1, strlen( "JAPW53ParamCh2=\n" ), fp );
    fwrite( "JAPW53ParamCh3=\n", 1, strlen( "JAPW53ParamCh3=\n" ), fp );
    fwrite( "FixDfsLimit=0\n", 1, strlen( "FixDfsLimit=0\n" ), fp );
    fwrite( "LongPulseRadarTh=0\n", 1, strlen( "LongPulseRadarTh=0\n" ), fp );
    fwrite( "AvgRssiReq=0\n", 1, strlen( "AvgRssiReq=0\n" ), fp );
    fwrite( "DFS_R66=0\n", 1, strlen( "DFS_R66=0\n" ), fp );
    fwrite( "BlockCh=\n", 1, strlen( "BlockCh=\n" ), fp );
    fwrite( "GreenAP=0\n", 1, strlen( "GreenAP=0\n" ), fp );
    fwrite( "PreAuth=0\n", 1, strlen( "PreAuth=0\n" ), fp );
    //ap auth or encrypt
    fwrite( "AuthMode=WPA2PSK\n", 1, strlen( "AuthMode=WPA2PSK\n" ), fp );
    fwrite( "EncrypType=AES\n", 1, strlen( "EncrypType=AES\n" ), fp );
    //auth encrypt
    fwrite( "WapiPsk1=\n", 1, strlen( "WapiPsk1=\n" ), fp );
    fwrite( "WapiPsk2=\n", 1, strlen( "WapiPsk2=\n" ), fp );
    fwrite( "WapiPsk3=\n", 1, strlen( "WapiPsk3=\n" ), fp );
    fwrite( "WapiPsk4=\n", 1, strlen( "WapiPsk4=\n" ), fp );
    fwrite( "WapiPsk5=\n", 1, strlen( "WapiPsk5=\n" ), fp );
    fwrite( "WapiPsk6=\n", 1, strlen( "WapiPsk6=\n" ), fp );
    fwrite( "WapiPsk7=\n", 1, strlen( "WapiPsk7=\n" ), fp );
    fwrite( "WapiPsk8=\n", 1, strlen( "WapiPsk8=\n" ), fp );

    fwrite( "WapiPskType=\n", 1, strlen( "WapiPskType=\n" ), fp );
    fwrite( "Wapiifname=\n", 1, strlen( "Wapiifname=\n" ), fp );
    fwrite( "WapiAsCertPath=\n", 1, strlen( "WapiAsCertPath=\n" ), fp );
    fwrite( "WapiUserCertPath=\n", 1, strlen( "WapiUserCertPath=\n" ), fp );
    fwrite( "WapiAsIpAddr=\n", 1, strlen( "WapiAsIpAddr=\n" ), fp );
    fwrite( "WapiAsPort=\n", 1, strlen( "WapiAsPort=\n" ), fp );

    fwrite( "BssidNum=1\n", 1, strlen( "BssidNum=1\n" ), fp );
    fwrite( "RekeyMethod=TIME\n", 1, strlen( "RekeyMethod=TIME\n" ), fp );
    fwrite( "RekeyInterval=3600\n", 1, strlen( "RekeyInterval=3600\n" ), fp );
    fwrite( "PMKCachePeriod=10\n", 1, strlen( "PMKCachePeriod=10\n" ), fp );
    fwrite( "MeshAutoLink=0\n", 1, strlen( "MeshAutoLink=0\n" ), fp );
    fwrite( "MeshAuthMode=\n", 1, strlen( "MeshAuthMode=\n" ), fp );
    fwrite( "MeshEncrypType=\n", 1, strlen( "MeshEncrypType=\n" ), fp );
    fwrite( "MeshDefaultkey=0\n", 1, strlen( "MeshDefaultkey=0\n" ), fp );
    fwrite( "MeshWEPKEY=\n", 1, strlen( "MeshWEPKEY=\n" ), fp );
    fwrite( "MeshWPAKEY=\n", 1, strlen( "MeshWPAKEY=\n" ), fp );
    fwrite( "MeshId=\n", 1, strlen( "MeshId=\n" ), fp );

/* BEGIN: Added by wupm, 2013/3/27 */
/* BEGIN: Deleted by wupm, 2013/4/15 */
#if	0	//def	RESET_APPASSWORD
	ResetAPPassword();
#endif

#if 1

    if ( strlen( bparam.stWifiRoute.szShareKey ) == 0x00 )
    {
        if ( strlen( bparam.stUserParam[2].szPassword ) == 0 )
        {
            memset( temp, 0x00, 256 );
            sprintf( temp, "WPAPSK1=123456789\n" );
            fwrite( temp, 1, strlen( temp ), fp );
        }

        else
        {
            memset( temp, 0x00, 256 );
            sprintf( temp, "WPAPSK1=%s\n", bparam.stWifiRoute.szShareKey );
            fwrite( temp, 1, strlen( temp ), fp );
        }
    }

    else
    {
        memset( temp, 0x00, 256 );
        sprintf( temp, "WPAPSK1=%s\n", bparam.stWifiRoute.szShareKey );
        fwrite( temp, 1, strlen( temp ), fp );
    }

#else
    memset( temp, 0x00, 256 );
    sprintf( temp, "WPAPSK1=123456789\n" );
    fwrite( temp, 1, strlen( temp ), fp );
#endif

    fwrite( "WPAPSK2=\n", 1, strlen( "WPAPSK2=\n" ), fp );
    fwrite( "WPAPSK3=\n", 1, strlen( "WPAPSK3=\n" ), fp );
    fwrite( "WPAPSK4=\n", 1, strlen( "WPAPSK4=\n" ), fp );
    fwrite( "WPAPSK5=\n", 1, strlen( "WPAPSK5=\n" ), fp );
    fwrite( "WPAPSK6=\n", 1, strlen( "WPAPSK6=\n" ), fp );
    fwrite( "WPAPSK7=\n", 1, strlen( "WPAPSK7=\n" ), fp );
    fwrite( "WPAPSK8=\n", 1, strlen( "WPAPSK8=\n" ), fp );
    fwrite( "DefaultKeyID=2\n", 1, strlen( "DefaultKeyID=2\n" ), fp );

    fwrite( "Key1Type=0\n", 1, strlen( "Key1Type=0\n" ), fp );
    fwrite( "Key1Str1=\n", 1, strlen( "Key1Str1=\n" ), fp );
    fwrite( "Key1Str2=\n", 1, strlen( "Key1Str2=\n" ), fp );
    fwrite( "Key1Str3=\n", 1, strlen( "Key1Str3=\n" ), fp );
    fwrite( "Key1Str4=\n", 1, strlen( "Key1Str4=\n" ), fp );
    fwrite( "Key1Str5=\n", 1, strlen( "Key1Str5=\n" ), fp );
    fwrite( "Key1Str6=\n", 1, strlen( "Key1Str6=\n" ), fp );
    fwrite( "Key1Str7=\n", 1, strlen( "Key1Str7=\n" ), fp );
    fwrite( "Key1Str8=\n", 1, strlen( "Key1Str8=\n" ), fp );

    fwrite( "Key2Type=0\n", 1, strlen( "Key2Type=0\n" ), fp );
    fwrite( "Key2Str1=\n", 1, strlen( "Key2Str1=\n" ), fp );
    fwrite( "Key2Str2=\n", 1, strlen( "Key2Str2=\n" ), fp );
    fwrite( "Key2Str3=\n", 1, strlen( "Key2Str3=\n" ), fp );
    fwrite( "Key2Str4=\n", 1, strlen( "Key2Str4=\n" ), fp );
    fwrite( "Key2Str5=\n", 1, strlen( "Key2Str5=\n" ), fp );
    fwrite( "Key2Str6=\n", 1, strlen( "Key2Str6=\n" ), fp );
    fwrite( "Key2Str7=\n", 1, strlen( "Key2Str7=\n" ), fp );
    fwrite( "Key2Str8=\n", 1, strlen( "Key2Str8=\n" ), fp );

    fwrite( "Key3Type=0\n", 1, strlen( "Key3Type=0\n" ), fp );
    fwrite( "Key3Str1=\n", 1, strlen( "Key3Str1=\n" ), fp );
    fwrite( "Key3Str2=\n", 1, strlen( "Key3Str2=\n" ), fp );
    fwrite( "Key3Str3=\n", 1, strlen( "Key3Str3=\n" ), fp );
    fwrite( "Key3Str4=\n", 1, strlen( "Key3Str4=\n" ), fp );
    fwrite( "Key3Str5=\n", 1, strlen( "Key3Str5=\n" ), fp );
    fwrite( "Key3Str6=\n", 1, strlen( "Key3Str6=\n" ), fp );
    fwrite( "Key3Str7=\n", 1, strlen( "Key3Str7=\n" ), fp );
    fwrite( "Key3Str8=\n", 1, strlen( "Key3Str8=\n" ), fp );

    fwrite( "Key4Type=0\n", 1, strlen( "Key4Type=0\n" ), fp );
    fwrite( "Key4Str1=\n", 1, strlen( "Key4Str1=\n" ), fp );
    fwrite( "Key4Str2=\n", 1, strlen( "Key4Str2=\n" ), fp );
    fwrite( "Key4Str3=\n", 1, strlen( "Key4Str3=\n" ), fp );
    fwrite( "Key4Str4=\n", 1, strlen( "Key4Str4=\n" ), fp );
    fwrite( "Key4Str5=\n", 1, strlen( "Key4Str5=\n" ), fp );
    fwrite( "Key4Str6=\n", 1, strlen( "Key4Str6=\n" ), fp );
    fwrite( "Key4Str7=\n", 1, strlen( "Key4Str7=\n" ), fp );
    fwrite( "Key4Str8=\n", 1, strlen( "Key4Str8=\n" ), fp );

    fwrite( "HSCounter=0\n", 1, strlen( "HSCounter=0\n" ), fp );
    fwrite( "HT_HTC=1\n", 1, strlen( "HT_HTC=1\n" ), fp );
    fwrite( "HT_RDG=1\n", 1, strlen( "HT_RDG=1\n" ), fp );
    fwrite( "HT_LinkAdapt=0\n", 1, strlen( "HT_LinkAdapt=0\n" ), fp );
    fwrite( "HT_OpMode=0\n", 1, strlen( "HT_OpMode=0\n" ), fp );
    fwrite( "HT_MpduDensity=5\n", 1, strlen( "HT_MpduDensity=5\n" ), fp );
    fwrite( "HT_EXTCHA=1\n", 1, strlen( "HT_EXTCHA=1\n" ), fp );
    fwrite( "HT_BW=1\n", 1, strlen( "HT_BW=1\n" ), fp );
    fwrite( "HT_AutoBA=1\n", 1, strlen( "HT_AutoBA=1\n" ), fp );
    fwrite( "HT_BADecline=0\n", 1, strlen( "HT_BADecline=0\n" ), fp );
    fwrite( "HT_AMSDU=0\n", 1, strlen( "HT_AMSDU=0\n" ), fp );
    fwrite( "HT_BAWinSize=64\n", 1, strlen( "HT_BAWinSize=64\n" ), fp );
    fwrite( "HT_GI=1\n", 1, strlen( "HT_GI=1\n" ), fp );
    fwrite( "HT_STBC=1\n", 1, strlen( "HT_STBC=1\n" ), fp );
    fwrite( "HT_MCS=33\n", 1, strlen( "HT_MCS=33\n" ), fp );
    fwrite( "HT_TxStream=0\n", 1, strlen( "HT_TxStream=0\n" ), fp );
    fwrite( "HT_RxStream=0\n", 1, strlen( "HT_RxStream=0\n" ), fp );
    fwrite( "HT_PROTECT=1\n", 1, strlen( "HT_PROTECT=1\n" ), fp );
    fwrite( "HT_DisallowTKIP=1\n", 1, strlen( "HT_DisallowTKIP=1\n" ), fp );
    fwrite( "HT_BSSCoexistence=0\n", 1, strlen( "HT_BSSCoexistence=0\n" ), fp );
    fwrite( "GreenAP=0\n", 1, strlen( "GreenAP=0\n" ), fp );
    fwrite( "WscConfMode=0\n", 1, strlen( "WscConfMode=0\n" ), fp );
    fwrite( "WscConfStatus=2\n", 1, strlen( "WscConfStatus=2\n" ), fp );
    fwrite( "WCNTest=0\n", 1, strlen( "WCNTest=0\n" ), fp );
    fwrite( "AccessPolicy0=0\n", 1, strlen( "AccessPolicy0=0\n" ), fp );
    fwrite( "AccessControlList0=\n", 1, strlen( "AccessControlList0=\n" ), fp );
    fwrite( "AccessPolicy1=0\n", 1, strlen( "AccessPolicy1=0\n" ), fp );
    fwrite( "AccessControlList1=\n", 1, strlen( "AccessControlList1=\n" ), fp );
    fwrite( "AccessPolicy2=0\n", 1, strlen( "AccessPolicy2=0\n" ), fp );
    fwrite( "AccessControlList2=\n", 1, strlen( "AccessControlList2=\n" ), fp );
    fwrite( "AccessPolicy3=0\n", 1, strlen( "AccessPolicy3=0\n" ), fp );
    fwrite( "AccessControlList3=\n", 1, strlen( "AccessControlList3=\n" ), fp );
    fwrite( "AccessPolicy4=0\n", 1, strlen( "AccessPolicy4=0\n" ), fp );
    fwrite( "AccessControlList4=\n", 1, strlen( "AccessControlList4=\n" ), fp );
    fwrite( "AccessPolicy5=0\n", 1, strlen( "AccessPolicy5=0\n" ), fp );
    fwrite( "AccessControlList5=\n", 1, strlen( "AccessControlList5=\n" ), fp );
    fwrite( "AccessPolicy6=0\n", 1, strlen( "AccessPolicy6=0\n" ), fp );
    fwrite( "AccessControlList6=\n", 1, strlen( "AccessControlList6=\n" ), fp );
    fwrite( "AccessPolicy7=0\n", 1, strlen( "AccessPolicy7=0\n" ), fp );
    fwrite( "AccessControlList7=\n", 1, strlen( "AccessControlList7=\n" ), fp );
    fwrite( "WdsEnable=0\n", 1, strlen( "WdsEnable=0\n" ), fp );
    fwrite( "WdsPhyMode=\n", 1, strlen( "WdsPhyMode=\n" ), fp );
    fwrite( "WdsEncrypType=NONE\n", 1, strlen( "WdsEncrypType=NONE\n" ), fp );
    fwrite( "WdsList=\n", 1, strlen( "WdsList=\n" ), fp );
    fwrite( "Wds0Key=\n", 1, strlen( "Wds0Key=\n" ), fp );
    fwrite( "Wds1Key=\n", 1, strlen( "Wds1Key=\n" ), fp );
    fwrite( "Wds2Key=\n", 1, strlen( "Wds2Key=\n" ), fp );
    fwrite( "Wds3Key=\n", 1, strlen( "Wds3Key=\n" ), fp );
    fwrite( "RADIUS_Server=0\n", 1, strlen( "RADIUS_Server=0\n" ), fp );
    fwrite( "RADIUS_Port=1812\n", 1, strlen( "RADIUS_Port=1812\n" ), fp );
    fwrite( "RADIUS_Key1=ralink\n", 1, strlen( "RADIUS_Key1=ralink\n" ), fp );
    fwrite( "RADIUS_Key2=\n", 1, strlen( "RADIUS_Key2=\n" ), fp );
    fwrite( "RADIUS_Key3=\n", 1, strlen( "RADIUS_Key3=\n" ), fp );
    fwrite( "RADIUS_Key4=\n", 1, strlen( "RADIUS_Key4=\n" ), fp );
    fwrite( "RADIUS_Key5=\n", 1, strlen( "RADIUS_Key5=\n" ), fp );
    fwrite( "RADIUS_Key6=\n", 1, strlen( "RADIUS_Key6=\n" ), fp );
    fwrite( "RADIUS_Key7=\n", 1, strlen( "RADIUS_Key7=\n" ), fp );
    fwrite( "RADIUS_Key8=\n", 1, strlen( "RADIUS_Key8=\n" ), fp );
    fwrite( "RADIUS_Acct_Server=\n", 1, strlen( "RADIUS_Acct_Server=\n" ), fp );
    fwrite( "RADIUS_Acct_Port=1813\n", 1, strlen( "RADIUS_Acct_Port=1813\n" ), fp );
    fwrite( "RADIUS_Acct_Key=\n", 1, strlen( "RADIUS_Acct_Key=\n" ), fp );
    fwrite( "own_ip_addr=\n", 1, strlen( "own_ip_addr=\n" ), fp );
    fwrite( "Ethifname=\n", 1, strlen( "Ethifname=\n" ), fp );
    fwrite( "EAPifname=\n", 1, strlen( "EAPifname=\n" ), fp );
    fwrite( "PreAuthifname=\n", 1, strlen( "PreAuthifname=\n" ), fp );
    fwrite( "session_timeout_interval=0\n", 1, strlen( "session_timeout_interval=0\n" ), fp );
    fwrite( "idle_timeout_interval=0\n", 1, strlen( "idle_timeout_interval=0\n" ), fp );
    fwrite( "WiFiTest=0\n", 1, strlen( "WiFiTest=0\n" ), fp );
    fwrite( "TGnWifiTest=0\n", 1, strlen( "TGnWifiTest=0\n" ), fp );
    fwrite( "ApCliEnable=1\n", 1, strlen( "ApCliEnable=1\n" ), fp );

    if ( strlen( bparam.stWifiParam.szSSID ) == 0x00 )
    {
        memset( temp, 0x00, 256 );
        sprintf( temp, "ApCliSsid=test\n" );
        fwrite( temp, 1, strlen( temp ), fp );
        bparam.stWifiParam.byAuthType = 2;
    }

    else
    {
        memset( temp, 0x00, 256 );
        sprintf( temp, "ApCliSsid=%s\n", bparam.stWifiParam.szSSID );
        fwrite( temp, 1, strlen( temp ), fp );
    }

    fwrite( "ApCliBssid=\n", 1, strlen( "ApCliBssid=\n" ), fp );
    memset( temp, 0x00, 256 );

    switch ( bparam.stWifiParam.byAuthType )
    {
#if 1

        case 0:         //none
        default:
            sprintf( temp, "ApCliAuthMode=OPEN\n" );
            fwrite( temp, 1, strlen( temp ), fp );

            memset( temp, 0x00, 256 );
            sprintf( temp, "ApCliEncrypType=NONE\n" );
            fwrite( temp, 1, strlen( temp ), fp );
            break;

        case 1:         //share
            if ( bparam.stWifiParam.byEncrypt == 0x01 )
            {
                sprintf( temp, "ApCliAuthMode=SHARED\n" );
                fwrite( temp, 1, strlen( temp ), fp );

                memset( temp, 0x00, 256 );
                sprintf( temp, "ApCliEncrypType=WEP\n" );
                fwrite( temp, 1, strlen( temp ), fp );
            }

            else
            {
                sprintf( temp, "ApCliAuthMode=OPEN\n" );
                fwrite( temp, 1, strlen( temp ), fp );

                memset( temp, 0x00, 256 );
                sprintf( temp, "ApCliEncrypType=WEP\n" );
                fwrite( temp, 1, strlen( temp ), fp );

            }

            break;
#else

        case 0:         //none
            sprintf( temp, "ApCliAuthMode=WPAPSK\n" );
            fwrite( temp, 1, strlen( temp ), fp );

            memset( temp, 0x00, 256 );
            sprintf( temp, "ApCliEncrypType=AES\n" );
            fwrite( temp, 1, strlen( temp ), fp );

        default:
#endif
        case 2:         //wpa-psk aes
            sprintf( temp, "ApCliAuthMode=WPAPSK\n" );
            fwrite( temp, 1, strlen( temp ), fp );

            memset( temp, 0x00, 256 );
            sprintf( temp, "ApCliEncrypType=AES\n" );
            fwrite( temp, 1, strlen( temp ), fp );
            break;

        case 3:         //wpa-psk tkip
            sprintf( temp, "ApCliAuthMode=WPAPSK\n" );
            fwrite( temp, 1, strlen( temp ), fp );

            memset( temp, 0x00, 256 );
            sprintf( temp, "ApCliEncrypType=TKIP\n" );
            fwrite( temp, 1, strlen( temp ), fp );
            break;

        case 4:         //wpa2-psk aes
            sprintf( temp, "ApCliAuthMode=WPA2PSK\n" );
            fwrite( temp, 1, strlen( temp ), fp );

            memset( temp, 0x00, 256 );
            sprintf( temp, "ApCliEncrypType=AES\n" );
            fwrite( temp, 1, strlen( temp ), fp );
            break;

        case 5:         //wpa2-psk tkip
            sprintf( temp, "ApCliAuthMode=WPA2PSK\n" );
            fwrite( temp, 1, strlen( temp ), fp );

            memset( temp, 0x00, 256 );
            sprintf( temp, "ApCliEncrypType=TKIP\n" );
            fwrite( temp, 1, strlen( temp ), fp );
            break;
    }

    if ( strlen( bparam.stWifiParam.szShareKey ) == 0x00 )
    {
        memset( temp, 0x00, 256 );
        sprintf( temp, "ApCliWPAPSK=\n" );
        fwrite( temp, 1, strlen( temp ), fp );

    }

    else
    {
        memset( temp, 0x00, 256 );
        sprintf( temp, "ApCliWPAPSK=%s\n", bparam.stWifiParam.szShareKey );
        fwrite( temp, 1, strlen( temp ), fp );
    }

#if 0
    fwrite( "ApCliAuthMode=WPA2PSK\n", 1, strlen( "ApCliAuthMode=WPA2PSK\n" ), fp );
    fwrite( "ApCliEncrypType=AES\n", 1, strlen( "ApCliEncrypType=AES\n" ), fp );
    fwrite( "ApCliWPAPSK=0123456789\n", 1, strlen( "ApCliWPAPSK=0123456789\n" ), fp );
#endif
    memset( temp, 0x00, 256 );
    sprintf( temp, "ApCliDefaultKeyID=%d\n", bparam.stWifiParam.byDefKeyType + 1 );
    fwrite( temp, 1, strlen( temp ), fp );
    bparam.stWifiParam.byKeyFormat = 1;

    if ( bparam.stWifiParam.byKeyFormat == 0 )
    {
        fwrite( "ApCliKey1Type=0\n", 1, strlen( "ApCliKey1Type=0\n" ), fp );
    }

    else
    {
        fwrite( "ApCliKey1Type=1\n", 1, strlen( "ApCliKey1Type=1\n" ), fp );
    }

    memset( temp, 0x00, 256 );
    sprintf( temp, "ApCliKey1Str=%s\n", bparam.stWifiParam.szKey1 );
    fwrite( temp, 1, strlen( temp ), fp );

    if ( bparam.stWifiParam.byKeyFormat == 0 )
    {
        fwrite( "ApCliKey2Type=0\n", 1, strlen( "ApCliKey2Type=0\n" ), fp );
    }

    else
    {
        fwrite( "ApCliKey2Type=1\n", 1, strlen( "ApCliKey2Type=1\n" ), fp );
    }

    memset( temp, 0x00, 256 );
    sprintf( temp, "ApCliKey2Str=%s\n", bparam.stWifiParam.szKey2 );
    fwrite( temp, 1, strlen( temp ), fp );

    if ( bparam.stWifiParam.byKeyFormat == 0 )
    {
        fwrite( "ApCliKey3Type=0\n", 1, strlen( "ApCliKey3Type=0\n" ), fp );
    }

    else
    {
        fwrite( "ApCliKey3Type=1\n", 1, strlen( "ApCliKey3Type=1\n" ), fp );
    }

    memset( temp, 0x00, 256 );
    sprintf( temp, "ApCliKey3Str=%s\n", bparam.stWifiParam.szKey3 );
    fwrite( temp, 1, strlen( temp ), fp );

    if ( bparam.stWifiParam.byKeyFormat == 0 )
    {
        fwrite( "ApCliKey4Type=0\n", 1, strlen( "ApCliKey4Type=0\n" ), fp );
    }

    else
    {
        fwrite( "ApCliKey4Type=1\n", 1, strlen( "ApCliKey4Type=1\n" ), fp );
    }

    memset( temp, 0x00, 256 );
    sprintf( temp, "ApCliKey4Str=%s\n", bparam.stWifiParam.szKey4 );
    fwrite( temp, 1, strlen( temp ), fp );
#if 0
    fwrite( "ApCliDefaultKeyID=1\n", 1, strlen( "ApCliDefaultKeyID=1\n" ), fp );
    fwrite( "ApCliKey1Type=1\n", 1, strlen( "ApCliKey1Type=1\n" ), fp );
    fwrite( "ApCliKey1Str=\n", 1, strlen( "ApCliKey1Str=\n" ), fp );
    fwrite( "ApCliKey2Type=1\n", 1, strlen( "ApCliKey2Type=1\n" ), fp );
    fwrite( "ApCliKey2Str=\n", 1, strlen( "ApCliKey2Str=\n" ), fp );
    fwrite( "ApCliKey3Type=1\n", 1, strlen( "ApCliKey3Type=1\n" ), fp );
    fwrite( "ApCliKey3Str=\n", 1, strlen( "ApCliKey3Str=\n" ), fp );
    fwrite( "ApCliKey4Type=1\n", 1, strlen( "ApCliKey4Type=1\n" ), fp );
#endif
    fwrite( "RadioOn=1\n", 1, strlen( "RadioOn=1\n" ), fp );
    fwrite( "SSID=\n", 1, strlen( "SSID=\n" ), fp );
    fwrite( "WPAPSK=\n", 1, strlen( "WPAPSK=\n" ), fp );
    fwrite( "Key1Str=\n", 1, strlen( "Key1Str=\n" ), fp );
    fwrite( "Key2Str=\n", 1, strlen( "Key2Str=\n" ), fp );
    fwrite( "Key3Str=\n", 1, strlen( "Key3Str=\n" ), fp );
    fwrite( "Key4Str=\n", 1, strlen( "Key4Str=\n" ), fp );
    //fwrite("\n",1,strlen("\n"),fp);
    fclose( fp );
}
#endif

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/4/16 */
#if	0
int OidQueryInformation( unsigned long OidQueryCode, int socket_id, char* DeviceName, void* ptr, unsigned long PtrLength )
{
    struct iwreq wrq;

    strcpy( wrq.ifr_name, DeviceName );
    wrq.u.data.length = PtrLength;
    wrq.u.data.pointer = ( caddr_t ) ptr;
    wrq.u.data.flags = OidQueryCode;

    return ( ioctl( socket_id, RT_PRIV_IOCTL, &wrq ) );
}

/* BEGIN: Added by wupm, 2013/1/16   BUG:50.2.128.124 */
pthread_mutex_t mutex_GetApWifiConnect = PTHREAD_MUTEX_INITIALIZER;
void Lock_GetApWifiConnect()
{
    pthread_mutex_lock( &mutex_GetApWifiConnect );
}
void UnLock_GetApWifiConnect()
{
    pthread_mutex_unlock( &mutex_GetApWifiConnect );
}
/* END:   Added by wupm, 2013/1/16 */

/* BEGIN: Added by GuoQi, 2013/3/13 BUG:71.2.192.4 */
/**
 * Detect AP current using channel num has been changed
 *
 * @author guoqi (3/13/2013)
 *
 * @param char * ssid
 *
 * @return -1, AP is down, ==0,channel no change; > 0, channel changed;
 */
int DetectApCli0ChannelChange( void )
{
    int     nLen, iRet = 0;
    int   channel = 1;
    char    temp[256];
    FILE*	fp = NULL;
    char*    pdst = NULL;
    memset( temp, 0x00, 256 );
    sprintf( temp, "iwpriv ra0 set SiteSurvey=1" );
    iRet = DoSystem( temp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "iwpriv ra0 get_site_survey | sed -n \"/%s/p\" > /tmp/apcli0channel.txt", bparam.stWifiParam.szSSID );
    DoSystem( temp );
    fp = fopen( "/tmp/apcli0channel.txt", "ab+" );

    if ( fp == NULL )
    {
        printf( "fopen failed\n" );
        return -1;
    }

    else
    {
        memset( temp, 0x00, 256 );
        fgets( temp, 256, fp );
        pdst = strstr( temp, bparam.stWifiParam.szSSID );

        if ( pdst != NULL )
        {
            if ( pdst <= temp )
            {
                iRet = -1;
            }

            else
            {
                nLen = pdst - temp;
                temp[nLen] = 0;
                channel = atoi( temp );

                if ( bparam.stWifiParam.channel != channel )
                {
                    iRet = 1;
                    printf( "ApCli0 Channel change from %d to %d\n", bparam.stWifiParam.channel, channel );
                    bparam.stWifiParam.channel = channel;
                }

                else
                {
                    iRet = 0;
                }
            }
        }
    }

    fclose( fp );
    return iRet;
}

int GetApCli0ConnectStatus( void )
{
    int     iRet = 0;
    char    temp[256];
    FILE*	fp = NULL;
    char*    pdst = NULL;
    memset( temp, 0x00, 256 );

    /* BEGIN: Modified by wupm, 2013/3/20 */
//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
    	sprintf( temp, "iwconfig apcli0 | sed -n \"/Not-Associated/p\" > /tmp/apcli0status.txt" );
	}
//#else
	else
	{
    	sprintf( temp, "iwconfig ra0 | sed -n \"/Not-Associated/p\" > /tmp/apcli0status.txt" );
	}
//#endif

    DoSystem( temp );
    fp = fopen( "/tmp/apcli0status.txt", "ab+" );

    if ( fp == NULL )
    {
        printf( "fopen failed\n" );
        iRet = 1;  /* sed Not-Associated failed, which means apcli0 status is connnected */
    }

    else
    {
        memset( temp, 0x00, 256 );
        fgets( temp, 256, fp );
        pdst = strstr( temp, "Not-Associated" );

        if ( pdst != NULL )
        {
            iRet = 0;
        }

        else
        {
            iRet = 1;
        }

        fclose( fp );
    }

    //printf("%s %d\n",__func__,iRet);
    return iRet;
}

int DetectApClioIpLost( void )
{
    int     iRet = 0;
    char    temp[256];
    FILE*	fp = NULL;
    char*    pdst = NULL;
    char*	 psrc = NULL;
    char*	 psrc1 = NULL;

    /* BEGIN: Modified by wupm, 2013/3/20 */
//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
    	iRet = DoSystem( "ifconfig apcli0 >/tmp/apcli0ip.txt" );
	}
//#else
	else
	{
    	iRet = DoSystem( "ifconfig ra0 >/tmp/apcli0ip.txt" );
	}
//#endif
    //printf( "DetectApClioIpLost...=%d\n", iRet );

    fp = fopen( "/tmp/apcli0ip.txt", "r" );

    if ( fp == NULL )
    {
        printf( "fopen failed\n" );
        iRet = 1;
    }

    memset( temp, 0x00, 128 );
    fgets( temp, 128, fp );
    fgets( temp, 128, fp );
    psrc = strstr( temp, "inet addr:" );
    psrc1 = strstr( temp, "Mask:" );

    if ( psrc == NULL )
    {
//        printf( "apcli0 ip lost...\n" );
        iRet = 1;;
    }

    fclose( fp );
    return iRet;
}



/* END: Added by GuoQi, 2013/3/13 */
int GetApWifiConnect( voide )
{
//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
	    static int nConnectFailedSum = 0;
	    int channel;
	    int                         s, ret, retry = 1;
	    unsigned int		ConnectStatus = NdisMediaStateDisconnected;
	    char				QueryCount ;
	    int nRetValue = 0;

		/* BEGIN: Added by wupm, 2013/3/28 */
		if ( bparam.stWifiParam.byEnable == 0 )
		{
			externwifistatus = 0x01;
			return 0;
		}

	    /* BEGIN: Added by GuoQi, 2013/3/13 BUG:71.2.192.4 */

	    //Textout( "Call GetApCli0ConnectStatus" );
	    ConnectStatus = GetApCli0ConnectStatus();

	    if ( ConnectStatus == 0x01 )
	    {
	        externwifistatus = 0x00; // connected
	    }

	    else
	    {
	        externwifistatus = 0x01; // disconnected
	        WorkLedMode( 3 );
	        nConnectFailedSum++;
	    }

	    if ( nConnectFailedSum > 0 ) // about 10 seconds
	    {
	        //Textout( "Call DetectApClioIpLost" );

	        if ( DetectApClioIpLost() > 0 )
	        {
	            RestartApcli0();
	            nConnectFailedSum = 0;
	        }

	        else
	        {
	            if ( nConnectFailedSum >= 10 )
	            {
	                nConnectFailedSum = 0;
	            }
	        }
	    }

	    if ( nConnectFailedSum >= 2 )
	    {
	        Textout( "==1==" );

	        if ( DetectApCli0ChannelChange() > 0 )
	        {
	            RestartApcli0();
	            nConnectFailedSum = 0;
	        }

	        else
	        {
	            printf( "AP is power off, apcli0 connect ap failed!\n" );
	            nConnectFailedSum = 0;
	        }
	    }

	    return ConnectStatus;
	}
//#else
	else
	{
	    int                 s, ret, retry = 1;
	    unsigned int		ConnectStatus = NdisMediaStateDisconnected;
	    char				QueryCount ;

		/* BEGIN: Added by wupm, 2013/3/28 */
		if ( bparam.stWifiParam.byEnable == 0 )
		{
			externwifistatus = 0x01;
			return 0;
		}

	    s = socket( AF_INET, SOCK_DGRAM, 0 );

	    if ( OidQueryInformation( OID_GEN_MEDIA_CONNECT_STATUS, s, "ra0", &ConnectStatus, sizeof( ConnectStatus ) ) < 0 )
	    {
	        Textout( "========Query OID_GEN_MEDIA_CONNECT_STATUS failed!========\n" );
	        close( s );
	        return -1;
	    }

	    close( s );
	    Textout1( "get wifi status:%d(Succ=1,Fail=0)\n", ConnectStatus );
	    #ifdef WIFI_DEBUG
	    {
	        char log[256];
	        memset(log, 0, 256);
	        sprintf( log, "get wifi status:%d(Succ=1,Fail=0)\n", ConnectStatus );
	        wifi_dbg( log, 0 );
	    }
	    #endif


	    if ( ConnectStatus == 0x01 )
	    {
	        //Succ
	        externwifistatus = 0x00;
	    }

	    else
	    {
	        //Fail
	        externwifistatus = 0x01;
	    }

	    return ConnectStatus;
	}
//#endif
}
#endif

//wifi result init
void WifiResultInit( void )
{
    memset( &WifiResult, 0x00, sizeof( WifiCont ) * 32 );
}

/* BEGIN: Deleted by wupm, 2014/3/4 */
#if	0
//wifi drivers install
void WifiDriversInit( void )
{
    char mac[20];
    char cmd[128];

    //DoSystem("insmod /system/system/drivers/rt3070sta.ko");
    memset( mac, 0x00, 20 );
    memcpy( mac, bparam.stIEBaseParam.szWifiMac, 17 );
    memset( cmd, 0x00, 128 );
    //sprintf(cmd,"insmod /lib/modules/2.6.21/kernel/drivers/net/wireless/rt2860v2_sta/rt2860v2_sta.ko mac=%s",mac);
    //sprintf(cmd,"insmod /lib/modules/2.6.21/kernel/drivers/net/wireless/rt2860v2_sta/rt2860v2_sta.ko");
    //sprintf(cmd,"insmod /lib/modules/2.6.21/kernel/drivers/net/wireless/rt2860v2_ap/rt2860v2_ap.ko mac=%s",mac);
    sprintf( cmd, "insmod /lib/modules/2.6.21/kernel/drivers/net/wireless/rt2860v2_ap/rt2860v2_ap.ko" );
    DoSystem( cmd );
    DoSystem( "ifconfig ra0 up" );
    printf( "==%s==\n", cmd );
}

//wifi up
void WifiUp( void )
{
    DoSystem( "ifconfig ra0 up" );
}
//
//void WifiRoute up
void WifiRouteStart( void )
{
    if ( bparam.stWifiRoute.ipaddr[0] != 0x00 )
    {
        char temp[64];
        memset( temp, 0x00, 64 );
        sprintf( temp, "ifconfig br0 %s", bparam.stWifiRoute.ipaddr );
        DoSystem( temp );
    }

    else
    {
        DoSystem( "ifconfig br0 192.168.9.1" );
    }
}
#endif

int RunScanWifi( void ) 	//run wifi scan and save wifiscan.txt
{
//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
	    int     iRet = 0;
	    char    temp[256];
	    FILE*	fp = NULL;
	    memset( temp, 0x00, 256 );
	    sprintf( temp, "iwpriv ra0 set SiteSurvey=1" );
	    iRet = DoSystem( temp );

	    memset( temp, 0x00, 256 );
	    sprintf( temp, "iwpriv ra0 get_site_survey > /tmp/wifiscan.txt" );
	    iRet = DoSystem( temp );
	    fp = fopen( "/tmp/wifiscan.txt", "ab+" );

	    if ( fp == NULL )
	    {
	        printf( "fopen failed\n" );
	        return -1;
	    }

	    fclose( fp );
	    return iRet;
	}
//#else
	else
	{
	    int     iRet = 0;
	    char    temp[256];
	    FILE*	fp = NULL;

	    memset( temp, 0x00, 256 );
	    sprintf( temp, "iwlist ra0 scanning > /tmp/wifiscan.txt" );
	    iRet = DoSystem( temp );
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
//#endif
}

/* BEGIN: Deleted by wupm, 2014/3/4 */
#if	0
int SaveESSID( char* pdst ) 		//save ssid
{
    int iRet = 0;

    bparam.stWifiParam.byEnable = 0x00;
    memset( bparam.stWifiParam.szSSID, 0x00, 64 );
    memcpy( bparam.stWifiParam.szSSID, pdst + 7, strlen( pdst ) - 9 );
    //printf("ssid:%s\n",bparam.stWifiParam.szSSID);
    return iRet;
}

int SaveKey( char* pdst ) 			//save key
{
    int 	iRet = 0;
    char	value;
    //printf("temp=%d:%s\n",strlen(pdst),pdst);

    value = *( pdst + 16 );

    if ( value == 'f' )
    {
        bparam.stWifiParam.byEncrypt = 0;
        //printf("temp:%s\n",pdst+15);
    }

    else
    {
        bparam.stWifiParam.byEncrypt = 1;
    }

    return iRet;
}

int SetWifiParam( int index ) 		//set wifi index of param
{
    FILE*    fp = NULL;
    char*	pdst = NULL;
    char	temp[256];
    char	flag = 0;
    char	value = 0;

    fp = fopen( "/tmp/wifiscan.txt", "r" );

    if ( fp == NULL )
    {
        printf( "fopen failed\n" );
        return -1;
    }

    fgets( temp, 256, fp );

    while ( !feof( fp ) )
    {
        memset( temp, 0x00, 256 );
        fgets( temp, 256, fp );
        pdst = strstr( temp, "ESSID:" );

        if ( pdst != NULL )
        {
            if ( value == index )
            {
                SaveESSID( pdst );
                flag = 1;
            }

            else
            {
                flag = 0;
            }

            value++;
        }

        if ( flag == 1 )
        {
            pdst = strstr( temp, "Encryption key:" );

            if ( pdst != NULL )
            {
                SaveKey( pdst );
            }
        }

    }

    fclose( fp );

    return 0;
}
#endif

void GetESSID( char* pdst, char sit ) 		//get ssid
{
    int iRet = 0;
    unsigned char i;
    memset( WifiResult[sit].ssid, 0x00, 64 );
//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
    	strcpy( WifiResult[sit].ssid, pdst );
	}
//#else
	else
	{
	    for ( i = 0; i < 64; i++ )
	    {
	        if ( pdst[i + 7] == 0x22 )
	        {
	            break;
	        }

	        WifiResult[sit].ssid[i] = pdst[i + 7];
	    }
	}
//#endif

    //printf("ssid:%s\n",WifiResult[sit].ssid);
}

void GetChannel( char channel, char sit ) 		//get wifi Mode
{
    unsigned char i;

    WifiResult[sit].channel = channel;
    //printf("mode:%s\n",WifiResult[sit].mode);
}

void GetWifiMac( char* pdst, char sit ) 		//get wifi mac
{
    memset( WifiResult[sit].mac, 0x00, 20 );
//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
    	strcpy( WifiResult[sit].mac, pdst );
	}
//#else
	else
	{
    	memcpy( WifiResult[sit].mac, pdst + 9, 18 );
	}
//#endif
    //printf("mac:%s\n",WifiResult[sit].mac);
}

void GetWifiMode( char* pdst, char sit ) 		//get wifi Mode
{
    unsigned char i;

    memset( WifiResult[sit].mode, 0x00, 8 );
//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
	}
//#else
	else
	{
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
//#endif
    //printf("mode:%s\n",WifiResult[sit].mode);
}

void GetKeyMode( char* pdst, char sit ) 		//get wifi key
{
    int iRet = 0;

    memset( WifiResult[sit].mode, 0x00, 8 );
    memcpy( WifiResult[sit].mode, pdst + 5, 8 );
    //printf("mode:%s\n",WifiResult[sit].mode);
}


void GetQuantMode( char* pdst, char sit ) 		//get wifi signal quant
{
    unsigned char i;
    unsigned char j = 0;

    memset( WifiResult[sit].db0, 0x00, 4 );
    memset( WifiResult[sit].db1, 0x00, 4 );
//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
    	strcpy( WifiResult[sit].db0 , pdst );
	}
//#else
	else
	{
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
//#endif
    printf( "quant:%s=%s\n", WifiResult[sit].db0, WifiResult[sit].db1 );
}

int FindChar( char* pbuf )
{
    int sit = 0;
    int iRet = -1;
    char* pos = pbuf;

    //find space
    while ( sit < 256 )
    {
        if ( *pos != 0x20 )
        {
            iRet = 0;
            break;
        }

        pos++;
        sit++;
    }

    if ( iRet )
    {
        return 0;
    }

    else
    {
        return sit;
    }
}
int FindNullChar( char* pbuf )
{
    int sit = 0;
    int iRet = -1;
    char* pos = pbuf;

    //find space
    while ( sit < 256 )
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
        return 0;
    }

    else
    {
        return sit;
    }
}

int GetStrFileValue( const char* pszSrc, char* pssid, char* pbessid, char* pauth, char* psignal )
{
    const char*		 pos1 = NULL, *pos = NULL, *psrc = pszSrc, *pdst = NULL;
    char                	temp[128];
    unsigned char       	i;
    const char*         	 pos2;
    char                	hanzi[4];
    int				len = 0;
    int 				len1 = 0;
    int				len2 = 0;

    //check src is null
    if ( !psrc )
    {
        printf( "pszSrc is NULL\n" );
        return -1;
    }

    //find ssid
    //find char
    len1 = FindChar( psrc );

    if ( len1 == 0x00 )
    {
        printf( "find char failed\n\n" );
        return -1;
    }

    //find nullchar
    len2 = FindNullChar( psrc + len1 );

    if ( len2 == 0x00 )
    {
        printf( "find null char failed\n" );
        return -1;
    }

    pos = pszSrc + len1;
    memcpy( pssid, pos, len2 );
    pdst = pszSrc + len1 + len2;
    //printf("len %d pssid:%s \n",len2,pssid);
    //find pbessid
    //find char
    psrc = pdst;
    len1 = FindChar( psrc );

    if ( len1 == 0x00 )
    {
        printf( "find char failed\n\n" );
        return -1;
    }

    //find nullchar
    len2 = FindNullChar( psrc + len1 );

    if ( len2 == 0x00 )
    {
        printf( "find null char failed\n" );
        return -1;
    }

    pos = psrc  + len1;
    memcpy( pbessid, pos, len2 );
    pdst += len1 + len2;

    if ( len2 < 15 )
    {
        return -1;
    }

    //printf("len %d pbessid:%s \n",len2,pbessid);
    //find auth
    //find char
    psrc = pdst;
    len1 = FindChar( psrc );

    if ( len1 == 0x00 )
    {
        printf( "find char failed\n\n" );
        return -1;
    }

    //find nullchar
    len2 = FindNullChar( psrc + len1 );

    if ( len2 == 0x00 )
    {
        printf( "find null char failed\n" );
        return -1;
    }

    pos = psrc  + len1;
    memcpy( pauth, pos, len2 );
    pdst += len1 + len2;
    //printf("len %d pauth:%s \n",len2,pauth);
    //find signal
    //find char
    psrc = pdst;
    len1 = FindChar( psrc );

    if ( len1 == 0x00 )
    {
        printf( "find char failed\n\n" );
        return -1;
    }

    //find nullchar
    len2 = FindNullChar( psrc + len1 );

    if ( len2 == 0x00 )
    {
        printf( "find null char failed\n" );
        return -1;
    }

    pos = psrc  + len1;
    memcpy( psignal, pos, len2 );
    pdst += len1 + len2;
    //printf("len %d psignal:%s \n",len2,psignal);

//	printf("psignal:%s\n",pdst);
    //printf("len1 %d len2 %d pos:%s pos1:%s\n",len1,len2,pos,pos1);
    return 0;
}

int ParseWifiScan( char* pbuf, char sit ) 		//parse wifi scan result
{
//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
	    char*	 pdst = NULL;
	    char*	 pdst1 = NULL;
	    char 	auth = 0, ency = 0, key = 0, channel = 1;
	    int		iRet;
	    char 	ssid[128];
	    char		bessid[128];
	    char		pauth[128];
	    char		psignal[4];
	    char		pchannel[4];

	    //find ssid
	    memset( ssid, 0x00, 128 );
	    memset( bessid, 0x00, 128 );
	    memset( pauth, 0x00, 128 );
	    memset( psignal , 0x00, 4 );
	    memset( pchannel, 0x00, 4 );
	    pdst = pbuf + 2;

	    if ( pbuf[1] == 0x30 )
	    {
	        pchannel[0] = pbuf[0];
	    }

	    else
	    {
	        pchannel[0] = pbuf[0];
	        pchannel[1] = pbuf[1];
	    }

	    iRet = GetStrFileValue( pdst, ssid, bessid, pauth, psignal );

	    //printf("iRet %d channel:%s=%d ssid:%s bessid:%s pauth:%s psignal:%s\n",iRet,pchannel,atoi(pchannel),ssid,bessid,pauth,psignal);
	    if ( iRet == 0x00 )
	    {
	        GetESSID( ssid, sit );
	        GetWifiMac( bessid, sit );
	        GetWifiMode( NULL, sit );
	        GetQuantMode( psignal, sit );
	        GetChannel( atoi( pchannel ), sit );
	        //auth
	        pdst = strstr( pauth, "NONE" );

	        if ( pdst != NULL )
	        {
	            ency = 0x00;
	            key == 0x00;
	        }

	        pdst = strstr( pauth, "WEP" );

	        if ( pdst != NULL )
	        {
	            ency = 0x00;
	            key == 0x01;
	        }

	        pdst = strstr( pauth, "WPAPSK" );

	        if ( pdst != NULL )
	        {
	            auth = 0x01;
	        }

	        pdst = strstr( pauth, "WPA2PSK" );

	        if ( pdst != NULL )
	        {
	            auth = 0x02;
	        }

	        pdst = strstr( pauth, "AES" );

	        if ( pdst != NULL )
	        {
	            ency = 0x01;
	        }

	        pdst = strstr( pauth, "TKIP" );

	        if ( pdst != NULL )
	        {
	            ency = 0x02;
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
	    }

	    return iRet;
	}
//#else
	else
	{
	    char* pdst = NULL;
	    char* pdst1 = NULL;
	    char auth = 0, ency = 0, key = 0;

	    pdst = strstr( pbuf, "Address: " );

	    if ( pdst != NULL )
	    {
	        //printf("addr:%s=%s\n",pdst,pdst+9);
	        GetWifiMac( pdst, sit );
	    }

	    pdst = strstr( pbuf, "ESSID:" );

	    if ( pdst != NULL )
	    {
	        //printf("essid:%s=%s\n",pdst,pdst+7);
	        GetESSID( pdst, sit );
	    }

	    pdst = strstr( pbuf, "Mode:" );

	    if ( pdst != NULL )
	    {
	        //printf("mode:%s=%s\n",pdst,pdst+5);
	        GetWifiMode( pdst, sit );
	    }

	    //printf("=============start=================\n");
	    pdst = strstr( pbuf, "Quality=" );

	    if ( pdst != NULL )
	    {
	        GetQuantMode( pdst, sit );
	    }

	    //printf("==============end================\n");
	    pdst = strstr( pbuf, "Encryption key:" );

	    if ( pdst != NULL )
	    {
	        pdst += 16;

	        if ( pdst[0] == 0x6e )	//on
	        {
	            //WifiResult[sit].keyon = 0x01;
	            key = 1;
	            //printf("key on\n");
	        }

	        else
	        {
	            //WifiResult[sit].keyon = 0x00;
	            key = 0;
	            //printf("key off\n");
	        }
	    }

	    pdst = strstr( pbuf, "WPA Version" );

	    if ( pdst != NULL )
	    {
	        auth = 1;
	        //printf("Authtype WPA:%s=%s\n",pdst,pdst+4);
	    }

	    pdst = strstr( pbuf, "WPA2 Version" );

	    if ( pdst != NULL )
	    {
	        auth = 2;
	        //printf("Authtype WPA2:%s=%s\n",pdst,pdst+4);
	    }

	    pdst = strstr( pbuf, "CCMP" );

	    if ( pdst != NULL )
	    {
	        ency = 1;
	        //printf("Authtype AEC:%s=%s\n",pdst,pdst+4);
	    }

	    pdst = strstr( pbuf, "TKIP" );

	    if ( pdst != NULL )
	    {
	        ency = 2;
	        //printf("Authtype AEC:%s=%s\n",pdst,pdst+4);
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
//#endif
}

/* BEGIN: Resolve WIFI-CHECK mothod, so can Parse "Space" SSID */
/*        Added by wupm(2073111@qq.com), 2014/9/27 */
#ifdef	SUPPORT_PARSE_SPACE_SSID

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

int FindSpaceFromStart( char* pbuf )
{
    int sit = 0;
    int iRet = -1;
    char* pos = pbuf;

    //find space
    while ( sit < 256 )
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

int FindEndSpace( char* pbuf )
{
    int sit = 0;
    int iRet = -1;
    char* pos = pbuf;

    //find space
    while ( sit < 256 )
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

#endif

int GetWifiResult( void )
{
//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
		/* BEGIN: Resolve WIFI-CHECK mothod, so can Parse "Space" SSID */
		/*        Modified by wupm(2073111@qq.com), 2014/9/27 */
		#ifdef	SUPPORT_PARSE_SPACE_SSID

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

	    Textout("Line 1: %s", strline);
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

	    Textout("nPosCh=%d, nPosSSID=%d, nPosBSSID=%d, nPosSecurity=%d, nPosSignal=%d", nPosCh, nPosSSID, nPosBSSID, nPosSecurity, nPosSignal);


	    memset(strline, 0, 256);
	    nssid = 0;
	    nline = 3;
	    while( ( iRet = GetFileLineEx(rfile, strline, nline) ) == 0)
	    {
	        memset(&WifiResult[nssid], 0, sizeof(WifiCont));

	        Textout("Index=%d", nssid);
	        //get channel
	        nSpace = FindSpaceFromStart(strline + nPosCh);
	        if(nSpace != -1)
	        {
	            memset(channel, 0, 4);
	            memcpy(channel, strline+nPosCh, nSpace);
	            WifiResult[nssid].channel = atoi(channel);
	            Textout("channel=%s(%d)", channel, WifiResult[nssid].channel);
	        }

	        //get ssid
	        nSpace = FindSpaceFromEnd(strline + nPosBSSID - 1, nPosSSID, nPosBSSID - 1);
	        if(nSpace != -1)
	        {
	            memcpy(WifiResult[nssid].ssid, strline + nPosSSID, nSpace);
	            Textout("ssid=%s", WifiResult[nssid].ssid);
	        }
	        else
	        {
	            Textout("Unsurpport the ssid");

				/* BEGIN: When Scan Wifi, if Found "Space" SSID, system will halt */
				/*        Added by wupm(2073111@qq.com), 2014/10/21 */
				nline++;

	            continue;
	        }

	        //get bssid
	        nSpace = FindSpaceFromStart(strline + nPosBSSID);
	        if(nSpace != -1)
	        {
	            memcpy(WifiResult[nssid].mac, strline + nPosBSSID, nSpace);
	            Textout("mac=%s", WifiResult[nssid].mac);
	        }

	        //get security
	        nSpace = FindSpaceFromStart(strline + nPosSecurity);
	        if(nSpace != -1)
	        {
	            int ency = 0;
	            int key = 0;
	            int auth = 0;
	            char* pdst = NULL;
	            memset(Authtype, 0, 32);
	            memcpy(Authtype, strline + nPosSecurity, nSpace);

	            Textout("Security=%s", Authtype);
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
	        nSpace = FindSpaceFromStart(strline + nPosSignal);
	        if(nSpace != -1)
	        {
	            memcpy(WifiResult[nssid].db0, strline + nPosSignal, nSpace);
	            Textout("db0=%s", WifiResult[nssid].db0);
	        }

	        nline++;
	        nssid++;
	        if(nssid > 31)
	        {
	            break;
	        }

	        memset(strline, 0, 256);
	    }

	    Textout("nssid=%d", nssid);
	    return nssid;

		#else

	    FILE*   	 fp = NULL;
	    char*   	 pdst = NULL;
	    char    	temp[256];
	    char    	sit = 0;
	    char		cell[2048];
	    char*   	 psrc = NULL;
	    int		iRet;

	    fp = fopen( "/tmp/wifiscan.txt", "r" );

	    if ( fp == NULL )
	    {
	        printf( "fopen failed\n" );
	        return -1;
	    }

	    fgets( temp, 256, fp );	// 1 line
	    printf( "1temp:%s\n", temp );
	    fgets( temp, 256, fp );	// 2 line
	    printf( "2temp:%s\n", temp );
	    memset( temp, 0x00, 256 );

	    while ( !feof( fp ) )
	    {
	        memset( temp, 0x00, 256 );
	        fgets( temp, 256, fp );

	        if ( temp[4] == 0x20 )
	        {
	            //memcpy(psrc,temp,strlen(temp));
	            //psrc += strlen(temp);
	            printf( "four isn't space\n" );
	            continue;
	        }

	        if ( sit >= 0 )
	        {
	            //printf("cell:%d=%s\n",sit,temp);
	            iRet = ParseWifiScan( temp, sit );

	            if ( iRet )
	            {
	                continue;
	            }
	        }

	        printf( "cell:%d\n", sit );
	        sit++;

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

	    //printf("============%d===================\n",sit);
	    return sit;

		#endif
		/* END:   Modified by wupm(2073111@qq.com), 2014/9/27 */
	}
//#else
	else
	{
	    FILE*    fp = NULL;
	    char*    pdst = NULL;
	    char    temp[256];
	    char    sit = -1;
	    char	cell[2048];
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
	            //printf("cell:%d=%s\n",sit,cell);
	            ParseWifiScan( cell, sit );
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

	    //printf("============%d===================\n",sit);
	    return sit;
	}
//#endif
}

int GetWifiScan( void )
{
    int 	iRet = 0;

//#ifdef	ENABLE_AP
	if ( ENABLE_AP )
	{
	}
//#else
	else
	{
	    wifistat = GetWifiEth();

	    if ( wifistat != 0x00 )
	    {
	        //WifiUp();
	        DoSystem( "ifconfig ra0 up" );
	    }
	}
//#endif

    wifilock();
    WifiResultInit();

    Textout("===run scan wifi===");
    iRet = RunScanWifi();

    Textout("===run scan end===");
    if ( iRet == 0 )
    {
        wificnt = GetWifiResult();
        Textout( "wifi scan wificnt %d", wificnt );
		if ( 1 )
		{
			int i;
			for(i=0;i<wificnt;i++)
			{
				Textout("SSID [ %d ] = [%s]", i, WifiResult[i].ssid);
			}
		}
        //UpdateWifiResultCgi(wificnt);
    }

    wifiunlock();
    return iRet;
}

/* BEGIN: Deleted by wupm, 2014/3/4 */
#if	0
int GetApMac( void )
{
    FILE* fp = NULL;
    unsigned char temp[256];
    char* pdst = NULL;

    DoSystem( "ifconfig ra0 > /tmp/routemac.txt" );
    fp = fopen( "/tmp/routemac.txt", "r" );

    if ( fp == NULL )
    {
        printf( "route mac is null\n" );
        return -1;
    }

    memset( temp, 0x00, 256 );
    fgets( temp, 256, fp );
    fclose( fp );
    pdst = strstr( temp, "HWaddr " );

    if ( pdst == NULL )
    {
        printf( "not find mac" );
        return -1;
    }

    pdst += 7;
    memset( bparam.stIEBaseParam.szMac, 0x00, 17 );
    memcpy( bparam.stIEBaseParam.szMac, pdst, 17 );
    return 0;
}

int GetApClientMac( void )
{
    FILE* fp = NULL;
    unsigned char temp[256];
    char* pdst = NULL;

    DoSystem( "ifconfig apcli0 > /tmp/routemac.txt" );
    fp = fopen( "/tmp/routemac.txt", "r" );

    if ( fp == NULL )
    {
        printf( "route mac is null\n" );
        return -1;
    }

    memset( temp, 0x00, 256 );
    fgets( temp, 256, fp );
    fclose( fp );
    pdst = strstr( temp, "HWaddr " );

    if ( pdst == NULL )
    {
        printf( "not find mac" );
        return -1;
    }

    pdst += 7;
    memset( bparam.stIEBaseParam.szWifiMac, 0x00, 17 );
    memcpy( bparam.stIEBaseParam.szWifiMac, pdst, 17 );
    return 0;
}
#endif

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/4/16 */
#if	0
void WifiStart( void ) 		//wifi start
{
    char    temp[128];


    Textout( "wifistart1\n" );
    memset( temp, 0x00, 128 );
    sprintf( temp, "route del default" );
    DoSystem( temp );

    printf( "wifistart2\n" );
    memset( temp, 0x00, 128 );
    sprintf( temp, "ifconfig apcli0 %s netmask %s up", bparam.stNetParam.szIpAddr, bparam.stNetParam.szMask );
    DoSystem( temp );

    printf( "wifistart3\n" );
    memset( temp, 0x00, 128 );
    sprintf( temp, "route add default gw %s apcli0", bparam.stNetParam.szGateway );
    DoSystem( temp );

    printf( "wifistart4\n" );
    DoSystem( "route add -net 255.255.255.255 netmask 255.255.255.255 dev apcli0 metric 1" );

#if 0
    DoSystem( "iwpriv ra0 set HwAntDiv=1" );
    DoSystem( "iwpriv ra0 set ATEANTDIV=1" );

    printf( "wifistart5\n" );
    DoSystem( "route add -net 255.255.255.255 netmask 255.255.255.255 dev ra0 metric 1" );

//	 printf("wifistart6\n");
//	 DoSystem("route add -net 192.168.9.0 netmask 255.255.255.0 gw 192.168.9.1 dev apcli0");
#endif
    printf( "wifistart6n" );
    memset( bparam.stNetParam.szDns1, 0x00, 16 );
    sprintf( bparam.stNetParam.szDns1, "8.8.8.8" );
#if 0
    DoSystem( "iwpriv ra0 set HwAntDiv=1" );
    DoSystem( "iwpriv ra0 set HwAntDiv=1" );
//#else
    DoSystem( "iwpriv ra0 set ATEANTDIV=1" );
    DoSystem( "iwpriv ra0 set AutoReconnect=1" );
#endif
    NoteGateWay();
}
#endif

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/4/16 */
#if	0
void WifiStartInit( void )          //wifi start
{
    char    temp[128];

    DoSystem( "ifconfig apcli0 up" );
    sleep( 1 );
}
#endif

/* BEGIN: Deleted by wupm, 2014/3/4 */
#if	0
void RouteStart( void )
{
    char    temp[128];

    memset( temp, 0x00, 128 );
    sprintf( temp, "iwpriv ra0 set NetworkType=Adhoc" );
    DoSystem( temp );

    memset( temp, 0x00, 128 );
    sprintf( temp, "iwpriv ra0 set AuthMode=OPEN" );
    DoSystem( temp );

    memset( temp, 0x00, 128 );
    sprintf( temp, "iwpriv ra0 set EncrypType=NONE" );
    DoSystem( temp );

    memset( temp, 0x00, 128 );
    sprintf( temp, "iwpriv ra0 set SSID=\"%s\"", bparam.stWifiRoute.szSSID );
    DoSystem( temp );

/* BEGIN: Added by wupm, 2013/3/27 */
/* BEGIN: Deleted by wupm, 2013/4/15 */
#if	0	//def	RESET_APPASSWORD
	ResetAPPassword();
#endif

    memset( temp, 0x00, 128 );
    sprintf( temp, "iwpriv ra0 set WPAPSK=\"%s\"", bparam.stWifiRoute.szShareKey );
    DoSystem( temp );

    memset( temp, 0x00, 128 );
    sprintf( temp, "iwpriv ra0 set SSID=\"%s\"", bparam.stWifiRoute.szSSID );
    DoSystem( temp );
}
#endif

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/4/16 */
#if	0
void SysWifiStartAp( void )          //wifi start
{
    char    temp[128];
#if 0
    DoSystem( "brctl delif br0 eth2" );
    DoSystem( "brctl addif br0 ra0" );

    RouteStart();
#endif
    Textout( "wifistart1\n" );
    memset( temp, 0x00, 128 );
    sprintf( temp, "route del default" );
    DoSystem( temp );

    printf( "wifistart2\n" );
    memset( temp, 0x00, 128 );
    sprintf( temp, "ifconfig ra0 %s netmask %s up", bparam.stWifiRoute.ipaddr, bparam.stWifiRoute.mask );
    DoSystem( temp );
#if 0
    memset( temp, 0x00, 128 );
    sprintf( temp, "route add default gw %s ra0", bparam.stWifiRoute.ipaddr );
    DoSystem( temp );
    printf( "wifistart5\n" );
#endif
    memset( temp, 0x00, 128 );
    sprintf( temp, "ifconfig apcli0 up" );
    DoSystem( temp );

}

void SetWifiChannel( void )
{
    char temp[128];

    if ( bparam.stWifiParam.channel == 0x00 )
    {
        bparam.stWifiParam.channel = 1;
    }

    printf( "======wifi channel=%d========\n", bparam.stWifiParam.channel );
    memset( temp, 0x00, 128 );
    sprintf( temp, "iwpriv apcli0 set Channel=%d", bparam.stWifiParam.channel );
    DoSystem( temp );
}
#endif

