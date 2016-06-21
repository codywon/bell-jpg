#include "hd_common.h"
#include "rs.h"
#include "alarm.h"
#include "param.h"

#include <sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include<termios.h>


struct termios oldtio;
int	shandle = -1;
int     chandle = -1;

int set_opt( int fd, int nSpeed, int nBits, char nEvent, int nStop )
{
    struct termios newtio;

    if ( tcgetattr( fd, &oldtio ) != 0 )
    {
        perror( "SetupSerial 1" );
        return -1;
    }

    memset( &newtio, 0, sizeof( struct termios ) );
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch ( nBits )
    {
        case 7:
            newtio.c_cflag |= CS7;
            break;

        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    switch ( nEvent )
    {
        case 'O':
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= ( INPCK | ISTRIP );
            break;

        case 'E':
            newtio.c_iflag |= ( INPCK | ISTRIP );
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;

        case 'N':
            newtio.c_cflag &= ~PARENB;
            break;
    }

    newtio.c_cflag &= ~CRTSCTS;

    switch ( nSpeed )
    {
        case 1200:
            cfsetispeed( &newtio, B1200 );
            cfsetospeed( &newtio, B1200 );
            break;

        case 2400:
            cfsetispeed( &newtio, B2400 );
            cfsetospeed( &newtio, B2400 );
            break;

        case 4800:
            cfsetispeed( &newtio, B4800 );
            cfsetospeed( &newtio, B4800 );
            break;

        case 9600:
            cfsetispeed( &newtio, B9600 );
            cfsetospeed( &newtio, B9600 );
            break;

        case 115200:
            cfsetispeed( &newtio, B115200 );
            cfsetospeed( &newtio, B115200 );
            break;

        default:
            cfsetispeed( &newtio, B9600 );
            cfsetospeed( &newtio, B9600 );
            break;
    }

    if ( nStop == 1 )
    {
        newtio.c_cflag &= ~CSTOPB;
    }

    else if ( nStop == 2 )
    {
        newtio.c_cflag |= CSTOPB;
    }

    newtio.c_cc[VTIME] = 150;//0;
    newtio.c_cc[VMIN] = 0;
    tcflush( fd, TCIFLUSH );

    if ( ( tcsetattr( fd, TCSANOW, &newtio ) ) != 0 )
    {
        perror( "com set error" );
        return -1;
    }

    printf( "set done!\n" );
    return 0;
}

void MotoSerialCmd( unsigned char* pbuf, int len )
{
    int iRetLen = 0;

    if ( shandle != -1 )
    {
        iRetLen = write( shandle, pbuf, len );
    }

    else
    {
        printf( "moto comm is failed\n" );
    }
}
int MotoSerialRcv( unsigned char* pbuf, int len )
{
    int iRet;
    iRet = read( shandle, pbuf, len );

    if ( iRet <= 0 )
    {
        printf( "recive failed=%x\n", iRet );
    }

    else
    {
        printf( "recive %d\n", iRet );
    }
}
void* RS232SendThread( void* param )
{
    char				szBuffer[64];
    int 				iRetLen;
    unsigned short   	ptztime;
    int				i;
    int				times = 0;
    int 		handle = ( int )param;

    if ( handle == -1 )
    {
        printf( "RS232RecvThread() handle = -1 errno %d\n", errno );
        return NULL;
    }

    for ( i = 0; i < 64; i++ )
    {
        szBuffer[i] = i;
    }

    while ( 1 )
    {
        sleep( 1 );
        MotoSerialCmd( szBuffer, 64 );
        printf( "send serial data=%d\n", times++ );
    }

    return NULL;
}

void* RS232RecvThread( void* param )
{
    char				szBuffer[64];
    int 				iRet;
    unsigned short   	ptztime;
    int				i;
    int				times = 0;
    int 		handle = ( int )param;

    if ( handle == -1 )
    {
        printf( "RS232RecvThread() handle = -1 errno %d\n", errno );
        return NULL;
    }

    while ( 1 )
    {
        iRet = MotoSerialRcv( szBuffer, 64 );
        printf( "recive serial data=%d\n", iRet );
    }

    return NULL;
}
int MotoSerialInit()
{
    pthread_t   threadID;
    printf( "===================\n" );
    shandle = open( "/dev/ttyS0", O_RDWR );

    if ( shandle != -1 )
    {
        if ( fcntl( shandle, F_SETFL, 0 ) < 0 )
        {
            printf( "fcntl failed!\n" );
        }

        if ( set_opt( shandle, 9600, 8, 'N', 1 ) < 0 )
        {
            printf( "set_opt error\n" );
            return 0;
        }
    }

    else
    {
        printf( "open file error = %d\n", errno );
    }

    printf( "===================%x\n", shandle );
    return 0;
}

void MotoSerialThread( void ) 	//date check thread is start
{
    pthread_t 	rcvthread;
    pthread_t 	sendthread;
//	pthread_create(&rcvthread, 0,&RS232RecvThread, NULL);
//	pthread_create(&sendthread, 0,&RS232SendThread, NULL);
}

