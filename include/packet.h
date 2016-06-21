#ifndef __PACKET_H__
#define __PACKET_H__

#define MESSAGE_VERSION		0x10

#define MAX_RECV_BUFFER_SIZE	4096

typedef struct _stMessageHead
{
    unsigned char	byVersion;
    unsigned char	byHeadLen;
    unsigned short	nType;

    unsigned short	nSessionID;
    unsigned short	nMessageLen;

} MESSAGE_HEAD, *PMESSAGE_HEAD;

int Packet( char* pszBuffer, unsigned short nType, unsigned short nSessionID, unsigned short nMessageLen );
const char* Unpacket( const char* pszBuffer, int iBufferSize, unsigned short* nType, unsigned short* nSessionID, unsigned short* nMessageLen );

#endif //__PACKET_H__
