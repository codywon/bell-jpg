/*
* copyright (C) 2013 XHX
* author: xiangqun
* date:2013.3.30
*/

/*
2014-08-14 :to support other solutions search,add:
    PPPP_InitializeExt()    :solution can specify the lower params(max session number/channel nmb/max size of packet), in additional device-name
    PPPP_SearchExt()    :st_landSearchExt
*/

/*
2014-09-13 :to support vgw
add:
    PPPP_ConnectVgw()    :use this function to connect the dev mirror/or create a dev mirror on vgw_server
change:    
    PPPP_Connect_Break()    :param-->1
    PPPP_Listen_Break()     :param-->2
*/


#ifndef _PPPP_APICOMPATIBLE_INC_H_
#define _PPPP_APICOMPATIBLE_INC_H_

#include "XQ_Global.h"

#if WIN32_DLL
	#define PPPP_API_EXPORTS 1
	#ifdef PPPP_API_EXPORTS
	#define PPPP_API_API __declspec(dllexport)
	#else
	#define PPPP_API_API __declspec(dllimport)
	#endif
#else
	#include <time.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <sys/param.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <netinet/ip.h>
	#include <netinet/udp.h>
	#include <net/if.h>
	#include <net/if_arp.h>
	#include <arpa/inet.h>
	#include <stdio.h>
	#include <errno.h>
	#include <stdlib.h>
	#include <string.h>
	#include <fcntl.h>
	#include <assert.h>
	#include <stddef.h>
	#include <ctype.h>
	#include <unistd.h>
	#include <semaphore.h>
	#include <signal.h>
	#define PPPP_API_API
#endif //// #ifdef LINUX

#ifdef _ARC_COMPILER
#include "net_api.h"
#endif 

#include "XQPPP_Define.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    PPPP_API_API UINT32 PPPP_GetAPIVersion();
    PPPP_API_API INT32 PPPP_QueryDID( const CHAR* DeviceName, CHAR* DID, INT32 DIDBufSize );
    PPPP_API_API INT32 PPPP_Initialize( char* server );
    PPPP_API_API INT32 PPPP_InitializeExt(char* LicenseString,UINT32 MaxNmbOfSession,UINT32 MaxNmbOfChannel,UINT32 MaxSizeOfChannel,UINT32 MaxSizeOfPacket,char *NodeName);
    PPPP_API_API INT32 PPPP_DeInitialize( void );
    PPPP_API_API INT32 PPPP_NetworkDetect(st_PPPP_NetInfo *NetInfo, UINT16 UDP_Port );
    PPPP_API_API INT32 PPPP_Share_Bandwidth( CHAR bOnOff );
    PPPP_API_API INT32 PPPP_Listen( const CHAR* MyID, const UINT32 TimeOut_Sec, UINT16 UDP_Port, CHAR bEnableInternet );
    PPPP_API_API INT32 PPPP_Listen_Break( void );
    PPPP_API_API INT32 PPPP_LoginStatus_Check( CHAR* bLoginStatus );
    PPPP_API_API INT32 PPPP_Connect( const CHAR* TargetID, CHAR bEnableLanSearch, UINT16 UDP_Port );    
    PPPP_API_API INT32 PPPP_ConnectVgw( const CHAR* TargetID, CHAR bEnableLanSearch, UINT16 UDP_Port );
    PPPP_API_API INT32 PPPP_ConnectByServer(const char *TargetID, char bEnableLanSearch,  UINT16 UDP_Port, char *ServerString);
    PPPP_API_API INT32 PPPP_Connect_Break();
    PPPP_API_API INT32 PPPP_Check( INT32 SessionHandle, st_PPPP_Session* SInfo );
    PPPP_API_API INT32 PPPP_Close( INT32 SessionHandle );
    PPPP_API_API INT32 PPPP_ForceClose( INT32 SessionHandle );
    PPPP_API_API INT32 PPPP_Write( INT32 SessionHandle, UCHAR Channel, CHAR* DataBuf, INT32 DataSizeToWrite );
    PPPP_API_API INT32 PPPP_Read( INT32 SessionHandle, UCHAR Channel, CHAR* DataBuf, INT32* DataSize, UINT32 TimeOut_ms );
    PPPP_API_API INT32 PPPP_Check_Buffer( INT32 SessionHandle, UCHAR Channel, UINT32* WriteSize, UINT32* ReadSize );
    PPPP_API_API INT32 PPPP_Search( UINT32 TimeOut,t_lanSearchRet *SearchResult);
    PPPP_API_API INT32 PPPP_SearchExt(st_lanSearchExtRet * SearchExtResult, UINT32 MaxNmbOfNode,UINT32 TimeOut_Ms);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif ////#ifndef _PPPP_API___INC_H

