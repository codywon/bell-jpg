#ifndef _PPCS_API___INC_H_
#define _PPCS_API___INC_H_

#ifdef WIN32DLL
#ifdef PPPP_API_EXPORTS
#define PPPP_API_API __declspec(dllexport)
#else
#define PPPP_API_API __declspec(dllimport)
#endif
#endif //// #ifdef WIN32DLL

#ifdef LINUX
#include <netinet/in.h>
#define PPPP_API_API 
#endif //// #ifdef LINUX

#ifdef _ARC_COMPILER
#include "net_api.h"
#define PPPP_API_API 
#endif //// #ifdef _ARC_COMPILER

#include "PPCS_Type.h"
#include "PPCS_Error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef struct{	
	CHAR bFlagInternet;		// Internet Reachable? 1: YES, 0: NO
	CHAR bFlagHostResolved;	// P2P Server IP resolved? 1: YES, 0: NO
	CHAR bFlagServerHello;	// P2P Server Hello? 1: YES, 0: NO
	CHAR NAT_Type;			// NAT type, 0: Unknow, 1: IP-Restricted Cone type,   2: Port-Restricted Cone type, 3: Symmetric 
	CHAR MyLanIP[16];		// My LAN IP. If (bFlagInternet==0) || (bFlagHostResolved==0) || (bFlagServerHello==0), MyLanIP will be "0.0.0.0"
	CHAR MyWanIP[16];		// My Wan IP. If (bFlagInternet==0) || (bFlagHostResolved==0) || (bFlagServerHello==0), MyWanIP will be "0.0.0.0"
} st_PPCS_NetInfo;
#define st_PPPP_NetInfo st_PPCS_NetInfo

typedef struct{	
	INT32  Skt;					// Sockfd
	struct sockaddr_in RemoteAddr;	// Remote IP:Port
	struct sockaddr_in MyLocalAddr;	// My Local IP:Port
	struct sockaddr_in MyWanAddr;	// My Wan IP:Port
	UINT32 ConnectTime;				// Connection build in ? Sec Before
	CHAR DID[24];					// Device ID
	CHAR bCorD;						// I am Client or Device, 0: Client, 1: Device
	CHAR bMode;						// Connection Mode: 0: P2P, 1:Relay Mode
	CHAR Reserved[2];				
} st_PPCS_Session;
#define st_PPPP_Session st_PPCS_Session

PPPP_API_API UINT32 PPCS_GetAPIVersion(void);
#define PPPP_GetAPIVersion PPCS_GetAPIVersion
PPPP_API_API INT32 PPCS_QueryDID(const CHAR* DeviceName, CHAR* DID, INT32 DIDBufSize);
#define PPPP_QueryDID PPCS_QueryDID
PPPP_API_API INT32 PPCS_Initialize(CHAR *Parameter);
#define PPPP_Initialize PPCS_Initialize
PPPP_API_API INT32 PPCS_DeInitialize(void);
#define PPPP_DeInitialize PPCS_DeInitialize
PPPP_API_API INT32 PPCS_NetworkDetect(st_PPCS_NetInfo *NetInfo, UINT16 UDP_Port);
#define PPPP_NetworkDetect PPCS_NetworkDetect
PPPP_API_API INT32 PPCS_NetworkDetectByServer(st_PPCS_NetInfo *NetInfo, UINT16 UDP_Port, CHAR *ServerString);
#define PPPP_NetworkDetectByServer PPCS_NetworkDetectByServer
PPPP_API_API INT32 PPCS_Share_Bandwidth(CHAR bOnOff);
#define PPPP_Share_Bandwidth PPCS_Share_Bandwidth
PPPP_API_API INT32 PPCS_Listen(const CHAR *MyID, const UINT32 TimeOut_Sec, UINT16 UDP_Port, CHAR bEnableInternet, const CHAR* APILicense);
#define PPPP_Listen PPCS_Listen
PPPP_API_API INT32 PPCS_Listen_Break(void);
#define PPPP_Listen_Break PPCS_Listen_Break
PPPP_API_API INT32 PPCS_LoginStatus_Check(CHAR* bLoginStatus);
#define PPPP_LoginStatus_Check PPCS_LoginStatus_Check
PPPP_API_API INT32 PPCS_Connect(const CHAR *TargetID, CHAR bEnableLanSearch, UINT16 UDP_Port);
#define PPPP_Connect PPCS_Connect
PPPP_API_API INT32 PPCS_ConnectByServer(const CHAR *TargetID, CHAR bEnableLanSearch, UINT16 UDP_Port, CHAR *ServerString);
#define PPPP_ConnectByServer PPCS_ConnectByServer
PPPP_API_API INT32 PPCS_Connect_Break();
#define PPPP_Connect_Break PPCS_Connect_Break
PPPP_API_API INT32 PPCS_Check(INT32 SessionHandle, st_PPCS_Session *SInfo);
#define PPPP_Check PPCS_Check
PPPP_API_API INT32 PPCS_Close(INT32 SessionHandle);
#define PPPP_Close PPCS_Close
PPPP_API_API INT32 PPCS_ForceClose(INT32 SessionHandle);
#define PPPP_ForceClose PPCS_ForceClose
PPPP_API_API INT32 PPCS_Write(INT32 SessionHandle, UCHAR Channel, CHAR *DataBuf, INT32 DataSizeToWrite);
#define PPPP_Write PPCS_Write
PPPP_API_API INT32 PPCS_Read(INT32 SessionHandle, UCHAR Channel, CHAR *DataBuf, INT32 *DataSize, UINT32 TimeOut_ms);
#define PPPP_Read PPCS_Read
/*Check write buffer how much data is not send out*/
PPPP_API_API INT32 PPCS_Check_Buffer(INT32 SessionHandle, UCHAR Channel, UINT32 *WriteSize, UINT32 *ReadSize);
#define PPPP_Check_Buffer PPCS_Check_Buffer
#ifdef __cplusplus
}
#endif // __cplusplus
#endif ////#ifndef _PPCS_API___INC_H_

