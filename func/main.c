#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "init.h"
#include "alarm.h"
#include "mywatchdog.h"
#include "boa.h"
#include "param.h"
#include "debug.h"

#include "BaSysTimer.h"

#include "mjpgbuf.h"
#include "abuf.h"
#include "vbuf.h"
#include "sensorapi.h"
//#include "yinetwork.h"

void Resourceinit( void )
{
    SysParamInit();

    
    /* BEGIN: KONX, Close PIR-LED when need, Set Initial-value, Adjust IR-LED */
    /*        Added by wupm, 2014/12/6 */
#ifdef  KONX
    if ( bparam.stBell.f1 == 0 && bparam.stBell.f2 == 0 )
    {
        Textout("Konx First Call, Set Parameters to Initial Value");
        bparam.stBell.f1 = 1;
        bparam.stBell.f2 = 1;
        bparam.stBell.lock_type = 0;    //Normal LOW
        bparam.stBell.lock_delay = 2;   //Delay 2 Seconds
        bparam.stBell.alarm_delay = 10; //PIR Alarm Delay 10 Seconds
        bparam.stBell.max_talk = 120;   //Talk Max 120 Seconds
        bparam.stBell.max_watch = 120;  //Watch Max 120 Seconds
        bparam.stBell.max_wait = 20;    //Call Wait 20 Seconds
        SaveSystemParam(&bparam);
    }
#endif

#ifdef  KANGJIEDENG
    if ( bparam.stBell.f1 == 0 && bparam.stBell.f2 == 0 )
    {
        Textout("Konx First Call, Set Parameters to Initial Value");
        bparam.stBell.f1 = 1;
        bparam.stBell.f2 = 1;
        bparam.stBell.lock_delay = 2;   //Delay 2 Seconds
        SaveSystemParam(&bparam);
    }
#endif    


}

int serverinit( void )
{
    AlarmThread();

    EncJBufInit();
    EncMBbufInit();
    EncABufInit();

    SensorInit();
    VencStart();
    
#if defined(SENSOR_8433)
    SetUseSensorDefaultFlag( SENSOR_DEFAULT_ENABLE );
    VideoParamInit_8433();
#elif defined(SENSOR_3861)
    VideoParamInit_3861();
#endif
    
    LogStart();
    FactoryThread();

    return 0;
}

BOOL ExistGpsFile()
{
	//cp /tmp/gps.txt /system/www/gps.txt
	FILE *f = fopen("/system/www/gps.txt", "rb");
	if ( f != NULL )
	{
		fclose(f);
		DoSystem("rm -f /system/www/gps.txt");
		return TRUE;
	}
	return FALSE;
}

void BaAudioDelayStartup(void)
{
    StartAudioPlay(WF_STARTUP, 1, NULL);
}

static int bInited = 0;
void NetworkInitOK()
{
    if(bInited == 0) bInited = 1;
    else return;
    
    Textout("-----------------network init ok---------------");
    MsgPushInit();
    P2PStart();

    WebThread();

    Networkhread();
    /*************************OBJECT SELF PROTOCOL***************************/
    #ifdef BAGGIO_API
    NvrCmdInit();
    NvrLiveStreamInit();
    NvrAudioStreamInit();
    {
        char cmd[64];
        int iRet = access( "/system/system/bin/ipcapp", F_OK );

        if ( iRet == 0 )
        {
            DoSystem( "chmod a+x /system/system/bin/ipcapp" );

            DoSystem( "./system/system/bin/ipcapp &" );
            Textout( "Running /system/system/bin/ipcapp" );
        }
    }
    #endif
    /*************************OBJECT SELF PROTOCOL***************************/    

	/* BEGIN: Auto Download Any Firmware From OUR SERVER */
	/*        Added by wupm, 2014/12/12 */
    #ifdef  AUTO_DOWNLOAD_FIRMWARE
	InitDownloadModule();
    #endif    

    NetCmdStart();
    DnsStart();
    UpnpStart();
    IeparamInit();
    EmailInit();        
    DateThread();

#ifdef SUPPORT_WPS
    InitWps();
#endif

}

extern void YiNetWorkThread( void );
int main( void )
{
#ifndef UNABLE_AUDIO
    AudioCodecInit();
	AudioCodecOpen();
	
#ifdef SUPPORT_FM34
    fm_Init();
#else 
	CheckAudioChip();
#endif

    AudioStart();
#endif

    BaSysTimerInit();

    Resourceinit();
    serverinit();
    InitMoto();
    MotoThreadStart();

#if 0
        SetEncPower(0);
        sleep(1);
        SetEncPower(1);
#endif

#if 0
#ifndef UNABLE_AUDIO
        AudioCodecInit();
        AudioCodecOpen();
        
#ifdef SUPPORT_FM34m
        fm_Init();
#else 
        CheckAudioChip();
#endif
    
        AudioStart();
#endif
#endif


	SendSystemVer();

	if ( bparam.stBell.bell_audio == FALSE )
	{
		//Empty Parameters
		if ( bparam.stBell.a == 0 && bparam.stBell.b == 0 )
		{
			bparam.stBell.a = 1;
			bparam.stBell.b = 1;
			bparam.stBell.bell_audio = 1;
			SaveSystemParam( &bparam );
		}
	}

	SetBellAlarmOn(bparam.stBell.alarm_on);

	#ifdef ZHENGSHOW

	#if defined (UK_CUSTOMERS_OLD_KERNEL)|| defined  (UK_CUSTOMERS_NEW_KERNEL)
    Textout("-------------------------------this is UK customers-----------------------------");
	if(access("/system/Wireless/mute.wav",F_OK) == 0 )
		DoSystem("rm /system/Wireless/mute.wav");

	if(access("/system/Wireless/alarm.wav",F_OK) == 0 )
		DoSystem("rm /system/Wireless/alarm.wav");

	if(access("/system/Wireless/open.wav",F_OK) == 0 )
		DoSystem("rm /system/Wireless/open.wav");

	if(access("/system/Wireless/startup.wav",F_OK) == 0 )
		DoSystem("rm /system/Wireless/startup.wav");

	if(access("/system/Wireless/talk-over.wav",F_OK) == 0 )
		DoSystem("rm /system/Wireless/talk-over.wav");

	if(access("/system/Wireless/reboot.wav",F_OK) == 0 )
		DoSystem("rm /system/Wireless/reboot.wav");

	if(access("/system/Wireless/talk-timeout.wav",F_OK) == 0 )
		DoSystem("rm /system/Wireless/talk-timeout.wav");

	if(access("/system/Wireless/wps.wav",F_OK) == 0 )
		DoSystem("rm /system/Wireless/wps.wav");

	if(access("/system/Wireless/configok.wav",F_OK) == 0 )
		DoSystem("rm /system/Wireless/configok.wav");

	
	bparam.stBell.alarm_on = 0;
	bparam.stBell.alarm_onok = 0;
	SetBellAlarmOn(0);

	//bparam.stBell.lock_delay = 1;
	
	NoteSaveSem();
	
    #else

    if(access("/param/zhengshi",F_OK) != 0 )
	{
		DoSystem("touch /param/zhengshi");
		
		bparam.stBell.alarm_on = 0;
		bparam.stBell.alarm_onok = 0;
		SetBellAlarmOn(0);

		NoteSaveSem();
	}


     ControlIO(BELL_NOTYCE_TO_MCU,1);

	#endif
	#endif

	SytemStartupLed();
    #ifdef JPUSH
    ReadJPushParams();
    #endif

    #ifdef LDPUSH
    ReadPushParams();
	LdPushStart();
    #endif
    
	if ( !ExistGpsFile())
	{
	    #ifdef DELAY_STARTUP       
	    StartAudioPlay(WF_MUTE, 10, BaAudioDelayStartup);
	    #else
		StartAudioPlay(WF_STARTUP, 1, NULL);
		#endif
	}

    #ifndef BAFANGDIANZI
	Textout("bparam.stBell.lock_type=%d",bparam.stBell.lock_type);
    if ( bparam.stBell.lock_type == 1 )//nc
    {
        #ifdef SUPPORT_FM34
		Textout("set lock is 0");
        ControlIO(BELL_OPENDOOR, 0);
		#ifdef NEW_BRAOD_AES
		ControlIO(BELL_OPENDOOR2, 0);
		#endif
        #else
        ControlIO(BELL_OPENDOOR, 1);
		Textout("set lock is 1");
        #endif
    }
	else//no
	{
		#ifdef SUPPORT_FM34
		Textout("set lock is 1");
        ControlIO(BELL_OPENDOOR, 1);
		#ifdef NEW_BRAOD_AES
		ControlIO(BELL_OPENDOOR2, 1);
		#endif
        #else
		Textout("set lock is 0");
        ControlIO(BELL_OPENDOOR, 0);
        #endif
	}
	#endif

	#ifdef YINETWORK
	YiNetWorkThread();
	#endif

	#ifdef NEW_BRAOD_AES
	AutoRebootInit();
	#endif
    while ( 1 )
    {
        sleep( 60 );
        //ControlIO(BELL_NOTYCE_TO_MCU,0);
        //Textout("11111111111111111111111111111111111111111111111111111");
    }

    return 0;
}

