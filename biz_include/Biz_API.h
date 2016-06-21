/*
*/
#ifndef _LG_LOGINLIB_H__
#define  _LG_LOGINLIB_H__

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
#endif //// #ifdef WIN32DLL

#include "Biz_Data.h"
#include "Biz_CallBack.h"

#ifdef __cplusplus
extern "C"
{
#endif
#if WIN32_DLL
PPPP_API_API INT32 Biz_Init(const char *bizString,UINT32 respTimeout_ms,UINT32 connTimeout_s,PFCALLBACK Func);	
#else
PPPP_API_API INT32 Biz_Init(const char *bizString,UINT32 respTimeout_ms,UINT32 connTimeout_s);
#endif

PPPP_API_API INT32 Biz_Deinit();

#if WIN32_DLL
PPPP_API_API INT32 Biz_InitExt(const char *bizString,t_app_Inf * appInf,UINT32 respTimeout_ms,UINT32 connTimeout_s,PFCALLBACK Func);	
#else
PPPP_API_API INT32 Biz_InitExt(const char *bizString,t_app_Inf * appInf,UINT32 respTimeout_ms,UINT32 connTimeout_s);
#endif
PPPP_API_API INT32 Biz_Aret(UINT32 seqNmb,void *data,UINT32 timeoutMs);
PPPP_API_API INT32 Biz_Acmd(UINT16 cmdType,void *cmdData);
PPPP_API_API INT32 Biz_Scmd(UINT16 cmdType,void *cmdData,void *retData,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_Check(t_biz_Inf  *lgnInf);
PPPP_API_API INT32 Biz_BufferCheck(UINT32 *sendBuf,UINT32 *recvBuf,UINT32 *unfinishedCmd);

PPPP_API_API int Biz_AppLgn(t_app_Inf * appInf);
PPPP_API_API int extBiz_AppLgn(t_app_Inf * appInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_AppVerUrlGet(const char *appVer,CHAR *verUrl);
PPPP_API_API INT32 extBiz_AppVerUrlGet(const char * appVer,CHAR *verUrl,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_SvrStrGet(const char *devIDPre,char *svrStr);
PPPP_API_API INT32 extBiz_SvrStrGet(const char *idPre,char *svrStr,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_SvcListGet(t_svcInf_s*svcInf);
PPPP_API_API INT32 extBiz_SvcListGet(t_svcInf_s *svcInf,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_SvcDetailGet(UINT32 svcID,t_svcInf_d *svcDetail);
PPPP_API_API INT32 extBiz_SvcDetailGet(UINT32 svcID,t_svcInf_d *svcDetail,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_BillDetailGet(UINT32 svcID,t_billInf_d *billingDetail);
PPPP_API_API INT32 extBiz_BillDetailGet(UINT32 svcID,t_billInf_d *billingDetail,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_UsrReg(const char *usrAcc,const char *usrPwd,char *usrID);
PPPP_API_API INT32 extBiz_UsrReg(const char *usrAcc,const char *usrPwd,char *usrID,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_UsrRegExt(const char *usrAcc,const char *usrPwd,const char * uMtel,const char * uMail,char *usrID);
PPPP_API_API INT32 extBiz_UsrRegExt(const char *usrAcc,const char *usrPwd,const char * uMtel,const char * uMail,char *usrID,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_UsrActiveRequest(UINT32 actType,const char *actStr);
PPPP_API_API INT32 extBiz_UsrActiveRequest(UINT32 actType,const char *actStr,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrActiveConfirm(const char * uID,const char * uAuth);
PPPP_API_API INT32 extBiz_UsrActiveConfirm(const char * uID,const char * uAuth,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_UsrRecoveryRequest(UINT32 mType,const char * mString);
PPPP_API_API INT32 extBiz_UsrRecoveryRequest(UINT32 mType,const char * mString,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_UsrRecoveryConfirm(const char * mString,const char * mAuth,char *uPwd);
PPPP_API_API INT32 extBiz_UsrRecoveryConfirm(const char * mString,const char * mAuth,char *uPwd,UINT32 timeoutMs);

PPPP_API_API INT32 Biz_UsrLgin(const char *usrAcc,const char *usrPwd,char *usrID);
PPPP_API_API INT32 extBiz_UsrLgin(const char *usrAcc,const char *usrPwd,char *usrID,UINT32 timeoutMs);

PPPP_API_API int Biz_UsrLginExt(const char *usrStr,const char *usrPwd,char *usrID,char * usrMtel,char * usrMail);
PPPP_API_API int extBiz_UsrLginExt(const char *usrStr,const char *usrPwd,char *usrID,char * usrMtel,char * usrMail,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_OusrLgin(const char *oID,char *usrID,char *usrAcc,char *usrPwd);
PPPP_API_API INT32 extBiz_OusrLgin(const char *oID,char *usrID,char *usrAcc,char *usrPwd,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrLgout();
PPPP_API_API INT32 Biz_UsrPwdChg(const char *usrAcc,const char *usrPwd);

PPPP_API_API INT32 Biz_UsrInfGet(t_uInf_d *userInf);
PPPP_API_API INT32 extBiz_UsrInfGet(t_uInf_d *userInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrInfSet(t_uInf_d userInf);
PPPP_API_API INT32 extBiz_UsrInfSet(t_uInf_d usrInf,unsigned int timeoutMs);

PPPP_API_API t_dgInf_s *Biz_UsrDevGrpLstGet(const char *parentDevGrpID,int *nmb);
PPPP_API_API t_dgInf_s *extBiz_UsrDevGrpLstGet(const char *parentDevGrpID,int *nmb,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrDevGrpGet(const char *devGrpID, t_dgInf_d *devGrpInf);
PPPP_API_API INT32 extBiz_UsrDevGrpGet(const char *devGrpID, t_dgInf_d *devGrpInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrDevGrpAdd(t_dgInf_s devGrpInf,char *dgID);
PPPP_API_API INT32 extBiz_UsrDevGrpAdd(t_dgInf_s devGrpInf,char *dgID,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrDevGrpSet(t_dgInf_d devGrpInf);
PPPP_API_API INT32 extBiz_UsrDevGrpSet(t_dgInf_d devGrpInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrDevGrpDel(const char* devGrpID);
PPPP_API_API INT32 extBiz_UsrDevGrpDel(const char* devGrpID,unsigned int timeoutMs);

PPPP_API_API t_devInf_s *Biz_UsrDevLstGet(const char *devGrpID, int *nmb);
PPPP_API_API t_devInf_s *extBiz_UsrDevLstGet(const char *devGrpID, int *nmb,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrDevGet(const char *devID,t_udevInf_d *devInfs);
PPPP_API_API INT32 extBiz_UsrDevGet(const char *devID,t_udevInf_d *devInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrDevAdd(t_devInf_s devInf);
PPPP_API_API INT32 extBiz_UsrDevAdd(t_devInf_s devInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrDevSet(t_udevInf_d devInf);
PPPP_API_API INT32 extBiz_UsrDevSet(t_udevInf_d devInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_UsrDevDel(const char *devID);
PPPP_API_API INT32 extBiz_UsrDevDel(const char *devID,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_MsgViewGet(unsigned int *unread, unsigned int *readed, unsigned int *pushed);
PPPP_API_API INT32 extBiz_MsgViewGet(unsigned int *unread, unsigned int *readed, unsigned int *pushed,unsigned int timeoutMs);

PPPP_API_API t_msg_s *Biz_MsgListGet(char msgType,int msgTag, int msgUsedTag,unsigned int start, int *nmb);
PPPP_API_API t_msg_s *extBiz_MsgListGet(char msgType,int msgTag, int msgUsedTag,unsigned int start, int *nmb,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_MsgGet(unsigned int msgID,t_msg_d *msgInfs);
PPPP_API_API INT32 extBiz_MsgGet(unsigned int msgID,t_msg_d *msgInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_MsgSet(t_msg_d msgInf);
PPPP_API_API INT32 extBiz_MsgSet(t_msg_d msgInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_MsgTag(unsigned int msgID,unsigned int tag);
PPPP_API_API INT32 extBiz_MsgTag(unsigned int msgID,unsigned int tag,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_MsgPush(const char * fromID,char *msgTitle,const char *msgContent);
PPPP_API_API INT32 extBiz_MsgPush(const char *fromID,char *msgTitle,const char *msgContent,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_MsgPushExt(const char *fromID,char *msgTitle,const char * msgParam,const char *msgContent);
PPPP_API_API INT32 extBiz_MsgPushExt(const char *fromID,char *msgTitle,const char * msgParam,const char *msgContent,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_SlrLogin(const char *slrAcc,const char *slrPwd,char *slrID);
PPPP_API_API INT32 extBiz_SlrLogin(const char *slrAcc,const char *slrPwd,char *slrID,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_SlrLogout();
PPPP_API_API INT32 Biz_SlrPwdChg(const char *slrAcc,const char *slrPwd);
PPPP_API_API t_devInf_s *Biz_SlrDevListGet(unsigned int devStatus,  unsigned int startFrom,  int *nmb);
PPPP_API_API t_devInf_s *extBiz_SlrDevListGet(unsigned int devStatus,  unsigned int startFrom,  int *nmb,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_SlrDevGet(const char *devID,t_sdevInf_d *devInfs);
PPPP_API_API INT32 extBiz_SlrDevGet(const char *devID,t_sdevInf_d *devInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_SlrDevSet(const char *devID,t_gps devGPS,const char *devAddr);
PPPP_API_API INT32 extBiz_SlrDevSet(const char *devID,t_gps devGPS,const char *devAddr,unsigned int timeoutMs);

PPPP_API_API t_uInf_s *Biz_FellowSearch(unsigned int sType,void *sValue,unsigned int start,int *nmb);
PPPP_API_API t_uInf_s *extBiz_FellowSearch(unsigned int sType,void *sValue,unsigned int start,int *nmb,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_fellowAdd(const char * fellowStr,const char * meDesc);
PPPP_API_API INT32 extBiz_fellowAdd(const char * fellowStr,const char * meDesc,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_fellowAccept(UINT32 ifAgree,const char * fellowStr);
PPPP_API_API INT32 extBiz_fellowAccept(UINT32 ifAgree,const char * fellowStr,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_FellowGet(const char *fellowID,t_uInf_d1 *fellowInf);
PPPP_API_API INT32 extBiz_FellowGet(const char *fellowID,t_uInf_d1 *fellowInf,unsigned int timeoutMs);

PPPP_API_API INT32 Biz_FellowSet(t_udevInf_d fellowInf);
PPPP_API_API INT32 Biz_FellowDel(const char *fellowID);

PPPP_API_API INT32 Biz_SysMgm(unsigned int mgmType);
PPPP_API_API INT32 extBiz_SysMgm(unsigned int mgmType,unsigned int timeoutMs);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

