#ifndef __RECORD_H__
#define __RECORD_H__

#include "hd_common.h"
#include "sysdata.h"
#include "protocol.h"

#define	REC_TYPE_NONE		0
#define REC_TYPE_MAN		1
#define REC_TYPE_TIMER		2
#define REC_TYPE_VLOSS		3
#define REC_TYPE_MOTION		4
#define	REC_TYPE_SENSOR		5

#define MAX_REC_TIME		(30 * 60)
#define DEF_ALARM_REC_TIME	(10)


//index table's file structure
typedef struct _stRecBeginHead
{
    unsigned int	dwBeginTime;

} REC_BEGIN_HEAD, *PREC_BEGIN_HEAD;

typedef struct _stRecFileHead
{
    REC_BEGIN_HEAD	stManRecHead[4];

    REC_BEGIN_HEAD	stAutoRecHead[4];

} REC_FILE_HEAD, *PREC_FILE_HEAD;

typedef struct _stRecItemInfo
{
    unsigned char	byChanID;
    unsigned char	byRecType;
    unsigned short	nRecTime;

    unsigned int	dwBeginTime;

    unsigned int	dwFileIndex;

} REC_ITEM_INFO, *PREC_ITEM_INFO;

int HD_REC_CreateThread( PRECORDPARAM	pstRecParam,
                         PVIDEOPARAM	pstVideoParam,
                         int		iValidChanNum );

int HD_REC_DestoryThread();

int HD_REC_Open( unsigned char byChanID, unsigned char byRecType );
int HD_REC_Close( unsigned char byChanID );

int HD_REC_UpdateRecType( unsigned char byChanID, unsigned char byRecType );

//byType: 1/0: video/audio
int HD_REC_SendOneFrame( unsigned char byChanID, unsigned char byType, const char* pszBuffer, int iBufferSize );

int HD_REC_QueryRecFile(
    int				iSocket,
    unsigned short		nSessionID,
    PQUERYRECFILEREQUEST	pstRequest );

int HD_REC_StreamRequest(
    int				iSocket,
    unsigned short		nSessionID,
    PFILESTREAMREQUEST		pstRequest );

void UpdateRecordInfo( const char* pszStartTime, const char* pszEndTime );

#endif //__RECORD_H__
