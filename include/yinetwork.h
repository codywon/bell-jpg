#ifndef _YINETWORK_H_
#define _YINETWORK_H_

#define MAX_WIFI_CNT 32

#define MAX_YI_DATA_LEN 1024
#define STARTCODE       0x13141314
#define YI_RECV_PORT    13140

typedef enum
{
    eGetLink,
	eEthLink,
	eWifiLink,
	eReadParam,
	eWriteParam,
	eToApMode,
	eToStaMode,
	eToStaReconn,
	eSmartConnection,
	eWifiScan,
	eFactoryParam,
	eFactoryDDNS,
	eOnline,
	eWifiStatus,
	eUsers,
	eRootUsers,
	eLockControl,
	eSetDeviceId,
	eSetWifi,
	eEnabneIr,
	eSetAutoRebootMins,
}eYiCmd;


typedef enum
{
    eRequest,
    eAck,
    eOK,
    eFail,
    eRead,
    eWrite,
    eWriteWifiParam,
    eWriteApParam
}eYiNetWorkSubCmd;


typedef struct _YiParamCmd
{
    unsigned int startcode;
    eYiCmd cmd;
    eYiNetWorkSubCmd subcmd;
    unsigned int datalen;
    char buffer[MAX_YI_DATA_LEN];
}YIPARAMCMD, *PYIPARAMCMD;

#endif





