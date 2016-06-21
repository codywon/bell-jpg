#ifndef __PROC_H__
#define __PROC_H__

#include "hd_common.h"
#include "sysdata.h"

#define MAX_HEART_BEAT_NUM	3

//video encoder stream's process
int VencStreamProc( int iEncChnID, unsigned char FrameType, unsigned int FrameNo, int length, unsigned char* pstStream );

//audio encoder stream's process
int AencStreamProc( int iEncChnID, /*AUDIO_STREAM_S*/unsigned char* pstStream );

//audio and video encoder stream's process
int AvencStreamProc( int iAVChanID, int iAencChanID, int iVencChanID, /*AVENC_LIST_S*/unsigned char* pVideoNode, /*AVENC_LIST_S*/unsigned char* pAudioNode );

//video motion's process
int MotionDetectResultProc( int iChnID, int iAlarmType );

//network's connect notify(command)
void NotifyConnected( int iSocket, const char* pszIpAddr, int iPort );

//network's disconnect notify(command)
void NotifyClose( int iSocket );

//network's receive message's process(command)
int RecvMessage( int iSocket, const char* pszMessage, int iMessageLen );

//network's command process(command)
int ProcMessage( int iSocket, unsigned short nType, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdRegisterRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdUnregisterRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdHeartBeatRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdUpdateRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdStreamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdQueryRecFileRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdFileInfoRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdFileStreamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdFileStreamScaleRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdFileStreamCtrlRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdPTZControl( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdLogQueryRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdRebootRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdRecordRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSendTransData( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdCaptureRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetBaseParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSetBaseParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetNetParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSetNetParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetVideoParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSetVideoParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetAudioParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSetAudioParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetRSParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSetRSParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetMotionParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSetMotionParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetSensorParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSetSensorParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetMaskParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSetMaskParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetVLossParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSetVLossParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetRecordParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSetRecordParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

int cmdGetAllParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int cmdSingleChanParamRequest( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

//network's connect notify(media stream)
void MediaNotifyConnected( int iSocket, const char* pszIpAddr, int iPort );

//network's disconnect notify(media stream)
void MediaNotifyClose( int iSocket );

//network's receive message's process(media stream)
int MediaRecvMessage( int iSocket, const char* pszMessage, int iMessageLen );

//network's command process(media stream)
int MediaProcMessage( int iSocket, unsigned short nType, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );
int mdaStreamNotify( int iSocket, unsigned short nSessionID, const char* pszMessage, unsigned short nMessageLen );

//timer check
int TimerDetect();

//osd process
void OsdProcess( unsigned int dwCurTime );

//set current time
void SetCurrentTime( unsigned int dwCurTime, int byzone );

//get current time
int GetCurrentTime();

//update video mask zone.
int UpdateMaskZone( int iChanID );

//update osd position.
int UpdateOsdPosition( int iChanID );

//check paramlist if or not change
int CheckParamList( int iUpdateIndex, PPARAMLIST pstNewParam, PPARAMLIST pstOldParam );

int CMD_ALARM_Send( char type, char value );
int LIVE_ALARM_Send( char type, char value );

void get_authfile( char* user, char* pass, char* auth );

#endif //__PROC_H__
