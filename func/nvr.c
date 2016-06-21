
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <errno.h>

#include "param.h"
#include "debug.h"
#include "protocol.h"
#include "vbuf.h"

#include "ipcparam.h"
#include "objapi.h"
#include "nvr.h"

extern char wificnt;

static UInt32 nvrAddress = 0;

void StartRecordSend(int sockuserfd)
{

}

void NvrAlarmSend(UInt32 type)
{
    if(nvrAddress != 0)
    {
        GetIpcParamResp( nvrAddress, PORT_CMD_NVR, IPC_GET_DOORBELL, ( char* )&type, sizeof( int ) );
    }
}

Int32 IpcCmdCallback( IPCPROTOCOL cmd, Int32 subtype, Int8* buffer, UInt32 len, UInt32 nvripaddr )
{
    //Textout( "From(0x%08x)...cmd=0x%04x, subtype=0x%04x, len=%d", nvripaddr, cmd, subtype, len );

    //Textout("IPC_MOTO_CTRL=0x%04x, SUBTYPE_SET=0x%04x", IPC_MOTO_CTRL, SUBTYPE_SET);
    nvrAddress = nvripaddr;
    switch ( cmd )
    {
        case IPC_DTIMEPARAM:            //eDTimeParam,            //DTIMEPARAM
            if ( subtype == SUBTYPE_GET )
            {
                //DTIMEPARAM param;
                //bzero( ( void* )&param, sizeof( DTIMEPARAM ) );
                //GetAppParam( eDTimeParam, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_DTIMEPARAM,
                                 ( char* )&bparam.stDTimeParam, sizeof( DTIMEPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eDTimeParam, ( void* )buffer, 0 );
                memcpy(&bparam.stDTimeParam, buffer, sizeof(DTIMEPARAM));
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_DTIMEPARAM );

            }

            break;

        case IPC_NETPARAM:               //eNetParam,              //NETWORKPARAM
            if ( subtype == SUBTYPE_GET )
            {
                //NETWORKPARAM param;
                //bzero( ( void* )&param, sizeof( NETWORKPARAM ) );
                //GetAppParam( eNetParam, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_NETPARAM,
                                 ( char* )&bparam.stNetParam, sizeof( NETPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eNetParam, ( void* )buffer, 0 );
                memcpy(&bparam.stNetParam, buffer, sizeof(NETPARAM));
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_NETPARAM );
            }

            break;

        case IPC_USERPARAM:             //eUserParam,             //USERPARAM

            if ( subtype == SUBTYPE_GET )
            {
                //USERPARAM param[MAX_USER];
                //bzero( ( void* )&param[0], sizeof( USERPARAM ) * MAX_USER );
                //GetAppParam( eUserParam, ( void* )&param[0], MAX_USER );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_USERPARAM,
                                 ( char* )&bparam.stUserParam[0], sizeof( USERPARAM ) * MAX_USER );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //len /= sizeof( USERPARAM );
                //Textout( "IPC_USERPARAM, len=%d", len );
                //SetAppParam( eUserParam, ( void* )buffer, len );
                memcpy(&bparam.stUserParam[0], buffer, sizeof(USERPARAM)*MAX_USER);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_USERPARAM );

            }

            break;

        case IPC_MULTDEVICE:             //eMultDevice,            //MULTDEVICE
            if ( subtype == SUBTYPE_GET )
            {
                //MULTDEVICE param[MAX_MULT_DEV];
                //bzero( ( void* )param, sizeof( MULTDEVICE ) * MAX_MULT_DEV );
                //GetAppParam( eMultDevice, ( void* )&param[0], MAX_MULT_DEV );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_MULTDEVICE,
                                 ( char* )&bparam.stMultDevice[0], sizeof( MULTDEVICE ) * MAX_MULT_DEV );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //len /= sizeof( MULTDEVICE );
                //Textout( "IPC_MULTDEVICE, len=%d", len );
                //SetAppParam( eMultDevice, ( void* )buffer, len );
                memcpy(&bparam.stMultDevice[0], buffer, sizeof(MULTDEVICE)*MAX_MULT_DEV);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_MULTDEVICE );
            }

            break;

        case IPC_WIFIPARAM:              //eWifiParam,             //WIFIPARAM
            if ( subtype == SUBTYPE_GET )
            {
                //WIFIPARAM param;
                //bzero( ( void* )&param, sizeof( WIFIPARAM ) );
                //GetAppParam( eWifiParam, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_WIFIPARAM,
                                 ( char* )&bparam.stWifiParam, sizeof( WIFIPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eWifiParam, ( void* )buffer, 0 );
                memcpy(&bparam.stWifiParam, buffer, sizeof(WIFIPARAM));
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_WIFIPARAM );
            }

            break;

        case IPC_PPPOEPARAM:             //ePppoeParam,            //PPPOEPARAM
            #if 0
            if ( subtype == SUBTYPE_GET )
            {
                PPPOEPARAM param;
                bzero( ( void* )&param, sizeof( PPPOEPARAM ) );
                GetAppParam( ePppoeParam, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_PPPOEPARAM,
                                 ( char* )&param, sizeof( PPPOEPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                SetAppParam( ePppoeParam, ( void* )buffer, 0 );
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_PPPOEPARAM );
            }
            #endif
            break;

        case IPC_UPNPPARAM:              //eUpnpParam,             //UPNPPARAM
            #if 0
            if ( subtype == SUBTYPE_GET )
            {
                UPNPPARAM param;
                bzero( ( void* )&param, sizeof( UPNPPARAM ) );
                GetAppParam( eUpnpParam, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_UPNPPARAM,
                                 ( char* )&param, sizeof( UPNPPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                SetAppParam( eUpnpParam, ( void* )buffer, 0 );
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_UPNPPARAM );
            }
            #endif
            break;

        case IPC_DDNSPARAM:              //eDdnsParam,             //DDNSPARAM
            if ( subtype == SUBTYPE_GET )
            {
                //DDNSPARAM param;
                //bzero( ( void* )&param, sizeof( DDNSPARAM ) );
                //GetAppParam( eDdnsParam, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_DDNSPARAM,
                                 ( char* )&bparam.stDdnsParam, sizeof( DDNSPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eDdnsParam, ( void* )buffer, 0 );
                memcpy(&bparam.stDdnsParam, buffer, sizeof(DDNSPARAM));
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_DDNSPARAM );
            }

            break;

        case IPC_MAILPARAM:              //eMailParam,             //MAILPARAM
            if ( subtype == SUBTYPE_GET )
            {
                //MAILPARAM param;
                //bzero( ( void* )&param, sizeof( MAILPARAM ) );
                //GetAppParam( eMailParam, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_MAILPARAM,
                                 ( char* )&bparam.stMailParam, sizeof( MAILPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eMailParam, ( void* )buffer, 0 );
                memcpy(&bparam.stMailParam, buffer, sizeof(MAILPARAM));
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_MAILPARAM );
            }

            break;

        case IPC_FTPPARAM:               //eFtpParam,              //FTPPARAM
            if ( subtype == SUBTYPE_GET )
            {
                //FTPPARAM param;
                //bzero( ( void* )&param, sizeof( FTPPARAM ) );
                //GetAppParam( eFtpParam, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_FTPPARAM,
                                 ( char* )&bparam.stFtpParam, sizeof( FTPPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eFtpParam, ( void* )buffer, 0 );
                memcpy(&bparam.stFtpParam, buffer, sizeof(FTPPARAM));
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_FTPPARAM );
            }

            break;

        case IPC_ALARMPARAM:             //eAlarmParam,            //ALARMPARAM
            if ( subtype == SUBTYPE_GET )
            {
                //ALARMPARAM param;
                //bzero( ( void* )&param, sizeof( ALARMPARAM ) );
                //GetAppParam( eAlarmParam, ( void* )&param, 0 );
                
                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_ALARMPARAM,
                                 ( char* )&bparam.stAlarmParam, sizeof( ALARMPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eAlarmParam, ( void* )buffer, 0 );
                memset(&bparam.stAlarmParam, buffer, sizeof(ALARMPARAM));
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_ALARMPARAM );
            }

            break;

        case IPC_RECORDPARAM:            //eRecordParam,           //RECORDPARAM
            if ( subtype == SUBTYPE_GET )
            {
                RECORDPARAM param;
                bzero( ( void* )&param, sizeof( RECORDPARAM ) );
                //GetAppParam( eRecordParam, ( void* )&param, 0 );
                param.RecordOver = bparam.stRecordSet.recordover;
                param.RecordTimerEnable = bparam.stRecordSet.timerenable;
                param.RecordSize = bparam.stRecordSet.timer;
                memcpy(&param.RecordTimer, &bparam.stRecordSet.timerecord, sizeof(TIMESCHDULE));

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_RECORDPARAM,
                                 ( char* )&param, sizeof( RECORDPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eRecordParam, ( void* )buffer, 0 );
                PRECORDPARAM pparam = (PRECORDPARAM)buffer;
                bparam.stRecordSet.recordover = pparam->RecordOver;
                bparam.stRecordSet.timerenable = pparam->RecordTimerEnable;
                bparam.stRecordSet.timer = pparam->RecordSize;
                memcpy(&bparam.stRecordSet.timerecord, &pparam->RecordTimer, sizeof(TIMESCHDULE));

                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_RECORDPARAM );
            }

            break;

        case IPC_DEVICENAME:             //eDeviceName,            //char*
            if ( subtype == SUBTYPE_GET )
            {
            }
            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eDeviceName, ( void* )buffer, 0 );
                if(strlen(buffer)<80)
                {
                    memset(bparam.stIEBaseParam.szDevName, 0, 80);
                    strcpy(bparam.stIEBaseParam.szDevName, buffer);
                }
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_DEVICENAME );
            }

            break;

        case IPC_THIRDDDNS:              //eThirdDdns,             //DDNSPARAM
            if ( subtype == SUBTYPE_GET )
            {
                //DDNSPARAM param;
                //bzero( ( void* )&param, sizeof( DDNSPARAM ) );
                //GetAppParam( eThirdDdns, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_DDNSPARAM,
                                 ( char* )&bparam.stDdnsParam, sizeof( DDNSPARAM ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eThirdDdns, ( void* )buffer, 0 );
                memcpy(&bparam.stDdnsParam, buffer, sizeof(DDNSPARAM));
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_THIRDDDNS );
            }

            break;

        case IPC_PTZPARAM:               //ePtzParam,              //PTZPARAM
            if ( subtype == SUBTYPE_GET )
            {
                PTZPARAM2 param;
                bzero( ( void* )&param, sizeof( PTZPARAM2 ) );
                //GetAppParam( ePtzParam, ( void* )&param, 0 );

                param.PtzCenterEnable = bparam.stPTZParam.byCenterEnable;
                param.PtzRate = bparam.stPTZParam.byRate;
                param.PtzUpRate = bparam.stPTZParam.byUpRate;
                param.PtzDownRate = bparam.stPTZParam.byDownRate;
                param.PtzLeftRate = bparam.stPTZParam.byLeftRate;
                param.PtzRightRate = bparam.stPTZParam.byRightRate;
                param.PtzLedMode = bparam.stPTZParam.byLedMode;
                param.PtzDisPresent = bparam.stPTZParam.byDisPresent;
                param.PtzOnStart = bparam.stPTZParam.byOnStart;
                param.PtzRunTimes = bparam.stPTZParam.byRunTimes;
                param.PtzSpeed = bparam.stPTZParam.byRate;

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_PTZPARAM,
                                 ( char* )&param, sizeof( PTZPARAM2 ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( ePtzParam, ( void* )buffer, 0 );
                PPTZPARAM2 pparam = (PPTZPARAM2)buffer;

                bparam.stPTZParam.byCenterEnable=pparam->PtzCenterEnable;
                bparam.stPTZParam.byRate=pparam->PtzRate;
                bparam.stPTZParam.byUpRate=pparam->PtzUpRate;
                bparam.stPTZParam.byDownRate=pparam->PtzDownRate;
                bparam.stPTZParam.byLeftRate=pparam->PtzLeftRate;
                bparam.stPTZParam.byRightRate=pparam->PtzRightRate;
                bparam.stPTZParam.byLedMode=pparam->PtzLedMode;
                bparam.stPTZParam.byDisPresent=pparam->PtzDisPresent;
                bparam.stPTZParam.byOnStart=pparam->PtzOnStart;
                bparam.stPTZParam.byRunTimes=pparam->PtzRunTimes;
                bparam.stPTZParam.byRate=pparam->PtzSpeed;       
                
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_PTZPARAM );
            }

            break;

        case IPC_WIFISCANRESULT:         //eWifiScanResult,        //WifiCont
            if ( subtype == SUBTYPE_GET )
            {
                //WifiCont param[MAX_WIFI_CNT];

                //bzero( ( void* )&param[0], sizeof( WifiCont ) * MAX_WIFI_CNT );
                //GetAppParam( eWifiScanResult, ( void* )&param[0], wificnt );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_WIFISCANRESULT,
                                 ( char* )&WifiResult[0], sizeof( WifiCont ) * wificnt );
            }

            else if ( subtype == SUBTYPE_SET )
            {
            }

            break;

        case IPC_CAMERAINFO:             //eCameraInfo,            //cameraInfo_t
            if ( subtype == SUBTYPE_GET )
            {
                cameraInfo_t param;
                bzero( ( void* )&param, sizeof( cameraInfo_t ) );
                //GetAppParam( eCameraInfo, ( void* )&param, 0 );

                param.bysize = bparam.stVencParam.bysize;
                param.brightness = bparam.stVencParam.brightness;
                param.contrast = bparam.stVencParam.contrast;
                param.chroma = bparam.stVencParam.chroma;
                param.saturation = bparam.stVencParam.saturation;
                param.OSDEnable = bparam.stVencParam.OSDEnable;
                param.videoenv = bparam.stVencParam.videoenv;
                param.videomode = bparam.stVencParam.videomode;
                param.byframerate = bparam.stVencParam.byframerate;
                param.byframeratesub = bparam.stVencParam.byframerate;
                
                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_CAMERAINFO,
                                 ( char* )&param, sizeof( cameraInfo_t ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
            }

            break;

        case IPC_STATUSPARAM:            //eStatusParam,           //STATUSPARAM
            if ( subtype == SUBTYPE_GET )
            {
                STATUSPARAM2 param;
                bzero( ( void* )&param, sizeof( STATUSPARAM2 ) );
                //GetAppParam( eStatusParam, ( void* )&param, 0 );

                memcpy(param.DeviceName, bparam.stIEBaseParam.szDevName, 32);
                param.DeviceName[31]=0;
                param.AlarmStatus = bparam.stStatusParam.warnstat;
                param.DdnsStatus = bparam.stStatusParam.ddnsstat;
                param.PnpStatus = bparam.stStatusParam.upnpstat;
                param.P2pStatus = bparam.stStatusParam.p2pstat;
                param.SdStatus = bparam.stStatusParam.sdstatus;
                param.WifiStatus = externwifistatus;
                param.InternetStatus = netok;
                param.VideoStatus = 0;
                param.systemok = 1;
                param.devicetype = 1;
                param.sdtotal = sdtotal;
                param.sdfree = sdfree;

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_STATUSPARAM,
                                 ( char* )&param, sizeof( STATUSPARAM2 ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
            }

            break;

        case IPC_SYSVERSION:             //eSysVersion,            //char*
            if ( subtype == SUBTYPE_GET )
            {
                //char param[32];
                //bzero( ( void* )&param[0], 32 );
                //GetAppParam( eSysVersion, ( void* )&param[0], 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_SYSVERSION,
                                 ( char* )&bparam.stIEBaseParam.sys_ver, sizeof(int) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
            }

            break;

        case IPC_ETHMAC:                 //eEthMac,                //char*
            if ( subtype == SUBTYPE_GET )
            {
                //char param[32];
                //bzero( ( void* )&param[0], 32 );
                //GetAppParam( eEthMac, ( void* )&param[0], 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_ETHMAC,
                                 ( char* )bparam.stIEBaseParam.szMac, strlen( bparam.stIEBaseParam.szMac ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
            }

            break;

        case IPC_WIFIMAC:                //eWifiMac,               //char*
            if ( subtype == SUBTYPE_GET )
            {
                //char param[32];
                //bzero( ( void* )&param[0], 32 );
                //GetAppParam( eWifiMac, ( void* )&param[0], 32 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_WIFIMAC,
                                 ( char* )bparam.stIEBaseParam.szWifiMac, strlen( bparam.stIEBaseParam.szWifiMac ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
            }

            break;

        case IPC_DEVICEID:               //eDeviceID,              //char*
            if ( subtype == SUBTYPE_GET )
            {
                //char param[32];
                //bzero( ( void* )&param[0], 32 );
                //GetAppParam( eDeviceID, ( void* )&param[0], 32 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_DEVICEID,
                                 ( char* )bparam.stIEBaseParam.dwDeviceID, strlen( bparam.stIEBaseParam.dwDeviceID ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
            }

            break;

        case IPC_VIDENCPARAM:            //eVidencParam,           //VIDENCPARAM
            if ( subtype == SUBTYPE_GET )
            {
                VIDENCPARAM2 param;
                bzero( ( void* )&param, sizeof( VIDENCPARAM2 ) );
                //GetAppParam( eVidencParam, ( void* )&param, 0 );

                param.Size = bparam.stVencParam.bysize;
                param.SizeSub = bparam.stVencParam.bysizesub;
                param.FrameRate = bparam.stVencParam.byframerate;
                param.FrameRateSub = bparam.stVencParam.byframeratesub;
                param.KeyFrame = bparam.stVencParam.keyframe;
                param.KeyFrameSub = bparam.stVencParam.keyframesub;
                param.Quant = bparam.stVencParam.quant;
                param.QuantSub = bparam.stVencParam.quantsub;
                param.RateMode = bparam.stVencParam.ratemode;
                param.RateModeSub = bparam.stVencParam.ratemodesub;
                param.BitRate = bparam.stVencParam.bitrate;
                param.BitRateSub = bparam.stVencParam.bitratesub;
                param.VideoMode = bparam.stVencParam.videomode;
                param.VideoEnv = bparam.stVencParam.videoenv;
                param.OsdEnable = bparam.stVencParam.OSDEnable;
                param.Birghtness = bparam.stVencParam.brightness;
                param.Chroma = bparam.stVencParam.chroma;
                param.Saturation = bparam.stVencParam.saturation;
                param.Contrast = bparam.stVencParam.contrast;
                param.IrCut = bparam.stVencParam.ircut;
                

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_VIDENCPARAM,
                                 ( char* )&param, sizeof( VIDENCPARAM2 ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
            }

            break;

        case IPC_MOTOSPEED:              //eMotoSpeed,             //char*
            if ( subtype == SUBTYPE_GET )
            {
                int motospeed = bparam.stPTZParam.byRate;
                //GetAppParam( eMotoSpeed, ( void* )&motospeed, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_MOTOSPEED,
                                 ( char* )&motospeed, sizeof( int ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
            }

            break;

        case IPC_SYSCONFIG:              //eSysConfig,             //SYSTEMCONFIG
            if ( subtype == SUBTYPE_GET )
            {
                SYSTEMCONFIG param;
                bzero( ( void* )&param, sizeof( SYSTEMCONFIG ) );
                //GetAppParam( eSysConfig, ( void* )&param, 0 );
                strcpy(param.DeviceId, bparam.stIEBaseParam.dwDeviceID);
                param.SysVersion = bparam.stIEBaseParam.sys_ver;
                param.AppVersion = bparam.stIEBaseParam.app_ver;
                strcpy(param.EthMac, bparam.stIEBaseParam.szMac);
                strcpy(param.WifiMac, bparam.stIEBaseParam.szWifiMac);
                strcpy(param.ApMac, bparam.stIEBaseParam.szWifiMac);
                param.SysMode = bparam.stIEBaseParam.sysmode;
                param.Factory = bparam.stIEBaseParam.factory;
                param.pnpport = 0;
                

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_SYSCONFIG,
                                 ( char* )&param, sizeof( SYSTEMCONFIG ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eSysConfig, ( void* )buffer, 0 );
                PSYSTEMCONFIG pparam = (PSYSTEMCONFIG)buffer;
                strcpy(bparam.stIEBaseParam.dwDeviceID, pparam->DeviceId);
                bparam.stIEBaseParam.sys_ver=pparam->SysVersion;
                bparam.stIEBaseParam.app_ver=pparam->AppVersion;
                strcpy(bparam.stIEBaseParam.szMac,pparam->EthMac);
                strcpy(bparam.stIEBaseParam.szWifiMac,pparam->WifiMac);
                bparam.stIEBaseParam.sysmode=pparam->SysMode;
                bparam.stIEBaseParam.factory=pparam->Factory;

                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_SYSCONFIG );
            }

            break;

        case IPC_RECORDAUDIO:            //eRecordAudio,           //int*
            if ( subtype == SUBTYPE_GET )
            {
                int param = bparam.stAlarmParam.reserved & 0x02;
                //GetAppParam( eRecordAudio, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_RECORDAUDIO,
                                 ( char* )&param, sizeof( int ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppParam( eRecordAudio, ( void* )buffer, 0 );
                bparam.stAlarmParam.reserved |= 0x02;
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_RECORDAUDIO );
            }

            break;

        case IPC_RECORDAUDIOLEVEL:       //eRecordAudioLevel,      //int*
            #if 0
            if ( subtype == SUBTYPE_GET )
            {
                int param = 0;
                GetAppParam( eRecordAudioLevel, ( void* )&param, 0 );

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_RECORDAUDIOLEVEL,
                                 ( char* )&param, sizeof( int ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                SetAppParam( eRecordAudioLevel, ( void* )buffer, 0 );
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_RECORDAUDIOLEVEL );
            }
            #endif
            break;



        //base of typedef enum _eControlType, in sysparam.h
        case IPC_IRCUTSWITCH:            //eIRCutSwitch,               //0-close; 1-open
            if ( subtype == SUBTYPE_GET )
            {
                int param = bparam.stVencParam.ircut;
                
                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_IRCUTSWITCH,
                                 ( char* )&param, sizeof( int ) );
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppControl( eIRCutSwitch, ( void* )buffer);
                bparam.stVencParam.ircut = *( ( int* )buffer);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_IRCUTSWITCH);
            }
            break;

        case IPC_ALARMLOG:              //eAlarmLog,                //NULL
            if ( subtype == SUBTYPE_GET )
            {
                int iRet = 0;
                char param[16*1024];
                memset(param, 0, 16*1024);
                //iRet = GetAppControl( eAlarmLog, ( void* )param);
                iRet = ReadAlarmLog( param);

                GetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_ALARMLOG,
                                 ( char* )param, iRet );
            }

            else if ( subtype == SUBTYPE_SET )
            {
            }       
            break;

        case IPC_CLEARLOG:               //eClearLog,                      //Null
            if ( subtype == SUBTYPE_GET )
            {
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppControl( eClearLog, ( void* )buffer);
                if ( access( "/param/alarmlog.bin", F_OK ) == 0x00 )
                {
                    system( "rm /param/alarmlog.bin" );
                }
                
                if ( access( "/param/alarmlog1.bin", F_OK ) == 0x00 )
                {
                    system( "rm /param/alarmlog1.bin" );
                }                
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_CLEARLOG);
            }

            break;

        case IPC_APPDEFAULT:             //eAppDefault,            //Null
            if ( subtype == SUBTYPE_GET )
            {
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppControl( eAppDefault, ( void* )buffer);
                
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_APPDEFAULT);
            }
            break;

        case IPC_REBOOT:                 //eReboot,                    //Null
            if ( subtype == SUBTYPE_GET )
            {
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppControl( eReboot, ( void* )buffer);
                SetRebootCgi();
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_REBOOT);
            }

            break;

        case IPC_RESTORE:                //eRestore,               //Null
            if ( subtype == SUBTYPE_GET )
            {
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppControl( eRestore, ( void* )buffer);
                DoSystem( "rm /system/www/system.ini" );
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_RESTORE);
            }
            break;

        case IPC_WIFISCAN:               //eWifiScan,              //Null
            if ( subtype == SUBTYPE_GET )
            {
                int param = wificnt;
                //GetDfWifiScanCnt( &param );
                Textout("ipcamera scan wifi count=%d", param);
                GetIpcParamResp(nvripaddr, 
                    PORT_CMD_NVR, 
                    IPC_WIFISCAN, 
                    &param, 
                    sizeof(int));
            }

            else if ( subtype == SUBTYPE_SET )
            {
                Textout("Do wifi scan...");
                //SetAppControl( eWifiScan, ( void* )buffer);
                StartWifiScan();
                sleep(4);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_WIFISCAN);
            }
            break;

        case IPC_SDFORMAT:               //eSdFormat,              //Null
            if ( subtype == SUBTYPE_GET )
            {
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppControl( eSdFormat, ( void* )buffer);
                NoteSDFormat();
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_SDFORMAT);
            }
            break;

        case IPC_AUDIOCTRL:              //eAudioCtrl,         //0-Stop Audio Capture; 1-Start Audio Capture
            if ( subtype == SUBTYPE_GET )
            {
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppControl( eAudioCtrl, ( void* )buffer);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_AUDIOCTRL);
            }
            break;
        case IPC_MAILTEST:              //eMailTest,         //Null
            if ( subtype == SUBTYPE_GET )
            {
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppControl( eMailTest, NULL);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_MAILTEST);
            }
            break;
        case IPC_FTPTEST:              //eFtpTest,         //NULL
            if ( subtype == SUBTYPE_GET )
            {
            }

            else if ( subtype == SUBTYPE_SET )
            {
                //SetAppControl( eFtpTest, NULL);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_FTPTEST);
            }
            break;
            

            //base of typedef enum _eCameraCmd
        case IPC_RESOLUTION:             //videoProperty_Resolustion = 0,     //0->VGA, 1->QVGA, 3->720P
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_Resolustion, *res );
                camera_control(0, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_RESOLUTION );

            }


            break;

        case IPC_BRIGHTNESS:             //videoProperty_brightness=1,        //0-255
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_brightness, *res );
                camera_control(1, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_BRIGHTNESS);

            }

            break;

        case IPC_CONTRAST:               //videoProperty_contrast=2,          //0-255
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_contrast, *res );
                camera_control(2, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_CONTRAST);

            }

            break;

        case IPC_MODE:                   //videoProperty_mode=3,               //50HZ 60HZ
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_mode, *res );
                camera_control(3, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_MODE);

            }

            break;

        case IPC_MIRRORFLIP:             //videoProperty_mirrorFlip=5,     //0-normal, 1-flip, 2-mirror, 3-mirror-flip
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_mirrorFlip, *res );
                camera_control(5, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_MIRRORFLIP);

            }

            break;

        case IPC_FRAMERATE:              //videoProperty_frameRate=6,          //frame rate 0-30
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_frameRate, *res );
                camera_control(6, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_FRAMERATE);

            }

            break;

        case IPC_DEFAULT:                //videoProperty_default=7,            //no param
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_default, *res );
                camera_control(7, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_DEFAULT );

            }

            break;

        case IPC_SATURATION:             //videoProperty_satuation=8,      //0~255
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_satuation, *res );
                camera_control(8, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_SATURATION);

            }

            break;

        case IPC_HUE:                    //videoProperty_hue=9,                //0~255
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_hue, *res );
                camera_control(9, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_HUE );

            }

            break;

        case IPC_OSD:                    //videoProperty_osd=10,               //0/1
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_osd, *res );
                camera_control(10, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_OSD );

            }

            break;

        case IPC_BITRATE:                //videoProperty_bitRate=13,           //set bit rate
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_bitRate, *res );
                camera_control(13, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_BITRATE );

            }

            break;

        case IPC_IRCUT:                  //videoProperty_ircut=14,             //0/1
            if ( subtype == SUBTYPE_SET )
            {
                int* res = ( int* )buffer;
                //SetCameraControl( videoProperty_ircut, *res );
                camera_control(14, *res);
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_IRCUT);

            }

            break;

        case IPC_QUANT:                  //videoProperty_quant=16,             //quant 0-5
        case IPC_PTZ:                    //videoProperty_ptzRate=100           //ptz speed
        case IPC_MOTO_CTRL:
        case IPC_GET_RECORD_LIST:
        case IPC_GET_RECORD_FILE:
            if ( subtype == SUBTYPE_GET )
            {
                Textout("IPC_PROTOCOL(%d) unsurpport", cmd);
            }

            break;

        case IPC_GET_DOORBELL:

            break;
        case IPC_UNLOCK:
            if ( subtype == SUBTYPE_SET )
            {
                Textout("IPC_UNLOCK");
                OnDoorOpenEx();
                SetIpcParamResp( nvripaddr,
                                 PORT_CMD_NVR,
                                 IPC_UNLOCK);
            }
            break;
        default:
            //Textout( "Unkown cmd=%d", cmd );
            break;
    }

    return 0;
}

void NvrCmdInit()
{
    Textout( "Nvr Cmd init, Ipaddr=%s, port=%d", bparam.stNetParam.szIpAddr, PORT_CMD_IPCAM );
    CreateCmdSocket( inet_addr( bparam.stNetParam.szIpAddr), PORT_CMD_IPCAM );
    CreateCmdProc();
    RegisterCmdCallback( IpcCmdCallback );
}

void NvrLiveStreamInit()
{
    Textout( "Nvr livestream init, Ipaddr=%s, port=%d", bparam.stNetParam.szIpAddr, PORT_VIDEO );
    CreateLiveStreamProc( inet_addr( bparam.stNetParam.szIpAddr ), PORT_VIDEO );
}

void NvrAudioStreamInit()
{
    Textout( "Nvr audiostream init, Ipaddr=%s, port=%d", bparam.stNetParam.szIpAddr, PORT_AUDIO );
    CreateAudioStreamProc( inet_addr( bparam.stNetParam.szIpAddr ), PORT_AUDIO );
}

