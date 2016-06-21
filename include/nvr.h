
#ifndef _NVR_H_
#define _NVR_H_

#define MAX_MULT_DEV    8

#define MAX_IPC_RECORD_CNT  8192

typedef struct
{
    char szFile[28];
    int	 nSize;
} IpcRecordFile;

typedef struct _IPCRECORDLIST
{
    unsigned int    filecount;
    IpcRecordFile   filename[MAX_IPC_RECORD_CNT];
}IpcRecordList;

typedef struct
{
    char szFile[28];
    int  offset;
}IpcRecordData;

typedef struct _TIMESCHDULE
{
    int		time[7][3];
} TIMESCHDULE, *PTIMESCHDULE;

typedef struct _RECORDPARAM
{
    char   			RecordOver;				//auto overlap
    char				RecordTimerEnable;		//record file 0->time 1->size
    char				RecordLock;				//record lock filename
    char				RecordSize;
    TIMESCHDULE   	RecordTimer;
} RECORDPARAM, *PRECORDPARAM;

typedef struct _PTZPARAM2
{
    char				PtzCenterEnable;
    char				PtzRate;
    char				PtzUpRate;
    char				PtzDownRate;

    char				PtzLeftRate;
    char				PtzRightRate;
    char				PtzLedMode;
    char				PtzDisPresent;

    char				PtzOnStart;
    char				PtzRunTimes;
    char				PtzSpeed;
    char				Other;
} PTZPARAM2, *PPTZPARAM2;

typedef struct _CAMERAINFO
{
	int	                bysize;
	int	                brightness;
	int	                contrast;
	int	                chroma;
	int	                saturation;
	int	                OSDEnable;
	int	                videoenv;
	int	                videomode;
	int	                byframerate;
	int	                byframeratesub;
}cameraInfo_t, *pCameraInfo_t;

//status param
typedef struct _STATUSPARAM2
{
    char            DeviceName[32];		//alias
    char       		AlarmStatus;	//0->no action 1->motion alarm 2->io alarm
    char       		DdnsStatus;		//0->no action 1->start... 2->login failed 3->login ok
    char       		PnpStatus;		//0->no action 1->start... 2->upnp failed  3->upnp is ok
    char       		P2pStatus;		//0->no action 1->start... 2->p2p failed   3->p2p is ok

    char			SdStatus;		//0->no sd    1->sd not fromat 2->bad sd(fat is error)
    //3->sd is ok 4->sd is record...
    //5->sd record is stop 6->sd record failed<write failed>
    char			WifiStatus;		//0->not action 1->connect ok 2->connect...  3->failed
    char			InternetStatus;	//0->not action 1->conenct ok  2->failed
    char			VideoStatus;		//0->video is ok  1->failed

    char			systemok;		//systemok
    char			devicetype;		//system type
    short               sdtotal;
    short               sdfree;
    char			other[2];		//other status
} STATUSPARAM2, *PSTATUSPARAM2;

typedef struct _VIDENCPARAM2
{
    char				Size;			//460X480->0  320x240->1 160x120->2 1280x720->3 640x360->4 5->1280x960 6->1920x1080
    char   			SizeSub;		//0->1/2 1->4/1
    char				FrameRate;		//main rate framerate
    char				FrameRateSub;	//sub  rate 1/1 1/2 1/3 4/1 1/5 1/6 1/7 1/8
    char				KeyFrame;		//main rate keyframe
    char   			KeyFrameSub;	//sub  rate keyframe
    char   			Quant;			//main rate quant
    char   			QuantSub;		//sub  rate quant
    char   			RateMode;		//main rate 0->vbr 1->cbr
    char   			RateModeSub;	//sub  rate 0->vbr 1->cbr
    unsigned short  	BitRate;			//main rate bitrate  kbps
    unsigned short	BitRateSub;		//sub  rate bitrate  kbps
    char				VideoMode;		//0->normal 1->mirr 2->flip 3->mirr and flip
    char				VideoEnv;		//0->50hz 1->60hz 2->outdoor 3->night mode
    char				OsdEnable;		//OSDEnable
    unsigned char   	Birghtness;
    unsigned char   	Chroma;
    unsigned char   	Saturation;
    unsigned char   	Contrast;
    char				IrCut;
    char				other[2];
} VIDENCPARAM2, *PVIDENCPARAM2;

typedef struct _SYSTEMCONFIG
{
    //char            	DeviceId[32];		//deviceid
    char            DeviceId[24];		//deviceid
	char			ApiLisense[8];
    int			        SysVersion;			//system version
    int			        AppVersion;			//app version
    char            	EthMac[20];			//ether mac
    char         	WifiMac[20];			//wifi mac
    char         	ApMac[20];			//wifi mac
    char			SysMode;			//0->ipcam 1->Ap camera 2->wifi self speaker 3->wifi speaker 4->control
    char			Factory;				//factory
    short		pnpport;			//p2p port
    char			pnpserver[256];		//p2p server
    char			pnpuser[64];			//p2p user
    char			pnppasswd[64];		//p2p passwd
} SYSTEMCONFIG, *PSYSTEMCONFIG;
#endif



