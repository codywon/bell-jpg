#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <dirent.h>
#include <dirent.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>

#include "init.h"
#include "alarm.h"
#include "param.h"
#include "dns.h"
#include "cmdhead.h"
#include "moto.h"
#include "nvram.h"
#include "debug.h"

#if	0
#define	IETextout(Fmt, args...)
#else
#define	IETextout	Textout
#endif

#define NON_NUM '0'

unsigned short  		logcnt = 0;
char					snapflag = 0x00;
char					cameraflag = 0x00;
char					updateflag = 0x00;
char					dbgflag = 0x00;
char					wifiscanflag = 0x00;
sem_t				wifiscansem;

#ifdef	P2P_SCC_LIB
extern STRU_SCCPARAM    g_sccParams;
#endif

/* BEGIN: Added by wupm, 2013/6/16 */
#ifdef	CLOSE_WEB_SERVER_ON_UPDATE
static BOOL bStopWebServer = FALSE;
void CloseWebServer()
{
    bStopWebServer = TRUE;
}
#endif

/* BEGIN: Added by wupm, 2013/6/16 */
#ifdef	AUTO_DOWNLOAD_FIRMWIRE
int cgidownloadfile( char* pbuf, char* pparam, unsigned char byPri )
{
	unsigned char           temp[2048];
    char                            temp1[64];
    char                            temp2[64];
    int                             len  = 0;
    int                             iRet = 0;
    char                            nexturl[64];
    int                             value;
    int                             iValue;
    char                            i = 0;
    char                            flag = 0;
    char                            decoderbuf[128];

	char	server[64];
	int 	nPort = 80;
	char	file[64];
	int		type;
/*
    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
	//auto_download_file.cgi?server=&port=&file=&type=
	IETextout("OK, param = [%s]", pparam);

    memset( temp, 0x00, 2048 );

    memset( temp2, 0x00, 64 );
	iRet = GetStrParamValue( pparam, "server", temp2, 63 );
	if ( iRet == 0 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( server, 0x00, 64 );
        strcpy( server, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
	iRet += GetStrParamValue( pparam, "file", temp2, 63 );
	if ( iRet == 0 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( file, 0x00, 64 );
        strcpy( file, decoderbuf );
    }

	if ( GetIntParamValue(pparam, "type", &type) != 0 )
	{
		type = 0;
	}

	if ( GetIntParamValue(pparam, "port", &nPort) != 0 )
	{
		nPort = 80;
	}

	if ( iRet == 0 )
	{
        downloadfile( server, nPort, file, type );
    }

    len += sprintf( temp + len, "var result=\"ok\";\r\n" );
    memcpy( pbuf, temp, len );
    return len;
}
#endif

int CgiCommand( char* pcmd )
{
    char* pdst = NULL;
    //IETextout("===============CgiCommand====================");
    IETextout( "IE Command, cmd len=%d, cmd[%s]\n", strlen(pcmd), pcmd );
    /* BEGIN: Added by wupm, 2013/6/16 */
#ifdef	CLOSE_WEB_SERVER_ON_UPDATE

    if ( bStopWebServer )
    {
        IETextout( "Web Server Stoped, Return 0" );
        return 0;
    }

#endif

    pdst = strstr( pcmd, "openlock.cgi" );

    if ( pdst != NULL )
    {
		IETextout("Return CGI_IEGET_OPENLOCK");
        return CGI_IEGET_OPENLOCK;
    }

    pdst = strstr( pcmd, "get_status.cgi" );

    if ( pdst != NULL )
    {
		IETextout("Return CGI_IEGET_STATUS");
        return CGI_IEGET_STATUS;
    }

    pdst = strstr( pcmd, "get_params.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_PARAM;
    }

    pdst = strstr( pcmd, "get_camera_params.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_CAM_PARAMS;
    }

    pdst = strstr( pcmd, "get_log.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_LOG;
    }

    pdst = strstr( pcmd, "get_misc.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_MISC;
    }

    pdst = strstr( pcmd, "get_record.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_RECORD;
    }

    pdst = strstr( pcmd, "get_record_file.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_RECORD_FILE;
    }

    pdst = strstr( pcmd, "get_wifi_scan_result.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_WIFI_SCAN;
    }

    pdst = strstr( pcmd, "get_factory_param.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_FACTORY;
    }

    pdst = strstr( pcmd, "set_ir_gpio.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_IR;
    }

    pdst = strstr( pcmd, "set_upnp.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_UPNP;
    }

    pdst = strstr( pcmd, "set_alarm.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_ALARM;
    }

    pdst = strstr( pcmd, "set_log.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_LOG;
    }

    pdst = strstr( pcmd, "set_users.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_USER;
    }

    pdst = strstr( pcmd, "set_alias.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_ALIAS;
    }

    pdst = strstr( pcmd, "set_mail.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_MAIL;
    }

    pdst = strstr( pcmd, "set_wifi.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_WIFI;
    }

    pdst = strstr( pcmd, "camera_control.cgi" );

    if ( pdst != NULL )
    {
        return CGI_CAM_CONTROL;
    }

    pdst = strstr( pcmd, "set_datetime.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_DATE;
    }

    pdst = strstr( pcmd, "set_media.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_MEDIA;
    }

    pdst = strstr( pcmd, "snapshot.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_SNAPSHOT;
    }

    pdst = strstr( pcmd, "set_ddns.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_DDNS;
    }

    pdst = strstr( pcmd, "set_misc.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_MISC;
    }

    pdst = strstr( pcmd, "test_ftp.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_FTPTEST;
    }

    pdst = strstr( pcmd, "decoder_control.cgi" );

    if ( pdst != NULL )
    {
        return CGI_DECODER_CONTROL;
    }

    pdst = strstr( pcmd, "set_default.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_DEFAULT;
    }

    pdst = strstr( pcmd, "set_moto_run.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_MOTO;
    }

    pdst = strstr( pcmd, "test_mail.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_MAILTEST;
    }

    pdst = strstr( pcmd, "mailtest.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_MAILTEST;
    }

    pdst = strstr( pcmd, "del_file.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEDEL_FILE;
    }

    pdst = strstr( pcmd, "login.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IELOGIN;
    }

    pdst = strstr( pcmd, "set_devices.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_DEVICE;
    }

    pdst = strstr( pcmd, "set_network.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_NETWORK;
    }

    pdst = strstr( pcmd, "ftptest.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_FTPTEST;
    }

    pdst = strstr( pcmd, "set_dns.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_DNS;
    }

    pdst = strstr( pcmd, "set_factory_param.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_FACTORY;
    }

	/* BEGIN: Append CGI for Change UUID/MAC */
	/*        Added by wupm(2073111@qq.com), 2014/9/19 */
    pdst = strstr( pcmd, "setuuid.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_P2PUUID;
    }

    pdst = strstr( pcmd, "set_pppoe.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_PPPOE;
    }

    pdst = strstr( pcmd, "reboot.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEREBOOT;
    }

    pdst = strstr( pcmd, "set_formatsd.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEFORMATSD;
    }

    pdst = strstr( pcmd, "set_recordsch.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_RECORDSCH;
    }

    pdst = strstr( pcmd, "wifi_scan.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_WIFISCAN;
    }

    pdst = strstr( pcmd, "restore_factory.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IERESTORE;
    }

    pdst = strstr( pcmd, "set_ftp.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_FTP;
    }

    pdst = strstr( pcmd, "set_rtsp.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_RTSP;
    }

    pdst = strstr( pcmd, "videostream.cgi" );

    if ( pdst != NULL )
    {
    	Textout("-----videostream.cgi--------");
        return CGI_IEGET_VIDEOSTREAM;
    }

    pdst = strstr( pcmd, "audiostream.cgi" );

    if ( pdst != NULL )
    {
    	Textout("-----audiostream.cgi--------");
        return CGI_IEGET_AUDIOSTREAM;
    }

    pdst = strstr( pcmd, "get_alarmlog.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_ALARMLOG;
    }

    pdst = strstr( pcmd, "set_alarmlogclr.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_ALARMLOGCLR;
    }

    pdst = strstr( pcmd, "upgrade_htmls.cgi" );

    if ( pdst != NULL )
    {
        return CGI_UPGRADE_APP;
    }

    pdst = strstr( pcmd, "upgrade_firmware.cgi" );

    if ( pdst != NULL )
    {
        return CGI_UPGRADE_SYS;
    }

    pdst = strstr( pcmd, "get_syswifi.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_SYSWIFI;
    }

    pdst = strstr( pcmd, "set_syswifi.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_SYSWIFI;
    }

    pdst = strstr( pcmd, "livestream.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_LIVESTREAM;
    }

    pdst = strstr( pcmd, "params_backup.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_BACKUPPARAM;
    }

    pdst = strstr( pcmd, "restrore_backup.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_SAVEPARAM;
    }

    pdst = strstr( pcmd, "get_iic.cgi" );

    if ( pdst != NULL )
    {
        return CGI_GET_IIC;
    }

    pdst = strstr( pcmd, "set_iic.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_IIC;
    }

    pdst = strstr( pcmd, "get_appversion.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_APPVERSION;
    }

    pdst = strstr( pcmd, "get_scc.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_SCC_CONFIG;
    }

    pdst = strstr( pcmd, "set_scc.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_SCC_CONFIG;;
    }

    /* BEGIN: Added by wupm, 2013/3/12 */
#ifdef	HONESTECH
    pdst = strstr( pcmd, "set_htclass_factory_params.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_HTCLASS_PARAMS;
    }

    pdst = strstr( pcmd, "set_htclass_alarm.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_HTCLASS_ALARM;
    }

    pdst = strstr( pcmd, "get_htclass.cgi" );

    if ( pdst != NULL )
    {
        return CGI_GET_HTCLASS;
    }

#endif

    /* BEGIN: Modified by wupm, 2013/1/11 */
    /*
    pdst = strstr( pcmd, "set_tutk.cgi" );

    if( pdst != NULL )
    {
    	return CGI_SET_TUTK;
    }

    pdst = strstr( pcmd, "get_tutk.cgi" );

    if( pdst != NULL )
    {
    	return CGI_GET_TUTK;
    }
    */
    /* END: Deleted by wupm, 2013/1/11 */

    /* BEGIN: Added by wupm, 2013/1/11 */
    pdst = strstr( pcmd, "setvendorinfo.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SMART_SET_VENDOR;
    }

    pdst = strstr( pcmd, "getvendorinfo.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SMART_GET_VENDOR;
    }

    pdst = strstr( pcmd, "settutkinfo.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SMART_SET_TUTK;
    }

    pdst = strstr( pcmd, "gettutkinfo.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SMART_GET_TUTK;
    }

    /* END:   Added by wupm, 2013/1/11 */

    /* BEGIN: Added by wupm, 2013/3/28 */
    pdst = strstr( pcmd, "set_remote_debug.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_REMOTEDEBUG;
    }

    /* BEGIN: Added by wupm, 2013/4/11 */
    pdst = strstr( pcmd, "set_debug_var.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_DEBUGVAR;
    }

    /* BEGIN: Added by wupm, 2013/5/7 */
    pdst = strstr( pcmd, "set_extra.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_EXTRA;
    }

    /* BEGIN: Added by Baggio.wu, 2013/7/11 */
    pdst = strstr( pcmd, "get_extra.cgi" );

    if ( pdst != NULL )
    {
        return CGI_GET_EXTRA;
    }

    /* BEGIN: Deleted by wupm, 2013/5/21 */
#if	0
    /* BEGIN: Added by wupm, 2013/5/7 */
    pdst = strstr( pcmd, "set_ap.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_;
    }

#endif

    /* BEGIN: Added by wupm, 2013/5/21 */
//EYESIGHT 909
	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
#if	0
    pdst = strstr( pcmd, "SetOemLxmCfg.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_OEM_LXM;
    }

    pdst = strstr( pcmd, "GetOemLxmCfg.cgi" );

    if ( pdst != NULL )
    {
        return CGI_GET_OEM_LXM;
    }
#endif

#if 1
    pdst = strstr( pcmd, "set_smarteye_factory_params.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_SMARTEYE;
    }

    pdst = strstr( pcmd, "get_smarteye.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IEGET_SMARTEYE;
    }

#endif

#ifdef	CAMERA_SETUP
    /* BEGIN: Added by wupm, 2013/6/6 */
    //camera_setup.cgi
    pdst = strstr( pcmd, "camera_setup.cgi" );

    if ( pdst != NULL )
    {
        return CGI_IESET_CAMERA_PARAMS;
    }

#endif

#if 0
    pdst = strstr( pcmd, "get_dbg.cgi" );

    if ( pdst != NULL )
    {
        return CMD_GET_DBG;
    }

#endif

#ifdef	AUTO_DOWNLOAD_FIRMWIRE
    pdst = strstr( pcmd, "auto_download_file.cgi" );

    if ( pdst != NULL )
    {
        return CGI_AUTO_DOWNLOAD_FILE;
    }

#endif

#ifdef	SUPPORT_MORE_SENSOR

	/* BEGIN: Added by wupm, 2013/5/25 */
    pdst = strstr( pcmd, "set_sensor_params.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_SENSOR;
    }
    pdst = strstr( pcmd, "get_sensor_params.cgi" );

    if ( pdst != NULL )
    {
        return CGI_GET_SENSOR;
    }
#endif

#ifdef CUSTOM_HTTP_ALARM
	//set_alarmsvr.cgi
    pdst = strstr( pcmd, "set_alarmsvr.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_ALARM_SERVER;
    }

    pdst = strstr( pcmd, "get_alarmsvr.cgi" );

    if ( pdst != NULL )
    {
        return CGI_GET_ALARM_SERVER;
    }
#endif

#ifdef TENVIS
	//set_alarmsvr.cgi
    pdst = strstr( pcmd, "set_tenvisddns.cgi" );

    if ( pdst != NULL )
    {
        return CGI_SET_TENVIS_DDNS;
    }

    pdst = strstr( pcmd, "get_tenvisddns.cgi" );

    if ( pdst != NULL )
    {
        return CGI_GET_TENVIS_DDNS;
    }
#endif

    return 0;
}

int hex2num( char c )
{
    if ( c >= '0' && c <= '9' )
    {
        return c - '0';
    }

    if ( c >= 'a' && c <= 'z' )
    {
        return c - 'a' + 10;
    }

    if ( c >= 'A' && c <= 'Z' )
    {
        return c - 'A' + 10;
    }

    printf( "unexpected char: %c", c );
    return NON_NUM;
}
/**
 * @brief URLDecode
 * @param str
 * @param strSize
 * @param result
 * @param resultSize
 *
 * @return: >0 result
  */
int URLDecode( const char* str, const int strSize, char* result, const int resultSize )
{
    char ch, ch1, ch2;
    int i;
    int j = 0;//record result index

    if ( ( str == NULL ) || ( result == NULL ) || ( strSize <= 0 ) || ( resultSize <= 0 ) )
    {
        return 0;
    }

    for ( i = 0; ( i < strSize ) && ( j < resultSize ); ++i )
    {
        ch = str[i];

        switch ( ch )
        {
            case '+':
                result[j++] = ' ';
                break;

            case '%':
                if ( i + 2 < strSize )
                {
                    ch1 = hex2num( str[i + 1] ); //high 4 bit
                    ch2 = hex2num( str[i + 2] ); //low 4 bit

                    if ( ( ch1 != NON_NUM ) && ( ch2 != NON_NUM ) )
                    {
                        result[j++] = ( char )( ( ch1 << 4 ) | ch2 );
                    }

                    i += 2;
                    break;
                }

                else
                {
                    break;
                }

            default:
                result[j++] = ch;
                break;
        }
    }

    result[j] = 0;
    return j;
}

int MyReplace( char* pszDst, const char* pszSrc, const char* pszStr1, const char* pszStr2 )
{
    int 	i = 0, length;

    if ( !pszDst || !pszSrc )
    {
        return -1;
    }

    length = strlen( pszSrc );

    while ( i <= length )
    {
        if ( !memcmp( pszSrc, pszStr1, strlen( pszStr1 ) ) )
        {
            pszSrc += strlen( pszStr1 );
            i += strlen( pszStr1 );
            memcpy( pszDst, pszStr2, strlen( pszStr2 ) );
            pszDst += strlen( pszStr2 );
        }

        else
        {
            *pszDst++ = *pszSrc++;
            i++;
        }
    }

    return 0;
}
/* BEGIN: Modified by wupm, 2013/7/1 */
#if	0
int GetStrParamValue( const char* pszSrc, const char* pszParamName, char* pszParamValue, int maxlen )
{
    const char* pos1, *pos = pszSrc;
    char                temp[128];
    unsigned char       i;
    const char*          pos2;
    char                hanzi[4];
    int len = 0;

	/* BEGIN: Added by wupm, 2013/7/1 */
	//Step1: "?Name="
	//Step2: "&Name="
	char szParam1[32] = {0};
	char szParam2[32] = {0};
	/* END:   Added by wupm, 2013/7/1 */

    //check src and pszParam
    if ( !pszSrc || !pszParamName )
    {
        return -1;
    }

	/* BEGIN: Added by wupm, 2013/7/1 */
	sprintf(szParam1, "?%s=", pszParamName);
	sprintf(szParam2, "&%s=", pszParamName);
	/* END:   Added by wupm, 2013/7/1 */

	/* BEGIN: Added by wupm, 2013/7/1 */
	//Step3: Begin of .cgi, Oriange String = xxxxxxxx.cgi?Name=&Name=&
	pos = strstr( pszSrc, ".cgi");
	if ( !pos )
	{
		IETextout("Not Found xxxxx.cgi");
		return -1;
	}
	/* END:   Added by wupm, 2013/7/1 */

    //find param
    /* BEGIN: Modified by wupm, 2013/7/1 */
    //pos1 = strstr( pos, pszParamName );
	pos1 = strstr( pos, szParam1);
	if ( !pos1 )
	{
		pos1 = strstr( pos, szParam2);
	}
	/* END:   Modified by wupm, 2013/7/1 */

    if ( !pos1 )
    {
        return -1;
    }

    //find param end
    /* BEGIN: Modified by wupm, 2013/7/1 */
    //pos = pos1 + strlen( pszParamName ) + 1;
    pos = pos1 + strlen( szParam1);
	/* END:   Modified by wupm, 2013/7/1 */

    pos1 = strstr( pos, "&" );

    //find
    if ( pos1 )
    {
        len = pos1 - pos;

        if ( len > maxlen )
        {
            len = maxlen;
        }

        memcpy( pszParamValue, pos, len );
    }

    else	//not find
    {
        //find space
        pos1 = strstr( pos, " " );

        if ( pos1 != NULL )
        {
            len = pos1 - pos;

            if ( len > maxlen )
            {
                len = maxlen;
            }

            memcpy( pszParamValue, pos, len );
        }

        else
        {
            //find cgi end
            len = strlen( pos );

            if ( len > maxlen )
            {
                len = maxlen;
            }

            memcpy( pszParamValue, pos, len );
        }
    }

    return 0;
}
#else
int GetStrParamValue2( const char* pszSrc, const char* pszParamName, char* pszParamValue, int maxlen )
{
    const char* pos1, *pos = pszSrc;
    char                temp[128];
    unsigned char       i;
    const char*          pos2;
    char                hanzi[4];
    int len = 0;

	/* BEGIN: Added by wupm, 2013/7/1 */
	//Step1: "?Name="
	//Step2: "&Name="
	char szParam1[32] = {0};
	char szParam2[32] = {0};
	/* END:   Added by wupm, 2013/7/1 */

    //check src and pszParam
    if ( !pszSrc || !pszParamName )
    {
        return -1;
    }

	/* BEGIN: Added by wupm, 2013/7/1 */
	sprintf(szParam1, "?%s=", pszParamName);
	sprintf(szParam2, "&%s=", pszParamName);
	/* END:   Added by wupm, 2013/7/1 */

	/* BEGIN: Added by wupm, 2013/7/1 */
	//Step3: Begin of .cgi, Oriange String = xxxxxxxx.cgi?Name=&Name=&
	pos = strstr( pszSrc, ".cgi");
	if ( !pos )
	{
		IETextout("Not Found xxxxx.cgi");
		return -1;
	}
	/* END:   Added by wupm, 2013/7/1 */

    //find param
    /* BEGIN: Modified by wupm, 2013/7/1 */
    //pos1 = strstr( pos, pszParamName );
	pos1 = strstr( pos, szParam1);
	if ( !pos1 )
	{
		pos1 = strstr( pos, szParam2);
	}
	/* END:   Modified by wupm, 2013/7/1 */

    if ( !pos1 )
    {
        return -1;
    }

    //find param end
    /* BEGIN: Modified by wupm, 2013/7/1 */
    //pos = pos1 + strlen( pszParamName ) + 1;
    pos = pos1 + strlen( szParam1);
	/* END:   Modified by wupm, 2013/7/1 */

    pos1 = strstr( pos, "&" );

    //find
    if ( pos1 )
    {
        len = pos1 - pos;

        if ( len > maxlen )
        {
            len = maxlen;
        }

        memcpy( pszParamValue, pos, len );
    }

    else	//not find
    {
        //find space
        pos1 = strstr( pos, " " );

        if ( pos1 != NULL )
        {
            len = pos1 - pos;

            if ( len > maxlen )
            {
                len = maxlen;
            }

            memcpy( pszParamValue, pos, len );
        }

        else
        {
            //find cgi end
            len = strlen( pos );

            if ( len > maxlen )
            {
                len = maxlen;
            }

            memcpy( pszParamValue, pos, len );
        }
    }

    return 0;
}

int GetStrParamValue( const char* pszSrc, const char* pszParamName, char* pszParamValue, int maxlen )
{
    const char* pos1, *pos = pszSrc;
    char                temp[128];
    unsigned char       i;
    const char*          pos2;
    char                hanzi[4];
    int len = 0;

    //check src and pszParam
    if ( !pszSrc || !pszParamName )
    {
        return -1;
    }

    //find param
    pos1 = strstr( pos, pszParamName );

    if ( !pos1 )
    {
        return -1;
    }

    //find param end
    pos = pos1 + strlen( pszParamName ) + 1;
    pos1 = strstr( pos, "&" );

    //find
    if ( pos1 )
    {
        len = pos1 - pos;

        if ( len > maxlen )
        {
            len = maxlen;
        }

        memcpy( pszParamValue, pos, len );
    }

    else	//not find
    {
        //find space
        pos1 = strstr( pos, " " );

        if ( pos1 != NULL )
        {
            len = pos1 - pos;

            if ( len > maxlen )
            {
                len = maxlen;
            }

            memcpy( pszParamValue, pos, len );
        }

        else
        {
            //find cgi end
            len = strlen( pos );

            if ( len > maxlen )
            {
                len = maxlen;
            }

            memcpy( pszParamValue, pos, len );
        }
    }

    return 0;
}
#endif

/* BEGIN: Added by wupm, 2013/5/25 */
unsigned int MyAtoI2( char str[] )
{
    int i;
    int weight = 1; 	// 权重
    unsigned int rtn = 0; 		// 用作返回

    for ( i = strlen( str ) - 1; i >= 0; i-- )
    {
        rtn += ( str[i] -  '0' ) * weight; //
        weight *= 10; // 增重
    }

    return rtn;
}

/* BEGIN: Added by wupm, 2013/5/29 */
unsigned int MyAtoI( char str[] )
{
    if ( str[0] ==  '-' )
    {
        return 0 - MyAtoI2( str + 1 );
    }

    else
    {
        return MyAtoI2( str );
    }
}

/* BEGIN: Modified by wupm, 2013/7/1 */
#if	1
int GetIntParamValue( const char* pszSrc, const char* pszParamName, int* iValue )
{
    char        szParamValue[256] = {0};
	int iRet = GetStrParamValue(pszSrc, pszParamName, szParamValue, 64);
	*iValue = 0;
	if ( iRet == 0 )
	{
		if ( szParamValue[0] != 0 )
			*iValue = atoi(szParamValue);
	}
	else
	{
		return -1;
	}
    return 0;
}
//#else
/* BEGIN: Added by Baggio.wu, 2013/7/15 */
int GetIntParamValue2( const char* pszSrc, const char* pszParamName, int* iValue )
{
    char        szParamValue[256];
    const char*  pos1, *pos = pszSrc;
    const char*  pos2;
    *iValue = -1;
    memset( szParamValue, 0, sizeof( szParamValue ) );

    if ( !pszSrc || !pszParamName )
    {
        return -1;
    }

    pos1 = strstr( pos, pszParamName );

    if ( !pos1 )
    {
        return -1;
    }

    pos = pos1 + strlen( pszParamName ) + 1;
    pos1 = strstr( pos, "&" );

    if ( pos1 )
    {
        memcpy( szParamValue, pos, pos1 - pos );
    }

    else
    {
        pos1 = strstr( pos, " " );

        if ( pos1 )
        {
            //printf("len %d\n",pos1-pos);
            memcpy( szParamValue, pos, pos1 - pos );
        }
    }

    if ( pos1 )
    {
        /* BEGIN: Modified by wupm, 2013/5/25 */
        //printf("len:%s\n",szParamValue);
        //*iValue = atoi( szParamValue );
        IETextout("atoi(%s) = %u", szParamValue, MyAtoI( szParamValue ));
        *iValue = MyAtoI( szParamValue );
        //*iValue = atoi( szParamValue );
    }

    else
    {
        /* BEGIN: Modified by wupm, 2013/5/25 */
        //printf("len:%s\n",pos);
        //IETextout("atoi(%s)", pos);
        //*iValue = atoi( pos );
        *iValue = MyAtoI( pos );
        //*iValue = atoi( pos );
    }

    return 0;
}
#endif

int RefreshUrl( char* pbuf, char* nexturl )
{
    int  len = 0;
    FILE* fp = NULL;
    char temp[128];
    unsigned int offset = 0;
    int iRet;
#if 0
    len =  sprintf( pbuf + len, "HTTP/1.1 200 OK\r\n" );
    len += sprintf( pbuf + len, "Date: Mon Jun 11 12:28:44 2012\r\n" );
    len += sprintf( pbuf + len, "Server: GoAhead-Webs\r\n" );
    len += sprintf( pbuf + len, "Accept-Ranges: bytes\r\n" );
    len += sprintf( pbuf + len, "Connection: close\r\n" );
    len += sprintf( pbuf + len, "Content-type: text/html\n\n" );
    memset( temp, 0x00, 128 );
    sprintf( temp, "/system/www/%s", nexturl );
    fp = fopen( temp, "rb" );

    if ( fp != NULL )
    {
        while ( !feof( fp ) )
        {
            len += fread( pbuf + offset, 1, 1024, fp );
        }

        fclose( fp );
    }

#else
    len += sprintf( pbuf + len, "<html>\n\n" );
    len += sprintf( pbuf + len, "<head>\n\n" );
    len += sprintf( pbuf + len, "<title></title>\n\n" );
    len += sprintf( pbuf + len, "<meta http-equiv=\"Cache-Control\" content=\"no-cache, must-reva lidate\">" );
    len += sprintf( pbuf + len, "<meta http-equiv=\"refresh\" content=\"0; url=/%s\" />\n\n", nexturl );
    len += sprintf( pbuf + len, "</head>\n\n" );
    len += sprintf( pbuf + len, "<body>\n\n" );
    len += sprintf( pbuf + len, "</body>\n\n" );
    len += sprintf( pbuf + len, "<html>\n\n" );
#endif
    return len;
}

int RefreshUrl1( char* pbuf, char* nexturl )
{
    int  len = 0;
    FILE* fp = NULL;
    char temp[128];
    unsigned int offset = 0;
    int iRet;
#if 0
    len =  sprintf( pbuf + len, "HTTP/1.1 200 OK\r\n" );
    len += sprintf( pbuf + len, "Date: Mon Jun 11 12:28:44 2012\r\n" );
    len += sprintf( pbuf + len, "Server: GoAhead-Webs\r\n" );
    len += sprintf( pbuf + len, "Accept-Ranges: bytes\r\n" );
    len += sprintf( pbuf + len, "Connection: close\r\n" );
    len += sprintf( pbuf + len, "Content-type: text/html\n\n" );
#endif
    memset( temp, 0x00, 128 );
    sprintf( temp, "/system/www/%s", nexturl );
    fp = fopen( temp, "r" );

    if ( fp != NULL )
    {
        while ( !feof( fp ) )
        {
            len += fread( pbuf + len, 1, 1024, fp );
        }

        fclose( fp );
    }

    return len;
}

//get user pri
//result:0->error user or passwd  error 1->vistor  2->opration  255->admin
/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/7/2 */
int GetUserPri( char* loginuse, char* loginpas )
{
    char i;
    int pri = 0x00;
    char user[32];
    char passwd[32];
    memset( user, 0x00, 32 );
    memset( passwd, 0x00, 32 );
    //printf("GetUserPri0\n");
    strcpy( user, loginuse );
    //printf("GetUserPri1\n");
    strcpy( passwd, loginpas );

    //printf("GetUserPri2\n");
    if ( strlen( user ) == 0x00 )
    {
        return 0;
    }

    //printf("GetUserPri3\n");
    /* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
	#if	0
    for ( i = 0; i < 3; i++ )
    {
        //printf("user:%s pwd:%s\n",bparam.stUserParam[i].szUserName,bparam.stUserParam[i].szPassword);
        //printf("loginuser:%s loginpas:%s len %d  len1 %d\n",loginuse,loginpas,strlen(loginuse),strlen(loginpas));
        if ( !memcmp( userbackup[i].szUserName, user, 31 )
             && !memcmp( userbackup[i].szPassword, passwd, 31 ) )
        {
            if ( i == 0 )
            {
                pri = 1;
            }

            else if ( i == 1 )
            {
                pri = 2;
            }

            else if ( i == 2 )
            {
                pri = 255;
            }

            break;
        }
    }
	#else
	for ( i = 0; i < MAX_USER; i++ )
    {
		//IETextout("nIndex = %d, user/pwd=[%s]/[%s], Param=[%s]/[%s]", i, bparam.stBell.user[i], bparam.stBell.pwd[i], user, passwd);
        if ( !strcmp( bparam.stBell.user[i], user )
             && !strcmp( bparam.stBell.pwd[i], passwd) )
        {
			/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/25 */

			/*
			if ( bUserExisted(i + 1) )
			{
				pri = -1;
				break;
			}
			*/

            if ( bparam.stBell.admin == i + 1 )
				pri = ( i + 1 ) + 100;
			else
				pri = i + 1;

			break;
        }
    }
	#endif

    //IETextout("Return pri=%d\n",pri);
    return pri;
}
/* END: Deleted by wupm(2073111@qq.com), 2014/7/2 */

int cgigetiic( unsigned char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int		iRet;
    int             len = 0;
    int		addr;
    unsigned char   value;
    printf( "pparam:%s\n", pparam );
    iRet = GetIntParamValue( pparam, "addr", &addr );

    if ( iRet )
    {
        memcpy( pbuf, temp, len );
        return len;
    }

    memset( temp, 0x00, 2048 );
    //value = iicget(addr);
    len += sprintf( temp + len, "var addr=%x;\r\n", addr );
    len += sprintf( temp + len, "var value=%x;\r\n", value );
    memcpy( pbuf, temp, len );
    return len;
}

int cgisetiic( unsigned char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             iRet;
    int             len = 0;
    int             addr;
    int   		value;
    printf( "pparam:%s\n", pparam );
    iRet = GetIntParamValue( pparam, "addr", &addr );

    if ( iRet )
    {
        len += sprintf( temp + len, "no addr\r\n" );
        memcpy( pbuf, temp, len );
        return len;
    }

    iRet = GetIntParamValue( pparam, "value", &value );

    if ( iRet )
    {
        len += sprintf( temp + len, "no addr\r\n" );
        memcpy( pbuf, temp, len );
        return len;
    }

    memset( temp, 0x00, 2048 );
    //iicset(addr,value);
    len += sprintf( temp + len, "set ok;\r\n" );
    memcpy( pbuf, temp, len );
    return len;
}

int cgigetdbg( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char   temp[2048];
    unsigned char   web[1024];
    short           i;
    short           weblen = 0;
    int             len = 0;
    return len;
}

#ifdef	P2P_SCC_LIB
int cgiGetSccConfig( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             result = 0;
/*

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/

    memset( temp, 0x00, 2048 );
    len += sprintf( temp + len, "var scc_server=\"%s\";\r\n", g_sccParams.szServer );
    len += sprintf( temp + len, "var scc_user=\"%s\";\r\n", g_sccParams.szUser );
    len += sprintf( temp + len, "var scc_pwd=\"%s\";\r\n", g_sccParams.szPwd );
    len += sprintf( temp + len, "var scc_port=%d;\r\n", g_sccParams.nServerPort );
    memcpy( pbuf, temp, len );
    return len;
}

int cgiSetSccConfig( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   	temp[2048];
    int                 len  = 0;
    int             	iRet = 0;
    char            	nexturl[64];
    char            	szServer[64];
    char				szUser[32];
    char				szPwd[32];
    int nValue = 0;

    //printf("cgiSetSccConfig.. byPri: %d\n", byPri);
    /*

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }*/

    memset( temp, 0, 2048 );
    memset( szServer, 0, sizeof( szServer ) );
    iRet = GetStrParamValue( pparam, "scc_server", szServer, 64 );

    if ( iRet == 0 )
    {
        memset( g_sccParams.szServer, 0, sizeof( g_sccParams.szServer ) );
        memcpy( g_sccParams.szServer, szServer, 64 );
        g_sccParams.szServer[63] = 0;
    }

    memset( szUser, 0, sizeof( szUser ) );
    iRet = GetStrParamValue( pparam, "scc_user", szUser, 32 );

    if ( iRet == 0 )
    {
        memset( g_sccParams.szUser, 0, sizeof( g_sccParams.szUser ) );
        memcpy( g_sccParams.szUser, szUser, 32 );
        g_sccParams.szUser[31] = 0;
    }

    memset( szPwd, 0, sizeof( szPwd ) );
    iRet = GetStrParamValue( pparam, "scc_pwd", szPwd, 32 );

    if ( iRet == 0 )
    {
        memset( g_sccParams.szPwd, 0, sizeof( g_sccParams.szPwd ) );
        memcpy( g_sccParams.szPwd, szPwd, 32 );
        g_sccParams.szPwd[31] = 0;
    }

    iRet = GetIntParamValue( pparam, "scc_port", &nValue );

    if ( iRet == 0 )
    {
        g_sccParams.nServerPort = nValue;
    }

    memset( nexturl, 0, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}
#endif


/* BEGIN: Added by wupm, 2013/6/18 */
#ifdef	AUTO_DOWNLOAD_FIRMWIRE
//界面固件版本号存放在www下的appver.js,客户ID存放在www下的oem.txt
//var app_ver="TH2.0.0.11";
#define	APP_ID_PATH	"/system/www/appver.js"
#define	OEM_ID_PATH	"/system/www/oem.txt"
void GetAppID(char *appver)
{
	char szTemp[64] = {0};
	char *p = NULL;
	char *pdst1 = NULL;

	FILE* fp = NULL;
    fp = fopen( APP_ID_PATH, "r" );
	memset(szTemp, 0, 64);
    if ( fp == NULL )
    {
		Textout1("Set App Ver = 0.0.0.0");
		strcpy(appver, "0.0.0.0");
        return;
    }
	else
	{
	    fgets( szTemp, 64, fp );
	    fclose( fp );
	}

	if ( szTemp[0] != 0 )
	{
		p = szTemp + strlen("var app_ver=\"");
		pdst1 = strstr(p, "\"");
		if ( pdst1 == NULL )
		{
		}
		else
		{
			szTemp[pdst1 - szTemp] = 0;
		}
		strcpy(appver, p);
		Textout1("Read AppVer = [%s]", appver);
	}
}
void GetOemID(char *oemver)
{
	FILE* fp = NULL;
    fp = fopen( OEM_ID_PATH, "r" );

    if ( fp == NULL )
    {
		IETextout("Set OEM Ver = 0");
		strcpy(oemver, "0");
        return;
    }

    fgets( oemver, 16, fp );
    fclose( fp );
	Textout1("Read OemVer = [%s]", oemver);
}
#endif

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
#if	1
int cgigetstatus( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char 	temp[2048];
    char		mac[20];
    int		len = 0;
    time_t     curTime;
    int		value;

	IETextout("Get Status CGI");

    curTime = time( NULL );
    //printf("get status.cgi\n");
    memset( temp, 0x00, 2048 );
    len += sprintf( temp + len, "var alias=\"%s\";\r\n", bparam.stIEBaseParam.szDevName );
    len += sprintf( temp + len, "var deviceid=\"%s\";\r\n", bparam.stIEBaseParam.dwDeviceID );
	len += sprintf( temp + len, "var apilisense=\"%s\";\r\n", bparam.stIEBaseParam.dwApiLisense);
	Textout("deviceid=\"%s\";", bparam.stIEBaseParam.dwDeviceID);
	Textout("apilisense=\"%s\";", bparam.stIEBaseParam.dwApiLisense);
    value = bparam.stIEBaseParam.sys_ver;
    len += sprintf( temp + len, "var sys_ver=\"%d.%d.%d.%d\";\r\n",
                    ( value >> 24 ) & 0xff, ( value >> 16 ) & 0xff, ( value >> 8 ) & 0xff, value & 0xff );


/* BEGIN: Added by wupm, 2013/6/19 */
#if	0	//def	AUTO_DOWNLOAD_FIRMWIRE
	if ( 1 )
	{
		char app_version[64];
		char oem_version[64];
		memset(app_version, 0, 64);
		memset(oem_version, 0, 64);
		GetAppID(app_version);
		GetOemID(oem_version);

		/* BEGIN: Modified by wupm, 2013/6/23 */
		len += sprintf(temp + len, "var app_version=\"%s\";\r\n", app_version);
		len += sprintf(temp + len, "var oem_id=\"%s\";\r\n", oem_version);
	}
#endif

    //len += sprintf(temp+len,"var app_ver=\"%s\";\r\n",bparam.stIEBaseParam.app_ver);
    len += sprintf( temp + len, "var now=%d;\r\n", bparam.stDTimeParam.dwCurTime ); // - bparam.stDTimeParam.byTzSel);
    //len += sprintf(temp+len,"var extern_ip_status=\"\";\r\n");//extern ip is need add by zqh
    len += sprintf( temp + len, "var alarm_status=%d;\r\n", bparam.stStatusParam.warnstat );

    /* BEGIN: Added by wupm, 2013/5/25 */
#ifdef	INSTEAD_UPNPSTATUS_TO_DEVICETYPE
    Textout1( "IE Get Status, Devicetype = %u", bparam.stExtraParam.nDevicetype );
    len += sprintf( temp + len, "var upnp_status=%u;\r\n", bparam.stExtraParam.nDevicetype );
#else
    len += sprintf( temp + len, "var upnp_status=%d;\r\n", bparam.stStatusParam.upnpstat );
#endif

    len += sprintf( temp + len, "var dnsenable=%d;\r\n", bparam.stDdnsParam.enable );
    len += sprintf( temp + len, "var osdenable=%d;\r\n", bparam.stVencParam.OSDEnable );
    //len += sprintf(temp+len,"var wpstatus=%d;\r\n",bparam.stWifiParam.wpsstatus);
    len += sprintf( temp + len, "var syswifi_mode=%d;\r\n", bparam.stIEBaseParam.sysmode );

    memset( mac, 0x00, 20 );
    memcpy( mac, bparam.stIEBaseParam.szMac, 17 );
    len += sprintf( temp + len, "var mac=\"%s\";\r\n", mac );
    memset( mac, 0x00, 20 );
    memcpy( mac, bparam.stIEBaseParam.szWifiMac, 17 );
    len += sprintf( temp + len, "var wifimac=\"%s\";\r\n", mac );
    len += sprintf( temp + len, "var dns_status=%d;\r\n", bparam.stDdnsParam.dnsstatus );
    len += sprintf( temp + len, "var authuser=%d;\r\n", encryptflag );

    /* BEGIN: Modified by wupm, 2013/5/24 */
    len += sprintf( temp + len, "var devicetype=21037151;\r\n" );	//0x0141005f
    len += sprintf( temp + len, "var devicesubtype=0;\r\n" );	//0
    //len += sprintf( temp + len, "var devicetype=%d;\r\n", (unsigned int)bparam.stExtraParam.nDevicetype);
    //len += sprintf( temp + len, "var devicesubtype=%d;\r\n", (unsigned int)bparam.stExtraParam.nDeviceSubtype);

    len += sprintf( temp + len, "var externwifi=%d;\r\n", externwifistatus );	//0


    /* BEGIN: Deleted by wupm, 2013/4/11 */
	Lock_GetStorageSpace();
    len += sprintf( temp + len, "var record_sd_status=%d;\r\n", bparam.stRecordSet.sdstatus );
    //len += sprintf( temp + len, "var sdstatus=%d;\r\n", bparam.stStatusParam.sdstatus );
    len += sprintf( temp + len, "var sdtotal=%d;\r\n", sdtotal );
    len += sprintf( temp + len, "var sdfree=%d;\r\n", sdfree );
	IETextout("Return: SD Size = %d / %d", sdfree, sdtotal);
    UnLock_GetStorageSpace();

    memcpy( pbuf, temp, len );
    return len;
}
#endif

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
#if	1
int cgigetparams( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char   temp[8192];
    int             len = 0;
    char		i;
    char		temp1[64];
    char		temp2[64];
    char		temp3[16];
    int 		iRet;
    char		userflag = 0x00;
    //printf("dns sver1 %d\n",bparam.stDdnsParam.byDdnsSvr);
    //printf("get params:%d\n",byPri);
    Textout1( "1 = [%s]", bparam.stNetParam.szDns1 );
    Textout1( "2 = [%s]", bparam.stNetParam.szDns2 );
    memset( temp, 0x00, 8192 );
    //date
    len += sprintf( temp + len, "var now1=%d;\r\n", bparam.stDTimeParam.dwCurTime ); // - bparam.stDTimeParam.byTzSel);
    len += sprintf( temp + len, "var tz=%d;\r\n", bparam.stDTimeParam.byTzSel );
    len += sprintf( temp + len, "var ntp_enable=%d;\r\n", bparam.stDTimeParam.byIsNTPServer );
    len += sprintf( temp + len, "var ntp_svr=\"%s\";\r\n", bparam.stDTimeParam.szNtpSvr );
    //network param groups
    len += sprintf( temp + len, "var dhcpen=%d;\r\n", bparam.stNetParam.byIsDhcp );
    len += sprintf( temp + len, "var ip=\"%s\";\r\n", bparam.stNetParam.szIpAddr );
    len += sprintf( temp + len, "var mask=\"%s\";\r\n", bparam.stNetParam.szMask );
    len += sprintf( temp + len, "var gateway=\"%s\";\r\n", bparam.stNetParam.szGateway );
    len += sprintf( temp + len, "var dns1=\"%s\";\r\n", bparam.stNetParam.szDns1 );
    len += sprintf( temp + len, "var dns2=\"%s\";\r\n", bparam.stNetParam.szDns2 );
    len += sprintf( temp + len, "var port=%d;\r\n", bparam.stNetParam.nPort );

    //mult device groups
    for ( i = 2; i < 10; i++ )
    {
        len += sprintf( temp + len, "var dev%d_host=\"%s\";\r\n", i, bparam.stMultDevice[i - 2].host );
        len += sprintf( temp + len, "var dev%d_alias=\"%s\";\r\n", i, bparam.stMultDevice[i - 2].alias );
        len += sprintf( temp + len, "var dev%d_user=\"%s\";\r\n", i, bparam.stMultDevice[i - 2].user );
        len += sprintf( temp + len, "var dev%d_pwd=\"%s\";\r\n", i, bparam.stMultDevice[i - 2].passwd );
        len += sprintf( temp + len, "var dev%d_port=%d;\r\n", i, bparam.stMultDevice[i - 2].port );
    }

    if ( byPri == 255 )
    {
        //user groups
        for ( i = 1; i < MAX_USER + 1; i++ )
        {
            char temp1[9];
            len += sprintf( temp + len, "var user%d_name=\"%s\";\r\n", i, bparam.stUserParam[i - 1].szUserName );
            len += sprintf( temp + len, "var user%d_pwd=\"%s\";\r\n", i, bparam.stUserParam[i - 1].szPassword );
            //len += sprintf(temp+len,"var user%d_pri=%d;\r\n",i,bparam.stUserParam[i-1].byPri);
        }

        //wifi param groups
        //len += sprintf(temp+len,"var wps_enable=%d;\r\n", bparam.stWifiParam.byWps);
        //memset(temp3,0x00,16);
        //memcpy(temp3,bparam.stWifiParam.wpspin,8);
        //len += sprintf(temp+len,"var wps_pin=\"%s\";\r\n", temp3);
        len += sprintf( temp + len, "var wifi_enable=%d;\r\n", bparam.stWifiParam.byEnable );
        memset( temp1, 0, sizeof( temp1 ) );
        memset( temp2, 0, sizeof( temp2 ) );
        MyReplace( temp1, bparam.stWifiParam.szSSID, "\"", "%22" );
        len += sprintf( temp + len, "var wifi_ssid=\"%s\";\r\n", temp1 );
        len += sprintf( temp + len, "var wifi_mode=%d;\r\n", bparam.stWifiParam.byWifiMode );
        len += sprintf( temp + len, "var wifi_encrypt=%d;\r\n", bparam.stWifiParam.byEncrypt );
        len += sprintf( temp + len, "var wifi_authtype=%d;\r\n", bparam.stWifiParam.byAuthType );
        len += sprintf( temp + len, "var wifi_defkey=%d;\r\n", bparam.stWifiParam.byDefKeyType );
        len += sprintf( temp + len, "var wifi_keyformat=%d;\r\n", bparam.stWifiParam.byKeyFormat );
        len += sprintf( temp + len, "var wifi_key1=\"%s\";\r\n", bparam.stWifiParam.szKey1 );
        len += sprintf( temp + len, "var wifi_key2=\"%s\";\r\n", bparam.stWifiParam.szKey2 );
        len += sprintf( temp + len, "var wifi_key3=\"%s\";\r\n", bparam.stWifiParam.szKey3 );
        len += sprintf( temp + len, "var wifi_key4=\"%s\";\r\n", bparam.stWifiParam.szKey4 );
        len += sprintf( temp + len, "var wifi_key1_bits=%d;\r\n", bparam.stWifiParam.byKey1Bits );
        len += sprintf( temp + len, "var wifi_key2_bits=%d;\r\n", bparam.stWifiParam.byKey2Bits );
        len += sprintf( temp + len, "var wifi_key3_bits=%d;\r\n", bparam.stWifiParam.byKey3Bits );
        len += sprintf( temp + len, "var wifi_key4_bits=%d;\r\n", bparam.stWifiParam.byKey4Bits );
        len += sprintf( temp + len, "var wifi_wpa_psk=\"%s\";\r\n", bparam.stWifiParam.szShareKey );
        len += sprintf( temp + len, "var wifi_channel=%d;\r\n", bparam.stWifiParam.channel );
        //adsl param groups
        len +=	sprintf( temp + len, "var pppoe_enable=%d;\r\n", bparam.stPppoeParam.byEnable );
        len +=	sprintf( temp + len, "var pppoe_user=\"%s\";\r\n", bparam.stPppoeParam.szUserName );
        len +=	sprintf( temp + len, "var pppoe_pwd=\"%s\";\r\n", bparam.stPppoeParam.szPassword );

        /* BEGIN: Deleted by wupm, 2013/5/3 */
#if	0
        //rtsp param groups
        len +=	sprintf( temp + len, "var rtsp_auth_enable=%d;\r\n", bparam.stRtspParam.byEnable );
        len +=	sprintf( temp + len, "var rtsp_user=\"%s\";\r\n", bparam.stRtspParam.szUserName );
        len +=	sprintf( temp + len, "var rtsp_pwd=\"%s\";\r\n", bparam.stRtspParam.szPassword );
#endif

        //upnp param groups
        len +=	sprintf( temp + len, "var upnp_enable=%d;\r\n", bparam.stUpnpParam.byEnable );
        //ddns param groups
        len +=	sprintf( temp + len, "var ddns_service=%d;\r\n", bparam.stDdnsParam.byDdnsSvr );
        len +=	sprintf( temp + len, "var ddns_proxy_svr=\"%s\";\r\n", bparam.stDdnsParam.szProxySvr );
        len +=	sprintf( temp + len, "var ddns_host=\"%s\";\r\n", bparam.stDdnsParam.szDdnsName );
        len +=	sprintf( temp + len, "var ddns_user=\"%s\";\r\n", bparam.stDdnsParam.szUserName );
        len +=	sprintf( temp + len, "var ddns_pwd=\"%s\";\r\n", bparam.stDdnsParam.szPassword );
        len +=	sprintf( temp + len, "var ddns_proxy_port=%d;\r\n", bparam.stDdnsParam.nProxyPort );
        len +=	sprintf( temp + len, "var ddns_mode=%d;\r\n", bparam.stDdnsParam.byMode );
        len +=  sprintf( temp + len, "var ddns_status=%d;\r\n", bparam.stDdnsParam.dnsstatus );
        //email param groups
        len +=	sprintf( temp + len, "var mail_sender=\"%s\";\r\n", bparam.stMailParam.szSender );
        len +=	sprintf( temp + len, "var mail_receiver1=\"%s\";\r\n", bparam.stMailParam.szReceiver1 );
        len +=	sprintf( temp + len, "var mail_receiver2=\"%s\";\r\n", bparam.stMailParam.szReceiver2 );
        len +=	sprintf( temp + len, "var mail_receiver3=\"%s\";\r\n", bparam.stMailParam.szReceiver3 );
        len +=	sprintf( temp + len, "var mail_receiver4=\"%s\";\r\n", bparam.stMailParam.szReceiver4 );
        len +=	sprintf( temp + len, "var mailssl=%d;\r\n", bparam.stMailParam.byCheck );
        len +=	sprintf( temp + len, "var mail_svr=\"%s\";\r\n", bparam.stMailParam.szSmtpSvr );
        len +=	sprintf( temp + len, "var mail_user=\"%s\";\r\n", bparam.stMailParam.szSmtpUser );
        len +=	sprintf( temp + len, "var mail_pwd=\"%s\";\r\n", bparam.stMailParam.szSmtpPwd );
        len +=	sprintf( temp + len, "var mail_port=%d;\r\n", bparam.stMailParam.nSmtpPort );
        len +=	sprintf( temp + len, "var mail_inet_ip=%d;\r\n", bparam.stMailParam.byNotify );
        //ftp param groups
        len +=	sprintf( temp + len, "var ftp_svr=\"%s\";\r\n", bparam.stFtpParam.szFtpSvr );
        len +=	sprintf( temp + len, "var ftp_user=\"%s\";\r\n", bparam.stFtpParam.szFtpUser );
        len +=	sprintf( temp + len, "var ftp_pwd=\"%s\";\r\n", bparam.stFtpParam.szFtpPwd );
        len +=	sprintf( temp + len, "var ftp_dir=\"%s\";\r\n", bparam.stFtpParam.szFtpDir );
        len +=	sprintf( temp + len, "var ftp_port=%d;\r\n", bparam.stFtpParam.nFtpPort );
        len +=	sprintf( temp + len, "var ftp_mode=%d;\r\n", bparam.stFtpParam.byMode );
        len +=	sprintf( temp + len, "var ftp_upload_interval=%d;\r\n", ( unsigned short )bparam.stFtpParam.nInterTime );
        len +=	sprintf( temp + len, "var ftp_filename=%d;\r\n", bparam.stFtpParam.szFileName );
        //alarm param groups
        len +=	sprintf( temp + len, "var alarm_motion_armed=%d;\r\n", bparam.stAlarmParam.byMotionEnable );
        len +=	sprintf( temp + len, "var alarm_motion_sensitivity=%d;\r\n", bparam.stAlarmParam.bySensitivity );
        len +=	sprintf( temp + len, "var alarm_input_armed=%d;\r\n", bparam.stAlarmParam.byAlarmInEnable );
        len +=	sprintf( temp + len, "var alarm_ioin_level=%d;\r\n", bparam.stAlarmParam.byAlarmInLevel );
        len +=	sprintf( temp + len, "var alarm_mail=%d;\r\n", bparam.stAlarmParam.byAlarmEmail );
        len +=	sprintf( temp + len, "var alarm_iolinkage=%d;\r\n", bparam.stAlarmParam.byLinkEnable );
        len +=	sprintf( temp + len, "var alarm_ioout_level=%d;\r\n", bparam.stAlarmParam.byAlarmOutLevel );
        len +=	sprintf( temp + len, "var alarm_upload_interval=%d;\r\n", bparam.stAlarmParam.byUploadInter );
        len +=  sprintf( temp + len, "var alarm_presetsit=%d;\r\n", bparam.stAlarmParam.szPresetSit );
        len +=  sprintf( temp + len, "var alarm_snapshot=%d;\r\n", bparam.stAlarmParam.bySnapshot );
        len +=  sprintf( temp + len, "var alarm_record=%d;\r\n", bparam.stAlarmParam.byAlarmRecord );
        len +=  sprintf( temp + len, "var alarm_schedule_enable=%d;\r\n", bparam.stAlarmParam.alarmen );
        len +=  sprintf( temp + len, "var alarm_http=%d;\r\n", bparam.stAlarmParam.alarm_http );
        len +=  sprintf( temp + len, "var alarm_http_url=\"%s\";\r\n", bparam.stAlarmParam.alarm_http_url );

/* BEGIN: Added by wupm, 2013/6/27 */
#ifdef	PLAY_AUDIO_FILE_ON_ALARM
		len +=  sprintf( temp + len, "var enable_alarm_audio=%d;\r\n", bparam.stAlarmParam.reserved & 0x01 );
#endif

        IETextout( "----- = [%d] [0x%08X] [%u]",
                 bparam.stAlarmParam.stAlarmTime.time[0][0],
                 bparam.stAlarmParam.stAlarmTime.time[0][0],
                 bparam.stAlarmParam.stAlarmTime.time[0][0] );

        len +=	sprintf( temp + len, "var alarm_schedule_sun_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[0][0] );
        len +=	sprintf( temp + len, "var alarm_schedule_sun_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[0][1] );
        len +=	sprintf( temp + len, "var alarm_schedule_sun_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[0][2] );
        len +=	sprintf( temp + len, "var alarm_schedule_mon_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[1][0] );
        len +=	sprintf( temp + len, "var alarm_schedule_mon_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[1][1] );
        len +=	sprintf( temp + len, "var alarm_schedule_mon_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[1][2] );
        len +=	sprintf( temp + len, "var alarm_schedule_tue_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[2][0] );
        len +=	sprintf( temp + len, "var alarm_schedule_tue_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[2][1] );
        len +=	sprintf( temp + len, "var alarm_schedule_tue_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[2][2] );
        len +=	sprintf( temp + len, "var alarm_schedule_wed_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[3][0] );
        len +=	sprintf( temp + len, "var alarm_schedule_wed_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[3][1] );
        len +=	sprintf( temp + len, "var alarm_schedule_wed_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[3][2] );
        len +=	sprintf( temp + len, "var alarm_schedule_thu_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[4][0] );
        len +=	sprintf( temp + len, "var alarm_schedule_thu_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[4][1] );
        len +=	sprintf( temp + len, "var alarm_schedule_thu_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[4][2] );
        len +=	sprintf( temp + len, "var alarm_schedule_fri_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[5][0] );
        len +=	sprintf( temp + len, "var alarm_schedule_fri_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[5][1] );
        len +=	sprintf( temp + len, "var alarm_schedule_fri_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[5][2] );
        len +=	sprintf( temp + len, "var alarm_schedule_sat_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[6][0] );
        len +=	sprintf( temp + len, "var alarm_schedule_sat_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[6][1] );
        len +=	sprintf( temp + len, "var alarm_schedule_sat_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[6][2] );
    }

    #ifdef CUSTOM_HTTP_ALARM
    /* BEGIN: Added by Baggio.wu, 2013/7/31 */
	len +=	sprintf( temp + len, "var alarmsvr=%d;\r\n", psdparam.bEnableAlarmSend );
    #endif

    memcpy( pbuf, temp, len );
    return len;
}
#endif

int cgigetcamera( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;
    //printf("read size %d\n",bparam.stVencParam.bysize);
    memset( temp, 0x00, 2048 );
    //camera param groups
    len += sprintf( temp + len, "var resolution=%d;\r\n", bparam.stVencParam.bysize );
    len += sprintf( temp + len, "var vbright=%d;\r\n", ( bparam.stVencParam.brightness == 0 ) ? 1 : bparam.stVencParam.brightness );
    len += sprintf( temp + len, "var vcontrast=%d;\r\n", bparam.stVencParam.contrast );
    len += sprintf( temp + len, "var vhue=%d;\r\n", bparam.stVencParam.chroma );
    len += sprintf( temp + len, "var vsaturation=%d;\r\n", bparam.stVencParam.saturation );
    len += sprintf( temp + len, "var OSDEnable=%d;\r\n", bparam.stVencParam.OSDEnable );
    len += sprintf( temp + len, "var mode=%d;\r\n", bparam.stVencParam.videoenv );
    len += sprintf( temp + len, "var flip=%d;\r\n", bparam.stVencParam.videomode );
    len += sprintf( temp + len, "var enc_framerate=%d;\r\n", bparam.stVencParam.byframerate );
    len += sprintf( temp + len, "var sub_enc_framerate=%d;\r\n", bparam.stVencParam.byframeratesub );
    len += sprintf( temp + len, "var speed=%d;\r\n", bparam.stPTZParam.byRate );
    len += sprintf( temp + len, "var enc_bitrate=%d;\r\n", bparam.stVencParam.bitrate );
    len += sprintf( temp + len, "var ircut=%d;\r\n", bparam.stVencParam.ircut );
    memcpy( pbuf, temp, len );
    return len;
}

int cgigetrecord( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;

	IETextout("Get Record CGI");

    memset( temp, 0x00, 2048 );
    //main rate encoder param
    len += sprintf( temp + len, "var enc_size=%d;\r\n", bparam.stVencParam.bysize );
    len += sprintf( temp + len, "var enc_framerate=%d;\r\n", bparam.stVencParam.byframerate );
    len += sprintf( temp + len, "var enc_keyframe=%d;\r\n", bparam.stVencParam.keyframe );
    len += sprintf( temp + len, "var enc_quant=%d;\r\n", bparam.stVencParam.quant );
    len += sprintf( temp + len, "var enc_ratemode=%d;\r\n", bparam.stVencParam.ratemode );
    len += sprintf( temp + len, "var enc_bitrate=%d;\r\n", bparam.stVencParam.bitrate );
    len += sprintf( temp + len, "var enc_main_mode=%d;\r\n", bparam.stVencParam.mainmode );
    //sub rate encoder param
    len += sprintf( temp + len, "var sub_enc_size=%d;\r\n", bparam.stVencParam.bysizesub );
    len += sprintf( temp + len, "var sub_enc_framerate=%d;\r\n", bparam.stVencParam.byframeratesub );
    len += sprintf( temp + len, "var sub_enc_keyframe=%d;\r\n", bparam.stVencParam.keyframesub );
    len += sprintf( temp + len, "var sub_enc_quant=%d;\r\n", bparam.stVencParam.quantsub );
    len += sprintf( temp + len, "var sub_enc_ratemode=%d;\r\n", bparam.stVencParam.ratemodesub );
    len += sprintf( temp + len, "var sub_enc_bitrate=%d;\r\n", bparam.stVencParam.bitratesub );
    len += sprintf( temp + len, "var enc_sub_mode=%d;\r\n", bparam.stVencParam.submode );
    //record param
    len += sprintf( temp + len, "var record_cover_enable=%d;\r\n", bparam.stRecordSet.recordover );
    len += sprintf( temp + len, "var record_timer=%d;\r\n", ( unsigned char )bparam.stRecordSet.timer );
    len += sprintf( temp + len, "var record_size=%d;\r\n", bparam.stRecordSet.size );
    len += sprintf( temp + len, "var record_time_enable=%d;\r\n", bparam.stRecordSet.timerenable );
    len += sprintf( temp + len, "var record_schedule_sun_0=%d;\r\n", bparam.stRecordSet.timerecord.time[0][0] );
    len += sprintf( temp + len, "var record_schedule_sun_1=%d;\r\n", bparam.stRecordSet.timerecord.time[0][1] );
    len += sprintf( temp + len, "var record_schedule_sun_2=%d;\r\n", bparam.stRecordSet.timerecord.time[0][2] );
    len += sprintf( temp + len, "var record_schedule_mon_0=%d;\r\n", bparam.stRecordSet.timerecord.time[1][0] );
    len += sprintf( temp + len, "var record_schedule_mon_1=%d;\r\n", bparam.stRecordSet.timerecord.time[1][1] );
    len += sprintf( temp + len, "var record_schedule_mon_2=%d;\r\n", bparam.stRecordSet.timerecord.time[1][2] );
    len += sprintf( temp + len, "var record_schedule_tue_0=%d;\r\n", bparam.stRecordSet.timerecord.time[2][0] );
    len += sprintf( temp + len, "var record_schedule_tue_1=%d;\r\n", bparam.stRecordSet.timerecord.time[2][1] );
    len += sprintf( temp + len, "var record_schedule_tue_2=%d;\r\n", bparam.stRecordSet.timerecord.time[2][2] );
    len += sprintf( temp + len, "var record_schedule_wed_0=%d;\r\n", bparam.stRecordSet.timerecord.time[3][0] );
    len += sprintf( temp + len, "var record_schedule_wed_1=%d;\r\n", bparam.stRecordSet.timerecord.time[3][1] );
    len += sprintf( temp + len, "var record_schedule_wed_2=%d;\r\n", bparam.stRecordSet.timerecord.time[3][2] );
    len += sprintf( temp + len, "var record_schedule_thu_0=%d;\r\n", bparam.stRecordSet.timerecord.time[4][0] );
    len += sprintf( temp + len, "var record_schedule_thu_1=%d;\r\n", bparam.stRecordSet.timerecord.time[4][1] );
    len += sprintf( temp + len, "var record_schedule_thu_2=%d;\r\n", bparam.stRecordSet.timerecord.time[4][2] );
    len += sprintf( temp + len, "var record_schedule_fri_0=%d;\r\n", bparam.stRecordSet.timerecord.time[5][0] );
    len += sprintf( temp + len, "var record_schedule_fri_1=%d;\r\n", bparam.stRecordSet.timerecord.time[5][1] );
    len += sprintf( temp + len, "var record_schedule_fri_2=%d;\r\n", bparam.stRecordSet.timerecord.time[5][2] );
    len += sprintf( temp + len, "var record_schedule_sat_0=%d;\r\n", bparam.stRecordSet.timerecord.time[6][0] );
    len += sprintf( temp + len, "var record_schedule_sat_1=%d;\r\n", bparam.stRecordSet.timerecord.time[6][1] );
    len += sprintf( temp + len, "var record_schedule_sat_2=%d;\r\n", bparam.stRecordSet.timerecord.time[6][2] );

    /* BEGIN: Added by wupm, 2013/4/11 */
	Lock_GetStorageSpace();
    len += sprintf( temp + len, "var record_sd_status=%d;\r\n", bparam.stRecordSet.sdstatus );
    //len += sprintf( temp + len, "var sdstatus=%d;\r\n", bparam.stStatusParam.sdstatus );
    len += sprintf( temp + len, "var sdtotal=%d;\r\n", sdtotal );
    len += sprintf( temp + len, "var sdfree=%d;\r\n", sdfree );
	IETextout("Return: SD Size = %d / %d", sdfree, sdtotal);
    UnLock_GetStorageSpace();

/* BEGIN: Added by wupm, 2013/7/1 */
    //IETextout("get record param,reserved=%02x", bparam.stAlarmParam.reserved);
    len +=  sprintf( temp + len, "var enable_record_audio=%d;\r\n", ( (bparam.stAlarmParam.reserved & 0x02) == 0x02 ) ? 1 : 0 );
    len +=  sprintf( temp + len, "var record_sensitivity=%d;\r\n", ((bparam.stAlarmParam.reserved & 0xF0) >> 4 ) & 0x0F );
/* END:   Added by wupm, 2013/7/1 */

    memcpy( pbuf, temp, len );
    return len;
}

int cgigetlogin( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char   temp[1024];
    unsigned char   temp1[128];
    int             len = 0;
    FILE*            fp = NULL;
    memset( temp, 0x00, 1024 );
    fp = fopen( "/param/login.cgi", "r" );

    if ( fp == NULL )
    {
        len += sprintf( temp + len, "login cgi is error\n\n" );
        memcpy( pbuf, temp, len );
        //IETextout("Return = [%s]", pbuf);
        return len;
    }

    while ( !feof( fp ) )
    {
        memset( temp1, 0x00, 128 );
        fgets( temp1, 128, fp );
        len += sprintf( temp + len, "%s\n", temp1 );
        IETextout("Return = [%s]", temp1);
    }

    fclose( fp );
    memcpy( pbuf, temp, len );
    //IETextout( "Return Buffer = [%s]", pbuf);
    //IETextout( "Return Len = %d", len );
    return len;
}

/* BEGIN: Modified by wupm, 2013/4/3 */
//int cgigetlog( unsigned char* pbuf, unsigned int rlen, unsigned int slen, unsigned char byPri )
int cgigetlog( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char   temp[1024 * 64];
    int             len = 0;

    /* BEGIN: Modified by wupm, 2013/4/3 */
    //char            name[128];
    char            name[256];

    char*            psrc = NULL;
    unsigned short  i;

    /* BEGIN: Deleted by wupm, 2013/4/3 */
    //short           rdcnt = rlen / 128 - 1;

    /* BEGIN: Added by wupm, 2013/4/3 */
#define	MAX_LOG_LEN	64 * 1024

    memset( temp, 0x00, 1024 * 64 );
    memset( plogbuf, 0x00, 1024 * 128 );
    logcnt = myReadLog( plogbuf );
    len += sprintf( temp + len, "var log_text=\"\";\r\n" );

    if ( logcnt > 0 )
    {
        psrc = plogbuf;

        for ( i = 0; i < logcnt; i++ )
        {
            memcpy( name, psrc, 128 );
            len += sprintf( temp + len, "log_text+=\"%s\\n\";\r\n", name );
            //IETextout("Return = [%s]", name);
            psrc += 128;

            /* BEGIN: Added by wupm, 2013/4/3 */
            if ( len + 256 >= MAX_LOG_LEN )
            {
                IETextout( "===========Total Count=%d, NOW Len = %d, break", logcnt, len );
                break;
            }
        }
    }

    memcpy( pbuf, temp, len );
    //IETextout( "Return Buffer = [%s]", pbuf);
    IETextout( "Return Len = %d", len );
    return len;
}

int cgigetalarmlog( unsigned char* pbuf, char* pparam, unsigned char byPri )
{
    int             len = 0;
    char            name[128];
    char*            psrc = NULL;
    unsigned short  i;
    /* BEGIN: Deleted by wupm, 2013/3/4 */
    //logcnt = ReadAlarmLog( plogbuf );
    //IETextout("logcnt %d ",logcnt);
    len += sprintf( pbuf + len, "var log_text=\"\";\r\n" );
    len += ReadAlarmLog( pbuf + len );
    //IETextout("Return Len = %d, = [%s]", len, pbuf);
    IETextout( "Return Len = %d", len );
    return len;
}

int cgisetalarmlogclr( unsigned char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    DoSystem( "rm /param/alarmlog.bin" );
    DoSystem( "rm /param/alarmlog1.bin" );
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

unsigned long GetFileSize( const char* filename )
{
    struct stat buf;

    //printf("filename:%s len:%d\n",filename,strlen(filename));
    //filename[24] = 0x00;
    if ( stat( filename, &buf ) < 0 )
    {
        return 0;
    }

    return ( unsigned long )buf.st_size;
}


/* BEGIN: Added by wupm, 2013/2/21 */
/* BEGIN: Deleted by wupm, 2013/2/25 */
#ifdef	GET_RECORDLIST_PAGE_BY_PAGE
static int bRecordFileListInited = 0;
typedef struct
{
    char szFile[28];
    int	nSize;
} TRecordFile;
#define	MAX_RECORD_COUNT 8192
typedef struct
{
    int nCount;
    TRecordFile oRecordFile[MAX_RECORD_COUNT];
} TRecordFileList;
TRecordFileList	gRecordFileList;
void ZeroRecordFileList()
{
    memset( &gRecordFileList, 0, sizeof( TRecordFileList ) );
}
int RecordFileListInited()
{
    return bRecordFileListInited;
}
/* BEGIN: Added by wupm, 2013/3/6 */
void SetRecordFileListInited( int bInit )
{
    bRecordFileListInited = bInit;
}
void TextoutFile( char* pbuf, int cmd )
{
#if 0
    FILE* fp = NULL;

    if ( cmd == 0x00 )
    {
        fp = fopen( "/tmp/File.txt", "ab+" );

        if ( fp == NULL )
        {
            printf( "dbg file open failed\n" );
            return;
        }
    }

    else
    {
        fp = fopen( "/tmp/File.txt", "wb" );

        if ( fp == NULL )
        {
            printf( "dbg file open failed\n" );
            return;
        }
    }

    fwrite( pbuf, 1, strlen( pbuf ), fp );
    fclose( fp );
#endif
}
void InitRecordFileList()
{
    int	nIndex = 0;
    FILE*	frecord = NULL;

    /*struct tm       *recordtm = NULL;
    struct tm       *recordtm2 = NULL;
    int 			temptime;
    temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
    recordtm = localtime( &temptime );
    IETextout("======================Init Start, %04d%-02d%-02d %02d:%02d:%02d",
    	recordtm->tm_year + 1900, recordtm->tm_mon + 1, recordtm->tm_mday,
    	recordtm->tm_hour, recordtm->tm_min, recordtm->tm_sec);*/

    time_t seconds1, seconds2;
    seconds1 = time( ( time_t* )NULL );
    IETextout( "======================Init Start, Time = %d", seconds1 );
    TextoutFile( "InitRecordFileList Start\n", 1 );

    Lock_GetRecordList();

    memset( &gRecordFileList, 0, sizeof( TRecordFileList ) );
    IETextout( "===ls Command Start" );
    TextoutFile( "Run ls Command\n", 0 );

    /* BEGIN: Modified by wupm, 2013/3/15 */
    //BUG: if TF Card is NULL, ls/mnt is ERROR, so ls /tmp NOT RUN
    //DoSystem( "rm -f /tmp/ok.txt && ls /mnt/*.h264 > /tmp/getfile.txt && ls /tmp > /tmp/ok.txt" );
    DoSystem( "rm -f /tmp/ok.txt" );
    DoSystem( "ls /mnt/*.h260 > /tmp/getfile.txt" );
    DoSystem( "ls /tmp > /tmp/ok.txt" );
    sleep( 1 );


    //ps -e | grep "sh -c ls /mnt/*.h264 > /tmp/getfile.txt && ls /tmp -l"
    //ps -e | grep "/mnt/*.h264"

    //Check ls Over ?
    while ( 1 )
    {
        FILE* f = fopen( "/tmp/ok.txt", "r" );

        if ( f == NULL )
        {
            sleep( 1 );
        }

        else
        {
            fclose( f );
            break;
        }
    }

    IETextout( "===ls Command Over" );
    TextoutFile( "ls Command Over, Now Open getfile.txt for read\n", 0 );

    //DoSystem( "sync");
    //DoSystem( "ls /tmp -l &");
    //sleep(60);
    IETextout( "======================Open File" );


    /* BEGIN: Modified by wupm, 2013/2/23 */
    //frecord = fopen( "/tmp/getfile.txt", "r" );
    //frecord = fopen( "/tmp/getfile.txt", "rb" );
    //DoSystem( "cp -f /tmp/getfile.txt /system/getfile.txt" );
    //IETextout("cp Command Over");
    //sleep(1);
    frecord = fopen( "/tmp/getfile.txt", "rb" );

    if ( frecord != NULL )
    {
        char temp1[256];
        char temp2[32];
        int filelen = 0;
        TextoutFile( "Open File Succ\n", 0 );

        while ( !feof( frecord ) )
        {
            memset( temp1, 0x00, 256 );

            /* BEGIN: Modified by wupm, 2013/2/23 */
            //fgets( temp1, 256, frecord );
            fread( temp1, 29, 1, frecord );
            //IETextout("Read Line %d = [%s]", gRecordFileList.nCount, temp1);
            //printf(".");
            memset( temp2, 0x00, 32 );
            memcpy( temp2, temp1 + 5, 23 );
            filelen = 0x00;
            temp1[28] = 0x00;
            filelen = GetFileSize( temp1 );

            if ( filelen == 0 )
            {
                IETextout( "FileSize == 0, FileName=[%s], continue", temp1 );
                continue;
            }

            gRecordFileList.oRecordFile[gRecordFileList.nCount].nSize = filelen;
            strcpy( gRecordFileList.oRecordFile[gRecordFileList.nCount].szFile, temp2 );
            gRecordFileList.nCount ++;

            if ( gRecordFileList.nCount >= MAX_RECORD_COUNT )
            {
                IETextout( "RecordFileCount more than MAX_RECORD_COUNT" );
                break;
            }
        }

        fclose( frecord );
    }

    else
    {
        IETextout( "Can not Open File getfile.txt for Read" );
        IETextout( "Open File ERROR\n", 0 );
    }

    //DoSystem( "rm -f /system/getfile.txt" );
    //IETextout("Remove TempFile Over");

    //20130223070000_010.h264
    /*
    if ( 1 )
    {
    	int i = 0;
    	char filename[64];
    	char buf[64] = {0};
    	for ( i = 0; i<3000; i++)
    	{
    		memset(filename, 0, 64);
    		sprintf(filename, "/mnt/2013022307%04d_111.h264", i);
    		FILE *f = fopen(filename, "wb");
    		fwrite(buf, 64, 1, f);
    		fclose(f);
    	}
    }
    */

    TextoutFile( "InitRecordFileList End\n", 0 );

    if ( 1 )
    {
        char tmp[256] = {0};
        sprintf( tmp, "gRecordFileList.nCount = %d\n", gRecordFileList.nCount );
        TextoutFile( tmp, 0 );
    }

    bRecordFileListInited = 1;
    UnLock_GetRecordList();

    /*
    temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
    recordtm2 = localtime( &temptime );
    IETextout("======================Init Over, %04d%-02d%-02d %02d:%02d:%02d",
    	recordtm2->tm_year + 1900, recordtm2->tm_mon + 1, recordtm2->tm_mday,
    	recordtm2->tm_hour, recordtm2->tm_min, recordtm2->tm_sec);

    temptime = (recordtm2->tm_hour * 60 * 60 + recordtm2->tm_min * 60 + recordtm2->tm_sec) -
    	(recordtm->tm_hour * 60 * 60 + recordtm->tm_min * 60 + recordtm->tm_sec);
    IETextout("======================Init Time = %d", temptime);*/

    seconds2 = time( ( time_t* )NULL );
    IETextout( "======================Init Over,  Time = %d", seconds2 );
    IETextout( "======================Init Total  Time = %d", seconds2 - seconds1 );
}
#define	DELETE_RECORDFILE_COUNT	5
int DeleteRecordFiles()
{
    int iRet=0;
    if ( gRecordFileList.nCount > 0 )
    {
        int nIndex = 0, nCount = 0;
        char szCommand[128] = {0};

        for (
            nIndex = gRecordFileList.nCount - 1, nCount = 0;
            nIndex >= 0 && nCount < DELETE_RECORDFILE_COUNT;
            nIndex --, nCount++ )
        {
            memset(szCommand, 0, 128);
            sprintf( szCommand, "rm -f /mnt/%s", gRecordFileList.oRecordFile[nIndex].szFile );
            DoSystem( szCommand );
            IETextout( "%s", szCommand );

            /* BEGIN: Added by Baggio.wu, 2013/7/8 */
            iRet = access( szCommand+6, F_OK );
            if ( iRet == 0x00 )
            {
                DoSystem( "mount -o rw,remount /mnt" );
                printf( "del filename failed[%s]\n", szCommand);
                break;
            }
        }
    }

    return iRet;
}
void DeleteRecordFileList()
{
    int iRet = 0;
    if ( !RecordFileListInited() )
    {
        //IETextout("!RecordFileListInited(), Dothing..");
        return;
    }

    Lock_GetRecordList();
    iRet = DeleteRecordFiles();
    if(iRet != 0)
    {
        //2 nCount  ---- 5
        if ( gRecordFileList.nCount >= DELETE_RECORDFILE_COUNT )
        {
            gRecordFileList.nCount -= DELETE_RECORDFILE_COUNT;
        }

        else
        {
            gRecordFileList.nCount = 0;
        }
    }

    UnLock_GetRecordList();
}
void DeleteRecordFile( char szFile[64] )
{
    int iRet=0;
    Lock_GetRecordList();

    if ( gRecordFileList.nCount > 0 )
    {
        int nIndex = 0;
        char szCommand[128] = {0};

        for (
            nIndex = 0;
            nIndex < gRecordFileList.nCount;
            nIndex ++ )
        {
            if ( strcmp( gRecordFileList.oRecordFile[nIndex].szFile, szFile ) == 0 )
            {
                sprintf( szCommand, "rm -f /mnt/%s", gRecordFileList.oRecordFile[nIndex].szFile );
                IETextout( "%s", szCommand );
                DoSystem( szCommand );

                if ( 1 )
                {
                    char tmp[256] = {0};
                    sprintf( tmp, "DeleteRecordFile() %s\n", szCommand );
                    TextoutFile( tmp, 0 );
                }

                for ( nIndex = nIndex + 1; nIndex < gRecordFileList.nCount; nIndex ++ )
                {
                    memcpy(	&gRecordFileList.oRecordFile[nIndex - 1], &gRecordFileList.oRecordFile[nIndex], sizeof( TRecordFile ) );
                }

                gRecordFileList.nCount --;

            /* BEGIN: Added by Baggio.wu, 2013/7/8 */
            iRet = access( szCommand+6, F_OK );
            if ( iRet == 0x00 )
            {
                DoSystem( "mount -o rw,remount /mnt" );
                printf( "del filename failed[%s]\n", szCommand);
                break;
            }

           /* END:   Added by Baggio.wu, 2013/7/8 */

                break;
            }
        }
    }

    UnLock_GetRecordList();
}
void Append2RecordFileList( char szFile[28], int nSize )
{
    int i;

    if ( !RecordFileListInited() )
    {
        //IETextout("!RecordFileListInited(), Dothing..");
        return;
    }

    Lock_GetRecordList();

    if ( gRecordFileList.nCount >= MAX_RECORD_COUNT )
    {
        IETextout( "RecordFileCount more than MAX_RECORD_COUNT, return" );
        TextoutFile( "Append:RecordFileCount more than MAX_RECORD_COUNT, return\n", 0 );
        UnLock_GetRecordList();
        return;
    }

    if ( gRecordFileList.nCount > 0 )
    {
        for ( i = gRecordFileList.nCount - 1; i >= 0; i-- )
        {
            memcpy(	&gRecordFileList.oRecordFile[i + 1], &gRecordFileList.oRecordFile[i], sizeof( TRecordFile ) );
        }
    }

    gRecordFileList.oRecordFile[0].nSize = nSize;
    strcpy( gRecordFileList.oRecordFile[0].szFile, szFile );
    gRecordFileList.nCount ++;
    UnLock_GetRecordList();
}
int GetRecordFileSize( int nIndex )
{
    if ( !RecordFileListInited() || nIndex >= gRecordFileList.nCount || nIndex >= MAX_RECORD_COUNT )
    {
        //IETextout("!RecordFileListInited(), return 0");
        return 0;
    }

    return gRecordFileList.oRecordFile[nIndex].nSize;
}
char* GetRecordFile( int nIndex )
{
    if ( !RecordFileListInited() || nIndex >= gRecordFileList.nCount || nIndex >= MAX_RECORD_COUNT )
    {
        //IETextout("!RecordFileListInited(), return NULL");
        return NULL;
    }

    return gRecordFileList.oRecordFile[nIndex].szFile;
}
int GetRecordFileCount()
{
    if ( !RecordFileListInited() )
    {
        //IETextout("!RecordFileListInited(), return 0");
        return 0;
    }

    return gRecordFileList.nCount;
}
#endif


/* END:   Added by wupm, 2013/2/21 */

#ifndef	GET_RECORDLIST_PAGE_BY_PAGE
int cgigetrecordfile( unsigned char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   	temp[1024 * 64];
    unsigned char   	temp1[128];
    char            		temp2[24];
    char            		temp3[128];
    int             		len = 0;
    FILE*           		 frecord = NULL;
    char            		namebackup[24];
    int				iRet = 0;
    int				recordcnt = 0;
    unsigned int		startdate = 0;
    unsigned int		enddate = 0;
    char				flag = 0x00;
    struct stat     		stStat;
    unsigned int		filelen;
    char				filesize[128];
    memset( temp, 0x00, 1024 * 64 );
    printf( "pbuf:%s\n", pparam );
    len += sprintf( temp + len, "var record_name0=new Array();\r\n" );
    len += sprintf( temp + len, "var record_size0=new Array();\r\n" );
    iRet = GetIntParamValue( pparam, "startdate", &startdate );

    if ( iRet != 0x00 )
    {
        flag = 0x01;
    }

    iRet = GetIntParamValue( pparam, "enddate", &enddate );

    if ( iRet != 0x00 )
    {
        flag = 0x01;
    }

    memset( filesize, 0x00, 128 );
    printf( "flag %d startdate %d enddate %d\n", flag, startdate, enddate );

    if ( ( flag == 0x00 ) && ( startdate != 0x00 ) && ( enddate != 0x00 ) )
    {
        printf( "find record file\n" );
        DoSystem( "ls /mnt/*.h260 > /tmp/getfile.txt" );
        frecord = fopen( "/tmp/getfile.txt", "r" );

        if ( frecord != NULL )
        {
            while ( !feof( frecord ) )
            {
                memset( temp1, 0x00, 128 );
                fgets( temp1, 256, frecord );
                memset( temp2, 0x00, 24 );
                memcpy( temp2, temp1 + 5, 23 );
                memset( temp3, 0x00, 128 );
                filelen = 0x00;
                //printf("temp1:%s\n",temp1);
                temp1[28] = 0x00;
                filelen = GetFileSize( temp1 );

                if ( filelen == 0 || temp1[0] == 0 )
                {
                    continue;
                }

                len += sprintf( temp + len, "record_name0[%d]=\"%s\";\r\n", recordcnt, temp2 );
                len += sprintf( temp + len, "record_size0[%d]=%d;\r\n", recordcnt, filelen );
                recordcnt++;

                /* BEGIN: Deleted by wupm, 2013/3/19 */
                //if ( recordcnt >= 192 )
                //{
                //	break;
                //}

                if ( len >= 1024 * 60 )
                {
                    break;
                }
            }

            fclose( frecord );
        }

        /*
        if( recordcnt > 0 )
        {
        	recordcnt--;
        }
        */
    }

    len += sprintf( temp + len, "var record_num0=%d;\r\n", recordcnt );
    memcpy( pbuf, temp, len );
    return len;
}
#else
int cgigetrecordfile( unsigned char* pbuf, char* pparam, unsigned char byPri )
{
    //Lock_GetRecordFile();
    unsigned char   	temp[1024 * 128];
    unsigned char   	temp1[128];
    char            		temp2[24];
    char            		temp3[128];
    int             		len = 0;
    FILE*           		 frecord = NULL;
    unsigned int		startdate = 0;
    unsigned int		enddate = 0;
    char				flag = 0x00;
    struct stat     		stStat;
    unsigned int		filelen;
    char				filesize[128];
    int				recordcnt = 0;
    int iRet = 0;

    /* BEGIN: Added by wupm, 2013/1/31   Version:159 */
#define	MAX_PAGE_SIZE	4096
    int	m_nPageIndex = 0;			//IN(Default = 1), OUT
    int m_nPageSize = 0;			//IN(Default = 0), OUT
    int m_nPageCount = 0;			//OUT
    int m_nRecordCount = 0;			//OUT

    memset( temp, 0x00, 1024 * 128 );
    //printf( "pbuf:%s\n", pparam );
    len += sprintf( temp + len, "var record_name0=new Array();\r\n" );
    len += sprintf( temp + len, "var record_size0=new Array();\r\n" );

    /* BEGIN: Deleted by wupm, 2013/1/31 */
    /*
    iRet = GetIntParamValue( pparam, "startdate", &startdate );

    if( iRet != 0x00 )
    {
    	flag = 0x01;
    }

    iRet = GetIntParamValue( pparam, "enddate", &enddate );

    if( iRet != 0x00 )
    {
    	flag = 0x01;
    }
    */
    /* END: Deleted by wupm, 2013/1/31 */

    /* BEGIN: Added by wupm, 2013/1/31 */

    //Get PageSize

    iRet = GetIntParamValue( pparam, "PageSize", &m_nPageSize );

    if ( iRet != 0x00 )
    {
        m_nPageSize = 0;
    }

    else if ( m_nPageSize >= 0 && m_nPageSize <= MAX_PAGE_SIZE )
    {
    }
    else
    {
        m_nPageSize = 0;
    }

    //Get PageIndex

    iRet = GetIntParamValue( pparam, "PageIndex", &m_nPageIndex );

    if ( iRet != 0x00 )
    {
        m_nPageIndex = 0;
    }

    else if ( m_nPageIndex >= 0 )
    {

    }
    else
    {
        m_nPageIndex = 0;
    }

    //Set to Default Value

    if ( m_nPageSize == 0 )
    {
        m_nPageSize = MAX_PAGE_SIZE;
        m_nPageIndex = 0;
    }

    /* END:   Added by wupm, 2013/1/31 */

    memset( filesize, 0x00, 128 );

    if ( 1 )
    {
        printf( "find record file\n" );

        /* BEGIN: Deleted by wupm, 2013/2/21 */
        /*
        Lock_GetRecordList();
        DoSystem( "ls /mnt/*.h264 > /tmp/getfile.txt" );
        UnLock_GetRecordList();
        */

        /*
        DoSystem("cat /tmp/getfile.txt > /tmp/wupm");
        DoSystem("cat /tmp/getfile.txt >> /tmp/wupm");
        DoSystem("cat /tmp/getfile.txt >> /tmp/wupm");
        DoSystem("cat /tmp/getfile.txt >> /tmp/wupm");
        DoSystem("cat /tmp/getfile.txt >> /tmp/wupm");
        DoSystem("cat /tmp/getfile.txt >> /tmp/wupm");
        DoSystem("cat /tmp/getfile.txt >> /tmp/wupm");
        DoSystem("cat /tmp/getfile.txt >> /tmp/wupm");
        DoSystem("cat /tmp/getfile.txt >> /tmp/wupm");
        DoSystem("cat /tmp/getfile.txt >> /tmp/wupm");
        DoSystem("mv /tmp/wupm /tmp/getfile.txt");
        */

        /* BEGIN: Added by wupm, 2013/2/19 */
        Lock_GetRecordFile();

        /* BEGIN: Deleted by wupm, 2013/2/21 */
        if ( 1 )
        {
            int nStartIndex = m_nPageIndex * m_nPageSize;
            int nStopIndex = nStartIndex + m_nPageSize;
            int nRecordCount = GetRecordFileCount();	//gRecordFileList.nCount;
            int nIndex = 0;
            char* pFile = NULL;

            for ( nIndex = nStartIndex; nIndex < nRecordCount && nIndex < nStopIndex; nIndex++, recordcnt++ )
            {
                pFile = GetRecordFile( nIndex );

                if ( pFile == NULL )
                {
                    continue;
                }

                len += sprintf( temp + len, "record_name0[%d]=\"%s\";\r\n",
                                nIndex - nStartIndex,
                                pFile
                                //gRecordFileList.oRecordFile[nIndex].szFile
                              );
                len += sprintf( temp + len, "record_size0[%d]=%d;\r\n",
                                nIndex - nStartIndex,
                                GetRecordFileSize( nIndex )
                                //gRecordFileList.oRecordFile[nIndex].nSize
                              );

                if ( len >= 1024 * 120 )
                {
                    m_nPageSize = recordcnt;
                    break;
                }
            }

            m_nRecordCount = nRecordCount;
            m_nPageCount = ( m_nRecordCount + m_nPageSize - 1 ) / m_nPageSize;
        }

        UnLock_GetRecordFile();
    }

    //InitRecordFileList();

    IETextout( "m_nPageIndex = %d, m_nPageSize = %d, m_nRecordCount = %d, m_nPageCount = %d",
             m_nPageIndex, m_nPageSize, m_nRecordCount, m_nPageCount );

    len += sprintf( temp + len, "var record_num0=%d;\r\n", recordcnt );
    len += sprintf( temp + len, "var PageIndex=%d;\r\n", m_nPageIndex );
    len += sprintf( temp + len, "var PageSize=%d;\r\n", m_nPageSize );
    len += sprintf( temp + len, "var RecordCount=%d;\r\n", m_nRecordCount );
    len += sprintf( temp + len, "var PageCount=%d;\r\n", m_nPageCount );
    memcpy( pbuf, temp, len );

    /*
    {
    	FILE *f = fopen("/tmp/zxh", "wb");
    	fwrite(pbuf, 1, len, f);
    	fclose(f);
    }
    */
    IETextout( "==============Len = %d", len );
    //UnLock_GetRecordFile();
    return len;
}
#endif

int cgigetftptest( unsigned char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;
    int nFtpResult = 0;

    memset( temp, 0x00, 2048 );
    nFtpResult = GetFtpTestResult();
    len += sprintf( temp + len, "var result=%d;\r\n", nFtpResult );
    memcpy( pbuf, temp, len );
    return len;
}

int cgisetftptest( unsigned char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             result = 0;
    char            nexturl[64];
    int             iRet = 0;
    static	char	flag = 0x00;
    memset( temp, 0x00, 2048 );

    if ( flag == 0x00 )
    {
        flag = 0x01;
        DoFtpTest();
        flag = 0x00;
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgigetmailtest( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;
    int nMailResult = 0;

    memset( temp, 0x00, 2048 );
    nMailResult = 0;
    len += sprintf( temp + len, "var result=%d;\r\n", nMailResult );
    memcpy( pbuf, temp, len );
    return len;
}

int cgisetmailtest( unsigned char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             result = 0;
    char            nexturl[64];
    int		iRet = 0;
    memset( temp, 0x00, 2048 );

    //iRet = DoMailTest();
    if(iRet == 0)
    {
        IETextout("Mail send over, OK or Not");
    }
    /* END:   Added by Baggio.wu, 2013/10/25 */

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
#if 1
        len += RefreshUrl( temp + len, nexturl );
#endif
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    printf( "sendmail len:%d\n", len );
    return len;
}

int cgigetwifiscan( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char temp[1024 * 32];
    int	len = 0;
    char    ssid[64];
    char    logcnt = 0;
    char    i, dir = 0;
    int     logtotal = 0;
    char    temp1[256];
    char    temp2[18];
    char    temp3[8];
    char    temp4[4];
    char    temp5[4];
    printf( "wificnt %d\n", wificnt );
    memset( temp, 0x00, 4096 );
    len += sprintf( temp + len, "var ap_number=%d;\r\n", wificnt );
    len += sprintf( temp + len, "var ap_ssid=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_mode=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_security=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_dbm0=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_dbm1=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_mac=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_channel=new Array();\r\n" );

    for ( i = 0; i < wificnt; i++ )
    {
        if ( i > 31 )
        {
            break;
        }

        memset( ssid, 0x00, 64 );
        memset( temp1, 0x00, 256 );
        memset( temp2, 0x00, 18 );
        memcpy( temp2, WifiResult[i].mac, 17 );
        memset( temp3, 0x00, 8 );
        memcpy( temp3, WifiResult[i].mode, 5 );
        memset( temp4, 0x00, 4 );
        memset( temp5, 0x00, 4 );
        memcpy( temp4, WifiResult[i].db0, strlen( WifiResult[i].db0 ) );
        memcpy( temp5, WifiResult[i].db1, strlen( WifiResult[i].db1 ) );

        switch ( WifiResult[i].Authtype )
        {
            case 0x00:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=0;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\'%s\';\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\'%s\';\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;

            case 0x01:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=1;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\'%s\';\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\'%s\';\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;

            case 0x02:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=2;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\'%s\';\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\'%s\';\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;

            case 0x03:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=3;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\'%s\';\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\'%s\';\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;

            case 0x04:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=4;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\'%s\';\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\'%s\';\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;

            case 0x05:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=5;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\'%s\';\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\'%s\';\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;
        }
    }

    memcpy( pbuf, temp, len );
    return len;
}

int GetFactoryCgi( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             result = 0;
    memset( temp, 0x00, 2048 );
    len += sprintf( temp + len, "var factory_server=\"%s\";\r\n", bparam.stDdnsParam.serversvr );
    len += sprintf( temp + len, "var factory_user=\"%s\";\r\n", bparam.stDdnsParam.serveruser );
    len += sprintf( temp + len, "var factory_passwd=\"%s\";\r\n", bparam.stDdnsParam.serverpwd );
    len += sprintf( temp + len, "var factory_heatbeat=%d;\r\n", bparam.stDdnsParam.serverbeat );
    len += sprintf( temp + len, "var factory_port=%d;\r\n", bparam.stDdnsParam.serverport );
    len += sprintf( temp + len, "var factory_index=%d;\r\n", bparam.stDdnsParam.factoryversion );
    len += sprintf( temp + len, "var factory_mode=%d;\r\n", bparam.stDdnsParam.factorymode );
    result = GetFactoryStatus();
    len += sprintf( temp + len, "var factory_status=%d;\r\n", result );
    #ifdef CHANGE_DDNS_ALARM_SERVER
    len += sprintf( temp + len, "var factory_alarmserver=\"%s\";\r\n", vstarparam.alarm_svr);
    #endif

    memcpy( pbuf, temp, len );
	IETextout("return len=%d", len);
    return len;
}

int cgisetalias( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   	temp[2048];
    int             		len  = 0;
    int             		iRet = 0;
    char            		loginuse[32];
    char            		loginpas[32];
    char            		nexturl[64];
    char            		szDeviceName[128];
    char				decoderbuf[128];

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    //printf("pparams:%s len:%d\n",pparam,strlen(pparam));
    memset( temp, 0x00, 2048 );
    memset( szDeviceName, 0x00, 128 );
    iRet = GetStrParamValue( pparam, "alias", szDeviceName, 79 );

    if ( iRet == 0x00 )
    {
        memset( bparam.stIEBaseParam.szDevName, 0x00, 80 );
        memset( decoderbuf, 0x00, 128 );
        URLDecode( szDeviceName, strlen( szDeviceName ), decoderbuf, 79 );
        //printf("pparams:%s len:%d\n",szDeviceName,strlen(szDeviceName));
        //printf("pparams:%s len:%d\n",decoderbuf,strlen(decoderbuf));
        strcpy( bparam.stIEBaseParam.szDevName, decoderbuf );

        /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_ALIAS
        myWriteLogIp( 0, LOG_SET_ALIAS );
#endif
#endif
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    NotifyToDaemon( 0x00, &bparam, sizeof( IEPARAMLIST ) );
    return len;
}

int cgisetdate( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    char            szNtpServer[64];
    int             dwCurTime = 0;
    int		bNtpEnable = 0;
    int		byTzSel = 0;
    char				decoderbuf[128];

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    IETextout( "set date start..., pparam=[%s]", pparam );
    iRet = GetIntParamValue( pparam, "now", &dwCurTime );

    if ( iRet == 0x00 )
    {
        bparam.stDTimeParam.dwCurTime = ( unsigned int )dwCurTime;
        bparam.stDTimeParam.byIsPCSync = 1;
    }

    else
    {
        bparam.stDTimeParam.byIsPCSync = 0;
    }

    iRet = GetIntParamValue( pparam, "tz", &byTzSel );

    if ( iRet == 0x00 )
    {
        bparam.stDTimeParam.byTzSel = byTzSel;
    }

    memset( szNtpServer, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "ntp_svr", szNtpServer, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( szNtpServer, strlen( szNtpServer ), decoderbuf, 31 );
        memset( bparam.stDTimeParam.szNtpSvr, 0x00, 32 );
        memcpy( bparam.stDTimeParam.szNtpSvr, decoderbuf, 32 );
    }

    iRet = GetIntParamValue( pparam, "ntp_enable", &bNtpEnable );

    if ( iRet == 0x00 )
    {
        bparam.stDTimeParam.byIsNTPServer = bNtpEnable;
        SetNtpRestart( bparam.stDTimeParam.byIsNTPServer );
        printf( "ntp enable %d\n", bparam.stDTimeParam.byIsNTPServer );
    }

    PcDateSync();
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_DATE
    myWriteLogIp( 0, LOG_SET_DATE );
#endif
#endif

    //IETextout("Return Len = %d, Return = [%s]", len, temp);
    //WriteDate(bparam.stDTimeParam.dwCurTime);
    return len;
}

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
#if	0
int cgisetdevice( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    char            temp2[80];
    int             iValue;
    char		i;
    char				decoderbuf[128];

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    memset( &bparam.stMultDevice, 0, sizeof( MULTDEVICE ) * 8 );

    for ( i = 0; i < 8; i++ )
    {
        memset( temp1, 0x00, 24 );
        sprintf( temp1, "dev%d_host", i + 2 );
        memset( temp2, 0x00, 80 );
        iRet = GetStrParamValue( pparam, temp1, temp2, 63 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
            memset( bparam.stMultDevice[i].host, 0x00, 64 );
            strcpy( bparam.stMultDevice[i].host, decoderbuf );
        }

        memset( temp1, 0x00, 24 );
        sprintf( temp1, "dev%d_alias", i + 2 );
        memset( temp2, 0x00, 80 );
        iRet = GetStrParamValue( pparam, temp1, temp2, 79 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 79 );
            memset( bparam.stMultDevice[i].alias, 0x00, 80 );
            strcpy( bparam.stMultDevice[i].alias, decoderbuf );
        }

        sprintf( temp1, "dev%d_port", i + 2 );
        GetIntParamValue( pparam, temp1, &iValue );
        bparam.stMultDevice[i].port = iValue;
        memset( temp1, 0x00, 24 );
        sprintf( temp1, "dev%d_user", i + 2 );
        memset( temp2, 0x00, 80 );
        iRet = GetStrParamValue( pparam, temp1, temp2, 31 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
            memset( bparam.stMultDevice[i].user, 0x00, 32 );
            strcpy( bparam.stMultDevice[i].user, decoderbuf );
        }

        memset( temp1, 0x00, 24 );
        sprintf( temp1, "dev%d_pwd", i + 2 );
        memset( temp2, 0x00, 80 );
        iRet = GetStrParamValue( pparam, temp1, temp2, 31 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
            memset( bparam.stMultDevice[i].passwd, 0x00, 32 );
            strcpy( bparam.stMultDevice[i].passwd, decoderbuf );
        }
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "ok\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}
#endif

int cgisetnetwork( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[16];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int		value;
    char				decoderbuf[128];

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    printf( "pparam:%s\n", pparam );
    memset( temp1, 0x00, 16 );
    iRet = GetStrParamValue( pparam, "ipaddr", temp1, 15 );

    if ( iRet == 0x00 )
    {
        iRet = CheckIPok( temp1 );

        if ( iRet == 0x00 )
        {
            memset( bparam.stNetParam.szIpAddr, 0x00, 16 );
            strcpy( bparam.stNetParam.szIpAddr, temp1 );
        }
    }

    memset( temp1, 0x00, 16 );
    iRet = GetStrParamValue( pparam, "mask", temp1, 15 );

    if ( iRet == 0x00 )
    {
        iRet = CheckIPok( temp1 );

        if ( iRet == 0x00 )
        {
            memset( bparam.stNetParam.szMask, 0x00, 16 );
            strcpy( bparam.stNetParam.szMask, temp1 );
        }
    }

    memset( temp1, 0x00, 16 );
    iRet = GetStrParamValue( pparam, "gateway", temp1, 15 );

    if ( iRet == 0x00 )
    {
        iRet = CheckIPok( temp1 );

        if ( iRet == 0x00 )
        {
            memset( bparam.stNetParam.szGateway, 0x00, 16 );
            strcpy( bparam.stNetParam.szGateway, temp1 );
        }
    }

    memset( temp1, 0x00, 16 );
    iRet = GetStrParamValue( pparam, "dns1", temp1, 15 );

    if ( iRet == 0x00 )
    {
        iRet = CheckIPok( temp1 );

        if ( iRet == 0x00 )
        {
            memset( bparam.stNetParam.szDns1, 0x00, 16 );
            strcpy( bparam.stNetParam.szDns1, temp1 );
        }
    }

    memset( temp1, 0x00, 16 );
    iRet = GetStrParamValue( pparam, "dns2", temp1, 15 );

    if ( iRet == 0x00 )
    {
        iRet = CheckIPok( temp1 );

        if ( iRet == 0x00 )
        {
            memset( bparam.stNetParam.szDns2, 0x00, 16 );
            strcpy( bparam.stNetParam.szDns2, temp1 );
        }
    }

    iRet = GetIntParamValue( pparam, "port", &value );

    if ( iRet == 0x00 )
    {
        if ( value >= 80 )
        {
            bparam.stNetParam.nPort = value;
        }
    }

    iRet = GetIntParamValue( pparam, "dhcp", &value );

    if ( iRet == 0x00 )
    {
        bparam.stNetParam.byIsDhcp = value;
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_NETWORK
    myWriteLogIp( 0, LOG_SET_NETWORK );
#endif
#endif

    return len;
}

int cgisetpppoe( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[24];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    char            szUserName[64];
    char            szPassword[64];
    int		iValue;
    char				decoderbuf[128];

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    memset( &bparam.stPppoeParam, 0x00, sizeof( PPPOEPARAM ) );
    memset( szUserName, 0, 64 );
    memset( szPassword, 0, 64 );
    printf( "pparam:%s\n", pparam );
    iRet = GetStrParamValue( pparam, "user", szUserName, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( szUserName, strlen( szUserName ), decoderbuf, 63 );
        memset( bparam.stPppoeParam.szUserName, 0x00, 63 );
        strcpy( bparam.stPppoeParam.szUserName, decoderbuf );
    }

    iRet = GetStrParamValue( pparam, "pwd", szPassword, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( szPassword, strlen( szPassword ), decoderbuf, 63 );
        memset( bparam.stPppoeParam.szPassword, 0x00, 63 );
        strcpy( bparam.stPppoeParam.szPassword, decoderbuf );
    }

    iRet = GetIntParamValue( pparam, "enable", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stPppoeParam.byEnable = iValue;
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgisetreboot( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[24];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char				decoderbuf[128];
    printf( "pparam:%s\n", pparam );

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var setreboot result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_REBOOT
    myWriteLogIp( 0, LOG_REBOOT );
#endif
#endif

    return len;
}


int cgisetupnp( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[24];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char				decoderbuf[128];

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    iRet = GetIntParamValue( pparam, "enable", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stUpnpParam.byEnable = iValue;
        //bparam.stStatusParam.upnpstat = iValue;
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgisetddns( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[24];
    char			temp2[64];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char				decoderbuf[128];

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    iRet = GetIntParamValue( pparam, "service", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stDdnsParam.byDdnsSvr = iValue;
        IETextout( "Sevice ID = %d", iValue );
    }

    iRet = GetIntParamValue( pparam, "enable", &iValue );

    if ( iRet == 0 )
    {
        bparam.stDdnsParam.enable = iValue;
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "user", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stDdnsParam.szUserName, 0x00, 32 );
        strcpy( bparam.stDdnsParam.szUserName, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "pwd", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( bparam.stDdnsParam.szPassword, 0x00, 32 );
        strcpy( bparam.stDdnsParam.szPassword, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "host", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stDdnsParam.szDdnsName, 0x00, 64 );
        strcpy( bparam.stDdnsParam.szDdnsName, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "proxy_svr", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stDdnsParam.szProxySvr, 0x00, 64 );
        strcpy( bparam.stDdnsParam.szProxySvr, decoderbuf );
        IETextout( "Proxy_Server = [%s]", decoderbuf );
    }

    iRet = GetIntParamValue( pparam, "proxy_port", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stDdnsParam.nProxyPort = iValue;
    }

    iRet = GetIntParamValue( pparam, "ddns_mode", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stDdnsParam.byMode = iValue;
    }

    IETextout( "Start DDNS" );
    SetThirdStatus();
    SetDdnsStart();
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgisetmail( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   	temp[2048];
    char            		temp1[24];
    char				temp2[64];
    int             		len  = 0;
    int             		iRet = 0;
    char            		passtemp[32];
    char            		usertemp[32];
    char            		nexturl[64];
    int             		value;
    int             		iValue;
    char				decoderbuf[128];
    //printf( "cgi param:%s\n", pparam );
    //IETextout("Enter SetMail");

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    memset( &bparam.stMailParam, 0x00, sizeof( MAILPARAM ) );
    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "sender", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szSender, 0x00, 64 );
        strcpy( bparam.stMailParam.szSender, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet =  GetStrParamValue( pparam, "receiver1", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szReceiver1, 0x00, 64 );
        strcpy( bparam.stMailParam.szReceiver1, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "receiver2", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szReceiver2, 0x00, 64 );
        strcpy( bparam.stMailParam.szReceiver2, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "receiver3", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szReceiver3, 0x00, 64 );
        strcpy( bparam.stMailParam.szReceiver3, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "receiver4", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szReceiver4, 0x00, 64 );
        strcpy( bparam.stMailParam.szReceiver4, decoderbuf );
    }

    IETextout( "--------------------1-------------" );
    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "svr", temp2, 63 );
    IETextout( "--------------------2-------------" );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        IETextout( "--------------------3-------------" );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        IETextout( "--------------------4-------------" );
        memset( bparam.stMailParam.szSmtpSvr, 0x00, 64 );
        strcpy( bparam.stMailParam.szSmtpSvr, decoderbuf );
        IETextout( "--------------------5-------------" );
    }

	/* BEGIN: Modified by wupm, 2013/7/4 */
    //GetIntParamValue( pparam, "port", &value );
    GetIntParamValue( pparam, "smtpport", &value );

    bparam.stMailParam.nSmtpPort = value;
    GetIntParamValue( pparam, "mail_inet_ip", &value );
    bparam.stMailParam.byNotify = value;
    memset( temp2, 0x00, 63 );
    iRet = GetStrParamValue( pparam, "user", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szSmtpUser, 0x00, 63 );
        strcpy( bparam.stMailParam.szSmtpUser, decoderbuf );
    }

    memset( temp2, 0x00, 63 );
    iRet = GetStrParamValue( pparam, "pwd", temp2, 63 );
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szSmtpPwd, 0x00, 63 );
        strcpy( bparam.stMailParam.szSmtpPwd, decoderbuf );
    }
    GetIntParamValue( pparam, "mail_inet_ip", &value );
    bparam.stMailParam.byNotify = value;
    GetIntParamValue( pparam, "ssl", &value );
    bparam.stMailParam.byCheck = value;
    //EmailConfig();
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_EMAIL
    myWriteLogIp( 0, LOG_SET_EMAIL );
#endif
#endif

    return len;
}

int cgisetftp( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char		temp2[64];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char				decoderbuf[128];

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    memset( &bparam.stFtpParam, 0x00, sizeof( FTPPARAM ) );
    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "svr", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stFtpParam.szFtpSvr, 0x00, 64 );
        strcpy( bparam.stFtpParam.szFtpSvr, decoderbuf );
    }

    GetIntParamValue( pparam, "port", &iValue );
    bparam.stFtpParam.nFtpPort = iValue;
    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "user", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( bparam.stFtpParam.szFtpUser, 0x00, 32 );
        strcpy( bparam.stFtpParam.szFtpUser, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "pwd", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( bparam.stFtpParam.szFtpPwd, 0x00, 32 );
        strcpy( bparam.stFtpParam.szFtpPwd, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "dir", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( bparam.stFtpParam.szFtpDir, 0x00, 32 );
        strcpy( bparam.stFtpParam.szFtpDir, decoderbuf );
        if(decoderbuf[0] == 0)
        {
            strcpy(bparam.stFtpParam.szFtpDir, "/" );
        }
    }

    GetIntParamValue( pparam, "mode", &iValue );
    bparam.stFtpParam.byMode = iValue;
    GetIntParamValue( pparam, "upload_interval", &iValue );
    bparam.stFtpParam.nInterTime = iValue;
    memset( temp1, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "filename", temp1, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stFtpParam.szFileName, 0x00, 64 );
        strcpy( bparam.stFtpParam.szFileName, decoderbuf );
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_FTP
    myWriteLogIp( 0, LOG_SET_FTP );
#endif
#endif

    return len;
}

int cgisetrtsp( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char				decoderbuf[128];

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
#if 0
    memset( &bparam.stRtspParam, 0x00, sizeof( RTSPPARAM ) );
    iRet = GetStrParamValue( pparam, "user", bparam.stRtspParam.szUserName, 63 );
    iRet = GetStrParamValue( pparam, "pwd", bparam.stRtspParam.szPassword, 63 );
    GetIntParamValue( pparam, "enable", &iValue );
    bparam.stRtspParam.byEnable = iValue;
#endif
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

/* BEGIN: Added by wupm, 2013/3/12 */
#ifdef	HONESTECH
THTCLASS_PARAMETERS	oHTParams;
#define	HT_PARAMTERS_FILE	"/param/HTClass.bin"
pthread_mutex_t mutex_HTFile = PTHREAD_MUTEX_INITIALIZER;
void LockHTFile()
{
    pthread_mutex_lock( &mutex_HTFile );
}
void UnLockHTFile()
{
    pthread_mutex_unlock( &mutex_HTFile );
}
void ReadHTClassParams()
{
    FILE* f = NULL;
    LockHTFile();
    memset( &oHTParams, 0, sizeof( THTCLASS_PARAMETERS ) );
    f = fopen( HT_PARAMTERS_FILE, "rb" );

    if ( f )
    {
        fread( &oHTParams, 1, sizeof( THTCLASS_PARAMETERS ), f );
        fclose( f );
    }

    UnLockHTFile();
}
void WriteHTClassParams()
{
    FILE* f = NULL;
    LockHTFile();
    f = fopen( HT_PARAMTERS_FILE, "wb" );

    if ( f )
    {
        fwrite( &oHTParams, 1, sizeof( THTCLASS_PARAMETERS ), f );
        fclose( f );
    }

    else
    {
        IETextout( "====Can not Write HTClass Parameters File [%s]", HT_PARAMTERS_FILE );
    }

    UnLockHTFile();
}
int SetHTClassParams( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[4096];
    char            temp1[64];
    char            temp2[64];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char			decoderbuf[128];

    //IETextout("SetHTClassParams = [%s]", pparam);

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    memset( &oHTParams.ddns, 0x00, sizeof( THTCLASS_FACTORY_PARAMETERS ) );

    if ( GetIntParamValue( pparam, "ht_ddns_interval", &iValue ) != -1 )
    {
        oHTParams.ddns.ht_ddns_interval = iValue;
    }

    if ( GetIntParamValue( pparam, "ht_ddns_port", &iValue ) != -1 )
    {
        oHTParams.ddns.ht_ddns_port = iValue;
    }

    //oHTParams.ddns.ht_ddns_status = HT_DDNS_FAIL;
    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "deviceModel", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_pid, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_ddns_svr", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_ddns_svr, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_ddns_host", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_ddns_host, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_ddns_user", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_ddns_user, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_ddns_pwd", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_ddns_pwd, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_camerasn", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_camerasn, temp1 );
    }

    WriteHTClassParams();
    len += sprintf( temp + len, "var result=\"ok\";\r\n" );
    memcpy( pbuf, temp, len );
    return len;
}
int SetHTClassAlarm( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[4096];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char			decoderbuf[128];

    //IETextout("SetHTClassAlarm = [%s]", pparam);

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    memset( &oHTParams.alarm, 0x00, sizeof( THTCLASS_ALARM_PARAMETERS ) );

    if ( GetIntParamValue( pparam, "ht_alarm_port", &iValue ) != -1 )
    {
        oHTParams.alarm.ht_alarm_port = iValue;
    }

    if ( GetIntParamValue( pparam, "alarm_enabled", &iValue ) != -1 )
    {
        oHTParams.alarm.alarm_enabled = iValue;
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_alarm_svr", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.alarm.ht_alarm_svr, temp1 );
    }

    WriteHTClassParams();
    len += sprintf( temp + len, "var result=\"ok\";\r\n" );
    memcpy( pbuf, temp, len );
    return len;
}
int GetHTClass( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            mac[20];
    int             len = 0;
    char			nexturl[64];
    int				iRet;
    char			decoderbuf[128];

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    memset( temp, 0x00, 2048 );
    len += sprintf( temp + len, "var ht_pid=\"%s\";\r\n", oHTParams.ddns.ht_pid );
    len += sprintf( temp + len, "var ht_ddns_svr=\"%s\";\r\n", oHTParams.ddns.ht_ddns_svr );
    len += sprintf( temp + len, "var ht_ddns_port=%d;\r\n", oHTParams.ddns.ht_ddns_port );
    len += sprintf( temp + len, "var ht_ddns_interval=%d;\r\n", oHTParams.ddns.ht_ddns_interval );
    len += sprintf( temp + len, "var ht_ddns_user=\"%s\";\r\n", oHTParams.ddns.ht_ddns_user );
    len += sprintf( temp + len, "var ht_ddns_pwd=\"%s\";\r\n", oHTParams.ddns.ht_ddns_pwd );
    len += sprintf( temp + len, "var ht_ddns_host=\"%s\";\r\n", oHTParams.ddns.ht_ddns_host );
    len += sprintf( temp + len, "var ht_camerasn=\"%s\";\r\n", oHTParams.ddns.ht_camerasn );
    len += sprintf( temp + len, "var ht_ddns_status=%d;\r\n", oHTParams.ddns.ht_ddns_status );
    len += sprintf( temp + len, "var ht_alarm_svr=\"%s\";\r\n", oHTParams.alarm.ht_alarm_svr );
    len += sprintf( temp + len, "var ht_alarm_port=%d;\r\n", oHTParams.alarm.ht_alarm_port );
    len += sprintf( temp + len, "var alarm_enabled=%d;\r\n", oHTParams.alarm.alarm_enabled );
    memcpy( pbuf, temp, len );
    return len;
}
#endif

int cgisetalarm( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[4096];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char				decoderbuf[128];
    printf( "set alarm config\n" );

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    memset( &bparam.stAlarmParam, 0x00, sizeof( ALARMPARAM ) );

    //bparam.stStatusParam.warnstat = 0x00;
    //motion
    if ( GetIntParamValue( pparam, "motion_armed", &iValue ) != -1 )
    {
        MotionResutlClear();
        bparam.stAlarmParam.byMotionEnable = iValue;
        printf( "motion enable=%d\n", bparam.stAlarmParam.byMotionEnable );
    }

    if ( GetIntParamValue( pparam, "motion_sensitivity", &iValue ) != -1 )
    {
        if ( iValue > 10 )
        {
            iValue = 10;
        }

        bparam.stAlarmParam.bySensitivity = iValue;
        printf( "motion sensit\n" );
    }

#ifdef	ENABLE_TAKE_PICTURE
	/* BEGIN: Added by wupm, 2013/6/16 */
	if ( GetIntParamValue( pparam, "enable_take_pic", &iValue ) != -1 )
    {
        bparam.stAlarmParam.enable_take_pic = iValue;
    }
#endif

/* BEGIN: Added by wupm, 2013/6/27 */
#ifdef	PLAY_AUDIO_FILE_ON_ALARM
	if ( GetIntParamValue( pparam, "enable_alarm_audio", &iValue ) != -1 )
    {
		if ( iValue == 1 )
			bparam.stAlarmParam.reserved |= 0x01;
		else
			bparam.stAlarmParam.reserved &= 0xFE;
    }
#endif

    //gpio
    if ( GetIntParamValue( pparam, "input_armed", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byAlarmInEnable = iValue;
        printf( "gpio enable=%d\n", bparam.stAlarmParam.byAlarmInEnable );
    }

    if ( GetIntParamValue( pparam, "ioin_level", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byAlarmInLevel = iValue;
        printf( "in level\n" );
    }

    //gpio link
    if ( GetIntParamValue( pparam, "iolinkage", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byLinkEnable = iValue;
        printf( "iolink\n" );
    }

    if ( GetIntParamValue( pparam, "ioout_level", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byAlarmOutLevel = iValue;
        printf( "ioout level\n" );
    }

    //alarm email
    if ( GetIntParamValue( pparam, "mail", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byAlarmEmail = iValue;
        printf( "mail\n" );
    }

    //alarm ftp
    if ( GetIntParamValue( pparam, "upload_interval", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byUploadInter = iValue;
        printf( "upload_interval\n" );
    }

    //alarm preset
    if ( GetIntParamValue( pparam, "preset", &iValue ) != -1 )
    {
        bparam.stAlarmParam.szPresetSit = iValue;
        printf( "preset\n" );
    }

    //alarm snapshot
    if ( GetIntParamValue( pparam, "snapshot", &iValue ) != -1 )
    {
        bparam.stAlarmParam.bySnapshot = iValue;
        printf( "snapshot\n" );
    }

    //alarm record
    if ( GetIntParamValue( pparam, "record", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byAlarmRecord = iValue;
        printf( "record\n" );
    }

    //alarm url
    if ( GetIntParamValue( pparam, "alarm_http", &iValue ) != -1 )
    {
        bparam.stAlarmParam.alarm_http = iValue;
        printf( "alarm_http\n" );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "alarm_http_url", temp1, 63 ) != -1 )
    {
        memset( bparam.stAlarmParam.alarm_http_url, 0x00, 64 );
        strcpy( bparam.stAlarmParam.alarm_http_url, temp1 );
        printf( "alarm_http\n" );
    }

    //alarm en
    if ( GetIntParamValue( pparam, "schedule_enable", &iValue ) != -1 )
    {
        bparam.stAlarmParam.alarmen = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sun_0", &iValue ) != -1 )
    {

        bparam.stAlarmParam.stAlarmTime.time[0][0] = iValue;
        IETextout( "----- = [%d] [0x%08X] [%u]",
                 bparam.stAlarmParam.stAlarmTime.time[0][0],
                 bparam.stAlarmParam.stAlarmTime.time[0][0],
                 bparam.stAlarmParam.stAlarmTime.time[0][0] );
    }

    if ( GetIntParamValue( pparam, "schedule_sun_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[0][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sun_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[0][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_mon_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[1][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_mon_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[1][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_mon_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[1][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_tue_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[2][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_tue_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[2][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_tue_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[2][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_wed_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[3][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_wed_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[3][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_wed_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[3][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_thu_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[4][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_thu_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[4][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_thu_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[4][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_fri_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[5][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_fri_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[5][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_fri_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[5][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sat_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[6][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sat_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[6][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sat_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[6][2] = iValue;
    }

    #ifdef CUSTOM_HTTP_ALARM
	if ( GetIntParamValue( pparam, "alarmsvr", &iValue ) != -1 )
    {
		/* BEGIN: Added by wupm, 2013/5/31 */
        psdparam.bEnableAlarmSend = iValue;
		WritePsdParams();
    }

    #endif

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_ALARM
    myWriteLogIp( 0, LOG_SET_ALARM );
#endif
#endif

    return len;
}

int cgisetmedia( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char				decoderbuf[128];
    printf( "set media\n" );

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    iRet = GetIntParamValue( pparam, "mainrate", &value );

    if ( value == 0 ) 	//main rate
    {
        GetIntParamValue( pparam, "enc_framerate", &value );
        bparam.stVencParam.byframerate = value;
        GetIntParamValue( pparam, "enc_keyframe", &value );
        bparam.stVencParam.keyframe = value;
        GetIntParamValue( pparam, "enc_quant", &value );

        if ( value >= 40 )
        {
            value = 40;
        }

        if ( value <= 10 )
        {
            value = 10;
        }

        bparam.stVencParam.quant = value;
        GetIntParamValue( pparam, "enc_ratemode", &value );
        bparam.stVencParam.ratemode = value;
        GetIntParamValue( pparam, "enc_bitrate", &value );
        printf( "bitrate value %d\n", value );
        bparam.stVencParam.bitrate = value;
        GetIntParamValue( pparam, "enc_main_mode", &value );
        bparam.stVencParam.mainmode = value;
    }

    else 			//sub rate
    {
        GetIntParamValue( pparam, "sub_enc_size", &value );
        bparam.stVencParam.bysizesub = value;
        GetIntParamValue( pparam, "sub_enc_framerate", &value );
        bparam.stVencParam.byframeratesub = value;
        GetIntParamValue( pparam, "sub_enc_keyframe", &value );
        bparam.stVencParam.keyframesub = value;
        GetIntParamValue( pparam, "sub_enc_quant", &value );

        if ( value >= 40 )
        {
            value = 40;
        }

        if ( value <= 10 )
        {
            value = 10;
        }

        bparam.stVencParam.quantsub = value;
        GetIntParamValue( pparam, "sub_enc_ratemode", &value );
        bparam.stVencParam.ratemodesub = value;
        GetIntParamValue( pparam, "sub_enc_bitrate", &value );
        bparam.stVencParam.bitratesub = value;
        GetIntParamValue( pparam, "enc_sub_mode", &value );
        bparam.stVencParam.submode = value;
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgisetrecordsch( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;

/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    if ( GetIntParamValue( pparam, "record_cover", &iValue ) != -1 )
    {
        bparam.stRecordSet.recordover = iValue;
    }

    if ( GetIntParamValue( pparam, "record_timer", &iValue ) != -1 )
    {
        bparam.stRecordSet.timer = iValue;
        ReadRecordSize();
    }

    if ( GetIntParamValue( pparam, "time_schedule_enable", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerenable = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sun_0", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[0][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sun_1", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[0][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sun_2", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[0][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_mon_0", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[1][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_mon_1", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[1][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_mon_2", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[1][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_tue_0", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[2][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_tue_1", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[2][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_tue_2", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[2][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_wed_0", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[3][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_wed_1", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[3][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_wed_2", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[3][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_thu_0", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[4][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_thu_1", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[4][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_thu_2", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[4][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_fri_0", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[5][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_fri_1", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[5][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_fri_2", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[5][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sat_0", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[6][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sat_1", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[6][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sat_2", &iValue ) != -1 )
    {
        bparam.stRecordSet.timerecord.time[6][2] = iValue;
    }

/* BEGIN: Added by wupm, 2013/7/1 */

	if ( GetIntParamValue( pparam, "enable_record_audio", &iValue ) != -1 )
    {
		if ( iValue == 1 )
			bparam.stAlarmParam.reserved |= 0x02;
		else
			bparam.stAlarmParam.reserved &= 0xFD;

    }
	if ( GetIntParamValue( pparam, "record_sensitivity", &iValue ) != -1 )
    {
		if ( iValue >= 0 && iValue <= 10 )
		{
			bparam.stAlarmParam.reserved &= 0x0F;
			bparam.stAlarmParam.reserved |= (( iValue << 4 ) & 0xF0);
		}
    }
/* END:   Added by wupm, 2013/7/1 */

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        printf( "next_url:%s\n", nexturl );
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    printf( "len %d\n", len );
    return len;
}

int cgisetmisc( char* pbuf, char* pparam, unsigned char byPri )
{
	return 0;
}

int cgisetuser( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[32];
    char		temp2[32];
    char		temp3[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char		i;
    char				decoderbuf[128];

    //printf("byPri %d param:%s\n",byPri,pparam);
/*    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }
*/
    memset( temp1, 0x00, 32 );
    iRet = GetStrParamValue( pparam, "user1", temp1, 31 );
    memset( decoderbuf, 0x00, 128 );
    URLDecode( temp1, strlen( temp1 ), decoderbuf, 31 );
    memset( temp1, 0x00, 32 );
    strcpy( temp1, decoderbuf );
    memset( temp2, 0x00, 32 );
    iRet += GetStrParamValue( pparam, "user2", temp2, 31 );
    memset( decoderbuf, 0x00, 128 );
    URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
    strcpy( temp2, decoderbuf );
    memset( temp3, 0x00, 32 );
    iRet += GetStrParamValue( pparam, "user3", temp3, 31 );
    memset( decoderbuf, 0x00, 128 );
    URLDecode( temp3, strlen( temp3 ), decoderbuf, 63 );
    strcpy( temp3, decoderbuf );

    if ( ( iRet == 0 ) && ( strlen( temp3 ) ) )
    {
        int iRet1;
        int iRet2;
        iRet = memcmp( temp3, temp1, 32 );

        //printf("cmp %d %d admin %s op %s vistor %s\n",iRet,iRet1,temp3,temp2,temp1);
        if ( iRet )
        {
            //printf("temp3:%s temp2:%s\n",temp3,temp2);
            iRet1 = memcmp( temp3, temp2, 32 );

            if ( iRet1 )
            {
                //printf("temp1:%s temp2:%s\n",temp1,temp2);
                iRet2 = memcmp( temp1, temp2, 32 );

                if ( ( iRet2 ) || ( ( iRet2 == 0 ) && ( strlen( temp1 ) == 0 ) ) )
                {
                    char passwd1[32];
                    char passwd2[32];
                    char passwd3[32];
                    //printf("temp1:%s\n",temp1);
                    memset( bparam.stUserParam[0].szUserName, 0x00, 32 );
                    memset( bparam.stUserParam[1].szUserName, 0x00, 32 );
                    memset( bparam.stUserParam[2].szUserName, 0x00, 32 );
                    strcpy( bparam.stUserParam[0].szUserName, temp1 );
                    strcpy( bparam.stUserParam[1].szUserName, temp2 );
                    strcpy( bparam.stUserParam[2].szUserName, temp3 );
                    memset( bparam.stUserParam[0].szPassword, 0x00, 32 );
                    memset( bparam.stUserParam[1].szPassword, 0x00, 32 );
                    memset( bparam.stUserParam[2].szPassword, 0x00, 32 );
                    memset( passwd1, 0x00, 32 );
                    GetStrParamValue( pparam, "pwd1", passwd1, 31 );
                    memset( decoderbuf, 0x00, 128 );
                    URLDecode( passwd1, strlen( passwd1 ), decoderbuf, 31 );
                    memset( passwd1, 0x00, 32 );
                    strcpy( passwd1, decoderbuf );
                    memset( passwd2, 0x00, 32 );
                    GetStrParamValue( pparam, "pwd2", passwd2, 31 );
                    memset( decoderbuf, 0x00, 128 );
                    URLDecode( passwd2, strlen( passwd2 ), decoderbuf, 31 );
                    memset( passwd2, 0x00, 32 );
                    strcpy( passwd2, decoderbuf );
                    memset( passwd3, 0x00, 32 );
                    GetStrParamValue( pparam, "pwd3", passwd3, 31 );
                    memset( decoderbuf, 0x00, 128 );
                    URLDecode( passwd3, strlen( passwd3 ), decoderbuf, 31 );
                    memset( passwd3, 0x00, 32 );
                    strcpy( passwd3, decoderbuf );
                    strcpy( bparam.stUserParam[0].szPassword, passwd1 );
                    strcpy( bparam.stUserParam[1].szPassword, passwd2 );
                    strcpy( bparam.stUserParam[2].szPassword, passwd3 );
                }
            }
        }
    }

    //printf("pparam:%s\n",pparam);
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
        //printf("nexturl:%s\n",nexturl);
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_USER
    myWriteLogIp( 0, LOG_SET_USER );
#endif
#endif

    return len;
}

int cgisetrestore( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len  = 0;
    int             iRet = 0;
    char            nexturl[64];
    int             value;
    int             iValue;

#ifndef FACTOP
	#ifdef	SYSTEM_PARAM_FILENAME
	Textout("Remove Parameters");
    DoSystem( "rm /system/www/BellParam.bin" );
	#else
    DoSystem( "rm /system/www/system.ini" );
	#endif
#else
	OnPressReset();
#endif

    /* BEGIN: Added by Baggio.wu, 2013/7/24 */
    //DoSystem( "rm /system/www/system.ini_backup" );
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgisetdefault( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len  = 0;
    int             iRet = 0;
    char            nexturl[64];

    DoSystem( "cp /system/www/BellParam.bin /system/www/BellParam-b.bin" );
    DoSystem( "sync" );
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgisetgpioir( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   	temp[2048];
    char            		temp1[64];
    char            		temp2[32];
    int             		len  = 0;
    int             		iRet = 0;
    char            		passtemp[32];
    char            		usertemp[32];
    char            		nexturl[64];
    int             		value;
    int             		iValue;
    char				decoderbuf[128];

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    iRet = GetIntParamValue( pparam, "val", &iValue );

    if ( iRet == 0 )
    {
        bparam.stPTZParam.gpioir = iValue;
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgisetlog( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char				decoderbuf[128];

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    DoSystem( "rm /param/systemindex.txt" );
    DoSystem( "rm /param/systemlog.txt" );
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_RESET_LOG
    myWriteLogIp( 0, LOG_RESET_LOG );
#endif
#endif

    return len;
}

int cgisetmoto( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char				decoderbuf[128];

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    //MotoTimeStop();
    //MotoAction(PTZ_LEFT_RIGHT,0x00,0x00);
    //MotoAction(PTZ_UP_DOWN,0x00,0x00);
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgisetdelfile( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[64];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    char            szFieldName[64];
    int             value;
    int             iValue;
    char				decoderbuf[128];

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    memset( szFieldName, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "name", szFieldName, 63 );

    if ( iRet == 0x00 )
    {
        char name[128];
        char delcmd[128];
        int  iRet;
        struct stat     stStat;
        memset( decoderbuf, 0x00, 128 );
        URLDecode( szFieldName, strlen( szFieldName ), decoderbuf, 63 );
        memset( szFieldName, 0x00, 64 );
        strcpy( szFieldName, decoderbuf );
        memset( delcmd, 0x00, 128 );

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE
        DeleteRecordFile( szFieldName );
#else
        /* BEGIN: Modified by wupm, 2013/4/3 */
        //sprintf( delcmd, "rm /system/www/record/%s", szFieldName );
        sprintf( delcmd, "rm -f /mnt/%s", szFieldName );

        if ( ( szFieldName[0] == 0x61 ) && ( szFieldName[1] == 0x6c ) && ( szFieldName[2] == 0x6c ) )
        {
            printf( "del all\n" );
            DoSystem( "rm -f /media/sd/*.h260" );
            DoSystem( "sync" );
        }

        else
        {
            printf( "del a file\n" );
#if 1
            DoSystem( delcmd );
            DoSystem( "sync" );
#else
            iRet = unlink( delcmd + 3 );
            iRet = access( delcmd + 3, F_OK );

            if ( iRet == 0x00 )
            {
                DoSystem( "mount -o rw,remount /media/sd" );
            }

#endif
        }

#endif
        /* END: Deleted by wupm, 2013/3/7 */
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_DELETE_FILE
    myWriteLogIp( 0, LOG_DELETE_FILE );
#endif
#endif

    return len;
}

int cgisetcamcontrol( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    int		iRet1 = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             iValue;
    unsigned char	cmd = 0;
    unsigned char	param = 0;
    char				decoderbuf[128];
    memset( temp, 0x00, 2048 );
    iRet = GetIntParamValue( pparam, "param", &iValue );
    cmd = iValue;
    iRet += GetIntParamValue( pparam, "value", &iValue );
    param = iValue;
    //printf("camcontrol iRet %d  iRet1 %d value %d param:%s len %d\n",iRet,iRet1,param,pparam,strlen(pparam));
    printf( "cameracontrol...param:%d,value:%d\n", cmd, param );

    if ( ( iRet | iRet1 ) == 0x00 )
    {
        //printf("camera_control cmd:%d param %d\n",cmd,param);
        if ( cameraflag == 0x00 )
        {
            cameraflag = 0x01;
            camera_control( cmd, param );
            cameraflag = 0x00;
        }

        //printf("camera_control end\n");
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgisetdecodercontrol( char* pbuf, char* pparam, unsigned char byPri )
{
	return 0;
}


//wifi scanning
//extern int wifiScanSit ;
//extern short wifiScanCmd;
extern int bScanEnd;

void ScanThreadProc( void* p )
{
    int iRet;

    while ( 1 )
    {
        if ( wifiscanflag == 0x00 )
        {
            usleep( 500*1000 );
            continue;
        }

        wifiscanflag = 0x00;
        GetWifiScan();
		bScanEnd = 1;
		//p2pgetwifiscan( wifiScanSit, wifiScanCmd);
    }
}
//date thread
void ScanThreadInit( void ) 	//date check thread is start
{
    pthread_t cgithread;
    pthread_create( &cgithread, 0, &ScanThreadProc, NULL );
	pthread_detach(cgithread);
}
//wait sec timeout
void cgiwaittimeout( int timeout )
{
    sleep( timeout );
}

void StartWifiScan()
{
    wifiscanflag = 0x01;
}

int cgisetwifiscan( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   	temp[2048];
    char            		temp1[64];
    char            		temp2[32];
    int             		len  = 0;
    int             		iRet = 0;
    int             		iRet1 = 0;
    char            		passtemp[32];
    char            		usertemp[32];
    char            		nexturl[64];
    int             		value;
    int             		iValue;
    char            		param[4];
    char			decoderbuf[128];

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    //sem_post(&wifiscansem);
    wifiscanflag = 0x01;
    cgiwaittimeout( 3 );
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    //printf("pparam:%s nexturl:%s iRet %d\n",pparam,nexturl,iRet);
    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}


int cgisetwifi( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[128];
    char		temp3[64];
    char            temp2[64];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char		i = 0, j = 0;
    char		flag = 0;
    char				decoderbuf[128];

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    GetIntParamValue( pparam, "poweronen", &value );
    bparam.stWifiParam.byPowerOn = value;
    GetIntParamValue( pparam, "enable", &value );
    bparam.stWifiParam.byEnable = value;
    GetIntParamValue( pparam, "wps_enable", &value );
    bparam.stWifiParam.byWps = value;
    memset( temp2, 0x00, 64 );
    memset( bparam.stWifiParam.wpspin, 0x00, 8 );
    iRet = GetStrParamValue( pparam, "wps_pin", temp2, 8 ); //bparam.stWifiParam.szSSID);

    if ( iRet == 0x00 )
    {
        memset( bparam.stWifiParam.wpspin, 0x00, 8 );
        strcpy( bparam.stWifiParam.wpspin, temp2 );
    }

    memset( bparam.stWifiParam.szSSID, 0x00, 64 );
    memset( bparam.stWifiParam.szShareKey, 0x00, 64 );
    memset( bparam.stWifiParam.szKey1, 0x00, 64 );
    memset( bparam.stWifiParam.szKey2, 0x00, 64 );
    memset( bparam.stWifiParam.szKey3, 0x00, 64 );
    memset( bparam.stWifiParam.szKey4, 0x00, 64 );
    //memset(bparam.stWifiParam.szIpAddr,0x00,16);
    memset( temp1, 0x00, 128 );
    printf( "pparam: %s\n", pparam );
    iRet = GetStrParamValue( pparam, "ssid", temp1, 63 ); //bparam.stWifiParam.szSSID);

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 63 );
        memset( temp3, 0x00, 64 );
        strcpy( temp3, decoderbuf );
        memset( bparam.stWifiParam.szSSID, 0x00, 64 );
        strcpy( bparam.stWifiParam.szSSID, temp3 );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "wpa_psk", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stWifiParam.szShareKey, 0x00, 64 );
        strcpy( bparam.stWifiParam.szShareKey, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "key1", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stWifiParam.szKey1, 0x00, 64 );
        strcpy( bparam.stWifiParam.szKey1, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "key2", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stWifiParam.szKey2, 0x00, 64 );
        strcpy( bparam.stWifiParam.szKey2, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "key3", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stWifiParam.szKey3, 0x00, 64 );
        strcpy( bparam.stWifiParam.szKey3, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "key4", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stWifiParam.szKey4, 0x00, 64 );
        strcpy( bparam.stWifiParam.szKey4, decoderbuf );
    }

    //GetStrParamValue(pparam, "ipaddr", bparam.stWifiParam.szIpAddr);
    GetIntParamValue( pparam, "mode", &value );
    bparam.stWifiParam.byWifiMode = value;
    GetIntParamValue( pparam, "channel", &value );
    bparam.stWifiParam.channel = value;
    printf( "channel:%d\n", bparam.stWifiParam.channel );
    GetIntParamValue( pparam, "encrypt", &value );
    bparam.stWifiParam.byEncrypt = value;
    GetIntParamValue( pparam, "authtype", &value );
    bparam.stWifiParam.byAuthType = value;
    GetIntParamValue( pparam, "keyformat", &value );
    bparam.stWifiParam.byKeyFormat = value;
    printf( "keyfromat:%d\n", value );
    GetIntParamValue( pparam, "defkey", &value );
    bparam.stWifiParam.byDefKeyType = value;
    GetIntParamValue( pparam, "key1_bits", &value );
    bparam.stWifiParam.byKey1Bits = value;
    GetIntParamValue( pparam, "key2_bits", &value );
    bparam.stWifiParam.byKey2Bits = value;
    GetIntParamValue( pparam, "key3_bits", &value );
    bparam.stWifiParam.byKey3Bits = value;
    GetIntParamValue( pparam, "key4_bits", &value );
    bparam.stWifiParam.byKey4Bits = value;
    bparam.stIEBaseParam.sysmode = 0x01;
    printf( "set wifi mode:%d\n", bparam.stIEBaseParam.sysmode );
    printf( "ssid: %s\n", bparam.stWifiParam.szSSID );


    /* BEGIN: Added by wupm, 2013/6/8 */
#if	1

    //authtype = 1 is WEP
    //defkey = 0-3 is KeyIndex
    //key1-4 is Key
    //KeyLen 5,13,16 = ASCII(1)
    //KeyLen 10,26,32 = HEX(0)
    if ( bparam.stWifiParam.byAuthType == 1 )
    {
        int nKeyIndex = bparam.stWifiParam.byDefKeyType;
        int nKeyLength = 0;

        switch ( nKeyIndex )
        {
            case 0:
                nKeyLength = strlen( bparam.stWifiParam.szKey1 );
                break;

            case 1:
                nKeyLength = strlen( bparam.stWifiParam.szKey2 );
                break;

            case 2:
                nKeyLength = strlen( bparam.stWifiParam.szKey3 );
                break;

            case 3:
                nKeyLength = strlen( bparam.stWifiParam.szKey4 );
                break;
        }

        switch ( nKeyLength )
        {
            case 5:
            case 13:
            case 16:
                bparam.stWifiParam.byKeyFormat = 1;	//Ascii
                break;

            case 10:
            case 26:
            case 32:
                bparam.stWifiParam.byKeyFormat = 0;	//Hex
                break;
        }
    }

#endif


    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_WIFI
    myWriteLogIp( 0, LOG_SET_WIFI );
#endif
#endif

    return len;
}

int cgisetsnapshot( char* pbuf, char* pparam, unsigned char byPri, unsigned int frameno )
{
    int len = 0;
    //len += GetSnapStream(pbuf+len,frameno);
    //printf("send len %d\n",len);
    return len;
}

int cgisetvideostream( char* pbuf, char* pparam, unsigned int frameno )
{
    int             len  = 0;

    if ( frameno == 0x00 )
    {
        len += sprintf( pbuf + len, "Content-type:multipart/x-mixed-replace;boundary=ipcam264\r\n" );
    }

    //printf("start capture\n");
    len += GetVideoStream( pbuf + len, frameno );
    //printf("videostream  %d frameno %d\n",len,frameno);
    return len;
}

int cgisetfactory( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[64];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char            i = 0;
    char            flag = 0;
    char				decoderbuf[128];
    //Textout( "setfactory = [%s]", pparam );

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }


    memset( temp, 0x00, 2048 );
    memset( temp2, 0x00, 32 );

	/* modify begin by yiqing, 2016-05-16 */
    if ( !GetStrParamValue( pparam, "deviceid", temp2, 24 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );
        nvram_bufset( RT2860_NVRAM, "factory_deviceid", temp2 );
        memset( bparam.stIEBaseParam.dwDeviceID, 0x00, sizeof(bparam.stIEBaseParam.dwDeviceID) );
        strcpy( bparam.stIEBaseParam.dwDeviceID, temp2 );
        len += sprintf( temp + len, "deviceid:%s\n\n", temp2 );
    }

	/* add begin by yiqing, 2016-05-16*/
	memset( temp2, 0x00, 32 );
	if ( !GetStrParamValue( pparam, "apilisense", temp2, 8 ) )
	{
		memset( decoderbuf, 0x00, 128 );
		URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
		memset( temp2, 0x00, 32 );
		strcpy( temp2, decoderbuf );
		nvram_bufset( RT2860_NVRAM, "factory_apilisense", temp2 );
		memset( bparam.stIEBaseParam.dwApiLisense, 0x00, sizeof(bparam.stIEBaseParam.dwApiLisense) );
		strcpy( bparam.stIEBaseParam.dwApiLisense, temp2 );
		len += sprintf( temp + len, "ApiLisense:%s\n\n", temp2 );
	}
	else
	{
		memset( temp2, 0x00, 32 );
		nvram_bufset( RT2860_NVRAM, "factory_apilisense", temp2 );
		memset( bparam.stIEBaseParam.dwApiLisense, 0x00, sizeof(bparam.stIEBaseParam.dwApiLisense) );
		len += sprintf( temp + len, "ApiLisense:%s\n\n", temp2 );
	}

    memset( temp2, 0x00, 32 );

    if ( !GetStrParamValue( pparam, "mac", temp2, 17 ) )
    {
        char* pmac = NULL;
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );

#ifdef	REMOVE_MAC_LIMIT
        /* BEGIN: Deleted by wupm, 2013/1/18   BUG:124 */
        //temp2[0] = 0x30;
        //temp2[1] = 0x30;
#else
        temp2[0] = 0x30;
        temp2[1] = 0x30;
#endif
        nvram_bufset( RT2860_NVRAM, "factory_mac", temp2 );

        if ( strlen( temp2 ) == 17 )
        {
            len += sprintf( temp + len, "mac:%s\n\n", temp2 );
            pmac = nvram_bufget( RT2860_NVRAM, "factory_mac" );
            printf( "mac:%s\n", pmac );
        }
    }

    memset( temp2, 0x00, 32 );

    if ( !GetStrParamValue( pparam, "wifimac", temp2, 17 ) )
    {
        char* pmac = NULL;
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 17 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );

#ifdef	REMOVE_MAC_LIMIT
        /* BEGIN: Deleted by wupm, 2013/1/18   BUG:124 */
        //temp2[0] = 0x30;
        //temp2[1] = 0x30;
#else
        temp2[0] = 0x30;
        temp2[1] = 0x30;
#endif

        nvram_bufset( RT2860_NVRAM, "factory_wifimac", temp2 );

        if ( strlen( temp2 ) == 17 )
        {
            len += sprintf( temp + len, "wifimac:%s\n\n", temp2 );
            pmac = nvram_bufget( RT2860_NVRAM, "factory_wifimac" );
            printf( "wifimac:%s\n", pmac );
        }
    }

    memset( temp1, 0x00, 64 );
    if ( !GetStrParamValue( pparam, "server", temp1, 63 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 63 );
        memset( temp1, 0x00, 64 );
        strcpy( temp1, decoderbuf );
        nvram_bufset( RT2860_NVRAM, "factory_server", temp1 );
        memset( bparam.stDdnsParam.serversvr, 0x00, 64 );
        strcpy( bparam.stDdnsParam.serversvr, temp1 );
    }

    if ( !GetIntParamValue( pparam, "port", &value ) )
    {
        char tempport[8];
        memset( tempport, 0x00, 8 );
        sprintf( tempport, "%d", value );
        nvram_bufset( RT2860_NVRAM, "factory_dnsport", tempport );
        bparam.stDdnsParam.serverport = value;
    }

    memset( temp1, 0x00, 64 );

    if ( !GetStrParamValue( pparam, "username", temp1, 31 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 31 );
        memset( temp1, 0x00, 32 );
        strcpy( temp1, decoderbuf );
        memset( bparam.stDdnsParam.serveruser, 0x00, 32 );
        nvram_bufset( RT2860_NVRAM, "factory_dnsuser", temp1 );
        strcpy( bparam.stDdnsParam.serveruser, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( !GetStrParamValue( pparam, "userpwd", temp1, 31 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 31 );
        memset( temp1, 0x00, 32 );
        strcpy( temp1, decoderbuf );
        memset( bparam.stDdnsParam.serverpwd, 0x00, 32 );
        nvram_bufset( RT2860_NVRAM, "factory_dnspwd", temp1 );
        strcpy( bparam.stDdnsParam.serverpwd, temp1 );
    }

    if ( !GetIntParamValue( pparam, "heartbeat", &value ) )
    {
        char tempbeat[8];
        memset( tempbeat, 0x00, 8 );
        sprintf( tempbeat, "%d", value );
        nvram_bufset( RT2860_NVRAM, "factory_heartbeat", tempbeat );
        bparam.stDdnsParam.serverbeat = value;
    }

    if ( !GetIntParamValue( pparam, "serviceindex", &value ) )
    {
        char tempenable[8];
        memset( tempenable, 0x00, 8 );
        sprintf( tempenable, "%d", value );
        nvram_bufset( RT2860_NVRAM, "factory_index", tempenable );
        bparam.stDdnsParam.factoryversion = value;
    }

    if ( !GetIntParamValue( pparam, "mode", &value ) )
    {
        char tempenable[8];
        memset( tempenable, 0x00, 8 );
        sprintf( tempenable, "%d", value );
        nvram_bufset( RT2860_NVRAM, "factory_mode", tempenable );
        bparam.stDdnsParam.factorymode = value;
    }

    /* BEGIN: Added by Baggio.wu, 2013/10/9 */
    #ifdef CHANGE_DDNS_ALARM_SERVER
    memset( &vstarparam, 0x00, sizeof(TVSTARPARAM) );
    memset( temp1, 0x00, 64 );
    if ( !GetStrParamValue( pparam, "alarm_server", temp1, 63 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 63 );
        memset( temp1, 0x00, 64 );
        strcpy( temp1, decoderbuf );
        if(strlen(temp1) > 0)
        {
            vstarparam.bEnable = 1;
            strcpy( vstarparam.alarm_svr, temp1 );
        }

        WriteVstarParams();
    }
    #endif
    /* END:   Added by Baggio.wu, 2013/10/9 */

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var setfactory result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    nvram_commit( RT2860_NVRAM );

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_FACTORY
    myWriteLogIp( 0, LOG_SET_FACTORY );
#endif
#endif

    return len;
}

/* BEGIN: Append CGI for Change UUID/MAC */
/*        Added by wupm(2073111@qq.com), 2014/9/19 */
void SaveNewUUID(char *szNewUUID)
{
	Textout("SaveNewUUID szNewUUID=%s",szNewUUID);
	FILE *f = fopen(AUTO_UUID_FILENAME, "wb");
	fwrite(szNewUUID, 1, strlen(szNewUUID), f);
	fclose(f);
}
int CGISetP2PUUID( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[64];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char            i = 0;
    char            flag = 0;
    char				decoderbuf[128];
    Textout( "setfactory = [%s]", pparam );

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }


    memset( temp, 0x00, 2048 );
    memset( temp2, 0x00, 32 );

    if ( !GetStrParamValue( pparam, "uuid", temp2, 31 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );
		if ( temp2[0] != 0 )
		{
			SaveNewUUID(temp2);
		}
		else
		{
			Textout("New UUID is NULL----!!!");
			SaveNewUUID(DEFAULT_UUID);
		}
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int cgiOpenLock( char* pbuf, char* pparam, unsigned char byPri )
{
	unsigned char	temp[2048];
	   char 		   temp1[64];
	   char 		   temp2[64];
	   int			   len	= 0;
	   int			   iRet = 0;
	   char 		   passtemp[32];
	   char 		   usertemp[32];
	   char 		   nexturl[64];
	   int			   value;
	   int			   iValue;
	   char 		   i = 0;
	   char 		   flag = 0;
	   char 			   decoderbuf[128];
	   //Textout( "setfactory = [%s]", pparam );
	
	   if ( 0 ) //byPri != ADMIN )
	   {
		   return 0;
	   }
	
	

	OnDoorOpen();
	
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
    return 0;
}


int cgigetmisc( unsigned char* pbuf, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             result = 0;
    char				decoderbuf[128];

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    memset( temp, 0x00, 2048 );
    len += sprintf( temp + len, "var ptz_patrol_rate=%d;\r\n", bparam.stPTZParam.byRate );
    len += sprintf( temp + len, "var ptz_patrol_up_rate=%d;\r\n", bparam.stPTZParam.byUpRate );
    len += sprintf( temp + len, "var ptz_patrol_down_rate=%d;\r\n", bparam.stPTZParam.byDownRate );
    len += sprintf( temp + len, "var ptz_patrol_left_rate=%d;\r\n", bparam.stPTZParam.byLeftRate );
    len += sprintf( temp + len, "var ptz_patrol_right_rate=%d;\r\n", bparam.stPTZParam.byRightRate );
    len += sprintf( temp + len, "var ptz_center_onstart=%d;\r\n", bparam.stPTZParam.byCenterEnable );
    len += sprintf( temp + len, "var ptz_disppreset=%d;\r\n", bparam.stPTZParam.byDisPresent );
    len += sprintf( temp + len, "var led_mode=%d;\r\n", bparam.stPTZParam.byLedMode );
    len += sprintf( temp + len, "var preset_onstart=%d;\r\n", bparam.stPTZParam.byOnStart );
    len += sprintf( temp + len, "var ptruntimes=%d;\r\n", bparam.stPTZParam.byRunTimes );
    memcpy( pbuf, temp, len );
    return len;
}

int cgiformatsd( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char            i = 0;
    char            flag = 0;

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    NoteSDFormat();
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_FORMAT_SD
    myWriteLogIp( 0, LOG_FORMAT_SD );
#endif
#endif

    return len;
}

/* BEGIN: Added by wupm, 2013/3/27 */
/* BEGIN: Deleted by wupm, 2013/4/15 */
/*
void ResetAPPassword()
{
	if ( bparam.stUserParam[2].szPassword[0] == 0 )
	{
		sprintf( bparam.stWifiRoute.szShareKey, "888888" );
	}
	else
	{
		sprintf( bparam.stWifiRoute.szShareKey, bparam.stUserParam[2].szPassword );
	}
}
*/

int cgigetsyswifi( unsigned char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            mac[20];
    int             len = 0;
    char		nexturl[64];
    int		iRet;
    char				decoderbuf[128];

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    //printf("get status.cgi\n");
    memset( temp, 0x00, 2048 );
    len += sprintf( temp + len, "var syswifi_encrypt=%d;\r\n", bparam.stWifiRoute.byEncrypt );
    len += sprintf( temp + len, "var syswifi_port=%d;\r\n", bparam.stWifiRoute.nport );

    /* BEGIN: Added by wupm, 2013/3/27 */
    /* BEGIN: Deleted by wupm, 2013/4/15 */
#if	0	//def	RESET_APPASSWORD
    ResetAPPassword();
#endif

    len += sprintf( temp + len, "var syswifi_key=\"%s\";\r\n", bparam.stWifiRoute.szShareKey );
    len += sprintf( temp + len, "var syswifi_ssid=\"%s\";\r\n", bparam.stWifiRoute.szSSID );
    len += sprintf( temp + len, "var syswifi_ipaddr=\"%s\";\r\n", bparam.stWifiRoute.ipaddr );
    len += sprintf( temp + len, "var syswifi_mask=\"%s\";\r\n", bparam.stWifiRoute.mask );
    len += sprintf( temp + len, "var syswifi_startip=\"%s\";\r\n", bparam.stWifiRoute.startip );
    len += sprintf( temp + len, "var syswifi_endip=\"%s\";\r\n", bparam.stWifiRoute.endip );
    memcpy( pbuf, temp, len );
    return len;
}

int cgisetsyswifi( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[32];
    int             len  = 0;
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char            i = 0;
    char            flag = 0;
    char				decoderbuf[128];

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    printf( "param:%s\n", pparam );
    memset( temp, 0x00, 2048 );

    if ( !GetIntParamValue( pparam, "syswifi_encrypt", &value ) )
    {
        bparam.stWifiRoute.byEncrypt = value;
    }

    if ( !GetIntParamValue( pparam, "syswifi_port", &value ) )
    {
        bparam.stWifiRoute.nport = value;
    }

    memset( temp1, 0x00, 64 );

    if ( !GetStrParamValue( pparam, "syswifi_ssid", temp1, 63 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 63 );
        memset( temp1, 0x00, 63 );
        strcpy( temp1, decoderbuf );
        memset( bparam.stWifiRoute.szSSID, 0x00, 64 );
        strcpy( bparam.stWifiRoute.szSSID, temp1 );
        printf( "ssid:%s\n", temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( !GetStrParamValue( pparam, "syswifi_key", temp1, 63 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        /* BEGIN: Modified by wupm, 2013/1/31   Version:159 */
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 63 );
        memset( temp1, 0x00, 63 );
        strcpy( temp1, decoderbuf );
        memset( bparam.stWifiRoute.szShareKey, 0x00, 64 );
        strcpy( bparam.stWifiRoute.szShareKey, temp1 );
        printf( "passwd:%s\n", temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( !GetStrParamValue( pparam, "syswifi_ipaddr", temp1, 15 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        /* BEGIN: Modified by wupm, 2013/1/31   Version:159 */
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 15 );
        memset( temp1, 0x00, 63 );
        strcpy( temp1, decoderbuf );
        memset( bparam.stWifiRoute.ipaddr, 0x00, 16 );
        strcpy( bparam.stWifiRoute.ipaddr, decoderbuf );
    }

    memset( temp1, 0x00, 64 );

    if ( !GetStrParamValue( pparam, "syswifi_mask", temp1, 15 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        /* BEGIN: Modified by wupm, 2013/1/31   Version:159 */
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 15 );
        memset( temp1, 0x00, 63 );
        strcpy( temp1, decoderbuf );
        memset( bparam.stWifiRoute.mask, 0x00, 16 );
        strcpy( bparam.stWifiRoute.mask, decoderbuf );
    }

    memset( temp1, 0x00, 64 );

    if ( !GetStrParamValue( pparam, "syswifi_startip", temp1, 15 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        /* BEGIN: Modified by wupm, 2013/1/31   Version:159 */
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 15 );
        memset( temp1, 0x00, 63 );
        strcpy( temp1, decoderbuf );
        memset( bparam.stWifiRoute.startip, 0x00, 16 );
        strcpy( bparam.stWifiRoute.startip, decoderbuf );
    }

    memset( temp1, 0x00, 64 );

    if ( !GetStrParamValue( pparam, "syswifi_endip", temp1, 15 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        /* BEGIN: Modified by wupm, 2013/1/31   Version:159 */
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 15 );
        memset( temp1, 0x00, 63 );
        strcpy( temp1, decoderbuf );
        memset( bparam.stWifiRoute.endip, 0x00, 16 );
        strcpy( bparam.stWifiRoute.endip, decoderbuf );
    }

    bparam.stIEBaseParam.sysmode = 0x00;
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

int GetUpdateFlag( void )
{
    return updateflag;
}
int ClrUpdateFlag( void )
{
    updateflag = 0;
}

int cgiupgradeapp( char* pbuf, char* pparam, unsigned char byPri )
{
    int		iRet;
    unsigned int	len = 0;
    unsigned int	offset;

    char		nexturl[64];
    char		temp[1024];

    printf( "===upgade Wave file===\n" );

    #ifndef SUPPORT_FM34
    Es838Close();
    #endif
    
    DoSystem( "mv /tmp/post.bin /tmp/post1.bin" );
    updateflag = 0x01;
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len = RefreshUrl1( pbuf, nexturl );
        //printf("pbuf:%s\n",pbuf);
        //memcpy(pbuf,temp,len);
    }

    else
    {
        len += sprintf( temp + len, "upgradeapp ok\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_UPGRADE_APP
    myWriteLogIp( 0, LOG_UPGRADE_APP );
#endif
#endif

    NotifyToDaemon( 0x02, &bparam, sizeof( IEPARAMLIST ) );
    return len;
}

int cgiupgradesys( char* pbuf, char* pparam, unsigned char byPri )
{
    int             		iRet = 0;
    unsigned int    	len = 0;

    char            nexturl[64];
    char            temp[4096];
    
    printf( "===upgade sys===\n" );
    #ifndef SUPPORT_FM34
    Es838Close();
    #endif
    
    DoSystem( "mv /tmp/post.bin /tmp/post1.bin" );
    updateflag = 0x01;

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        printf("update return next_url\n");
        len += RefreshUrl1( pbuf, nexturl );
    }

    else
    {
        printf("update return ok\n");
        len += sprintf( temp + len, "upgradesys ok\n" );
        memcpy( pbuf, temp, len );
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_UPGRADE_SYS
    myWriteLogIp( 0, LOG_UPGRADE_SYS );
#endif
#endif

    NotifyToDaemon( 0x01, &bparam, sizeof( IEPARAMLIST ) );
    return len;
}

unsigned char AdjustUserPri( char* url )
{
    int		iRet;
    int		iRet1;
    unsigned char 	byPri = 0;
    char    	loginuse[32];
    char    	loginpas[32];
    char		decoderbuf[128];
    char		temp2[128];
    memset( loginuse, 0x00, 32 );
    memset( loginpas, 0x00, 32 );
    memset( temp2, 0x00, 128 );
    iRet = GetStrParamValue( url, "loginuse", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 15 );
        memset( loginuse, 0x00, 31 );
        strcpy( loginuse, decoderbuf );
    }

    memset( temp2, 0x00, 128 );
    iRet1 = GetStrParamValue( url, "loginpas", temp2, 31 );

    if ( iRet1 == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 15 );
        memset( loginpas, 0x00, 31 );
        strcpy( loginpas, decoderbuf );
    }

    if ( iRet == 0 )
    {
        if ( iRet1 == 0x00 )
        {
            //printf("user %s pwd:%s\n",loginuse,loginpas);
            byPri = GetUserPri( loginuse, loginpas );
            return byPri;
        }
    }

    memset( loginuse, 0x00, 32 );
    memset( loginpas, 0x00, 32 );
    memset( temp2, 0x00, 128 );
    iRet = GetStrParamValue( url, "user", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 15 );
        memset( loginuse, 0x00, 31 );
        strcpy( loginuse, decoderbuf );
    }

    memset( temp2, 0x00, 128 );
    iRet1 = GetStrParamValue( url, "pwd", temp2, 31 );

    if ( iRet1 == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 15 );
        memset( loginpas, 0x00, 31 );
        strcpy( loginpas, decoderbuf );
    }

    if ( iRet == 0 )
    {
        if ( iRet1 == 0x00 )
        {
            //printf("user %s pwd:%s\n",loginuse,loginpas);
            byPri = GetUserPri( loginuse, loginpas );
            return byPri;
        }
    }

    return byPri;
}

int cgigetsaveparam( unsigned char* pbuf, unsigned char* pparam, unsigned char byPri )
{
    /* BEGIN: Deleted by wupm, 2013/3/9 */
#if	1
    return 0;
#else
    int		iRet;
    unsigned int	len = 0;
    unsigned int	offset;
    FILE*		fp = NULL;
    unsigned char*	pbuffer = NULL;
    unsigned char*	ptemp   = NULL;
    unsigned char*	ptemp1  = NULL;
    unsigned char*	ptemp2  = NULL;
    unsigned char*	ptemp3  = NULL;
    unsigned char*	pstart  = NULL;
    unsigned char*	pend    = NULL;
    unsigned char	bound[128];
    int		boundlen = 0;
    char		nexturl[64];
    char		temp[1024];

    printf( "=======cgi get save param==========\n" );
    DoSystem( "mv /tmp/post.bin /tmp/system.ini" );
    updateflag = 0x01;
    NoteParamUpdate();
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len = RefreshUrl1( pbuf, nexturl );
        //printf("pbuf:%s\n",pbuf);
        //memcpy(pbuf,temp,len);
    }

    else
    {
        len += sprintf( temp + len, "ok\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
#endif
    /* END: Deleted by wupm, 2013/3/9 */
}


int ReadSystemInit( unsigned char* temp )
{
    FILE* fp = NULL;
    int iRet;

	#ifdef	SYSTEM_PARAM_FILENAME
    fp = fopen( SYSTEM_PARAM_FILENAME, "rb" );
	#else
	fp = fopen( "/system/www/system.ini", "rb" );
	#endif

    if ( fp == NULL )
    {
        return 0;
    }

    iRet = fread( temp, 1, 1024 * 8, fp );
    fclose( fp );
    return iRet;
}

int cgisetbackupparam( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char           temp[1024 * 8];
    char                            nexturl[64];
    int                             len = 0;
    int                             iRet;

    printf( "cgisetbackupparam\n" );

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

#if 0
    memset( temp, 0x00, 1024 * 8 );
    len = ReadSystemInit( temp );
    memcpy( pbuf, temp, len );
    printf( "backup param=%d\n", len );
#endif
    return len;
}

#ifdef	SCC
int cgiGetSccConfig( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             result = 0;

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    memset( temp, 0x00, 2048 );

    len += sprintf( temp + len, "var scc_server=\"%s\";\r\n", g_sccParams.szServer );
    len += sprintf( temp + len, "var scc_user=\"%s\";\r\n", g_sccParams.szUser );
    len += sprintf( temp + len, "var scc_pwd=\"%s\";\r\n", g_sccParams.szPwd );
    len += sprintf( temp + len, "var scc_port=%d;\r\n", g_sccParams.nServerPort );
    memcpy( pbuf, temp, len );

    return len;
}

int cgiSetSccConfig( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   	temp[2048];
    int                 len  = 0;
    int             	iRet = 0;
    char            	nexturl[64];
    char            	szServer[64];
    char				szUser[32];
    char				szPwd[32];
    int nValue = 0;

    //printf("cgiSetSccConfig.. byPri: %d\n", byPri);
    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    memset( temp, 0, 2048 );
    memset( szServer, 0, sizeof( szServer ) );
    iRet = GetStrParamValue( pparam, "scc_server", szServer, 64 );

    if ( iRet == 0 )
    {
        memset( g_sccParams.szServer, 0, sizeof( g_sccParams.szServer ) );
        memcpy( g_sccParams.szServer, szServer, 64 );
        g_sccParams.szServer[63] = 0;
    }

    memset( szUser, 0, sizeof( szUser ) );
    iRet = GetStrParamValue( pparam, "scc_user", szUser, 32 );

    if ( iRet == 0 )
    {
        memset( g_sccParams.szUser, 0, sizeof( g_sccParams.szUser ) );
        memcpy( g_sccParams.szUser, szUser, 32 );
        g_sccParams.szUser[31] = 0;
    }

    memset( szPwd, 0, sizeof( szPwd ) );
    iRet = GetStrParamValue( pparam, "scc_pwd", szPwd, 32 );

    if ( iRet == 0 )
    {
        memset( g_sccParams.szPwd, 0, sizeof( g_sccParams.szPwd ) );
        memcpy( g_sccParams.szPwd, szPwd, 32 );
        g_sccParams.szPwd[31] = 0;
    }

    iRet = GetIntParamValue( pparam, "scc_port", &nValue );

    if ( iRet == 0 )
    {
        g_sccParams.nServerPort = nValue;
    }

    memset( nexturl, 0, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;

}

#endif

int cgigetsmarteye( unsigned char* pbuf, unsigned char* pparam, unsigned char byPri )
{
    /* BEGIN: Deleted by wupm, 2013/3/9 */
#if	1
    return 0;
#else
    unsigned char   temp[2048];
    int             len = 0;
    int             result = 0;
    memset( temp, 0x00, 2048 );
    len += sprintf( temp + len, "var mac=\"%s\";\r\n", eyeparam.mac );
    len += sprintf( temp + len, "var mid=\"%s\";\r\n", eyeparam.mid );
    len += sprintf( temp + len, "var pid=\"%s\";\r\n", eyeparam.pid );
    len += sprintf( temp + len, "var sn=\"%s\";\r\n", eyeparam.sn );
    len += sprintf( temp + len, "var se_ddns_enable=%d;\r\n", eyeparam.se_ddns_enable );
    len += sprintf( temp + len, "var se_ddns_svr=\"%s\";\r\n", eyeparam.se_ddns_svr );
    len += sprintf( temp + len, "var se_ddns_port=%d;\r\n", eyeparam.se_ddns_port );
    len += sprintf( temp + len, "var se_ddns_interval=%d;\r\n", eyeparam.se_ddns_interval );
    len += sprintf( temp + len, "var se_ddns_name=\"%s\";\r\n", eyeparam.se_ddns_name );
    len += sprintf( temp + len, "var se_ddns_user=\"%s\";\r\n", eyeparam.se_ddns_user );
    len += sprintf( temp + len, "var se_ddns_pwd=\"%s\";\r\n", eyeparam.se_ddns_pwd );
    result = GetFactoryStatus();
    len += sprintf( temp + len, "var se_ddns_status=%d;\r\n", result );
    memcpy( pbuf, temp, len );
    return len;
#endif
    /* END: Deleted by wupm, 2013/3/9 */
}

void GetAppVersion( char* appver )
{
    FILE* fp = NULL;
    fp = fopen( "/system/www/appversion.txt", "r" );

    if ( fp == NULL )
    {
        return;
    }

    fgets( appver, 16, fp );
    fclose( fp );
    //printf("====appver===%d========\n",strlen(appver));
    //add process \r\n
}

int cgigetappversion( unsigned char* pbuf, unsigned char* pparam, unsigned char byPri )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             result = 0;
    char		appver[16];

    memset( appver, 0x00, 16 );
    memset( temp, 0x00, 2048 );
    GetAppVersion( appver );
    len += sprintf( temp + len, "var appver=\"%s\";\r\n", appver );
    memcpy( pbuf, temp, len );
    return len;
}

int cgisetsmarteye( char* pbuf, char* pparam, unsigned char byPri )
{
    /* BEGIN: Deleted by wupm, 2013/3/9 */
#if	1
    return 0;
#else
    unsigned char           temp[2048];
    char                            temp1[64];
    char                            temp2[64];
    int                             len  = 0;
    int                             iRet = 0;
    char                            passtemp[32];
    char                            usertemp[32];
    char                            nexturl[64];
    int                             value;
    int                             iValue;
    char                            i = 0;
    char                            flag = 0;
    char                            decoderbuf[128];

    printf( "cgisetsmarteye:[%s]\n", pparam );

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    memset( temp, 0x00, 2048 );

    memset( temp2, 0x00, 32 );

    if ( !GetStrParamValue( pparam, "mac", temp2, 17 ) )
    {
        char* pmac = NULL;
        unsigned char mac;
        unsigned char mactemp[4];
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );

        memset( eyeparam.mac, 0x00, 17 );
        strcpy( eyeparam.mac, temp2 );
        printf( "---set factory_wifimac=[%s]\n", temp2 );
        nvram_bufset( RT2860_NVRAM, "factory_wifimac", temp2 );

        mac = GetLastMac( temp2[15], temp2[16] );
        memset( mactemp, 0x00, 4 );
        sprintf( mactemp, "%x", mac );
        printf( "%02x-%02x-%02x-%02x\n", mactemp[0], mactemp[1], mactemp[2], mactemp[3] );
        temp2[15] = mactemp[0];
        temp2[16] = mactemp[1];
        printf( "---set factory_mac:%s\n", temp2 );
        nvram_bufset( RT2860_NVRAM, "factory_mac", temp2 );

        //sync ssid
        memset( bparam.stWifiRoute.szSSID, 0x00, 64 );
    }

    memset( temp2, 0x00, 32 );

    if ( !GetStrParamValue( pparam, "mid", temp2, 31 ) )
    {
        char* pmac = NULL;
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );
        memset( eyeparam.mid, 0x00, 32 );
        strcpy( eyeparam.mid, temp2 );
    }

    memset( temp2, 0x00, 32 );

    if ( !GetStrParamValue( pparam, "pid", temp2, 31 ) )
    {
        char* pmac = NULL;
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );
        memset( eyeparam.pid, 0x00, 32 );
        strcpy( eyeparam.pid, temp2 );
    }

    memset( temp2, 0x00, 32 );

    if ( !GetStrParamValue( pparam, "sn", temp2, 31 ) )
    {
        char* pmac = NULL;
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );
        memset( eyeparam.sn, 0x00, 32 );
        strcpy( eyeparam.sn, temp2 );
    }

    /* BEGIN: Modified by wupm, 2013/1/11 */
    //if( !GetIntParamValue( pparam, "ddns_enable", &value ) )
    if ( !GetIntParamValue( pparam, "se_ddns_enable", &value ) )
    {
        eyeparam.se_ddns_enable = value;
    }

    /* BEGIN: Modified by wupm, 2013/1/11 */
    //if( !GetIntParamValue( pparam, "ddns_interval", &value ) )
    if ( !GetIntParamValue( pparam, "se_ddns_interval", &value ) )
    {
        eyeparam.se_ddns_interval = value;
    }

    memset( temp2, 0x00, 32 );

    /* BEGIN: Modified by wupm, 2013/1/11 */
    //if( !GetStrParamValue( pparam, "ddns_svr", temp2, 31 ) )
    if ( !GetStrParamValue( pparam, "se_ddns_svr", temp2, 31 ) )
    {
        char* pmac = NULL;
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );

        memset( eyeparam.se_ddns_svr, 0x00, 32 );
        strcpy( eyeparam.se_ddns_svr, temp2 );
    }

    memset( temp2, 0x00, 32 );

    /* BEGIN: Modified by wupm, 2013/1/11 */
    //if( !GetIntParamValue( pparam, "ddns_port", &value ) )
    if ( !GetIntParamValue( pparam, "se_ddns_port", &value ) )
    {
        eyeparam.se_ddns_port = value;
    }

    memset( temp1, 0x00, 64 );

    /* BEGIN: Modified by wupm, 2013/1/11 */
    //if( !GetStrParamValue( pparam, "ddns_name", temp1, 63 ) )
    if ( !GetStrParamValue( pparam, "se_ddns_name", temp1, 63 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 31 );
        memset( temp1, 0x00, 32 );
        strcpy( temp1, decoderbuf );

        memset( eyeparam.se_ddns_name, 0x00, 64 );
        strcpy( eyeparam.se_ddns_name, temp1 );
        printf( "eyeparam.se_ddns_name = [%s]\n", eyeparam.se_ddns_name );
    }

    memset( temp1, 0x00, 64 );

    /* BEGIN: Modified by wupm, 2013/1/11 */
    //if( !GetStrParamValue( pparam, "ddns_user", temp1, 63 ) )
    if ( !GetStrParamValue( pparam, "se_ddns_user", temp1, 63 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 63 );
        memset( temp1, 0x00, 63 );
        strcpy( temp1, decoderbuf );

        memset( eyeparam.se_ddns_user, 0x00, 64 );
        strcpy( eyeparam.se_ddns_user, temp1 );
    }

    memset( temp1, 0x00, 64 );

    /* BEGIN: Modified by wupm, 2013/1/11 */
    //if( !GetStrParamValue( pparam, "ddns_pwd", temp1, 63 ) )
    if ( !GetStrParamValue( pparam, "se_ddns_pwd", temp1, 63 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 63 );
        memset( temp1, 0x00, 63 );
        strcpy( temp1, decoderbuf );

        memset( eyeparam.se_ddns_pwd, 0x00, 64 );
        strcpy( eyeparam.se_ddns_pwd, temp1 );
    }

    bparam.stDdnsParam.factoryversion = 13;	//smarteye
    WriteSmartEye();
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    nvram_commit( RT2860_NVRAM );
    return len;
#endif
    /* END: Deleted by wupm, 2013/3/9 */
}


/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
#if	0
int GetExtraParameters( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   	temp[1024 * 4];
    int             		len = 0;

    memset( temp, 0x00, 1024 * 4 );
    len += sprintf( temp + len, "var close_ap=%d;\n", bparam.stExtraParam.bDisableAP );
    len += sprintf( temp + len, "var close_mic=%d;\n", bparam.stExtraParam.bAudioLineIN );
    memcpy( pbuf, temp, len );
    return len;
}

/* BEGIN: Added by wupm, 2013/5/3 */
int SetExtraParameters( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char           		temp[2048];
    char                            nexturl[64];
    int                             iValue = 0;
    int								len = 0;
    int								iRet = 0;
    char                            bSave = FALSE;

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    iRet = GetIntParamValue( pparam, "close_ap", &iValue );

    if ( iRet == 0 )
    {
        bparam.stExtraParam.bDisableAP = iValue;
        bSave = TRUE;
    }

    iRet = GetIntParamValue( pparam, "close_mic", &iValue );

    if ( iRet == 0 )
    {
        bparam.stExtraParam.bAudioLineIN = iValue;
        bSave = TRUE;
    }

    /* BEGIN: Added by wupm, 2013/5/24 */

    iRet = GetIntParamValue2( pparam, "devicetype", &iValue );

    if ( iRet == 0 )
    {
        bparam.stExtraParam.nDevicetype = iValue;
        IETextout( "IE Set devicetype = %u", bparam.stExtraParam.nDevicetype );
        bSave = TRUE;
    }

    /*
    iRet = GetIntParamValue( pparam, "devicesubtype", &iValue );

    if ( iRet == 0 )
    {
        bparam.stExtraParam.nDeviceSubtype = iValue;
    	bSave = TRUE;
    }
    */

    if ( bSave )
    {
        NoteSaveSem();
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    memset( temp, 0, 2048 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

/* BEGIN: Added by wupm, 2013/5/21 */
int GetOemLxmCfg( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char   temp[8192];
    int             len = 0;
    char		i;
    char		temp1[64];
    char		temp2[64];
    char		temp3[16];
    int 		iRet;
    char		userflag = 0x00;

    memset( temp, 0x00, 8192 );

    //svr0=serverip:port&CameraId=xxx&updateInterval=xxx
    //staticip=xxx.xxx.xxx.xxx&port=yyy
    //mask=1.1.1.1&gateway=1.1.1.1&dns1=1.1.1.1&dns2=1.1.1.1
    //wifi_enable=1&ssid=xxx&AuthMode=WPA2PSK&EncrypType=AES&wifiPassword=yyy

    len += sprintf( temp + len, "var svr0=%s:%d;\r\n", bparam.stExtraParam.szServerIP, bparam.stExtraParam.nServerPort );
    len += sprintf( temp + len, "var CameraId=%s;\r\n", bparam.stExtraParam.szCameraID );
    len += sprintf( temp + len, "var updateInterval=%d;\r\n", bparam.stExtraParam.nInterval );

    len += sprintf( temp + len, "var staticip=%s;\r\n", bparam.stNetParam.szIpAddr );
    len += sprintf( temp + len, "var port=%d;\r\n", bparam.stNetParam.nPort );
    len += sprintf( temp + len, "var mask=%s;\r\n", bparam.stNetParam.szMask );
    len += sprintf( temp + len, "var gateway=%s;\r\n", bparam.stNetParam.szGateway );
    len += sprintf( temp + len, "var dns1=%s;\r\n", bparam.stNetParam.szDns1 );
    len += sprintf( temp + len, "var dns2=%s;\r\n", bparam.stNetParam.szDns2 );

    memcpy( pbuf, temp, len );
    return len;
}

/* BEGIN: Added by wupm, 2013/5/21 */
int SetOemLxmCfg( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char           		temp[2048];
    char                            nexturl[64];
    int                             iValue = 0;
    int								len = 0;
    int								iRet = 0;
    BOOL                            bSave = FALSE;
    BOOL							bContinue = FALSE;

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

    IETextout( "IE = [%s]", pparam );

    //svr0=serverip:port&CameraId=xxx&updateInterval=xxx
    //staticip=xxx.xxx.xxx.xxx&port=yyy
    //mask=1.1.1.1&gateway=1.1.1.1&dns1=1.1.1.1&dns2=1.1.1.1
    //wifi_enable=1&ssid=xxx&AuthMode=WPA2PSK&EncrypType=AES&wifiPassword=yyy

    iRet = GetIntParamValue( pparam, "updateInterval", &iValue );

    if ( iRet == 0 )
    {
        bparam.stExtraParam.nInterval = iValue;
        memset( nexturl, 0x00, 64 );
        iRet = GetStrParamValue( pparam, "CameraId", nexturl, 63 );

        if ( iRet == 0 && nexturl[0] != 0 )
        {
            strcpy( bparam.stExtraParam.szCameraID, nexturl );
            bContinue = TRUE;
        }

        memset( nexturl, 0x00, 64 );
        iRet = GetStrParamValue( pparam, "svr0", nexturl, 63 );

        if ( iRet == 0 && nexturl[0] != 0 && bContinue )
        {
            int nIndex = 0;
            int nLen = strlen( nexturl );

            for ( nIndex = 0; nIndex < nLen; nIndex++ )
            {
                if ( nexturl[nIndex] == ':' )
                {
                    memset( bparam.stExtraParam.szServerIP, 0, 16 );
                    memcpy( bparam.stExtraParam.szServerIP, nexturl, nIndex );
                    bparam.stExtraParam.nServerPort = atoi( nexturl + nIndex + 1 );
                    IETextout( "Server = [%s], Port = %d", bparam.stExtraParam.szServerIP, bparam.stExtraParam.nServerPort );
                    bSave = TRUE;
                    break;
                }
            }
        }
    }

    bContinue = FALSE;
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "staticip", nexturl, 63 );

    if ( iRet == 0 && nexturl[0] != 0 )
    {
        iRet = CheckIPok( nexturl );

        if ( iRet == 0 )
        {
            bparam.stNetParam.byIsDhcp = 0;
            strcpy( bparam.stNetParam.szIpAddr, nexturl );
            bContinue = TRUE;
        }
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "mask", nexturl, 63 );

    if ( iRet == 0 && nexturl[0] != 0 && bContinue )
    {
        iRet = CheckIPok( nexturl );

        if ( iRet == 0 )
        {
            strcpy( bparam.stNetParam.szMask, nexturl );
            bContinue = TRUE;
        }

        else
        {
            bContinue = FALSE;
        }
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "gateway", nexturl, 63 );

    if ( iRet == 0 && nexturl[0] != 0 && bContinue )
    {
        iRet = CheckIPok( nexturl );

        if ( iRet == 0 )
        {
            strcpy( bparam.stNetParam.szGateway, nexturl );
            bContinue = TRUE;
        }

        else
        {
            bContinue = FALSE;
        }
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "dns1", nexturl, 63 );

    if ( iRet == 0 && nexturl[0] != 0 && bContinue )
    {
        iRet = CheckIPok( nexturl );

        if ( iRet == 0 )
        {
            strcpy( bparam.stNetParam.szDns1, nexturl );
            bContinue = TRUE;
        }

        else
        {
            bContinue = FALSE;
        }
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "dns2", nexturl, 63 );

    if ( iRet == 0 && nexturl[0] != 0 )
    {
        iRet = CheckIPok( nexturl );

        if ( iRet == 0 )
        {
            strcpy( bparam.stNetParam.szDns2, nexturl );
        }
    }

    iRet = GetIntParamValue( pparam, "port", &iValue );

    if ( iRet == 0 && bContinue )
    {
        if ( iValue >= 80 )
        {
            bparam.stNetParam.nPort = iValue;
            bSave = TRUE;
        }
    }

    /*
    iRet = GetIntParamValue( pparam, "wifi_enable", &iValue );
    if ( iRet == 0 )
    {
        bparam.stExtraParam.nInterval = iValue;
    	bSave = TRUE;
    	memset( nexturl, 0x00, 64 );
        iRet = GetStrParamValue( pparam, "ssid", nexturl, 63 );
    	if ( iRet == 0 && nexturl[0] != 0 )
    	{
    	}
    	memset( nexturl, 0x00, 64 );
        iRet = GetStrParamValue( pparam, "AuthMode", nexturl, 63 );
    	if ( iRet == 0 && nexturl[0] != 0 )
    	{
    	}
    	memset( nexturl, 0x00, 64 );
        iRet = GetStrParamValue( pparam, "EncrypType", nexturl, 63 );
    	if ( iRet == 0 && nexturl[0] != 0 )
    	{
    	}
    	memset( nexturl, 0x00, 64 );
        iRet = GetStrParamValue( pparam, "wifiPassword", nexturl, 63 );
    	if ( iRet == 0 && nexturl[0] != 0 )
    	{
    	}
    }
    */

    if ( bSave )
    {
        NoteSaveSem();
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    memset( temp, 0, 2048 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}
#endif

#ifdef	CAMERA_SETUP
/* BEGIN: Added by wupm, 2013/6/6 */
int CGISetCameraParams( char* pbuf, char* pparam, unsigned char byPri )
{
	return 0;
}
#endif

#ifdef CUSTOM_HTTP_ALARM
int GetAlarmEnable()
{
	return psdparam.bEnableAlarmSend;
}
void GetAlarmServer(char *svr, int *pPort)
{
	if ( psdparam.alarm_svr[0] == 0 )
		svr[0] = 0;
	else
		strcpy(svr, psdparam.alarm_svr);
	*pPort = psdparam.alarm_port;
}
void GetAlarmUser(char *user, char *pwd)
{
	if ( psdparam.alarm_user[0] == 0 )
		user[0] = 0;
	else
		strcpy(user, psdparam.alarm_user);

	if ( psdparam.alarm_pwd[0] == 0 )
		pwd[0] = 0;
	else
		strcpy(pwd, psdparam.alarm_pwd);
}

int CGIGetAlarmServer( char* pbuf, char* pparam, unsigned char byPri )
{
	unsigned char   temp[2048];
    int             len = 0;
    int             result = 0;
    memset( temp, 0x00, 2048 );
    len += sprintf( temp + len, "var alarm_svr=\"%s\";\r\n", psdparam.alarm_svr );
    len += sprintf( temp + len, "var alarm_user=\"%s\";\r\n", psdparam.alarm_user );
    len += sprintf( temp + len, "var alarm_pwd=\"%s\";\r\n", psdparam.alarm_pwd );
    len += sprintf( temp + len, "var alarm_port=\"%d\";\r\n", psdparam.alarm_port );
    memcpy( pbuf, temp, len );
    return len;
}
/* BEGIN: Added by wupm, 2013/5/25 */
int CGISetAlarmServer( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char           temp[2048];
    char                            temp1[64];
    char                            temp2[64];
    int                             len  = 0;
    int                             iRet = 0;
    char                            passtemp[32];
    char                            usertemp[32];
    char                            nexturl[64];
    int                             value;
    int                             iValue;
    char                            i = 0;
    char                            flag = 0;
    char                            decoderbuf[128];

	/* BEGIN: Added by wupm, 2013/5/31 */
	BOOL	bUpdate = FALSE;

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

	//alarm_svr：		报警服务器，长度 <= 64
	//alarm_port：		报警服务端口
	//alarm_user：		报警服务器用户，长度 <= 64
	//alarm_pwd：		报警服务器密码，长度 <= 64

    memset( temp, 0x00, 2048 );

    memset( temp2, 0x00, 32 );

    if ( !GetStrParamValue( pparam, "alarm_svr", temp2, 63 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );

		memset(psdparam.alarm_svr, 0, 64 );
		strcpy(psdparam.alarm_svr, temp2);
		bUpdate = TRUE;
    }

/* BEGIN: Added by wupm, 2013/7/22 */
    memset( temp2, 0x00, 32 );
    if ( !GetStrParamValue( pparam, "alarm_user", temp2, 63 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );

		memset(psdparam.alarm_user, 0, 64 );
		strcpy(psdparam.alarm_user, temp2);
		bUpdate = TRUE;
    }

    memset( temp2, 0x00, 32 );
    if ( !GetStrParamValue( pparam, "alarm_pwd", temp2, 63 ) )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( temp2, 0x00, 32 );
        strcpy( temp2, decoderbuf );

		memset(psdparam.alarm_pwd, 0, 64 );
		strcpy(psdparam.alarm_pwd, temp2);

		bUpdate = TRUE;
    }

	iRet = GetIntParamValue( pparam, "alarm_port", &iValue );
    if ( iRet == 0 )
    {
        psdparam.alarm_port = iValue;
		bUpdate = TRUE;
    }

	if ( bUpdate )
		WritePsdParams();

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

#endif

#ifdef TENVIS
//http://192.168.1.106:81/set_tenvisddns.cgi?loginuse=admin&loginpas=&fac_enable=1
//http://192.168.1.106:81/get_tenvisddns.cgi?loginuse=admin&loginpas=
int CGIGetTenvisDdns( char* pbuf, char* pparam, unsigned char byPri )
{
	unsigned char   temp[128];
    int             len = 0;

    memset( temp, 0x00, 128 );
    len += sprintf( temp + len, "var fac_enable=\"%d\";\r\n", tenvisparam.nEnable);
    memcpy( pbuf, temp, len );
    return len;
}
/* BEGIN: Added by wupm, 2013/5/25 */
int CGISetTenvisDdns( char* pbuf, char* pparam, unsigned char byPri )
{
    unsigned char           temp[2048];
    int                             len  = 0;
    int                             iRet = 0;
    char                            nexturl[64];
    int                             iValue = 0;

	/* BEGIN: Added by wupm, 2013/5/31 */
	BOOL	bUpdate = FALSE;

    if ( 0 ) //byPri != ADMIN )
    {
        return 0;
    }

	iRet = GetIntParamValue( pparam, "fac_enable", &iValue );
    if ( iRet == 0 )
    {
        tenvisparam.nEnable = iValue;
		bUpdate = TRUE;
        if(iValue == 1)
        {
            NoteTenvisFatoryDdns();
        }
    }

	if ( bUpdate )
	{
		WriteTenvisParams();
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "next_url", nexturl, 63 );

    if ( iRet == 0x00 )
    {
        len += RefreshUrl( temp + len, nexturl );
        memcpy( pbuf, temp, len );
    }

    else
    {
        len += sprintf( temp + len, "var result=\"ok\";\r\n" );
        memcpy( pbuf, temp, len );
    }

    return len;
}

#endif

int  CgiProcess( unsigned short cgicmd, char* url, char* pbuf, int len, char auth, unsigned char pri )
{
    int 				iRet = 0;
    char*				pparam;
    unsigned char		byPri;
    //printf("CgiProcess cgicmd %x pparam:%s len %d\n",cgicmd,temp1,strlen(temp1));
    //printf("cgicmd %x len %d pparam:%s\n",cgicmd,len,url);

    printf("cgicmd %x auth %d  byPri %d len %d url:%s\n",cgicmd,auth,pri,len,url);
    wifiok = 1;

    if ( cgicmd != CGI_UPGRADE_APP )
    {
        if ( cgicmd != CGI_UPGRADE_SYS )
        {
            if ( auth == 0x00 )
            {
                char temp[512];
                int  wlen = 0;

                if ( len )
                {
                    return 0;
                }

                #if 0
                byPri = AdjustUserPri( url );

                printf("url:%s byPri %d\n",url,byPri);
                if ( byPri == 0x00 )
                {
                    memset( temp, 0x00, 512 );
                    wlen += sprintf( temp + wlen, "var result=\"Auth Failed\";\r\n" );
                    memcpy( pbuf, temp, wlen );
                    return wlen;
                }
                #else
                byPri = 255;
                #endif
            }

            else
            {
                byPri = pri;
            }
        }
    }

    if ( updateflag )
    {
        if ( cgicmd != CGI_IEREBOOT )
        {
            printf( "=============update...============\n" );
            return 0 ;
        }
    }

    if ( dbgflag++ < 10 )
    {
        pparam = strstr( url, ".cgi" );
    }

    else
    {
        pparam = strstr( url, ".cgi" );
        dbgflag = 0;
    }

    //printf("cgicmd %x\n",cgicmd);
    switch ( cgicmd )
    {
        case CGI_IEGET_STATUS:
            if ( len == 0x00 )
            {
				IETextout("Call cgigetstatus");
				/* BEGIN: Modified by wupm(2073111@qq.com), 2014/6/10 */
                iRet = cgigetstatus( pbuf, byPri );
            }

            break;

        case CGI_IEGET_PARAM:
            if ( len == 0x00 )
            {
				IETextout("Call cgigetparams");
				/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
                iRet = cgigetparams( pbuf, byPri );
            }

            break;

        case CGI_IEGET_RECORD:
            if ( len == 0x00 )
            {
                iRet = cgigetrecord( pbuf, byPri );
            }

            break;

        case CGI_IEGET_CAM_PARAMS:
            if ( len == 0x00 )
            {
                iRet = cgigetcamera( pbuf, byPri );
            }

            break;

        case CGI_IELOGIN:
            if ( len == 0x00 )
            {
                iRet = cgigetlogin( pbuf, byPri );
            }

            break;

        case CGI_IEGET_LOG:
            if ( len == 0x00 )
            {
                printf( "cgigetlog\n" );

                /* BEGIN: Modified by wupm, 2013/4/3 */
                //iRet = cgigetlog(pbuf,rlen,len,byPri);
                iRet = cgigetlog( pbuf, byPri );
            }

            break;

        case CGI_IEGET_RECORD_FILE:
            if ( len == 0x00 )
            {
                iRet = cgigetrecordfile( pbuf, pparam, byPri );
            }

            break;

        case CGI_IEGET_WIFI_SCAN:
            if ( len == 0x00 )
            {
                iRet = cgigetwifiscan( pbuf, byPri );
            }

            break;

        case CGI_IEGET_MISC:
            if ( len == 0x00 )
            {
                iRet = cgigetmisc( pbuf, byPri );
            }

            break;

        case CGI_IEGET_FTPTEST:
            if ( len == 0x00 )
            {
                iRet = cgigetftptest( pbuf, pparam, byPri );
            }

            break;

        case CGI_IESET_FTPTEST:
            if ( len == 0x00 )
            {
                iRet = cgisetftptest( pbuf, pparam, byPri );
            }

            break;

        case CGI_IEGET_MAILTEST:
            if ( len == 0x00 )
            {
                iRet = cgigetmailtest( pbuf, byPri );
            }

            break;

        case CGI_IESET_MAILTEST:
            if ( len == 0x00 )
            {
                iRet = cgisetmailtest( pbuf, pparam, byPri );
            }

            break;

        case CGI_IESET_ALIAS:
            if ( len == 0x00 )
            {
                iRet = cgisetalias( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_DATE:
            if ( len == 0x00 )
            {
                iRet = cgisetdate( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_DEVICE:
            if ( len == 0x00 )
            {
				/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
                //iRet = cgisetdevice( pbuf, pparam, byPri );
                //NoteSaveSem();
            }

            break;

        case CGI_IESET_NETWORK:
            if ( len == 0x00 )
            {
                iRet = cgisetnetwork( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_PPPOE:
            if ( len == 0x00 )
            {
                iRet = cgisetpppoe( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_UPNP:
            if ( len == 0x00 )
            {
                iRet = cgisetupnp( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_DDNS:
            if ( len == 0x00 )
            {
                iRet = cgisetddns( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_MAIL:
            if ( len == 0x00 )
            {
                iRet = cgisetmail( pbuf, pparam, byPri );
                IETextout( "-------------OK--------" );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_FTP:
            if ( len == 0x00 )
            {
                iRet = cgisetftp( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_RTSP:
            if ( len == 0x00 )
            {
                iRet = cgisetrtsp( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_ALARM:
            if ( len == 0x00 )
            {
                iRet = cgisetalarm( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_MEDIA:
            if ( len == 0x00 )
            {
                iRet = cgisetmedia( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_RECORDSCH:
            if ( len == 0x00 )
            {
                iRet = cgisetrecordsch( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_MISC:
            if ( len == 0x00 )
            {
                iRet = cgisetmisc( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_USER:
            if ( len == 0x00 )
            {
                iRet = cgisetuser( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_DEFAULT:
            if ( len == 0x00 )
            {
                iRet = cgisetdefault( pbuf, pparam, byPri );
            }

            break;

        case CGI_IESET_IR:
            if ( len == 0x00 )
            {
                iRet = cgisetgpioir( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_LOG:
            if ( len == 0x00 )
            {
                iRet = cgisetlog( pbuf, pparam, byPri );
            }

            break;

        case CGI_IESET_MOTO:
            if ( len == 0x00 )
            {
                iRet = cgisetmoto( pbuf, pparam, byPri );
            }

            break;

        case CGI_IESET_WIFI:
            if ( len == 0x00 )
            {
                iRet = cgisetwifi( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IESET_WIFISCAN:
            if ( len == 0x00 )
            {
                iRet = cgisetwifiscan( pbuf, pparam, byPri );
            }

            break;

        case CGI_IESET_SNAPSHOT:
            if ( len == 0x00 )
            {
                if ( snapflag == 0x00 )
                {
                    snapflag = 0x01;
                    //iRet = cgisetsnapshot(pbuf,pparam,byPri,frameno);
                    //printf("iRet:%d\n",iRet);
                    snapflag = 0x00;
                }
            }

            break;

        case CGI_IEGET_VIDEOSTREAM:
        {
            static char times = 0;

            if ( snapflag == 0x00 )
            {
                snapflag = 0x01;
                iRet = cgisetvideostream( pbuf, pparam, index );

                if ( len == 0x00 )
                {
                    times = 0x00;
                }

                snapflag = 0x00;
            }
        }
        break;

        case CGI_CAM_CONTROL:
            if ( len == 0x00 )
            {
                iRet = cgisetcamcontrol( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_DECODER_CONTROL:
            if ( len == 0x00 )
            {
                iRet = cgisetdecodercontrol( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_IEDEL_FILE:
            if ( len == 0x00 )
            {
                iRet = cgisetdelfile( pbuf, pparam, byPri );
            }

            break;

        case CGI_IEREBOOT:
            if ( len == 0x00 )
            {
                iRet = cgisetreboot( pbuf, pparam, byPri );
				IETextout("software Reboot....");
                SetRebootCgi();
            }

            break;

        case CGI_IERESTORE:
            if ( len == 0x00 )
            {
                iRet = cgisetrestore( pbuf, pparam, byPri );
                //SetRebootCgi();
            }

            break;

        case CGI_IEGET_FACTORY:
            if ( len == 0x00 )
            {
                iRet = GetFactoryCgi( pbuf, byPri );
            }

            break;

        case CGI_IESET_FACTORY:
            if ( len == 0x00 )
            {
                iRet = cgisetfactory( pbuf, pparam, byPri );
                //SetRebootCgi();
            }

            break;

		case CGI_IEGET_OPENLOCK:
            if ( len == 0x00 )
            {
                iRet = cgiOpenLock( pbuf, pparam, byPri );
            }
			break;
           

		/* BEGIN: Append CGI for Change UUID/MAC */
		/*        Added by wupm(2073111@qq.com), 2014/9/19 */
        case CGI_IESET_P2PUUID:
            if ( len == 0x00 )
            {
                iRet = CGISetP2PUUID( pbuf, pparam, byPri );
                //SetRebootCgi();
            }

            break;

        case CGI_IEGET_SYSWIFI:
            if ( len == 0x00 )
            {
                iRet = cgigetsyswifi( pbuf, pparam, byPri );
            }

            break;

        case CGI_IESET_SYSWIFI:
            if ( len == 0x00 )
            {
                iRet = cgisetsyswifi( pbuf, pparam, byPri );
                NoteSaveSem();
            }

            break;

        case CGI_UPGRADE_APP:
            if ( len == 0x00 )
            {
                printf( "APP:%s\n", pparam );
                iRet = cgiupgradeapp( pbuf, pparam, byPri );
                //SetRebootCgi();
            }

            break;

        case CGI_UPGRADE_SYS:
            if ( len == 0x00 )
            {
                printf( "SYS:%s\n", pparam );
                iRet = cgiupgradesys( pbuf, pparam, byPri );
            }

            break;

        case CGI_IEFORMATSD:
            if ( len == 0x00 )
            {
                iRet = cgiformatsd( pbuf, pparam, byPri );
            }

            break;

        case CGI_IEGET_ALARMLOG:
            if ( len == 0x00 )
            {
                iRet = cgigetalarmlog( pbuf, pparam, byPri );
            }

            break;

        case CGI_IESET_ALARMLOGCLR:
            if ( len == 0x00 )
            {
                iRet = cgisetalarmlogclr( pbuf, pparam, byPri );
            }

            break;

#ifdef	P2P_SCC_LIB

        case CGI_IEGET_SCC_CONFIG:
            if ( len == 0x00 )
            {
                iRet = cgiGetSccConfig( pbuf, pparam, byPri );
            }

            break;

        case CGI_IESET_SCC_CONFIG:
            if ( len == 0x00 )
            {
                iRet = cgiSetSccConfig( pbuf, pparam, byPri );
                NotifySccConfigeSave();
            }

#endif

        case CGI_IESET_BACKUPPARAM:
            if ( len == 0x00 )
            {
                iRet = cgisetbackupparam( pbuf, pparam, byPri );
            }

            break;

        case CGI_IEGET_SAVEPARAM:
            if ( len == 0x00 )
            {
                iRet = cgigetsaveparam( pbuf, pparam, byPri );
            }

            break;

        case CGI_GET_IIC:
            if ( len == 0x00 )
            {
                iRet = cgigetiic( pbuf, pparam, byPri );
            }

            break;

        case CGI_SET_IIC:
            if ( len == 0x00 )
            {
                iRet = cgisetiic( pbuf, pparam, byPri );
            }

            break;

            /* BEGIN: Deleted by wupm, 2013/1/11 */
            /*
            case CGI_SET_TUTK:
            	if( len == 0x00 )
            	{
            		iRet = cgisettutk( pbuf, pparam, byPri );
            	}

            	break;

            case CGI_GET_TUTK:
            	if( len == 0x00 )
            	{
            		iRet = cgigettutk( pbuf, pparam, byPri );
            	}

            	break;
            */
            /* END: Deleted by wupm, 2013/1/11 */
#ifdef	HONESTECH

        case CGI_SET_HTCLASS_PARAMS:
            if ( len == 0x00 )
            {
                iRet = SetHTClassParams( pbuf, pparam, byPri );
            }

            break;

        case CGI_SET_HTCLASS_ALARM:
            if ( len == 0x00 )
            {
                iRet = SetHTClassAlarm( pbuf, pparam, byPri );
            }

            break;

        case CGI_GET_HTCLASS:
            if ( len == 0x00 )
            {
                iRet = GetHTClass( pbuf, pparam, byPri );
            }

            break;
#endif



#if 1

        case CGI_IESET_SMARTEYE:
            if ( len == 0x00 )
            {
                iRet = cgisetsmarteye( pbuf, pparam, byPri );
            }

            break;

        case CGI_IEGET_SMARTEYE:
            if ( len == 0x00 )
            {
                iRet = cgigetsmarteye( pbuf, pparam, byPri );
            }

            break;
#endif

        case CGI_IEGET_APPVERSION:
            if ( len == 0x00 )
            {
                iRet = cgigetappversion( pbuf, pparam, byPri );
            }

            break;

            /* BEGIN: Added by wupm, 2013/3/28 */
        case CGI_SET_REMOTEDEBUG:
            if ( len == 0x00 )
            {
                //iRet = SetRemoteDebug( pbuf, pparam, byPri );
            }

            break;

            /* BEGIN: Added by wupm, 2013/4/11 */
        case CGI_SET_DEBUGVAR:
            if ( len == 0x00 )
            {
                //iRet = SetDebugVar( pbuf, pparam, byPri );
            }

            break;

            /* BEGIN: Added by wupm, 2013/5/3 */
        case CGI_SET_EXTRA:
            if ( len == 0x00 )
            {
				/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
                //iRet = SetExtraParameters( pbuf, pparam, byPri );
            }

            break;

		case CGI_GET_EXTRA:
            if ( len == 0x00 )
            {
				/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
                //iRet = GetExtraParameters( pbuf, pparam, byPri );
            }
            break;


            /* BEGIN: Added by wupm, 2013/5/21 */
        case CGI_SET_OEM_LXM:
            if ( len == 0x00 )
            {
				/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
                //iRet = SetOemLxmCfg( pbuf, pparam, byPri );
            }

            break;

        case CGI_GET_OEM_LXM:
            if ( len == 0x00 )
            {
				/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
                //iRet = GetOemLxmCfg( pbuf, pparam, byPri );
            }

            break;

#ifdef	CAMERA_SETUP

            /* BEGIN: Added by wupm, 2013/6/6 */
        case CGI_IESET_CAMERA_PARAMS:
            if ( len == 0x00 )
            {
                iRet = CGISetCameraParams( pbuf, pparam, byPri );
            }

            break;
#endif

#ifdef	AUTO_DOWNLOAD_FIRMWIRE

        case CGI_AUTO_DOWNLOAD_FILE:
            if ( len == 0x00 )
            {
                iRet = cgidownloadfile( pbuf, pparam, byPri );
            }

            break;
#endif

#ifdef	SUPPORT_MORE_SENSOR
		case CGI_SET_SENSOR:
            if ( len == 0x00 )
            {
                iRet = CGISetSensor( pbuf, pparam, byPri );
				if ( iRet == 0 )
				{
					IETextout("NOOO Right");
				}
				else
				{
					NoteSaveSem();
				}
            }
            break;
		case CGI_GET_SENSOR:
            if ( len == 0x00 )
            {
                iRet = CGIGetSensor( pbuf, pparam, byPri );
            }
            break;
#endif

#ifdef CUSTOM_HTTP_ALARM
		case CGI_SET_ALARM_SERVER:
            if ( len == 0x00 )
            {
                iRet = CGISetAlarmServer( pbuf, pparam, byPri );
            }
            break;
		case CGI_GET_ALARM_SERVER:
            if ( len == 0x00 )
            {
                iRet = CGIGetAlarmServer( pbuf, pparam, byPri );
            }
            break;
#endif

#ifdef TENVIS
		case CGI_SET_TENVIS_DDNS:
            if ( len == 0x00 )
            {
                iRet = CGISetTenvisDdns( pbuf, pparam, byPri );
            }
            break;
		case CGI_GET_TENVIS_DDNS:
            if ( len == 0x00 )
            {
                iRet = CGIGetTenvisDdns( pbuf, pparam, byPri );
            }
            break;

#endif

        default:
            break;
    }

    return iRet;
}

void IeparamInit( void )
{
    ScanThreadInit();
}

