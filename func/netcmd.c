#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>

#include "network.h"
#include "param.h"
#include "init.h"
#include "protocol.h"
#include "abuf.h"
#include "vbuf.h"
#include "mjpgbuf.h"
#include "cmdhead.h"
#include "debug.h"

#define MAX_CONNECT_SOCKET   64
//#define MAX_WEB_SOCKET       128

#define max(a, b) ((a)>(b)?(a):(b))

int GetSnapStream( unsigned char* pdst, char flag );
//livestream user
LIVEUSER                	streamuserj[MAX_CONN_USER_NUM];
//videostream user
LIVEUSER                	videouserj[MAX_CONN_USER_NUM];
//audio stream user
LIVEUSER                	audiouserj[MAX_CONN_USER_NUM];
LIVEUSER                	streamuserr[MAX_CONN_USER_NUM];

unsigned int		refcntlive = 0;
unsigned int		refcntvideo = 0;
unsigned int		refcntrecord = 0;
unsigned int		refcntaudio = 0;

pthread_mutex_t    	acceptmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t 		csocketmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t 		socketmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t         	capturemutext = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t         	streammutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t		alarmsocket0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t		alarmsocket1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t		alarmsocket2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t		alarmsocket3 = PTHREAD_MUTEX_INITIALIZER;

//jpeg user sem
/* BEGIN: Continue try to Resolve BUG of System Halt */
/*        Deleted by wupm(2073111@qq.com), 2014/10/23 */
sem_t			usersemj[MAX_CONN_USER_NUM];

unsigned char		usertimesj[MAX_CONN_USER_NUM];

/* BEGIN: Continue try to Resolve BUG of System Halt */
/*        Deleted by wupm(2073111@qq.com), 2014/10/23 */
sem_t			videosemj[MAX_CONN_USER_NUM];

unsigned char		videotimesj[MAX_CONN_USER_NUM];

//audio
/* BEGIN: Continue try to Resolve BUG of System Halt */
/*        Deleted by wupm(2073111@qq.com), 2014/10/23 */
sem_t			aisemsend[MAX_CONN_USER_NUM];

//video buffer
const unsigned char* pjpeg  = NULL;
const unsigned char* paudio = NULL;
const unsigned char* pvideo = NULL;
const unsigned char* psnap  = NULL;

short			webcnt = 0;
unsigned short	streamcnt = 0;
char				testflag = 0x01;

unsigned char		livebuf0[MAX_JPEG_BUFFER_SIZE];
unsigned char		livebuf1[MAX_JPEG_BUFFER_SIZE];
unsigned char		livebuf2[MAX_JPEG_BUFFER_SIZE];
unsigned char		livebuf3[MAX_JPEG_BUFFER_SIZE];
unsigned char		videobuf[1024 * 128];

void Acceptlock( void )
{
    pthread_mutex_lock( &acceptmutex );
}

void Acceptunlock( void )
{
    pthread_mutex_unlock( &acceptmutex );
}

void capturelock( void )
{
    pthread_mutex_lock( &capturemutext );
}

void captureunlock( void )
{
    pthread_mutex_unlock( &capturemutext );
}

void socketlock( void )
{
    pthread_mutex_lock( &socketmutex );
}

void socketunlock( void )
{
    pthread_mutex_unlock( &socketmutex );
}

void streamlock( void )
{
    pthread_mutex_lock( &streammutex );
}

void streamunlock( void )
{
    pthread_mutex_unlock( &streammutex );
}

void livelock( void )
{
    pthread_mutex_lock( &csocketmutex );
}

void liveunlock( void )
{
    pthread_mutex_unlock( &csocketmutex );
}

void alarm0lock( void )
{
    pthread_mutex_lock( &alarmsocket0 );
}

void alarm0unlock( void )
{
    pthread_mutex_unlock( &alarmsocket0 );
}
void alarm1lock( void )
{
    pthread_mutex_lock( &alarmsocket1 );
}

void alarm1unlock( void )
{
    pthread_mutex_unlock( &alarmsocket1 );
}

void alarm2lock( void )
{
    pthread_mutex_lock( &alarmsocket2 );
}

void alarm2unlock( void )
{
    pthread_mutex_unlock( &alarmsocket2 );
}
void alarm3lock( void )
{
    pthread_mutex_lock( &alarmsocket3 );
}

void alarm3unlock( void )
{
    pthread_mutex_unlock( &alarmsocket3 );
}

void live_dbg( char* pbuf, int cmd )
{
    FILE* fp = NULL;

    if ( cmd == 0x00 )
    {
        fp = fopen( "/tmp/live.txt", "ab+" );

        if ( fp == NULL )
        {
            printf( "live dbg file open failed\n" );
            return;
        }
    }

    else
    {
        fp = fopen( "/tmp/live.txt", "wb" );

        if ( fp == NULL )
        {
            printf( "live dbg file open failed\n" );
            return;
        }
    }

    fwrite( pbuf, 1, strlen( pbuf ), fp );
    fclose( fp );
}

int PushLiveSocketj( int socket, int streamid, int audio, int http )
{
    int    i;
    int     nFlag    = 1;
    char    flag     = 0x00;
    int     iBufSize = 0;
    int		nIndex = -1;
    livelock();

    //TEST
    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        //Textout( "nIndex = %d, socket = %d, RefCount = %d", i, streamuserj[i].socket, streamuserj[i].refcnt );

        if ( streamuserj[i].socket == socket )
        {
            nIndex = i;
        }
    }

//find null scoket
    if ( nIndex == -1 )
    {
        for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
        {
            if ( streamuserj[i].socket == -1 )
            {
                flag = 0x01;
                nIndex = i;
                break;
            }
        }
    }

//find,shutdown socket
    //if( flag == 0x00 )
    if ( nIndex == -1 )
    {
#if 1
        unsigned int refback = refcntlive - MAX_CONN_USER_NUM;

        for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
        {
            printf( "i %d refback %d refcnt %d socket %d\n", i, refback, streamuserj[i].refcnt, streamuserj[i].socket );

            if ( streamuserj[i].refcnt <= refback )
            {
                printf( "refback close socket sit %d socket %d\n", i, streamuserj[i].socket );
                socketlock();
                //BOOL bDontLinger = FALSE;
                //setsockopt(streamuserj[i].socket,SOL_SOCKET,SO_DONTLINGER,(const char*)&bDontLinger,sizeof(BOOL));
                iBufSize = 8192;
                setsockopt( socket, SOL_SOCKET, SO_RCVBUF, ( void* )&iBufSize, sizeof( int ) );
                CloseSocket( streamuserj[i].socket );
                streamuserj[i].socket = -1;
                streamuserj[i].refcnt  = -1;
                streamuserj[i].streamid = -1;
                streamuserj[i].http = -1;
                streamuserj[i].head = -1;
                streamuserj[i].talk = 0;
                InitUserIndicator( streamid, i );
                socketunlock();
                usleep( 500000 );
                //sleep(2);
                printf( "close socket ok\n" );
                nIndex = i;
                break;
            }
        }

#else
        //CloseSocket(socket);
        liveunlock();
        return 2;
#endif
    }

//add socket

    //for( i = 0; i < MAX_CONN_USER_NUM; i++ )
    if ( nIndex >= 0 )
    {
        i = nIndex;
        //if( streamuserj[i].socket == -1 )
        {
            //struct timeval timeout = {30,0};
            printf( "push socket %d sit %d\n", socket, i );
            //setsockopt(socket,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(struct timeval));
            setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, ( char* )&nFlag, sizeof( int ) );
            //iBufSize = 1024 * 128;
            //setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (void *)&iBufSize, sizeof(int));
            //setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (void *)&iBufSize, sizeof(int));
            socketlock();
            InitUserIndicator( streamid, i );
            SetUserIndicator( streamid, i );
            streamuserj[i].refcnt  = refcntlive;
            streamuserj[i].streamid = streamid;
            streamuserj[i].http = http;
            streamuserj[i].head = 0x00;
            streamuserj[i].audio = audio;
            streamuserj[i].socket = socket;
            streamuserj[i].talklen = 0x00;
            socketunlock();
        }
    }

    refcntlive++;
    liveunlock();

    return 0;
}

int StreamLivePushSocket( int socket, int streamid, int audio)
{
    PushLiveSocketj(socket, streamid, audio, 0);
}

//record stream socket
int PushRecordSocket( int socket, int streamid, int audio, int http, char* filename, unsigned int offset )
{
    int    i;
    int     nFlag    = 1;
    char    flag     = 0x00;
    int     iBufSize = 0;
    livelock();

//find null scoket
    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        if ( streamuserr[i].socket == -1 )
        {
            flag = 0x01;
            break;
        }
    }

//find,shutdown socket
    if ( flag == 0x00 )
    {
#if 1
        unsigned int refback = refcntrecord - MAX_CONN_USER_NUM;

        for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
        {
            printf( "i %d refback %d refcnt %d socket %d\n", i, refback, streamuserr[i].refcnt, streamuserr[i].socket );

            if ( streamuserr[i].refcnt <= refback )
            {
                //printf("refback close socket sit %d socket %d\n",i,streamuserr[i].socket);
                socketlock();
                CloseSocket( streamuserr[i].socket );
                streamuserr[i].socket = -1;
                streamuserr[i].refcnt  = -1;
                streamuserr[i].streamid = -1;
                streamuserr[i].http = -1;
                streamuserr[i].head = -1;
                streamuserr[i].talk = 0;
                InitUserIndicator( streamid, i );
                socketunlock();
                break;
            }
        }

#else
        //CloseSocket(socket);
        liveunlock();
        return 2;
#endif
    }

//add socket
    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        if ( streamuserr[i].socket == -1 )
        {
            //struct timeval timeout = {30,0};
            printf( "push socket %d sit %d\n", socket, i );
            //setsockopt(socket,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(struct timeval));
            setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, ( char* )&nFlag, sizeof( int ) );
            iBufSize = 16384;
            setsockopt( socket, SOL_SOCKET, SO_RCVBUF, ( void* )&iBufSize, sizeof( int ) );
            setsockopt( socket, SOL_SOCKET, SO_SNDBUF, ( void* )&iBufSize, sizeof( int ) );
            socketlock();
            InitUserIndicator( streamid, i );
            //SetRecordIndicator( streamid, i, filename, offset );
            streamuserr[i].refcnt  = refcntrecord;
            streamuserr[i].streamid = streamid;
            streamuserr[i].http = http;
            streamuserr[i].head = 0x00;
            streamuserr[i].audio = audio;
            streamuserr[i].socket = socket;
            streamuserr[i].talklen = 0x00;
            socketunlock();
            break;
        }
    }

    refcntrecord++;
    liveunlock();
    return 0;
}

//jpeg stream socket

void PushVideoSocketj( int socket, int streamid, int audio, int http )
{
    int    i;
    int     nFlag    = 1;
    char    flag     = 0x00;
    int     iBufSize = 0;
    streamlock();

//find null scoket
    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        if ( videouserj[i].socket == -1 )
        {
            flag = 0x01;
            break;
        }
    }

//find,shutdown socket
    if ( flag == 0x00 )
    {
        unsigned int refback = refcntvideo - MAX_CONN_USER_NUM;

        for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
        {
            printf( "video i %d refback %d refcnt %d socket %d\n", i, refback, videouserj[i].refcnt, videouserj[i].socket );

            if ( videouserj[i].refcnt <= refback )
            {
                //printf("refback close socket sit %d socket %d\n",i,videouserj[i].socket);
                socketlock();
                SendLiveResult( socket, 200 );
                CloseSocket( videouserj[i].socket );
                videouserj[i].socket = -1;
                videouserj[i].refcnt  = -1;
                videouserj[i].streamid = -1;
                videouserj[i].http = -1;
                videouserj[i].head = -1;
                videouserj[i].talk = 0;
                socketunlock();
                usleep( 500000 );
                break;
            }
        }
    }

//add socket
    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        if ( videouserj[i].socket == -1 )
        {
            //struct timeval timeout = {30,0};
            printf( "video push socket %d sit %d\n", socket, i );
            //setsockopt(socket,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(struct timeval));
            setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, ( char* )&nFlag, sizeof( int ) );
            // iBufSize = 1024 * 4;
            //setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (void *)&iBufSize, sizeof(int));
            //setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (void *)&iBufSize, sizeof(int));
            socketlock();
            videouserj[i].refcnt  = refcntvideo;
            videouserj[i].streamid = streamid;
            videouserj[i].http = http;
            videouserj[i].head = 0x00;
            videouserj[i].audio = audio;
            videouserj[i].socket = socket;
            videouserj[i].talklen = 0x00;
            userindj[i].offset = encbufj.prelen;
            socketunlock();
            break;
        }
    }

    refcntvideo++;
    streamunlock();
}

//audio stream socket
void PushAudioSocketj( int socket, int streamid, int audio, int http )
{
    int    i;
    int     nFlag    = 1;
    char    flag     = 0x00;
    int     iBufSize = 0;
    streamlock();

//find null scoket
    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        if ( audiouserj[i].socket == -1 )
        {
            flag = 0x01;
            break;
        }
    }

//find,shutdown socket
    if ( flag == 0x00 )
    {
        unsigned int refback = refcntaudio - MAX_CONN_USER_NUM;

        for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
        {
            printf( "audio i %d refback %d refcnt %d socket %d\n", i, refback, audiouserj[i].refcnt, audiouserj[i].socket );

            if ( audiouserj[i].refcnt <= refback )
            {
                //printf("refback close socket sit %d socket %d\n",i,audiouserj[i].socket);
                socketlock();
                SendLiveResult( socket, 200 );
                CloseSocket( audiouserj[i].socket );
                audiouserj[i].socket = -1;
                audiouserj[i].refcnt  = -1;
                audiouserj[i].streamid = -1;
                audiouserj[i].http = -1;
                audiouserj[i].head = -1;
                audiouserj[i].talk = 0;
                socketunlock();
                break;
            }
        }
    }

//add socket
    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        if ( audiouserj[i].socket == -1 )
        {
            //struct timeval timeout = {30,0};
            printf( "audio push socket %d sit %d\n", socket, i );
            //setsockopt(socket,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(struct timeval));
            setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, ( char* )&nFlag, sizeof( int ) );
            iBufSize = 8192;
            setsockopt( socket, SOL_SOCKET, SO_RCVBUF, ( void* )&iBufSize, sizeof( int ) );
            setsockopt( socket, SOL_SOCKET, SO_SNDBUF, ( void* )&iBufSize, sizeof( int ) );
            socketlock();
            audiouserj[i].refcnt  = refcntaudio;
            audiouserj[i].streamid = streamid;
            audiouserj[i].http = http;
            audiouserj[i].head = 0x00;
            audiouserj[i].audio = audio;
            audiouserj[i].socket = socket;
            audiouserj[i].talklen = 0x00;
            socketunlock();
            break;
        }
    }

    refcntaudio++;
    streamunlock();
}
//jpeg stream
void Checksnapshotcgi( char* pbuf, int socket, char authflag )
{
    int 	iRet;
    char 	usertemp[32];
    char 	pwdtemp[32];
    char	flag = 0x00;
    struct timeval timeout = {30, 0};
    char       temp2[128];
    char       decoderbuf[128];
#ifdef	SCC_OEM
	/* BEGIN: Modified by wupm, 2013/2/25 */
	if (1)
	{
		int z; // Status code
		int iKeepAlive = 1;
		z = setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (void*)&iKeepAlive, sizeof(iKeepAlive));

		if (z)
		{
			Textout("SO_KEEPALIVE Error, errno = %d", errno);
		}
	}

	if (1)
	{
#define TRUE     1
#define FALSE    0
		int z; // Status code
		struct linger so_linger;
		so_linger.l_onoff = TRUE;
		so_linger.l_linger = 0;
		z = setsockopt(socket,
					   SOL_SOCKET,
					   SO_LINGER,
					   &so_linger,
					   sizeof so_linger);

		if (z)
		{
			Textout("SO_LINGER Error, errno = %d", errno);
		}
	}
#endif
	/* END:   Modified by wupm, 2013/2/25 */
    setsockopt( socket, SOL_SOCKET, SO_SNDTIMEO, ( char* )&timeout, sizeof( struct timeval ) );
    setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout, sizeof( struct timeval ) );
    wifiok = 1;

    if ( updateflag )
    {
        CloseSocket( socket );
        return -1;
    }

    if ( encryptflag )
    {
        CloseSocket( socket );
        return -1;
    }

    if ( authflag == 0x00 )
    {
        memset( usertemp, 0x00, 32 );
        memset( pwdtemp, 0x00, 32 );
        memset( temp2, 0x00, 128 );
        memset( decoderbuf, 0x00, 128 );
        iRet = GetStrParamValue( pbuf, "user", temp2, 31 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
            memset( usertemp, 0x00, 32 );
            strcpy( usertemp, decoderbuf );
        }

        memset( temp2, 0x00, 128 );
        memset( decoderbuf, 0x00, 128 );
        iRet += GetStrParamValue( pbuf, "pwd", temp2, 31 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
            memset( pwdtemp, 0x00, 32 );
            strcpy( pwdtemp, decoderbuf );
        }

        if ( iRet )
        {
            memset( temp2, 0x00, 128 );
            memset( decoderbuf, 0x00, 128 );
            memset( usertemp, 0x00, 32 );
            memset( pwdtemp, 0x00, 32 );
            iRet = GetStrParamValue( pbuf, "loginuse", temp2, 31 );

            if ( iRet == 0x00 )
            {
                memset( decoderbuf, 0x00, 128 );
                URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
                memset( usertemp, 0x00, 32 );
                strcpy( usertemp, decoderbuf );
            }

            memset( temp2, 0x00, 128 );
            memset( decoderbuf, 0x00, 128 );
            iRet += GetStrParamValue( pbuf, "loginpas", temp2, 31 );

            if ( iRet == 0x00 )
            {
                memset( decoderbuf, 0x00, 128 );
                URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
                memset( pwdtemp, 0x00, 32 );
                strcpy( pwdtemp, decoderbuf );
            }
        }

        if ( iRet == 0x00 )
        {
            iRet = GetUserPri( usertemp, pwdtemp );

            if ( iRet > 0x00 )
            {
                flag = 0x01;
            }
        }
    }

    else
    {
        flag = 0x01;
    }

    if ( flag == 0x01 )
    {
        iRet = GetSnapStream( psnap, 0x01 );

        if ( iRet > 0 )
        {
            int iBufSize = 8192;
            int     nFlag    = 1;
            setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, ( char* )&nFlag, sizeof( int ) );
            //setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (void *)&iBufSize, sizeof(int));
            //setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (void *)&iBufSize, sizeof(int));
            //printf("send snap %d\n",iRet);
            SendSnapData( socket, psnap, iRet );
        }
    }

    CloseSocket( socket );
    usleep( 10000 );
}
//200->用户名及密码错误
//201->用户数己满
int SendLiveResult( int socket, int value )
{
    LIVEHEAD                phead;
    memset( &phead, 0x00, sizeof( LIVEHEAD ) );
    phead.type = value;
    SendLiveData( socket, &phead, sizeof( LIVEHEAD ) );
}
#if 1
int real_set_block_fd( int fd )
{
    int flags;
    flags = fcntl( fd, F_GETFL );

    if ( flags == -1 )
    {
        return -1;
    }

    flags &= ~O_NONBLOCK;
    flags = fcntl( fd, F_SETFL, flags );
    printf( "flags %x\n", flags );
    return flags;
}
#endif
//check live stream cgi
int Checklivestreamcgi( char* pbuf, int socket, char authflag )
{
    int 	iRet = 0;
    char 	usertemp[32];
    char 	pwdtemp[32];
    int		streamid = 0;
    int		audio = 0;
    int    	result = 200;
    char       temp2[128];
    char       decoderbuf[128];
    
    /* BEGIN: KONX, Close PIR-LED when need, Set Initial-value, Adjust IR-LED */
    /*        Added by wupm, 2014/12/6 */
#if defined (ZHENGSHOW) || defined (KONX)  || defined (BELINK)

    Textout("IE Video Call....Enable IR(11)");
    EnableIR(11);
#endif
    

#if 1

    struct timeval timeout = {30, 0};

    Textout( "=====check live stream=================[%s], socket = %d, authflag = %d", pbuf, socket, authflag );
    //real_set_block_fd(socket);
    wifiok = 1;
    setsockopt( socket, SOL_SOCKET, SO_SNDTIMEO, ( char* )&timeout, sizeof( struct timeval ) );
    setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout, sizeof( struct timeval ) );

    //live_dbg("check livestream",0);
    if ( updateflag )
    {
        printf( "=====check live stream====1\n" );
        CloseSocket( socket );
        printf( "=====check live stream====2\n" );
        return -1;
    }

#else
    CloseSocket( socket );
    return -1;
#endif

#if	0
/////////////////////////

#if 1
    memset( usertemp, 0x00, 32 );
    memset( pwdtemp, 0x00, 32 );
    memset( temp2, 0x00, 128 );
    memset( decoderbuf, 0x00, 128 );
    iRet = GetStrParamValue( pbuf, "user", temp2, 31 );
    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( usertemp, 0x00, 32 );
        strcpy( usertemp, decoderbuf );
    }

    memset( temp2, 0x00, 128 );
    memset( decoderbuf, 0x00, 128 );
    iRet += GetStrParamValue( pbuf, "pwd", temp2, 31 );
    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( pwdtemp, 0x00, 32 );
        strcpy( pwdtemp, decoderbuf );
    }

    Textout("user=%s, pwd=%s, iRet=%d", usertemp, pwdtemp, iRet);

    if ( iRet )
    {
        memset( temp2, 0x00, 128 );
        memset( decoderbuf, 0x00, 128 );
        iRet = GetStrParamValue( pbuf, "loginuse", temp2, 31 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
            memset( usertemp, 0x00, 32 );
            strcpy( usertemp, decoderbuf );
        }

        memset( temp2, 0x00, 128 );
        memset( decoderbuf, 0x00, 128 );
        iRet += GetStrParamValue( pbuf, "loginpas", temp2, 31 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
            memset( pwdtemp, 0x00, 32 );
            strcpy( pwdtemp, decoderbuf );
        }
    }

#else
    memset( usertemp, 0x00, 32 );
    memset( pwdtemp, 0x00, 32 );
    iRet = GetStrParamValue( pbuf, "user", usertemp, 31 );
    memset( temp2, 0x00, 128 );
    memset( decoderbuf, 0x00, 128 );
    iRet += GetStrParamValue( pbuf, "pwd", pwdtemp, 31 );

    if ( iRet )
    {
        memset( temp2, 0x00, 128 );
        memset( decoderbuf, 0x00, 128 );
        iRet = GetStrParamValue( pbuf, "loginuse", usertemp, 31 );
        iRet += GetStrParamValue( pbuf, "loginpas", pwdtemp, 31 );
    }

#endif

//////////////////////
#endif


    iRet += GetIntParamValue( pbuf, "streamid", &streamid );
    Textout( "live find user pwd=%d encrypt=%d, StreamID = %d", iRet, encryptflag, streamid );
    if ( ( streamid < 0 ) || ( streamid > 6 ) )
    {
        //printf("streamid error=%d\n",streamid);
    }

#if 1

    if ( ( iRet == 0x00 ) && ( encryptflag == 0x00 ) )
    {
#else

    if ( iRet == 0x00 )
    {
#endif
		/*
        iRet = GetUserPri( usertemp, pwdtemp );
        Textout("check User pri, iRet=%d", iRet);
        if ( iRet > 0x00 )
        */
        if ( 1 )
        {
            iRet = GetIntParamValue( pbuf, "audio", &audio );
            if ( iRet )
            {
                audio = 0x00;
            }

            Textout( "audio %d streamid %d\n", audio, streamid );

            switch ( streamid )
            {
                case 0x00:
                    iRet = -1;
                    result = 202;
                    break;

                case 0x01:
                    iRet = -1;
                    result = 202;
                    break;

                case 0x02:
                    iRet = -1;
                    break;

                case 0x03:
                case 0x0a:
                    if ( 1)
                    {
						int i=0;
                        iRet = PushLiveSocketj( socket, streamid, audio, 0x00 );

						Textout("PushLiveSocketj Return %d", iRet);
						for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
					    {
					        Textout1( "nIndex = %d, socket = %d, RefCount = %d", i, streamuserj[i].socket, streamuserj[i].refcnt );
					    }

                        if ( iRet )
                        {
                            result = 201;
                            iRet = -1;
                        }
                    }

                    else
                    {
                        result = 203;
                        iRet = -1;
                    }

                    break;

                case 0x04:
                {
                    char filename[64];
                    unsigned int offset = 0;
                    memset( filename, 0x00, 64 );
                    iRet = GetStrParamValue( pbuf, "filename", filename, 63 );
                    iRet += GetIntParamValue( pbuf, "offset", &offset );

                    if ( iRet == 0x00 )
                    {
                        printf( "filename %s offset %d\n", filename, offset );
                        iRet = PushRecordSocket( socket, streamid, audio, 0x00, filename, offset );
                    }

                    if ( iRet )
                    {
                        result = LIVE_MAX_CONNECT;
                        iRet = -1;
                    }
                }
                break;

                default:
                    iRet = -1;
                    result = 202;
                    break;
            }
        }

        else
        {
            iRet = -1;
            result = 200;
        }
    }

    else 	//
    {
        iRet = -1;
    }

#if 1

    if ( iRet == -1 )
    {
        if ( encryptflag )
        {
            result = 204;
        }

        //printf( "close socket result %d\n", result );
        //printf("=====check live stream====3\n");
        SendLiveResult( socket, result );
        CloseSocket( socket );
    }

#endif
    //printf( "=====check live stream end====\n" );
    //live_dbg("check livestream end",0);
    return iRet;
}

//videostream
int Checkvideostreamcgi( char* pbuf, int socket, char authflag )
{
    int     iRet = 0;
    int     status = -1;
    char    usertemp[32];
    char    pwdtemp[32];
    char       temp2[128];
    char       decoderbuf[128];
    struct timeval timeout = {30, 0};
    wifiok = 1;
    live_dbg( "check videostream", 0 );
    setsockopt( socket, SOL_SOCKET, SO_SNDTIMEO, ( char* )&timeout, sizeof( struct timeval ) );
    setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout, sizeof( struct timeval ) );

    if ( updateflag )
    {
        CloseSocket( socket );
        return -1;
    }

    if ( encryptflag )
    {
        CloseSocket( socket );
        return -1;
    }

    printf( "check video stream %d pbuf:%s\n", authflag, pbuf );

    if ( authflag == 0x00 )
    {
        memset( usertemp, 0x00, 32 );
        memset( pwdtemp, 0x00, 32 );
        memset( temp2, 0x00, 128 );
        memset( decoderbuf, 0x00, 128 );
        iRet = GetStrParamValue( pbuf, "user", temp2, 31 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
            memset( usertemp, 0x00, 32 );
            strcpy( usertemp, decoderbuf );
            printf( "usertemp1:%s\n", usertemp );
        }

        memset( temp2, 0x00, 128 );
        memset( decoderbuf, 0x00, 128 );
        iRet += GetStrParamValue( pbuf, "pwd", temp2, 31 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
            memset( pwdtemp, 0x00, 32 );
            strcpy( pwdtemp, decoderbuf );
        }

        if ( iRet )
        {
            memset( usertemp, 0x00, 32 );
            memset( pwdtemp, 0x00, 32 );
            memset( temp2, 0x00, 128 );
            memset( decoderbuf, 0x00, 128 );
            iRet = GetStrParamValue( pbuf, "loginuse", temp2, 31 );

            if ( iRet == 0x00 )
            {
                memset( decoderbuf, 0x00, 128 );
                URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
                memset( usertemp, 0x00, 32 );
                strcpy( usertemp, decoderbuf );
            }

            memset( temp2, 0x00, 128 );
            memset( decoderbuf, 0x00, 128 );
            iRet += GetStrParamValue( pbuf, "loginpas", temp2, 31 );

            if ( iRet == 0x00 )
            {
                memset( decoderbuf, 0x00, 128 );
                URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
                memset( pwdtemp, 0x00, 32 );
                strcpy( pwdtemp, decoderbuf );
            }
        }

#if 1

        if ( ( iRet == 0x00 ) && ( encryptflag == 0x00 ) )
        {
#else

        if ( iRet == 0x00 )
        {
#endif
            iRet = GetUserPri( usertemp, pwdtemp );
            printf( "check video stream iRet %d user:%s pwd:%s\n", iRet, usertemp, pwdtemp );

            if ( iRet > 0x00 )
            {
                PushVideoSocketj( socket, 0x03, 0x00, 0x01 );
            }

            else
            {
                CloseSocket( socket );
            }
        }

        else
        {
            CloseSocket( socket );
        }
    }

    else
    {
        //printf("push video socket\n");
        status = 0x00;
        PushVideoSocketj( socket, 0x03, 0x00, 0x01 );
    }

    live_dbg( "check videostream end", 0 );
    return status;
}

//audiostream
int Checkaudiostreamcgi( char* pbuf, int socket, char authflag )
{
    int     iRet = 0;
    int     status = -1;
    char    usertemp[32];
    char    pwdtemp[32];
    char       decoderbuf[128];
    char		temp2[128];
    struct timeval timeout = {30, 0};
    wifiok = 1;
    Textout( "check audiostream---===================-------------------");
    SetAudioGongFang(TRUE);	
    setsockopt( socket, SOL_SOCKET, SO_SNDTIMEO, ( char* )&timeout, sizeof( struct timeval ) );
    setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout, sizeof( struct timeval ) );

    if ( updateflag )
    {
        CloseSocket( socket );
        return -1;
    }

    if ( encryptflag )
    {
        CloseSocket( socket );
        return -1;
    }

	///////////////////////////////
	#if	0
	
    printf( "check audio stream %d pbuf:%s\n", authflag, pbuf );
    memset( usertemp, 0x00, 32 );
    memset( pwdtemp, 0x00, 32 );
    memset( temp2, 0x00, 128 );
    memset( decoderbuf, 0x00, 128 );
    iRet = GetStrParamValue( pbuf, "user", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( usertemp, 0x00, 32 );
        strcpy( usertemp, decoderbuf );
        printf( "usertemp1:%s\n", usertemp );
    }

    memset( temp2, 0x00, 128 );
    memset( decoderbuf, 0x00, 128 );
    iRet += GetStrParamValue( pbuf, "pwd", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( pwdtemp, 0x00, 32 );
        strcpy( pwdtemp, decoderbuf );
    }

    if ( iRet )
    {
        memset( usertemp, 0x00, 32 );
        memset( pwdtemp, 0x00, 32 );
        memset( temp2, 0x00, 128 );
        memset( decoderbuf, 0x00, 128 );
        iRet = GetStrParamValue( pbuf, "loginuse", temp2, 31 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
            memset( usertemp, 0x00, 32 );
            strcpy( usertemp, decoderbuf );
        }

        memset( temp2, 0x00, 128 );
        memset( decoderbuf, 0x00, 128 );
        iRet += GetStrParamValue( pbuf, "loginpas", temp2, 31 );

        if ( iRet == 0x00 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
            memset( pwdtemp, 0x00, 32 );
            strcpy( pwdtemp, decoderbuf );
        }
    }

	////////////////////////////
	#endif

    if ( ( iRet == 0x00 ) && ( encryptflag == 0x00 ) )
    {
    	/*
        iRet = GetUserPri( usertemp, pwdtemp );
        printf( "check audio stream iRet %d user:%s pwd:%s\n", iRet, usertemp, pwdtemp );

        if ( iRet > 0x00 )
        */
        if ( 1 )
        {
            PushAudioSocketj( socket, 0x01, 0x01, 0x00 );
        }

        else
        {
            CloseSocket( socket );
        }
    }

    else
    {
        CloseSocket( socket );
    }

    live_dbg( "check audiostream end", 0 );
    return status;
}

int FindHttpEnd( unsigned char* pbuf, int len )
{
    int iRet = -1;
    int i;
    unsigned char* pdst = pbuf;

    for ( i = 0; i < len + 4; i++ )
    {
        if ( ( pdst[i] == 0x0d ) && ( pdst[i + 1] == 0x0a ) && ( pdst[i + 2] == 0x0d ) && ( pdst[i + 3] == 0x0a ) )
        {
            iRet = 0x00;
            break;
        }
    }

    return iRet;
}

void* SocketProc( void* p )
{
    int 		newfd   = -1;
    int             iBufLen = 0;
    char            nFlag 	= 1;
    fd_set          fdSocket1;
    int             maxsock;
    char		flag;
    char		i;
    int             iRet = -1;
    printf( "Socket proc is start pid=%d\n", getpid() );
    maxsock = 0;

    while ( 1 )
    {
        sleep( 1 );
    }

    return NULL;
}

int NetCmdSocketInit( void )
{
    int                     iRet = -1;
    pthread_t               threadnet;
    pthread_t               threadaccept;

    if ( pthread_create( &threadnet, NULL, & SocketProc, NULL ) )
    {
        printf( "network createthread failed\n" );
        return iRet;
    }
	pthread_detach(threadnet);
}

void StreamUserInit( void )
{
    char i;
    memset( &streamuserj, 0x00, sizeof( LIVEUSER )*MAX_CONN_USER_NUM );
    memset( &videouserj, 0x00, sizeof( LIVEUSER )*MAX_CONN_USER_NUM );
    memset( &audiouserj, 0x00, sizeof( LIVEUSER )*MAX_CONN_USER_NUM );
    memset( &streamuserr, 0x00, sizeof( LIVEUSER )*MAX_CONN_USER_NUM );

    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        streamuserj[i].socket = -1;
        videouserj[i].socket = -1;
        audiouserj[i].socket = -1;
        streamuserr[i].socket = -1;
    }
}

void myWriteH264( const unsigned char* pbuf, int len )
{
    FILE* fp = fopen( "/tmp/h264.dat", "ab+" );

    if ( fp )
    {
        fwrite( pbuf, 1, len, fp );
        fclose( fp );
    }
}
//==========================jpeg rate send======================
int SendSnapData( int socket, unsigned char* pbuf, unsigned int iBufLen )
{
    int             iRet = 0;
    int             len = 0x00;
    int             sendlen = 0;
    char*            pdst = NULL;

    if ( socket == -1 )
    {
        //netunlock();
        printf( "send jpeg end1\n" );
        return -1;
    }

    //printf("send0 %d\n",iBufLen);
    while ( len < iBufLen )
    {
#if 1
        iRet = send( socket, pbuf + len, iBufLen - len, 0 );

        if ( iRet == -1 )
        {
            printf( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //printf("socket %d send %d total %d\n",socket,len,iBufLen);
        }

#else

        if ( ( iBufLen - len ) >= 1024 )
        {
            sendlen = 1024;
        }

        else
        {
            sendlen = iBufLen - len;
        }

        pdst = pbuf + len;
        iRet = send( socket, pdst, sendlen, 0 );

        if ( iRet == -1 )
        {
            printf( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //printf("socket %d send %d total %d\n",socket,iRet,iBufLen);
        }

#endif
    }

    //printf("send0 end\n");
    if ( len == -1 )
    {
        return -1;
    }

    return 0;
}

//note user
/* BEGIN: Continue try to Resolve BUG of System Halt */
/*        Deleted by wupm(2073111@qq.com), 2014/10/23 */

void NoteJpegUser( void )
{
    char i;

    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        sem_post( &usersemj[i] );
    }
}

int GetVideoStream( unsigned char* pdst, unsigned int videoindex )
{
    int                     iRet;
    unsigned int            len     = 0;
    unsigned char*           pbuf   = NULL;
    LIVEHEAD                phead;
    int			offset;
    int			weblen = 0;
    unsigned short		streamcnt = 0;
    //get prelen
    mjpglock();
    offset = encbufj.prelen;
    //read live head
    memcpy( &phead, encbufj.pbuf + offset, sizeof( LIVEHEAD ) );
    //read buffer or need copy buffer?
    pbuf = encbufj.pbuf + offset + sizeof( LIVEHEAD );
    len = phead.len;
    mjpgunlock();

    if ( len >= 1024 * 128 )
    {
        printf( "jpeg len %d keylen %d\n", len, encbufj.prelen );
        return 0;
    }

    weblen += sprintf( pdst + weblen, "\r\n--ipcam264\r\n" );
    weblen += sprintf( pdst + weblen, "Content-Type:image/jpeg\n" );
    weblen += sprintf( pdst + weblen, "Content-Length:%d\n\n", len );
    memcpy( pdst + weblen, pbuf, len );
    //printf("videoindex %d len %d\n",videoindex,len);
    //printf("frameno:%d\n",phead.frameno);
    return len + weblen;
}

int GetSnapStream( unsigned char* pdst, char flag )
{
    int                     iRet;
    unsigned int            len     = 0;
    unsigned char*           pbuf   = NULL;
    LIVEHEAD                phead;
    int                     offset;
    int                     weblen = 0;

    mjpglock();
    offset = encbufj.prelen;
    //read live head
    memcpy( &phead, encbufj.pbuf + offset, sizeof( LIVEHEAD ) );
    //read buffer or need copy buffer?
    pbuf = encbufj.pbuf + offset + sizeof( LIVEHEAD );
    len = phead.len;

    if ( len >= MAX_JPEG_BUFFER_SIZE)
    {
        printf( "jpeg len %d keylen %d\n", len, encbufj.prelen );
        mjpgunlock();
        return 0;
    }

    if ( flag == 0x01 )
    {
        weblen += sprintf( pdst + weblen, "HTTP/1.1 200 OK\r\n" );
        weblen += sprintf( pdst + weblen, "Server: GoaHead\r\n" );
        weblen += sprintf( pdst + weblen, "Tue, 12 Jun 2012 01:56:34 GMT\r\n" );
        weblen += sprintf( pdst + weblen, "Content-Type:image/jpeg\r\n" );
        weblen += sprintf( pdst + weblen, "Content-Length:%d\r\n", len );
        weblen += sprintf( pdst + weblen, "Connection: close\n\n" );
    }

    memcpy( pdst + weblen, pbuf, len );

    mjpgunlock();
    return len + weblen;
}

int CaptureJpeg( char* filename )
{
    int                     	iRet;
    unsigned int           len     = 0;
    unsigned char*       pbuf   = NULL;
    LIVEHEAD            phead;
    unsigned int		offset;
    FILE*				fp = NULL;

    char writeBuffer[512*1024];

    if ( GetH264Flag() == 0x00 )
    {
        return -1;
    }

    capturelock();
    mjpglock();
    //get prelen
    offset = encbufj.prelen;
    //read live head
    memcpy( &phead, encbufj.pbuf + offset, sizeof( LIVEHEAD ) );
    //read buffer or need copy buffer?
    pbuf = encbufj.pbuf + offset + sizeof( LIVEHEAD );
    len = phead.len;

    if ( len >= 1024 * 512 )
    {
        printf( "------capture jpeg len %d keylen %d\n", len, encbufj.prelen );

        mjpgunlock();
        captureunlock();
        return -1;
    }

    memset(writeBuffer, 0, 512*1024);
    memcpy(writeBuffer, pbuf, len);

    mjpgunlock();
    captureunlock();

    fp = fopen( filename, "wb" );

    if ( fp < 0 )
    {
        return -1;
    }

    fwrite( writeBuffer, 1, len, fp );
    fclose( fp );

    return 0;
}

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/8/3 */
#if	0
/* BEGIN: Added by wupm, 2014/3/4 */
void CapturePicture(char *szImageFileName)
{
	time_t          curTime;
    int             temptime;
    struct tm*       tmptm = NULL;
	char filename[256];

	//filename mac+name+alarm+date+no
    memset( filename, 0x00, 256 );
    temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
    tmptm = localtime( &temptime );
	sprintf( szImageFileName, "%04d%02d%02d%02d%02d%02d.JPG",
                     tmptm->tm_year + 1900, tmptm->tm_mon + 1, tmptm->tm_mday,
                     tmptm->tm_hour, tmptm->tm_min, tmptm->tm_sec);
	sprintf( filename, "/mnt/%s", szImageFileName);
	CaptureJpeg( filename );
}
#endif

int SendLiveData( int socket, unsigned char* pbuf, unsigned int iBufLen )
{
    int             iRet = 0;
    int             len = 0x00;
    int             sendlen = 0;
    char*            pdst = NULL;

    if ( socket == -1 )
    {
        //netunlock();
        printf( "send jpeg end1\n" );
        return -1;
    }

    while ( len < iBufLen )
    {
        iRet = send( socket, pbuf, iBufLen, 0 );

        if ( iRet == -1 )
        {
            Textout( "send failed len=%d errno:%s\n", sendlen, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //printf("socket %d send %d total %d\n",socket,iRet,iBufLen);
        }
    }

    if ( len == -1 )
    {
        return -1;
    }

    return 0;
}

//alarm note all
int LIVE_ALARM_Send( char type )
{
    //char          szBuffer[1024];
    char            i;
    LIVEHEAD        livehead;       //livestream head
    char            sreamid;
    struct timeval  tv;
    struct timezone tz;
    int		iRet;
    char		usit;
    usit = 0;

    if ( streamuserj[usit].socket > 0 )
    {
        livehead.startcode	= STARTCODE;
        livehead.type           = type;
        livehead.len            = 0x00;
        livehead.streamid       = streamuserj[usit].streamid;
        livehead.frameno        = 0x00;
        livehead.militime       = tv.tv_usec / 1000;
        livehead.sectime        = tv.tv_sec;
        alarm0lock();
        iRet = SendLiveData( streamuserj[usit].socket, &livehead, sizeof( LIVEHEAD ) );
        alarm0unlock();
    }

    usit = 1;

    if ( streamuserj[usit].socket > 0 )
    {
        livehead.startcode	= STARTCODE;
        livehead.type           = type;
        livehead.len            = 0x00;
        livehead.streamid       = streamuserj[usit].streamid;
        livehead.frameno        = 0x00;
        livehead.militime       = tv.tv_usec / 1000;
        livehead.sectime        = tv.tv_sec;
        alarm1lock();
        iRet = SendLiveData( streamuserj[usit].socket, &livehead, sizeof( LIVEHEAD ) );
        alarm1unlock();
    }

    usit = 2;

    if ( streamuserj[usit].socket > 0 )
    {
        livehead.startcode	= STARTCODE;
        livehead.type           = type;
        livehead.len            = 0x00;
        livehead.streamid       = streamuserj[usit].streamid;
        livehead.frameno        = 0x00;
        livehead.militime       = tv.tv_usec / 1000;
        livehead.sectime        = tv.tv_sec;
        alarm2lock();
        iRet = SendLiveData( streamuserj[usit].socket, &livehead, sizeof( LIVEHEAD ) );
        alarm2unlock();
    }

    usit = 3;

    if ( streamuserj[usit].socket > 0 )
    {
        livehead.startcode	= STARTCODE;
        livehead.type           = type;
        livehead.len            = 0x00;
        livehead.streamid       = streamuserj[usit].streamid;
        livehead.frameno        = 0x00;
        livehead.militime       = tv.tv_usec / 1000;
        livehead.sectime        = tv.tv_sec;
        alarm3lock();
        iRet = SendLiveData( streamuserj[usit].socket, &livehead, sizeof( LIVEHEAD ) );
        alarm3unlock();
    }
}
//==========================jpeg rate send======================
int SendJpegData0( int socket, unsigned char* pbuf, unsigned int iBufLen )
{
    int             iRet = 0;
    int             len = 0x00;

    if ( socket == -1 )
    {
        //netunlock();
        printf( "send jpeg end1\n" );
        return -1;
    }

    alarm0lock();

    //printf("send0 %d\n",iBufLen);
    while ( len < iBufLen )
    {
        iRet = send( socket, pbuf + len, iBufLen - len, 0 );
        if ( iRet == -1 )
        {
            Textout( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }
        else
        {
            len += iRet;
            Textout0("1 socket %d send %d total %d\n",socket,len,iBufLen);
        }

    }

    //printf("send0 end\n");
    alarm0unlock();

    if ( len == -1 )
    {
        return -1;
    }

    return 0;
}
//send jpeg stream
void SendJpegStream0( char usersit )
{
    int                     		iRet = 0;
    unsigned int            	len     = 0;
    unsigned char*          	 pbuf   = NULL;
    LIVEHEAD                	phead;
    char 				usit = 0;
    unsigned int			offset = 0;
#if 1
    mjpglock();
    offset = encbufj.prelen;
    memcpy( &phead, encbufj.pbuf + offset, sizeof( LIVEHEAD ) );
    pbuf = encbufj.pbuf + offset;
    len = phead.len + sizeof( LIVEHEAD );

    //Textout("video len=%d", phead.len);
    if ( ( len >= MAX_JPEG_BUFFER_SIZE ) || ( len <= 0x00 ) )
    {
        printf( "=========jpeg len %d keylen %d=========\n", len, encbufj.prelen );
        mjpgunlock();
        return;
    }

    memcpy( livebuf0, pbuf, len );
    mjpgunlock();
    iRet = SendJpegData0( streamuserj[usit].socket, livebuf0, len );
#endif

    if ( iRet )
    {
        char j;
        char flag = 0;
        printf( "send jpeg stream failed %d\n", usit );
        socketlock();
        CloseSocket( streamuserj[usit].socket );
        streamuserj[usit].socket = -1;
        streamuserj[usit].refcnt  = -1;
        InitUserIndicator( SJPEGRATE, usit );
        socketunlock();

        for ( j = 0; j < MAX_CONN_USER_NUM; j++ )
        {
            if ( streamuserj[j].socket != -1 )
            {
                flag = 0x01;
            }
        }
    }

    //printf("user %d send %d\n",usit,iRet);
    // userindj[usit].index++;
    //userindj[usit].offset += len;
    //PopVJencData();
}

//==========================jpeg rate send======================
int SendJpegData1( int socket, unsigned char* pbuf, unsigned int iBufLen )
{
    int             iRet = 0;
    int             len = 0x00;
    int             sendlen = 0;
    char*            pdst = NULL;

    if ( socket == -1 )
    {
        //netunlock();
        printf( "send jpeg end1\n" );
        return -1;
    }

    alarm1lock();

    while ( len < iBufLen )
    {
#if 1
        iRet = send( socket, pbuf + len, iBufLen - len, 0 );

        if ( iRet == -1 )
        {
            Textout( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            Textout4("2 socket %d send %d total %d\n",socket,len,iBufLen);
        }

#else

        if ( ( iBufLen - len ) >= 1024 )
        {
            sendlen = 1024;
        }

        else
        {
            sendlen = iBufLen - len;
        }

        pdst = pbuf + len;
        iRet = send( socket, pdst, sendlen, 0 );

        if ( iRet == -1 )
        {
            printf( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //printf("socket %d send %d total %d\n",socket,iRet,iBufLen);
        }

#endif
    }

    alarm1unlock();

    //printf("send1 end\n");
    if ( len == -1 )
    {
        return -1;
    }

    return 0;
}
//send jpeg stream
void SendJpegStream1( char usersit )
{
    int                     		iRet = 0;
    unsigned int            	len     = 0;
    unsigned char*          	 pbuf   = NULL;
    LIVEHEAD                	phead;
    char 				usit = 1;
    unsigned int			offset = 0;
#if 1
    mjpglock();
    //get prelen
    offset = encbufj.prelen;
    //read live head
    memcpy( &phead, encbufj.pbuf + offset, sizeof( LIVEHEAD ) );
    //read buffer or need copy buffer?
    pbuf = encbufj.pbuf + offset;
    len = phead.len + sizeof( LIVEHEAD );

    if ( ( len >= MAX_JPEG_BUFFER_SIZE ) || ( len <= 0x00 ) )
    {
        printf( "=========jpeg len %d keylen %d=========\n", len, encbufj.prelen );
        mjpgunlock();
        return;
    }

    memcpy( livebuf1, pbuf, len );
    memcpy( &phead, livebuf1, sizeof( LIVEHEAD ) );
    //printf("prelen1 %d len %d  type %d stream %d frameno %d\n",encbufj.prelen,len,phead.type,phead.streamid,phead.frameno);
    mjpgunlock();
    //printf("send jpeg\n");
    iRet = SendJpegData1( streamuserj[usit].socket, livebuf1, len );
    //printf("send end1\n");
#endif

    if ( iRet )
    {
        char j;
        char flag = 0;
        printf( "send jpeg stream failed %d\n", usit );
        socketlock();
        CloseSocket( streamuserj[usit].socket );
        streamuserj[usit].socket = -1;
        streamuserj[usit].refcnt  = -1;
        InitUserIndicator( SJPEGRATE, usit );
        socketunlock();

        for ( j = 0; j < MAX_CONN_USER_NUM; j++ )
        {
            if ( streamuserj[j].socket != -1 )
            {
                flag = 0x01;
            }
        }
    }

    //printf("user %d send %d\n",usit,iRet);
    // userindj[usit].index++;
    //userindj[usit].offset += len;
    //PopVJencData();
}

//==========================jpeg rate send======================
int SendJpegData2( int socket, unsigned char* pbuf, unsigned int iBufLen )
{
    int             iRet = 0;
    int             len = 0x00;
    int             sendlen = 0;
    char*            pdst = NULL;

    if ( socket == -1 )
    {
        //netunlock();
        printf( "send jpeg end1\n" );
        return -1;
    }

    //printf("send2 %d\n",iBufLen);
    alarm2lock();

    while ( len < iBufLen )
    {
#if 1
        iRet = send( socket, pbuf + len, iBufLen - len, 0 );

        if ( iRet == -1 )
        {
            Textout( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            Textout4("3 socket %d send %d total %d\n",socket,len,iBufLen);
        }

#else

        if ( ( iBufLen - len ) >= 1024 )
        {
            sendlen = 1024;
        }

        else
        {
            sendlen = iBufLen - len;
        }

        pdst = pbuf + len;
        iRet = send( socket, pdst, sendlen, 0 );

        if ( iRet == -1 )
        {
            printf( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //printf("socket %d send %d total %d\n",socket,iRet,iBufLen);
        }

#endif
    }

    alarm2unlock();

    //printf("send2 end\n");
    if ( len == -1 )
    {
        return -1;
    }

    return 0;
}
//send jpeg stream
void SendJpegStream2( char usersit )
{
    int                     		iRet = 0;
    unsigned int            	len     = 0;
    unsigned char*          	 pbuf   = NULL;
    LIVEHEAD                	phead;
    char 				usit = 2;
    unsigned int			offset = 0;
#if 1
    mjpglock();
    //get prelen
    offset = encbufj.prelen;
    //read live head
    memcpy( &phead, encbufj.pbuf + offset, sizeof( LIVEHEAD ) );
    //read buffer or need copy buffer?
    pbuf = encbufj.pbuf + offset;
    len = phead.len + sizeof( LIVEHEAD );

    if ( ( len >= MAX_JPEG_BUFFER_SIZE ) || ( len <= 0x00 ) )
    {
        printf( "=========jpeg len %d keylen %d=========\n", len, encbufj.prelen );
        mjpgunlock();
        return;
    }

    memcpy( livebuf2, pbuf, len );
    memcpy( &phead, livebuf2, sizeof( LIVEHEAD ) );
    //printf("prelen %d len %d  type %d stream %d frameno %d\n",encbufj.prelen,len,phead.type,phead.streamid,phead.frameno);
    mjpgunlock();
    //printf("jpeg len:%d\n",len);
    iRet = SendJpegData2( streamuserj[usit].socket, livebuf2, len );
#endif

    if ( iRet )
    {
        char j;
        char flag = 0;
        printf( "send jpeg stream failed %d\n", usit );
        socketlock();
        CloseSocket( streamuserj[usit].socket );
        streamuserj[usit].socket = -1;
        streamuserj[usit].refcnt  = -1;
        InitUserIndicator( SJPEGRATE, usit );
        socketunlock();

        for ( j = 0; j < MAX_CONN_USER_NUM; j++ )
        {
            if ( streamuserj[j].socket != -1 )
            {
                flag = 0x01;
            }
        }
    }

    //printf("user %d send %d\n",usit,iRet);
    // userindj[usit].index++;
    //userindj[usit].offset += len;
    //PopVJencData();
}

//==========================jpeg rate send======================
int SendJpegData3( int socket, unsigned char* pbuf, unsigned int iBufLen )
{
    int             iRet = 0;
    int             len = 0x00;
    int             sendlen = 0;
    char*            pdst = NULL;

    if ( socket == -1 )
    {
        //netunlock();
        Textout5( "send jpeg end1\n" );
        return -1;
    }

    //printf("send3 %d\n",iBufLen);
    alarm3lock();

    while ( len < iBufLen )
    {
#if 1
        iRet = send( socket, pbuf + len, iBufLen - len, 0 );

        if ( iRet == -1 )
        {
            Textout( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //Textout5("4 socket %d send %d total %d\n",socket,len,iBufLen);
        }

#else

        if ( ( iBufLen - len ) >= 1024 )
        {
            sendlen = 1024;
        }

        else
        {
            sendlen = iBufLen - len;
        }

        pdst = pbuf + len;
        iRet = send( socket, pdst, sendlen, 0 );

        if ( iRet == -1 )
        {
            printf( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //printf("socket %d send %d total %d\n",socket,iRet,iBufLen);
        }

#endif
    }

    alarm3unlock();

    //printf("send3 end\n");
    if ( len == -1 )
    {
        return -1;
    }

    return 0;
}
//send jpeg stream
void SendJpegStream3( char usersit )
{
    int                     		iRet = 0;
    unsigned int            	len     = 0;
    unsigned char*          	 pbuf   = NULL;
    LIVEHEAD                	phead;
    char 				usit = 3;
    unsigned int			offset = 0;
#if 1
    mjpglock();
    //get prelen
    offset = encbufj.prelen;
    //read live head
    memcpy( &phead, encbufj.pbuf + offset, sizeof( LIVEHEAD ) );
    //read buffer or need copy buffer?
    pbuf = encbufj.pbuf + offset;
    len = phead.len + sizeof( LIVEHEAD );

    if ( ( len >= MAX_JPEG_BUFFER_SIZE ) || ( len <= 0x00 ) )
    {
        printf( "=========jpeg len %d keylen %d=========\n", len, encbufj.prelen );
        mjpgunlock();
        return;
    }

    memcpy( livebuf3, pbuf, len );
    memcpy( &phead, livebuf3, sizeof( LIVEHEAD ) );
    //printf("prelen %d len %d  type %d stream %d frameno %d\n",encbufj.prelen,len,phead.type,phead.streamid,phead.frameno);
    mjpgunlock();
    //Textout5("4 jpeg len:%d\n",len);
    iRet = SendJpegData3( streamuserj[usit].socket, livebuf3, len );
    //iRet = 0;
#endif

    if ( iRet )
    {
        char j;
        char flag = 0;
        printf( "send jpeg stream failed %d\n", usit );
        socketlock();
        CloseSocket( streamuserj[usit].socket );
        streamuserj[usit].socket = -1;
        streamuserj[usit].refcnt  = -1;
        InitUserIndicator( SJPEGRATE, usit );
        socketunlock();

        for ( j = 0; j < MAX_CONN_USER_NUM; j++ )
        {
            if ( streamuserj[j].socket != -1 )
            {
                flag = 0x01;
            }
        }
    }

    //printf("user %d send %d\n",usit,iRet);
    // userindj[usit].index++;
    //userindj[usit].offset += len;
    //PopVJencData();
}
void SetAudioIndex( short index )
{
    char i;

    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        //userindj[i].aindex = index;
    }
}
//send audio sem
/* BEGIN: Continue try to Resolve BUG of System Halt */
/*        Deleted by wupm(2073111@qq.com), 2014/10/23 */

void NoteAudioSend( void )
{
    char i;

    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        sem_post( &aisemsend[i] );
    }
}
//==========================jpeg rate send======================
int SendAudioData( int socket, unsigned char* pbuf, unsigned int iBufLen )
{
    int             iRet = 0;
    int             len = 0x00;
    int             sendlen = 0;
    char*            pdst = NULL;

    if ( socket == -1 )
    {
        //netunlock();
        printf( "send jpeg end1\n" );
        return -1;
    }

    while ( len < iBufLen )
    {
        iRet = send( socket, pbuf, iBufLen, 0 );

        if ( iRet == -1 )
        {
            Textout( "send failed len=%d errno:%s\n", sendlen, strerror( errno ) );
            SetAudioGongFang(FALSE);
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //printf("socket %d send %d total %d\n",socket,iRet,iBufLen);
        }
    }

    if ( len == -1 )
    {
        return -1;
    }

    return 0;
}
//send jpeg audio
void SendJpegAudio( char usit )
{
    int                     iRet;
    unsigned int        len     = 0;
    unsigned char*     pbuf   = NULL;
    LIVEHEAD           phead;
    unsigned char	    temp[1024];
    //printf("SendJpegAudio %d %d\n",userindj[usit].aindex,encbufa.preindex);
    //check new audio
#if 0

    if ( userindj[usit].aindex == encbufa.preindex )
    {
        return;
    }

#endif
    audiolock();
    userindj[usit].aindex = encbufa.preindex;
    //read live head
    userindj[usit].aoffset = encbufa.prelen;
    memcpy( &phead, encbufa.pbuf + userindj[usit].aoffset, sizeof( LIVEHEAD ) );
    //read buffer or need copy buffer?
    pbuf = encbufa.pbuf + userindj[usit].aoffset;

    if ( len >= 1024 * 128 )
    {
        printf( "=========read audio len failed=%d============", len );
        audiounlock();
        return;
    }

#if 0
    len = phead.len;
    //printf("adpcm encoder\n");
    ADPCMEncode( pbuf + sizeof( LIVEHEAD ), len, temp + sizeof( LIVEHEAD ) );
    phead.len = len / 4;
    memcpy( temp, &phead, sizeof( LIVEHEAD ) );
    //printf("send audio %d\n",len/4);
    iRet = SendJpegData1( streamuserj[usit].socket, temp, len / 4 + sizeof( LIVEHEAD ) );	//adpcm = pcm /4
#else
    len = phead.len + sizeof( LIVEHEAD );
    memcpy( temp, pbuf, len );
    audiounlock();
    iRet = SendAudioData( audiouserj[usit].socket, temp, len );
    //printf("Send Audio Data %d\n",len);
#endif

    if ( iRet )
    {
        char j;
        char flag = 0;
        printf( "send jpeg audio failed\n" );
        socketlock();
        CloseSocket( audiouserj[usit].socket );
        audiouserj[usit].socket = -1;
        audiouserj[usit].refcnt  = -1;
        socketunlock();

        for ( j = 0; j < MAX_CONN_USER_NUM; j++ )
        {
            if ( audiouserj[j].socket != -1 )
            {
                flag = 0x01;
            }
        }
    }

    //printf("audio user %d send %d\n",usit,len);
}
//send jpeg stream proc
/* BEGIN: Continue try to Resolve BUG of System Halt */
/*        Deleted by wupm(2073111@qq.com), 2014/10/23 */
#if	1
void* AudioSendProc1( void* p )
{
    int     iRet;
    int usit = 0;

    if ( ( sem_init( &aisemsend[usit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    printf( "send live audio:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &aisemsend[usit] );

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 10000 );
            continue;
        }

        if ( audiouserj[usit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        //send audio
        if ( audiouserj[usit].audio == 0x01 )
        {
            //send audio
            SendJpegAudio( usit );
        }

        usleep( 1000 );
    }
}

void* AudioSendProc2( void* p )
{
    int     iRet;
    int usit = 1;

    if ( ( sem_init( &aisemsend[usit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    printf( "send live audio:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &aisemsend[usit] );

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 10000 );
            continue;
        }

        if ( audiouserj[usit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        //send audio
        if ( audiouserj[usit].audio == 0x01 )
        {
            //send audio
            SendJpegAudio( usit );
        }

        usleep( 1000 );
    }
}
void* AudioSendProc3( void* p )
{
    int     iRet;
    int usit = 2;

    if ( ( sem_init( &aisemsend[usit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    printf( "send live audio:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &aisemsend[usit] );

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 10000 );
            continue;
        }

        if ( audiouserj[usit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        //send audio
        if ( audiouserj[usit].audio == 0x01 )
        {
            //send audio
            SendJpegAudio( usit );
        }

        usleep( 1000 );
    }
}

void* AudioSendProc4( void* p )
{
    int     iRet;
    int usit = 3;

    if ( ( sem_init( &aisemsend[usit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    printf( "send live audio:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &aisemsend[usit] );

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 10000 );
            continue;
        }

        if ( audiouserj[usit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        //send audio
        if ( audiouserj[usit].audio == 0x01 )
        {
            //send audio
            SendJpegAudio( usit );
        }

        usleep( 1000 );
    }
}
#endif
/* END: Deleted by wupm(2073111@qq.com), 2014/10/23 */

//send jpeg stream proc
/* BEGIN: Continue try to Resolve BUG of System Halt */
/*        Deleted by wupm(2073111@qq.com), 2014/10/23 */
#if	1
void* SendLiveProcj0( void* p )
{
    int     iRet;
    char    streamid = -1;
    int    usersit = 0;

    if ( ( sem_init( &usersemj[usersit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    usertimesj[usersit] = 0;
    printf( "send live jpeg:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &usersemj[usersit] );

        if ( streamuserj[usersit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 1000 );
            continue;
        }

        wifiok = 1;
        streamid = streamuserj[usersit].streamid;
        //send video
        SendJpegStream0( usersit );
        //printf("jpeg0 send end\n");
#if 0

        //send audio
        if ( streamuserj[usersit].audio == 0x01 )
        {
            //send audio
            SendJpegAudio( usersit );
        }

#endif
        usleep( 1000 );
    }
}
void* SendLiveProcj1( void* p )
{
    int     iRet;
    char    streamid = -1;
    int    usersit = 1;

    if ( ( sem_init( &usersemj[usersit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    usertimesj[usersit] = 0;
    printf( "send live jpeg:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &usersemj[usersit] );

        if ( streamuserj[usersit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 1000 );
            continue;
        }

        wifiok = 1;
        streamid = streamuserj[usersit].streamid;
        Textout0("jpeg1 rate streamid %d\n",streamid);
        //send video
        SendJpegStream1( usersit );
        //printf("jpeg1 send end\n");
#if 0

        //send audio
        if ( streamuserj[usersit].audio == 0x01 )
        {
            //send audio
            SendJpegAudio( usersit );
        }

#endif
        usleep( 1000 );
    }
}
void* SendLiveProcj2( void* p )
{
    int     iRet;
    char    streamid = -1;
    int    usersit = 2;
    usertimesj[usersit] = 0;

    if ( ( sem_init( &usersemj[usersit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    printf( "send live jpeg:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &usersemj[usersit] );

        if ( streamuserj[usersit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 1000 );
            continue;
        }

        wifiok = 1;
        streamid = streamuserj[usersit].streamid;
        Textout0("jpeg2 rate streamid %d\n",streamid);
        //send video
        SendJpegStream2( usersit );
        //printf("jpeg2 send end\n");
#if 0

        //send audio
        if ( streamuserj[usersit].audio == 0x01 )
        {
            //send audio
            SendJpegAudio( usersit );
        }

#endif
        usleep( 1000 );
    }
}
void* SendLiveProcj3( void* p )
{
    int     iRet;
    char    streamid = -1;
    int    usersit = 3;
    usertimesj[usersit] = 0;

    if ( ( sem_init( &usersemj[usersit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    printf( "send live jpeg:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &usersemj[usersit] );

        if ( streamuserj[usersit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 1000 );
            continue;
        }

        wifiok = 1;
        streamid = streamuserj[usersit].streamid;
        Textout0("jpeg3 rate streamid %d\n",streamid);
        //send video

		/* BEGIN: Modified by wupm, 2013/4/10 */
        //SendJpegStream3(usersit);
        //printf("jpeg3 send end\n");
#if 0

        //send audio
        if ( streamuserj[usersit].audio == 0x01 )
        {
            //send audio
            SendJpegAudio( usersit );
        }

#endif
        usleep( 1000 );
    }
}
#endif
/* END: Deleted by wupm(2073111@qq.com), 2014/10/23 */

//==========================jpeg rate send======================
int SendVideoData( int socket, unsigned char* pbuf, unsigned int iBufLen )
{
    int             iRet = 0;
    int             len = 0x00;
    int             sendlen = 0;
    char*            pdst = NULL;

    if ( socket == -1 )
    {
        //netunlock();
        printf( "send jpeg end1\n" );
        return -1;
    }

    //printf("send3 %d\n",iBufLen);
    //alarm3lock();
    while ( len < iBufLen )
    {
#if 1
        iRet = send( socket, pbuf + len, iBufLen - len, 0 );

        if ( iRet == -1 )
        {
            printf( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //printf("socket %d send %d total %d\n",socket,iRet,iBufLen);
        }

#else

        if ( ( iBufLen - len ) >= 1024 )
        {
            sendlen = 1024;
        }

        else
        {
            sendlen = iBufLen - len;
        }

        pdst = pbuf + len;
        iRet = send( socket, pdst, sendlen, 0 );

        if ( iRet == -1 )
        {
            printf( "send failed len=%d error %d errno:%s\n", iRet, errno, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //printf("socket %d send %d total %d\n",socket,iRet,iBufLen);
        }

#endif
    }

    //alarm3unlock();
    //printf("send3 end\n");
    if ( len == -1 )
    {
        return -1;
    }

    return 0;
}
//set video to 0
void setvideooffset( void )
{
    userindj[0].offset = 0;
    userindj[1].offset = 0;
    userindj[2].offset = 0;
    userindj[3].offset = 0;
}

//send video stream
void SendVideoStream( int usit )
{
    int                     	iRet;
    unsigned int           len     = 0;
    unsigned int		slen    = 0;
    unsigned char*        pbuf   = NULL;
    LIVEHEAD              phead;
    unsigned int		offset  = 0;
    unsigned char*		sendbuf = videobuf;
    char*			 date;
    int				testlen = 0;
#if 0
    struct  timeval start, end;
    int     timeuse;
#endif
    mjpglock();
    //get prelen
    offset = encbufj.prelen;
    //read live head
    memcpy( &phead, encbufj.pbuf + offset, sizeof( LIVEHEAD ) );
    //read buffer or need copy buffer?
    pbuf = encbufj.pbuf + offset + sizeof( LIVEHEAD );
    len = phead.len;
    //printf("frame no:%d len %d\n",phead.frameno,len);

    if ( ( len >= MAX_JPEG_BUFFER_SIZE ) || ( len == 0x00 ) )
    {
        printf( "jpeg len %d keylen %d\n", len, encbufj.prelen );
        mjpgunlock();
        return;
    }

    if ( videouserj[usit].http == 0x01 )
    {
        slen =  sprintf( sendbuf + slen, "HTTP/1.1 200 OK\r\n" );

        if ( ( date = websGetDateString( NULL ) ) != NULL )
        {
            slen += sprintf( sendbuf + slen, "Date: %s\r\n", date );
        }

        slen += sprintf( sendbuf + slen, "Server: GoAhead-Webs\r\n" );
        slen += sprintf( sendbuf + slen, "Accept-Ranges: bytes\r\n" );
        slen += sprintf( sendbuf + slen, "Connection: close\r\n" );
        slen += sprintf( sendbuf + slen, "Content-Type: multipart/x-mixed-replace;boundary=object-ipcamera\r\n" );
        videouserj[usit].http = 0x00;
    }

    slen += sprintf( sendbuf + slen, "\r\n--object-ipcamera\r\n" );
    slen += sprintf( sendbuf + slen, "Content-Type:image/jpeg\r\n" );
    slen += sprintf( sendbuf + slen, "Content-Length:%d\r\n\r\n", len ); //+strlen("\r\n--object-ipcamera\r\n"));
    memcpy( sendbuf + slen, pbuf, len );
    //sprintf(sendbuf+slen+len,"\r\n--object-ipcamera\r\n");
    slen += len;
    mjpgunlock();
    // printf("video len:%d socket %d\n",slen,videouserj[usit].socket);
    //  gettimeofday( &start, NULL );
    iRet = SendVideoData( videouserj[usit].socket, sendbuf, slen );

    //gettimeofday( &end, NULL );
    //timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    //printf("diff %d\n",timeuse);
    if ( iRet )
    {
        char j;
        char flag = 0;
        socketlock();
        CloseSocket( videouserj[usit].socket );
        videouserj[usit].socket = -1;
        videouserj[usit].refcnt  = -1;
        socketunlock();

        for ( j = 0; j < MAX_CONN_USER_NUM; j++ )
        {
            if ( videouserj[j].socket != -1 )
            {
                flag = 0x01;
            }
        }
    }

    userindj[usit].offset += len + sizeof( LIVEHEAD );
}
//send video stream proc

/* BEGIN: Continue try to Resolve BUG of System Halt */
/*        Deleted by wupm(2073111@qq.com), 2014/10/23 */
#if	0
void* SendVideoProcj0( void* p )
{
    int     iRet;
    char    streamid = -1;
    char  usersit = 0;

    if ( ( sem_init( &videosemj[usersit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    videotimesj[usersit] = 0;
    printf( "send video jpeg:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &videosemj[usersit] );

        if ( videouserj[usersit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 1000 );
            continue;
        }

        wifiok = 1;
        streamid = videouserj[usersit].streamid;
        //send video
        Textout5("send video stream %d\n",usersit);
        SendVideoStream( usersit );
        //printf("send video sream end %d\n",usersit);
        //usleep(10);
    }
}

//send video stream proc
void* SendVideoProcj1( void* p )
{
    int     iRet;
    char    streamid = -1;
    char  usersit = 1;

    if ( ( sem_init( &videosemj[usersit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    videotimesj[usersit] = 0;
    printf( "send video jpeg:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &videosemj[usersit] );

        if ( videouserj[usersit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 1000 );
            continue;
        }

        wifiok = 1;
        streamid = videouserj[usersit].streamid;
        //send video
        Textout5("send video stream %d\n",usersit);
        SendVideoStream( usersit );
        //printf("send video stream end %d\n",usersit);
        usleep( 1000 );
    }
}

//send video stream proc
void* SendVideoProcj2( void* p )
{
    int     iRet;
    char    streamid = -1;
    char  usersit = 2;
    videotimesj[usersit] = 0;

    if ( ( sem_init( &videosemj[usersit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    printf( "send video jpeg:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &videosemj[usersit] );

        if ( videouserj[usersit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 1000 );
            continue;
        }

        wifiok = 1;
        streamid = videouserj[usersit].streamid;
        //send video
        Textout5("send video stream %d\n",usersit);
        SendVideoStream( usersit );
        //printf("send video stream end %d\n",usersit);
        usleep( 1000 );
    }
}
//send video stream proc
void* SendVideoProcj3( void* p )
{
    int     iRet;
    char    streamid = -1;
    char  usersit = 3;
    videotimesj[usersit] = 0;

    if ( ( sem_init( &videosemj[usersit], 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    printf( "send video jpeg:%d\n", getpid() );

    while ( 1 )
    {
        sem_wait( &videosemj[usersit] );

        if ( videouserj[usersit].socket == -1 )
        {
            usleep( 1000 );
            continue;
        }

        if ( GetH264Flag() == 0x00 )
        {
            usleep( 1000 );
            continue;
        }

        wifiok = 1;
        streamid = videouserj[usersit].streamid;
        //send video
        Textout5("send video stream %d\n",usersit);
        //SendVideoStream( usersit );
        //printf("send video stream end %d \n",usersit);
        usleep( 1000 );
    }
}
#endif
/* END: Deleted by wupm(2073111@qq.com), 2014/10/23 */

#if	0

void recordcontrol( int cmd , int offset, int sessid )
{
    recordlock();

    switch ( cmd )
    {
        case RECORD_MOVE:
            userindr[sessid].offset  = offset;
            userindr[sessid].flag  = 0x01;
            break;

        case RECORD_SUSPEND:
            userindr[sessid].recordflag = 0x01;
            break;

        case RECORD_RESUME:
            userindr[sessid].recordflag = 0x00;
            break;

        default:
            break;
    }

    recordunlock();
}
//==========================h264 rate send======================
int SendRecordData( int socket, unsigned char* pbuf, unsigned int iBufLen )
{
    int             iRet = 0;
    int             len = 0x00;
    int             sendlen = 0;
    char*            pdst = NULL;

    //netlock();
    if ( socket == -1 )
    {
        //netunlock();
        printf( "send jpeg end1\n" );
        return -1;
    }

    while ( len < iBufLen )
    {
#if 0

        if ( ( iBufLen - len ) >= 1024 )
        {
            sendlen = 1024;
        }

        else
        {
            sendlen = iBufLen - len;
        }

        pdst = pbuf + len;
#endif
        iRet = send( socket, pbuf, iBufLen, 0 );

        if ( iRet == -1 )
        {
            Textout( "send failed len=%d errno:%s\n", sendlen, strerror( errno ) );
            len = -1;
            break;
        }

        else
        {
            len += iRet;
            //printf("socket %d send %d total %d\n",socket,iRet,iBufLen);
        }
    }

    //netunlock();
    if ( len == -1 )
    {
        return -1;
    }

    return 0;
}

int ReadFileData( char usit, unsigned char* pbuf, unsigned int* length )
{
    FILE*			 fp;
    LIVEHEAD               phead;
    int				iRet;
    int 				status = LIVE_RECORD_END;
    unsigned int		len = 0;
    char				filename[64];;

    Textout1("Read SDCard recorder file[%s]\n", userindr[usit].filename);
    fp = fopen( userindr[usit].filename, "rb" );

    if ( fp == NULL )
    {
        return LIVE_RECORD_NOT;
    }

    Textout1("userindr[usit].flag=%d", userindr[usit].flag);
    if ( userindr[usit].flag == 0x01 )
    {
        //find startcode
        userindr[usit].flag = 0x00;

        while ( !feof( fp ) )
        {
            fseek( fp, userindr[usit].offset, SEEK_SET );
            iRet = fread( &phead, 1, sizeof( LIVEHEAD ), fp );

            Textout1("read file, [audio=0x06, mj=0x03, h264=0,1]type=%d, len=%d", phead.type, phead.len);
            //find start key frame
            if ( ( phead.startcode == STARTCODE ) && ( phead.type == 0x03 ) )
            {
                if ( ( phead.len + sizeof(LIVEHEAD))>= MAX_JPEG_BUFFER_SIZE )
                {
                    printf( "phead.len=%d, >= 1024 * 256\n", phead.len);
                    status =  LIVE_RECORD_BAD;
                }

                else
                {
                    //printf("find start head and len %d\n",phead.len);
                    phead.streamid = RECORDRATE;
                    phead.sessid = usit;
                    memcpy( pbuf, &phead, sizeof( LIVEHEAD ) );
                    fread( pbuf + sizeof( LIVEHEAD ), 1, phead.len, fp );
                    len =  phead.len + sizeof( LIVEHEAD );
                    status = LIVE_RECORD_OK;
                    userindr[usit].offset += len;
                }

                break;
            }

            userindr[usit].offset++;
        }
    }

    else
    {
        fseek( fp, userindr[usit].offset, SEEK_SET );
        iRet = fread( &phead, 1, sizeof( LIVEHEAD ), fp );

        Textout1("read file, startcode=0x%08x, type=%d, len=%d", phead.startcode, phead.type, phead.len);
        if ( phead.startcode == STARTCODE )
        {
            if ( ( phead.len + sizeof(LIVEHEAD))>= MAX_JPEG_BUFFER_SIZE )
            {
                printf( "phead.len=%d, >= 128*1024\n", phead.len );
                status =  LIVE_RECORD_BAD;
            }

            else
            {
                //printf("find1 start head and len %d\n",phead.len);
                phead.streamid = RECORDRATE;
                phead.sessid = usit;
                memcpy( pbuf, &phead, sizeof( LIVEHEAD ) );
                fread( pbuf + sizeof( LIVEHEAD ), 1, phead.len, fp );
                len =  phead.len + sizeof( LIVEHEAD );
                status = LIVE_RECORD_OK;
                userindr[usit].offset += len;
            }
        }
    }

    fclose( fp );
    *length = len;
    //printf("");
    //printf("offset %d len %d\n",userindr[usit].offset,len);
    return status;
}

//send jpeg stream
void SendRecordStream( char usit )
{
    int                     	iRet;
    int				status;
    unsigned int           len     = 0;
    unsigned char*        pbuf   = NULL;
    unsigned char		 sendbuf[MAX_JPEG_BUFFER_SIZE];
    static unsigned int	 frameno = 0;

    if ( userindr[usit].recordflag  == 0x01 )
    {
        return;
    }

    recordlock();

    if ( userindr[usit].flag > 0 )
    {
        frameno = 0;
    }

    frameno++;
    status = ReadFileData( usit, sendbuf, &len );
    // printf("read len %d\n");
#if 0
    //read live head
    memcpy( &phead, encbufm.pbuf + userindr[usit].offset, sizeof( LIVEHEAD ) );
    //read buffer or need copy buffer?
    pbuf = encbufm.pbuf + userindr[usit].offset;
    len = phead.len + sizeof( LIVEHEAD );
#endif
    //printf("offset %d prelen %d len %d\n",userindm[usit].offset,encbufm.prelen,len);
    //printf("frameno %d type %d\n",frameno,phead.type);
#if 0

    if ( len >= 1024 * 128 )
    {
        status = -1;
    }

    else
    {
        memcpy( sendbuf, pbuf, len );
        status = 0;
    }

#endif
    recordunlock();

//	 printf("h264 len:%d\n",len);
    if ( status == 0x00 )
    {
        iRet = SendRecordData( streamuserr[usit].socket, sendbuf, len );
    }

    else
    {
        iRet = -1;
        //send err code
        printf( "len >= 1024 * 128\n" );
    }

    if ( iRet )
    {
        char j;
        char flag = 0;
        printf( "send failed %d\n", usit );
        socketlock();
        CloseSocket( streamuserr[usit].socket );
        streamuserr[usit].socket = -1;
        streamuserr[usit].refcnt  = -1;
        InitUserIndicator( RECORDRATE, usit );
        socketunlock();
    }

    //  userindr[usit].offset += len;
}
//send record stream proc
void* SendRecordProcj0( void* p )
{
    int     iRet;
    char    streamid = -1;
    char    usersit = 0;
    printf( "send record file:%d\n", getpid() );

    while ( 1 )
    {
        if ( streamuserr[usersit].socket == -1 )
        {
            usleep( 500000 );
            continue;
        }

        streamid = streamuserr[usersit].streamid;

        //printf("h264 rate streamid %d\n",streamid);
        if ( streamid == RECORDRATE )
        {
            SendRecordStream( usersit );
        }

        usleep( 40000 );
    }
}
void* SendRecordProcj1( void* p )
{
    int     iRet;
    char    streamid = -1;
    char    usersit = 1;
    printf( "send record file:%d\n", getpid() );

    while ( 1 )
    {
        if ( streamuserr[usersit].socket == -1 )
        {
            usleep( 500000 );
            continue;
        }

        streamid = streamuserr[usersit].streamid;

        //printf("h264 rate streamid %d\n",streamid);
        if ( streamid == RECORDRATE )
        {
            SendRecordStream( usersit );
        }

        usleep( 40000 );
    }
}
void* SendRecordProcj2( void* p )
{
    int     iRet;
    char    streamid = -1;
    char    usersit = 2;
    printf( "send record file:%d\n", getpid() );

    while ( 1 )
    {
        if ( streamuserr[usersit].socket == -1 )
        {
            usleep( 500000 );
            continue;
        }

        streamid = streamuserr[usersit].streamid;

        //printf("h264 rate streamid %d\n",streamid);
        if ( streamid == RECORDRATE )
        {
            SendRecordStream( usersit );
        }

        usleep( 1000 );
    }
}

void* SendRecordProcj3( void* p )
{
    int     iRet;
    char    streamid = -1;
    char    usersit = 3;
    printf( "send record file:%d\n", getpid() );

    while ( 1 )
    {
        if ( streamuserr[usersit].socket == -1 )
        {
            usleep( 500000 );
            continue;
        }

        streamid = streamuserr[usersit].streamid;

        //printf("h264 rate streamid %d\n",streamid);
        if ( streamid == RECORDRATE )
        {
            SendRecordStream( usersit );
        }

        usleep( 1000 );
    }
}
#endif

//audio play stream proc
void* AudioPlayProc( void* p )
{
    int     			iRet;
    int    			i = 0;
    char				flag;
    char    			streamid = -1;
    fd_set  			fdSocket;
    int             		maxsock;
    struct timeval 	tv;
    Textout( "AudioPlayProc:%d", getpid() );

    while ( 1 )
    {
        flag = 0x00;
        FD_ZERO( &fdSocket );

        for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
        {
            if ( streamuserj[i].socket != -1 )
            {
                FD_SET( streamuserj[i].socket, &fdSocket );
                maxsock = max( maxsock, streamuserj[i].socket );
                flag = 0x01;
                //break;
            }
        }

        if ( flag == 0x00 )
        {
            sleep( 2 );
            continue;
        }

        //select time
        tv.tv_sec       	= 2;
        tv.tv_usec      	= 0;
        //select socket incoming
        iRet = select( maxsock + 1, &fdSocket, NULL, NULL, &tv );

        if ( iRet < 0 )
        {
            //printf("select failed max socket %d errno %d\n",maxsock,errno);
            sleep( 1 );
            continue;
        }

        else if ( iRet == 0 )
        {
            sleep( 1 );
            continue;
        }

        //select talk socket
        for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
        {
            if ( ( streamuserj[i].socket != -1 ) && ( FD_ISSET( streamuserj[i].socket, &fdSocket ) ) )
            {
                int             	socketfd;
                int             	len;
                LIVEHEAD       phead;
                char            	flag = 0x00;
                //recive audio buffer
                //printf("select flag=%d\n",i);
                socketfd = streamuserj[i].socket;

                if ( streamuserj[i].talklen < MAX_TALKLEN )
                {
                    len = recv( streamuserj[i].socket, streamuserj[i].buffer + streamuserj[i].talklen, MAX_TALKLEN - streamuserj[i].talklen, 0 );
                    if ( len > 0 )
                    {
                        flag = 0x01;
                        streamuserj[i].talklen += len;
                        Textout0("main recive talk len %d",len);
                    }

                    else
                    {
                        streamuserj[i].talklen = 0x00;
                        /* BEGIN: Added by wupm, 2013/3/1 */
                        /*
                        Textout( "recive failed by wupm errno=%d\n", errno );
                        CloseSocket( streamuserj[i].socket );
                        //PopSocket( acceptsock[i].socket );
                        usleep( 100000 );
                        continue;
                        */
                        /* END:   Added by wupm, 2013/3/1 */
                    }
                }

                else
                {
                    streamuserj[i].talklen = 0x00;
                }

                if ( flag == 0x00 )
                {
                    usleep( 100000 );
                    continue;
                }

                //find start code
                memcpy( &phead, streamuserj[i].buffer, sizeof( LIVEHEAD ) );

				/* BEGIN: Modified by wupm(2073111@qq.com), 2014/7/10 */
                //Textout0("phead startcode=0x%08x, type %d phead len %d, frameno=%d",phead.startcode, phead.type,phead.len, phead.frameno);
                //if ( ( phead.startcode != STARTCODE ) || ( phead.type != 0x08 ) || ( phead.len != MAX_TALKLEN_DATA ) )
                if ( ( phead.type != 0x08 ) || ( phead.len != MAX_TALKLEN_DATA ) )
                {
                    short sit = 0;

                    while ( streamuserj[i].talklen > 0 )
                    {
                        memcpy( &phead, streamuserj[i].buffer + sit, sizeof( LIVEHEAD ) );

                        if ( ( phead.type == 0x08 ) && ( phead.len == MAX_TALKLEN_DATA ) )
                        {
                            //printf("Find head=================%d\n",streamuserj[i].talklen);
                            break;
                        }

                        else
                        {
                            streamuserj[i].talklen--;
                            sit++;
                        }
                    }

                    //continue;
                }

                //check stream head
                if ( streamuserj[i].talklen >= MAX_TALKLEN )
                {
                    //printf("talklen %d\n",streamuserj[i].talklen);
                    memcpy( &phead, streamuserj[i].buffer, sizeof( LIVEHEAD ) );

                    if ( phead.type == 0x08 )
                    {
                        //send audio data
                        //Textout("send audio data\n");
                        /* BEGIN: Added by Baggio.wu, 2013/9/23 */
                        phead.adpcmsample = 0;
                        phead.adpcmindex = 0;
						/* BEGIN: Added by wupm, 2013/5/29 */
						DecoderClr(phead.adpcmsample, phead.adpcmindex);

                        Textout0("sit=%d, len=%d, frameno=%d, adpcmsample=%d, adpcmindex=%d",
                                i, MAX_TALKLEN - sizeof( LIVEHEAD ), phead.frameno,
                                phead.adpcmsample, phead.adpcmindex);

                        HD_AudioOutSendDataNew( streamuserj[i].buffer + sizeof( LIVEHEAD ), MAX_TALKLEN - sizeof( LIVEHEAD ));
                    }

                    streamuserj[i].talklen = 0x00;
                }
            }
        }

        usleep( 1000 );
    }
}
//stream thread init
void StreamUserThread( void )
{
    pthread_t        threadj0;
    pthread_t        threadj1;
    pthread_t        threadj2;
    pthread_t        threadj3;
    pthread_t		threada1;
    pthread_t		threada2;
    pthread_t		threada3;
    pthread_t		threada4;
    pthread_t		threadao;

	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/7/10 */
	#if	1

    if ( pthread_create( &threadj0, NULL, &SendLiveProcj0, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threadj1, NULL, &SendLiveProcj1, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threadj2, NULL, &SendLiveProcj2, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threadj3, NULL, &SendLiveProcj3, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

	#endif
	
	#if	0

    if ( pthread_create( &threadv0, NULL, &SendVideoProcj0, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threadv1, NULL, &SendVideoProcj1, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threadv2, NULL, &SendVideoProcj2, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threadv3, NULL, &SendVideoProcj3, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

	#endif

	#if	1

    if ( pthread_create( &threada1, NULL, &AudioSendProc1, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threada2, NULL, &AudioSendProc2, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threada3, NULL, &AudioSendProc3, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threada4, NULL, &AudioSendProc4, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threadao, NULL, AudioPlayProc, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

	#endif

	#if	0

    if ( pthread_create( &threadr0, NULL, SendRecordProcj0, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threadr1, NULL, SendRecordProcj1, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threadr2, NULL, SendRecordProcj2, NULL ) )
    {
        printf( "network createthread failed\n" );
    }

    if ( pthread_create( &threadr3, NULL, SendRecordProcj3, NULL ) )
    {
        printf( "network createthread failed\n" );
    }
	#endif
}
//network command init
void NetCmdInit( void )
{
#if 0
    pjpeg = malloc( 1024 * 128 );

    if ( pjpeg == NULL )
    {
        printf( "pjpeg is null\n" );
    }

    paudio = malloc( 1024 * 128 );

    if ( paudio == NULL )
    {
        printf( "paudio is null\n" );
    }

    pvideo = malloc( 1024 * 128 );

    if ( pvideo == NULL )
    {
        printf( "pvideo is null\n" );
    }

#endif
    psnap = malloc( MAX_JPEG_BUFFER_SIZE);

    if ( psnap == NULL )
    {
        printf( "psnap is null\n" );
    }

    StreamUserInit();
    NetCmdSocketInit();
}
//network command thread init
void NetCmdThread( void )
{
    StreamUserThread();
}

/* BEGIN: Added by wupm, 2013/5/6 */
#ifdef	AUTO_REBOOT
BOOL CheckLiveUser()
{
	int i;

	for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        if (
			(streamuserj[i].socket != -1 && streamuserj[i].socket != 0) ||	//livestream user
			(streamuserr[i].socket != -1 && streamuserr[i].socket != 0) ||	//record user
			(videouserj[i].socket != -1 && videouserj[i].socket != 0) ||	//videostream user
			(audiouserj[i].socket != -1 && audiouserj[i].socket != 0) 		//audio stream user
			)
		{
			Textout("index = %d, streamuserj = %d, userr=%d, videouser = %d, audio =%d",
				i,
				streamuserj[i].socket,
				streamuserr[i].socket,
				videouserj[i].socket,
				audiouserj[i].socket
				);
			return TRUE;
		}
    }
	return FALSE;
}
#endif


