/***/
#ifndef _LG_LOGINLIB_H__
#define  _LG_LOGINLIB_H__

#define READ_TIMEOUT 10000

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
//#include <iostream>                      
//#include <memory>
//#include <string>
//#include <map>
//#include <vector>

//#include "opr_type.h"
#include "biz_type.h"
#include "libs3.h"

//using namespace std;
#define MAX_ENGINE_NMB  10

//common define
#define CMD_ENC_STEP    4
#define CMD_MAX_MSG_SIZE   1400
#define CMD_MAX_PKT_SIZE	1024
#define MAX_MSGTITLE_SIZE   64
#define MAX_MSGCONTENT_SIZE 1000
#define MAX_NAME_SIZE    16
#define MAX_ACC_SIZE    16
#define MAX_PWD_SIZE    16
#define MAX_MSG_CNT_SIZE    256
#define MAX_MSG_RTN_NMB    4
#define MAX_DEV_RTN_NMB    16
#define MAX_SVC_RTN_NMB     32
#define MAX_HB_COUNTER  5
#define SESSION_IOCTL_TIMER 30 //s  

#define MAX_CNN_NUM     100
#define MAX_THD_NUM     100
#define MIN_THD_NUM     30

#define DBUSER  "root"
#define DBHOST "127.0.0.1"
#define DBPWD "hhgrhhgr"
#define DBNAME "biz"


#define MAX_ID_SIZE    20
#define MAX_GID_SIZE    24
#define SESSION_STR_SIZE    32
#define MSG_HEADER_LEN  8
#define MAX_APPLETOKEN_SIZE 64
#define MAX_VERSION_SIZE 12


//err define
#define ERR_BIZ_SUCCESSFUL					 0
#define ERR_BIZ_UNSUCCESSFUL                                           -31
#define ERR_BIZ_NOT_INITIALIZED				-32
#define ERR_BIZ_ALREADY_INITIALIZED			-33
#define ERR_BIZ_TIME_OUT						-34
#define ERR_BIZ_INVALID_PARAMETER			-35
#define ERR_BIZ_FAIL_TO_RESOLVE_NAME		-36
#define ERR_BIZ_INSUFFICIENT_RESOURC		           -37
#define ERR_BIZ_TOO_FAST_INPUT			           -38
#define ERR_BIZ_PORT_BIND_FAILED		                    -39
#define ERR_BIZ_NOT_LOGIN                                                    -40
#define ERR_BIZ_ACCOUNT_ALREADY_EXIST		-41
#define ERR_BIZ_INVALID_USER					-42
#define ERR_BIZ_NO_RESULT					-43
#define ERR_BIZ_INVALID_VERSION                                     -44
#define ERR_BIZ_INVALID_SERVICE                                      -45
#define ERR_BIZ_INVALID_BILLING                                       -46
#define ERR_BIZ_ALREADY_LOGOUT				-47
#define ERR_BIZ_INCORECT_PACKET				-48
#define ERR_BIZ_SVR_OPERATION_FAILED                     -49
#define ERR_BIZ_ID_EXCEED_LIMITATION                        -50
#define ERR_BIZ_INVALID_IDPRE                                           -51
#define ERR_BIZ_INVALID_BIZSTR                                           -52


typedef struct app_Inf
    {
    CHAR appVer[MAX_VERSION_SIZE];
    CHAR appID[MAX_ID_SIZE];
    CHAR appleToken[MAX_APPLETOKEN_SIZE];
    }t_app_Inf;

typedef struct biz_Inf
{
UINT32 usrGrp;
UINT32 seqNmb;
UINT32 heartbeatCounter;
CHAR appVer[MAX_VERSION_SIZE];
CHAR sessionStr[SESSION_STR_SIZE];
CHAR lginID[MAX_ID_SIZE];
CHAR appID[MAX_ID_SIZE];
struct sockaddr_in *svrAddr;
}t_biz_Inf;

typedef struct gps
    {
    long gps_x;
    long gps_y;
    }t_gps;

//simple service infomation
typedef struct svcInf_s
    {
    UINT32  svcID;
    UINT32 billingPrice;
    CHAR svcName[MAX_ACC_SIZE];
    CHAR billingName[MAX_ACC_SIZE];
    }t_svcInf_s;

//detail service infomation
typedef struct svcInf_d
    {
    UINT32 svcID;
    CHAR svcName[MAX_ACC_SIZE];
    CHAR svcDesc[MAX_MSG_CNT_SIZE];
    }t_svcInf_d;

//detail billing infomation
typedef struct billInf_d
    {
    UINT16 billingID;
    UINT16 billingMoney;
    CHAR billingName[MAX_ACC_SIZE];
    CHAR billingDesc[MAX_MSG_CNT_SIZE];
    }t_billInf_d;

//simple device group infomation
typedef struct 
    {
    CHAR dGID[MAX_GID_SIZE];//device group ID
    CHAR pdGID[MAX_GID_SIZE];//the parent device groupp ID  
    CHAR dGName[MAX_NAME_SIZE];//device group anme
    }t_dgInf_s;

//detail device group infomation
typedef struct 
    {
    CHAR dGID[MAX_GID_SIZE];//device group ID
    CHAR pdGID[MAX_GID_SIZE];//the parent device groupp ID  
    CHAR dGName[MAX_NAME_SIZE];//device group anme
    UINT32 dGEnabled;//if enabled
    CHAR dGDesc[MAX_MSG_CNT_SIZE];//describtion of the device group
    }t_dgInf_d;

//simple infomation of device
typedef struct 
    {
    CHAR dID[MAX_ID_SIZE];//device ID
    CHAR dGID[MAX_GID_SIZE];//device group id the device belong to
    CHAR dName[MAX_NAME_SIZE];//device name
    CHAR dAcc[MAX_ACC_SIZE]; //access account
    CHAR dPwd[MAX_PWD_SIZE];//access password
    }t_devInf_s;

//detail infomation for user-device
typedef struct 
    {
    CHAR dID[MAX_ID_SIZE];//device ID
    CHAR dGID[MAX_GID_SIZE];//device group id the device belong to
    CHAR dName[MAX_NAME_SIZE];//device name
    CHAR dAcc[MAX_ACC_SIZE]; //access account
    CHAR dPwd[MAX_PWD_SIZE];//access password
    UINT32 dSvcMap;//the bitmap for servcie 
    UINT32 dEnable;//if enabled 0'enabled,1->disenabled
    UINT32 dTag; //owner or just accessor
    CHAR dDesc[MAX_MSG_CNT_SIZE];//desribe infomation
    }t_udevInf_d;

//detail infomation for saler-device
typedef struct 
    {
    CHAR dID[MAX_ID_SIZE];
    CHAR dAcc[MAX_ACC_SIZE]; 
    CHAR dPwd[MAX_PWD_SIZE];
    UINT32 dStatus;//设备状态，用于标识设备生命周期(供、铺、派、售、修)
    UINT32 dEnable;//启用否
    UINT32 dSPrice;//实际销售价        
    LONG dGpsX;//GPS
    LONG dGpsY;//GPS
    CHAR dAddr[MAX_MSG_CNT_SIZE];//设备地址文字描述信息
    }t_sdevInf_d;

//simple infomation for user
typedef struct 
    {
    CHAR uID[MAX_ID_SIZE];//user id
    CHAR uName[MAX_NAME_SIZE] ;//user name /alias
    }t_uInf_s;

//detail infomation for user
typedef struct 
    {
    CHAR uID[MAX_ID_SIZE];//userID
    CHAR uOID[SESSION_STR_SIZE];//thirdth authentication (maybe null)
    CHAR uName[MAX_NAME_SIZE]; //user name /alias
    CHAR uAcc[MAX_ACC_SIZE]; //acount
    CHAR uPwd[MAX_PWD_SIZE];//password (when search return NULL)
    UINT32 uRegTime;//regist time
    UINT32 uLastLgnTime;//regist time
    UINT32 uAge;//age
    UINT32 uSex;//sex
    UINT32 uEnable;  
    LONG uGpsX;//GPS
    LONG uGpsY;//GPS    
    CHAR uAddr[MAX_MSG_CNT_SIZE];//address
    }t_uInf_d;

//detail infomation for fellow
typedef struct 
    {
    CHAR uID[MAX_ID_SIZE];//userID
    CHAR uName[MAX_NAME_SIZE]; //user name /alias
    UINT32 uRegTime;//regist time
    UINT32 uLastLgnTime;//regist time
    UINT32 uAge;//age
    UINT32 uSex;//sex
    UINT32 uEnable;  
    LONG uGpsX;//GPS
    LONG uGpsY;//GPS    
    CHAR uAddr[MAX_MSG_CNT_SIZE];//address
    }t_uInf_d1;

//simple infomation for msg
typedef struct msg_s
    {
    UINT32 msgID;//msg id
    UINT32 pmsgID;//msg id
    CHAR msgTitle[MAX_MSGTITLE_SIZE];
    CHAR msgFrom[MAX_ID_SIZE];//from who    
    UINT32 msgTag;//msg type
    UINT32 msgCTime;//create time    
    }t_msg_s;

//detail infomation for msg
typedef struct msg_d
    {
    UINT32 msgID;//msg id
    UINT32 pmsgID;//msg id
    CHAR msgTitle[MAX_MSGTITLE_SIZE];
    CHAR msgFrom[MAX_ID_SIZE];//from who    
    UINT32 msgTag;//msg type
    UINT32 msgCTime;//create time    
    CHAR msgTo[MAX_MSG_CNT_SIZE];//to whos,split with ';'
    CHAR msgContent[MAX_MSGCONTENT_SIZE];//content
    }t_msg_d;

//safe delete define
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


#ifdef __cplusplus
extern "C"
{
#endif
INT32 Biz_Init(const char *bizString);
INT32 Biz_Deinit();
INT32 Biz_AppLgn(t_app_Inf * appInf);
INT32 Biz_AppVerUrlGet(const char *appVer,CHAR *verUrl);
INT32 Biz_SvrStrGet(const char *devIDPre,char *svrStr);
INT32 Biz_SvcListGet(t_svcInf_s*svcInf);
INT32 Biz_SvcDetailGet(UINT32 svcID,t_svcInf_d *svcDetail);
INT32 Biz_BillDetailGet(UINT32 svcID,t_billInf_d *billingDetail);

INT32 Biz_UsrReg(const char *usrAcc,const char *usrPwd,char *usrID);
INT32 Biz_UsrLgin(const char *usrAcc,const char *usrPwd,char *usrID);
INT32 Biz_OusrLgin(const char *oID,char *usrID,char *usrAcc,char *usrPwd);
INT32 Biz_UsrLgout();
INT32 Biz_UsrPwdChg(const char *usrAcc,const char *usrPwd);
INT32 Biz_UsrInfGet(t_uInf_d *userInf);
INT32 Biz_UsrInfSet(t_uInf_d userInf);

t_dgInf_s *Biz_UsrDevGrpLstGet(const char *parentDevGrpID,int *nmb);
INT32 Biz_UsrDevGrpGet(const char *devGrpID, t_dgInf_d *devGrpInf);
INT32 Biz_UsrDevGrpAdd(t_dgInf_s devGrpInf,char *dgID);
INT32 Biz_UsrDevGrpSet(t_dgInf_d devGrpInf);
INT32 Biz_UsrDevGrpDel(const char* devGrpID);

 t_devInf_s *Biz_UsrDevLstGet(const char *devGrpID, int *nmb);
INT32 Biz_UsrDevGet(const char *devID,t_udevInf_d *devInfs);
INT32 Biz_UsrDevAdd(t_devInf_s devInf);
INT32 Biz_UsrDevSet(t_udevInf_d devInf);
INT32 Biz_UsrDevDel(const char *devID);

INT32 Biz_MsgViewGet(UINT32 *unread, UINT32 *readed, UINT32 *pushed);
t_msg_s *Biz_MsgListGet(char msgType,INT32 msgTag, INT32 msgUsedTag,UINT32 start, INT32 *nmb);
INT32 Biz_MsgGet(UINT32 msgID,t_msg_d *msgInfs);
INT32 Biz_MsgSet(t_msg_d msgInf);
INT32 Biz_MsgTag(UINT32 msgID,UINT32 tag);
INT32 Biz_MsgPush(const char * fromID,char *msgTitle,const char *msgContent);

INT32 Biz_SlrLogin(const char *slrAcc,const char *slrPwd,char *slrID);
INT32 Biz_SlrLogout();
INT32 Biz_SlrPwdChg(const char *slrAcc,const char *slrPwd);
t_devInf_s *Biz_SlrDevListGet(UINT32 devStatus,  UINT32 startFrom,  INT32 *nmb);
INT32 Biz_SlrDevGet(const char *devID,t_sdevInf_d *devInfs);
INT32 Biz_SlrDevSet(const char *devID,t_gps devGPS,const char *devAddr);

t_uInf_s *Biz_FellowSearch(UINT32 sType,void *sValue,UINT32 start,INT32 *nmb);
INT32 Biz_FellowGet(const char *fellowID,t_uInf_d1 *fellowInf);
INT32 Biz_FellowSet(t_udevInf_d fellowInf);
INT32 Biz_FellowDel(const char *fellowID);

INT32 Biz_GetLgnInf(t_biz_Inf  *lgnInf);
INT32 Biz_RmtShutdown();

typedef struct cs_bkt_inf
    {
    char fileName[256];
    time_t  createTime;
    uint64_t    size;
    }t_cs_bkt_inf;

INT32 Biz_CS_List_Get(const char *prefixStr, const char *filterStr,int maxNmb, t_cs_bkt_inf *csFiletInfs);
INT32 Biz_CS_File_Put(const char *fileName, const char *sourcePathName,S3CannedAcl cannedAcl);
INT32 Biz_CS_File_Get(const char *destPathName, const char *fileName);
INT32 Biz_CS_File_Del(const char *fileName);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

