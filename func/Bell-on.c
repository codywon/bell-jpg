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

#if	1
#define	BellonTextout(Fmt, args...)
#else
#define	BellonTextout	Textout
#endif

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/7/11 */
/*static BOOL bOnDelayThread = FALSE;
BOOL GetOnDelayThread()
{
	return bOnDelayThread;
}*/

static BOOL bDoorOning = FALSE;
static int nOnDelayTime = 0;
void SetDoorOn(BOOL bOn)
{
	bDoorOning = bOn;

	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
	//bell_data.on_ok = FALSE;
	bparam.stBell.alarm_onok = FALSE;

	nOnDelayTime = 0;
}
void* OnDelayProc( void* p )
{
	//int time = 0;
	while(1)
	{
		if (!bDoorOning)
		{
			sleep(1);
			nOnDelayTime = 0;
			continue;
		}

		//time++;
		nOnDelayTime++;
		/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
		//if ( nOnDelayTime >= bell_data.on_delay_time )
		BellonTextout("Oning..., Time = %d, delay = %d", nOnDelayTime, bparam.stBell.alarm_delay);
		if ( nOnDelayTime >= bparam.stBell.alarm_delay )
		{
			/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
			/*
			if ( bell_data.on )
			{
				bell_data.on_ok = TRUE;
			}
			*/
			if ( bparam.stBell.alarm_on )
			{
				BellonTextout("Set ON OK = TRUE");
				bparam.stBell.alarm_onok = TRUE;
			}
			bDoorOning = FALSE;
			nOnDelayTime = 0;
		}

		sleep(1);
	}
}

void StartOnDelayThread()
{
	static pthread_t	threadopendelay = 0;

	/* BEGIN: Modified by wupm(2073111@qq.com), 2014/7/10 */
	//return;

	if ( threadopendelay == 0 )
	{
//		if ( 1 )
		if ( pthread_create( &threadopendelay, NULL, &OnDelayProc, NULL ) )
		{
			BellonTextout("[On Delay Proc] Create Fail...");
		}
		else
		{
			BellonTextout("[On Delay Proc] Create Succ");
			//bOnDelayThread = TRUE;
			pthread_detach(threadopendelay);
		}
	}
}

