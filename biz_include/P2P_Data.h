
#ifndef _p2p_data_h
#define _p2p_data_h

#include "XQ_Global.h"

#define PPPP_MAX_SESSION_NUMBER 32
#define PPPP_MAX_CHANNEL_NUMBER 8


//***********20140814-start
typedef struct globalParams
{
UINT32 _maxNmbOfSession;
UINT32 _maxNmbOfChannel;
UINT32 _maxSizeOfChannel;
UINT32 _maxSizeOfPacket;
CHAR _devName[32];
}st_globalParams;



//network information
typedef struct PPPP_NetInfo
{
	CHAR bFlagInternet; 	// Internet Reachable? 1: YES, 0: NO
	CHAR bFlagHostResolved; // P2P Server IP resolved? 1: YES, 0: NO
	CHAR bFlagServerHello;	// P2P Server Hello? 1: YES, 0: NO
	CHAR NAT_Type;			// NAT type, 0: Unknow, 1: IP-Restricted Cone type,   2: Port-Restricted Cone type, 3: Symmetric
	CHAR MyLanIP[16];		// My LAN IP. If (bFlagInternet==0) || (bFlagHostResolved==0) || (bFlagServerHello==0), MyLanIP will be "0.0.0.0"
	CHAR MyWanIP[16];		// My Wan IP. If (bFlagInternet==0) || (bFlagHostResolved==0) || (bFlagServerHello==0), MyWanIP will be "0.0.0.0"
} st_PPPP_NetInfo;

//session information
typedef struct PPPP_Session
    {
    INT32  Skt; 				// Sockfd
    struct sockaddr_in RemoteAddr;	// Remote IP:Port
    struct sockaddr_in MyLocalAddr; // My Local IP:Port
    struct sockaddr_in MyWanAddr;	// My Wan IP:Port
    UINT32 ConnectTime; 			// Connection build in ? Sec Before
    CHAR DID[24];// Device ID
    CHAR bCorD; 					// I am Client or Device, 0: Client, 1: Device
    CHAR bMode;  
    CHAR Reserved[2];
    } st_PPPP_Session;

//p2p search
typedef struct lanSearchRet
{
CHAR mIP[16];
CHAR mDID[24];
}t_lanSearchRet;

//p2p search extend
typedef struct lanSearchExtRet
{
CHAR mIP[16];
CHAR mDID[24];
CHAR mName[32];
}st_lanSearchExtRet;

//channel full-information include packet detailed
typedef struct queueInfo
    {
    UINT16 cSN; //current (next for) packet serial number
    UINT16 fSN;// the first packet serial number in queue, which the front pointer points 
    UINT16 rSN;// the last packet serial number in queue, which the rear pointer points 
    UINT16 pSN;//serial number of the packet inserted into queue last time
    UINT32 pMaxSize;//very packet max size
    UINT32 pSumSize;//队列中实际节点数据长度之和  
    UINT32 qMaxSize; //队列最大节点个数
    UINT32 pNmb; //队列中实际节点个数
    }st_queueInfo;

//session information
typedef struct sessionInfo
    {
    CHAR Reserved[2];
    UINT16 chnlNmb;
    INT32 Status;
    st_PPPP_Session connInfo;
    st_queueInfo recvDataInfo[PPPP_MAX_CHANNEL_NUMBER];
    st_queueInfo sendDataInfo[PPPP_MAX_CHANNEL_NUMBER];
    }st_sessionInfo;

//nat type
typedef enum
	{
	OPEN_INTERNET=0,
	RESTRICTED_CONE_NAT,
	RESTRICTED_PORT_CONE_NAT,
	SYMMETRIC_NAT,
	FULL_CONE_NAT,
	FIREWALL_BLOCK_UDP,
	SYMMETRIC_UDP_FIREWALL,
	BLOCKED,
	NAT_UNKNOWN
	}t_nat_type;

//p2p session mode
typedef enum 
	{
	MODE_P2P=0,
	MODE_RELAY,
	MODE_UNKNOWN
	}t_pppp_mode;


#endif


