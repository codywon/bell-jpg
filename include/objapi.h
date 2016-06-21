
#ifndef _OBJ_API_H_
#define _OBJ_API_H_

#include "param.h"
#include "../func/download.h"


typedef Int32 (*CmdCallback)(IPCPROTOCOL cmd, Int32 subtype, Int8* buffer, UInt32 len, UInt32 nvripaddr);
typedef Int32 (*MediaCallback)(Int8* buffer, UInt32 len);

Int32 CreateCmdSocket( UInt32 localipaddr, UInt16 localport);
Int32 CreateCmdProc();
Int32 RegisterCmdCallback(CmdCallback callback);
Int32 UnregisterCmdCallback(CmdCallback callback);

Int32 GetIpcParamResp( UInt32 remoteipaddr, UInt16 remoteport, IPCPROTOCOL cmd, Int8* data, UInt32 len );
Int32 SetIpcParamResp( UInt32 remoteipaddr, UInt16 remoteport, IPCPROTOCOL cmd);

/*
*Create VideoStream Tcp Server
*When success of client "connect", start video frame send
*ipaddr: in, device ip address
*port: in, the port of tcp server
*
*return 0-create thread fail, >0 success
*/
Int32 CreateLiveStreamProc( Int32 ipaddr, UInt16 port );

/*
*Create AudioStream Tcp Server
*When success of client "connect", start audio frame send
*ipaddr: in, device ip address
*port: in, the port of tcp server
*
*return 0-create thread fail, >0 success
*/
Int32 CreateAudioStreamProc( Int32 ipaddr, UInt16 port );

/*
*Create RecordStream Tcp Server
*When success of client "connect", start video/audio frame send
*ipaddr: in, device ip address
*port: in, the port of tcp server
*
*return 0-create thread fail, >0 success
*/
Int32 CreateRecordStreamProc( Int32 ipaddr, UInt16 port );

#endif

