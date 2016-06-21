/******************************************************************************

  Copyright (C), 2011-2012, wupm(szwpm@Tom.com) Co., Ltd.

 ******************************************************************************
  File Name     : wpslib.h
  Version       : Initial Draft
  Author        : wupm
  Created       : 2014/3/11
  Last Modified :
  Description   : wpslib.c header file
  Function List :
  History       :
  1.Date        : 2014/3/11
    Author      : wupm
    Modification: Created file

******************************************************************************/

#ifndef __WPSLIB_H__
#define __WPSLIB_H__

#ifndef	BOOL
#define	BOOL 	int
#define	TRUE 	1
#define	FALSE	0
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

//*************************************************
//Return TRUE if Success
	//char szSSID[64]
	//char szShareKey[64]
	//int nAuthType 		0x02 = WPAPSK+AES
	//						0x03 = WPAPSK+TKIP
	//						0x04 = WPA2PSK+AES
	//						0x05 = WPA2PSK+TKIP
	//int nChannel
//*************************************************
extern BOOL GetWPSResult(char *pszSsid, char *pszKey, int *pnAuthType);

//*************************************************
//After StartWPS(),
//you Should Create a Timer to Call GetWPSResult whihin 2 Minutes
//if WPS Success, you should save parameters to 'system.ini', then Reboot
//*************************************************
extern void StartWPS();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __WPSLIB_H__ */
