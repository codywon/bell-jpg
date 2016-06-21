#include "packet.h"

#include <netinet/in.h>
#include <stdio.h>

int Packet( char* pszBuffer, unsigned short nType, unsigned short nSessionID, unsigned short nMessageLen )
{
    PMESSAGE_HEAD pstMessageHead;

    if ( !pszBuffer )
    {
        return 0;
    }

    pstMessageHead = ( PMESSAGE_HEAD )pszBuffer;
    pstMessageHead->byVersion	= MESSAGE_VERSION;
    pstMessageHead->byHeadLen	= MESSAGE_VERSION;
    pstMessageHead->nType		= nType;
    pstMessageHead->nSessionID	= nSessionID;
    pstMessageHead->nMessageLen	= nMessageLen;
    return nMessageLen + sizeof( MESSAGE_HEAD );
}

const char* Unpacket( const char* pszBuffer, int iBufferSize, unsigned short* nType, unsigned short* nSessionID, unsigned short* nMessageLen )
{
    PMESSAGE_HEAD 	pstMessageHead;

    if ( !pszBuffer )
    {
        return NULL;
    }

    pstMessageHead = ( PMESSAGE_HEAD )pszBuffer;

    if ( pstMessageHead->byVersion != MESSAGE_VERSION ||
         pstMessageHead->byHeadLen != MESSAGE_VERSION ||
         ( int )( pstMessageHead->nMessageLen + sizeof( MESSAGE_HEAD ) ) > iBufferSize )
    {
        *nType		= 0;
        *nSessionID	= 0;
        *nMessageLen	= 0;
        return NULL;
    }

    *nType			= pstMessageHead->nType;
    *nSessionID		= pstMessageHead->nSessionID;
    *nMessageLen	= pstMessageHead->nMessageLen;
    return pszBuffer + sizeof( MESSAGE_HEAD );
}
