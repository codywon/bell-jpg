//实现获取快照的回调函数
#ifndef _BIZ_CB_H_
#define _BIZ_CB_H_

#include "XQ_Global.h"

typedef enum
	{
	CBNOTIFY_SESSION_STATUS=0,
	CBNOTIFY_SYSTEM_MSG
	}st_cbnotify_type;

typedef struct
	{
	UINT32 nmb;
	t_sys_notify *sysMsg;
	}st_cbSystemMsg;


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if WIN32_DLL
typedef int (WINAPI *PFCALLBACK)(int nType,void *param);
int WINAPI callbackNotify(int nType,void *param); 
#else
void callbackNotify(int nType,void *param);
#endif

#ifdef __cplusplus
		}
#endif // __cplusplus

#endif

