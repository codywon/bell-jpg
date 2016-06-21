#ifndef	_DOWNLOAD_H_
#define	_DOWNLOAD_H_

typedef	int Int32;
typedef unsigned int UInt32;
typedef unsigned short UInt16;
typedef char Int8;
typedef unsigned char UInt8;
typedef void Void;

#define MAX_TI_NET_PACKET_SIZE      (512*1024)
typedef Int32 (*TiNetCallback)(Int8* buffer, UInt32 len);

typedef struct
{
    UInt32 port;
    Int32  socket;
    TiNetCallback callback;
}TiNetHandler;

#define	REQUEST_SERVER_IP_ADDRESS	"115.29.4.30"
#define	REQUEST_SERVER_PORT			24201

#define	REQUEST_TIMEOUT				5

Int32 TiNetAddListener(UInt16 port, TiNetCallback callback);
Int32 TiNetUdpSend( char *server_ip, Int8* data, UInt32 len );

///////////////////////////////////////////////////////////////////////////
//
//	Application
//
///////////////////////////////////////////////////////////////////////////
#define	REQUEST_LOCAL_PORT			9632
#define	REQUEST_FREQUENCY			3600

#define	REQUEST_START_BYTE			0x0A593C14
typedef struct
{
	int nStartByte;
	int nMajorVersion;
	int nMinorVersion;
	int nCustomerID;
	char szID[24];	
}TRequest;

typedef struct
{
	int nStartByte;
	char szDownloadServer[32];
	int nDownloadPort;
	char szDownloadPath[128];
}TRequestACK, *pTRequestACK;

#define	DOP_KILL		1	//killall
#define	DOP_REPLACE		2	//replace current file
#define	DOP_REPLACEMOD	3	//replace current file, then chmod a+x
#define	DOP_MOVE		4	//rm -rf 
#define	DOP_MKDIR		5	//mkdir -p 
#define	DOP_BACKUP		6	//copy this file to /tmp/filename, then restore it
#define	DOP_RESTORE		7	//restore /tmp/filename to ...
#define	DOP_REBOOT		8	//reboot when success, must be LAST item

typedef struct
{
	int nOperator;
	char szPath[48];
	int nFileSize;	//if backup/restore, this is serailno(0-7), max=8
}TDownloadFileItem;

typedef struct
{
	int nStartByte;
	int nFileItemCount;
	int nTotalSize;
	int nReserved;
}TDownloadFileHead;

#endif

