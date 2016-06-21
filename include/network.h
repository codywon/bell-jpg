#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "hd_common.h"

#define MAX_CONNECT_NUM		8
#define MAX_WEB_SOCKET       128

typedef void ( *cb_NotifyConnected )( int iSocket, const char* pszIpAddr, int iPort );
typedef void ( *cb_NotifyClose )( int iSocket );

typedef int ( *cb_RecvMessage )( int iSocket, const char* pszMessage, int iMessageLen );

typedef struct Hd_Client_S
{
    int			iSocket;

} HD_CLIENT, *PHD_CLIENT;

typedef struct _AcceptSock
{
    int					socket;
    short				len;
    short				talklen;
    unsigned int		refcnt;
    char				liveflag;
    char				audioflag;
    char				recordflag;
    char				recordfirstflag;
    unsigned int		recordlen;
    char				filename[64];
    char				buffer[1024];
    char				talkbuffer[1024];

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/12 */
	int					nIndex;	//UserIndex, 1-8, 101-108
	char				szUser[32];	//UserName

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/7/17 */
	int					state;		//PS_IDLE, PS_CALLING, PS_TALKING
	int					stateEx;	//PS_IDLE, PS_WATCHING

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/7/17 */
	char szCallID[64];	// = {0};
	char szWatchID[64];	// = {0};
	char szAlarmID[64];	// = {0};
} ACCEPTSOCK, *PACCEPTSOCK;

typedef struct _USERDATA
{
    char			other;
    char			gmindex;
    short		bufindex;
    unsigned int	offset;
} USERDATA, *PUSERDATA;

typedef struct Hd_Server_Thread_S
{
    unsigned char	byIsTcp;
    unsigned char	byIsStoped;
    unsigned short	nPort;

    int			iSocketNum;

    int			iSocket;

    int			iSocketCount;

    int			iSocketArray[MAX_CONNECT_NUM];

    unsigned short	nSessionIDArray[MAX_CONNECT_NUM];

    unsigned char	byHBCount[MAX_CONNECT_NUM];

    unsigned char	byIsLogined[MAX_CONNECT_NUM];

    struct sockaddr_in 	saddr[MAX_CONNECT_NUM];

    pthread_t		threadID;

    cb_NotifyConnected	NotifyConnected;
    cb_NotifyClose	NotifyClose;
    cb_RecvMessage	RecvMessage;

    char		szBuffer[MAX_CONNECT_NUM][4096];
    int			iRecvLength[MAX_CONNECT_NUM];

    char		firstflag[MAX_CONNECT_NUM];
    char		firstbuf[MAX_CONNECT_NUM][8];

} HD_SERVER_THREAD_S, *PHD_SERVER_THREAD_S;

typedef struct Hd_Client_Thread_S
{
    unsigned char	byIsTcp;
    unsigned char	byIsStoped;
    unsigned short	nPort;

    char		szIpAddr[20];

    int			iSocket;

    pthread_t		threadID;

    struct sockaddr_in 	saddr;

    cb_NotifyConnected	NotifyConnected;
    cb_NotifyClose	NotifyClose;
    cb_RecvMessage	RecvMessage;

} HD_CLIENT_THREAD_S, *PHD_CLIENT_THREAD_S;

int HD_NET_SVR_Open( PHD_SERVER_THREAD_S pstServerThread );

int HD_NET_SVR_Close( PHD_SERVER_THREAD_S pstServerThread );

int HD_NET_SVR_Send( PHD_SERVER_THREAD_S pstServerThread, int iSocket, const char* pszBuffer, int iBufLen );

int HD_NET_CLI_Open( PHD_CLIENT_THREAD_S pstClientThread );

int HD_NET_CLI_Close( PHD_CLIENT_THREAD_S pstClientThread );

int HD_NET_CLI_Send( PHD_CLIENT_THREAD_S pstClientThread, const char* pszBuffer, int iBufLen );

extern pthread_mutex_t socketmutex;
void socketlock( void );
void socketunlock( void );

extern pthread_mutex_t csocketmutex;
void csocketlock( void );
void csocketunlock( void );

int GetNetStat( void );
int GetNetStatEth( void );

extern ACCEPTSOCK              websock[MAX_WEB_SOCKET];

#endif //__NETWORK_H__

