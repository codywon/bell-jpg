#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define DEV_TYPE_DVS		0
#define DEV_TYPE_DVR		1
#define DEV_TYPE_CLIENT		2
#define DEV_TYPE_STREAM_SERVER	3
#define DEV_TYPE_RECORD_SERVER	4

#define VIDEO_TYPE_MPEG4	0
#define VIDEO_TYPE_H264		1

#define AUDIO_TYPE_MP3		0

#define MAX_PACKET_LENGTH	1024

#define ERR_SUCCESS		0
#define STARTCODE					0xa815aa55

typedef struct _stLiveHead
{
    unsigned int   		startcode;	//  0xa815aa55
    char				type;
    char  			streamid;
    unsigned short 	militime;
    unsigned int 		sectime;
    unsigned int    	frameno;
    unsigned int 		len;
    unsigned char		version;
    unsigned char		size;
    unsigned char		sessid;
    unsigned char		framerate;

	/* BEGIN: Added by wupm, 2013/5/29 */
    //unsigned char		other1[8];
	int				adpcmindex;
	int				adpcmsample;
} LIVEHEAD, *PLIVEHEAD;

//network's command
#define CMD_REGISTER_REQUEST					0x0101
typedef struct _stRegisterRequest
{
    unsigned int	dwDeviceID;

    unsigned char	byChanNum;
    unsigned char	byDevType;
    unsigned short	nReserved;

    char		szUserName[8];

    char		szPassword[8];

} REGISTERREQUEST, *PREGISTERREQUEST;

#define CMD_REGISTER_RESPONSE					0x8101
typedef struct _stCommonResponse
{
    unsigned short		nResult;
    unsigned short		nReserved;

} COMMONRESPONSE, *PCOMMONRESPONSE;

#define CMD_UNREGISTER_REQUEST					0x0102
//no parameter

#define CMD_UNREGISTER_RESPONSE					0x8102

#define CMD_HEART_BEAT_REQUEST					0x0103
//no parameter

#define CMD_HEART_BEAT_RESPONSE					0x8103
//no parameter

#define UPDATE_REQUEST						0x0104
typedef struct _stUpdateRequest
{
    unsigned char		byFlag;
    unsigned char		byReserved;
    unsigned short		nFtpPort;

    unsigned int		dwFtpIpAddr;

    char			szFileName[64];
} UPDATEREQUEST, *PUPDATEREQUEST;

#define UPDATE_RESPONSE						0x8104
//COMMONRESPONSE



#define STREAM_REQUEST						0x0201
typedef struct _stStreamRequest
{
    unsigned char		byType;		//request's type:
    //	0: main code(video)
    //	1: sub code(video)
    //	2: audio
    //	3: talk
    unsigned char		byChanID;
    unsigned char		byStatus;
    unsigned char 		byTransProto;	// 0/1: TCP/UDP

    unsigned int		dwDeviceID;

    unsigned short		nReserved;
    unsigned short		nMediaPort;

    unsigned int		dwMediaIpAddr;

} STREAMREQUEST, *PSTREAMREQUEST;

#define STREAM_RESPONSE						0x8201
//common's response info.

#define STREAM_NOTIFY						0x0202
typedef struct _stStreamNofity
{
    unsigned int	dwDeviceID;

} STREAMNOTIFY, *PSTREAMNOTIFY;

#define KEY_FRAME_REQUEST					0x0203
typedef struct _stKeyFrameRequest
{
    unsigned char	byChanID;
    unsigned char	byReserved[3];

} KEYFRAMEREQUEST, *PKEYFRAMEREQUEST;

#define QUERY_REC_FILE_REQUEST					0x0204
typedef struct _stQueryRecFileRequest
{
    unsigned char	byChanID;
    unsigned char	byQueryType;
    unsigned char	byOperType;
    unsigned char	byReserved;

    unsigned int	dwStartTime;

    unsigned int	dwStopTime;

} QUERYRECFILEREQUEST, *PQUERYRECFILEREQUEST;

#define QUERY_REC_FILE_RESPONSE					0x8204
//	nRecCount	unsigned short;
//	nTotalCount	unsigned short;
//	stRecInfo	RECINFO * nRecCount;
typedef struct _stRecInfo
{
#if 0
    char		szRecFileName[24];
#else
    unsigned char	byChanID;
    unsigned char	byRecType;
    unsigned short	nRecTime;

    unsigned int	dwBeginTime;

    unsigned int	dwFileIndex;
#endif
} RECINFO, *PRECINFO;

#define FILE_INFO_REQUEST					0x0205
//RECINFO

#define FILE_INFO_RESPONSE					0x8205
typedef struct _stFileInfoResponse
{
    unsigned char	byVideoType;
    unsigned char	byVideoMode;
    unsigned short	nReserved;

    unsigned int	dwStartTime;

    unsigned int	dwStopTime;
} FILEINFORESPONSE, *PFILEINFORESPONSE;

#define FILE_STREAM_REQUEST					0x0206
typedef struct _stFileStreamRequest
{
    RECINFO		stRecFile;

    unsigned int	dwMediaIpAddr;

    unsigned short	nMediaPort;
    unsigned char	byIsStart;
    unsigned char	byTransProto;

} FILESTREAMREQUEST, *PFILESTREAMREQUEST;

#define FILE_STREAM_RESPONSE					0x8206
//COMMONRESPONSE

#define FILE_STREAM_SCALE_REQUEST				0x0207
//no parameter

#define FILE_STREAM_SCALE_RESPONSE				0x8207
typedef struct _stFileStreamScaleResponse
{
    unsigned int	dwCurFrameNo;

    unsigned int	dwStreamSize;

} FILESTREAMRESPONSE, *PFILESTREAMRESPONSE;

#define FILE_STREAM_CTRL_REQUEST				0x0208
typedef struct _stFileStreamCtrlRequest
{
    unsigned char	byOperType;
    unsigned char	byReserved[3];

    unsigned int	dwValue;

} FILESTREAMCTRLREQUEST, *PFILESTREAMCTRLREQUEST;

#define FILE_STREAM_CTRL_RESPONSE				0x8208
//COMMONRESPONSE




#define SEND_VIDEO_DATA						0x0301
typedef struct _stVideoDataHead
{
    unsigned char		byVideoType;
    unsigned char		byChannelID;
    unsigned char		byVideoMode;
    unsigned char		byReserved;

    unsigned char		byFrameType;
    unsigned char		byIsAckFlag;
    unsigned char		byPackIndex;
    unsigned char		byPackCount;

    unsigned int		dwFrameID;

    unsigned int		dwFrameLength;

    unsigned int		dwSecTimeStamp;

    unsigned short		nMiliTimeStamp;
    unsigned short		nDataLength;

} VIDEODATAHEAD, *PVIDEODATAHEAD;

#define SEND_AUDIO_DATA						0x0302
typedef struct _stAudioDataHead
{
    unsigned char		byAudioType;
    unsigned char		byChannelID;
    unsigned char		byIsAckFlag;
    unsigned char		byPackIndex;

    unsigned char		byPackCount;
    unsigned char		byDataType;
    unsigned short		nReserved;

    unsigned int		dwFrameID;

    unsigned int		dwFrameLength;

    unsigned int		dwSecTimeStamp;

    unsigned short		nMiliTimeStamp;
    unsigned short		nDataLength;

} AUDIODATAHEAD, *PAUDIODATAHEAD;



#define PTZ_CTRL_REQUEST					0x0401
typedef struct _stPTZRequest
{
    unsigned char	byCommand;
    unsigned char	bySpeed0;
    unsigned char	bySpeed1;
    unsigned char	byReserved;

} PTZREQUEST, *PPTZREQUEST;
#if 0
typedef enum _ePtzCtrlCmd
{
    PTZ_STOP = 0,
    PTZ_UP,
    PTZ_DOWN,
    PTZ_LEFT,
    PTZ_LEFT_UP,
    PTZ_LEFT_DOWN,
    PTZ_RIGHT,
    PTZ_RIGHT_UP,
    PTZ_RIGHT_DOWN,
    PTZ_AUTO,
    PTZ_PREFAB_BIT_SET,
    PTZ_PREFAB_BIT_DEL,
    PTZ_PREFAB_BIT_RUN,
    PTZ_MODE_SET_START,
    PTZ_MODE_SET_STOP,
    PTZ_MODE_SET_RUN,
    PTZ_MENU_OPEN,
    PTZ_MENU_EXIT,
    PTZ_MENU_ENTER,
    PTZ_FLIP,
    PTZ_START,
    PTZ_CRUISE_RUN,
    LENS_APERTURE_OPEN,
    LENS_APERTURE_CLOSE,
    LENS_ZOOM_IN,
    LENS_ZOOM_OUT,
    LENS_FOCAL_NEAR,
    LENS_FOCAL_FAR,
    AUX_GO,
    AUX_STOP,
} E_PTZ_CTRL_COMMAND;
#endif
#define PTZ_CTRL_RESPONSE					0x8401
//COMMONRESPONSE

#define LOG_QUERY_REQUEST					0x0402
typedef struct _stLogQueryRequest
{
    unsigned int	dwDeviceID;

    unsigned char	byChanID;
    unsigned char	byQueryType;
    unsigned char	byLogType;
    unsigned char	byReserved0;

    unsigned short	nBeginYear;
    unsigned char	byBeginMon;
    unsigned char	byBeginDay;

    unsigned char	byBeginHour;
    unsigned char	byBeginMin;
    unsigned char	byBeginSec;
    unsigned char	byReserved1;

    unsigned short	nEndYear;
    unsigned char	byEndMon;
    unsigned char	byEndDay;

    unsigned char	byEndHour;
    unsigned char	byEndMin;
    unsigned char	byEndSec;
    unsigned char	byReserved2;

} LOGQUERYREQUEST, *PLOGQUERYREQUEST;

#define LOG_QUERY_RESPONSE					0x8402
// nTotalRecords	unsigned short;
// nRecCoutOfPack	unsigned short;
typedef struct _stLogInfo
{
    unsigned int	dwDeviceID;

    unsigned char	byChanID;
    unsigned char	byReserved[3];

    unsigned short	nYear;
    unsigned char	byMon;
    unsigned char	byDay;

    unsigned char	byHour;
    unsigned char	byMin;
    unsigned char	bySec;
    unsigned char	byLogType;

    unsigned int	dwIpAddr;

    unsigned short	nCommand;
    unsigned short	nStatus;

} LOGINFO, *PLOGINFO;

#define ALARM_NOTIFY						0x0403
//LOGINFO

#define DEV_REBOOT_REQUEST					0x0404
//no parameter

#define DEV_REBOOT_RESPONSE					0x8404
//no parameter

#define RECORD_REQUEST						0x0405
typedef struct _stRecordRequest
{
    unsigned char	byChanID;
    unsigned char	byStatus;
    unsigned short	nReserved;

} RECORDREQUEST, *PRECORDREQUEST;

#define RECORD_RESPONSE						0x8405
//COMMONRESPONSE

#define SEND_TRANS_DATA_REQUEST					0x0406
typedef struct _stTransDataHead
{
    unsigned short	nTransDataLen;
    unsigned short	nComNum;

} TRANSDATAHEAD, *PTRANSDATAHEAD;
//	szTransData	char[nTransDataLen];

#define SEND_TRANS_DATA_RESPONSE				0x8406
//COMMONRESPONSE

#define TRANS_DATA_NOTFIY					0x0407

#if 0
#define CAPTURE_REQUEST						0x0407
typedef struct _stCaptureRequest
{
    unsigned char	byChanID;
    unsigned char	byPicFormat;
    unsigned char	byCapCount;
    unsigned char	byReserved;

} CAPTUREREQUEST, *PCAPTUREREQUEST;

#define CAPTURE_RESPONSE					0x8407
//COMMONRESPONSE
#endif


#define GET_BASE_PARAM_REQUEST					0x0501
//no parameter

#define GET_BASE_PARAM_RESPONSE					0x8501
//	nResult		unsigned short;
//	nReserved	unsigned short;
//	stBaseParam	BASEPARAM

#define SET_BASE_PARAM_REQUEST					0x0502
//	stBaseParam	BASEPARAM

#define SET_BASE_PARAM_RESPONSE					0x8502
//COMMONRESPONSE
#define GET_AUDIO_PARAM_REQUEST					0x0505
typedef struct _stGetAudioParamRequest
{
    unsigned char	byChanID;
    unsigned char	byReserved[3];

} GETAUDIOPARAMREQUEST, *PGETAUDIOPARAMREQUEST;

#define GET_AUDIO_PARAM_RESPONSE				0x8505
//	nResult		unsigned short;
//	nReserved	unsigned short;
//	stAudioParam	AUDIOPARAM

#define SET_AUDIO_PARAM_REQUEST					0x0506
//    byChanID		unsigned char;
//    byReserved	unsigned char[3];
//	stAudioParam	AUDIOPARAM

#define SET_AUDIO_PARAM_RESPONSE				0x8506
//COMMONRESPONSE

#define GET_RS_PARAM_REQUEST					0x0507
typedef struct _stGetRSParamRequest
{
    unsigned char	byIsRS485;
    unsigned char	byReserved[3];

} GETRSPARAMREQUEST, *PGETRSPARAMREQUEST;

#define GET_RS_PARAM_RESPONSE					0x8507
//	nResult		unsigned short;
//	byIsRS485	unsigned char;
//	byReserved	unsigned char;
//	stRsParam	RSPARAM

#define SET_RS_PARAM_REQUEST					0x0508
//	byIsRS485	unsigned char;
//	byReserved	unsigned char[3];
//	stRsParam	RSPARAM

#define SET_RS_PARAM_RESPONSE					0x8508
//COMMONRESPONSE

#define GET_MOTION_PARAM_REQUEST				0x0509
typedef struct _stGetMotionParamRequest
{
    unsigned char	byChanID;
    unsigned char	byReserved[3];

} GETMOTIONPARAMREQUEST, *PGETMOTIONPARAMREQUEST;

#define GET_MOTION_PARAM_RESPONSE				0x8509
//	nResult		unsigned short;
//	nReserved	unsigned short;
//	stMotionParam	MOTIONPARAM

#define SET_MOTION_PARAM_REQUEST				0x050A
//	byChanID	unsigned char;
//	byReserved	unsigned char[3];
//	stMotionParam	MOTIONPARAM

#define SET_MOTION_PARAM_RESPONSE				0x850A
//COMMONRESPONSE

#define GET_SENSOR_PARAM_REQUEST				0x050B
typedef struct _stGetSensorParamRequest
{
    unsigned char	byChanID;
    unsigned char	byReserved[3];

} GETSENSORPARAMREQUEST, *PGETSENSORPARAMREQUEST;

#define GET_SENSOR_PARAM_RESPONSE				0x850B
//	nResult		unsigned short;
//	nReserved	unsigned short;
//	stSensorParam	SENSORPARAM

#define SET_SENSOR_PARAM_REQUEST				0x050C
//	byChanID	unsigned char;
//	byReserved	unsigned char[3];
//	stSensorParam	SENSORPARAM

#define SET_SENSOR_PARAM_RESPONSE				0x850C
//COMMONRESPONSE

#define GET_MASK_PARAM_REQUEST					0x050D
typedef struct _stGetMaskParamRequest
{
    unsigned char	byChanID;
    unsigned char	byReserved[3];

} GETMASKPARAMREQUEST, *PGETMASKPARAMREQUEST;

#define GET_MASK_PARAM_RESPONSE					0x850D
//	nResult		unsigned short;
//	nReserved	unsigned short;
//	stMotionParam	MOTIONPARAM

#define SET_MASK_PARAM_REQUEST					0x050E
//	byChanID	unsigned char;
//	byReserved	unsigned char[3];
//	stMotionParam	MOTIONPARAM

#define SET_MASK_PARAM_RESPONSE					0x850E
//COMMONRESPONSE

#define GET_VLOSS_PARAM_REQUEST					0x050F
typedef struct _stGetVLossParamRequest
{
    unsigned char	byChanID;
    unsigned char	byReserved[3];

} GETVLOSSPARAMREQUEST, *PGETVLOSSPARAMREQUEST;

#define GET_VLOSS_PARAM_RESPONSE				0x850F
//	nResult		unsigned short;
//	nReserved	unsigned short;
//	stVLossParam	VLOSSPARAM

#define SET_VLOSS_PARAM_REQUEST					0x0510
//	byChanID	unsigned char;
//	byReserved	unsigned char[3];
//	stVLossParam	VLOSSPARAM

#define SET_VLOSS_PARAM_RESPONSE				0x8510
//COMMONRESPONSE

#define GET_RECORD_PARAM_REQUEST				0x0511
typedef struct _stGetRecordParamRequest
{
    unsigned char	byChanID;
    unsigned char	byReserved[3];

} GETRECORDPARAMREQUEST, *PGETRECORDPARAMREQUEST;

#define GET_RECORD_PARAM_RESPONSE				0x8511
//	nResult		unsigned short;
//	nReserved	unsigned short;
//	stRecordParam	RECORDPARAM

#define SET_RECORD_PARAM_REQUEST				0x0512
//	byChanID	unsigned char;
//	byReserved	unsigned char[3];
//	stRecordParam	RECORDPARAM

#define SET_RECORD_PARAM_RESPONSE				0x8512
//COMMONRESPONSE

#define GET_NET_PARAM_REQUEST					0x0513
//no parameter

#define GET_NET_PARAM_RESPONSE					0x8513
// stNetParam		NETWORKPARAM

#define SET_NET_PARAM_REQUEST					0x0514
// stNetParam		NETWORKPARAM

#define SET_NET_PARAM_RESPONSE					0x8514
//COMMONRESPONSE

#define GET_ALL_PARAM_REQUEST					0x0515

#define GET_ALL_PARAM_RESPONSE					0x8515

#define GET_SINGLE_CHN_PARAM_REQUEST				0x0516

#define GET_SINGLE_CHN_PARAM_RESPONSE				0x8516

#define SET_GPIO_STATUS						0x060a
#define SET_GPIO_STATUS_RESPONSE				0x860a
#define SET_ALARM_NOTE						0x8560
#define SET_ALARM_NOTE1						0x8561
#define SET_ALARM_NOTE2						0x8562

#define GET_MULT_USER_REQUEST					0x060b
#define GET_MULT_USER_RESPONSE                                  0x860b

#define SET_BRIGHT_PARAM					0x060c
#define GET_BRIGHT_PARAM					0x860c

#define SET_CONTRAST_PARAM					0x060d
#define GET_CONTRAST_PARAM					0x860d

#define SET_FRAMERATE_PARAM                                     0x060e
#define GET_FRAMERATE_PARAM                                     0x860e

#define SET_HZ5060_PARAM                                     	0x060f
#define GET_HZ5060_PARAM                                     	0x860f

#define SET_ROTATION_VIDEO					0x0610
#define GET_ROTATION_VIDEO					0x8610

#define SET_VIDEO_SIZE						0x0611
#define GET_VIDEO_SIZE						0x8611
#define SET_OSD_ENABLE						0x0612
#define GET_OSD_ENABLE						0x8612

typedef struct _stCmdVideoParam
{
    unsigned char byBrightness;
    unsigned char byContast;
    unsigned char byMirror;
    unsigned char byOsd;

    unsigned char byFrameRate;
    unsigned char byFrequency;
    unsigned char byReserved[2];
} CMDVIDEOPARAM, *PCMDVIDEOPARAM;

typedef struct _stCmdEncParam
{
    unsigned char byResolution;
    unsigned char byKeyFrame;
    unsigned short nBitRate;
    unsigned char byQuality;
    unsigned char byReserved[3];
} CMDENCPARAM, *PCMDENCPARAM;
typedef struct _stRequestHead
{
    unsigned char byChanID;
    unsigned char byReserved[3];
} RETQUESTHEAD, *PRETQUESTHEAD;

//COMMONRESPONSE
//stVideoParam VIDEOPARAM;
//stReqHead RETQUESTHEAD;
//stVideoParam VIDEOPARAM;
//COMMONRESPONSE
//RETQUESTHEAD
//COMMONRESPONSE
//stEncParam ENCPARAM;
//stReqHead RETQUESTHEAD;
//stEncParam ENCPARAM;

#define GET_VIDEO_PARAM_REQUEST 	0x0521
#define GET_VIDEO_PARAM_RESPONSE 	0x8521
#define SET_VIDEO_PARAM_REQUEST 	0x0522
#define SET_VIDEO_PARAM_RESPONSE 	0x8522
#define GET_ENC_PARAM_REQUEST 		0x0523
#define GET_ENC_PARAM_RESPONSE 		0x8523
#define SET_ENC_PARAM_REQUEST 		0x0524
#define SET_ENC_PARAM_RESPONSE          0x8524

#endif //__PROTOCOL_H__
