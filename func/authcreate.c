#include <stdio.h>
#include <string.h>
//#include "base64.h"
#include "md5.h"
#define AUTH_LEN 1024
/*
 * Name: base64decode
 *
 * Description: Decodes BASE-64 encoded string
 *
 * Used by auth.c
 */
int base64decode( void* dst, char* src, int maxlen )
{
    int bitval, bits;
    int val;
    int len, x, y;
    len = strlen( src );
    bitval = 0;
    bits = 0;
    y = 0;

    for ( x = 0; x < len; x++ )
    {
        if ( ( src[x] >= 'A' ) && ( src[x] <= 'Z' ) )
        {
            val = src[x] - 'A';
        }

        else if ( ( src[x] >= 'a' ) && ( src[x] <= 'z' ) )
        {
            val = src[x] - 'a' + 26;
        }

        else if ( ( src[x] >= '0' ) && ( src[x] <= '9' ) )
        {
            val = src[x] - '0' + 52;
        }

        else if ( src[x] == '+' )
        {
            val = 62;
        }

        else if ( src[x] == '-' )
        {
            val = 63;
        }

        else
        {
            val = -1;
        }

        if ( val >= 0 )
        {
            bitval = bitval << 6;
            bitval += val;
            bits += 6;

            while ( bits >= 8 )
            {
                if ( y < maxlen )
                {
                    ( ( char* )dst )[y++] = ( bitval >> ( bits - 8 ) ) & 0xFF;
                }

                bits -= 8;
                bitval &= ( 1 << bits ) - 1;
            }
        }
    }

    if ( y < maxlen )
    {
        ( ( char* )dst )[y++] = 0;
    }

    return y;
}


static char base64chars[64] = "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

/*
 * Name: base64encode()
 *
 * Description: Encodes a buffer using BASE64.
 *
 * Used by auth.c
 */
void base64encode( unsigned char* from, char* to, int len )
{
    while ( len )
    {
        unsigned long k;
        int c;
        c = ( len < 3 ) ? len : 3;
        k = 0;
        len -= c;

        while ( c-- )
        {
            k = ( k << 8 ) | *from++;
        }

        *to++ = base64chars[( k >> 18 ) & 0x3f ];
        *to++ = base64chars[( k >> 12 ) & 0x3f ];
        *to++ = base64chars[( k >> 6 ) & 0x3f ];
        *to++ = base64chars[ k & 0x3f ];
    }

    *to++ = 0;
}
#if 0
int main( int argc, char* argv[] )
{
    char auth[AUTH_LEN] = {0, };
    get_authfile( "admin", "admin", auth );
    printf( "%s\n", auth );
    return 0;
}
#endif
#if 0
void get_authfile( char* user, char* pass, char* auth )
{
    struct MD5Context mc;
    unsigned char final[16];
    char encoded_passwd[0x40];
    MD5Init( &mc );
    MD5Update( &mc, ( unsigned char* )pass, strlen( pass ) );
    MD5Final( final, &mc );
    strcpy( encoded_passwd, "$1$" );
    base64encode( final, encoded_passwd + 3, 16 );
    snprintf( auth, AUTH_LEN, "%s:%s\n", user, encoded_passwd );
}
#endif
