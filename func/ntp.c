#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.h"

#define NTP_SERVER    	"time.nist.gov"
#define NTP_PORT      	123

#define JAN_1970     	0x83aa7e80

#define NTPFRAC(x) 		(4294 * (x) + ((1981 * (x))>>11))
#define USEC(x) 		(((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))

#define Data(i) ntohl(((unsigned int *)data)[i])

#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC -6

struct ntptime
{
    unsigned int coarse;
    unsigned int fine;
};

void send_packet( int fd )
{
    unsigned int data[12];
    struct timeval now;
    int ret;
    unsigned char* pdst = NULL;

    if ( sizeof( data ) != 48 )
    {
        fprintf( stderr, "size error\n" );
        return;
    }

    memset( ( char* )data, 0, sizeof( data ) );
    data[0] = htonl( ( LI << 30 ) | ( VN << 27 ) | ( MODE << 24 )
                     | ( STRATUM << 16 ) | ( POLL << 8 ) | ( PREC & 0xff ) );
    data[1] = htonl( 1 << 16 ); /* Root Delay (seconds) */
    data[2] = htonl( 1 << 16 ); /* Root Dispersion (seconds) */
    gettimeofday( &now, NULL );
    data[10] = htonl( now.tv_sec + JAN_1970 ); /* Transmit Timestamp coarse */
    data[11] = htonl( NTPFRAC( now.tv_usec ) ); /* Transmit Timestamp fine   */
    ret = 0;
    pdst = ( unsigned char* )data;

    while ( 1 )
    {
        int len;
        len = send( fd, pdst + ret, 48, 0 );

        if ( ret == -1 )
        {
            printf( "send failed\n" );
            break;
        }

        ret += len;
        pdst += ret;

        if ( ret >= 48 )
        {
            break;
        }
    }
}

void get_udp_arrival_local_timestamp( struct ntptime* udp_arrival_local_timestamp )
{
    struct timeval udp_arrival_local_time;
    gettimeofday( &udp_arrival_local_time, NULL );
    //print just precise to second
    Textout( "local time=>%s\n", ctime( &( udp_arrival_local_time.tv_sec ) ) );
}

void get_new_time( unsigned int* data, struct timeval* new_time )
{
    //int li, vn, mode, stratum, poll, prec;
    //int delay, disp, refid;
    //struct ntptime reftime, orgtime, rectime;
    struct ntptime trantime;
    /*li      = Data(0) >> 30 & 0x03;
    vn      = Data(0) >> 27 & 0x07;
    mode    = Data(0) >> 24 & 0x07;
    stratum = Data(0) >> 16 & 0xff;
    poll    = Data(0) >>  8 & 0xff;
    prec    = Data(0)       & 0xff;
    if (prec & 0x80) prec|=0xffffff00;
    delay   = Data(1);
    disp    = Data(2);
    refid   = Data(3);
    reftime.coarse = Data(4);
    reftime.fine   = Data(5);
    orgtime.coarse = Data(6);
    orgtime.fine   = Data(7);
    rectime.coarse = Data(8);
    rectime.fine   = Data(9);*/
    trantime.coarse = Data( 10 );
    trantime.fine   = Data( 11 );

    new_time->tv_sec = trantime.coarse - JAN_1970;
    
    new_time->tv_usec = USEC( trantime.fine );
    //print just precise to second
    Textout( "server time=>%s\n", ctime( &( new_time->tv_sec ) ) );
}

void set_local_time( struct timeval new_time )
{
    /* need root user. */
    if ( 0 != getuid() && 0 != geteuid() )
    {
        Textout( "must be root user!:0)\n" );
        return;
    }

    Textout( "Set NEW Time" );
    settimeofday( &new_time, NULL );
}

int NtpRun( char* NtpServer )
{
    int sockfd;
    struct sockaddr_in addr_src, addr_dst;
    fd_set fds_read;
    int ret;
    int receivebytes;
    unsigned int buf[12];
    int addr_len;
    struct timeval timeout, new_time;
    struct ntptime udp_arrival_local_timestamp;
    struct hostent* host;
    int	i;
    
    addr_len = sizeof( struct sockaddr_in );
    memset( &addr_src, 0, addr_len );
    addr_src.sin_family = AF_INET;
    addr_src.sin_addr.s_addr = htonl( INADDR_ANY );
    addr_src.sin_port = htons( 0 );
    
    memset( &addr_dst, 0, addr_len );
    addr_dst.sin_family = AF_INET;

    dnslock();
    host = gethostbyname( NtpServer );
    dnsunlock();
    if ( host == NULL )
    {
        //Textout("Get NTP host fail");        
        return -1;
    }

    memcpy( &( addr_dst.sin_addr.s_addr ), host->h_addr_list[0], 4 );
    addr_dst.sin_port = htons( NTP_PORT );

    while ( 1 )
    {
        sleep( 10 );
        if ( -1 == ( sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) )
        {
            Textout( "ntp create socket error" );
            continue;
        }

        ret = bind( sockfd, ( struct sockaddr* )&addr_src, addr_len );

        if ( -1 == ret )
        {
            Textout( "ntp bind error" );
            close( sockfd );
            continue;
        }

        ret = connect( sockfd, ( struct sockaddr* )&addr_dst, addr_len );

        if ( -1 == ret )
        {
            Textout( "ntp connect error" );
            close( sockfd );
            continue;
        }

        send_packet( sockfd );
        FD_ZERO( &fds_read );
        FD_SET( sockfd, &fds_read );
        timeout.tv_sec = 6;
        timeout.tv_usec = 0;
        ret = select( sockfd + 1, &fds_read, NULL, NULL, &timeout );

        if ( 0 == ret || !FD_ISSET( sockfd, &fds_read ) )
        {
            close( sockfd );
            continue;
        }

        receivebytes = recvfrom( sockfd, buf, sizeof( buf ), 0, ( struct sockaddr* )&addr_dst, &addr_len );

        if ( -1 == receivebytes )
        {
            Textout( "========NTP recvfrom error===========" );
            close( sockfd );
            continue;
        }

        get_udp_arrival_local_timestamp( &udp_arrival_local_timestamp );
        get_new_time( buf, &new_time );
        set_local_time( new_time );
        break;
    }

    close( sockfd );
    return 0;
}


