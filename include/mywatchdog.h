#ifndef __MY_WATCHDOG_H__
#define __MY_WATCHDOG_H__

int OpenWatchdog( int iTimeOut );

int FeedWatchdog();

int CloseWatchdog();

#endif //__MY_WATCHDOG_H__
