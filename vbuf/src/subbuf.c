
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "param.h"
#include "protocol.h"
#include "init.h"

#include "subbuf.h"

const unsigned char* pbufs = NULL;

ENCBUF              encbufs;
ENCBUFIND       	userinds[MAX_CONN_USER_NUM];
ENCBUFIND       	userindsubp2p[MAX_CONN_USER_NUM];

pthread_mutex_t     bufmutexts = PTHREAD_MUTEX_INITIALIZER;

void ClearH264SubBuf( void )
{
    subLock();
    encbufs.firstflag = 0x00;
    encbufs.firstflag = 0x00;
    encbufs.prelen = 0x00;
    encbufs.preindex = 0x00;
    encbufs.totallen = 0x00;
    subUnlock();
}

void SetSubStreamFlag( char flag )
{
    int i=0;

    for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
    {
        userinds[i].flag = flag;
        userindsubp2p[i].flag = flag;
    }
}

void PushVSencData( unsigned char* pbuf, char type, char size, int len, unsigned int frameno )
{  
    LIVEHEAD        	livehead;       //livestream head
    struct timeval  	tv;
    struct timezone 	tz;
    char				flag = 0x00;

    subLock();
    
    if ( encbufs.firstflag == 0x01 )
    {
        if ( encbufs.totallen + len + sizeof( LIVEHEAD )  > EBUF_MAX_SLEN )
        {
            printf( "sub len > maxlen total %d len %d\n", encbufs.totallen, len );
            flag = 0x01;
        }
    }

    else
    {
        if ( encbufs.totallen + len + sizeof( LIVEHEAD )  > EBUF_MAX_SLEN / 2 )
        {
            printf( "sub len > maxlen/2\n" );
            flag = 0x01;
        }
    }

    if ( ( type == 0x00 ) || ( flag == 0x01 ) )
    {
        if ( encbufs.firstflag == 0x01 )
        {
            encbufs.firstindex	= encbufs.index;
            encbufs.index     	= 0x00;
            encbufs.totallen  	= 0x00;
            encbufs.firstflag 	= 0x00;
            SetSubStreamFlag( 0x01 );
        }

        else
        {
            encbufs.firstindex	= encbufs.index;
            encbufs.index     	= 0x00;
            encbufs.totallen  	= EBUF_MAX_SLEN / 2;
            encbufs.firstflag 	= 0x01;
            SetSubStreamFlag( 0x02 );
        }
    }

    gettimeofday( &tv, &tz );
    livehead.type             = type;
    livehead.len               = len;
    livehead.streamid       = 0;
    livehead.frameno        = frameno;
    livehead.militime         = tv.tv_usec / 1000;
		
    livehead.sectime         = tv.tv_sec;
    livehead.startcode		= STARTCODE;
    livehead.size			= size;

    livehead.framerate = GetSubFrameRate();
    //push data
    memcpy( (char*)(pbufs + encbufs.totallen), &livehead, sizeof( LIVEHEAD ) ); //livestream head
    memcpy( (char*)(pbufs + encbufs.totallen + sizeof( LIVEHEAD )), pbuf, len ); //media data

    encbufs.prelen = encbufs.totallen;
    encbufs.preindex = encbufs.index;
    encbufs.totallen += len + sizeof( LIVEHEAD );
    encbufs.index++;

    subUnlock();
}



//=================================================================
//extern api
void subLock( void )
{
    pthread_mutex_lock( &bufmutexts );
}

void subUnlock( void )
{
    pthread_mutex_unlock( &bufmutexts );
}

void EncSBbufInit( void )
{
    memset( &encbufs, 0x00, sizeof( ENCBUF ) );
    pbufs = malloc( EBUF_MAX_SLEN );

    if ( pbufs == NULL )
    {
        printf( "malloc sub buf failed============\n" );
    }

    encbufs.pbuf = (unsigned char*)pbufs;
}


