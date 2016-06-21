/* ****************************************************************************

  Copyright (C), 2011-2012, wupm(szwpm@Tom.com) Co., Ltd.

 ******************************************************************************
  File Name     : debug.h
  Version       : Initial Draft
  Author        : szwpm@Tom.com
  Created       : 2012/12/21
  Last Modified :
  Description   : Debug Output
  Function List :
  History       :
  1.Date        : 2012/12/21
    Author      : szwpm@Tom.com
    Modification: Created file

******************************************************************************/


#ifndef WUPM_DEBUG_H
#define WUPM_DEBUG_H

#include <stdio.h>

#define YINETWORK

#ifdef SENSOR_3861
#undef SENSOR_3861
#define SENSOR_3861
#endif

#ifdef SENSOR_8433
#undef SENSOR_8433
#define SENSOR_8433
#define USE_SENSOR_DEFAULT
#endif
//#define   FOBJECTTEST     //IPCAERA
//#define   FOBJECT         //RISEN-DOOR
//#define   FOBJECT_FM34
//#define     FUHONG
//#define     EYESIGHT
//#define   FRISEN          //RISEN-DOOR
//#define   FACTOP          //ACTOP
//#define   FEEDDOG
//#define   ZIGBEE
//#define   KONX
//#define   ZILINK
#define ZHENGSHOW   //20141124
//#define     KANGJIEDENG
//#define     YUELAN
//#define     BELINK
//#define     JINQIANXIANG
//#define  BAFANGDIANZI
////////////////////////////////////////////////////////////////////////////////

//#define   CUSTOMER_ID     0
//0=OJT
//60=FOBJECT_FM34
//12=FUHONG
//15=EYESIGHT
//1=GO
//2=KCOX
//8=ZIGBEE
//9=AHX
//10=ACTOP
//3=ZILINK
//4=ZHENGSHOW
//5=KANGJIEDENG
//7=YUELAN
//11=BELINK
//16=JINQIANXIANG
//61=BAFANGDIANZI


#define LANGUAGE_ID 2   //1=Chinese
						//2=English

#define PRODUCT_ID  1   //1=MJPEG
//2=HD720(AIT-8433/SONIX 291A/B)
//0=DONT SUPPORT IOS, APK VERSION V0730

#define VER_SOFTWARE    ( PRODUCT_ID * 10 + LANGUAGE_ID )
#define MAJOR_VERSION   3

////////////////////////////////////////////////////////////////////////////////
#define REMOVE_AUDIOSTREAM_CGI
#define SYSTEM_PARAM_FILENAME   "/system/www/BellParam.bin"
#define AUTO_UUID_FILENAME  "/system/www/auto.bin"
#define DEFAULT_UUID        "OBJ-123456-ABCDE"

#define TALK_CALLING    0   //ID=OK, state=CALLING
#define TALK_EXPIRED    1   //ID=INVALID
#define TALK_OVER       2   //ID=NULL
#define TALK_NOBYAGREE  3   //ID=OK, state=IDLE
#define TALK_TAKING     4   //ID=OK, state=TALKING

#define ENABLE_IOS_MESSAGE

#define BELL_V2_1
#define CAMERA_TYPE         0   //0=MJ, 1=HD
#define HARDWARE_VERSION    "1.1.20140323"

//#define   SUPPORT_IPCAMERA
#define ENABLE_AUDIO_PLAY
#define REMOVE_ENCRYPT

#define SUPPORT_WPS

#define ENABLE_APPEND_SNAPSHOT_HEAD
#define ENABLE_BELL_LOG
#define INSMOD_UVC_DRIVER

//////////////////////////////////////////////////////////////////////////

#ifdef  FOBJECTTEST
#define CUSTOMER_ID     100
#endif

#ifdef  FOBJECT
#define CUSTOMER_ID     0

//#define JPUSH

/* BEGIN: Added by yiqing, 2015/4/7 */
//#define P2P_BIZ_LIB
#define XQ_P2P

/* BEGIN: Added by yiqing, 2015/4/13 */
#define BAGGIO_API

/* BEGIN: Added by yiqing, 2015/5/11 */
#define LDPUSH


#endif

#ifdef  FOBJECT_FM34
#define CUSTOMER_ID     9
#define MAJOR_VERSION   4

#define SUPPORT_FM34
//#define JPUSH
#define LDPUSH
#define XQ_P2P

#define BAGGIO_API

#endif

#ifdef FUHONG
#define CUSTOMER_ID     12
#define SUPPORT_FM34
#define VOLUME_INCREASE
#define CHANGE_VIDEO_MIRROR
#endif

#ifdef  EYESIGHT
#define CUSTOMER_ID     15
#define SUPPORT_FM34
#endif

#ifdef JINQIANXIANG
#define CUSTOMER_ID     16
#endif

#ifdef  KANGJIEDENG
#define CUSTOMER_ID     5
#endif


#ifdef  ZHENGSHOW
#define CUSTOMER_ID     4
#undef MAJOR_VERSION

/***************version*****************/
//#define UK_CUSTOMERS_OLD_KERNEL
#define UK_CUSTOMERS_NEW_KERNEL
//#define OLD_KERNEL_OBJ
//#define PREFIX_OBJ
//#define PREFIX_ZSKJ
//#define OLD_KERNEL_XDBL
//#define NEW_KERNEL_XDBL
//#define PPCS_P2P_TEST

/***************************************/


#if defined (UK_CUSTOMERS_OLD_KERNEL)//old kernel / prefix AEX
#define MAJOR_VERSION   3
#undef INSMOD_UVC_DRIVER

#elif defined (OLD_KERNEL_OBJ)   //old kernel
#define MAJOR_VERSION   4
//#define   MAJOR_VERSION   3

#undef INSMOD_UVC_DRIVER

#elif  defined (UK_CUSTOMERS_NEW_KERNEL)//prefix AEX
#define MAJOR_VERSION   5


#define NEW_BRAOD_AES  //英国客户新版
#ifdef NEW_BRAOD_AES
#define SUPPORT_FM34
#define SUPPORT_IRCUT
#endif
/* add begin by yiqing, 2016-05-13*/
#define PPCS_AES_P2P //善云p2p

#elif defined (PREFIX_OBJ)
#define MAJOR_VERSION   6

#elif defined (PREFIX_ZSKJ)
#define MAJOR_VERSION   7
//#undef INSMOD_UVC_DRIVER //test

#define SUPPORT_FM34
#define SUPPORT_IRCUT

#elif defined (OLD_KERNEL_XDBL)
#define MAJOR_VERSION   8
#undef INSMOD_UVC_DRIVER

#elif defined (NEW_KERNEL_XDBL)
#define MAJOR_VERSION   9

#elif defined (PPCS_P2P_TEST)
#define MAJOR_VERSION   10
#define SUPPORT_FM34
#define SUPPORT_IRCUT
#define PPCS_AES_P2P //善云p2p

#endif

//#define JPUSH
#define LDPUSH
#define BAGGIO_API
#ifndef SUPPORT_FM34
#define ES8388S
#endif

#ifndef PPCS_API
#define XQ_P2P
#endif
/* BEGIN: Added by yiqing, 2015/5/7 */
//#define LOCK_TYPE_NC

#endif


#ifdef  FRISEN
#define CUSTOMER_ID     1
#if PRODUCT_ID > 0      //SUPPORT IOS
#endif
#endif

#ifdef  FACTOP
#define CUSTOMER_ID     10
#define AUTO_DOWNLOAD_FIRMWARE
#undef SUPPORT_WPS
#endif

#ifdef  FEEDDOG
#define CUSTOMER_ID     9

#if 0  // old kernel
#define MAJOR_VERSION   2
#undef INSMOD_UVC_DRIVER
#define ES8388S
#endif

#define BAGGIO_API
//#define JPUSH
#define XQ_P2P

#define LDPUSH

/* add begin by yiqing, 2015-07-27, 原因: 中控*/
//#define ZHONGKONG


#endif



#ifdef  YUELAN
#define CUSTOMER_ID     7
#endif

#ifdef  BELINK
#define CUSTOMER_ID     11
/* add begin by yiqing, 2015-10-31*/
//#define DELAY_STARTUP

/* add begin by yiqing, 2015-10-31*/
#define LDPUSH
#define BAGGIO_API
//#define ES8388S
#define XQ_P2P

#endif

#ifdef  ZIGBEE
#define CUSTOMER_ID     8
#define DELAY_STARTUP
#endif

#ifdef  KONX
#define CUSTOMER_ID     2
#define AUTO_DOWNLOAD_FIRMWARE
#endif

#ifdef  ZILINK
#define CUSTOMER_ID     3
//#define DELAY_STARTUP

#undef SUPPORT_WPS

#define BAGGIO_API
#define XQ_P2P
//#define JPUSH
#define LDPUSH

#endif

#ifdef  BAFANGDIANZI
#define CUSTOMER_ID     61

/* add begin by yiqing, 2015-08-07, 原因: */
#define XQ_P2P

#define BAGGIO_API

#define GET_RECORDLIST_PAGE_BY_PAGE
//#undef ENABLE_AUDIO_PLAY
//#define UNABLE_AUDIO

#endif


///////////////////////////////////////////////////////////////////////////////

#define BOOL    int
#define TRUE    1
#define FALSE   0

///////////////////////////////////////////////////////////////////////////////

/* BEGIN: Added by wupm, 2013/6/14 */
#define LOG_SYSTEM_START    "System Starting..."
#define LOG_MOTION_ALARM    "Motion Alarm"
#define LOG_GPIO_ALARM      "GPIO Alarm"
#define LOG_FORMAT_SD       "Foramt SD"
#define LOG_SET_ALIAS       "Set Alias"
#define LOG_SET_DATE        "Set Datetime"
#define LOG_SET_NETWORK     "Set Network"
#define LOG_SET_WIFI        "Set Wifi"
#define LOG_REBOOT          "Reboot"
#define LOG_SET_EMAIL       "Set Email"
#define LOG_SET_FTP         "Set FTP"
#define LOG_SET_ALARM       "Set Alarm"
#define LOG_SET_USER        "Set User"
#define LOG_RESTORE         "Restore Parameters"
#define LOG_BACKUP          "Backup Parameters"
#define LOG_SET_FACTORY     "Set Factory"
#define LOG_DELETE_FILE     "Delete File"
#define LOG_UPGRADE_APP     "Upgrade App FW"
#define LOG_UPGRADE_SYS     "Upgrade Sys FW"
#define LOG_RESET_LOG       "Reset Log Content"

///////////////////////////////////////////////////////////////////////////////

/* BEGIN: Added by wupm, 2014/3/7 */
#define INSTEAD_RESET_TO_PUSHDOWN
/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/10 */
#define VERSION_RELEASE
#define PRESS_BUTTON    2
#define PRESS_PIR       1


/////////////////////////////////////////////////////////////////////////
/* BEGIN: Added by Baggio.wu, 2013/7/25 */
#define OutputDebugString(Fmt, args...) \
{   \
    printf("==== %-16s, line %4d, %-24s:"Fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##args); \
}

#define OutputDebugString2(Fmt, args...) \
{   \
    FILE* fp = NULL;    \
    char info[256]; \
    memset(info, 0, 256);   \
    sprintf(info, "=== %-16s, line %4d, %-24s: "Fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##args); \
    printf(info);   \
    fp = fopen( "/param/bell.log", "ab+" ); \
    if( fp != NULL )    \
    {   \
        fwrite( info, 1, strlen( info ), fp );  \
        fclose( fp );   \
    }   \
}


#define Textout(Fmt, args...)   OutputDebugString(Fmt, ##args)
#define Textout0(Fmt, args...)  //OutputDebugString(Fmt, ##args)
#define Textout1(Fmt, args...)  //OutputDebugString(Fmt, ##args)
#define Textout2(Fmt, args...)  //OutputDebugString(Fmt, ##args)
#define Textout3(Fmt, args...)  //OutputDebugString(Fmt, ##args)
#define Textout4(Fmt, args...)  //OutputDebugString(Fmt, ##args)
#define Textout5(Fmt, args...)  //OutputDebugString("==========="Fmt, ##args)

/* END:   Added by Baggio.wu, 2013/7/25 */

#endif
