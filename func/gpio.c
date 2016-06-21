
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>

#include "param.h"
#include "debug.h"
#include "ralink_gpio.h"

int fdGpio = -1;
int BaGpioOpen()
{
    if(fdGpio == -1)
    {
        fdGpio = open( "/dev/gpio", O_RDWR );
        if(fdGpio < 0)
        {
            Textout("Ba GPIO open fail");
            return -1;
        }
    }

    return fdGpio;
}

void DefaultLedStatus()
{
#ifdef ZILINK
    /*Close LED D2*/
    ControlIO(BELL_WPS_LED, TRUE);
#endif

}

void SytemStartupLed()
{
#if defined(ZILINK) || defined(ZHENGSHOW)
    /*Close LED D7/8*/
    ControlBellLED(CB_OPEN);
#endif

#if defined(FEEDDOG)  || defined(FUHONG) || defined(YUELAN) || defined(BELINK)
    ControlBellLED(CB_CLOSE);
#endif

}

static unsigned int autoRebootMIns = 0;
void SetAutoRbootMinsValue(unsigned char hour,unsigned char min)
{
	
	Textout("SetAutoRbootMinsValue hour:%d,min:%d",hour,min);
	int value = (hour*60 +min);
	
	if(value>=0 && value < (23*60+59))
	{
		Textout("value:%d",value);
		autoRebootMIns = (unsigned int)value;
		Textout("autoRebootMIns=%d",autoRebootMIns);
	}
}
void *AutoRebootThreadProc(void *p)
{
	struct tm* timenow;
    struct timeval time;

	unsigned int hour = 0;
	unsigned int min = 0;
	unsigned int mins = 0;

	sleep(25);

	/*
	while(1)
	{
		OnDoorOpenEx();
		OnDoor2OpenEx();
		sleep(6);
	}
	*/
	while(1)
	{	
		gettimeofday( &time, NULL );
        time.tv_sec -= bparam.stDTimeParam.byTzSel;
        timenow = localtime( &time.tv_sec );

		hour = timenow->tm_hour;
		min = timenow->tm_min;
		mins = hour *60 + min;
        //Textout("hour:%d  min:%d",hour,min);

		if(mins == autoRebootMIns)
		{
			Textout("*************************************************");
			Textout("****************[hour:%d  min:%d]****************",hour,min);
			Textout("*************************************************");
			ControlIO(AUTO_REBOOT_IO1, 1);
			ControlIO(AUTO_REBOOT_IO2, 1);
			sleep(60);
		}
		sleep(1);

	}

}

void AutoRebootInit()
{
	static pthread_t	autorebootThread = 0;

	if ( pthread_create( &autorebootThread, NULL, &AutoRebootThreadProc, NULL ) )
	{
		Textout("[autorebootThread] Create Fail...");
	}
	else
	{
		pthread_detach(autorebootThread);
	}
}




