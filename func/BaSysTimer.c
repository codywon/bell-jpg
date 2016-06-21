
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>

#include "debug.h"
#include "BaSysTimer.h"

stBaSysTimer gstBaSysTimer[MAX_BA_SYS_TIMER_COUNT];

void RegisterBaSysCallback(unsigned int sit, unsigned int interval, BaSysTimerCallback callback)
{
	//Textout("RegisterBaSysCallback");
    int i=0;
    for(i=0; i<MAX_BA_SYS_TIMER_COUNT; i++)
    {
        if(gstBaSysTimer[i].callback == NULL)
        {
            break;
        }
    }

    if(interval == 0) 
		interval = MAX_BA_SYS_TIMER_SECOND;

    if(i < MAX_BA_SYS_TIMER_COUNT)
    {
        gstBaSysTimer[i].nSit = sit;
        gstBaSysTimer[i].nTimer = 0;
        gstBaSysTimer[i].nInterval = interval;
        gstBaSysTimer[i].callback = callback;
    }
}

void UnRegisterBaSysCallback(unsigned int sit, BaSysTimerCallback callback)
{
	//Textout("UnRegisterBaSysCallback");
    int i=0;
    for(i=0; i<MAX_BA_SYS_TIMER_COUNT; i++)
    {
        if(gstBaSysTimer[i].callback == callback && gstBaSysTimer[i].nSit == sit)
        {
            break;
        }
    }

    if(i < MAX_BA_SYS_TIMER_COUNT)
    {
        gstBaSysTimer[i].nSit = 0xFF;
        gstBaSysTimer[i].nTimer = 0;
        gstBaSysTimer[i].nInterval = 0;
        gstBaSysTimer[i].callback = NULL;
    }
}


static void BaSysTimer(int sig)
{
    int i=0;
	//Textout("------------------BaSysTimer-----------------");
    for(i=0; i<MAX_BA_SYS_TIMER_COUNT; i++)
    {
        if(gstBaSysTimer[i].callback != NULL)
        {
			//Textout("-----------------gstBaSysTimer[i]:%d-------------------",i);
           gstBaSysTimer[i].nTimer++;
           if(gstBaSysTimer[i].nTimer >= gstBaSysTimer[i].nInterval)
           {
                gstBaSysTimer[i].nTimer = 0;
                gstBaSysTimer[i].callback(gstBaSysTimer[i].nSit);
           }
        }
    }
}

void *SysTimerProc(void * p)
{
	while(1)
	{
		BaSysTimer(0);
		sleep(1);
	}
	
}
void BaSysTimerInit( void )
{
	//Textout("--------------BaSysTimerInit----------------------");

	#if 0
    struct itimerval        value;
    int                     ret;
    
    value.it_value.tv_sec = MAX_BA_SYS_TIMER_SECOND;
    value.it_value.tv_usec = MAX_BA_SYS_TIMER_USECOND;
    value.it_interval.tv_sec = value.it_value.tv_sec;
    value.it_interval.tv_usec = value.it_value.tv_usec;

    memset(gstBaSysTimer, 0, sizeof(stBaSysTimer)*MAX_BA_SYS_TIMER_COUNT);
    
    signal( SIGALRM, BaSysTimer );

    ret = setitimer( ITIMER_REAL, &value, NULL );
    if ( ret < 0 )
    {
        Textout( "setitimer ITIMER_REAL fail" );
        return;
    }

	#else
	pthread_t thread_id;
    
    pthread_create (&thread_id, NULL, &SysTimerProc, NULL);
	#endif
}


