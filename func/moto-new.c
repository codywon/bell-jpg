#include <sys/types.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>             /* signal */
#include <semaphore.h>
#include "ralink_gpio.h"

#include "alarm.h"
#include "param.h"
#include "init.h"

#include "moto.h"
#include "debug.h"
#include "ipcparam.h"


#if	0
#define	IOTextout(Fmt, args...)
#else
#define	IOTextout	Textout
#endif

/* BEGIN: Added by yiqing, 2015/4/10 */
#define FILE_CAPTURE    "/tmp/photo.jpg"


void CheckDefKey( void );

#define DIR_LEFTRIGHT_STOP		0x00
#define DIR_LEFT					0x01
#define DIR_RIGHT				0x02

#define DIR_UPDOWN_STOP		0x00
#define DIR_UP					0x01
#define DIR_DOWN				0x02

int motofd = -1;
int portvalue = 0;
int gpiovalue = 0;

struct timeval stAlarmMotionTime;

BOOL bAudioPowerAmplifierStatus = FALSE;

void CheckDefKey( void );

int ReadGpio( void )
{
	static int nPrevValue = 0;
    int 	iRet;
    iRet = ioctl( motofd, RALINK_GPIO_READ, &gpiovalue );
	if ( nPrevValue != gpiovalue )
	{
		//if ( gpiovalue != 0x00303807 && gpiovalue != 0x00302807 )
		//{
		//	IOTextout("GPIO Value Changed = %08X, old = %08X", gpiovalue, nPrevValue);
		//}
		nPrevValue = gpiovalue;
	}
    return iRet;
}

BOOL IsFreeTime();

BOOL IsRedIrEffective()
{
#ifdef FEEDDOG
    if(gpiovalue & _MOTO_LEFT)
        return FALSE;
    else
        return TRUE;
#endif


#ifdef FUHONG
if(gpiovalue & _MOTO_DOWN)
    return FALSE;
else
    return TRUE;
#endif
}

/* add begin by yiqing, 2015-07-16, 原因: */
void SetEncPower(int  value)
{
    int iRet = 0;
     Textout("set encPower:%d",value);
    if(value)
    {
        portvalue |= _ENCPOWER;
        portvalue |= _MOTO_D3;
    }
    else
    {
        portvalue &= ~_ENCPOWER;
        portvalue &= ~_MOTO_D3;
    }
    iRet=ioctl( motofd, RALINK_GPIO_WRITE, portvalue );
    Textout("iRet:%d",iRet);
}
/* add end by yiqing, 2015-07-16 */

BOOL bMotoActionEnable = 1;

void setMotoActionFlag(BOOL type)
{
	bMotoActionEnable = type;
}
void* MotoActionProc( void* p )
{
    while ( 1 )
    {
        if( 0 == bMotoActionEnable )
        {
            sleep(1);
            continue;
        }
        ReadGpio();

		CheckDefKey();

        #if defined (FEEDDOG) || defined (FUHONG)
		if(IsFreeTime())
		    portvalue &= ~BELL_IR_OUT;
        #endif
        
        #if defined (FUHONG)
		if(IsFreeTime())
		    portvalue |= BELL_IR_OUT;
        #endif

        ioctl( motofd, RALINK_GPIO_WRITE, portvalue );

        
        //printf("portvalue=0x%08x\n", portvalue);
        usleep( 10*1000 );
    }

    return NULL;
}

void ControlIO(int nAddress, BOOL bOpen)
{
	if ( bOpen )
    {
		portvalue |= nAddress;
    }
    else
    {
		portvalue &= ~nAddress;
    }
   
	//IOTextout("SetBellLed = %d\n", bOpen);
	ioctl( motofd, RALINK_GPIO_WRITE, portvalue );
	//printf("portvalue=0x%08x\n", portvalue);
}

#ifdef  KONX
static BOOL bNight = FALSE;
void ControlIRLED();
#endif


extern void OnFirstAlarm(int nAlarmIndex);
void OnIROpen()
{
    Textout("OnIROpen");
    #ifdef FUHONG
    ControlIO(BELL_IR_OUT, 0);
    #endif
    
	#ifdef	FEEDDOG
	ControlIO(BELL_IR_OUT, 1);
	#endif

	#ifdef	ZIGBEE
	ControlIO(BELL_IR_OUT, 0);
	#endif

	#ifdef	KONX
    //ControlIO(BELL_IR_OUT, 0);
    bNight = TRUE;
    /* BEGIN: KONX, Close PIR-LED when need, Set Initial-value, Adjust IR-LED */
    /*        Added by wupm, 2014/12/6 */
    ControlIRLED();
	#endif

    #ifdef SUPPORT_IRCUT

	/* add begin by yiqing, 2015-10-10罗胤洪版本测试，gpio报警*/
	//OnFirstAlarm(AT_PIR);
	//return;
	/* add end by yiqing, 2015-10-10 */
	
    ControlIO(BELL_IR_CONTROL0, 0);
	ControlIO(BELL_IR_CONTROL1, 1);
	usleep(200000);
	ControlIO(BELL_IR_CONTROL0, 0);
	ControlIO(BELL_IR_CONTROL1, 0);
    #endif
}
void OnIRClose()
{
	Textout("OnIRClose");
    #ifdef FUHONG
    ControlIO(BELL_IR_OUT, 0);
    #endif
    
	#ifdef	FEEDDOG
	ControlIO(BELL_IR_OUT, 0);
	#endif

	#ifdef	ZIGBEE
	ControlIO(BELL_IR_OUT, 1);
	#endif

	#ifdef	KONX
    //ControlIO(BELL_IR_OUT, 1);
    bNight = FALSE;
    /* BEGIN: KONX, Close PIR-LED when need, Set Initial-value, Adjust IR-LED */
    /*        Added by wupm, 2014/12/6 */
    ControlIRLED();
	#endif

    #ifdef SUPPORT_IRCUT
    ControlIO(BELL_IR_CONTROL0, 1);
	ControlIO(BELL_IR_CONTROL1, 0);
	usleep(200000);
	ControlIO(BELL_IR_CONTROL0, 0);
	ControlIO(BELL_IR_CONTROL1, 0);
    #endif
}

/* BEGIN: Zigbee */
/*        Added by wupm(2073111@qq.com), 2014/9/9 */
void OnBackLightOpen()
{
	#ifdef	ZIGBEE
	ControlIO(BELL_PIR_OUT, 0);
	bPIRStatus = 0;
	//Textout("OnBackLightOpen");
	#endif
}
void OnBackLightClose()
{
	#ifdef	ZIGBEE
	ControlIO(BELL_PIR_OUT, 1);
	bPIRStatus = 1;
	//Textout("OnBackLightClose");
	#endif
}


void SetBellLock(BOOL bOpen)
{
    
#if defined (KONX)
    //True = OK, Start
    if ( bOpen )
    {
        if ( bparam.stBell.lock_type == 1 ) //High, -->Low
        {
            Textout("Open Lock, N High, Control Lock Low");
            ControlIO(BELL_OPENDOOR, 0);
        }
        else                                //Low, --->High
        {
            Textout("Open Lock, N Low, Control Lock High");
            ControlIO(BELL_OPENDOOR, 1);
        }
    }
    //False = Over
    else
    {
        if ( bparam.stBell.lock_type == 1 ) //High
        {
            Textout("Close Lock, N High, Control Lock High");
            ControlIO(BELL_OPENDOOR, 1);
        }
        else                                //Low
        {
            Textout("Close Lock, N Low, Control Lock Low");
            ControlIO(BELL_OPENDOOR, 0);
        }
    }
#elif (defined(JINQIANXIANG) || defined(BAFANGDIANZI))

#else
#ifdef SUPPORT_FM34
    if ( bparam.stBell.lock_type == 0)//no
    {
        if(1 == bOpen )
        {
            ControlIO(BELL_OPENDOOR, 0);
        }
        else
        {
            ControlIO(BELL_OPENDOOR, 1);
        }
    }
    else                               //nc
    {
        ControlIO(BELL_OPENDOOR, bOpen);
    }
#else
    if ( bparam.stBell.lock_type == 1 )//nc
    {
        if(1 == bOpen )
        {
            ControlIO(BELL_OPENDOOR, 0);
        }
        else
        {
            ControlIO(BELL_OPENDOOR, 1);
        }
    }
    else                               //no
    {
        ControlIO(BELL_OPENDOOR, bOpen);
    }
#endif
#endif
}

void SetBell2Lock(BOOL bOpen)
{
    
#ifdef ZHENGSHOW
    if ( bparam.stBell.lock_type == 0)//no
    {
        if(1 == bOpen )
        {
            ControlIO(BELL_OPENDOOR2, 0);
        }
        else
        {
            ControlIO(BELL_OPENDOOR2, 1);
        }
    }
    else                               //nc
    {
        ControlIO(BELL_OPENDOOR2, bOpen);
    }
#endif
}


/* BEGIN: ACTOP, Red LED */
/*        Added by wupm(2073111@qq.com), 2014/10/13 */
#ifdef	FACTOP
//static BOOL bStartFlash2 = FALSE;
//static BOOL bBellFlashing2 = FALSE;
#define	BELL_FLASH_RATE_FAST2	3	//150	//ms
#define	BELL_FLASH_RATE_SLOW2	10	//300	//ms
#define	CONTINUE_TIMES			1200	//1min=60s=60000ms=1200times
//static int nBellFlashRate2 = BELL_FLASH_RATE_SLOW2;
#define	REDLED_NORMAL	0
#define	REDLED_FAST		1	//Enter AP
#define	REDLED_CONTINUE	2	//Exit AP
#define	REDLED_DELAYING	3
#define	REDLED_SLOW		4	//PIR
#define	REDLED_STOP		5	//Exit PIR
static int nFlashState = REDLED_NORMAL;
void SetBellLed2(BOOL bOpen)
{
	ControlIO(BELL_RED_LED, bOpen);
}
void *BellFlashingProc2(void *p)
{
	int t = 0;
	BOOL bOpen = FALSE;
	while(1)
	{
		if ( nFlashState == REDLED_NORMAL )
		{
			sleep(1);
			continue;
		}
		switch(nFlashState)
		{
			case REDLED_FAST:
				if ( t >= BELL_FLASH_RATE_FAST2 )
				{
					t = 0;
					bOpen = !bOpen;
					SetBellLed2(bOpen);
				}
				break;
			case REDLED_SLOW:
				if ( t >= BELL_FLASH_RATE_SLOW2 )
				{
					t = 0;
					bOpen = !bOpen;
					SetBellLed2(bOpen);
				}
				break;
			case REDLED_CONTINUE:
				t = 0;
				SetBellLed2(TRUE);
				nFlashState = REDLED_DELAYING;
				break;
			case REDLED_DELAYING:
				if ( t >= CONTINUE_TIMES )
				{
					t = 0;
					nFlashState = REDLED_STOP;
				}
				break;
			case REDLED_STOP:
				if ( bparam.stBell.status == NS_OK )
				{
					SetBellLed2(FALSE);
				}
				else
				{
					SetBellLed2(TRUE);
				}
				nFlashState = REDLED_NORMAL;
				break;
		}

		usleep(50 * 1000);
		t++;
	}
	return 0;
}
void ControlBellLED2(int nBellState)
{
	static pthread_t thread_id = 0;
	if ( thread_id == 0 )
	{
		pthread_create( &thread_id, NULL, &BellFlashingProc2, NULL );
		pthread_detach(thread_id);
	}

	switch(nBellState)
	{
		case CB_CLOSE:
			if ( nFlashState == REDLED_NORMAL )
			SetBellLed2(FALSE);
			break;
		case CB_OPEN:
			if ( nFlashState == REDLED_NORMAL )
			SetBellLed2(TRUE);
			break;

		case CB_FLASH_FAST:	//Enter AP
			//SetBellFlashing2(CB_FLASH_FAST);
			nFlashState = REDLED_FAST;
			break;
		case CB_CONTINUE_MOMENT://Exit AP
			nFlashState = REDLED_CONTINUE;
			break;

		case CB_FLASH_SLOW:	//PIR
			//SetBellFlashing2(CB_FLASH_SLOW);
			nFlashState = REDLED_SLOW;
			break;
		case CB_FLASH_STOP:	//PIR End
			//bBellFlashing2 = FALSE;
			//SetBellLed2(FALSE);
			nFlashState = REDLED_STOP;
			break;
	}
}
#endif


#if	1
static BOOL bStartFlash = FALSE;
static BOOL bBellFlashing = FALSE;
#define	BELL_FLASH_RATE_FAST	3	//150	//ms
#define	BELL_FLASH_RATE_SLOW	6	//300	//ms
static int nBellFlashRate = BELL_FLASH_RATE_SLOW;
void SetBellLed(BOOL bOpen)
{
	//Textout("------------bOpen:%d-----------------",bOpen);
#if (defined(JINQIANXIANG) || defined(BAFANGDIANZI) || defined(NEW_BRAOD_AES))
#else
	ControlIO(BELL_LED, bOpen);
#endif
}
void *BellFlashingProc(void *p)
{
	int t = 0;
	BOOL bOpen = FALSE;
	while(1)
	{
		if ( !bStartFlash )
		{
			sleep(1);
			continue;
		}

		if ( !bBellFlashing )
		{
			SetBellLed(FALSE);
			bStartFlash = FALSE;
			//break;
			continue;
		}

		if ( t >= nBellFlashRate )
		{
			t = 0;
			bOpen = !bOpen;
			SetBellLed(bOpen);
		}

		usleep(50 * 1000);
		t++;
	}
	//IOTextout("Flashing... Stop!!");
	return 0;
}
void SetBellFlashing(int nSpeed)
{
	static pthread_t thread_id = 0;

	if ( nSpeed == CB_FLASH_SLOW )
	{
		nBellFlashRate = BELL_FLASH_RATE_SLOW;
		bBellFlashing = TRUE;
	}
	else if ( nSpeed == CB_FLASH_FAST )
	{
		nBellFlashRate = BELL_FLASH_RATE_FAST;
		bBellFlashing = TRUE;
	}

	if ( thread_id == 0 )
	{
		pthread_create( &thread_id, NULL, &BellFlashingProc, NULL );
		pthread_detach(thread_id);
	}

	bStartFlash = TRUE;
}
void ControlBellLED(int nBellState)
{
	switch(nBellState)
	{
		case CB_CLOSE:
			bBellFlashing = FALSE;
			usleep(100*1000);
			SetBellLed(FALSE);
			break;
		case CB_OPEN:
			bBellFlashing = FALSE;
			usleep(100*1000);
			SetBellLed(TRUE);
			break;
		case CB_FLASH_FAST:	//Network Config Mode, Status LED
			SetBellFlashing(CB_FLASH_FAST);
			break;
		case CB_FLASH_SLOW:	//Call
			SetBellFlashing(CB_FLASH_SLOW);
			break;
		case CB_FLASH_STOP:	//Call
			bBellFlashing = FALSE;
			usleep(100*1000);
			SetBellLed(FALSE);
			break;
	}
}
#endif

/***********************************AP MODE START*************************************************/
BOOL NowIsWPSState();

#define	AP_MAX_WAIT_TIME		300		//5Min
static unsigned int nTimerApWaiting = 0;
static BOOL bAPWait = FALSE;
BOOL NowIsAPState()
{
	return bAPWait;
}

void SetAPState(BOOL state)
{
    bAPWait = state;
}

void CallbackApModeWaiting(unsigned int sit)
{  
    if ( bAPWait )
    {
        //IOTextout("AP.... nTimerApWaiting = %d, MAX = %d", nTimerApWaiting, AP_MAX_WAIT_TIME);
        nTimerApWaiting++;       
    }

    if( !bAPWait) 
    {
        IOTextout("AP Mode wifi config ok");
    }

    if( nTimerApWaiting >= AP_MAX_WAIT_TIME) 
    {
        IOTextout("AP Wait Timeout, Close AP");
    }


    if( !bAPWait || nTimerApWaiting >= AP_MAX_WAIT_TIME) 
    {
        UnRegisterBaSysCallback(0xFF, CallbackApModeWaiting);
        CloseAp();
        SetAPState(FALSE);

        /*************exit config mode led*************/
        
    	#ifdef FACTOP
    	/* BEGIN: ACTOP, Resolve Bugs */
    	/*        Added by wupm(2073111@qq.com), 2014/10/18 */
    	ControlBellLED2(CB_CONTINUE_MOMENT);
    	#endif
    	
    	if ( bparam.stBell.status == NS_OK )
    	{
    		/* BEGIN: ACTOP, Red LED */
    		/*        Modified by wupm(2073111@qq.com), 2014/10/13 */
    		#ifdef	FACTOP

    		/* BEGIN: ACTOP, Resolve Bugs */
    		/*        Deleted by wupm(2073111@qq.com), 2014/10/18 */
    		//ControlBellLED2(CB_CLOSE);

    		ControlBellLED(CB_CLOSE);
    		#endif
    		
    		#if defined( ZILINK ) || defined(ZHENGSHOW)
    		ControlBellLED(CB_OPEN);
    		#endif

    		#if defined(FEEDDOG) || defined(FUHONG) || defined(YUELAN) || defined(BELINK)||defined(FOBJECT_FM34)
            ControlBellLED(CB_CLOSE);
    		#endif
    	}
    	else
    	{
    		/* BEGIN: ACTOP, Red LED */
    		/*        Modified by wupm(2073111@qq.com), 2014/10/13 */
    		#ifdef	FACTOP

    		/* BEGIN: ACTOP, Resolve Bugs */
    		/*        Deleted by wupm(2073111@qq.com), 2014/10/18 */
    		//ControlBellLED2(CB_OPEN);

    		ControlBellLED(CB_CLOSE);

    		#else

    		ControlBellLED(CB_FLASH_FAST);
    		#endif
    	}        
    	
        /*************exit config mode led*************/
    }
    
}

void BaAudioApModeCallback(void)
{
    OpenAp();

    /*************enter config mode led*************/
#ifdef  FACTOP
	ControlBellLED2(CB_FLASH_FAST);
	ControlBellLED(CB_FLASH_FAST);
#else
	ControlBellLED(CB_FLASH_FAST);
#endif

    /*************enter config mode led*************/

        
    SetAPState(TRUE);
    nTimerApWaiting = 0;
    RegisterBaSysCallback(0xFF, 1, CallbackApModeWaiting);
}

void OnEnterAPMode()
{
    if(GetPlayAlarmAudioState())
    {
        Textout("doorbell is play audio,ignore this enter");
        return;
    }
    
	IOTextout("Enter AP Mode, bell_mode = %d, wps=%d", bparam.stBell.bell_mode, NowIsWPSState());
	if ( bparam.stBell.bell_mode == 1 && !NowIsWPSState())
    {
        if( !NowIsAPState() )
        {
    		StartAudioPlay( WF_CONFIG, 1, BaAudioApModeCallback);
        }
        else
        {
            nTimerApWaiting = 0;
            IOTextout("Is in AP Mode");
        }
    }
}
/***********************************AP MODE END*************************************************/

/***********************************WPS MODE START**********************************************/
static BOOL bWPSWait = FALSE;
BOOL NowIsWPSState()
{
    return bWPSWait;
}

void SetWPSState(BOOL state)
{
    bWPSWait = state;
}

void BaAudioWpsModeCallback(void)
{
#ifdef SUPPORT_WPS
    SetWPSState(TRUE);
    StopVideoCaptureEx();
    NoteWpsStart();    
#endif    
}

void OnEnterWpsMode()
{
    if(GetPlayAlarmAudioState())
    {
        Textout("doorbell is play audio,ignore this enter");
        return;
    }
    
	Textout("OnEnterWpsMode");
#ifdef SUPPORT_WPS
    if( !NowIsAPState() )
    {
        IOTextout("Enter WPS Mode");

        #ifdef ZILINK
        /*Open LED D2*/
        ControlIO(BELL_WPS_LED, FALSE);
        #endif

        StartAudioPlay( WF_WPS, 1, BaAudioWpsModeCallback);
    }
#endif    
}



/***********************************WPS MODE END***********************************************/

void OnPirOpen()
{
	bPIRStatus = 0;
	Textout("OnPirOpen");
}

void OnPirClose()
{
	bPIRStatus = 1;
	Textout("OnPirClose");
}

/* add begin by yiqing, 2015-09-04, 原因:单片机开门后通知5350处理函数 */
extern void SetMcuNotyceFlag(int flag);
void OnMcuNotyce()
{
    SetMcuNotyceFlag(1);
    Textout("*******************OnMcuNotyce********************");
}/* add end by yiqing, 2015-09-04 */


#if	1
#define	BELL_ALARM_TIME_RESET	10	//reserved signal
#define	BELL_ALARM_TIME_DELAY	30	//inside-delay
typedef	struct
{
	BOOL bMotion;
	BOOL bPir;
	BOOL bReset;
	BOOL bRunning;
	char szTime[64];
}BELL_ALARM_T;
BELL_ALARM_T BellAlarm;
static unsigned int nTimerAlarm = 0;
BOOL IsFreeTime()
{
    if(BellAlarm.bRunning || CheckStartCalling() || IsSendingVideo() ) 
        return FALSE;

    return TRUE;    
}

void CaptureAlarmJPEG()
{

	char szPath[64];
	sprintf(szPath, "/tmp/%s.jpg", BellAlarm.szTime);

	CaptureJpeg(szPath);
	/* BEGIN: Add New CGI: get_doorbelllogs.cgi */
	/*        Modified by wupm(2073111@qq.com), 2014/8/11 */
	#ifdef	ENABLE_BELL_LOG
	//AppendBellLog(BellAlarm.szTime);
	#else
	AppendJpegList(szPath);
	#endif
}

BOOL CheckAlarmTimeSpan()
{
	char szTime[64];
	if ( GetLocalTime(szTime) )
	{
		strcpy(BellAlarm.szTime, szTime);
		return TRUE;
	}
	return FALSE;
}

BOOL CheckAlarmCondition()
{
	if ( !CheckStartCalling() )
	{
		if ( bparam.stBell.alarm_on && bparam.stBell.alarm_onok )
		{
			switch(bparam.stBell.alarm_type)
			{
				case AT_MOTION:
					return BellAlarm.bMotion;
				case AT_PIR:
					return BellAlarm.bPir;
				case AT_MOTION_PIR:				//MOTION and PIR
					return ( BellAlarm.bMotion && BellAlarm.bPir );
			}
		}
	}
	
	return FALSE;
}



#if defined (ZHENGSHOW) || defined (KONX) || defined (BELINK)
#define	IR_CALL		0x00000001
#define	IR_WATCH1	0x00000002
#define	IR_WATCH2	0x00000004
#define	IR_WATCH3	0x00000008
#define	IR_WATCH4	0x00000010
#define	IR_WATCH5	0x00000020
#define	IR_WATCH6	0x00000040
#define	IR_WATCH7	0x00000080
#define	IR_WATCH8	0x00000100
#define	IR_ALARM1	0x00000200
#define	IR_ALARM2	0x00000400
#define IR_IEWATCH  0x00000800
static int nIRState = 0;
void EnableIR(int nIndex)	//0=Call, 1-8=Watch, 9,10=Alarm
{
	Textout("EnableIR nIndex=%d",nIndex);
	int nValue = 0;
	switch(nIndex)
	{
		case 0:
			nValue = IR_CALL;
			break;
		case 1:
			nValue = IR_WATCH1;
			break;
		case 2:
			nValue = IR_WATCH2;
			break;
		case 3:
			nValue = IR_WATCH3;
			break;
		case 4:
			nValue = IR_WATCH4;
			break;
		case 5:
			nValue = IR_WATCH5;
			break;
		case 6:
			nValue = IR_WATCH6;
			break;
		case 7:
			nValue = IR_WATCH7;
			break;
		case 8:
			nValue = IR_WATCH8;
			break;
		case 9:
			nValue = IR_ALARM1;
			break;
		case 10:
			nValue = IR_ALARM2;
			break;
            /* BEGIN: KONX, Close PIR-LED when need, Set Initial-value, Adjust IR-LED */
            /*        Added by wupm, 2014/12/6 */
        case 11:
            nValue = IR_IEWATCH;
            break;
		default:
			return;
	}
	nIRState |= nValue;

#ifdef  KONX
    if ( bNight && (nIRState != 0))
    {
        ControlIO(BELL_IR_CONTROL, 0);	//Open
        
        /* BEGIN: Open IRLED, sleep sometime, then make PHOTO, and close IRLED */
        /*        Added by wupm, 2014/12/9 */
        if ( nIndex == 9 || nIndex == 10 || nIndex == 0 )
        {
			sleep(1);
        }
		/* END:   Added by wupm, 2014/12/9 */
    }
    else
    {
        ControlIO(BELL_IR_CONTROL, 1);	//Close
    }
#else
#ifndef SUPPORT_IRCUT
    ControlIO(BELL_IR_CONTROL, !(nIRState == 0 ));
#endif
#endif
		
}
void DisableIR(int nIndex)
{
	Textout("DisableIR nIndex=%d",nIndex);

	int nValue = 0;
	switch(nIndex)
	{
		case 0:
			nValue = IR_CALL;
			break;
		case 1:
			nValue = IR_WATCH1;
			break;
		case 2:
			nValue = IR_WATCH2;
			break;
		case 3:
			nValue = IR_WATCH3;
			break;
		case 4:
			nValue = IR_WATCH4;
			break;
		case 5:
			nValue = IR_WATCH5;
			break;
		case 6:
			nValue = IR_WATCH6;
			break;
		case 7:
			nValue = IR_WATCH7;
			break;
		case 8:
			nValue = IR_WATCH8;
			break;
		case 9:
			nValue = IR_ALARM1;
			break;
		case 10:
			nValue = IR_ALARM2;
			break;
            /* BEGIN: KONX, Close PIR-LED when need, Set Initial-value, Adjust IR-LED */
            /*        Added by wupm, 2014/12/6 */
        case 11:
            nValue = IR_IEWATCH;
            break;
		default:
			return;
	}
	nIRState &= ~nValue;

#ifdef  KONX
    if ( bNight && (nIRState != 0))	
    {   
		ControlIO(BELL_IR_CONTROL, 0);	//Open
		
        /* BEGIN: Open IRLED, sleep sometime, then make PHOTO, and close IRLED */
        /*        Added by wupm, 2014/12/9 */
        if ( nIndex == 9 || nIndex == 10 || nIndex == 0 )
        {
			sleep(1);
        }
		/* END:   Added by wupm, 2014/12/9 */
    }    
    else
    {

        ControlIO(BELL_IR_CONTROL, 1);
    }
#else
#ifndef SUPPORT_IRCUT
	ControlIO(BELL_IR_CONTROL, !(nIRState == 0 ));
#endif
#endif
}

/* BEGIN: KONX, Close PIR-LED when need, Set Initial-value, Adjust IR-LED */
/*        Added by wupm, 2014/12/6 */
void ControlIRLED()
{
#ifdef  KONX
    if ( bNight && (nIRState != 0))
	{   
		ControlIO(BELL_IR_CONTROL, 0);	
    }
    else
    {
        ControlIO(BELL_IR_CONTROL, 1);
    }
#endif
}


#endif

void ResetBellAlarm()
{
	memset(&BellAlarm, 0, sizeof(BELL_ALARM_T));
}

void CallbackAlarmPeriod(unsigned int sit)
{
    nTimerAlarm++;
    if ( nTimerAlarm >= BELL_ALARM_TIME_DELAY || BellAlarm.bRunning == FALSE)
    {
        ResetBellAlarm();
        UnRegisterBaSysCallback(0xFF, CallbackAlarmPeriod);
        IOTextout("Bell Alarm Inside Delaying END!!");
    }
}

void CallbackAlarmDelay(unsigned int sit)
{
    nTimerAlarm++;
    IOTextout("Bell Alarm Delaying..... t = %d", nTimerAlarm);
    if ( BellAlarm.bRunning == TRUE )
    {
        if ( nTimerAlarm >= bparam.stBell.alarm_delay )
        {
            if ( CheckAlarmCondition() )
            {
                //notify
                IOTextout("OnFoundAlarm = [%s]", BellAlarm.szTime);
                OnFoundAlarm(BellAlarm.bMotion, BellAlarm.bPir, BellAlarm.szTime);
        
                nTimerAlarm = 0;
                UnRegisterBaSysCallback(0xFF, CallbackAlarmDelay);
                RegisterBaSysCallback(0xFF, 1, CallbackAlarmPeriod);
            }
            else
            {
                IOTextout("Break Alarm Delaying Because Condition ERROR");
                ResetBellAlarm();
                UnRegisterBaSysCallback(0xFF, CallbackAlarmDelay);
            }
        }
    }
    else
    {
        IOTextout("Break Alarm Delaying by Other(Call)..");
        UnRegisterBaSysCallback(0xFF, CallbackAlarmDelay);
    }
}

void StartAlarmThread(int nAlarmIndex)
{   
	switch(nAlarmIndex)
	{
		case AT_MOTION:
		    //IOTextout("Motion alarm");
			BellAlarm.bMotion = TRUE;
			break;
		case AT_PIR:
		    IOTextout("PIR alarm");
			BellAlarm.bPir = TRUE;
			break;

		case AT_MOTION_PIR:
			BellAlarm.bMotion = TRUE;
			BellAlarm.bPir = TRUE;
			break;
	}

    if(BellAlarm.bRunning == TRUE)
    {
        //IOTextout("Still in alarm status");
        return;
    }

    BellAlarm.bRunning = TRUE;

    /*whether allow alarm, and capture photo*/
    if ( CheckAlarmCondition() )
    {
        if ( CheckAlarmTimeSpan() )
        {
            
            #if defined (FEEDDOG) || defined (FUHONG)
            if(IsRedIrEffective())
                OnIROpen();
            #endif    
            
            #if defined (ZHENGSHOW) || defined (KONX) || defined (BELINK)
            EnableIR(10);
            #endif
            
            CaptureAlarmJPEG();
            
            
            #if defined (ZHENGSHOW) || defined (KONX) || defined (BELINK)
            DisableIR(10);
            #endif
        

            
            if ( bparam.stBell.alarm_delay == 0 )
            {
                //notify
                IOTextout("OnFoundAlarm = [%s]", BellAlarm.szTime);
                OnFoundAlarm(BellAlarm.bMotion, BellAlarm.bPir, BellAlarm.szTime);

                nTimerAlarm = 0;
                RegisterBaSysCallback(0xFF, 1, CallbackAlarmPeriod);
            }
            else
            {
                nTimerAlarm = 0;
                RegisterBaSysCallback(0xFF, 1, CallbackAlarmDelay);
            }
        }
        else
        {
            ResetBellAlarm();
        }
    }
    else
    {
        ResetBellAlarm();
        //IOTextout("Discard alarm, bStartCall=%d, alarm_on=%d, alarm_onok=%d, alarm_type=%d", 
        //        CheckStartCalling(), bparam.stBell.alarm_on, bparam.stBell.alarm_onok, bparam.stBell.alarm_type);
    }

	
}

void OnFirstAlarm(int nAlarmIndex)
{
	/* modify begin by yiqing, 2015-11-11 ,在观看视频的时候不报警*/
	//if(NowIsAPState() || NowIsWPSState() )
    if(NowIsAPState() || NowIsWPSState()|| !IsFreeTime() )
    {	
    	//Textout("the alarm type is %d",nAlarmIndex);
        //IOTextout("IPC config WIFI(in AP-%d or in WPS-%d or in FreeTime-%d), forbiden alarm", NowIsAPState(), NowIsWPSState(),!IsFreeTime());
        return;
    }
    
	switch(nAlarmIndex)
	{
		case AT_MOTION:
		case AT_PIR:
		case AT_MOTION_PIR:
			StartAlarmThread(nAlarmIndex);
			break;
	}
	//Textout("AlarmIndex:%d",nAlarmIndex);
}

void OnMotionDetected()
{
	OnFirstAlarm(AT_MOTION);
}

void OnPIRDetected()
{
	OnFirstAlarm(AT_PIR);
}

void OnPirAndMotionDetected()
{
	OnFirstAlarm(AT_MOTION_PIR);

}

void OnDoorNvrAlarm()
{
	if(BellAlarm.bMotion)
	    NvrAlarmSend(eDoorBell_Motion);
	if(BellAlarm.bPir)
	    NvrAlarmSend(eDoorBell_PIR);
}
#endif

/* BEGIN: KONX, Close PIR-LED when need, Set Initial-value, Adjust IR-LED */
/*        Added by wupm, 2014/12/6 */
#ifdef  KONX
static BOOL bReSetKonxLedOn = FALSE;
static BOOL bSetKonxLedOn = FALSE;
void *OnOpenLedProc(void *p)
{
    int t = 0;
    while(1)
    {
        if ( bReSetKonxLedOn )
        {
            t = 0;
            bSetKonxLedOn = TRUE;
            bReSetKonxLedOn = FALSE;
            Textout("Open Konx LED when PIR================");
#ifndef NEW_BRAOD_AES
            ControlIO(BELL_LED, 1);
#endif
        }

        if ( bSetKonxLedOn )
        {
            if ( t >= bparam.stBell.alarm_delay )
            {
                if ( !CheckStartCalling() )
                {
                    Textout("Close Konx LED when PIR Over================");
#ifndef NEW_BRAOD_AES
                    ControlIO(BELL_LED, 0);
#endif
                }
                else
                {
                    Textout("Close Konx LED when PIR Over, But Calling , So Dothing================");
                }
                bSetKonxLedOn = FALSE;
            }
            t++;
        }
        sleep(1);
    }
}
void ReSetKonxLedOn()
{
    bReSetKonxLedOn = TRUE;
}
#endif


void OnPir()
{
	/* BEGIN: KONX */
	/*        Modified by wupm(2073111@qq.com), 2014/10/25 */
	//OnFirstAlarm(AT_PIR);
    Textout("********************PIR***********************");

	if ( NowIsAPState() )
	{
	}
	else
	{
#ifdef  KONX
        Textout("=====================Open When PIR");
        /* BEGIN: KONX, Close PIR-LED when need, Set Initial-value, Adjust IR-LED */
        /*        Added by wupm, 2014/12/6 */
#if 0
        ControlIO(BELL_LED, 1);
#else
        if ( 1 )
        {
            static pthread_t threadOpenLed = 0;
            if ( threadOpenLed == 0 )
            {
                pthread_create( &threadOpenLed, NULL, &OnOpenLedProc, NULL );
                pthread_detach(threadOpenLed);
            }
            ReSetKonxLedOn();
        }
#endif
#endif
        
		OnFirstAlarm(AT_PIR);
	}
}

#ifdef	FACTOP
void OnPirOn()
{
	/* BEGIN: ACTOP, Resolve Bugs */
	/*        Modified by wupm(2073111@qq.com), 2014/10/18 */
	if ( NowIsAPState() )
	{
	}
	else
	{
		/* BEGIN: ACTOP: Change RED-LED Logic, On or Off */
		/*        Modified by wupm(2073111@qq.com), 2014/10/21 */
		//ControlBellLED2(CB_FLASH_SLOW);
		//ControlBellLED2(CB_OPEN);
		ControlBellLED2(CB_CLOSE);

		OnFirstAlarm(AT_PIR);
	}
}
void OnPirClose()
{
	if ( NowIsAPState() )
	{
	}
	else
	{
		/* BEGIN: ACTOP: Change RED-LED Logic, On or Off */
		/*        Modified by wupm(2073111@qq.com), 2014/10/21 */
		//ControlBellLED2(CB_FLASH_STOP);
		//ControlBellLED2(CB_CLOSE);
		ControlBellLED2(CB_OPEN);
	}
}
#endif

void OnDoorOpen();

extern void SetAudioGongFang(BOOL bOpen);
void OnPressCall()
{
    
    
    if(NowIsAPState() || NowIsWPSState())
    {
        IOTextout("IPC config WIFI(in AP-%d or in WPS-%d), forbiden Calling", NowIsAPState(), NowIsWPSState());
        return;
    }
	

	IOTextout("Press Button OK");
    
	StartCall();
}
void OnReset()
{
	IOTextout("OnReset");
	DoSystem( "rm /system/www/BellParam.bin" );

#ifdef LDPUSH
    DoSystem( "rm /param/pushparamlist.bin" );
#endif
    SetRebootCgi();
}

void OnReboot()
{
    SetRebootCgi();
}

void BaAudioResetCallback(void)
{
	OnReset();
}
void OnPressReset()
{
    Textout("OnPressReset");
	StartAudioPlay(WF_RESET_OK, 1, BaAudioResetCallback);
}

void OnAlarmIn();

/* BEGIN: KONX */
/*        Added by wupm(2073111@qq.com), 2014/10/25 */
void OnPressAlarm();
void OnAlarmInPressDown();


void OnOpenAp()
{
    OpenAp();        
    SetAPState(TRUE);
}

void OnOpenSta()
{
    CloseAp();
    SetAPState(FALSE);
}

void OnPressCapture()
{
    sendP2PAlarm(NOTE_CLIENT_CAP_PIC);
}

#ifdef JINQIANXIANG
void OnPressRecord()
{
    sendP2PAlarm( NOTE_CLIENT_RECORD );
}
#endif

#ifdef BAFANGDIANZI
void OnSenSorIn()
{
	int bCaptureOk = -1;

	
	Textout("OnSenSorIn");
	
	//本地拍照上传到ftp服务器
	bCaptureOk = CaptureJpeg(FILE_CAPTURE);
	gettimeofday(&stAlarmMotionTime, NULL);
	Textout("bCaptureOk:%d",bCaptureOk);
	AlarmFtp( 1, &stAlarmMotionTime, bCaptureOk);

	//通知手机拍照
	OnPressCapture();
}
#endif

BELLGPIO BellGpio[MAX_BELL_GPIO_COUNT] =
{
#ifdef	FOBJECTTEST
	{BELL_RESET, 	1, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
#endif

#ifdef	FOBJECT
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	0, 	NULL},
	{BELL_RESET, 	1, -1, 0, 2, 	OnEnterWpsMode, 20, OnReset},
#endif

#ifdef	FOBJECT_FM34
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	-1, NULL},
	//{BELL_RESET, 	1, -1, 0, 2, 	OnEnterWpsMode, 20, OnReset},
    {BELL_RESET, 	1, -1, 0, 2, 	OnEnterWpsMode, 20, OnPressReset},
	{BELL_PIR_IN,  -1, -1, 0, 2, 	OnPirOpen,      -1, OnPirClose},
	#endif
	
#ifdef	JINQIANXIANG
	{BELL_AP_P2P, 	1, -1, 0, 4, 	OnOpenAp, 	-1, OnOpenSta},
	{BELL_CAPTURE, 	1, -1, 0, 4, 	OnPressCapture, 	0, 	NULL},
	{BELL_RECORD, 	1, -1, 0, 2, 	OnPressRecord, 0, NULL},
	{BELL_CAPTURE2, 	1, -1, 0, 4, 	OnPressCapture, 	-1, NULL},
#endif



#ifdef	FUHONG
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	0, 	NULL},
	{BELL_RESET, 	1, -1, 0, 2, 	OnEnterWpsMode, 20, OnReset},
#endif

#ifdef	EYESIGHT
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	0, 	NULL},
	{BELL_RESET, 	1, -1, 0, 2, 	OnEnterWpsMode, 20, OnReset},
#endif

#ifdef	KANGJIEDENG
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	0, 	NULL},
	{BELL_RESET, 	1, -1, 0, 2, 	OnEnterWpsMode, 20, OnReset},
#endif


#ifdef	FRISEN
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	0, 	NULL},
	{BELL_RESET, 	1, -1, 0, 2, 	OnEnterWpsMode, 20, OnReset},
#endif

#ifdef	ZHENGSHOW
#ifdef NEW_BRAOD_AES
	{BELL_BUTTON, 	    0, -1, 0, 2, 	OnPressCall, 	-1,NULL},
	{AP_KEY, 	        0, -1, 0, 2, 	OnEnterAPMode, 	-1, NULL},
#else
	{BELL_BUTTON, 	    0, -1, 0, 2, 	OnPressCall, 	42, OnEnterAPMode},
#endif
	{BELL_ALARMIN, 	    1, -1, 0, 4, 	OnDoorOpen, 	0, 	NULL},
	{BELL_RESET, 	    1, -1, 0, 2, 	OnEnterWpsMode, 30, OnPressReset},
    {BELL_PIR,         -1, -1, 0, 4, 	OnIROpen, 		-1,	OnIRClose},
#endif

/*
#ifdef	FACTOP
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	0, 	NULL},
	{BELL_RESET, 	1, -1, 0, 10, 	OnReset, 		0, 	NULL},
	{BELL_PIR, 		0, -1, 0, 4, 	OnPir, 			0, 	NULL},
#endif
*/
//Once 50ms
//Push Button(OnPressCall) = 100ms = 2times
//Long Press(OnEnterAPMode) = 3000ms = 60times
//First Press(Reset) = 10 000ms = 200times
//Pust Reset(Reset) = 500ms = 10times
#ifdef	FACTOP
	//{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	60, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	0, 	NULL},
	//{BELL_RESET, 	1, -1, 0, 2,    OnEnterWpsMode, 10, 	OnPressReset},	
	{BELL_RESET, 	1, -1, 0, 10, 	OnPressReset, 	0, 	NULL},
	{BELL_PIR, 		0, -1, 0, 4, 	OnPirOn, 		-1, OnPirClose},
#endif
/* BEGIN: Ai-Hua-xun */
/*        Modified by wupm(2073111@qq.com), 2014/8/8 */
#ifdef	FEEDDOG
#ifndef ZHONGKONG
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
#else
    {BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	120, OnEnterAPMode},
#endif
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	-1, NULL},
	{BELL_RESET, 	1, -1, 0, 2,    OnEnterWpsMode, 10, 	OnReset},
	{BELL_PIR, 		0, -1, 0, 1, 	OnPir, 			-1, 	NULL},
#endif


#ifdef	YUELAN
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	0, 	NULL},
	{BELL_RESET, 	1, -1, 0, 2,    OnEnterWpsMode, 10, 	OnReset},
	{BELL_PIR, 		0, -1, 0, 1, 	OnPir, 			0, 	NULL},
#endif

#ifdef	BELINK
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	120, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnDoorOpen, 	0, 	NULL},
	//{BELL_RESET, 	1, -1, 0, 2,    OnEnterWpsMode, 10, 	OnReset},
#endif

/* BEGIN: Zigbee */
/*        Added by wupm(2073111@qq.com), 2014/9/9 */
#ifdef	ZIGBEE
	{BELL_BUTTON, 	1, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnAlarmIn,		0, 	NULL},
	{BELL_IR_IN,   -1, -1, 0, 4, 	OnIROpen, 		-1,	OnIRClose},
	{BELL_PIR_IN,  -1, -1, 0, 2, 	OnBackLightOpen,-1, OnBackLightClose},
	{BELL_RESET, 	1, -1, 0, 2,    OnEnterWpsMode, 10, 	OnReset},
#endif
/* BEGIN: KONX */
/*        Added by wupm(2073111@qq.com), 2014/10/25 */
#ifdef	KONX
	{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	30, OnEnterAPMode},
//   {BELL_ALARMIN,  0, -1, 0, 4,    OnAlarmIn,      0,  NULL},
    {BELL_ALARMIN,  -1,-1, 0, 4,    OnAlarmIn,      -1, OnAlarmInPressDown},
    {BELL_RESET,    1, -1, 0, 10,   OnPressReset,   0,  NULL},
    {BELL_PIR_IN,   0, -1, 0, 0,    OnPir,          0,  NULL},
	{BELL_SWITCH,   0, -1, 0, 4, 	OnPressAlarm,	0,  NULL},
	{BELL_IR_DETECT,-1,-1, 0, 4, 	OnIROpen, 		-1,	OnIRClose},
#endif

#ifdef	ZILINK
	//{BELL_BUTTON, 	0, -1, 0, 2, 	OnPressCall, 	160, OnEnterAPMode},
	/* add begin by yiqing, 2015-08-31, 原因:ap模式由8秒改为5秒 */
    {BELL_BUTTON,   0, -1, 0, 2,    OnPressCall,    100, OnEnterAPMode},

	{BELL_ALARMIN, 	1, -1, 0, 4, 	OnAlarmIn,		0, 	NULL},
	{BELL_RESET, 	1, -1, 0, 2,    OnReboot,      160, OnPressReset},
#endif

#ifdef	BAFANGDIANZI
	{BELL_AP,	    1, -1, 0, 4,	OnOpenAp,	   30, OnOpenSta},
	{BELL_SENSOR_IN,1, -1, 0, 2, 	OnSenSorIn,		0, 	NULL},
#endif



};

//{BELL_AP,       1, -1, 0, 2, 	OnEnterAPMode,		0, 	NULL},

/*
*/
/* BEGIN: ACTOP, When First Long-Press-Button(10s) = Reset  */
/*        Added by wupm(2073111@qq.com), 2014/10/17 */
#ifdef	FACTOP
#define	LONG_PRESS_BUTTON_TIMES	200
static BOOL bFirstPress = TRUE;
#endif

void* DefKeyProc( void* p )
{
    int i = 0;
    BOOL bLongAction[MAX_BELL_GPIO_COUNT];
    memset(bLongAction, 0, MAX_BELL_GPIO_COUNT * sizeof(BOOL));

	BOOL bKeyRelease[MAX_BELL_GPIO_COUNT];
	memset(bKeyRelease, 1, MAX_BELL_GPIO_COUNT * sizeof(BOOL));

    while ( 1 )
    {
		usleep(50*1000);

		for(i = 0; i<MAX_BELL_GPIO_COUNT; i++)
		{
			/* BEGIN: Ai-Hua-xun */
			/*        Added by wupm(2073111@qq.com), 2014/8/8 */
			if (BellGpio[i].long_times == -1 && BellGpio[i].OnLongPress != NULL )
			{
				//Switch IO
				if ( BellGpio[i].value != BellGpio[i].normal )
				{
					BellGpio[i].total ++;
					if ( BellGpio[i].total > BellGpio[i].times )
					{
						BellGpio[i].normal = BellGpio[i].value;
						BellGpio[i].total = 0;
						if ( BellGpio[i].value == 0 )
						{
							if ( BellGpio[i].OnPress != NULL )
							{
								//Textout("Call OnPress");
								BellGpio[i].OnPress();
							}
						}
						else
						{
							if ( BellGpio[i].OnLongPress != NULL )
							{
								//Textout("OnLongPress");
								BellGpio[i].OnLongPress();
							}
						}
					}
				}
			}

			else
			{

				if ( BellGpio[i].value != BellGpio[i].normal )
				{
					BellGpio[i].total ++;
					//Textout("total=%d, longtimes=%d", BellGpio[i].total, BellGpio[i].long_times);
                    if ( BellGpio[i].long_times > 0 && BellGpio[i].total > BellGpio[i].long_times && bLongAction[i] == FALSE)
                    {
                        bLongAction[i] = TRUE;
                        if ( BellGpio[i].OnLongPress != NULL )
                        {
                            BellGpio[i].OnLongPress();
                        }
                    
                        /* BEGIN: ACTOP, When First Long-Press-Button(10s) = Reset  */
                        /*        Added by wupm(2073111@qq.com), 2014/10/17 */
                        #ifdef  FACTOP
                        bFirstPress = FALSE;
                        #endif
                    }
					else if(BellGpio[i].long_times == -1 && BellGpio[i].OnLongPress == NULL && bKeyRelease[i] == TRUE)
					{
						bKeyRelease[i] = FALSE;
						
						if ( BellGpio[i].OnPress != NULL )
						{
							BellGpio[i].OnPress();
						}
					}
					
				}
				else
				{
					if ( BellGpio[i].total > 0 )
					{
						//Textout("BellGpio[%d]:%d  total:%d",i,BellGpio[i].value,BellGpio[i].total);
						/* BEGIN: ACTOP, When First Long-Press-Button(10s) = Reset  */
						/*        Added by wupm(2073111@qq.com), 2014/10/17 */
						#ifdef	FACTOP
						if ( bFirstPress && BellGpio[i].long_times > 0 && BellGpio[i].total > LONG_PRESS_BUTTON_TIMES )
						{
							/* BEGIN: ACTOP, Resolve Bugs */
							/*        Modified by wupm(2073111@qq.com), 2014/10/18 */
							//OnReset();
							//StartPlayAlarmAudio(WF_REBOOT, 1, OnReset);
							OnPressReset();
						}
						else
						#endif

						if ( BellGpio[i].total > BellGpio[i].times  && bLongAction[i] == FALSE && bKeyRelease[i] == TRUE )
						{
							if ( BellGpio[i].OnPress != NULL )
							{
								BellGpio[i].OnPress();
							}

							/* BEGIN: ACTOP, When First Long-Press-Button(10s) = Reset  */
							/*        Added by wupm(2073111@qq.com), 2014/10/17 */
							#ifdef	FACTOP
							bFirstPress = FALSE;
							#endif
						}
						BellGpio[i].total = 0;
					}

					bLongAction[i] = FALSE;
					bKeyRelease[i] = TRUE;
				}
			}
		}
    }
}

void CheckDefKey( void )
{
	int i;
	for(i=0; i<MAX_BELL_GPIO_COUNT; i++)
	{
		//Check
		//printf("gpiovalue=0x%08x\n", gpiovalue);
		if ( gpiovalue & BellGpio[i].address )
		{
			//BellGpio[i].value = BellGpio[i].normal;		//NORMAL
			//if ( i == 4 )	Textout("io Value = 1");
			BellGpio[i].value = 1;
		}
		else
		{
			//BellGpio[i].value = !BellGpio[i].normal;	//CHANGED!!
			//if ( i == 4 )	Textout("io Value = 0");
			BellGpio[i].value = 0;
		}
	}
}

extern ALARMACTION	motionparam;
extern ALARMACTION	gpioparam;

void ResetAlarmParam()
{
	memset(&motionparam, 0, sizeof(ALARMACTION));
	memset(&gpioparam, 0, sizeof(ALARMACTION));
}

void SetAlarmParam()
{
	gpioparam.flag = 1;
	motionparam.flag =1;
	motionparam.time = 0;
}

static int	bMotionWaiting = FALSE;
void SetMotionWaiting(int bWaiting)
{
	bMotionWaiting = bWaiting;
}

void SetAudioPowerOpen( int value )
{
    /* modify begin by yiqing, 2015-07-17, 原因: 测试*/
	ControlIO(_ENCPOWER, value);	//YES, Set to 1, POWER is HIGH(Default)
}

static unsigned int nTimerCloseGongfang = 0;
void BaDelayCloseGongFangTimer(unsigned int sit)
{
    nTimerCloseGongfang++;
    if(nTimerCloseGongfang >= 2)
    {
        #ifdef BELL_AUDIO_V2
        gpiovalue &= ~BELL_AUDIO_V2;
        #endif
    
        #ifdef BELL_AUDIO
        
        //portvalue |= BELL_AUDIO;

            //#ifdef PREFIX_ZSKJ
             //portvalue &= ~BELL_AUDIO;
            //#else
            portvalue |= BELL_AUDIO;
            //#endif
            
        #endif

        #ifdef BELL_AUDIO_2ND
        portvalue &= ~BELL_AUDIO_2ND;
        #endif

        Textout("Close GongFang");
        
        UnRegisterBaSysCallback(0xFF, BaDelayCloseGongFangTimer);
    }
}


void SetAudioGongFang(BOOL bOpen)
{
    //Textout("SetAudioGongFang  %d",bOpen);
#ifndef UNABLE_AUDIO
    if(bOpen)
    {
        UnRegisterBaSysCallback(0xFF, BaDelayCloseGongFangTimer);

        #ifdef BELL_AUDIO_V2
        gpiovalue |= BELL_AUDIO_V2;
        #endif


        #ifdef BELL_AUDIO
        //portvalue &= ~BELL_AUDIO;

            //#ifdef PREFIX_ZSKJ
            //portvalue |= BELL_AUDIO;
            //#else
            portvalue &= ~BELL_AUDIO;
            //#endif
        #endif

        #ifdef BELL_AUDIO_2ND
        portvalue |= BELL_AUDIO_2ND;
        #endif

        Textout("Open GongFang");
    }
    else
    {
    	
        if(DoorBellIsIdle())
        {
            nTimerCloseGongfang = 0;
            RegisterBaSysCallback(0xFF, 1, BaDelayCloseGongFangTimer);
        }
    }
#endif
}
void setGongFang(BOOL bFlag)
{
    if(bFlag)
    {
        #ifdef BELL_AUDIO_V2
        gpiovalue |= BELL_AUDIO_V2;
        #endif


        #ifdef BELL_AUDIO
        portvalue &= ~BELL_AUDIO;
        #endif

        #ifdef BELL_AUDIO_2ND
        portvalue |= BELL_AUDIO_2ND;
        #endif

        Textout("Open GongFang");
    }
    else
    {
        #ifdef BELL_AUDIO_V2
       gpiovalue &= ~BELL_AUDIO_V2;
        #endif
            
        #ifdef BELL_AUDIO
        portvalue |= BELL_AUDIO;
        #endif
        
        #ifdef BELL_AUDIO_2ND
        portvalue &= ~BELL_AUDIO_2ND;
        #endif
        
        Textout("Close GongFang");

    }
}

int CheckMotoRun( void )
{
    return 0;
}

#ifdef SENSOR_8433
pthread_mutex_t          		gpiomutex = PTHREAD_MUTEX_INITIALIZER;
void GpioLock( void )
{
    pthread_mutex_lock( &gpiomutex );
}

void GpioUnLock( void )
{
    pthread_mutex_unlock( &gpiomutex );
}

void GpioStatusLed( char value )
{
    int mask;

    while ( motofd == -1 )
    {
        Textout( "Sleep, Because motofd == -1" );
        motofd = BaGpioOpen();
        sleep( 1 );
        continue;
    }

    GpioLock();
    mask = ~_MOTO_D1;
    //mask = ~_MOTO_D0;
    portvalue = portvalue & mask;

    if ( value )
    {
        portvalue |= _MOTO_D1;
        //portvalue |= _MOTO_D0;
    }

    Textout( "motofd = %d, portvalue = %08x", motofd, portvalue );
    //ioctl( gpiohandle, RALINK_GPIO_WRITE, gpiovalue );
    ioctl( motofd, 3, portvalue );
    GpioUnLock();
}

void ResetSensor()
{
    GpioStatusLed( 0 );
    sleep( 1 );
    GpioStatusLed( 1 );
    sleep( 2 );
}
#endif

void ClearI2C()
{
    portvalue &= ~0x00000006;
}


//init motot
int InitMoto( void )
{
    int iRet = 0;

    motofd = BaGpioOpen();
    if ( motofd < 0 )
    {
        printf( "Can't open /dev/gpio\n" );
        return -1;
    }


//#if defined(SUPPORT_FM34) || defined(BELINK)
#if defined(SUPPORT_FM34)

	/*Baggio Set I2C_SCLK + I2C_DAT input mode to mask RT5350 Ctrl*/
    //iRet = ioctl( motofd , RALINK_GPIO_SET_DIR, 0x0dc27800 );


		#if defined (SUPPORT_IRCUT)
		/* add begin by yiqing, 2015-10-20设置第17脚MOTO_UP为输入脚PIR检测用*/
		iRet = ioctl( motofd , RALINK_GPIO_SET_DIR, 0x0dc07806 );
		#else
		/* modify begin by yiqing, 2015-07-17, 原因: 设置25脚为输出脚，控制音频芯片复位*/
		iRet = ioctl( motofd , RALINK_GPIO_SET_DIR, 0x0dc27806 );
		#endif

#elif defined(JINQIANXIANG)
    iRet = ioctl( motofd , RALINK_GPIO_SET_DIR, 0x0fc04006 );
#else
#ifdef SENSOR_8433
    iRet = ioctl( motofd , RALINK_GPIO_SET_DIR, 0x0fc07806 );
#else
#ifdef ZILINK
	iRet = ioctl( motofd , RALINK_GPIO_SET_DIR, 0x0fc07806 );
#else
	iRet = ioctl( motofd , RALINK_GPIO_SET_DIR, 0x0fc07806 );
#endif
#endif
#endif

    iRet = ioctl( motofd, RALINK_GPIO_READ, &portvalue );

    portvalue |= 0x02000006;
    portvalue |= 0x02000006;

    #ifdef SUPPORT_FM34
    portvalue |= (1<<25);
    SetAudioPowerOpen(1);
    #else
    portvalue &= ~(1<<25);
    #endif

    
    DefaultLedStatus();    

    CheckDefKey();
    return iRet;
}

void MotoThreadStart( void )
{   
    pthread_t		threadmoto;
    pthread_t		threaddefkey;


    if ( pthread_create( &threadmoto, NULL, &MotoActionProc, NULL ) )	//ReadGPIO
    {
        printf( "network createthread failed\n" );
    }
	pthread_detach(threadmoto);


    if ( pthread_create( &threaddefkey, NULL, &DefKeyProc, NULL ) )
    {
        printf( "RESET-KEY createthread failed\n" );
    }
	pthread_detach(threaddefkey);
}



