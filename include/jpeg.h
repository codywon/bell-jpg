#ifndef __JPEG_H__
#define __JPEG_H__

#include "hd_common.h"

int HD_JPEG_Open( int iEncChan );
int HD_JPEG_Close( int iEncChan );

int HD_JPEG_SaveJPEG( int iEncChan, const char* pszFileName );

#endif //__JPEG_H__
