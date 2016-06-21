#ifndef __SYSDATA_H__
#define __SYSDATA_H__

typedef struct _stBaseParam
{
    unsigned int	dwDeviceID;
    char		szDevName[24];
    unsigned char	byFormat;
    unsigned char	byIsUpdateTime;
    unsigned char	byVChanAmount;
    unsigned char	byAChanAmount;
    unsigned char	bySensorAmount;
    unsigned char	byRelayAmount;
    unsigned char	byPanAddr;
    unsigned char	byUseComNum;
    unsigned long	dwCurTime;
    unsigned int	dwVersion;

} BASEPARAM, *PBASEPARAM;

typedef struct _stEncParam
{
    unsigned char	byFrameRate;
    unsigned char	byRateMode;
    unsigned char	byQuality;
    unsigned char	byEncType;

    unsigned char	byVideoSize;
    unsigned char	byReserved[3];

    unsigned short	nMaxFrameKey;
    unsigned short	nBitRate;
} ENCPARAM, *PENCPARAM;

typedef struct _stVideoParam
{
    unsigned char	byBrightness;
    unsigned char	byChroma;
    unsigned char	bySaturation;
    unsigned char	byContrast;
    unsigned int	dwOsdType;
    unsigned char	szChanName[16];
    unsigned short	nDateX;
    unsigned short	nDateY;
    unsigned short	nBitX;
    unsigned short	nBitY;
    unsigned short	nNameX;
    unsigned short	nNameY;
    ENCPARAM		stMainEncParam;
    ENCPARAM		stSubEncParam;
} VIDEOPARAM, *PVIDEOPARAM;

typedef struct _stAudioParam
{
    unsigned char	byEncType;
    unsigned char	byEnable;
    unsigned short	nBitRate;

} AUDIOPARAM, *PAUDIOPARAM;


typedef struct _stRSParam
{
    unsigned char	byDataBit;
    unsigned char	byStopBit;
    unsigned char	byCheckBit;
    unsigned char	byProtocol;
    unsigned int	dwBoudRate;
} RSPARAM, *PRSPARAM;

typedef struct _stDetectTime
{
    unsigned char	byEnable;
    unsigned char	byBeginHour;
    unsigned char	byBeginMin;
    unsigned char	byBeginSec;

    unsigned char	byEndHour;
    unsigned char	byEndMin;
    unsigned char	byEndSec;
    unsigned char	byReserved;

} DETECTTIME, *PDETECTTIME;

typedef struct _stMotionParam
{
    unsigned char	byEnable;
    unsigned char	byReserved;
    unsigned short	nAlarmOutput;
    unsigned short	nAlarmRecorder;
    unsigned short	nAutoClearTime;
    unsigned int	nDetectLevel;
    unsigned int	dwDetectZone0[45];
    unsigned char	byDetectZone1[45];
    unsigned char	byDetectEnable;
    unsigned char	byReserved1[2];

    DETECTTIME		stDetectTime[8];

} MOTIONPARAM, *PMOTIONPARAM;

typedef struct _stSensorParam
{
    unsigned char	byEnable;
    unsigned char	byAlarmType;
    unsigned char	byAlarmOutput;
    unsigned char	byRecChannel;

    unsigned short	nAutoClrTime;
    unsigned short	nReserved;

    DETECTTIME		stDetectTime[8];

} SENSORPARAM, *PSENSORPARAM;

typedef struct _stVMaskParam
{
    unsigned char	byType;
    unsigned char	byIsPublic;
    unsigned char	byLayer;
    unsigned char	byEnable;

    unsigned int	dwBgColor;

    unsigned short	nPosX;
    unsigned short	nPosY;

    unsigned short	nWidth;
    unsigned short	nHeight;

    unsigned char	byFgAlpha;
    unsigned char	byBgAlpha;
    unsigned short	nReserved;

} VMASKPARAM, *PVMASKPARAM;

typedef struct _stVLossParam
{
    unsigned char	byEnable;
    unsigned char	byAlarmOutput;
    unsigned char	byRecChannel;
    unsigned char	byReserved;

    unsigned short	nAutoClrTime;
    unsigned short	nReserved;

} VLOSSPARAM, *PVLOSSPARAM;

typedef struct _stRecordParam
{
    unsigned char	byEnable;
    unsigned char	byRecType;
    unsigned char	byAutoCover;
    unsigned char	byStoreMedia;

    DETECTTIME		stRecTime[8];

    unsigned short	nRecTime;
    unsigned short	nReserved;

} RECORDPARAM, *PRECORDPARAM;

typedef struct _stCaptureParam
{
    unsigned short	nCapAmount;
    unsigned short	nCapInterTime;

    unsigned char	byStrategyFlag;
    unsigned char	byYear;
    unsigned char	byMon;
    unsigned char	byDay;

    unsigned char	byWeekFlag;
    unsigned char	byHour;
    unsigned char	byMin;
    unsigned char	bySec;

//   DETECTTIME		stRecTime[8];

} CAPTUREPARAM, *PCAPTUREPARAM;

typedef struct _stNetworkParam1
{
    unsigned char	byPppoeEnable;
    unsigned char	byDdnsEnable;
    unsigned char	byDhcpEnable;
    unsigned char	byRegType;

    unsigned int	dwLocalIpAddr;

    unsigned int	dwSubMask;

    unsigned int	dwGateWay;

    unsigned int	dwMCastIpAddr;

    unsigned short	nCmdPort;
    unsigned short	nMediaPort;

    unsigned short	nMCastPort;
    unsigned short	nWebPort;

    unsigned char	szMacAddr[6];
    unsigned char	byConnAmount;
    unsigned char	by3GEnable;

    unsigned int	dwRegServIpAddr;

    unsigned short	nRegServPort;
    unsigned short	nAlarmPort;

    unsigned int	dwAlarmIpAddr;

    char		szDeviceID[24];

    char		szPppoeName[32];

    char		szPppoePass[32];

    char		szDdnsName[32];

    char		szDdnsPass[32];

    char		szDdns[64];

    char		szDdnsServerName[64];

} NETWORKPARAM1, *PNETWORKPARAM1;

typedef struct _stParamList
{
    BASEPARAM		stBaseParam;

    NETWORKPARAM1	stNetParam;

    RSPARAM		stRS232Param;

    RSPARAM		stRS485Param;

    VIDEOPARAM		stVideoParam[4];

    AUDIOPARAM		stAudioParam[4];

    MOTIONPARAM		stMotionParam[4];

    SENSORPARAM		stSensorParam[4];

    VMASKPARAM		stVMaskParam[4][4];

    VLOSSPARAM		stVLossParam[4];

    RECORDPARAM		stRecordParam[4];

    CAPTUREPARAM	stCaptureParam[4];

} PARAMLIST, *PPARAMLIST;

typedef struct _stHxServInfo
{
    char		szRegIpAddr[16];

    unsigned short	nRegPort;
    unsigned short	nReserved;

} HXSERVINFO, *PHXSERVINFO;

typedef struct _stInsideMessage
{
    unsigned short	nResult;
    unsigned short	nReserved;

    unsigned short	nPackIndex;
    unsigned short	nPackAmount;

    char		szPackData[1024];
} INSIDEMESSAGE, *PINSIDEMESSAGE;

int HD_SD_InitParam( PPARAMLIST pstParam );
int HD_SD_SaveParam( PPARAMLIST pstParam );
int HD_SD_SetDefaultParam( PPARAMLIST pstParam );
int HD_SD_CheckVideoParam( unsigned char byFormat, PVIDEOPARAM pstVideoParam );
int HD_SD_CheckVMaskParam( PVMASKPARAM pstVMaskParam );

int HD_SD_GetHXServInfo( PHXSERVINFO pstHXServInfo );
int HD_SD_SetHXServInfo( PHXSERVINFO pstHXServInfo );

#endif //__SYSDATA_H__

