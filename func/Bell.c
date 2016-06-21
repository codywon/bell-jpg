#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <netinet/tcp.h>

#include "init.h"
#include "alarm.h"
#include "mywatchdog.h"
#include "boa.h"
#include "param.h"
#include "debug.h"

void CloseAp()
{
	Textout("Close ap");

    IpcStaMode();
}
void OpenAp()
{
	Textout("Open ap");

    IpcApMode();
}

/* BEGIN: Added by wupm, 2013/11/1 */
#if	0

/* Type define */
typedef unsigned char byte;
typedef unsigned int uint32;

#define	false FALSE
#define	true  TRUE
//#define	BUFFER_SIZE	1024

	uint32 _state[4];	/* state (ABCD) */
	uint32 _count[2];	/* number of bits, modulo 2^64 (low-order word first) */
	byte _buffer[64];	/* input buffer */
	byte _digest[16];	/* message digest */
	BOOL _finished;		/* calculate finished ? */
	char szMD5Out[64];

	//static const byte PADDING[64];	/* padding for calculate */
	//static const char HEX[16];
	//enum { BUFFER_SIZE = 1024 };

/* Constants for MD5Transform routine. */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* F, G, H and I are basic MD5 functions.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
	(a) += F ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
	(a) += G ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
	(a) += H ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
	(a) += I ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}

const byte PADDING[64] = { 0x80 };
const char HEX[16] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'a', 'b',
	'c', 'd', 'e', 'f'
};


void MD5_encode(const uint32* input, byte* output, int length) ;
void MD5_decode(const byte* input, uint32* output, int length) ;
char *MD5_bytesToHexString(const byte* input, int length) ;
void MD5_transform(const byte block[64]) ;

/* Reset the calculate state */
void MD5_reset() {

	_finished = false;
	/* reset number of bits. */
	_count[0] = _count[1] = 0;
	/* Load magic initialization constants. */
	_state[0] = 0x67452301;
	_state[1] = 0xefcdab89;
	_state[2] = 0x98badcfe;
	_state[3] = 0x10325476;

	memset(szMD5Out, 0, 64);
}

/* MD5 block update operation. Continues an MD5 message-digest
operation, processing another message block, and updating the
context.
*/
void MD5_update(const byte* input, int length) {

	uint32 i, index, partLen;

	_finished = false;

	/* Compute number of bytes mod 64 */
	index = (uint32)((_count[0] >> 3) & 0x3f);

	/* update number of bits */
	if ((_count[0] += ((uint32)length << 3)) < ((uint32)length << 3)) {
		++_count[1];
	}
	_count[1] += ((uint32)length >> 29);

	partLen = 64 - index;

	/* transform as many times as possible. */
	if (length >= partLen) {

		memcpy(&_buffer[index], input, partLen);
		MD5_transform(_buffer);

		for (i = partLen; i + 63 < length; i += 64) {
			MD5_transform(&input[i]);
		}
		index = 0;

	} else {
		i = 0;
	}

	/* Buffer remaining input */
	memcpy(&_buffer[index], &input[i], length - i);
}

/* MD5 finalization. Ends an MD5 message-_digest operation, writing the
the message _digest and zeroizing the context.
*/
void MD5_final() {

	byte bits[8];
	uint32 oldState[4];
	uint32 oldCount[2];
	uint32 index, padLen;

	/* Save current state and count. */
	memcpy(oldState, _state, 16);
	memcpy(oldCount, _count, 8);

	/* Save number of bits */
	MD5_encode(_count, bits, 8);

	/* Pad out to 56 mod 64. */
	index = (uint32)((_count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	MD5_update(PADDING, padLen);

	/* Append length (before padding) */
	MD5_update(bits, 8);

	/* Store state in digest */
	MD5_encode(_state, _digest, 16);

	/* Restore current state and count. */
	memcpy(_state, oldState, 16);
	memcpy(_count, oldCount, 8);
}

/* MD5 basic transformation. Transforms _state based on block. */
void MD5_transform(const byte block[64]) {

	uint32 a = _state[0], b = _state[1], c = _state[2], d = _state[3], x[16];

	MD5_decode(block, x, 64);

	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	_state[0] += a;
	_state[1] += b;
	_state[2] += c;
	_state[3] += d;
}

/* Encodes input (ulong) into output (byte). Assumes length is
a multiple of 4.
*/
void MD5_encode(const uint32* input, byte* output, int length) {

	int i,j;
	for (i = 0, j = 0; j < length; ++i, j += 4) {
		output[j]= (byte)(input[i] & 0xff);
		output[j + 1] = (byte)((input[i] >> 8) & 0xff);
		output[j + 2] = (byte)((input[i] >> 16) & 0xff);
		output[j + 3] = (byte)((input[i] >> 24) & 0xff);
	}
}

/* Decodes input (byte) into output (ulong). Assumes length is
a multiple of 4.
*/
void MD5_decode(const byte* input, uint32* output, int length) {

	int i,j;
	for (i = 0, j = 0; j < length; ++i, j += 4) {
		output[i] = ((uint32)input[j]) | (((uint32)input[j + 1]) << 8) |
		(((uint32)input[j + 2]) << 16) | (((uint32)input[j + 3]) << 24);
	}
}

/* Convert byte array to hex string. */
char *MD5_bytesToHexString(const byte* input, int length) {

	//string str;
	//str.reserve(length << 1);
	int i;
	for (i = 0; i < length; ++i) {
		int t = input[i];
		int a = t / 16;
		int b = t % 16;
		//str.append(1, HEX[a]);
		//str.append(1, HEX[b]);
		szMD5Out[2 * i] = HEX[a];
		szMD5Out[2 * i + 1] = HEX[b];
	}
	//return str;
	return szMD5Out;
}

/* Return the message-digest */
const byte* MD5_digest() {

	if (!_finished) {
		_finished = true;
		MD5_final();
	}
	return _digest;
}

/* Convert digest to string value */

char *MD5_toString() {
	return MD5_bytesToHexString(MD5_digest(), 16);
}


/* BEGIN: Added by wupm, 2013/11/1 */
/////////////////////////////////////////////////////////////////
void OnDoorRequest(char *host_name, int host_port, char *pData, int nDataSize)
{
	/*
	SOCKADDR_IN		saServer;
	LPHOSTENT		lphostent;
	WSADATA			wsadata;
	SOCKET			hsocket;
	*/
	char             		ipaddr[16];
    int                     rsocket = -1;
    struct sockaddr_in      daddr;
    struct timeval          TimeOut;
    int 					nFlag;

	int						nRet;
	//char					tChars[1024];

	/*
	if(WSAStartup(MAKEWORD(1,2),&wsadata))
	{
		MessageBox("can't   initial   socket");
	}
	*/

	/*
	lphostent=gethostbyname(host_name);
	if(lphostent==NULL)
	{
		MessageBox("lphostent   is   null");
	}
	*/

	memset( ipaddr, 0x00, 16 );

/* BEGIN: Modified by wupm, 2013/12/2 */
#if	0
    nRet = GetDnsIp( host_name, ipaddr );
	Textout("Server = [%s], IPADDR = [%s]", host_name, ipaddr);
	ipaddr[0] = 0;
#else
	strcpy(ipaddr, "183.232.25.234");
#endif

	//hsocket   =   socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//init socket
    rsocket = InitSocket( 0, 1, NULL );             //tcp
    nFlag = 1;
    setsockopt( rsocket, IPPROTO_TCP, TCP_NODELAY, ( void* )&nFlag, sizeof( int ) );
    TimeOut.tv_sec = 30;
    TimeOut.tv_usec = 0;
    setsockopt( rsocket, SOL_SOCKET, SO_RCVTIMEO, ( char* )&TimeOut, sizeof( TimeOut ) );
    TimeOut.tv_sec = 30;
    TimeOut.tv_usec = 0;
    setsockopt( rsocket, SOL_SOCKET, SO_SNDTIMEO, ( char* )&TimeOut, sizeof( TimeOut ) );
    bzero( &daddr, sizeof( struct sockaddr_in ) );

	/*
	saServer.sin_family   =   AF_INET;
	saServer.sin_port   =   htons(host_port);
	saServer.sin_addr   =   *((LPIN_ADDR)*lphostent->h_addr_list);
	*/
	daddr.sin_family        = AF_INET;
    daddr.sin_port          = htons(host_port);
    daddr.sin_addr.s_addr   = inet_addr( ipaddr );


	//nRet = connect(hsocket, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN));
	nRet = connect( rsocket, ( struct sockaddr* )&daddr, sizeof( struct sockaddr ) );
	if(nRet == -1)	//SOCKET_ERROR)
	{
		/*
		MessageBox("Can't connect");
		closesocket(hsocket);
		//WSACleanup();
		*/
		Textout( "Can't connect");
        CloseSocket( rsocket );
		return;
	}
	else
	{
		//MessageBox("connected   with host");
		Textout("connected   with host");
	}

	//nRet = send(hsocket, pData, nDataSize, 0);
	nRet = send(rsocket, pData, nDataSize, 0);
	if(nRet == -1)	//SOCKET_ERROR)
	{
		/*
		MessageBox("send()   failed");
		closesocket(hsocket);
		//WSACleanup();
		*/
		Textout( "Can't Send");
        CloseSocket( rsocket );
		return;
	}
	else
	{
		//MessageBox("send() OK");
		Textout("send() OK");
	}

	char   dest[1024];
	nRet=0;
	if(1)
	{
		//nRet=recv(hsocket,(LPSTR)dest,sizeof(dest),0);
		nRet=recv(rsocket,(char *)dest,sizeof(dest),0);
		if(nRet>0)

		{
			dest[nRet]=0;

		}
		else
		{

			dest[0]=0;
		}


		//sprintf(tChars,"Received bytes:%d",nRet);
		//MessageBox(tChars);
		Textout("Received bytes:%d",nRet);
		//sprintf(tChars,"Result:%s",dest);
		//MessageBox(tChars);
		Textout("Result:%s",dest);
	}


	//closesocket(hsocket);
	CloseSocket(rsocket);
	//WSACleanup();
}

void OnDoorPost(char *Connect_String, char *host_name, int host_port, char *pData, int nDataSize)
{
	char request[2048];
	sprintf(request,
		"POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: receiver/Receiver/1.1\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Content-Length: %d\r\n\r\n"
		"%s\r\n\r\n",

		Connect_String,
		host_name,
		nDataSize,
		pData);

	OnDoorRequest(host_name, host_port, request, strlen(request));
}


int GetDoorSendNumber()
{
#if	0
	static int sendno = 1000;
	return ++sendno;
#else
	/*
	FILE *f = fopen("/param/Door.bin", "rb");
	int sendno = 0;
	if ( f != NULL )
	{
		fread(&sendno, 1, sizeof(int), f);
		fclose(f);
	}
	sendno++;
	f = fopen("/param/Door.bin", "wb");
	fwrite(&sendno, 1, sizeof(int), f);
	fclose(f);
	return sendno;
	*/
	//ReadBellData();

	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
	/*
	bell_data.sendid++;
	WriteBellData();
	return bell_data.sendid;
	*/
	return 0;
#endif
}

#define	USE_CUSTOM_MESSAGE

void OnDoorPush(char *uuid, int type, char *send_date, char *pic_file, char *video_file)
{
	#ifdef 	USE_CUSTOM_MESSAGE
	char *Connect_String = "http://api.jpush.cn:8800/v2/custom_message";
	#else
	char *Connect_String = "http://api.jpush.cn:8800/sendmsg/v2/sendmsg";
	#endif

	char *host_name = "api.jpush.cn";
	int host_port = 8800;

	//char *uuid = "OBJ002868GHYYC";

	int sendno = GetDoorSendNumber();
	char *app_key = "f3987aed6c55726410ad643b";
	char *master_secret = "f6bca372d1ba192c4dbe077d";
	int receiver_type = 2;
	//char *platform = "android,ios";
	char *platform = "android";
	int time_to_live = 86400;

	char md5_str[32] = {0};
	//CString szTmp1;
	char szTmp1[1024];
	//szTmp1.Format("%d%d%s%s",
	sprintf(szTmp1, "%d%d%s%s",
		sendno,
		receiver_type,
		uuid,
		master_secret );

	if ( 1 )
	{
		//char szOut[64];
		//MD5 md5;
		MD5_reset();
		MD5_update((const char *)szTmp1, strlen(szTmp1));
		//CString szTmp(md5.toString().c_str());
		//MD5_bytesToHexString((const byte *)szTmp1, strlen(szTmp1), szOut)
		sprintf(md5_str, "%s", MD5_toString());
		Textout("MD5 = [%s]", md5_str);
	}

	char msg_content[1024] = {0};
#ifdef	USE_CUSTOM_MESSAGE
	sprintf(msg_content,"%s,%d,%s,%s,%s,",
#else
	sprintf(msg_content,"\"%s,%d,%s,%s,%s,\"",
#endif
		uuid,
		type,
		send_date,
		pic_file,
		video_file);

	char request[2048] = {0};
	sprintf(request,
		"sendno=%d"
		"&app_key=%s"
		"&receiver_type=%d"
		"&receiver_value=%s"
		"&verification_code=%s"
		"&platform=\"%s\""

/* BEGIN: Modified by wupm, 2013/11/4 */
#ifdef	USE_CUSTOM_MESSAGE
		"&txt=%s",
#else

#if	0
		"&msg_type=1"
		"&msg_content={\"n_content\":%s}",
#else
		"&msg_type=2"
		"&msg_content={\"message\":%s}",
#endif
		//"&msg_content={\"message\":{\"id\":\"OBJ002868GHYYC\",\"type\":\"1\",\"date\":\"1234\"}}",

#endif

		sendno++,
		app_key,
		receiver_type,
		uuid,
		md5_str,
		platform,
		//time_to_live,
		msg_content
		);

	OnDoorPost(Connect_String, host_name, host_port, request, strlen(request));

}
/*
void OnDoorGetTime(char *szDatetime)
{
	int             temptime;
    struct tm*      tmptm = NULL;
	//char szDatetime[64];
	temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
    tmptm = localtime( &temptime );
	memset(szDatetime, 0, 64);
	sprintf( szDatetime, "%04d%02d%02d%02d%02d%02d",
                     tmptm->tm_year + 1900, tmptm->tm_mon + 1, tmptm->tm_mday,
                     tmptm->tm_hour, tmptm->tm_min, tmptm->tm_sec);
}
*/

void TransferUUID(char *oldUUID, char *szUUID)
{
	int len = strlen(oldUUID);
	int i,k;
	for(i=0, k=0;i<len;i++)
	{
		if ( oldUUID[i] == '-' )
		{
			continue;
		}
		if ( oldUUID[i] >= 'a' && oldUUID[i] <= 'z' )
		{
			szUUID[k] = oldUUID[i] + 'A' - 'a';
			k++;
		}
		else
		{
			szUUID[k] = oldUUID[i];
			k++;
		}
	}
	szUUID[k] = 0;
	Textout("TARGET UUID = [%s]", szUUID);
}


void BuildBellString(char *p)
{
	char szUUID[64] = {0};

	int             temptime;
    struct tm*      tmptm = NULL;
	char szDatetime[64];
	temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
    tmptm = localtime( &temptime );
	memset(szDatetime, 0, 64);
	sprintf( szDatetime, "%04d%02d%02d%02d%02d%02d",
                     tmptm->tm_year + 1900, tmptm->tm_mon + 1, tmptm->tm_mday,
                     tmptm->tm_hour, tmptm->tm_min, tmptm->tm_sec);

	if ( bparam.stIEBaseParam.dwDeviceID[0] == 0 )
	{
		Textout("UID is NULL");
		return;
	}

	TransferUUID(bparam.stIEBaseParam.dwDeviceID, szUUID);

	sprintf(p,"%s,%d,%s,%s,%s,",
		szUUID,
		PRESS_BUTTON,
		szDatetime,
		"NO",	//jpgfile,
		"NO"	//videofile
		);
}

void OnDoorCall(int type, char *jpgfile, char *videofile)
{
	char szUUID[64] = {0};
	// TODO: Add your control notification handler code here
#if	0
	OnDoorPush("OBJ002868GHYYC", 1, "20130920123456", "20130920123456.JPG", "20130920123456.H264");
	//OnPush("OBJ002868GHYYC", 2, "20130920123456", "20130920123456.JPG", "NO");
#else

	int             temptime;
    struct tm*      tmptm = NULL;
	char szDatetime[64];
	temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
    tmptm = localtime( &temptime );
	memset(szDatetime, 0, 64);
	sprintf( szDatetime, "%04d%02d%02d%02d%02d%02d",
                     tmptm->tm_year + 1900, tmptm->tm_mon + 1, tmptm->tm_mday,
                     tmptm->tm_hour, tmptm->tm_min, tmptm->tm_sec);
	Textout("DateTime = %04d-%02d-%02d %02d:%02d:%02d",
                     tmptm->tm_year + 1900, tmptm->tm_mon + 1, tmptm->tm_mday,
                     tmptm->tm_hour, tmptm->tm_min, tmptm->tm_sec);

	if ( bparam.stIEBaseParam.dwDeviceID[0] == 0 )
	{
		Textout("UID is NULL");
		return;
	}
	else
	{
		Textout("UUID = [%s]", bparam.stIEBaseParam.dwDeviceID);
	}

	TransferUUID(bparam.stIEBaseParam.dwDeviceID, szUUID);

	if ( 1 )
	{
		char msg_content[1024] = {0};
		sprintf(msg_content,"%s,%d,%s,%s,%s,",
			//"OBJ002868GHYYC",
			//"OBJ008445PRSPK",
			//bparam.stIEBaseParam.dwDeviceID,
			szUUID,
			type,
			szDatetime,
			jpgfile,
			videofile);
		//OnDoorP2PAlarm(msg_content);
	}

	OnDoorPush(
		//"OBJ002868GHYYC",
		//"OBJ008445PRSPK",
		//bparam.stIEBaseParam.dwDeviceID,
		szUUID,
		type,
		szDatetime,
		jpgfile,
		videofile);
#endif
}

#endif


