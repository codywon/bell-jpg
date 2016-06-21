#ifndef __AO_DEC_H__
#define __AO_DEC_H__

#include "hd_common.h"

int HD_AODec_Open( int iADecChnID );

int HD_AODec_Close( int iADecChnID );

int HD_AODec_SendStream( int iAoDevID, int iAoChnID, int iADecChnID, const char* pszStream, int iStreamLen );

#endif //__AO_DEC_H__
