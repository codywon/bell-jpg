
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "param.h"
#include "protocol.h"
#include "init.h"

#include "abuf.h"

const unsigned char* pbufa = NULL;

ENCBUF          encbufa;

pthread_mutex_t bufmutexta = PTHREAD_MUTEX_INITIALIZER;

void audiolock( void )
{
    pthread_mutex_lock( &bufmutexta );
}

void audiounlock( void )
{
    pthread_mutex_unlock( &bufmutexta );
}

void PushAencData( unsigned char* pbuf, int len, unsigned int frameno )
{
    LIVEHEAD        livehead;
    struct timeval  tv;
    struct timezone tz;

    audiolock();

    if ( ( encbufa.totallen + len + sizeof( LIVEHEAD )  > EBUF_MAX_ALEN ) || ( encbufa.index >= MAX_FRAME_CNT ) )
    {
        encbufa.firstindex = encbufa.index;
        encbufa.index     = 0x00;
        encbufa.totallen  = 0x00;
        encbufa.firstflag = 0x01;
    }

    gettimeofday( &tv, &tz );
    livehead.type           = 0x06;
    livehead.len            = len;
    livehead.streamid       = 0x01;
    livehead.frameno        = frameno;
    livehead.militime       = tv.tv_usec / 1000;
    livehead.sectime        = tv.tv_sec;
    livehead.startcode  = STARTCODE;

    /* BEGIN: Added by wupm, 2013/5/29 */
    livehead.adpcmindex = GetEncoderAudioIndex();       //g_nEnAudioIndex;
    livehead.adpcmsample = GetEncoderAduioPreSample();  //g_nEnAudioPreSample;


    //push data
    memcpy( (char*)(pbufa + encbufa.totallen), &livehead, sizeof( LIVEHEAD ) ); //livestream head
    memcpy( (char*)(pbufa + encbufa.totallen + sizeof( LIVEHEAD )), pbuf, len ); //media data head
    encbufa.prelen = encbufa.totallen;
    encbufa.preindex = encbufa.index;
    encbufa.totallen += len + sizeof( LIVEHEAD );
    encbufa.index++;
    //SetAudioIndex( encbufa.preindex );
    audiounlock();
}

void EncABufInit( void )
{
    memset( &encbufa, 0x00, sizeof( ENCBUF ) );
    //audio main buffer
    pbufa = malloc( EBUF_ASIZE * 4 );

    if ( pbufa == NULL )
    {
        printf( "malloc audio buf failed============\n" );
    }

    encbufa.pbuf = (unsigned char*)pbufa;
}


