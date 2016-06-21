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


static int nKeyState = FALSE;
static int nKeyTime = 0;
int GetKeyState	()
{
	return nKeyState;
}
void* ThreadKeyPressProc( void* p )
{
	//int time = 0;
	while(1)
	{
		if ( nKeyState == TRUE )
		{
			nKeyTime ++;
			if ( nKeyTime >= 60 )
			{
				nKeyTime = 0;
				nKeyState = FALSE;
			}
		}
		sleep(1);
	}
}
void SetKeyState(int keyState)
{
	static pthread_t threadKeyPress = 0;
	if ( nKeyState == 1 )
	{
		if ( threadKeyPress == 0 )
		{
//			if ( 1 )
			if ( pthread_create( &threadKeyPress, NULL, &ThreadKeyPressProc, NULL ) )
			{
				Textout("[Thread ThreadKeyPressProc Proc] Create Fail...");
			}
			else
			{
				Textout("[Thread ThreadKeyPressProc Proc] Create Succ");
				pthread_detach(threadKeyPress);
			}
		}
	}
	nKeyTime = 0;
	nKeyState = keyState;
}

