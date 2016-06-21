#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h> 
#include "param.h"


#ifdef JPUSH

/* Type define */
typedef unsigned char byte;
typedef unsigned int uint32;

#ifndef	BOOL
#define	BOOL 	int
#define	TRUE 	1
#define	FALSE	0
#endif

#define	false FALSE
#define	true  TRUE


uint32 _state[4];	/* state (ABCD) */
uint32 _count[2];	/* number of bits, modulo 2^64 (low-order word first) */
byte _buffer[64];	/* input buffer */
byte _digest[16];	/* message digest */
BOOL _finished;		/* calculate finished ? */
char szMD5Out[64];


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

/*
#define OutputDebugString1(Fmt, args...) \
{   \
    printf("==== %-16s, line %4d, %-24s:"Fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##args); \
}

#define Textout(Fmt, args...) OutputDebugString1(Fmt, ##args)
#define Textout1(Fmt, args...) {}
*/


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


extern int InitSocket( unsigned char byIsServer, unsigned char byIsTcp, struct sockaddr_in* saddr );
extern int CloseSocket( int iSocket );

int OnDoorRequest(char *host_name, int host_port, char *pData, int nDataSize)
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

	memset( ipaddr, 0x00, 16 );


#if	1
    //nRet = GetDnsIp( host_name, ipaddr );
	//Textout("Server = [%s], IPADDR = [%s]", host_name, ipaddr);
	
	//ipaddr[0] = 0;
#else
	strcpy(ipaddr, "183.232.25.234");
#endif

	if(jpush_address[0] != 0){
		strcpy(ipaddr, jpush_address);
	}
	else
	{
		strcpy(ipaddr, "183.232.25.234");
	}
	Textout("jpush ipaddr:%s",ipaddr);
	
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

	daddr.sin_family        = AF_INET;
    daddr.sin_port          = htons(host_port);
    daddr.sin_addr.s_addr   = inet_addr( ipaddr );


	nRet = connect( rsocket, ( struct sockaddr* )&daddr, sizeof( struct sockaddr ) );
	if(nRet == -1)	//SOCKET_ERROR)
	{
		Textout( "Can't connect %s",host_name);
        CloseSocket( rsocket );
		return -1;
	}
	else
	{

		Textout("connected   with host %s",host_name);
	}

	nRet = send(rsocket, pData, nDataSize, 0);
	if(nRet == -1)	//SOCKET_ERROR)
	{
		Textout( "Can't Send");
        CloseSocket( rsocket );
		return -1;
	}
	else
	{
		Textout("send() OK");
		//Textout("pData:%s",pData);
	}

	char   dest[1024];
	nRet=0;
	
	nRet=recv(rsocket,(char *)dest,sizeof(dest),0);
	
	if(nRet>0)
	{
		dest[nRet]=0;

		

		if(strstr(dest,"Succeed") == NULL )
		{
			nRet = -1;
			Textout("Received bytes:%d",nRet);
			printf("Result:%s\n",dest);
		}
		else
		{
			printf("jPush send Succeed!\n");
			nRet = 0;
		}
	}
	else
	{
		printf("jPush not recv data! ");
		dest[0]=0;
		nRet = -1;
	}

	CloseSocket(rsocket);
	return nRet;
}

int OnDoorPost(char *Connect_String, char *host_name, int host_port, char *pData, int nDataSize)
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
	//printf("data:%s\n",request);
	return OnDoorRequest(host_name, host_port, request, strlen(request));
}

int GetDoorSendNumber()
{
#if	1
	static int sendno = 0;
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
	
	//bell_data.sendid++;
	//WriteBellData();
	//return bell_data.sendid;
	return 1234;
#endif
}

//#define	USE_CUSTOM_MESSAGE

//void OnDoorPush(char *uuid, int type, char *send_date, char *pic_file, char *video_file)
int jPush(char *send_data)
{
	static int total = 0;;
	static int succeed = 0;
	int iRet = 0;
	//Textout("jPush");
	char *Connect_String = "http://api.jpush.cn:8800/v2/push";
	//char *Connect_String = "https://api.jpush.cn/v2/push";

	//char *host_name = "api.jpush.cn";
	char *host_name = JPUSH_HOST_NAME;
	int host_port = 8800;

	int sendno = GetDoorSendNumber();
	#if 1
	//char *app_key = "d85cfab48185f35639c28a27";
	//char *master_secret = "9d1345b39e884cc08f5268da";
	#else
	char *app_key = "1a7160f816cac10b7cc41ea5";
	char *master_secret = "605c51e90fd84745bcdd6323";
	#endif

	int i;
	char app_key[64];
	char master_secret[64];
	char receiver_value[64];
	int receiver_type =3;

	int sendTotalNum = 0;
	int sendSuccessNum = 0;

	//ReadJPushParams();
	
	for(i=0; i<MAX_JPUSH_SIZE; i++)
	{
		memset(app_key,0,sizeof(app_key));
		memset(master_secret,0,sizeof(master_secret));
		memset(receiver_value,0,sizeof(receiver_value));

		//Textout("registerTime[%d]:%d    time:%d",i,jpushparamlist.registerTime[i],time(NULL));
		
		if(jpushparamlist.registerTime[i] > 0)
		{
			//Textout("registerTime[%d]:%d",i,jpushparamlist.registerTime[i]);
			//Textout("time:%d",time(NULL));
			
			if(jpushparamlist.registerTime[i] + (2*24*3600) < time(NULL)
                || jpushparamlist.registerTime[i]   > (time(NULL) + 24*3600) )
			//if(jpushparamlist.registerTime[i] + (10*60) < time(NULL))  //10min test
			{
                /*
				memset(&jpushparamlist.stJpushParam[i],0,sizeof(JPUSHPARAM));
				jpushparamlist.registerTime[i] = 0;
				WriteJPushParams();
				Textout("Registration information has expired! delete jpush i:%d",i);
				*/
			}
			else
			{
				Textout("jpush i:%d",i);
				strcpy(app_key,jpushparamlist.stJpushParam[i].appKey);
				strcpy(master_secret,jpushparamlist.stJpushParam[i].masterKey);
				strcpy(receiver_value,jpushparamlist.stJpushParam[i].receiverValue);
				//receiver_type = jpushparamlist.stJpushParam[i].receiverType;

				printf("app_key:%s, master_secret:%s,receiver_value:%s\n",app_key,master_secret,receiver_value);

				char *platform = "android,ios";

				char md5_str[32] = {0};
				char szTmp1[1024];
				
				sprintf(szTmp1, "%d%d%s%s",
					sendno,
					receiver_type,
					receiver_value,
					master_secret );

				MD5_reset();
				MD5_update((const char *)szTmp1, strlen(szTmp1));

				sprintf(md5_str, "%s", MD5_toString());
				//Textout("MD5 = [%s]", md5_str);

				//char n_content[1024] = {0};
				//sprintf(n_content,"\"%s\"",send_data);

				char n_extras[100] = {0};
				sprintf(n_extras,"{\"ios\":{\"badge\":1,\"sound\":\"notify.wav\"}}");

				char request[2048] = {0};
				sprintf(request,
					"sendno=%d"
					"&app_key=%s"
					"&receiver_type=%d"
					"&receiver_value=%s"
					"&verification_code=%s"
					"&platform=\"%s\""
					"&msg_type=1"
					"&msg_content={\"n_content\":\"%s\",\"n_extras\":%s}"
					"&apns_production=1",

					sendno++,
					app_key,
					receiver_type,
					receiver_value,
					md5_str,
					platform,
					//n_content,
					send_data,
					n_extras
					);
				sendTotalNum ++;
				total ++;

				if(OnDoorPost(Connect_String, host_name, host_port, request, strlen(request)) == 0)
				{
					sendSuccessNum ++;
					succeed ++;
					iRet = sendSuccessNum;
				}
			}
		}
		
	}
	printf("jPush sendTotalNum:%d, success:%d,  failure:%d\n",sendTotalNum,sendSuccessNum,sendTotalNum - sendSuccessNum);
	printf("jPush total:%d, success:%d,  failure:%d\n",total,succeed,total - succeed);
	return iRet;
}



void jPushTest(int singno)
{
	static int testNo = 0;
	testNo++;
	
	Textout("Start the No:%d test",testNo);
	jPush("jPush test");
}

void jPushInit(void)
{
    struct itimerval tick;
    
    signal(SIGALRM, jPushTest);
    memset(&tick, 0, sizeof(tick));

    tick.it_value.tv_sec = 5;
    tick.it_value.tv_usec = 0;

    tick.it_interval.tv_sec = 5;
    tick.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
            printf("Set timer failed!\n");
}

#endif



