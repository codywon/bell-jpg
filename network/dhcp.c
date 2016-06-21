#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <unistd.h>
#include <sys/time.h>

#include "cmdhead.h"
#include "param.h"
#include "network.h"


//init dhcp config
void DhcpServerConfig( void )
{
    FILE*    fp = NULL;
    char    temp[256];
    fp = fopen( "/tmp/udhcpd.conf", "wb" );

    if ( fp == NULL )
    {
        printf( "Wifi config failed\n" );
        return;
    }

    memset( temp, 0x00, 256 );
    sprintf( temp, "start %s\n", bparam.stWifiRoute.startip );
    fwrite( temp, 1, strlen( temp ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "end %s\n", bparam.stWifiRoute.endip );
    fwrite( temp, 1, strlen( temp ), fp );
    fwrite( "interface ra0\n", 1, strlen( "interface ra0\n" ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "option subnet %s\n", bparam.stWifiRoute.mask );
    fwrite( temp, 1, strlen( temp ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "option dns  8.8.8.8\n" );
    fwrite( temp, 1, strlen( temp ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "option router  %s\n", bparam.stWifiRoute.ipaddr );
    fwrite( temp, 1, strlen( temp ), fp );
    fwrite( "option lease 864000\n", 1, strlen( "option lease 864000\n" ), fp );
    fclose( fp );
}

void DhcpReleaseFile( void )
{
    FILE*    fp = NULL;
    char    temp[256];
    fp = fopen( "/var/lib/misc/udhcpd.leases", "wb" );

    if ( fp == NULL )
    {
        printf( "Wifi config failed\n" );
        return;
    }

    fclose( fp );
}

//init Dhcp Route
void InitDhcpRoute( void )
{
    DhcpServerConfig();
    DoSystem( "mkdir -p /var/lib/misc" );
    DhcpReleaseFile();
}
//Dhcp Route start
void DhcpRouteStart( void )
{
    DoSystem( "udhcpd -fS /tmp/udhcpd.conf &" );
}

void DhcpRouteStop( void )
{
    DoSystem( "killall udhcpd" );
}
