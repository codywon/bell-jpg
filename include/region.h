#ifndef __REGION_H__
#define __REGION_H__

#include "hd_common.h"

#define	HD_RGN_COVER		0
#define HD_RGN_OVERLAY		1
#define HD_RGN_SOFTOVERLAY	2

typedef struct Hd_Region_S
{
    int			iType;
    int			bIsPublic;
    unsigned int	dwColor;
    unsigned int	dwLayer;
    unsigned int	dwBgAlpha;
    unsigned int	dwFgAlpha;
    int			iVencGroup;
    int			iX;
    int			iY;
    int			iWidth;
    int			iHeight;
    int			iDevID;
    int			iChnID;
    //REGION_CRTL_CODE_E 	enCtrl;
    //REGION_HANDLE	handle;
} HD_REGION_S, *PHD_REGION_S;

int HD_RegionOpen( PHD_REGION_S pstRegion );

int HD_RegionClose( PHD_REGION_S pstRegion );

#endif //__REGION_H__
