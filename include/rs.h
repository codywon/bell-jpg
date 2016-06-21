#ifndef __SERICAL_PORT_H__
#define __SERICAL_PORT_H__

#define RS485_SEND_EN		24
#define RS485_SEND_EN_PORT 	4

int HD_RS232_Open();
int HD_RS232_Close();

int HD_RS485_Open();
int HD_RS485_Close();

int HD_RS232_SetParam( int nSpeed, int nBits, char nEvent, int nStop );

int HD_RS485_SetParam( int nSpeed, int nBits, char nEvent, int nStop );

int HD_RS232_SendData( const char* pszData, int iDataLen );
int HD_RS485_SendData( const char* pszData, int iDataLen );

#endif //__SERICAL_PORT_H__
