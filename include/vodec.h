#ifndef __VO_DEC_H__
#define __VO_DEC_H__

#include "hd_common.h"

int HD_H264Dec_Open( int 	iDecChn,
                     int		iPicWidth,
                     int		iPicHeight,
                     int		iRefFrameNum,
                     int		iPriority );

int HD_H264Dec_Close( int iDecChn, HI_BOOL bForce );

int HD_H264Dec_SendStream( int iDecChn, const unsigned char* pszStream, int iStreamLen );

#endif //__VO_DEC_H__
