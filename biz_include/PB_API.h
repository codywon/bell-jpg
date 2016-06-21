
#ifndef _BPPP_API_H_
#define _BPPP_API_H_

#include "XQ_Global.h"

#if WIN32_DLL
#define PPPP_API_EXPORTS 1
#ifdef PPPP_API_EXPORTS
#define PPPP_API_API __declspec(dllexport)
#else
#define PPPP_API_API __declspec(dllimport)
#endif

#else
#define PPPP_API_API
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h> 
#include <semaphore.h>
#include <signal.h>
#endif

#include "XQ_Type.h"
#include "PB_Error.h"
#include "P2P_Data.h"
#include "Biz_Data.h"

//initialise parameters
typedef struct InitParam
{
    char *bizString;//bizserver string, to provide message push, user self-service
    char *p2pString;//p2pserver string, to provide p2p communication
    t_app_Inf 	bizAppInf; //biz-communication infor
    st_globalParams p2pParams;//p2p-session params, like max session, and so on
}st_InitParam;

#ifdef __cplusplus
extern "C"
{
#endif

#if WIN32_DLL
PPPP_API_API INT32 BPPP_Init(st_InitParam *initParams,UINT32 respTimeout_ms,UINT32 connTimeout_s,PFCALLBACK Func);	
#else
PPPP_API_API INT32 BPPP_Init(st_InitParam *initParams,UINT32 respTimeout_ms,UINT32 connTimeout_s);
#endif
PPPP_API_API INT32 BPPP_Deinit();

//new bizAPI---start
PPPP_API_API INT32 Biz_Aret(UINT32 seqNmb,void *data,UINT32 timeoutMs);
PPPP_API_API INT32 Biz_Acmd(UINT16 cmdType,void *cmdData);
PPPP_API_API INT32 Biz_Scmd(UINT16 cmdType,void *cmdData,void *retData,UINT32 timeoutMs);
PPPP_API_API INT32 Biz_Check(t_biz_Inf  *lgnInf);
PPPP_API_API INT32 Biz_BufferCheck(UINT32 *sendBuf,UINT32 *recvBuf,UINT32 *unfinishedCmd);
//new bizAPI---end



//old bizAPI---start
#if WIN32_DLL
PPPP_API_API INT32 Biz_Init(const char *bizString,UINT32 respTimeout_ms,UINT32 connTimeout_s,PFCALLBACK Func);	
PPPP_API_API INT32 Biz_InitExt(const char *bizString,t_app_Inf * appInf,UINT32 respTimeout_ms,UINT32 connTimeout_s,PFCALLBACK Func);	
#else
PPPP_API_API INT32 Biz_Init(const char *bizString,UINT32 respTimeout_ms,UINT32 connTimeout_s);
PPPP_API_API INT32 Biz_InitExt(const char *bizString,t_app_Inf * appInf,UINT32 respTimeout_ms,UINT32 connTimeout_s);
#endif
PPPP_API_API INT32 Biz_Deinit();

PPPP_API_API INT32 Biz_AppLgn(t_app_Inf * appInf);
PPPP_API_API INT32 Biz_AppVerUrlGet(const char *appVer,CHAR *verUrl);
PPPP_API_API INT32 Biz_SvrStrGet(const char *devIDPre,char *svrStr);
PPPP_API_API INT32 Biz_SvcListGet(t_svcInf_s*svcInf);
PPPP_API_API INT32 Biz_SvcDetailGet(UINT32 svcID,t_svcInf_d *svcDetail);
PPPP_API_API INT32 Biz_BillDetailGet(UINT32 svcID,t_billInf_d *billingDetail);
PPPP_API_API INT32 Biz_UsrReg(const char *usrAcc,const char *usrPwd,char *usrID);
PPPP_API_API INT32 Biz_UsrRegExt(const char *usrAcc,const char *usrPwd,const char * uMtel,const char * uMail,char *usrID);
PPPP_API_API INT32 Biz_UsrActiveRequest(UINT32 actType,const char *actStr);
PPPP_API_API INT32 Biz_UsrActiveConfirm(const char * uID,const char * uAuth);
PPPP_API_API INT32 Biz_UsrRecoveryRequest(UINT32 mType,const char * mString);
PPPP_API_API INT32 Biz_UsrRecoveryConfirm(const char * mString,const char * mAuth,char *uPwd);
PPPP_API_API INT32 Biz_UsrLgin(const char *usrAcc,const char *usrPwd,char *usrID);
PPPP_API_API INT32 Biz_UsrLginExt(const char *usrStr,const char *usrPwd,char *usrID,char * usrMtel,char * usrMail);
PPPP_API_API INT32 Biz_OusrLgin(const char *oID,char *usrID,char *usrAcc,char *usrPwd);
PPPP_API_API INT32 Biz_UsrLgout();
PPPP_API_API INT32 Biz_UsrInfGet(t_uInf_d *userInf);
PPPP_API_API INT32 Biz_UsrInfSet(t_uInf_d userInf);
PPPP_API_API INT32 Biz_UsrPwdChg(const char *usrAcc,const char *usrPwd);
PPPP_API_API t_dgInf_s *Biz_UsrDevGrpLstGet(const char *parentDevGrpID,int *nmb);
PPPP_API_API INT32 Biz_UsrDevGrpGet(const char *devGrpID, t_dgInf_d *devGrpInf);
PPPP_API_API INT32 Biz_UsrDevGrpAdd(t_dgInf_s devGrpInf,char *dgID);
PPPP_API_API INT32 Biz_UsrDevGrpSet(t_dgInf_d devGrpInf);
PPPP_API_API INT32 Biz_UsrDevGrpDel(const char* devGrpID);
PPPP_API_API t_devInf_s *Biz_UsrDevLstGet(const char *devGrpID, int *nmb);
PPPP_API_API INT32 Biz_UsrDevGet(const char *devID,t_udevInf_d *devInfs);
PPPP_API_API INT32 Biz_UsrDevAdd(t_devInf_s devInf);
PPPP_API_API INT32 Biz_UsrDevSet(t_udevInf_d devInf);
PPPP_API_API INT32 Biz_UsrDevDel(const char *devID);
PPPP_API_API INT32 Biz_MsgViewGet(unsigned int *unread, unsigned int *readed, unsigned int *pushed);
PPPP_API_API t_msg_s *Biz_MsgListGet(char msgType,int msgTag, int msgUsedTag,unsigned int start, int *nmb);
PPPP_API_API INT32 Biz_MsgGet(unsigned int msgID,t_msg_d *msgInfs);
PPPP_API_API INT32 Biz_MsgSet(t_msg_d msgInf);
PPPP_API_API INT32 Biz_MsgTag(unsigned int msgID,unsigned int tag);
PPPP_API_API INT32 Biz_MsgPush(const char * fromID,char *msgTitle,const char *msgContent);
PPPP_API_API INT32 Biz_MsgPushExt(const char *fromID,char *msgTitle,const char * msgParam,const char *msgContent);

PPPP_API_API INT32 Biz_SlrLogin(const char *slrAcc,const char *slrPwd,char *slrID);
PPPP_API_API INT32 Biz_SlrLogout();
PPPP_API_API t_devInf_s *Biz_SlrDevListGet(unsigned int devStatus,  unsigned int startFrom,  int *nmb);
PPPP_API_API INT32 Biz_SlrDevGet(const char *devID,t_sdevInf_d *devInfs);
PPPP_API_API INT32 Biz_SlrDevSet(const char *devID,t_gps devGPS,const char *devAddr);

PPPP_API_API t_uInf_s *Biz_FellowSearch(unsigned int sType,void *sValue,unsigned int start,int *nmb);
PPPP_API_API INT32 Biz_fellowAdd(const char * fellowStr,const char * meDesc);
PPPP_API_API INT32 Biz_fellowAccept(UINT32 ifAgree,const char * fellowStr);
PPPP_API_API INT32 Biz_FellowGet(const char *fellowID,t_uInf_d1 *fellowInf);
PPPP_API_API INT32 Biz_FellowSet(t_udevInf_d fellowInf);
PPPP_API_API INT32 Biz_FellowDel(const char *fellowID);

PPPP_API_API INT32 Biz_SysMgm(unsigned int mgmType);
//old bizAPI---end

PPPP_API_API UINT32 PPPP_GetAPIVersion();
PPPP_API_API INT32 PPPP_QueryDID( const CHAR* DeviceName, CHAR* DID, INT32 DIDBufSize );
PPPP_API_API INT32 PPPP_Initialize( char* server );
PPPP_API_API INT32 PPPP_InitializeExt(char* LicenseString,UINT32 MaxNmbOfSession,UINT32 MaxNmbOfChannel,UINT32 MaxSizeOfChannel,UINT32 MaxSizeOfPacket,char *NodeName);
PPPP_API_API INT32 PPPP_DeInitialize( void );

PPPP_API_API INT32 PPPP_QueryDID( const CHAR* DeviceName, CHAR* DID, INT32 DIDBufSize );
PPPP_API_API INT32 PPPP_NetworkDetect(st_PPPP_NetInfo* NetInfo, UINT16 UDP_Port );
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

#endif

