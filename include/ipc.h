#ifndef __IPC_H__
#define __IPC_H__

int IPCSocketInit( void );
int DeviceSend( unsigned char* pbuf, unsigned int len, char type, int times );

int aiIPCSocketInit( void );
int aiDeviceSend( unsigned char* pbuf, unsigned int len );

int aoIPCSocketInit( void );
int aoIPCRcvData( unsigned char* pbuf );

#endif //__IPC_H__
