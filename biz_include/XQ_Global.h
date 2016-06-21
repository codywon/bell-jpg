#ifndef _XQ_GLOBALDEFINE_H_
#define _XQ_GLOBALDEFINE_H_

#define WIN32_DLL 0
#define ANDROID_LIB 0
#define IOS_LIB 0
#define LINUX_LIB 1

#define BIZ_CLIENT 1
#define BIZ_SERVER 0


#include "XQ_Type.h"

#ifndef THREAD_SLEEP
	#if WIN32_DLL
		#define THREAD_SLEEP(n) Sleep(n)
	#endif
	#if ANDROID_LIB || IOS_LIB || LINUX_LIB
                    #define SLEEP_SELECT(mSec) \
                            {	\
                            struct timeval tv;\
                            tv.tv_sec=mSec/1000;\
                            tv.tv_usec=(mSec%1000)*1000;\
                            select(1,NULL,NULL,NULL,&tv);\
                            }    
		#define THREAD_SLEEP(n) SLEEP_SELECT(n)
	#endif   
#endif


#if WIN32_DLL
	
	#include <winsock2.h>
	#include "windows.h"
	#include <Ws2tcpip.h>
	#include <stdio.h>
	#pragma comment(lib,"ws2_32.lib")

	typedef    unsigned long    DWORD;
 	typedef   DWORD(WINAPI *PTHREAD_START_ROUTINE)   (LPVOID lpThreadParameter);
	typedef     PTHREAD_START_ROUTINE      LPTHREAD_START_ROUTINE;
	#define PROCCESS_CALL			 PTHREAD_START_ROUTINE  

	typedef SOCKET  SOCKETFD;
	typedef HANDLE	PROCESS_HANDLE,*PPROCESS_HANDLE;
	typedef CRITICAL_SECTION COMMO_LOCK,*PCOMMO_LOCK;
	typedef VOID COMMO_LOCK_ATTR,*PCOMMO_LOCK_ATTR;
	typedef HANDLE COMMO_SEMAPHORE,*PCOMMO_SEMAPHORE; 

	#define DEBUG2(Fmt,...) \
		{   \
			printf("= %-16s, line %4d, %-16s:"Fmt"\n", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
		}
#else
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/ip.h>

	#include <errno.h>
	#include <fcntl.h>
	#include <assert.h>
	#include <sys/param.h>
	#include <sys/ioctl.h>
	#include <stddef.h>
	#include <ctype.h>
	#include <signal.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <pthread.h>
	#include <semaphore.h>

	#define PROCCESS_CALL			void * 

	typedef INT32  SOCKETFD;
	typedef pthread_t	PROCESS_HANDLE,*PPROCESS_HANDLE;
	typedef pthread_mutex_t	COMMO_LOCK,*PCOMMO_LOCK;
	typedef pthread_mutexattr_t COMMO_LOCK_ATTR,*PCOMMO_LOCK_ATTR;
	typedef sem_t COMMO_SEMAPHORE,*PCOMMO_SEMAPHORE; 
	#if ANDROID_LIB
	#include <android/log.h>
		#define DEBUG2(Fmt, args...) \
			{   \
				__android_log_print(ANDROID_LOG_INFO, "P2PLIB","= %-16s, line %4d, %-16s:"Fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##args); \
			}
	#else
		#define DEBUG2(Fmt, args...) \
			{   \
				printf("= %-16s, line %4d, %-16s:"Fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##args); \
			}
	#endif
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)\
{\
	if((p) != NULL)\
	{\
		delete (p) ;\
		(p) = NULL ;\
	}\
}
#endif

#ifndef SAFE_DELETEA
#define SAFE_DELETEA(p)\
{\
	if((p) != NULL)\
	{\
		delete[] (p) ;\
		(p) = NULL ;\
	}\
}
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(p)\
{\
	if((p) != NULL)\
	{\
		free(p) ;\
		(p) = NULL ;\
	}\
}
#endif

//for validation

typedef struct just_validation
	{
	INT32		isLegal;
	}t_just_validation;


#ifdef __cplusplus
extern "C" {
#endif

int XQCommoLockInit(
	PCOMMO_LOCK				lpCommoLock,
	PCOMMO_LOCK_ATTR        lpCommoLockAttr //for win32 this is a unused data
);

int XQGetCommoLock(
	PCOMMO_LOCK				lpCommoLock
);

int XQTryCommoLock(
	PCOMMO_LOCK				lpCommoLock
);

int XQPutCommoLock(
	PCOMMO_LOCK				lpCommoLock
);

int XQCommoLockDestory(
	PCOMMO_LOCK				lpCommoLock
);



void XQCommoProcessInit(
	PPROCESS_HANDLE pHandle
	);

void XQCommoProcessDeInit(
	PPROCESS_HANDLE pHandle
	);

int XQCommoProcessCreate(
	PPROCESS_HANDLE pHandle,
	PROCCESS_CALL  pFun,	
	void *arg
);

int XQCommoProcessJoin(
	PPROCESS_HANDLE pHandle
);

int XQCommoSignalInit(PCOMMO_SEMAPHORE mSem);
int XQCommoSignalPost(PCOMMO_SEMAPHORE mSem);
int XQCommoSignalWait(PCOMMO_SEMAPHORE mSem);
int XQCommoSignalDestroy(PCOMMO_SEMAPHORE mSem);

#ifdef __cplusplus
}
#endif

#endif

