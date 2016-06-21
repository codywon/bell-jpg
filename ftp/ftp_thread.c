#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "cmdhead.h"

int GetStrParamValue( const char* pszSrc, const char* pszParamName, char* pszParamValue )
{
    const char* pos1, *pos = pszSrc;
    unsigned char       len = 0;

    if ( !pszSrc || !pszParamName )
    {
        return -1;
    }

    pos1 = strstr( pos, pszParamName );

    if ( !pos1 )
    {
        return -1;
    }

    pos = pos1 + strlen( pszParamName ) + 1;
    pos1 = strstr( pos, "&" );

    if ( pos1 )
    {
        memcpy( pszParamValue, pos, pos1 - pos );
        len = pos1 - pos;
    }

    else
    {
        pos1 = strstr( pos, " " );

        if ( pos1 != NULL )
        {
            memcpy( pszParamValue, pos, pos1 - pos );
            len = pos1 - pos;
        }
    }

    return 0;
}

int GetIntParamValue( const char* pszSrc, const char* pszParamName, int* iValue )
{
    char        szParamValue[256];
    const char*  pos1, *pos = pszSrc;
    *iValue = -1;
    memset( szParamValue, 0, sizeof( szParamValue ) );

    if ( !pszSrc || !pszParamName )
    {
        return -1;
    }

    pos1 = strstr( pos, pszParamName );

    if ( !pos1 )
    {
        return -1;
    }

    pos = pos1 + strlen( pszParamName ) + 1;
    pos1 = strstr( pos, "&" );

    if ( pos1 )
    {
        memcpy( szParamValue, pos, pos1 - pos );
    }

    else
    {
        pos1 = strstr( pos, " " );

        if ( pos1 )
        {
            //printf("len %d\n",pos1-pos);
            memcpy( szParamValue, pos, pos1 - pos );
        }

        else
        {
            return -1;
        }
    }

    *iValue = atoi( szParamValue );
    return 0;
}

int main( void )
{
    int iRet;
    IPCInit();
    IPCThread();

    while ( 1 )
    {
        sleep( 1 );
    }

    return 0;
}

