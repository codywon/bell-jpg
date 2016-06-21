#ifndef __LOG_H__
#define __LOG_H__

#define LOG_TYPE_VLOSS_ALARM	1
#define LOG_TYPE_SENSOR_ALARM	2
#define LOG_TYPE_MOTION_ALARM	3
#define LOG_TYPE_OPERATION	4

#include "protocol.h"

const char* WriteToLogFile(
    unsigned char 	byChanID,
    unsigned char 	byLogType,
    unsigned int 	dwOperIpAddr,
    unsigned short	nCommand,
    unsigned short	nStatus );

int QueryLogInfo( int		iSocket,
                  unsigned short	nSessionID,
                  PLOGQUERYREQUEST	pstRequest );

#endif //__LOG_H__
