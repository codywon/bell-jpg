
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "param.h"
#include "protocol.h"

#include "vbuf.h"
#include "mjpgbuf.h"
#include "subbuf.h"

const unsigned char* pbufm = NULL;

ENCBUF              encbufm;
ENCBUFIND       	userindm[MAX_CONN_USER_NUM];
ENCBUFIND       	userindp2p[MAX_CONN_USER_NUM];

ENCBUFIND       	userindsd;
ENCBUFIND       	userindonvif;
pthread_mutex_t     bufmutextm = PTHREAD_MUTEX_INITIALIZER;

void SdInitUser( void )
{
    memset( &userindsd, 0x00, sizeof( ENCBUFIND ) );
}

void ClearH264Buf( void )
{
    mainLock();
    encbufm.firstflag = 0x00;
    encbufm.firstflag = 0x00;
    encbufm.prelen = 0x00;
    encbufm.preindex = 0x00;
    encbufm.totallen = 0x00;
    mainUnlock();
}

void SetMainStreamFlag( unsigned char flag )
{
    int  i = 0;

    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        userindm[i].flag = flag;
        userindp2p[i].flag = flag;
    }

    userindsd.flag = flag;
    userindonvif.flag = flag;
}

void InitUserIndicator( char streamid, int i )
{
    switch ( streamid )
    {
        case 0:
            memset( &userindm[i], 0x00, sizeof( ENCBUFIND ) );
            break;

        case 1:
            break;

        case 2:
            break;

        case 3:
            memset( &userindj[i], 0x00, sizeof( ENCBUFIND ) );
            break;

        case 4:
            //memset( &userindr[i], 0x00, sizeof( ENCBURECORD ) );
            break;
        default:
            printf("Unkown streamid=%d\n", streamid);
            break;
    }
}
//set user indicator
void SetUserIndicator( char streamid, int usit )
{
    mainLock();

    switch ( streamid )
    {
        case 0:		//main bitrate
            userindm[usit].flag = 0x01;
            userindm[usit].offset = encbufm.prelen;
            break;

        case 1:		//sub bitrate
            userinds[usit].flag = 0x01;
            userinds[usit].offset = encbufs.prelen;        
            break;

        case 2:		//main jpeg
            break;

        case 3:		//sub jpeg
            userindj[usit].index = encbufj.preindex;
            userindj[usit].offset = encbufj.prelen;
            break;

        case 4:		//record play
            break;

        default:
            printf("Unkown streamid=%d\n", streamid);
            break;
    }

    mainUnlock();
}

//void PushVMencData( unsigned char* pbuf, char type, char size, int len, unsigned int frameno )
void PushVMencData( unsigned char* pbuf, char streamid, char type, char size, int len, unsigned int frameno )
{
    LIVEHEAD        	livehead;       //livestream head
    struct timeval  	tv;
    struct timezone 	tz;
    char				flag = 0x00;

    mainLock();

    //check buffer is greater buffer max
    if ( encbufm.firstflag == 0x01 )
    {
        if ( encbufm.totallen + len + sizeof( LIVEHEAD )  > EBUF_MAX_MLEN )
        {
            printf( "len > maxlen total %d len %d\n", encbufm.totallen, len );
            flag = 0x01;
        }
    }

    else
    {
        if ( encbufm.totallen + len + sizeof( LIVEHEAD )  > EBUF_MAX_MLEN / 2 )
        {
            printf( "len > maxlen/2\n" );
            flag = 0x01;
        }
    }

    if ( ( type == 0x00 ) || ( flag == 0x01 ) )
    {
        if ( encbufm.firstflag == 0x01 )
        {
            encbufm.firstindex	= encbufm.index;
            encbufm.index     	= 0x00;
            encbufm.totallen  	= 0x00;
            encbufm.firstflag 	= 0x00;
            SetMainStreamFlag( 0x01 );
        }

        else
        {
            encbufm.firstindex	= encbufm.index;
            encbufm.index     	= 0x00;
            encbufm.totallen  	= EBUF_MAX_MLEN / 2;
            encbufm.firstflag 	= 0x01;
            SetMainStreamFlag( 0x02 );
        }

        //printf("jpeg first flag=%d\n",encbufj.index);
    }

    gettimeofday( &tv, &tz );
    livehead.type           = type;
    livehead.len            = len;
    livehead.streamid       = 0;
    livehead.frameno        = frameno;
    livehead.militime       = tv.tv_usec / 1000;
    livehead.sectime        = tv.tv_sec;
    livehead.startcode		= 0xa815aa55;
    livehead.size     = size;

    livehead.framerate = GetMainFrameRate();

    //push data
    memcpy( (char*)pbufm + encbufm.totallen, (char*)&livehead, sizeof( LIVEHEAD ) ); //livestream head
    memcpy( (char*)pbufm + encbufm.totallen + sizeof( LIVEHEAD ), (char*)pbuf, len ); //media data

    encbufm.prelen = encbufm.totallen;
    encbufm.preindex = encbufm.index;
    encbufm.totallen += len + sizeof( LIVEHEAD );
    encbufm.index++;
    encbufm.flag = 0x01;

    //printf("vbuf->frameno=%d, type=%d, len=%d, prelen=%d\n", frameno, type, len, encbufm.prelen);

    mainUnlock();
}

//=================================================================
//extern api
void mainLock( void )
{
    pthread_mutex_lock( &bufmutextm );
}

void mainUnlock( void )
{
    pthread_mutex_unlock( &bufmutextm );
}

void EncMBbufInit( void )
{
    memset( &encbufm, 0x00, sizeof( ENCBUF ) );
    pbufm = malloc( EBUF_MAX_MLEN );

    if ( pbufm == NULL )
    {
        printf( "malloc audio buf failed============\n" );
    }

    encbufm.pbuf = (unsigned char*)pbufm;
}


