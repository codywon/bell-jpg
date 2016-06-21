
#ifndef _PARAM_H__
#define __PARAM_H__

#define LISTEN_NUM      10


#define MAX_CMD_LENGTH      1024
#define MAX_FRAME_LENGTH    ( 512 * 1024 )
#define MAX_AUDIO_LENGTH    ( 4 * 1024 )


#define PORT_CMD_IPCAM      10000  //object ipcam use
#define PORT_CMD_NVR        10001  //nvr client use
#define PORT_VIDEO          10002  //TCP port
#define PORT_AUDIO          10003  //TCP port
#define PORT_RECORD         10004  //TCP port

#define START_CODE          0xAABB

#define TYPE_HEART          0x11    //heart beat message
#define TYPE_PLAY_END       0x12	//one record file end

#define SUBTYPE_GET         0x100
#define SUBTYPE_GET_RESP    0x101
#define SUBTYPE_SET         0x102
#define SUBTYPE_SET_RESP    0x103

typedef enum
{
    eDoorBell_Call      = 0x00000001,
    eDoorBell_Motion    = 0x00000002,
    eDoorBell_PIR       = 0x00000004,
    eDoorBell_GPIO      = 0x00000008,
    eDoorBell_Button    = 0x00000010
}stDoorBellStatus;

typedef struct _IPCPARAM
{
    short   startcode;      //START_CODE
    short   cmd;            //IPCPROTOCOL
    int     subtype;
    int     len;
    char    reserved[64];
} IPCPARAM, *PIPCPARAM;

typedef enum _eIpcProtocol
{
    //base of typedef enum _eInterfaceType  in sysparam.h
    IPC_DTIMEPARAM=0x1000,      //eDTimeParam,            //DTIMEPARAM
    IPC_NETPARAM,               //eNetParam,              //NETWORKPARAM
    IPC_USERPARAM,              //eUserParam,             //USERPARAM
    IPC_MULTDEVICE,             //eMultDevice,            //MULTDEVICE
    IPC_WIFIPARAM,              //eWifiParam,             //WIFIPARAM
    IPC_PPPOEPARAM,             //ePppoeParam,            //PPPOEPARAM
    IPC_UPNPPARAM,              //eUpnpParam,             //UPNPPARAM
    IPC_DDNSPARAM,              //eDdnsParam,             //DDNSPARAM
    IPC_MAILPARAM,              //eMailParam,             //MAILPARAM
    IPC_FTPPARAM,               //eFtpParam,              //FTPPARAM
    IPC_ALARMPARAM,             //eAlarmParam,            //ALARMPARAM
    IPC_RECORDPARAM,            //eRecordParam,           //RECORDPARAM
    IPC_DEVICENAME,             //eDeviceName,            //char*
    IPC_THIRDDDNS,              //eThirdDdns,             //DDNSPARAM
    IPC_PTZPARAM,               //ePtzParam,              //PTZPARAM
    IPC_WIFISCANRESULT,         //eWifiScanResult,        //WifiCont
    IPC_CAMERAINFO,             //eCameraInfo,            //cameraInfo_t
    IPC_STATUSPARAM,            //eStatusParam,           //STATUSPARAM
    IPC_SYSVERSION,             //eSysVersion,            //char*
    IPC_ETHMAC,                 //eEthMac,                //char*
    IPC_WIFIMAC,                //eWifiMac,               //char*
    IPC_DEVICEID,               //eDeviceID,              //char*
    IPC_VIDENCPARAM,            //eVidencParam,           //VIDENCPARAM
    IPC_MOTOSPEED,              //eMotoSpeed,             //char*
    IPC_SYSCONFIG,              //eSysConfig,             //SYSTEMCONFIG
    IPC_RECORDAUDIO,            //eRecordAudio,           //int*
    IPC_RECORDAUDIOLEVEL,       //eRecordAudioLevel,      //int*

    //base of typedef enum _eControlType, in sysparam.h
    IPC_IRCUTSWITCH,            //eIRCutSwitch,               //0-close; 1-open
    IPC_ALARMLOG,               //eAlarmLog                 //NULL
    IPC_CLEARLOG,               //eClearLog,                      //Null
    IPC_APPDEFAULT,             //eAppDefault,            //Null
    IPC_REBOOT,                 //eReboot,                    //Null
    IPC_RESTORE,                //eRestore,               //Null
    IPC_WIFISCAN,               //eWifiScan,              //Null
    IPC_SDFORMAT,               //eSdFormat,              //Null
    IPC_AUDIOCTRL,              //eAudioCtrl,         //0-Stop Audio Capture; 1-Start Audio Capture
    IPC_MAILTEST,               //eMailTest,          //NULL
    IPC_FTPTEST,                //eFtpTest,           //NULL

    //base of typedef enum _eCameraCmd
    IPC_RESOLUTION,             //videoProperty_Resolustion = 0,     //0->VGA, 1->QVGA, 3->720P
    IPC_BRIGHTNESS,             //videoProperty_brightness=1,        //0-255
    IPC_CONTRAST,               //videoProperty_contrast=2,          //0-255
    IPC_MODE,                   //videoProperty_mode=3,               //50HZ 60HZ
    IPC_MIRRORFLIP,             //videoProperty_mirrorFlip=5,     //0-normal, 1-flip, 2-mirror, 3-mirror-flip
    IPC_FRAMERATE,              //videoProperty_frameRate=6,          //frame rate 0-30
    IPC_DEFAULT,                //videoProperty_default=7,            //no param
    IPC_SATURATION,             //videoProperty_satuation=8,      //0~255
    IPC_HUE,                    //videoProperty_hue=9,                //0~255
    IPC_OSD,                    //videoProperty_osd=10,               //0/1
    IPC_BITRATE,                //videoProperty_bitRate=13,           //set bit rate
    IPC_IRCUT,                  //videoProperty_ircut=14,             //0/1
    IPC_QUANT,                  //videoProperty_quant=16,             //quant 0-5
    IPC_PTZ,                    //videoProperty_ptzRate=100           //ptz speed

    //moto control
    IPC_MOTO_CTRL,              //                                      //int
    IPC_GET_RECORD_LIST,        //                                      //IpcRecordList
    IPC_GET_RECORD_FILE,        //                                      //char* the data of one frame of video/audio
    IPC_ALARM_FOUND,            //                                      //sizeof(int), alarm push
    IPC_EXTRA_PARAM,            //eExtraParams,                         //EXTRA_PARAMS
    IPC_RECORD_OK,              //                                      //the fullpath of file
    IPC_GET_DOORBELL,           //stDoorBellStatus
    IPC_UNLOCK                  //
}IPCPROTOCOL;

#endif

