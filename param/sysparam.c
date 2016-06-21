#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <dirent.h>
#include "param.h"
#include "nvram.h"
#include "debug.h"

IEPARAMLIST 		bparam;
IEPARAMCMD		bnetparam;
/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/12 */
//USERPARAM		userbackup[3];
SMARTEYE		eyeparam;

#ifdef TENVIS
TTENVISPARAM tenvisparam;
#endif
/* BEGIN: Added by Baggio.wu, 2013/7/31 */
#ifdef CUSTOM_HTTP_ALARM
TPSDPARAM psdparam;
#endif

#ifdef CHANGE_DDNS_ALARM_SERVER
TVSTARPARAM vstarparam;
#endif


#ifdef JPUSH
JPUSHPARAMLIST jpushparamlist;
#endif

//#ifdef LDPUSH
PUSHPARAMLIST pushparamlist;
char jpush_address[16] = {0};
char xpush_address[16] = {0};
char ypush_address[16] = {0};
char bEnablePush = 0;
//#endif 

int bPIRStatus = 0;

pthread_mutex_t filemutex = PTHREAD_MUTEX_INITIALIZER;

void SeParamDefault( void );

void filelock( void )
{
    pthread_mutex_lock( &filemutex );
}

void fileunlock( void )
{
    pthread_mutex_unlock( &filemutex );
}


#ifdef LDPUSH
void ReadPushParams(void)
{
	FILE*	fp = NULL;

	if(access("/param/pushparamlist.bin", F_OK) == -1)
	{
		Textout("pushparamlist.bin not exit");
		memset( &pushparamlist,0x00,sizeof( PUSHPARAMLIST));
		WritePushParams();
	}
	
	memset( &pushparamlist,0x00,sizeof( PUSHPARAMLIST));
	fp = fopen( "/param/pushparamlist.bin","rb");
	if( fp)
	{
		fread( &pushparamlist,1,sizeof ( PUSHPARAMLIST ),fp);
		fclose ( fp );
	}
}

void WritePushParams(void)
{
	Textout("WritePushParams");
	FILE*	 fp = NULL;
	fp = fopen( "/param/pushparamlist.bin", "wb" );

	if ( fp )
	{
		fwrite( &pushparamlist, 1, sizeof( PUSHPARAMLIST ), fp );
		DoSystem("sync");
		fclose( fp );
	}
	else
	{
		Textout("/param/pushparamlist.bin not exit");
	}
}
#endif


#ifdef JPUSH
void ReadJPushParams(void)
{
	FILE*	fp = NULL;

	if(access("/param/jpushparamlist.bin", F_OK) == -1)
	{
		Textout("jpushparamlist.bin not exit");
		//DoSystem("touch /param/jpushparamlist.bin")
		memset( &jpushparamlist,0x00,sizeof( JPUSHPARAMLIST));
		WriteJPushParams();
	}
	
	memset( &jpushparamlist,0x00,sizeof( JPUSHPARAMLIST));
	fp = fopen( "/param/jpushparamlist.bin","rb");
	if( fp)
	{
		fread( &jpushparamlist,1,sizeof ( JPUSHPARAMLIST ),fp);
		fclose ( fp );
	}
}

void WriteJPushParams(void)
{
	Textout("WriteJPushParams");
	FILE*	 fp = NULL;
	fp = fopen( "/param/jpushparamlist.bin", "wb" );

	if ( fp )
	{
        
		fwrite( &jpushparamlist, 1, sizeof( JPUSHPARAMLIST ), fp );
		DoSystem("sync");
		fclose( fp );
	}
	else
	{
		Textout("/param/jpushparamlist.bin not exit");
	}
}
#endif

#ifdef TENVIS
void ReadTenvisParams( void )
{
    FILE*    fp = NULL;
    memset( &tenvisparam, 0x00, sizeof( TTENVISPARAM) );
    fp = fopen( "/param/tenvisparam.bin", "rb" );
    if ( fp )
    {
        fread( &tenvisparam, 1, sizeof( TTENVISPARAM ), fp );
        fclose( fp );
    }
}
void WriteTenvisParams()
{
    FILE*    fp = NULL;
    fp = fopen( "/param/tenvisparam.bin", "wb" );

    if ( fp )
    {
        fwrite( &tenvisparam, 1, sizeof( TTENVISPARAM ), fp );
        fclose( fp );
    }
}
#endif


/* BEGIN: Added by Baggio.wu, 2013/7/31 */
#ifdef CUSTOM_HTTP_ALARM
void ReadPsdParams( void )
{
    FILE*    fp = NULL;
    memset( &psdparam, 0x00, sizeof( TPSDPARAM ) );
    fp = fopen( "/param/psdparam.bin", "rb" );

    if ( fp )
    {
        fread( &psdparam, 1, sizeof( TPSDPARAM ), fp );
        fclose( fp );
    }
}
void WritePsdParams()
{
    FILE*    fp = NULL;
    fp = fopen( "/param/psdparam.bin", "wb" );

    if ( fp )
    {
        fwrite( &psdparam, 1, sizeof( TPSDPARAM ), fp );
        fclose( fp );
    }
}
#endif
/* END:   Added by Baggio.wu, 2013/7/31 */

#ifdef CHANGE_DDNS_ALARM_SERVER
#define FILE_VSTAR_PARAM    "/param/vstarparam.bin"
void ReadVstarParams( void )
{
    FILE*    fp = NULL;
    memset( &vstarparam, 0x00, sizeof( TVSTARPARAM ) );
    fp = fopen( FILE_VSTAR_PARAM, "rb" );
    if ( fp )
    {
        fread( &vstarparam, 1, sizeof( TVSTARPARAM ), fp );
        fclose( fp );
    }
}
void WriteVstarParams()
{
    FILE*    fp = NULL;
    fp = fopen( FILE_VSTAR_PARAM, "wb" );

    if ( fp )
    {
        fwrite( &vstarparam, 1, sizeof( TVSTARPARAM ), fp );
        fclose( fp );
    }
}
#endif

//read data from config
void ReadSmartEye( void )
{
    FILE*    fp = NULL;
    int     curtime = 0;
    memset( &eyeparam, 0x00, sizeof( SMARTEYE ) );
    fp = fopen( "/param/smarteye.bin", "rb" );

    if ( fp )
    {
        fread( &eyeparam, 1, sizeof( SMARTEYE ), fp );
        fclose( fp );
    }
}
//write date to config
void WriteSmartEye( int curtime )
{
    FILE*    fp = NULL;
    fp = fopen( "/param/smarteye.bin", "wb" );

    if ( fp )
    {
        fwrite( &eyeparam, 1, sizeof( SMARTEYE ), fp );
        fclose( fp );
    }
}

void userdisp( void )
{
    char i, j;

    for ( i = 0; i < MAX_USER; i++ )
    {
        for ( j = 0; j < 32; j++ )
        {
            printf( "%02x-", bparam.stUserParam[i].szUserName[j] );
        }

        printf( "\n" );

        for ( j = 0; j < 32; j++ )
        {
            printf( "%02x-", bparam.stUserParam[i].szPassword[j] );
        }

        printf( "\n" );
    }
}

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/12 */
#if	1
void CheckUserPri( void )
{
}
#else
void CheckUserPri( void )
{
    char i;
    char adminflag = 0x00;
    char temp[32];
    //printf("======================\n");
    //printf("user:%s pwd:%s\n",bparam.stUserParam[0].szUserName,bparam.stUserParam[0].szPassword);
#if 0
    memset( &userbackup, 0x00, sizeof( USERPARAM ) );

    if ( ( bparam.stUserParam[0].szUserName[0] == 0x00 ) || ( strlen( bparam.stUserParam[0].szUserName ) == 0x00 ) )
    {
        //vistor
        memset( bparam.stUserParam[0].szUserName, 0x00, 32 );
        memset( bparam.stUserParam[0].szPassword, 0x00, 32 );
        memset( temp, 0x00, 32 );
        sprintf( temp, "admin0" );
        sprintf( bparam.stUserParam[0].szUserName, temp );
    }

    //printf("user:%s pwd:%s\n",bparam.stUserParam[1].szUserName,bparam.stUserParam[1].szPassword);
    if ( ( bparam.stUserParam[1].szUserName[0] == 0x00 ) || ( strlen( bparam.stUserParam[1].szUserName ) == 0x00 ) )
    {
        //opration
        memset( bparam.stUserParam[1].szUserName, 0x00, 32 );
        memset( bparam.stUserParam[1].szPassword, 0x00, 32 );
        memset( temp, 0x00, 32 );
        sprintf( temp, "admin1" );
        sprintf( bparam.stUserParam[1].szUserName, temp );
    }

#endif

    //printf("user:%s pwd:%s\n",bparam.stUserParam[2].szUserName,bparam.stUserParam[2].szPassword);
    if ( ( bparam.stUserParam[2].szUserName[0] == 0x00 ) || ( strlen( bparam.stUserParam[2].szUserName ) == 0x00 ) )
    {
        //admin
        memset( bparam.stUserParam[2].szUserName, 0x00, 32 );
        memset( bparam.stUserParam[2].szPassword, 0x00, 32 );
        memset( temp, 0x00, 32 );
        sprintf( temp, "admin" );
        sprintf( bparam.stUserParam[2].szUserName, temp );
#ifdef	SCC_OEM
		/* BEGIN: Added by wupm, 2012/12/29 */
		sprintf(bparam.stUserParam[2].szPassword, temp);
#endif
    }

    memcpy( &userbackup, &bparam.stUserParam, sizeof( USERPARAM ) * 3 );
}
#endif

void ReadFactoryMac( void )
{
    char* pmac = NULL;
    pmac = nvram_bufget( RT2860_NVRAM, "factory_mac" );
    printf( "mac:%s %02x\n", pmac, pmac[0] );

    if ( pmac[0] != 0x00 )
    {
        memset( bparam.stIEBaseParam.szMac, 0x00, 17 );
        memcpy( bparam.stIEBaseParam.szMac, pmac, 17 );
    }

    else
    {
        memcpy( bparam.stIEBaseParam.szMac, "00:00:00:00:00:01", 17 );
    }

    pmac = nvram_bufget( RT2860_NVRAM, "factory_wifimac" );
    printf( "wifimac:%s %02x\n", pmac, pmac[0] );

    if ( pmac[0] != 0x00 )
    {
        memset( bparam.stIEBaseParam.szWifiMac, 0x00, 17 );
        memcpy( bparam.stIEBaseParam.szWifiMac, pmac, 17 );
    }

    else
    {
        memcpy( bparam.stIEBaseParam.szWifiMac, "00:00:00:00:00:02", 17 );
    }

    printf( "wifimac:%s\n", bparam.stIEBaseParam.szWifiMac );
}

void ReadRactoryID( void )
{
    char* pdeviceid = NULL;

	/* BEGIN: Modified by wupm, 2013/7/1 */
    //char len;
    char len = 0;
	
    memset( bparam.stIEBaseParam.dwDeviceID, 0x00, 24 );
    pdeviceid = nvram_bufget( RT2860_NVRAM, "factory_deviceid" );

    if ( pdeviceid )
    {
        len = strlen( pdeviceid );

        if ( len >= 23 )
        {
            len = 23;
        }

        memcpy( bparam.stIEBaseParam.dwDeviceID, pdeviceid, len );
    }
    else
    {
        memcpy( bparam.stIEBaseParam.dwDeviceID, "", len );
    }

	memset( bparam.stIEBaseParam.dwApiLisense, 0x00, 8 );
	pdeviceid = NULL;
    pdeviceid = nvram_bufget( RT2860_NVRAM, "factory_apilisense" );
    if ( pdeviceid )
    {
        len = strlen( pdeviceid );

        if ( len >= 7 )
        {
            len = 7;
        }

        memcpy( bparam.stIEBaseParam.dwApiLisense, pdeviceid, len );
    }
    else
    {
        memcpy( bparam.stIEBaseParam.dwApiLisense, "", 0 );
    }
}

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/11 */
#if	1
void ReadDnsFactory( void )
{
}
#else
void ReadDnsFactory( void )
{
    char* pdst = NULL;
    char value[8];
    //if (bparam.stDdnsParam.byDdnsSvr >= 10){
    //server name
    memset( bparam.stDdnsParam.serversvr, 0x00, 64 );
    pdst = nvram_bufget( RT2860_NVRAM, "factory_server" );

    if ( pdst )
    {
        strcpy( bparam.stDdnsParam.serversvr, pdst );
    }

    //server port
    pdst = nvram_bufget( RT2860_NVRAM, "factory_dnsport" );

    if ( pdst )
    {
        bparam.stDdnsParam.serverport = atoi( pdst );
    }

    //server beat
    pdst = nvram_bufget( RT2860_NVRAM, "factory_heartbeat" );

    if ( pdst )
    {
        bparam.stDdnsParam.serverbeat = atoi( pdst );
    }

    //server username
    memset( bparam.stDdnsParam.serveruser, 0x00, 32 );
    pdst = nvram_bufget( RT2860_NVRAM, "factory_dnsuser" );

    if ( pdst )
    {
        strcpy( bparam.stDdnsParam.serveruser, pdst );
    }

    //server passwd
    memset( bparam.stDdnsParam.serverpwd, 0x00, 32 );
    pdst = nvram_bufget( RT2860_NVRAM, "factory_dnspwd" );

    if ( pdst )
    {
        strcpy( bparam.stDdnsParam.serverpwd, pdst );
    }

    //enable
    pdst = nvram_bufget( RT2860_NVRAM, "factory_index" );

    if ( pdst )
    {
        bparam.stDdnsParam.factoryversion = atoi( pdst );
    }

    //mode
    pdst = nvram_bufget( RT2860_NVRAM, "factory_mode" );

    if ( pdst )
    {
        bparam.stDdnsParam.factorymode = atoi( pdst );
    }

    //}
}
#endif


/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/11 */

void ResetVideoConfig()
{
	bparam.stBell.video_mod	=	VM_SPEED;		//1/2/3/4, Current mod

	bparam.stBell.video_resolution[VM_SPEED] = VS_QVGA;
	bparam.stBell.video_framerate[VM_SPEED] = 20;
	bparam.stBell.video_bitrate[VM_SPEED] = 2*1024;

	bparam.stBell.video_resolution[VM_MID] = VS_QVGA;
	bparam.stBell.video_framerate[VM_MID] = 15;
	bparam.stBell.video_bitrate[VM_MID] = 2*1024;

	bparam.stBell.video_resolution[VM_SLOW] = VS_VGA;
	bparam.stBell.video_framerate[VM_SLOW] = 12;
	bparam.stBell.video_bitrate[VM_SLOW] = 2*1024;

	bparam.stBell.video_resolution[VM_CUSTOM] = VS_QVGA;
	bparam.stBell.video_framerate[VM_CUSTOM] = 20;
	bparam.stBell.video_bitrate[VM_CUSTOM] = 2*1024;

	bparam.stBell.video_brightness = bparam.stVencParam.brightness;
	bparam.stBell.video_contrast = bparam.stVencParam.contrast;
	bparam.stBell.video_saturation = bparam.stVencParam.saturation;
	bparam.stBell.video_hue = bparam.stVencParam.chroma;
}
void BackupBellVideo()
{
	int i = bparam.stBell.video_mod;
	bparam.stBell.current_resolution = bparam.stBell.video_resolution[i];
	bparam.stBell.current_framerate = bparam.stBell.video_framerate[i];
	bparam.stBell.current_bitrate = bparam.stBell.video_bitrate[i];
	bparam.stBell.current_brightness = bparam.stBell.video_brightness;
	bparam.stBell.current_contrast = bparam.stBell.video_contrast;
	bparam.stBell.current_saturation = bparam.stBell.video_saturation;
	bparam.stBell.current_hue = bparam.stBell.video_hue;
}

int ReadParamFile(FILE *fp)
{
	int nRead = fread( &bparam, 1, sizeof( IEPARAMLIST ), fp );
	if ( nRead == sizeof( IEPARAMLIST ) )
	{
		BackupBellVideo();
	}
	Textout("stBell = [%s]/[%s]", bparam.stBell.user[0], bparam.stBell.pwd[0]);
	return nRead;
}

void SetBellDefault()
{
	int i = 0;
	Textout("******************************SetBellDefault********************************");
	memset(&bparam.stBell, 0, sizeof(BELLPARAMS));

	bparam.stBell.bell_on	=	1;	//1
	bparam.stBell.bell_audio=	0;	//1
	bparam.stBell.bell_mode	=	1;	//1

	bparam.stBell.max_watch	=	30;	//30
	bparam.stBell.max_talk	=	60;	//60
	bparam.stBell.max_wait	=	15;	//15

	bparam.stBell.lock_type	=	1;	//0
	bparam.stBell.lock_delay=	10;	//10

	bparam.stBell.pin		=	1;
	bparam.stBell.pin_bind	=	0;	//0=ÃÅ´Å
	bparam.stBell.pout		=	1;
	bparam.stBell.pout_bind	=	0;	//0=ºô½Ð£¬1=±¨¾¯

	bparam.stBell.alarm_on	=	0;	//1
	bparam.stBell.alarm_onok=	0;	//1
	bparam.stBell.alarm_type=	0;	//0=MOTION,1=PIR,2=MOTION+PIR
	bparam.stBell.alarm_level=	3;	//Min=1, Max=5
	bparam.stBell.alarm_delay=	20;	//20
	bparam.stBell.alarm_start_hour	=	0;
	bparam.stBell.alarm_start_minute=	0;
	bparam.stBell.alarm_stop_hour	=	23;
	bparam.stBell.alarm_stop_minute	=	59;

	strcpy(bparam.stBell.user[0], "admin");
	//bparam.stBell.user[MAX_USER][32];	//user[0] = "admin"
	//bparam.stBell.pwd[MAX_USER][32];
	//bparam.stBell.xalias[32];			//"Bell"
	bparam.stBell.admin		=	1;		//1

	ResetVideoConfig();

	//strcpy(bparam.stBell.hw_ver, HARDWARE_VERSION);
	//strcpy(bparam.stBell.sw_ver, SOFTWARE_VERSION);

	/*
	bparam.stBell.xmac[8];
	bparam.stBell.xwifi_mac[8];

	bparam.stBell.xdhcp;
	bparam.stBell.xdns_auto;
	bparam.stBell.xport;

	bparam.stBell.xdns1[16];
	bparam.stBell.xdns2[16];
	bparam.stBell.xip[16];
	bparam.stBell.xmask[16];
	bparam.stBell.xgateway[16];

	bparam.stBell.xwifi;			//enable wifi
	bparam.stBell.xssid[64];
	bparam.stBell.xkey[32];
	bparam.stBell.xdevice_id[32];
	*/

	bparam.stBell.ap_mode = 0;		//in ap state ?
	bparam.stBell.status = 0;		//0=OK,1=DHCP,2=ROUTER,3=INTERNET(ERROR)
	bparam.stBell.sensor = CAMERA_TYPE;		//0=MJ, 1=HD

	BackupBellVideo();
}

void InitSystemParam( void )
{
	#ifdef	SYSTEM_PARAM_FILENAME
    FILE*    fp;
    //fp = fopen( "/system/www/system.ini", "rb" );
    fp = fopen(SYSTEM_PARAM_FILENAME, "rb");
    Textout( " ************************InitSystemParam*********************** " );

    if ( fp )
    {
        //printf( "=========================read system param from file==================================\n" );
		Textout("Read Config File");
        //if ( sizeof( IEPARAMLIST ) != fread( &bparam, 1, sizeof( IEPARAMLIST ), fp ) )
        if ( sizeof(IEPARAMLIST) != ReadParamFile(fp) )
        {
			Textout("Config File Size ERROR, Set to Default");
            //param size isn't ok
            //Textout( "Key = [%s]", bparam.stWifiRoute.szShareKey );
            SeParamDefault();
        }

        CheckUserPri();
        fclose( fp );
        bparam.stStatusParam.warnstat = 0x00;
    }
	else
	{
		Textout("File Not Exist, Default");
		SeParamDefault();
		bparam.stStatusParam.warnstat = 0x00;
	}
	#else
	FILE*    fp;
    fp = fopen( "/system/www/system.ini", "rb" );

    if ( fp )
    {
		Textout("Read Config File");
        //if ( sizeof( IEPARAMLIST ) != fread( &bparam, 1, sizeof( IEPARAMLIST ), fp ) )
        if ( sizeof(IEPARAMLIST) != ReadParamFile(fp) )
        {
            Textout("read system.ini fail, check system.ini_backup");
            /* BEGIN: Added by Baggio.wu, 2013/7/4 */
            struct statfs statFS;

			if ( statfs( "/system/www/system.ini_backup", &statFS ) != -1 )
            {
				/* BEGIN: Modified by wupm, 2014/3/5 */
				/*
                Textout("system.ini_backup OK");
                DoSystem( "cp /system/www/system.ini_backup /system/www/system.ini" );
                DoSystem( "sync" );
                fp = fopen( "/system/www/system.ini", "rb" );
                if ( fp )
                {
                    if ( sizeof( IEPARAMLIST ) != fread( &bparam, 1, sizeof( IEPARAMLIST ), fp ) )
                    {
                        Textout("read system.ini_backup fail, set to default");
                        SeParamDefault();
                        DoSystem( "rm -rf /system/www/system.ini_backup" );
                    }

                    close( fp );
                }
                */
                if ( 0 )
                {
                }

                else
                {
                    Textout( "set system param to default" );
                    SeParamDefault();
                }

                CheckUserPri();
            }

            /* END:   Added by Baggio.wu, 2013/7/4 */
            else
            {
                Textout( "set system param to default" );
                SeParamDefault();
            }
        }

        #ifdef DEFAULT_BRIGHTNESS
        if(bparam.stVencParam.brightness == 0)
        {
            bparam.stVencParam.brightness = 90;
        }

        if(bparam.stVencParam.contrast == 128)
        {
           bparam.stVencParam.contrast=160;
        }
        #endif

        CheckUserPri();
        fclose( fp );
        bparam.stStatusParam.warnstat = 0x00;
    }

    else
    {
        Textout("system.ini not exist, check system.ini_backup");
        struct statfs statFS;

        /* BEGIN: Added by Baggio.wu, 2013/7/4 */
        if ( statfs( "/system/www/system.ini_backup", &statFS ) != -1 )
        {
			/* BEGIN: Modified by wupm, 2014/3/5 */
			/*
            Textout( "system.ini_backup is exist" );
            DoSystem( "cp /system/www/system.ini_backup /system/www/system.ini" );
            DoSystem( "sync" );
            fp = fopen( "/system/www/system.ini", "rb" );

            if ( fp )
            {
                if ( sizeof( IEPARAMLIST ) != fread( &bparam, 1, sizeof( IEPARAMLIST ), fp ) )
                {
                    Textout( "read system.ini_backup fail, set to default" );
                    SeParamDefault();
                    DoSystem( "rm -rf /system/www/system.ini_backup" );
                }

                close( fp );
            }
            */
            if ( 0 )
            {
            }

            else
            {
                Textout( "set system param to default" );
                SeParamDefault();
            }

            CheckUserPri();
        }

        /* END:   Added by Baggio.wu, 2013/7/4 */

        else if ( statfs( "/system/www/system-b.ini", &statFS ) == -1 )
        {
            Textout( "set system param to default" );
            SeParamDefault();
            CheckUserPri();
            SaveSystemParam( &bparam );
            DoSystem( "sync" );
        }

        else
        {
            DoSystem( "cp /system/www/system-b.ini /system/www/system.ini" );
            DoSystem( "sync" );
            fp = fopen( "/system/www/system.ini", "rb" );

            if ( fp )
            {
                //if ( sizeof( IEPARAMLIST ) != fread( &bparam, 1, sizeof( IEPARAMLIST ), fp ) )
                if ( sizeof(IEPARAMLIST) != ReadParamFile(fp) )
                {
                    Textout( "set system param to default" );
                    SeParamDefault();
                    DoSystem( "rm -rf /system/www/system-b.ini" );
                }

                close( fp );
            }

            else
            {
                Textout( "set system param to default" );
                SeParamDefault();
            }

            CheckUserPri();
        }
    }

    printf( "user1: %s, pwd1: %s\n", bparam.stUserParam[0].szUserName, bparam.stUserParam[0].szPassword );
    printf( "user2: %s, pwd2: %s\n", bparam.stUserParam[1].szUserName, bparam.stUserParam[1].szPassword );
    printf( "user3: %s, pwd3: %s\n", bparam.stUserParam[2].szUserName, bparam.stUserParam[2].szPassword );
	#endif
}
int SaveSystemParam( PIEPARAMLIST pstParam )
{
    filelock();

    /* BEGIN: Added by Baggio.wu, 2013/7/4 */
    //DoSystem( "cp /system/www/system.ini /system/www/system.ini_backup" );
    DoSystem( "sync" );
    /* END:   Added by Baggio.wu, 2013/7/4 */

    if ( 1 )
    {
        FILE*        fp;
		#ifdef	SYSTEM_PARAM_FILENAME
        fp = fopen( SYSTEM_PARAM_FILENAME, "wb" );
		#else
		fp = fopen( "/system/www/system.ini", "wb" );
		#endif

        if ( fp )
        {
            fwrite( &bparam, 1, sizeof( IEPARAMLIST ), fp );
            fclose( fp );
        }
    }

    DoSystem( "sync" );
    fileunlock();
    return 0;
}



void SeParamDefault( void )
{
	Textout("Go-To-Here");
    filelock();
    //base param
    memset( &bparam, 0x00, sizeof( IEPARAMLIST ) );
    strcpy( bparam.stIEBaseParam.dwDeviceID, "" );
	strcpy( bparam.stIEBaseParam.dwApiLisense, "");

    #if (defined(JINQIANXIANG) || defined(BAFANGDIANZI))
	strcpy( bparam.stIEBaseParam.szDevName, "IPCAM" );
    #else
	strcpy( bparam.stIEBaseParam.szDevName, "Bell" );
    #endif

    bparam.stIEBaseParam.sysmode  = 0;				//0->baby montior 1->HD IPCAM
    bparam.stIEBaseParam.factory  = 0;
    //status param
    //date time
    bparam.stDTimeParam.byIsNTPServer	= 0x01;
    memcpy( bparam.stDTimeParam.szNtpSvr, "time.nist.gov", 13 );
    //net param
    bparam.stNetParam.byIsDhcp 		= 0x01;
    bparam.stNetParam.nPort			= 81;

    sprintf( bparam.stNetParam.szIpAddr, "192.168.1.249" );
    sprintf( bparam.stNetParam.szMask, "255.255.255.0" );
    sprintf( bparam.stNetParam.szGateway, "192.168.1.1" );
    sprintf( bparam.stNetParam.szDns1, "8.8.8.8" );
    sprintf( bparam.stNetParam.szDns2, "8.8.8.8" );
    //wifi param
    //wifi route

	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/4/16 */
	/*
    sprintf( bparam.stWifiRoute.szSSID, "IPCAM" );
    sprintf( bparam.stWifiRoute.ipaddr, "192.168.9.1" );
    sprintf( bparam.stWifiRoute.mask, "255.255.255.0" );
    sprintf( bparam.stWifiRoute.startip, "192.168.9.2" );
    sprintf( bparam.stWifiRoute.endip, "192.168.9.254" );
    sprintf( bparam.stWifiRoute.szShareKey, "0123456789" );
    */
	if (1)
	{
	    //sprintf(bparam.stWifiRoute.szSSID,"WIFICAM");
	    sprintf( bparam.stWifiRoute.ipaddr, "192.168.246.1" );
	    sprintf( bparam.stWifiRoute.mask, "255.255.255.0" );
	    sprintf( bparam.stWifiRoute.startip, "192.168.246.2" );
	    sprintf( bparam.stWifiRoute.endip, "192.168.246.254" );
	    //sprintf(bparam.stWifiRoute.szShareKey,"0123456789");
	}

    bparam.stWifiRoute.byEncrypt = 4;		//wpa2-psk aes
    bparam.stWifiRoute.nport = 80;
    bparam.stWifiParam.byWps = 1;			//pbc
    //adsl param
    //rtsp param
    //video enc
    bparam.stVencParam.bysize 		= 0x05;	//main rate VGA
    bparam.stVencParam.bysizesub 		= 0x03;	//sub rate  QVGA

#ifdef	SCC_OEM
    /* BEGIN: Modified by wupm, 2013/3/1 */
    bparam.stVencParam.byframerate 		= 12;	//30;
    bparam.stVencParam.byframeratesub 	= 6;	//15;
#else
    bparam.stVencParam.byframerate 		= 30;
    bparam.stVencParam.byframeratesub 	= 15;
#endif
    bparam.stVencParam.keyframe 		= 60;
    bparam.stVencParam.keyframesub 		= 30;
    bparam.stVencParam.bitrate              = 1024 * 2;
    bparam.stVencParam.bitratesub           = 384;
    bparam.stVencParam.quant 		= 20;		//
    bparam.stVencParam.quantsub 		= 30;		//
    bparam.stVencParam.ratemode 		= 0;     	//main profile
    bparam.stVencParam.ratemodesub 		= 1;     	//cbr
    bparam.stVencParam.codecprofile 	= 1;     	//main profile
    bparam.stVencParam.codecprofilesub 	= 0;     	//baseline profile
    #ifdef DEFAULT_BRIGHTNESS
    bparam.stVencParam.brightness   	= 90;     	//brightness
    bparam.stVencParam.contrast		= 160;         	//contrast
    #else
    bparam.stVencParam.brightness   	= 0;     	//brightness
    bparam.stVencParam.contrast		= 128;         	//contrast
    #endif
    bparam.stVencParam.chroma		= 0;         	//chroma
    bparam.stVencParam.saturation   	= 0;         	//saturation
    //audio enc
    bparam.stAencParam.format 		= 0x00;
    //upnp
    bparam.stUpnpParam.byEnable		= 0x01;
    //ptz
    /* BEGIN: Modified by wupm, 2013/3/19 */
#ifdef	HUHONG
    bparam.stPTZParam.byRate			= 5;
    bparam.stPTZParam.byLedMode			= 0x01;
#else
    bparam.stPTZParam.byRate			= 0x01;
#endif

/* BEGIN: Added by wupm, 2013/6/6 */
#ifdef	HUHONG_7501
    bparam.stPTZParam.byRate			= 5;
    bparam.stPTZParam.byLedMode			= 0x01;
#endif

	//ircut
#ifdef	SCC
	/* BEGIN: Modified by wupm, 2013/1/22   Version:125 */
	//bparam.stVencParam.ircut 			= 0x01;
	bparam.stVencParam.ircut 			= 0x00;
#else
    bparam.stVencParam.ircut 			= 0x01;
#endif

    bparam.stVencParam.nSensorChip 		= 0;

	bparam.stIEBaseParam.bEnableAP1 = 0x00;
	bparam.stIEBaseParam.bEnableAP2 = 0x00;


    //mail
    //ftp
    //record
    //multdevice
    //alarm param
    //user param
    //ddns param
    //upnp
    //ptz param
    //mail param

	/* BEGIN: Added by wupm, 2014/1/1 */
	bparam.stIEBaseParam.bEnableAP1 = 0x00;
	bparam.stIEBaseParam.bEnableAP2 = 0x00;

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/11 */
	SetBellDefault();
    fileunlock();

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/16 */
	SaveSystemParam(&bparam);
}
