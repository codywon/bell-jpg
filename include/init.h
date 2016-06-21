#ifndef __INIT_H__
#define __INIT_H__

#define MAX_CONN_USER_NUM 4
#define	ADMIN		255
#define VISTOR  	1
#define OPRATION 	2

typedef struct _stOsdInfo
{
    unsigned char		byOsdType;
    unsigned char		byFrameRate;
    unsigned short		nBitRate;

    unsigned char		szTitle[16];

    unsigned int		dwCurTime;

    unsigned short		nDateX;
    unsigned short		nDateY;

    unsigned short		nBitRateX;
    unsigned short		nBitRateY;

    unsigned short		nTitleX;
    unsigned short		nTitleY;

} OSD_INFO, *POSD_INFO;

#endif //__INIT_H__
