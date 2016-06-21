#ifndef __CMD_HEAD_H_
#define __CMD_HEAD_H_
//IPC command
#define CGI_SET_BASE				0x8000
#define CGI_GET_BASE				0x9000

#define CGI_SET_LABEL				0x8001
#define CGI_GET_LABEL				0x9001

#define CGI_SET_PPPOE           		0x8002
#define CGI_GET_PPPOE           		0x9002

#define CGI_SET_ALARM				0x8003
#define CGI_GET_ALARM				0x9003

#define CGI_SET_DATE				0x8004
#define CGI_GET_DATE				0x9004

#define CGI_SET_DDNS				0x8005
#define CGI_GET_DDNS				0x9005

#define CGI_SET_FTP             		0x8006
#define CGI_GET_FTP             		0x9006

#define CGI_SET_ETHNET         			0x8007
#define CGI_GET_ETHNET				0x9007

#define CGI_SET_MAIL            		0x8008
#define CGI_GET_MAIL            		0x9008

#define CGI_SET_PTZ				0x8009
#define CGI_GET_PTZ				0x9009

#define CGI_SET_UPNP            		0x800a
#define CGI_GET_UPNP            		0x900a

#define CGI_SET_USER            		0x800b
#define CGI_GET_USER            		0x900b

#define CGI_SET_WIFI            		0x800c
#define CGI_GET_WIFI            		0x900c

#define CGI_SET_VENC    			0x800d
#define CGI_GET_VENC    			0x900d

#define CGI_SET_AENC    			0x800e
#define CGI_GET_AENC    			0x900e

#define CGI_GET_RECORD        			0x800f
#define CGI_SET_RECORD        			0x900f

#define CGI_GET_PRESET        			0x8010
#define CGI_SET_PRESET        			0x9010

#define CGI_GET_MULTDEV       			0x8011
#define CGI_SET_MULTDEV      			0x9011

#define CGI_GET_FACTORY       			0x8012
#define CGI_SET_FACTORY      			0x9012


#define CGI_GET_LIVESTREAM			0x7001
#define CMD_RTSP_START    			0x7002
#define CMD_RTSP_STOP     			0x7003
#define CMD_VENC_START				0x7004
#define CMD_VENC_STOP				0x7005
#define CMD_AENC_START				0x7006
#define CMD_AENC_STOP				0x7007

#define CMD_CAPTURE_JPEG			0x7008
#define CMD_SDFORMAT				0x7009
#define CMD_DELFILE       			0x700a
#define CMD_LOG_CLR				0x700b
#define CMD_LOG_GET				0x700c

#define CMD_SSID_GET				0x700d
#define CMD_CAMERA_SET      			0x700e
#define CMD_FTPTEST				0x700f
#define CMD_MAILTEST				0x7010
#define CMD_PTZ_SET     			0x7011
#define CMD_DEFAULT_SET       			0x7012
#define CMD_IR_SET          			0x7013
#define CMD_OSD_ENABLE           		0x7014
#define CMD_WIFILIST				0x7015
#define CMD_FACTORY_GET				0x7016
#define CMD_ALARM_REPORT			0x7017
#define CMD_GET_DBG				0x7018
#define CMD_REBOOT					0x7019

typedef struct _IPCCMDPARAM
{
    short		ipccmd;		//command list
    char		index;		//0->all param other->index of param
    char       	result;		//0->ok other->failed
    unsigned int 	len;		//param len
} IPCCMDPARAM, *PIPCCMDPARAM;

//IE CGI CMD
#define CGI_IEGET_STATUS			0x6001
#define CGI_IEGET_PARAM				0x6002
#define CGI_IEGET_CAM_PARAMS			0x6003
#define CGI_IEGET_LOG				0x6004
#define CGI_IEGET_MISC				0x6005
#define CGI_IEGET_RECORD			0x6006
#define CGI_IEGET_RECORD_FILE			0x6007
#define CGI_IEGET_WIFI_SCAN			0x6008
#define CGI_IEGET_FACTORY			0x6009
#define CGI_IESET_IR				0x600a
#define CGI_IESET_UPNP				0x600b
#define CGI_IESET_ALARM				0x600c
#define CGI_IESET_LOG				0x600d
#define CGI_IESET_USER				0x600e
#define CGI_IESET_ALIAS				0x600f
#define CGI_IESET_MAIL				0x6010
#define CGI_IESET_WIFI				0x6011
#define CGI_CAM_CONTROL				0x6012
#define CGI_IESET_DATE				0x6013
#define CGI_IESET_MEDIA				0x6014
#define CGI_IESET_SNAPSHOT			0x6015
#define CGI_IESET_DDNS				0x6016
#define CGI_IESET_MISC				0x6017
#define CGI_IEGET_FTPTEST			0x6018
#define CGI_DECODER_CONTROL			0x6019
#define CGI_IESET_DEFAULT			0x601a
#define CGI_IESET_MOTO				0x601b
#define CGI_IEGET_MAILTEST			0x601c
#define CGI_IESET_MAILTEST			0x601d
#define CGI_IEDEL_FILE				0x601e
#define CGI_IELOGIN				0x601f
#define CGI_IESET_DEVICE			0x6020
#define CGI_IESET_NETWORK			0x6021
#define CGI_IESET_FTPTEST			0x6022
#define CGI_IESET_DNS				0x6023
#define CGI_IESET_OSD				0x6024
#define CGI_IESET_FACTORY			0x6025
#define CGI_IESET_PPPOE				0x6026
#define CGI_IEREBOOT				0x6027
#define CGI_IEFORMATSD				0x6028
#define CGI_IESET_RECORDSCH			0x6029
#define CGI_IESET_WIFISCAN			0x602a
#define CGI_IERESTORE				0x602b
#define CGI_IESET_FTP				0x602c
#define CGI_IESET_RTSP				0x602d
#define CGI_IEGET_VIDEOSTREAM			0x602e
#define CGI_UPGRADE_APP				0x602f
#define CGI_UPGRADE_SYS				0x6030

#define CGI_SET_IIC				0x6031
#define CGI_GET_IIC				0x6032

#endif

