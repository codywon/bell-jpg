#ifndef __IEPARAM_H__
#define __IEPARAM_H__

#include "debug.h"



#define MAX_JPUSH_SIZE 16

//#ifdef LDPUSH

#define MAX_PUSH_SIZE 16
#define XPUSH_HOST_NAME "openapi.xg.qq.com"
#define YPUSH_HOST_NAME  "api.tuisong.baidu.com"
#define JPUSH_HOST_NAME  "api.jpush.cn"

typedef struct _stJPushParam
{
	char appKey[64];
	char masterKey[64];
	char receiverValue[64];
	
}JPUSHPARAM,*PJPUSHPARAM;

typedef struct _stYPushParam
{
    char apikey[32];
    char secret_key[48];
    char channel_id[20];
    //unsigned char deploy_status;
}YPUSHPARAM,*PYPUSHPARAM;

typedef struct _stXPushParam
{
    //int access_id;
    char access_id[32];
    char secret_key[40];
    char device_token[68];
}XPUSHPARAM,*PXPUSHPARAM;

typedef struct _stPushParamList
{
    JPUSHPARAM stJPushParam[MAX_PUSH_SIZE];
    YPUSHPARAM stYPushParam[MAX_PUSH_SIZE];
    XPUSHPARAM stXPushParam[MAX_PUSH_SIZE];
    time_t registerTime[MAX_PUSH_SIZE];
    char environment[MAX_PUSH_SIZE];// 1:开发  2:生产
    char devicetype[MAX_PUSH_SIZE];//推送设备类型 0:安卓 1:苹果
    int pushType;
    int validity;
}PUSHPARAMLIST,PPUSHPARAMLIST;
//#endif



typedef struct _stJPushParamList
{	JPUSHPARAM stJpushParam[MAX_JPUSH_SIZE];
    time_t registerTime[MAX_JPUSH_SIZE];
}JPUSHPARAMLIST,*PJPUSHPARAMLIST;



/* BEGIN: Added by Baggio.wu, 2013/7/16 */
#define MAX_VIDEO_BUFFER_SIZE       512*1024
#define MAX_JPEG_BUFFER_SIZE        (256*1024)

#define MAX_USER	8

#define USHORT unsigned short

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/12 */
#if	0
typedef struct
{
	int sendid;
	int on;
	int on_delay_time;
	int on_ok;
	int alarm_delay_time;
	int motion_enable;
	int motion_level;
	int pir_enable;
	int record_enable;
	int record_size;	//m
	int record_cover;
	int ircut_disable;
	int	color_disable;
	/* BEGIN: Modified by wupm(2073111@qq.com), 2014/4/16 */
	int	nLockDelayTime;
	int reserved[28];
}TTT_DOORBELL;

/* BEGIN: Added by wupm(2073111@qq.com), 2014/4/16 */
typedef	struct
{
	BOOL bAudioSending[4];	//MAX_P2P_CONNECT];
	BOOL bVideoSending[4];	//MAX_P2P_CONNECT];
	BOOL bAudioRecving[4];	//MAX_P2P_CONNECT];
}T_AVSTATE;
#endif

//add by iven 2012/9/1
typedef struct tag_STRU_SCCPARAM
{
	unsigned short nLanPort; /* ??óòí??àìy???úo? */
	unsigned char bEnableInternet; /* ?￥áaí?1|?ü???ˉ */
	unsigned char szLanPwd[129]; /* LAN ?ü?? */
	unsigned char szUser[32]; /* scc ó??§?? */
	unsigned char szPwd[32]; /* scc ?ü?? */
	unsigned char szServer[64]; /* scc ·t???÷μ??· */
	unsigned short nServerPort; /* scc ·t???÷???ú */
	unsigned short nReserved; /* ±￡á? */

} STRU_SCCPARAM, *PSTRU_SCCPARAM;

/* BEGIN: Modified by wupm, 2013/1/11 */
/*
typedef struct __TutkParam
{
	char			uid[32];
	char  			user[32];
	char  			passwd[32];
	unsigned char 	model[16];
	unsigned char 	vendor[16];
	unsigned int 	version;
	unsigned int 	channel;
	unsigned int 	total;
	unsigned int 	free;
	unsigned char 	reserved[8];
} TutkParam, *pTutkParam;
*/
typedef struct __TutkParam
{
    char			uid[32];
    char  			user[32];	//unuseful
    char  			passwd[32];	//unuseful
    unsigned char 	model[16];
    unsigned char 	vendor[16];
    unsigned int 	version;	//unuseful
    unsigned int 	channel;	//unuseful
    unsigned int 	total;		//unuseful
    unsigned int 	free;		//unuseful
    unsigned char 	reserved[8];//unuseful

    unsigned int	bEnable;
    char			szServer[4][64];
} TutkParam, *pTutkParam;
/* END:   Modified by wupm, 2013/1/11 */


typedef struct _ieDetectTime
{
    unsigned char       byEnable;
    unsigned char       byBeginHour;
    unsigned char       byBeginMin;
    unsigned char       byBeginSec;

    unsigned char       byEndHour;
    unsigned char       byEndMin;
    unsigned char       byEndSec;
    unsigned char       byReserved;

} IEDETECTTIME, *PIEDETECTTIME;

typedef struct _ieSchduleTime
{
    int		time[7][3];
} IESCHDULETIME, *PIESCHDULETIME;

typedef struct _stIEBaseParam
{
    //char            dwDeviceID[32];		//deviceid
    char            dwDeviceID[24];		//deviceid
	char			dwApiLisense[8];
    char            szDevName[80];		//alias
    int		sys_ver;		//system version
    int		app_ver;		//app version
    char            szMac[17];		//mac
    char		sysmode;		//0->Baby Monitor 1->HD IPCAM
    char		factory;
    char		other1;
    char         szWifiMac[17];		//mac
    /* BEGIN: Added by Baggio.wu, 2013/9/30 */
    //char		other[3];
    char        bUseFactoryMac;

	/* BEGIN: Modified by wupm, 2014/1/1 */
    //char		other[2];
    char		bEnableAP1;
	char		bEnableAP2;
} IEBASEPARAM, *PIEBASEPARAM;

typedef struct _stStatusParam
{
    char       	warnstat;	//0->normal 1->motion alarm 2->io alarm
    char       	ddnsstat;	//0->no start 1->start... 2->login failed 3->login ok
    char       	upnpstat;	//0->no start 1->start... 2->upnp failed  3->upnp is ok
    char       	p2pstat;	//0->no start 1->start... 2->p2p failed   3->p2p is ok
    char		sdstatus;	//0->no sd    1->sd not fromat 2->bad sd(fat is error)  3->sd is ok 4->sd is record...
    //5->sd record is stop 6->sd record failed<write failed>
    char		sysstatus;	//0->system ok 1->system warning
    char		externip[16];
} STATUSPARAM, *PSTATUSPARAM;

typedef struct _stDTimeParam
{
    int        	dwCurTime;
    int       	byTzSel;
    char       	byIsNTPServer;
    char       	byIsPCSync;
    short       	byReserved;
    char            szNtpSvr[32];
} DTIMEPARAM, *PDTIMEPARAM;

typedef struct _stUserParam
{
    char		szUserName[32];
    char		szPassword[32];
} USERPARAM, *PUSERPARAM;

typedef struct _stNetParam
{
    char		szIpAddr[16];
    char            szMask[16];
    char            szGateway[16];
    char            szDns1[16];
    char		szDns2[16];
    char            byIsDhcp;
    char		other[3];
    USHORT      	nPort;
    USHORT      	rtspport;
} NETPARAM, *PNETPARAM;

typedef struct _stWifiParam
{
    unsigned char	byEnable;
    unsigned char	byWifiMode;
    unsigned char	byEncrypt;
    unsigned char	byAuthType;

    unsigned char	byDefKeyType;
    unsigned char	byKeyFormat;
    unsigned char	szReserved;
    unsigned char   byPowerOn;

    unsigned char	byWps;		//0->not start 1->pbc 2->pbin
    unsigned char	wpsstatus;	//0->not action 1->wps failed 2->wps ok 3->wps overlap detection
    //				4->wps process 5->undefine status
    unsigned char	wpsother;
    unsigned char	channel;
    unsigned char	wpspin[8];	//pin codec

    unsigned char	szSSID[64];

    unsigned char	szKey1[64];
    unsigned char	szKey2[64];
    unsigned char	szKey3[64];
    unsigned char	szKey4[64];

    unsigned char	byKey1Bits;
    unsigned char	byKey2Bits;
    unsigned char	byKey3Bits;
    unsigned char	byKey4Bits;

    unsigned char	szShareKey[64];
} WIFIPARAM, *PWIFIPARAM;

typedef struct _stRouteParam
{
    unsigned char   byEncrypt;
    unsigned char   other;
    unsigned short	nport;

    unsigned char   szSSID[64];
    unsigned char   szShareKey[64];
    char		ipaddr[16];
    char		mask[16];
    char		startip[16];
    char		endip[16];
} ROUTEPARAM, *PROUTEPARAM;

typedef struct _stPppoeParam
{
    char		szUserName[64];
    char		szPassword[64];
    char		byEnable;
    char		szReserved[3];

} PPPOEPARAM, *PPPPOEPARAM;

/* BEGIN: Deleted by wupm, 2013/5/3 */
/*
typedef struct _stRtspParam
{
    char            szUserName[64];
    char            szPassword[64];
    char            byEnable;
    char            szReserved[3];

} RTSPPARAM, *PRTSPPARAM;
*/


/* BEGIN: Added by wupm, 2013/5/29 */
//Change BOOL From char to int
typedef struct
{
	BOOL bDisableAP;
	BOOL bAudioLineIN;

	/* BEGIN: Modified by wupm, 2013/5/21 */
	//char szReserved[130];
	char	szServerIP[16];
	int		nServerPort;
	char	szCameraID[16];
	int		nInterval;

	/* BEGIN: Modified by wupm, 2013/5/24 */
	//char	szReserved[84];
	int		nDevicetype;
	char	szReserved[80];
}EXTRA_PARAMS;

typedef struct _stvencparam
{
    unsigned char		bysize;		//VGA->0  QVGA->1 
    unsigned char   	bysizesub;	//0->1/2 1->4/1
    unsigned char		byframerate;	//main rate framerate
    unsigned char		byframeratesub;	//sub  rate 1/1 1/2 1/3 4/1 1/5 1/6 1/7 1/8
    unsigned char		keyframe;	//main rate keyframe
    unsigned char   	keyframesub;	//sub  rate keyframe
    unsigned char   	quant;		//main rate quant
    unsigned char   	quantsub;	//sub  rate quant
    unsigned char   	ratemode;	//main rate 0->vbr 1->cbr
    unsigned char   	ratemodesub;	//sub  rate 0->vbr 1->cbr
    unsigned char   	codecprofile;	//main rate base profile->0 main profile->1
    unsigned char   	codecprofilesub;//sub  rate base profile->0 main profile->1
    unsigned short  	bitrate;	//main rate bitrate  kbps
    unsigned short	bitratesub;	//sub  rate bitrate  kbps
    unsigned char		videomode;	//0->normal 1->mirr 2->flip 3->mirr and flip
    unsigned char		videoenv;	//0->50hz 1->60hz 2->outdoor 3->night mode
    unsigned char		OSDEnable;	//OSDEnable
    unsigned char		brightness;
    unsigned char   	chroma;
    unsigned char   	saturation;
    unsigned char   	contrast;
    unsigned char   	mainmode;	//0->user 1-10
    unsigned char		submode;	//0->user 1-10
    unsigned char		ircut;

	/* BEGIN: Added by wupm, 2013/7/2 */
    //unsigned char		other[2];
    unsigned char		nSensorChip;
	unsigned char		other;
	
} VENCPARAM, *PVENCPARAM;

typedef struct _staencparam
{
    unsigned char   format;         //0->pcm 1->speex
    unsigned char   invol;     	//input volume
    unsigned char   outvol;         //output volume
    unsigned char   cancel;		//cancel 0->disable 1->enable
    unsigned short  bitrate;        //audio bitrate
    int		other;
} AENCPARAM, *PAENCPARAM;

typedef struct _stUpnpParam
{
    unsigned char   byEnable;
    unsigned char   other;
    unsigned short	nReserved;
} UPNPPARAM, *PUPNPPARAM;

typedef struct _stDdnsParam
{
    char				szProxySvr[64];
    char				szDdnsName[64];
    char				szUserName[32];
    char				szPassword[32];
#if 1
    char            		serversvr[64];
    char            		servername[64];
    char            		serveruser[32];
    char            		serverpwd[32];
    unsigned short	serverport;
    unsigned short	serverbeat;
#endif
    unsigned short	nProxyPort;
    unsigned char		byDdnsSvr;
    unsigned char		byMode;
    char				enable;
    char				factorymode;
    char				dnsstatus;
    char				factoryversion;		//10->vstarcam 11->xinhuaan 12->hiware
} DDNSPARAM, *PDDNSPARAM;

typedef struct _stMailParam
{
    char		szSender[64];
    char		szReceiver1[64];
    char		szReceiver2[64];
    char		szReceiver3[64];
    char		szReceiver4[64];
    char		szSmtpSvr[64];
    unsigned short	nSmtpPort;
    unsigned char	byCheck;
    unsigned char	byNotify;
    char		szSmtpUser[64];
    char		szSmtpPwd[64];
    int		other;
} MAILPARAM, *PMAILPARAM;


typedef struct _stAlarmParam
{
    unsigned char	byMotionEnable;	//motion enable
    unsigned char	bySensitivity;	//motion sen
    unsigned char	byAlarmInEnable;//gpio enable
    unsigned char	byAlarmInLevel;//gpio detect value

    unsigned char	byLinkEnable;	//link out
    unsigned char	byAlarmOutLevel;//link out value
    unsigned char   byAlarmEmail;	//alarm email
    unsigned char	byUploadInter;	//alarm ftp

    unsigned char   byAlarmRecord;	//alarm record
    unsigned char   szPresetSit;	//alarm ptz
    unsigned char	bySnapshot;	//alarm snapshot
    unsigned char	alarmen;	//start alarm detect

    unsigned char	record;		//alarm record
    unsigned char	alarm_http;	//alarm http

	/* BEGIN: Modified by wupm, 2013/6/16 */
    //unsigned char	other[2];	//alarm
    unsigned char	enable_take_pic;

	/* BEGIN: Modified by wupm, 2013/6/27 */
    unsigned char	reserved;	//BIT 	0	enable_alarm_audio(0,1)
    							//		1	enable_record_audio(0,1)
    							//		4-7	record_sensitivity(0,1-10)

    unsigned char	alarm_http_url[64];	//alarm http url

    IESCHDULETIME	stAlarmTime;

} ALARMPARAM, *PALARMPARAM;

typedef struct _stFtpParam
{
    char            szFtpSvr[64];

    unsigned short  nFtpPort;
    unsigned char   byMode;
    unsigned char   byReserved;

    char            szFtpUser[32];
    char            szFtpPwd[32];
    char            szFtpDir[32];
    char		szFileName[64];

    short      	nInterTime;
    short      	nReserved;
} FTPPARAM, *PFTPPARAM;

typedef struct _stPTZParam
{
    char		byCenterEnable;
    char		byRate;
    char		byUpRate;
    char		byDownRate;

    char		byLeftRate;
    char		byRightRate;
    char		byLedMode;
    char		byDisPresent;
    char		byOnStart;
    char		byRunTimes;
    char		gpioir;
    char		other1;
    int		other2;
} PTZPARAM, *PPTZPARAM;

typedef struct _stRecordSet
{
    char   		recordover;
    char			timer;		//record file 0->time 1->size
    char			timerenable;	//enable timer record
    char			sdstatus;
    char			other;
    int			size;		//time:second  size:mbyte
    IESCHDULETIME   timerecord;
} RECORDSET, *PRECORDSET;

typedef struct _stMultDevice
{
    char		host[64];
    char		alias[80];
    unsigned int	other;
    char		user[32];
    char		passwd[32];
    unsigned short	port;
} MULTDEVICE, *PMULTDEVICE;

#include "debug.h"
#ifdef	VERSION_RELEASE
/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/10 */
//sizeof(MULTDEVICE) = 64+80+64+6 = 214 *4 = 856
//sizeof(BELLPARAMS) = 64+32*8(=256)+24+32+84+4+128=592
#define	VM_SPEED		0
#define	VM_MID			1
#define	VM_SLOW			2
#define	VM_CUSTOM		3
#define	VM_COUNT		4

#define	AT_MOTION		0
#define	AT_PIR			1
#define	AT_MOTION_PIR	2

//----------------------------

#define	EVENT_MOTION		1
#define	AT_PIR			1
#define	AT_MOTION_PIR	2

//----------------------------
#define	UM_ADDUSER		0
#define	UM_DELUSER		1
#define	UM_UPPRI		2
#define	UM_UPALIAS		3
#define UM_UPPWD		4

/*
0=请求监视
1=停止监视
2=接听
3=开锁
4=结束本次通话
*/
#define	CID_WATCH		0
#define	CID_WATCH_STOP	1
#define CID_AGREE		2
#define	CID_OPEN_LOCK	3
#define	CID_TALK_STOP	4
#define	CID_REFUSE		5

#define	VS_QQVGA		0
#define	VS_QVGA			1
#define	VS_VGA			2
#define	VS_720P			3

#define	PS_IDLE			0
#define	PS_WATCHING		1
#define	PS_CALLING		2
#define	PS_TALKING		3

#define	US_OFFLINE		0
#define	US_ONLINE		1
#define	US_WATCHING		2
#define	US_TALKING		3

#define	WF_STARTUP		0	//系统启动成功，欢迎使用。
#define	WF_CONFIG		1	//系统已切换为网络配置模式，请在5分钟之内完成网络配置。
#define	WF_REBOOT		2	//配置成功，系统重新启动中，请稍候。
#define	WF_CONFIGOK		3	//配置成功
#define	WF_OPENLOCK		4	//门锁已开
#define	WF_WELCOME		5	//欢迎光临。呼叫中，请稍候。
#define	WF_CALL_TIMEOUT	6	//超时无人接听，请稍候再拨。
#define	WF_CALLING		7	//爱的罗曼斯
#define	WF_TALK_TIMEOUT	8	//通话超时
#define	WF_TALK_OVER	9	//对方已挂机，再次感谢您的光临。
//#define	WF_BEEP			10	//Beep, 	 Send Audio
//#define	WF_BEEPBEEP		11	//Beep-Beep, Recv Audio
#define	WF_RESET_OK		12	//复位操作成功，系统重新启动中，请稍候

/* BEGIN: KONX */
/*        Added by wupm(2073111@qq.com), 2014/10/25 */
#define	WF_ALARM_IN		13	//Alarm Sound
#define	WF_SCARE		14	//Alarm Sound
#define WF_WPS          15
#define WF_MUTE         16
#define	WF_COUNT		17	//Total-Count

//#define	PRESS_BUTTON	2
//#define	PRESS_PIR		1

//0=OK,1=DHCP,2=ROUTER,3=INTERNET(ERROR)
#define	NS_OK			0
#define	NS_DHCP			1
#define	NS_ROUTER		2
#define	NS_INTERNET		3

#define	BELL_WATH_WAIT_TIMEOUT	0
#define	BELL_CALL				1
#define	BELL_ALARM				2
#define	BELL_AGREE				3
#define BELL_CALL_WAIT_TIMEOUT	4
#define	BELL_CALL_TIMEOUT		5
//#define	BELL_WATH_STOP			6
#define	BELL_RESET_TIMER		7
#define	BELL_AGREE_ME			8

//typedef int (*MYFUN)(int, int);
//这种用法一般用在给函数定义别名的时候
//上面的例子定义MYFUN 是一个函数指针, 函数类型是带两个int 参数, 返回一个int

typedef void(*MYFUN)(void);

typedef struct
{
	int address;
	int normal;	//1
	int value;	//Current Value
	int total;	//total times
	int times;
	MYFUN OnPress;
	int long_times;	//0,>0, if < 0, means this is a switch IO
					//if value=0, run OnPress
					//if value=1, run OnLongPress
	MYFUN OnLongPress;
}BELLGPIO;

//INPUT GPIO
#define	_DEFKEY				0x00000001		//4
#define	_ALARM_IN			0x00200000		//1

#define	_MOTO_LEFT			0x00100000		//3
#define	_MOTO_RIGHT			0x00080000		//5
#define	_MOTO_UP			0x00020000		//2
#define	_MOTO_DOWN			0x00040000		//6

//OUTPUT GPIO
#define	_ALARMOUT			0x08000000		//25

#define	_MOTO_D0			0x00000800		//23	IRCUT
#define	_MOTO_D1			0x00001000		//29	SetLedStatus	Front-LED
#define	_MOTO_D2			0x00002000		//21	Left-LED
#define	_MOTO_D3			0x00004000		//26	Right-LED
#define	_MOTO_D4			0x04000000		//24
#define	_MOTO_D5			0x01000000		//22
#define	_MOTO_D6			0x00800000		//30
#define	_MOTO_D7			0x00400000		//27

#define	_ENCPOWER			0x02000000		//STATUSLED_AU
#define	_SCL				0x00000004		//I2C Clock
#define	_SDA				0x00000002		//I2C Data

#ifdef FOBJECTTEST
//INPUT(DEFAULT)
#define	BELL_RESET			_DEFKEY			//Reset
//#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
//#define	MAX_BELL_GPIO_COUNT	2
#define	MAX_BELL_GPIO_COUNT	1
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D1		//
#endif

#ifdef FOBJECT
//INPUT(RISEN)
#define	BELL_BUTTON			_MOTO_LEFT		//Button
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset
#define	MAX_BELL_GPIO_COUNT	3

//OUTPUT(RISEN)
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D2		//Status LED
#define BELL_AUDIO          _MOTO_UP        //Ctrl LY8898
#define BELL_CTRL433        _MOTO_D5
#endif

#ifdef FOBJECT_FM34
//INPUT(RISEN)
#define	BELL_BUTTON			_MOTO_LEFT		//Button
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset
#define BELL_PIR_IN         _MOTO_RIGHT

#define	MAX_BELL_GPIO_COUNT	4

//OUTPUT(RISEN)
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D3		//Status LED
#define BELL_AUDIO          _MOTO_UP        //Ctrl LY8898
#define BELL_CTRL433        _MOTO_D5
#endif

#ifdef JINQIANXIANG
//INPUT(RISEN)
#define	BELL_AP_P2P			_MOTO_D2		//AP / P2P SWITCH
#define	BELL_CAPTURE		_MOTO_D0		//PHOTO
#define	BELL_RECORD			_MOTO_D1	    //RECORD
#define BELL_CAPTURE2       _DEFKEY
#define	MAX_BELL_GPIO_COUNT	4
#endif


#ifdef FUHONG
//INPUT(RISEN)
#define	BELL_BUTTON			_MOTO_LEFT		//Button
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset
#define	MAX_BELL_GPIO_COUNT	3

//OUTPUT(RISEN)
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D3		/*Baggio: STATUS LED, 1-ON / 0-OFF */
#define BELL_AUDIO          _MOTO_UP        //Ctrl LY8898
#define BELL_CTRL433        _MOTO_D5
#define	BELL_IR_OUT			_MOTO_D0        /*Baggio: IR LED, 0-ON / 1-OFF*/
#endif

#ifdef EYESIGHT
//INPUT(RISEN)
#define	BELL_BUTTON			_MOTO_LEFT		//Button
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset
#define	MAX_BELL_GPIO_COUNT	3

//OUTPUT(RISEN)
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D2		//Status LED
#define BELL_AUDIO          _MOTO_UP        //Ctrl LY8898
#define BELL_CTRL433        _MOTO_D5
#endif


#ifdef KANGJIEDENG
//INPUT(RISEN)
#define	BELL_BUTTON			_MOTO_LEFT		//Button
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset
#define	MAX_BELL_GPIO_COUNT	3

//OUTPUT(RISEN)
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D2		//Status LED
#define BELL_AUDIO          _MOTO_UP        //Ctrl LY8898
#define BELL_CTRL433        _MOTO_D5
#endif


#ifdef ZHENGSHOW
//INPUT(RISEN)
#define	BELL_BUTTON			_MOTO_LEFT		//Button
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset
/* add begin by yiqing, 2015-09-06, 原因: 检测光敏电阻变化控制IRC*/
#define	BELL_PIR			_MOTO_UP		//PIR

#ifdef NEW_BRAOD_AES
/* add begin by yiqing, 2016-03-29,英国客户ap按键*/
#define AP_KEY              _MOTO_DOWN
#define	MAX_BELL_GPIO_COUNT	5
#else
#define	MAX_BELL_GPIO_COUNT	4
#endif


//OUTPUT(RISEN)
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D2		//Status LED
#define	BELL_IR_CONTROL		_MOTO_D0		//1=Enable, 0=Disable  镜头板红外灯电源控制,加pir功能后去掉
#define	BELL_IR_CONTROL0		_MOTO_D0		//1=Enable, 0=Disable
#define	BELL_IR_CONTROL1		_MOTO_D1		//1=Enable, 0=Disable
#define BELL_AUDIO          _MOTO_D4        //0=On, 1=Off  
/* add begin by yiqing, 2015-09-04, 原因: 5350开门后通知单片机*/
#define BELL_NOTYCE_TO_MCU   _MOTO_D6//_MOTO_D3

/* add begin by yiqing, 2016-03-26*/
#define	BELL_OPENDOOR2		_MOTO_D7		//Open Door(K)

/* add begin by yiqing, 2016-03-26,英国客户新AES，每天00:00重启*/
#define AUTO_REBOOT_IO1     _MOTO_D2
#define AUTO_REBOOT_IO2     _MOTO_D5

#endif


#ifdef FRISEN
//INPUT(RISEN)
#define	BELL_BUTTON			_MOTO_LEFT		//Button
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset
#define	MAX_BELL_GPIO_COUNT	3

//OUTPUT(RISEN)
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D2		//Status LED
#endif

#ifdef FACTOP
//INPUT(ACTOP)
#define	BELL_BUTTON			_MOTO_LEFT		//Button
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset
#define	BELL_PIR			_MOTO_UP		//PIR
#define	MAX_BELL_GPIO_COUNT	4

//OUTPUT(ACTOP)
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D5		//Status LED(D1\D3)

/* BEGIN: ACTOP, Red LED */
/*        Added by wupm(2073111@qq.com), 2014/10/13 */
#define	BELL_RED_LED		_MOTO_D6

//OTHER(ACTOP)
#define	BELL_SENSOR_D1		_MOTO_D1
#define	BELL_LAMP_D0		_MOTO_D0
#define	BELL_LAMP_DRIGHT	_MOTO_RIGHT

#define	BELL_POWER_D9		_MOTO_D3
#define	BELL_POWER_D8		_MOTO_D4
#define	BELL_RJ45_LED2_		_MOTO_D2
#endif

/* BEGIN: Ai-Hua-xun */
/*        Added by wupm(2073111@qq.com), 2014/8/8 */
#ifdef FEEDDOG
/*
1，PIR，飞线至MOTO_UP。常态为低，触发后为高。（启动后1分钟后生效）。目前的逻辑是做为报警信号来处理的。
2，按键：MOTO_RIGHT。常态为低，按下触发为高。目前的逻辑是：长按进入配置模式，短按为门铃呼叫。
5，DEFKEY：复位按键。
6，ALARM_IN：由GN_KEY控制，作用未确定。暂时没做。
7, MOTO_LEFT：光敏电阻，通知主控去控制红外灯。高=灯灭，低=灯亮。

3，指示灯：MOTO_D3。目前的逻辑是：呼叫时闪烁，常态熄灭。网络工作不正常时，快速闪烁。
4，红外灯：MOTO_D0。控制逻辑待定。根据通知去控制。
*/
//INPUT(Ai-Hua-xun)
/* modify begin by yiqing, 2015-07-27, 原因:中控修改*/
#ifndef ZHONGKONG
#define	BELL_BUTTON			_MOTO_RIGHT		//Button
#else
#define	BELL_BUTTON	            _MOTO_LEFT
#endif

#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset 

#ifndef ZHONGKONG
#define	BELL_PIR			_MOTO_UP		//PIR
#else
#define	BELL_PIR			_MOTO_RIGHT		//PIR
#endif
//#define	BELL_IR_IN			_MOTO_LEFT		/*Baggio: 1-normal/0-effective*/
#define	MAX_BELL_GPIO_COUNT	4 /*when calling or watching, check BELL_IR_IN*/

//OUTPUT(ACTOP)
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D3		/*Baggio-Status LED(1-off/0-on)*/
#define	BELL_IR_OUT			_MOTO_D0        /*Baggio: IR LED, 1-ON / 0-OFF*/
#endif

#ifdef YUELAN
#define	BELL_BUTTON			_MOTO_RIGHT		//Button
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset
#define	BELL_PIR			_MOTO_UP		//PIR
//#define	BELL_IR_IN			_MOTO_LEFT		/*Baggio: 1-normal/0-effective*/
#define	MAX_BELL_GPIO_COUNT	4 /*when calling or watching, check BELL_IR_IN*/

#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D3		/*Baggio-Status LED(1-off/0-on)*/
#define	BELL_IR_OUT			_MOTO_D0        /*Baggio: IR LED, 1-ON / 0-OFF*/
#endif

#ifdef BELINK
#define	BELL_BUTTON			_MOTO_LEFT		//Button
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Open Door)
#define	BELL_RESET			_DEFKEY			//Reset
#define	MAX_BELL_GPIO_COUNT	3

#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(K)
#define	BELL_LED			_MOTO_D3		/*Baggio-Status LED(1-off/0-on)*/
#define BELL_NET_LED        _MOTO_D2

//#define BELL_AUDIO_V2       _MOTO_UP        /*Baggio NS4158 0-normal / 1-effective*/

#define BELL_AUDIO          _MOTO_UP
#define	BELL_IR_CONTROL		_MOTO_D0


#endif


/* BEGIN: KONX */
/*        Added by wupm(2073111@qq.com), 2014/10/25 */
#ifdef	KONX
//in
#define	BELL_ALARMIN		_ALARM_IN	//0-1,Sendto Handphone and LocalRing
#define	BELL_PIR_IN			_MOTO_UP	//0-1,PIR Alarm
#define	BELL_BUTTON			_MOTO_LEFT	//0-1,Call
#define	BELL_RESET			_DEFKEY		//1-0,Reset
#define	BELL_SWITCH			_MOTO_RIGHT	//0-1,Switch Function
										//Off,Open LED(Default)
										//On,Close LED, Enable _ALARM_IN
#define	BELL_IR_DETECT		_MOTO_DOWN	//Switch Button
										//1,Night, Open IR LED
										//0,Day,Close IR LED
//in-count
#define	MAX_BELL_GPIO_COUNT	6
//out
#define	BELL_LED			_MOTO_D2	//Status LED
#define BELL_ONOFF_LED		_MOTO_D4
										//_MOTO_D5	//Control by MOTO_RIGHT
										//20141029 Changed
#define BELL_OPENDOOR		_ALARMOUT	//Open Door,0-1,maintain 2s
#define	BELL_IR_OUT			_MOTO_D3	//1=Close, 0=Open, Control by  BELL_IR_DETECT

//20141130										
#define	BELL_IR_CONTROL		_MOTO_D3	//1=Enable, 0=Disable	                                        
#define BELL_SPK_CONTROL    _MOTO_D0    //Control Speak                                    

#endif

#ifdef	ZILINK
//in
#define	BELL_ALARMIN		_ALARM_IN	//1-0,Sendto Handphone
#define	BELL_BUTTON			_MOTO_LEFT	//0-1,Call                          PCB:SW2-SW3
#define	BELL_RESET			_DEFKEY		//1-0,Reset                       PCB:SW1
//in-count
#define	MAX_BELL_GPIO_COUNT	3
//out
#define	BELL_NETWORK_LED	_MOTO_D2	//0=On,1=Off
#define BELL_OPENDOOR		_ALARMOUT	//Open Door,0-1,maintain 2s
#define	BELL_WPS_LED		_MOTO_D3	//0=On,1=Off
#define	BELL_LED			_MOTO_D1	//Status LED
										//0=On, 1=Off
										//When Talking=On
										//if NO ANSWER, Delay 20S
#define BELL_AUDIO          _MOTO_D5    //0=On, 1=Off               /********HW2.0 will remove it********/
#define	BELL_AUDIO_2ND		_MOTO_D0	//1=Enable, 0=Disable    /********Hw2.0 will using it********/
#endif

/* BEGIN: Fix BUG for NEW Version for 0815-APK */
/*        Added by wupm(2073111@qq.com), 2014/9/2 */
#ifdef	ZIGBEE
#define	BELL_RESET			_DEFKEY			//Reset
#define	BELL_BUTTON			_MOTO_LEFT		//Button
#define	BELL_PIR_IN			_MOTO_UP		//PIR(Notify, Control BELL_PIR_OUT)(BackLight)
#define	BELL_ALARMIN		_ALARM_IN		//Alarm in(Notify)(433STUDY)
#define	BELL_IR_IN			_MOTO_RIGHT		//IR_DETECT(Control BELL_IR_OUT)

#define	MAX_BELL_GPIO_COUNT	5   /*5->4 cancel reset key*/

#define BELL_CALL_LED       _MOTO_D1
#define	BELL_LED			_MOTO_D2		//Status LED
#define	BELL_PIR_OUT		_MOTO_D6		//BackLight
#define	BELL_IR_OUT			_MOTO_D3		//IR Control, Open/Close IR LED
#define	BELL_OPENDOOR		_ALARMOUT		//Open Door(Reserved)
#endif

#ifdef BAFANGDIANZI
//INPUT
#define BELL_SENSOR_IN      _MOTO_RIGHT
#define BELL_AP             _DEFKEY        //AP / P2P SWITCH

#define	MAX_BELL_GPIO_COUNT	2

#endif

//ControlBellLED
#define	CB_OPEN				0
#define	CB_CLOSE			1
#define	CB_FLASH_SLOW		2
#define	CB_FLASH_FAST		3
#define	CB_FLASH_STOP		4
//#define	CB_FLASH_ONCE		5	//ACTOP
#define	CB_CONTINUE_MOMENT	6	//ACTOP

typedef struct
{
	char	bell_on;	//1
	char 	bell_audio;	//1
	char	bell_mode;	//1
	char	a;

	short	max_watch;	//30
	short	max_talk;	//60
	short	max_wait;	//15
	short	b;

	char	lock_type;	//0
	char	lock_delay;	//10
	char	c1;
	char 	c2;

	char	pin;
	char	pin_bind;
	char	pout;
	char	pout_bind;

	char	alarm_on;	//1
	char	alarm_type;
	char	alarm_level;
	char	alarm_onok;

	short	alarm_delay;	//20
	short	e;
	char	alarm_start_hour;
	char	alarm_stop_hour;
	char	alarm_start_minute;
	char	alarm_stop_minute;

	char	user[MAX_USER][32];	//user[0] = "admin"
	char	pwd[MAX_USER][32];
	char	xalias[32];			//"Bell"

	char	admin;				//1
	char	f1;             //20141206, KONX, default = 0; if Set Lock, then Set to 1
	char	f2;             //20141206, KONX, default = 0; if Set Lock, then Set to 1
	char	video_mod;		//1/2/3/4, Current mod

	char	video_resolution[VM_COUNT];
	char	video_framerate[VM_COUNT];
	short	video_bitrate[VM_COUNT];

	char	video_brightness;
	char	video_contrast;
	char	video_saturation;
	char	video_hue;

	char	hw_ver[16];
	char	sw_ver[16];

	char	xmac[8];
	char	xwifi_mac[8];

	char	xdhcp;
	char	xdns_auto;
	short	xport;

	char	xdns1[16];
	char	xdns2[16];
	char	xip[16];
	char	xmask[16];
	char	xgateway[16];

	char	xwifi;			//enable wifi
	char	ap_mode;		//in ap state ?
	char	status;
	char	sensor;			//0=MJ, 1=HD

	char	xssid[64];
	char	xkey[32];
	char	xdevice_id[32];

	//
	char	current_brightness;
	char	current_contrast;
	char	current_saturation;
	char	current_hue;

	char	current_resolution;
	char	current_framerate;
	short	current_bitrate;

	char	xx[64];
}BELLPARAMS;
#endif

typedef struct _stPreSet
{
    unsigned short	leveltime;
    unsigned short	verttime;
    unsigned char	speed;
    unsigned char	used;
    unsigned char	other[2];
} PRESET, *PPRESET;


typedef struct _IEParamList
{
    IEBASEPARAM		stIEBaseParam;
    STATUSPARAM		stStatusParam;
    PPPOEPARAM		stPppoeParam;
    ALARMPARAM		stAlarmParam;
    DTIMEPARAM		stDTimeParam;
    DDNSPARAM		stDdnsParam;
    FTPPARAM		stFtpParam;
    NETPARAM		stNetParam;
    MAILPARAM		stMailParam;
    PTZPARAM		stPTZParam;
    UPNPPARAM		stUpnpParam;
    USERPARAM		stUserParam[MAX_USER];
    WIFIPARAM		stWifiParam;
    ROUTEPARAM		stWifiRoute;
    VENCPARAM		stVencParam;
    AENCPARAM       stAencParam;
    RECORDSET       stRecordSet;
    PRESET			stPreset[16];

    MULTDEVICE		stMultDevice[8];

	/* BEGIN: Modified by wupm, 2013/5/3 */
    //RTSPPARAM		stRtspParam;
    EXTRA_PARAMS	stExtraParam;

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/11 */
	BELLPARAMS		stBell;

} IEPARAMLIST, *PIEPARAMLIST;

typedef struct _stSmartEye
{
    char				mac[17];
    char				macother;
    char				mid[32];
    char				pid[32];
    char				sn[32];
    char				se_ddns_svr[32];
    unsigned short	se_ddns_port;
    unsigned short	se_ddns_interval;
    char				se_ddns_enable;
    char				se_ddns_status;
    unsigned short	other;
    char				se_ddns_name[64];
    char				se_ddns_user[64];
    char				se_ddns_pwd[64];
} SMARTEYE, *PSMARTEYE;

typedef struct _IEParamCmd
{
    unsigned short	startcode;
    unsigned char		cmd;
    unsigned char		other;
    IEPARAMLIST		bparam;
} IEPARAMCMD, *PIEPARAMCMD;

typedef struct _RECORDSCC
{
	unsigned char	motionrecord;
	unsigned char	externrecord;
	unsigned char	autorecord;
	unsigned char	recordcove;
	unsigned char	recordaudio;
	unsigned char	recordlen;
	unsigned char	other[2];
} RECORDSCC, *PRECORDSCC;

#if 0
typedef struct _stBcastParam
{
    char            szIpAddr[16];
    char            szMask[16];
    char            szGateway[16];
    char            szDns1[16];
    char            szDns2[16];
    USHORT          nPort;
    USHORT          other0;
    USHORT          other1;
    USHORT          rtspport;
    unsigned char   szMacAddr[6];
    USHORT          other;
    char            dwDeviceID[32];
    char            dhcp;
    char            szDevName[32];
} BCASTPARAM, *PBCASTPARAM;
#else
typedef struct _stBcastParam
{
    char            		szIpAddr[16];
    char            		szMask[16];
    char            		szGateway[16];
    char            		szDns1[16];
    char            		szDns2[16];
    unsigned char   	szMacAddr[6];
    USHORT          	nPort;
    //char            		dwDeviceID[32];
	char            		dwDeviceID[24];
    char            		dwApiLisense[8];
    char            		szDevName[80];
    char				sysver[16];
    char				appver[16];
    char            		szUserName[32];
    char            		szPassword[32];
    char            		sysmode;        		//0->baby 1->HDIPCAM
    char				dhcp;
    char            		other[2];       		//other
    char				other1[20];			//other1
} BCASTPARAM, *PBCASTPARAM;

typedef struct _stBcastDDNS
{
    char			szProxySvr[64];
    char			szDdnsName[64];
    char			szUserName[32];
    char			szPassword[32];
} BCASTDDNS, *PBCASTDDNS;
#endif

typedef struct _stBcastJpeg
{
    int		startcode;	//jpeg head
    short		cmd;		//jpeg cmd
    char		other;		//other
    char		other1[8];	//other1
    int		len;		//len
    int		other2;
} BCASTJPEG, *PBCASTJPEG;

typedef struct _stBcastProtocol
{
    short		startcode;
    short		cmd;
    BCASTPARAM	bcast;
} BCASTPROTOCOL, *PBCASTPROTOCOL;

typedef struct __SDFILEINDEX
{
    int     	now;            //current time
    char    	name[24];
    off_t   	offsetstart;    //sd start offset
    off_t   	offsetend;      //sd end offset
    char    	type;           //1bit ->time 2bit ->motion flag 3bit ->gpio flag
    char    	init;           //
    char    	other1;         //
    char    	endflag;        //0xaa->end ok flag
} SDFILEINDEX, *PSDFILEINDEX;

typedef struct __SDOFFSET
{
    unsigned int    index;
    off_t           start;
    off_t           end;
} SDOFFSET, *PSDOFFSET;
#if 0
typedef struct __MediaHead
{
    char                    byFrameType;                            //0->audio 1->I frame 2->p frame
    char                    other[3];
    unsigned int            dwOffset;
    unsigned int            dwFrameLen;
    unsigned int            dwFrameNo;
} MEDIAHEAD, *PMEDIAHEAD;
#endif

typedef struct _stDefaultParam
{
    char            mac[6];
    char            server[30];
    char            deviceid[32];
    unsigned short  port;
    unsigned short  interval;
    char            subname[64];
    char            dnsuser[64];
    char            dnspaswd[64];
    char            mid[32];
    char            pid[32];
    char            defaultip[16];
} DEFAULTPARAM, *PDEFAULTPARAM;

typedef struct _stRecDataHead
{
    unsigned int		dwTimeStamp;		//time:s
    unsigned short		nMilliTimeStamp;	//time:ms
    unsigned char		byIsVideo;		//1->video,0->audio
    unsigned char		byFrameType;		//i frame->1 p frame->0
    unsigned int		dwFrameNo;		//frameno
    unsigned int		dwFrameLength;		//len
} REC_DATA_HEAD, *PREC_DATA_HEAD;

#if 0
typedef struct _MYOSDParam
{
    unsigned char		OsdType;
    unsigned char		FrameRate;
    unsigned short 		BitRate;
    unsigned char		Title[16];
    DateTime		stDateTime;
    short			datex;
    short			datey;
    short			bitx;
    short			bity;
    short			titlex;
    short			titley;
} MYOSDPARAM, *PMYOSDPARAM;
#endif
typedef struct __WifiCont
{
    char			ssid[64];
    char			mode[8];
    char			key[64];
    char               mac[20];
    char               modetype;       //adhoc infra
    char			Enctype;	//key on off
    char			Authtype;	//auth
    char			keyon;
    char			db0[4];
    char			db1[4];
    char			channel;
    char			other[3];
} WifiCont, *PWifiCont;

typedef struct __ielog
{
    unsigned int 	dwCurTime;
    unsigned int  	szIpAddr;
    char		opration[64];
} IELOG, *PIELOG;

typedef struct __IEloghead
{
    unsigned char	IEIndex;
    unsigned char 	other;
    unsigned short  IETotal;
} LOGHEAD, *PLOGHEAD;

typedef struct __WIFISCAN
{
    unsigned char	networktype;
    unsigned char  	encrypt;
    unsigned short	other;
    char		SSID[64];
} WIFISCAN, *PWIFISCAN;

typedef struct __WIFIHEAD
{
    unsigned char  	WIFIindex;
    unsigned char	other;
    unsigned short  WIFItotal;
} WIFIHEAD, *PWIFIHEAD;

typedef struct _stUpdateHead
{
    unsigned short	nCommand;
    unsigned short  nDataLen;

    unsigned short	nPackIndex;
    unsigned short	nPackCount;

    unsigned char	szMacAddr[6];
    unsigned char	system;
    unsigned char	other;
} UPDATE_HEAD, *PUPDATE_HEAD;

typedef struct _stUserInfo
{
    char    szUserName[16];
    char    szPassword[16];
} USER_INFO, *PUSER_INFO;

typedef struct _ALARMLOG
{
    char		type;
    char		mon;
    char 		day;
    char    	hour;
    char		min;
    char		sec;
    unsigned short	year;
} ALARMLOG, *PALARMLOG;


/* BEGIN: Added by wupm, 2013/3/12 */
typedef struct
{
    //user	Current user
    //pwd		Current password
    char ht_pid[32];		//(deviceModel Name)	HTI-3300 (Assigned value on production)
    //*IPCam model name assigned by honestech    length <=20
    char ht_ddns_svr[64];	//ddns.mybabyok.com   (Assigned value on production)
    //*DDNS domain name operated by honestech     length <=64
    char ht_ddns_host[64];	//HT_DDNS host name    length <=64
    char ht_ddns_user[64];	//HT_DDNS user ID                   length <=64
    char ht_ddns_pwd[64];	//HT_DDNS user Password      length <=64
    unsigned int ht_ddns_interval;	//18000 (min seconds, 18000 = 180 sencodes)  (Assigned value on production)
    //*DDNS server update interval operated by honestech
    unsigned int ht_ddns_port;		//80 (Assigned value on production)
    //*DDNS domain Port operated by honestech
    char ht_camerasn[32];	//IP camera serial number     length <=  20
    //*IP camera serial number  (supplied by honestech)
    unsigned int ht_ddns_status;
    //HT_DDNS  Status show
    //0: Normal operation
    //1: contact failure
    //2: user  ID/PW inconsistency
} THTCLASS_FACTORY_PARAMETERS;
#define	HT_DDNS_SUCC	0
#define	HT_DDNS_FAIL	1
#define	HT_DDNS_ERROR	2
typedef	struct
{
    //user	Current user
    //pwd	Current password
    //char ht_ddns_user[64];	//HT_DDNS user ID  length <=64
    //char ht_ddns_pwd[64];	//HT_DDNS user Password    length <=64
    char ht_alarm_svr[64];	//alarm.mybabyok.com   (Assigned value on production)
    //*alarm server  operated by honestech     length <=64
    unsigned int ht_alarm_port;		//80 (Assigned value on production)
    //*alarm server Port operated by honestech
    unsigned int alarm_enabled;		//1 :  Alarm sending
    //0:  Alarm no-sending
    //If the setting condition of set_alarm.cgi meets, it sends alarm.
    //char ht_ddns_honst[64];	//HT_DDNS host name    length <=64
} THTCLASS_ALARM_PARAMETERS;
typedef struct
{
    THTCLASS_FACTORY_PARAMETERS	ddns;
    THTCLASS_ALARM_PARAMETERS	alarm;
} THTCLASS_PARAMETERS;
/* END:   Added by wupm, 2013/3/12 */

#ifdef TENVIS
typedef struct
{
	int		nEnable;
	char	szReserved[16];
} TTENVISPARAM, *PTTENVIPARAM;
#endif

/* BEGIN: Added by Baggio.wu, 2013/7/31 */
#ifdef CUSTOM_HTTP_ALARM
typedef struct
{
	int		nSensorChip;
	char	alarm_svr[64];
	int		alarm_port;
	char	alarm_user[64];
	char	alarm_pwd[64];
	char	szReserved[128];
	int		bEnableAlarmSend;
} TPSDPARAM, *PTPSDPARAM;
#endif

#ifdef CHANGE_DDNS_ALARM_SERVER
typedef struct
{
	int		bEnable;
	char	alarm_svr[64];
	char	reserved[128];
} TVSTARPARAM, *PTVSTARPARAM;
#endif
/* END:   Added by Baggio.wu, 2013/7/31 */

typedef enum _WIFIMODE
{
    eStaMode,
    eApMode,
    eMaxMode
}eWifiMode;

#define LIVE_RECORD_OK			0
#define LIVE_RECORD_NOT			205
#define LIVE_RECORD_BAD			206
#define LIVE_RECORD_END			207
#define LIVE_MAX_CONNECT			201

#define SJPEGRATE	3

#define RECORDRATE	4


#define MAX_TALKLEN (256 + 32)
#define MAX_TALKLEN_DATA 256

#define EBUF_MAX_MLEN (4*1024*1024)
#define EBUF_MAX_SLEN (2*1024*1024)
#define EBUF_MAX_JLEN (512*1024)
typedef struct __ENCBUF
{
    unsigned short	index;			//frame index
    char			flag;			//flag == 1 need wait send flag== 0 don't wait
    char			firstflag;		//first write buffer
    short			firstindex;		//first total len
    short			preindex;		//last index;
    unsigned int	totallen;			//total len
    unsigned int	prelen;			//last frame offset
    unsigned short	sit;				//sit
    unsigned short	presit;			//presit
    unsigned char*	pbuf;			//main buffer;
} ENCBUF, *PENCBUF;

typedef struct __ENCBUFIND
{
    unsigned short      index;          //frame index
    unsigned short      aindex;         //audio index
    unsigned char                flag;           //flag == 1 need wait send flag== 0 don't wait
    char                type;           //0->h264 iframe 1->h264 bframe 2->ogg audio 3->pcm audio 4->jpeg
    short               other;
    unsigned int        aoffset;                //audio offset
    unsigned int        offset;         //frame offset
} ENCBUFIND, *PENCBUFIND;

typedef struct _SDRECORD
{
    char				filename[40];
    char				timer;		//timer calute 0->timer 1->size
    char				flag;		//0->create a filename 1->filename is ok
    unsigned short       size;           //timer:second len:mbyte
    char				start;		//0->stop record 1->start record
    char				other;
    unsigned short	timer1;		//inc timer
    char				motion;
    char				gpio;
    char				schdule;
    char				other1;
} SDRECORD, *PSDRECORD;

typedef struct __ENCBURECORD
{
    unsigned short	index;		//frame index
    unsigned short  	aindex;		//audio index
    char				flag;		//flag == 1 need wait send flag== 0 don't wait
    char				type;		//0->h264 iframe 1->h264 bframe 2->ogg audio 3->pcm audio 4->jpeg
    short			recordflag;
    unsigned int		aoffset;		//audio offset
    unsigned int		offset;		//frame offset
    unsigned char		filename[64];

#ifdef AVI_RECORD_TO_SD_CARD_ENABLE
		//Modify by liangdong on 2014-3-20 14:30 
		unsigned int AudioOffset;
		unsigned int VideoOffset;
		char				 VideoAudioFlag;
		char         FirstLinkFlag;
		char         Reserver1;
		char         Reserver2;
#endif		
} ENCBURECORD, *PENCBURECORD;

typedef struct _LIVEUSER
{
    int           	socket;
    unsigned int    refcnt;
    char		other;
    char        	streamid;
    char        	http;
    char         	head;
    char       	audio;
    char		talk;
    short		talklen;
    char		buffer[1024];
} LIVEUSER, *PLIVEUSER;


#define MAX_WIFI_ITEM_COUNT     32

extern eWifiMode geWifiMode;

extern BOOL bAudioPowerAmplifierStatus;


extern IEPARAMLIST 	bparam;
extern IEPARAMCMD	bnetparam;
extern WifiCont    		WifiResult[MAX_WIFI_ITEM_COUNT];
extern unsigned char* 	 plogbuf;
extern char			enccnt;
extern struct tm*     	 recordtm;
extern long long        	sdtotal;
extern long long        	sdfree;
extern unsigned char	sdstatus;
extern char         		webflag;
extern char             	arecord;
extern unsigned char    h264buf[1400];
extern unsigned char    mp3buf[960];
extern char            	dnsresult[64];
extern char			sendsit;
extern unsigned char    third_status;
extern char             	maildns[64];
extern char             	wificnt;
extern SDOFFSET         sdoffset;
extern char    			sdflag;
extern int              		sdtime;
extern char            	recordtimeflag;
extern char           		recordstopflag;
extern char           		recordalarmflag;
extern unsigned short  	weblog[256];
extern USERPARAM	userbackup[3];

extern char			updateflag;
extern unsigned char 	encryptflag;
extern int 			upnpp2pport;
extern int 			bupnpp2pport;
extern char			netok;
extern char			wifiok;
extern int				externwifistatus;
extern SMARTEYE			eyeparam;

extern int bPIRStatus;

#ifdef JPUSH
extern JPUSHPARAMLIST jpushparamlist;
extern char jpush_address[16];
#endif

//#ifdef LDPUSH
extern PUSHPARAMLIST pushparamlist;
extern char xpush_address[16];
extern char ypush_address[16];
extern char jpush_address[16];
extern char bEnablePush;
//#endif


#ifdef TENVIS
extern TTENVISPARAM tenvisparam;
#endif
/* BEGIN: Added by Baggio.wu, 2013/7/31 */
#ifdef CUSTOM_HTTP_ALARM
extern TPSDPARAM	psdparam;
#endif

#ifdef CHANGE_DDNS_ALARM_SERVER
extern TVSTARPARAM vstarparam;
#endif
extern TutkParam 	tutkparam;
/* BEGIN: Deleted by Baggio.wu, 2013/7/10 */
//void DnsSendAlarm( char alarmtype );
void setDnsSendAlarmFlag( int flag, int alarmType );
void LedClose( void );
void LedMode1( char flag );
void LedMode2( char flag );

int SaveSystemParam( PIEPARAMLIST pstParam );

void SystemBoot( void );

/* BEGIN: Added by yiqing, 2015/3/18 */
void ReadJPushParams(void);
void WriteJPushParams(void);
int jPush(char *send_data);
/* END:   Added by yiqing, 2015/3/18 */

#ifdef LDPUSH
void ReadPushParams(void);
void WritePushParams(void);
int LdPush(char *send_data,char type);
#endif

#endif //__CONFIG_H__

