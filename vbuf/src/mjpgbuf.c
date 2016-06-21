
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "param.h"
#include "protocol.h"
#include "init.h"

#include "mjpgbuf.h"


ENCBUF          encbufj;
const unsigned char* pbufj = NULL;
ENCBUFIND       	userindj[MAX_CONN_USER_NUM];

pthread_mutex_t bufmutextj = PTHREAD_MUTEX_INITIALIZER;


//void PushVJencData( unsigned char* pbuf, char type, char size, int len, unsigned int frameno )
void PushVJencData( unsigned char* pbuf, char streamid, char type, char size, int len, unsigned int frameno )
{
    LIVEHEAD        livehead;       //livestream head
    struct timeval  tv;
    struct timezone tz;

    mjpglock();

    //check
    if ( encbufj.totallen + len + sizeof( LIVEHEAD )  > EBUF_MAX_JLEN )
    {
        encbufj.firstindex	= encbufj.index;
        encbufj.index     	= 0x00;
        encbufj.totallen  	= 0x00;
        encbufj.firstflag 	= 0x01;
    }

    gettimeofday( &tv, &tz );
    livehead.type           	= type;
    livehead.len            	= len;
    livehead.streamid       	= 0x03;
    livehead.frameno        	= frameno;
    livehead.militime       	= tv.tv_usec / 1000;
    livehead.sectime          = tv.tv_sec;
    livehead.startcode	       = STARTCODE;
    livehead.size			= size;
    //push data
    memcpy( (char*)(pbufj + encbufj.totallen), &livehead, sizeof( LIVEHEAD ) ); //livestream head
    memcpy( (char*)(pbufj + encbufj.totallen + sizeof( LIVEHEAD )), pbuf, len ); //media data

    encbufj.prelen = encbufj.totallen;
    encbufj.preindex = encbufj.index;
    encbufj.totallen += len + sizeof( LIVEHEAD );
    encbufj.index++;
    
    mjpgunlock();
}

void mjpglock( void )
{
    pthread_mutex_lock( &bufmutextj );
}
void mjpgunlock( void )
{
    pthread_mutex_unlock( &bufmutextj );
}

void EncJBufInit( void )
{
    memset( &encbufj, 0x00, sizeof( ENCBUF ) );

    pbufj = malloc( EBUF_MAX_JLEN );
    if ( pbufj == NULL )
    {
        printf( "malloc jpeg buf failed=============\n" );
    }

    encbufj.pbuf = (unsigned char*)pbufj;
}


