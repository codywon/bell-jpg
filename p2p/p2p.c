#include	<stdio.h>
#include	<stdlib.h>
#include 	<semaphore.h>
#include	<pthread.h>

#include "p2pcmd.h"

#include "init.h"
#include "alarm.h"
#include "param.h"
#include "dns.h"
#include "cmdhead.h"
#include "network.h"
#include "vbuf.h"
#include "abuf.h"
#include "mjpgbuf.h"
#include "protocol.h"
#include "debug.h"

#include "ipcparam.h"

#ifdef PPCS_API
#include "PPCS_Type.h"
#include "PPCS_Error.h"
#include "PPCS_API.h"
#else

#ifdef P2P_BIZ_LIB
#include "XQ_Global.h"
#include "XQ_Type.h"
#include "PB_Error.h"
#include "P2P_Data.h"
#include "Biz_Data.h"
#include "Biz_CallBack.h"
#include "PB_API.h"
#else
#include "PPPP_Type.h"
#include "PPPP_Error.h"
#include "PPPP_API.h"

#include "biz.h"
#endif
#endif

#if	0
#define	P2PTextout(Fmt, args...)
#else
#define	P2PTextout	Textout
#endif

//int wifiScanSit = 0;
//short wifiScanCmd = 0;
int bScanEnd = 0;

static BOOL bPrevSetWifiCgi = FALSE;

void ResetWatch(int sit);
void ResetTalk(int sit);

int GetStrParamValueEx( const char* pszSrc, const char* pszParamName, char* szValue );
int GetIntParamValueEx( const char* pszSrc, const char* pszParamName, int* iValue );

void StartWatch(int sit);

int 			p2phandle = -1;
ACCEPTSOCK		p2pstream[MAX_P2P_CONNECT];
ENCBURECORD     p2precordplay[MAX_P2P_CONNECT];
unsigned int	p2prefcnt = 0;
char			p2pstart = 0;
sem_t           p2psem;
sem_t			p2paudio;
pthread_mutex_t	p2pmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	p2pwritemutex = PTHREAD_MUTEX_INITIALIZER;
unsigned char		videosendbuf[MAX_JPEG_BUFFER_SIZE];
unsigned char		recordsendbuf[MAX_JPEG_BUFFER_SIZE];
unsigned char   sendFrameRate = 0;

void p2plock( void )
{
    pthread_mutex_lock( &p2pmutex );
}

void p2punlock( void )
{
    pthread_mutex_unlock( &p2pmutex );
}

void p2pwritelock( void )
{
    pthread_mutex_lock( &p2pwritemutex );
}

void p2pwriteunlock( void )
{
    pthread_mutex_unlock( &p2pwritemutex );
}


void P2p_streaminit( void )
{
    int i = 0;

    memset( &p2pstream, 0x00, sizeof( ACCEPTSOCK ) * MAX_P2P_CONNECT );

    for ( i = 0; i < MAX_P2P_CONNECT; i++ )
    {
        p2pstream[i].socket = -1;
        p2pstream[i].len    = 0;
    }
}


#define	MAX_PPPP_WRITE_SIZE	(32 * 1024)
INT32 PPPP_WriteEx( INT32 SessionHandle, UCHAR Channel, CHAR* DataBuf, INT32 DataSizeToWrite )
{
	int iRet = 0;
	
	p2pwritelock();
	{
    	char *pDataBuf = DataBuf;
    	int nDataSizeToWrite = DataSizeToWrite;
    	while( nDataSizeToWrite > 0 )
    	{
    		iRet = PPPP_Write( SessionHandle, Channel, pDataBuf, ( nDataSizeToWrite > MAX_PPPP_WRITE_SIZE ) ? MAX_PPPP_WRITE_SIZE : nDataSizeToWrite );
    		if ( iRet < 0 )	{ DataSizeToWrite = -1; break;}
    		pDataBuf += iRet;
    		nDataSizeToWrite -= iRet;
    	}
    }
    p2pwriteunlock();
	return DataSizeToWrite;
}


//p2p stream socket
void PushP2pSocket( int socket )
{
    /* BEGIN: Modified by wupm, 2013/7/1 */
    //char i;
    int i;
    char    flag     = 0x00;
    p2plock();

	//find null scoket
    for ( i = 0; i < MAX_P2P_CONNECT; i++ )
    {
        if ( p2pstream[i].socket == -1 )
        {
            flag = 0x01;
            break;
        }
    }

//find,if full and to shutdown socket
    if ( flag == 0x00 )
    {
        unsigned int refback = p2prefcnt - MAX_P2P_CONNECT;

        for ( i = 0; i < MAX_CONN_USER_NUM; i++ )
        {
            //printf( "i %d refback %d p2prefcnt %d\n", i, refback, p2pstream[i].refcnt );

            if ( p2pstream[i].refcnt <= refback )
            {
                PPPP_Close( socket );	//close p2p session handle
                p2pstream[i].socket = -1;
                p2pstream[i].refcnt = 0;
                p2pstream[i].len    = 0;
                p2pstream[i].liveflag = 0x00;
                p2pstream[i].audioflag = 0x00;
                p2pstream[i].recordflag = 0x00;
                p2pstream[i].recordlen = 0x00;
                memset( p2pstream[i].buffer, 0x00, 1024 );

				/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/12 */
				//P2PTextout("Set [%d] to PS_IDLE", i);
				p2pstream[i].nIndex = 0;
				p2pstream[i].state = PS_IDLE;
				p2pstream[i].stateEx = PS_IDLE;
                break;
            }
        }
    }

//add socket
    for ( i = 0; i < MAX_P2P_CONNECT; i++ )
    {
        if ( p2pstream[i].socket == -1 )
        {
            //printf( "push socket %d sit %d\n", socket, i );
            p2pstream[i].socket = socket;
            p2pstream[i].len    = 0;
            p2pstream[i].refcnt = p2prefcnt;
            p2pstream[i].recordflag = 0x00;
            p2pstream[i].recordlen = 0x00;
            p2pstream[i].liveflag = 0x00;
            p2pstream[i].audioflag = 0x00;
            PPPP_Share_Bandwidth( 0 );
            //printf( "PPPP_Share_Bandwidth(0) iRet\n" );

			/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/12 */
			//P2PTextout("Set [%d] to PS_IDLE", i);
			//p2pstream[i].nIndex = 0;
			p2pstream[i].state = PS_IDLE;
			p2pstream[i].stateEx = PS_IDLE;
            break;
        }
    }

    p2prefcnt++;
    p2punlock();
}


void PopP2pSocket( int socket )
{
    /* BEGIN: Modified by wupm, 2013/7/1 */
    //char i;
    int i;

    char flag = 0x00;
    int iRet;
    char flag1 = 0;
    p2plock();

    for ( i = 0; i < MAX_P2P_CONNECT; i++ )
    {
        if ( p2pstream[i].socket == socket )
        {
            iRet = PPPP_ForceClose( socket );
            p2pstream[i].socket    = -1;
            p2pstream[i].len       = 0;
            p2pstream[i].refcnt    = 0;
            p2pstream[i].liveflag = 0x00;
            p2pstream[i].audioflag = 0x00;
            p2pstream[i].talklen = 0x00;
            p2pstream[i].recordflag = 0x00;
            p2pstream[i].recordlen = 0x00;
            memset( p2pstream[i].buffer, 0x00, 1024 );
            memset( p2pstream[i].talkbuffer, 0x00, 1024 );
            flag = 0x01;
            //printf( "close p2p socket %d\n", p2pstream[i].socket );
            //break;

			/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/12 */
			//P2PTextout("Set [%d] to PS_IDLE", i);
			p2pstream[i].nIndex = 0;
			p2pstream[i].state = PS_IDLE;
			p2pstream[i].stateEx = PS_IDLE;
        }

        if ( p2pstream[i].socket != -1 )
        {
            flag1 = 0x01;
        }
    }

    p2punlock();

    if ( flag == 0x00 )
    {
        //printf( "not find socket=%d\n", socket );
    }

    if ( flag1 == 0x00 )
    {
        iRet = PPPP_Share_Bandwidth( 1 );
        //printf( "PPPP_Share_Bandwidth(1) iRet %d\n", iRet );
    }
}

/* BEGIN: Read P2P/BIZ Server-String from file */
/*        Added by wupm(2073111@qq.com), 2014/8/14 */
#define	SS_MAX_LEN	1024
static char szP2pServerString[SS_MAX_LEN+1] = {0};
static char szBizServerString[SS_MAX_LEN+1] = {0};

#define	SS_P2P	0
#define	SS_BIZ	1
#define	SS_P2P_FILENAME	"/system/www/object.bin"
#define	SS_BIZ_FILENAME	"/system/www/biz.bin"
#define	SS_P2P_DEFAULT	"BPGBBOEOKJMBHJNCFFDFAEEJCMMHDANKDFADBNHDBGJMLDLOCIACCJOPHMLCJPKJABMELOCMOCJKABHGIANGNPBLIPOIFPCPAIGJDCFDMMLCEFHLACBDPGNA"

#define SS_ACTOP_P2P_SERVER_STRING  "EJLXLRPGLKPDSYIFHULNEOHWEEHXLQEHIHIBIAAOPIIDSQAREGENPALOSTELERPDLNHYLKHXEILSHUICEMEEEHEPEK-INIPJGIHHXEREXEZFQEYFIIGELEGAQSSPCLMEQEHHWEG"

/* BEGIN: NEW OBJECT P2P SERVER */
/*        Modified by wupm(2073111@qq.com), 2014/9/25 */
//DOOR-BELL(OJTBL-)
//"SVTDEHAYLOSQTBHYPAARPCPFLKLQSTPNPDTAEIHUPLASEEPIPKAOSUPESXLXPHLUSQLVLSPALNLTLRLKLOLMLP-JBIOIWIHHXERFLFGFQEYFIIGELEGAQSSPCLMEQEHHWEG"
//OJT-IPCAM
//"SVTDEHAYLOSQTBHYPAARPCPFLKLQSTPNPDTAEIHUPLASEEPIPKAOSUPESXLXPHLUSQLVLSPALNLTLRLKLOLMLP-QHPUQCBBLVTDMRMEMMIGAVLMHWEGAQSSEQLNPCLMHWEGPNUBBPTYBAHXAQSSPCLMHWTCLQLOPJEGAQLXPXQLQGPMEHSSPCLMHWEGLWSXAQLTLPIHMPMHMJIGARSSPCLMHWEGEQSTAQSSERNCMOBKBAPDPCLMHWEGAQTCLNSSPCBBQIMQMSPMHXLMHWEGAQSSLWEHPCLMTDMXTRTMIGPHHWEGAQSSPCEQARLMHWPNMDQBPWBALREGAQSSPCLMTCSTHWEGLXMVMLMGPMPDAQSSPCLWLNLM
//OLD-OJT-IPCAM
//"BPGBBOEOKJMBHJNCFFDFAEEJCMMHDANKDFADBNHDBGJMLDLOCIACCJOPHMLCJPKJABMELOCMOCJKABHGIANGNPBLIPOIFPCPAIGJDCFDMMLCEFHLACBDPGNA"

#define	SS_BIZ_DEFAULT	"AQTDPDPKEISQLVASPALSLULKSUPELRPNLPLOLQSSPCLXLNLM"

#ifdef P2P_BIZ_LIB
#define BIZ_SERVER_STRING_NEW "PCLXIBIALKLNELLOHUENEOEEHYHZHXIHEJEIEKLMHWEREHEG"
#endif
/* BEGIN: NEW OBJECT BIZ SERVER */
/*        Modified by wupm(2073111@qq.com), 2014/9/25 */
//Release Server
//"AQTDPDPKEISQLVASPALSLULKSUPELRPNLPLOLQSSPCLXLNLM"
//Test Server
//"EGBBLNHXAUAOEHARASSQPHSVPASUSTSWTDPFPEPGAQSSPNPDPC"

//20141024, DNS
//TEST
//dev: HYLXJUGEKBHCHUGLEDGOGQGNEEGFGRGPIHEJEIEKLMHWEREHEG
//RELEASE
//pro: HYLXKHGMGWGEKBHCHUGLEDGOGQGNEEGFGRGPIHEJEIEKLMHWEREHEG

char *ReadServerString(int nServer)
{
	
	

	if ( nServer == SS_P2P )
	{
		memset(szP2pServerString, 0, SS_MAX_LEN+1);
		FILE *f = fopen(SS_P2P_FILENAME, "rb");
		if ( f != NULL )
		{
			int nReadBytes = fread(szP2pServerString, 1, SS_MAX_LEN, f);
			fclose(f);
			if ( nReadBytes > 0 )
			{
				return szP2pServerString;
			}
		}



		//20141209
		//BUG!!!!!!!!!
        #ifndef  FACTOP
		sprintf(szP2pServerString, "%s", SS_P2P_DEFAULT);
        #else
		sprintf(szP2pServerString, "%s", SS_ACTOP_P2P_SERVER_STRING);
        #endif
        
		return szP2pServerString;
	}

	if ( nServer == SS_BIZ)
	{
		memset(szBizServerString, 0, SS_MAX_LEN+1);
		FILE *f = fopen(SS_BIZ_FILENAME, "rb");
		if ( f != NULL )
		{
			int nReadBytes = fread(szBizServerString, 1, SS_MAX_LEN, f);
			fclose(f);
			if ( nReadBytes > 0 )
			{
				return szBizServerString;
			}
		}
		sprintf(szBizServerString, "%s", SS_BIZ_DEFAULT);
		return szBizServerString;
	}

	return NULL;
}


#ifdef P2P_BIZ_LIB
void callbackNotify(int nType,void *param)
{
	return ;
}
#endif

extern BOOL ExistGpsFile();
int P2P_init( void )
{
    char *pServerString = NULL;
    int 	version;
    int	iRet;
    st_PPPP_NetInfo param;
    version = PPPP_GetAPIVersion();
    P2PTextout( "P2P version:%x", version );

#ifdef P2P_BIZ_LIB
		st_InitParam initParams;
#endif



#ifdef XQ_P2P
    P2PTextout( "USE XQ SERVER, NEW LIB" );
    #ifdef ZILINK
    /* modify begin by yiqing, 2015-06-15, 原因: */
    pServerString = (char *) "EJTDHXEHIASQARSTEIPAPHATLKASPDEKPNLNLTAUHUHXSULVEESVSWAOEHPKLULXARSTPGSQPDLNPEPALRPFLKLOLQLP-$$";
    //pServerString = (char *)"SVLXSTLULOLKPLHYHUPDHWPFEELQLNIHLRIEAOLVEMSQHXENIBPAEIEGLPERELIALKEHEOHZHUEKIFEEEPEJ-FLIOFGIHHXERFWIYFAEYFIIGELEGAQSSPCLMEQEHHWEG";
    #endif

    #ifdef FOBJECT
    //pServerString = (char *)"SVLXSTLULOLKPLHYHUPDHWPFEELQLNIHLRIEAOLVEMSQHXENIBPAEIEGLPERELIALKEHEOHZHUEKIFEEEPEJ-JBIOIWIHHXERFLFGFQEYFIIGELEGAQSSPCLMEQEHHWEG";

	//use libBPPP_API_20150423.a
	//pServerString = (char *)"SVLXSTLULOLKPLHYHUPDHWPFEELQLNIHLRIEAOLVEMSQHXENIBPAEIEGLPERELIALKEHEOHZHUEKIFEEEPEJ-QHPUQCBBLVTDMRMEMMIGAVLMHWEGAQSSEQLNPCLMHWEGPNUBBPTYBAHXAQSSPCLMHWTCLQLOPJEGAQLXPXQLQGPMEHSSPCLMHWEGLWSXAQLTLPIHMPMHMJIGARSSPCLMHWEGEQSTAQSSERNCMOBKBAPDPCLMHWEGAQTCLNSSPCBBQIMQMSPMHXLMHWEGAQSSLWEHPCLMTDMXTRTMIGPHHWEGAQSSPCEQARLMHWPNMDQBPWBALREGAQSSPCLMTCSTHWEGLXMVMLMGPMPDAQSSPCLWLNLM";
	pServerString = (char *)"EJTDHXEHIASQARSTEIPAPHATLKASPDEKPNLNLTAUHUHXSULVEESVSWAOEHPKLULXARSTPGSQPDLNPEPALRPFLKLOLQLP-$$";
	//zhuohao server
	//pServerString =(char *)"EJLXLRPGLKPDSYIFHULNEOHWEEHXLQEHIHIBIAAOPIIDSQAREGENPALOSTELERPDLNHYLKHXEILSHUICEMEEEHEPEK-INIPJGIHHXEREXEZFQEYFIIGELEGAQSSPCLMEQEHHWEG";
	#endif

    #ifdef ZHENGSHOW
        #if defined (UK_CUSTOMERS_OLD_KERNEL)|| defined (UK_CUSTOMERS_NEW_KERNEL)        /* modify begin by yiqing, 2015-06-09, 原因:针对英国客户 */
        pServerString = (char *)"EJLXPHLQLKPDSYLRHULOHWEELTHYIHIBEIAOLNIDSQHXEPPIPALSEGERELIALKICEMHUEHEKIEEEEOEN-$$";
		
        #elif defined (OLD_KERNEL_OBJ)
        pServerString = (char *)"EJTDHXEHIASQARSTEIPAPHATLKASPDEKPNLNLTAUHUHXSULVEESVSWAOEHPKLULXARSTPGSQPDLNPEPALRPFLKLOLQLP-$$";

        #elif defined (PREFIX_ZSKJ)
        pServerString = (char *)"HZLXPHPGLKSYPIHUPDTALQEEPKLRIHLNLUPEAOEPLOSQHXENEJPAHYIALSERIBEKLKICIEHUEIELEGEEEHEOEM-$$";
		//pServerString = (char *)"SVLXSTLULOLKPLHYHUPDHWPFEELQLNIHLRIEAOLVEMSQHXENIBPAEIEGLPERELIALKEHEOHZHUEKIFEEEPEJ-$$";

        #elif defined (OLD_KERNEL_XDBL) || defined(NEW_KERNEL_XDBL)
        pServerString = (char *)"HZLXPHPGLKSYPIHUPDTALQEEPKLRIHLNLUPEAOEPLOSQHXENEJPAHYIALSERIBEKLKICIEHUEIELEGEEEHEOEM-$$";

        #else 
        pServerString = (char *)"EJTDHXEHIASQARSTEIPAPHATLKASPDEKPNLNLTAUHUHXSULVEESVSWAOEHPKLULXARSTPGSQPDLNPEPALRPFLKLOLQLP-$$";
        #endif  
    #endif

	#ifdef FEEDDOG
	pServerString = (char *)"SVLXSTLULOLKPLHYHUPDHWPFEELQLNIHLRIEAOLVEMSQHXENIBPAEIEGLPERELIALKEHEOHZHUEKIFEEEPEJ-$$";

	#endif
	
	#ifdef FOBJECT_FM34
		pServerString = (char *)"SVLXSTLULOLKPLHYHUPDHWPFEELQLNIHLRIEAOLVEMSQHXENIBPAEIEGLPERELIALKEHEOHZHUEKIFEEEPEJ-$$";
	
	#endif

	#ifdef BELINK
		pServerString = (char *)"SVLXSTLULOLKPLHYHUPDHWPFEELQLNIHLRIEAOLVEMSQHXENIBPAEIEGLPERELIALKEHEOHZHUEKIFEEEPEJ-$$";
		//pServerString = (char *)"EJTDHXEHIASQARSTEIPAPHATLKASPDEKPNLNLTAUHUHXSULVEESVSWAOEHPKLULXARSTPGSQPDLNPEPALRPFLKLOLQLP-$$";
	#endif


    if(pServerString == NULL)
    {
        pServerString = (char *)"HZLXPHPGLKSYPIHUPDTALQEEPKLRIHLNLUPEAOEPLOSQHXENEJPAHYIALSERIBEKLKICIEHUEIELEGEEEHEOEM-$$";
    }
    
#else
    #ifdef BIZ_SERVER_STRING_NEW
    pServerString = P2P_SERVER_STRING;
    #else
		#ifdef PPCS_AES_P2P
	    	pServerString = (char *)("EFGHFDBMKDIHGEJDEBHLFBEPGHNEHCMEHLFFBNDCAIJOKOKDDDBLDEPEHAKGIOKCBFNFKPCLPBNIAA");//英国客户善云新字符串
		#else
			/
			#ifdef 0//PPCS_P2P_TEST
			pServerString = (char *)("EBGAEIBIKHJJGFJJEEHOFAENHLNBHGNMHMFDAADAAOJNKNKGDNAPDJPIGAKOIHLBBJNMKLDIPENJBKDE");//test
			#else
		    P2PTextout( "USE OBJECT SERVER, OLD LIB" );
			pServerString = ReadServerString(SS_P2P);
			if ( pServerString == NULL )
			{
				Textout("ReadServerString(P2P) ERROR");
				pServerString = "BPGBBOEOKJMBHJNCFFDFAEEJCMMHDANKDFADBNHDBGJMLDLOCIACCJOPHMLCJPKJABMELOCMOCJKABHGIANGNPBLIPOIFPCPAIGJDCFDMMLCEFHLACBDPGNA";
			}
			#endif
	    #endif
	#endif
#endif
   	Textout("p2pServerSrting:%s",pServerString); 

	char *pBizServerString = NULL;

#ifdef BIZ_SERVER_STRING_NEW
    pBizServerString=BIZ_SERVER_STRING_NEW;
#else
	pBizServerString=ReadServerString(SS_BIZ);
#endif


#ifdef P2P_BIZ_LIB
	initParams.p2pString = pServerString;
	initParams.bizString = pBizServerString;
	strncpy(initParams.bizAppInf.appVer,"010101001",12);
	strncpy(initParams.p2pParams._devName,"testNode",16);
	initParams.p2pParams._maxNmbOfSession=32;
	initParams.p2pParams._maxNmbOfChannel=8;
	initParams.p2pParams._maxSizeOfChannel=128;
	initParams.p2pParams._maxSizeOfPacket=1024;
	iRet=BPPP_Init(&initParams,1000,5*60);
	P2PTextout("P2P ServerString = [%s]", pServerString);
	P2PTextout("Biz ServerString = [%s]", pBizServerString)
	Textout("BPPP_Init finished: the ret is %d",iRet);
	
#else

    //Textout("*********************************************");

    //Textout("p2pServerSrting=%s,len:=%d",pServerString,strlen(pServerString));

	iRet = PPPP_Initialize(pServerString);
	Textout("P2P ServerString = [%s]", pServerString);

	if ( iRet != ERROR_PPPP_SUCCESSFUL )
    {
        printf( "error initialize iRet %d\n", iRet );
		
    }
    
	Textout("----------P2P_Init Return = %d", iRet);

    /*Biz server initial*/
	{
		
		iRet = Biz_Init(pBizServerString);
		P2PTextout("Biz ServerString = [%s]", pBizServerString)
	}
	Textout("----------Biz_Init Return = %d", iRet);

#endif

	iRet = PPPP_NetworkDetect( &param, 0 );
    printf( "iRet %x\n", iRet );
    printf( "bFlagInternet %d\n", param.bFlagInternet );
    printf( "bFlagHostResolved %d\n", param.bFlagHostResolved );
    printf( "bFlagServerHello %d\n", param.bFlagServerHello );
    printf( "NAT_Type %d\n", param.NAT_Type );

    PPPP_Share_Bandwidth( 1 );
    Textout("*********************************************");

    #ifdef LDPUSH
    bEnablePush = 1;
    #endif
    
    return param.bFlagHostResolved;
}

void p2pdelay( int delay )
{
    int i = 0;

    for ( i = 0; i < delay; i++ )
    {
        sleep( 1 );
    }
}

void* p2pInitThreadProc( void* p )
{
    st_PPPP_NetInfo param;

    while ( 1 )
    {
        PPPP_NetworkDetect( &param, 0 );
        if ( param.bFlagHostResolved == 0 )
        {
            p2pdelay( 10 );
            continue;
        }

        p2pdelay( 10 );
    }
}

void P2P_close( int handle )
{
    if ( handle >= 0 )
    {
        //printf( "P2P_close\n" );
        PopP2pSocket( handle );
    }
}

#if	1
/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/25 */
BOOL bUserExisted(int sit, int nIndex)
{
	int i = 0;
	for(i=0; i<MAX_P2P_CONNECT; i++ )
	{
		//P2PTextout("Channel (%d), sokcet = %d, PrevIndex = %d", i, p2pstream[i].socket, p2pstream[i].nIndex);
		if ( sit == i )
		{
			continue;
		}
		if ( p2pstream[i].socket != -1 && p2pstream[i].nIndex == nIndex )
		{
			return TRUE;
		}
	}
	return FALSE;
}

/* BEGIN: Added by wupm(2073111@qq.com), 2014/7/4 */
int ResetUserConnect(int sit)
{
	P2PTextout("Reset Channel = %d", sit);
	PopP2pSocket(p2pstream[sit].socket);
}

int P2PGetUserPri( int sit, char* loginuse, char* loginpas )
{
    int i;
    int pri = 0x00;		//Username is ERROR
    char user[32];
    char passwd[32];
    
    memset( user, 0x00, 32 );
    memset( passwd, 0x00, 32 );

    memcpy( user, loginuse, 32);
    memcpy( passwd, loginpas, 32 );

    if ( strlen( user ) == 0x00 || user[0] == 0 )
    {
        return 0;
    }

	for ( i = 0; i < MAX_USER; i++ )
    {
        if ( !strcmp( bparam.stBell.user[i], user ) )
        {
			if ( !strcmp( bparam.stBell.pwd[i], passwd) )
	        {
				if ( bUserExisted(sit, i + 1) )
				{
					pri = -1;	//Already Used

					/* BEGIN: Added by wupm(2073111@qq.com), 2014/7/2 */
					break;
				}

	            if ( bparam.stBell.admin == i + 1 )
					pri = ( i + 1 ) + 100;
				else
					pri = i + 1;

				break;
	        }
			else
			{
				pri = -3;	//Password is ERROR
				break;
			}
        }
    }

    P2PTextout("Return pri=[%d]================\n",pri);
    return pri;
}
#endif

/* BEGIN: Modified by wupm(2073111@qq.com), 2014/6/28 */
//Return
//-1	Exist
//0		Password/Username ERROR
//1-8	Normal user
//101-108	Administrator
//and	p2pstream[sit].nIndex = User Index
int CheckP2pPri( int sit )
{
    int             iRet;
    char            usertemp[32];
    char            pwdtemp[32];
    unsigned char*   pbuf = p2pstream[sit].buffer + sizeof( CMDHEAD );
    
    memset( usertemp, 0x00, 32 );
    memset( pwdtemp, 0x00, 32 );
    iRet = GetStrParamValue( pbuf, "loginuse", usertemp, 31 );

    if ( iRet == 0 )
    {
        iRet = GetStrParamValue( pbuf, "loginpas", pwdtemp, 31 );
    }

    if ( iRet )
    {
        memset( usertemp, 0x00, 32 );
        memset( pwdtemp, 0x00, 32 );
        iRet = GetStrParamValue( pbuf, "user", usertemp, 31 );

        if ( iRet == 0 )
        {
            iRet = GetStrParamValue( pbuf, "pwd", pwdtemp, 31 );
        }

        if ( iRet )
        {
            return 0;
        }

        //printf("1user:%s=pwd:%s\n",usertemp,pwdtemp);
    }

    else
    {
        //printf("2user:%s=pwd:%s\n",usertemp,pwdtemp);
    }

    iRet = P2PGetUserPri( sit, usertemp, pwdtemp );

	if ( iRet > 100 )
	{
		p2pstream[sit].nIndex = iRet - 100;
	}
	else if ( iRet > 0 )
	{
		p2pstream[sit].nIndex = iRet;
	}
	//Textout("========TEST 2======");

	/* BEGIN: Add New CGI: get_doorbelllogs.cgi */
	/*        Added by wupm(2073111@qq.com), 2014/8/12 */
	#ifdef	ENABLE_BELL_LOG
	strcpy(p2pstream[sit].szUser, usertemp);
	#endif

	//Textout("========TEST 3======");

	//P2PTextout("CheckP2P, return = %d, nUserIndex = %d, Connect Index = %d, socket = %d",
	//	iRet, p2pstream[sit].nIndex, sit, p2pstream[sit].socket);

    return iRet;
}

//==================p2p cgi control param======================
/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
#if	1
int p2pgetstatus( int sit, short cmd )
{
    unsigned char   temp[2048];
    char		mac[20];
    int             len = 0;
    int		byPri = 0;
    CMDHEAD 	head;
    int		iRet;
    int		value;
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//	printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        ////head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    len += sprintf( temp + len, "alias=\"%s\";\r\n", bparam.stIEBaseParam.szDevName );
    len += sprintf( temp + len, "deviceid=\"%s\";\r\n", bparam.stIEBaseParam.dwDeviceID );
    value = bparam.stIEBaseParam.sys_ver;
    len += sprintf( temp + len, "sys_ver=\"%d.%d.%d.%d\";\r\n",
                    //( value & 0xff ), ( value >> 8 ) & 0xff, ( value >> 16 ) & 0xff, ( value >> 24 ) & 0xff );
                    ( value >> 24 ) & 0xff, ( value >> 16 ) & 0xff, ( value >> 8 ) & 0xff, value & 0xff );

    /* BEGIN: Added by wupm, 2013/6/19 */
#ifdef	AUTO_DOWNLOAD_FIRMWIRE

    if ( 1 )
    {
        char app_version[64];
        char oem_version[64];
        memset( app_version, 0, 64 );
        memset( oem_version, 0, 64 );
        GetAppID( app_version );
        GetOemID( oem_version );

        /* BEGIN: Added by wupm, 2013/6/23 */
        len += sprintf( temp + len, "app_version=\"%s\";\r\n", app_version );
        len += sprintf( temp + len, "oem_id=\"%s\";\r\n", oem_version );
    }

#endif

    len += sprintf( temp + len, "now=%d;\r\n", bparam.stDTimeParam.dwCurTime );
    len += sprintf( temp + len, "alarm_status=%d;\r\n", bparam.stStatusParam.warnstat );

#ifdef	INSTEAD_UPNPSTATUS_TO_DEVICETYPE
    len += sprintf( temp + len, "upnp_status=%u;\r\n", bparam.stExtraParam.nDevicetype );
#else
    len += sprintf( temp + len, "upnp_status=%d;\r\n", bparam.stStatusParam.upnpstat );
#endif

    len += sprintf( temp + len, "dnsenable=%d;\r\n", bparam.stDdnsParam.enable );
    len += sprintf( temp + len, "osdenable=%d;\r\n", bparam.stVencParam.OSDEnable );
    memset( mac, 0x00, 20 );
    memcpy( mac, bparam.stIEBaseParam.szMac, 17 );
    len += sprintf( temp + len, "mac=\"%s\";\r\n", mac );
    memset( mac, 0x00, 20 );
    memcpy( mac, bparam.stIEBaseParam.szWifiMac, 17 );
    len += sprintf( temp + len, "wifimac=\"%s\";\r\n", mac );
    len += sprintf( temp + len, "dns_status=%d;\r\n", bparam.stDdnsParam.dnsstatus );
    len += sprintf( temp + len, "sdstatus=%d;\r\n", bparam.stStatusParam.sdstatus );

    /* BEGIN: Modified by wupm, 2013/5/24 */
    len += sprintf( temp + len, "devicetype=0x00;\r\n" );
    //len += sprintf( temp + len, "devicetype=%d;\r\n", (unsigned int)bparam.stExtraParam.nDevicetype);
    //len += sprintf( temp + len, "devicesubtype=%d;\r\n", (unsigned int)bparam.stExtraParam.nDeviceSubtype);

    /* BEGIN: Deleted by wupm, 2013/4/11 */
    Lock_GetStorageSpace();
    len += sprintf( temp + len, "record_sd_status=%d;\r\n", bparam.stRecordSet.sdstatus );
    //len += sprintf( temp + len, "sdstatus=%d;\r\n", bparam.stStatusParam.sdstatus );
    len += sprintf( temp + len, "sdtotal=%d;\r\n", sdtotal );
    len += sprintf( temp + len, "sdfree=%d;\r\n", sdfree );
    UnLock_GetStorageSpace();


//	len += sprintf( temp + len, "externwifi=%d;\r\n", value );	//0
    len += sprintf( temp + len, "var externwifi=%d;\r\n", externwifistatus );	//0
    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd	= cmd;
    head.len        = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );

//	printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
#endif

#define	bIsNewWatching(sit)		( ( filename[0] != 0) && (p2pstream[sit].szWatchID[0] == 0 ) )
#define	bIsSameCalling(sit)		( ( filename[0] != 0) && (p2pstream[sit].szCallID[0] != 0) && ( strcmp(filename, p2pstream[sit].szCallID) == 0 ))
#define	bIsSameWatching(sit)	( ( filename[0] != 0) && ( p2pstream[sit].szWatchID[0] != 0) && ( strcmp(filename, p2pstream[sit].szWatchID) == 0 ) )
#define	bIsSameAlarming(sit)	( ( filename[0] != 0) && ( p2pstream[sit].szAlarmID[0] != 0) && ( strcmp(filename, p2pstream[sit].szAlarmID) == 0 ) )
/* BEGIN: 2G/3G/4G/WIFI, Change Video FrameRate */
/*        Added by wupm, 2014/12/9 */
#if defined	(FACTOP) || defined (KONX)
BOOL CheckStartCalling();
void SetWatchFrameRate(int nFrameRate, int sit, char *filename)
{
    Textout("Set Watching FrameRate = %d, Calling = %d", nFrameRate, CheckStartCalling());
    if ( !CheckStartCalling() )
    {
        bparam.stVencParam.byframerate = nFrameRate;
    }
}
void SetAgreeFrameRate(int nFrameRate, int sit, char *filename)
{
    Textout("Set Calling FrameRate = %d", nFrameRate);
    bparam.stVencParam.byframerate = nFrameRate;
}
#endif

void BaAudioTalkOverCallback(void)
{   
#if  defined( ZILINK ) || defined(ZHENGSHOW)
    /*Close LED D7/8*/
    ControlBellLED(CB_OPEN);
    return;
#endif    

#if defined(FEEDDOG) || defined(FUHONG) || defined(YUELAN) || defined(BELINK)
    /*Close LED D7/8*/
    ControlBellLED(CB_CLOSE);
    return;
#endif

    ControlBellLED(CB_FLASH_STOP);
}

static char lock1Flag = 0;
static char lock2Flag = 0;
int p2plivestream( int sit )
{
    int 		i;
    int     	iRet;
    int     	streamid;
	char		filename[64] = {0};
    int byPri = 0;
    int         nFrameRate = -1;
    unsigned char*	 pbuf = p2pstream[sit].buffer + sizeof( CMDHEAD );
    byPri = CheckP2pPri( sit );
    if ( byPri == 0x00 )
    {
        return -1;
    }

	iRet = GetIntParamValueEx( pbuf, "streamid", &streamid );
	iRet += GetStrParamValueEx( pbuf, "filename", filename );

	#ifndef	SUPPORT_IPCAMERA
    if ( iRet != 0 )
    {
        return -1;
    }
	#endif

	P2PTextout("Recv(%d).. streamid = %d, filename = [%s], now CallID = [%s], WatchID = [%s], AlarmID = [%s]",
		sit, streamid, filename,
		p2pstream[sit].szCallID,
		p2pstream[sit].szWatchID,
		p2pstream[sit].szAlarmID);

    /* BEGIN: 2G/3G/4G/WIFI, Change Video FrameRate */
    /*        Added by wupm, 2014/12/9 */
    #if defined	(FACTOP) || defined (KONX)
    if ( 1 )
    {
        int nRetValue = GetIntParamValueEx( pbuf, "framerate", &nFrameRate);
        if ( nRetValue != 0 || nFrameRate < 0 || nFrameRate >= 25)
        {
            nFrameRate = -1;
        }
        //nFrameRate = 15;
    }
    #endif
    /* END:   Added by wupm, 2014/12/9 */
		

	#ifndef	REMOVE_ENCRYPT
	if ( encryptflag == 1 )
	{
		Textout("Encrypt Invalid, Donothing...");
		return 0;
	}
	#endif

	/* BEGIN: Modified by wupm(2073111@qq.com), 2014/6/24 */
    switch ( streamid )
    {
		/*
		4=要文件
		20=请求监视
		21=停止监视
		22=接听
		23=延时开锁1
		24=结束本次通话
		25=拒绝本次呼叫
		26=复位当前监视的时长
		27=复位当前通话的时长
		41=Start Audio
		42=Stop Audio
		
		28=延时开锁2
		29=长按开锁1
		30=长按关锁1
		31=长按开锁2
		32=长按关锁2
		*/
		#ifdef	REMOVE_AUDIOSTREAM_CGI

        #ifdef  FEEDDOG
        case 51:
            P2PTextout("Recv 51, Open IR LED""Now, Donothing...");
            break;
        #endif
        
		
		case 41:
			P2PTextout("Recv 41, Request Start Audio(%d)", sit);
			SetAudioInStatus(1);
			
			#ifndef UNABLE_AUDIO
			#ifndef SUPPORT_FM34
            SetAudioGongFang(FALSE);
            #endif
			if ( bIsSameWatching(sit) )
			{
				P2PTextout("bIsSameWatching");
				switch(p2pstream[sit].stateEx)
				{
					case PS_WATCHING:
						//ResetWatch(sit);
						P2PTextout("Start Audio(%d)", sit);
						p2pstream[sit].audioflag = 1;
						break;
					default:
						P2PTextout("----UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else if ( bIsSameAlarming(sit) )
			{
				P2PTextout("bIsSameAlarming");
				switch(p2pstream[sit].stateEx)
				{
					case PS_WATCHING:
						//ResetWatch(sit);
						P2PTextout("Start Audio(%d)", sit);
						p2pstream[sit].audioflag = 1;
						break;
					default:
						P2PTextout("----UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else if ( bIsSameCalling(sit) )
			{
				P2PTextout("bIsSameCalling");
				switch(p2pstream[sit].state)
				{
					case PS_TALKING:
						//ResetTalk(sit);
						P2PTextout("Start Audio(%d)", sit);
						p2pstream[sit].audioflag = 1;
						break;
					default:
						P2PTextout("----UNKNOW STATE(%d) = %d", sit, p2pstream[sit].state);
						break;
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			#endif
			break;
		case 42:
			P2PTextout("Recv 42, Request Stop Audio(%d)", sit);
			SetAudioInStatus(0);
			#ifndef UNABLE_AUDIO
			#ifndef SUPPORT_FM34
            SetAudioGongFang(TRUE);			
            #endif
			if ( bIsSameWatching(sit) )
			{
				P2PTextout("bIsSameWatching");
				switch(p2pstream[sit].stateEx)
				{
					case PS_WATCHING:
						//ResetWatch(sit);
						P2PTextout("Stop Audio(%d)", sit);
						p2pstream[sit].audioflag = 0;
						break;
					default:
						P2PTextout("----UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else if ( bIsSameAlarming(sit) )
			{
				P2PTextout("bIsSameAlarming");
				switch(p2pstream[sit].stateEx)
				{
					case PS_WATCHING:
						//ResetWatch(sit);
						P2PTextout("Stop Audio(%d)", sit);
						p2pstream[sit].audioflag = 0;
						break;
					default:
						P2PTextout("----UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else if ( bIsSameCalling(sit) )
			{
				P2PTextout("bIsSameCalling");
				switch(p2pstream[sit].state)
				{
					case PS_TALKING:
						//ResetTalk(sit);
						P2PTextout("Stop Audio(%d)", sit);
						p2pstream[sit].audioflag = 0;
						break;
					default:
						P2PTextout("----UNKNOW STATE(%d) = %d", sit, p2pstream[sit].state);
						break;
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			#endif
			break;
		#endif

		case 26:
			P2PTextout("Recv 26, Request Reset Watch, STATE(%d) = %d", sit, p2pstream[sit].stateEx);

			/* BEGIN: KONX */
			/*        Added by wupm(2073111@qq.com), 2014/10/25 */
			//PlayAlarmSount
			#ifdef	KONX		
    		if ( bKonxScareing() )
    			StopAudioPlay();
    		else
    			StartAudioPlay(WF_SCARE, 3, NULL);
			break;
			#endif

			if ( bIsSameWatching(sit) )
			{
				P2PTextout("Reset Normal Watch Time");
				switch(p2pstream[sit].stateEx)
				{
					case PS_IDLE:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_WATCHING:
						ResetWatch(sit);
						//
						break;
					/*
					case PS_CALLING:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_TALKING:
						P2PTextout("----UNEXPECT Code");
						break;
					*/
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else if ( bIsSameAlarming(sit) )
			{
				P2PTextout("Reset Alarm Watch Time");
				switch(p2pstream[sit].stateEx)
				{
					case PS_IDLE:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_WATCHING:
						ResetWatch(sit);
						break;
					/*
					case PS_CALLING:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_TALKING:
						P2PTextout("----UNEXPECT Code");
						break;
					*/
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			break;
		case 27:
			P2PTextout("Recv 27, Reset Talk Time..., STATE(%d) = %d", sit, p2pstream[sit].state);

			/* BEGIN: KONX */
			/*        Added by wupm(2073111@qq.com), 2014/10/25 */
			//PlayAlarmSount
#ifdef  KONX        
    		if ( bKonxScareing() )
    			StopAudioPlay();
    		else
    			StartAudioPlay(WF_SCARE, 3, NULL);
			break;
#endif

			if ( bIsSameCalling(sit) )
			{
				switch(p2pstream[sit].state)
				{
					case PS_IDLE:
						P2PTextout("----UNEXPECT Code");
						break;
					/*
					case PS_WATCHING:
						P2PTextout("----UNEXPECT Code");
						break;
					*/
					case PS_CALLING:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_TALKING:
						ResetTalk(sit);
						break;
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].state);
						break;
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			break;

		case 20:
			P2PTextout("Recv 20, Request Start Watch, STATE(%d) = %d", sit, p2pstream[sit].stateEx);
            SetAudioInStatus(1);
            #ifdef SUPPORT_FM34
            SetAudioGongFang(TRUE);
            #endif				
			if ( bIsNewWatching(sit) )
			{
				P2PTextout("NEW Watch, stateEx[%d]=%d", sit, p2pstream[sit].stateEx);
				switch(p2pstream[sit].stateEx)
				{
					case PS_IDLE:
						Textout("Start Video(sit=%d)", sit);
						strcpy(p2pstream[sit].szWatchID, filename);
						p2pstream[sit].stateEx = PS_WATCHING;
						p2pstream[sit].liveflag = 1;

						#ifdef	REMOVE_AUDIOSTREAM_CGI
						P2PTextout("Start Audio(sit=%d)", sit);
						p2pstream[sit].audioflag = 1;
						#endif


                        /* BEGIN: 2G/3G/4G/WIFI, Change Video FrameRate */
                        /*        Added by wupm, 2014/12/9 */
                        #if defined	(FACTOP) || defined (KONX)
                        if ( nFrameRate != -1 )
                        {
                            Textout("SetWatchFrameRate = %d", nFrameRate);
                            SetWatchFrameRate(nFrameRate, sit, filename);
                        }
                        #endif
                        /* END:   Added by wupm, 2014/12/9 */

						StartWatch(sit);
						break;
					case PS_WATCHING:
						P2PTextout("----UNEXPECT Code");
						break;
					/*
					case PS_CALLING:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_TALKING:
						P2PTextout("----UNEXPECT Code");
						break;
					*/
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else if ( bIsSameWatching(sit) )
			{
				Textout("Watch Already Exist");
				switch(p2pstream[sit].stateEx)
				{
					case PS_IDLE:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_WATCHING:
						P2PTextout("----UNEXPECT Code");
						break;
					/*
					case PS_CALLING:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_TALKING:
						P2PTextout("----UNEXPECT Code");
						break;
					*/
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else if ( bIsSameAlarming(sit) )
			{
				P2PTextout("Alarm Watch");
				switch(p2pstream[sit].stateEx)
				{
					case PS_IDLE:
						P2PTextout("Start Video(sit=%d)", sit);
						p2pstream[sit].stateEx = PS_WATCHING;
						p2pstream[sit].liveflag = 1;

						#ifdef	REMOVE_AUDIOSTREAM_CGI
						P2PTextout("Start Audio(sit=%d)", sit);
						p2pstream[sit].audioflag = 1;
						#endif

                        /* BEGIN: 2G/3G/4G/WIFI, Change Video FrameRate */
                        /*        Added by wupm, 2014/12/9 */
                        #if defined	(FACTOP) || defined (KONX)
                        if ( nFrameRate != -1 )
                        {
                            Textout("SetWatchFrameRate = %d", nFrameRate);
                            SetWatchFrameRate(nFrameRate, sit, filename);
                        }
                        #endif
                        /* END:   Added by wupm, 2014/12/9 */


						StartWatch(sit);
						break;
					case PS_WATCHING:
						P2PTextout("----UNEXPECT Code");
						break;
					/*
					case PS_CALLING:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_TALKING:
						P2PTextout("----UNEXPECT Code");
						break;
					*/
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			break;
		case 21:
			P2PTextout("Recv 21, Request Stop Watch, STATE(%d) = %d", sit, p2pstream[sit].stateEx);
#ifdef	KONX
                    if ( bKonxScareing() )
                        StopAudioPlay(FALSE);
#endif
			if ( bIsSameWatching(sit) )
			{
				switch(p2pstream[sit].stateEx)
				{
					case PS_IDLE:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_WATCHING:
						P2PTextout("Stop Video(%d)", sit);
						p2pstream[sit].szWatchID[0] = 0;
						p2pstream[sit].stateEx = PS_IDLE;
						p2pstream[sit].liveflag = 0;

						#ifdef	REMOVE_AUDIOSTREAM_CGI
						P2PTextout("Stop Audio(%d)", sit);
						p2pstream[sit].audioflag = 0;
						#endif
						break;
					/*
					case PS_CALLING:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_TALKING:
						P2PTextout("----UNEXPECT Code");
						break;
					*/
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else if ( bIsSameAlarming(sit) )
			{
				switch(p2pstream[sit].stateEx)
				{
					case PS_IDLE:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_WATCHING:
						P2PTextout("Stop Video(%d)", sit);
						p2pstream[sit].szWatchID[0] = 0;
						p2pstream[sit].stateEx = PS_IDLE;
						p2pstream[sit].liveflag = 0x00;

						#ifdef	REMOVE_AUDIOSTREAM_CGI
						P2PTextout("Stop Audio(%d)", sit);
						p2pstream[sit].audioflag = 0x00;
						#endif

						break;
					/*
					case PS_CALLING:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_TALKING:
						P2PTextout("----UNEXPECT Code");
						break;
					*/
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].stateEx);
						break;
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}

            #ifdef SUPPORT_FM34
            SetAudioGongFang(FALSE);
            #endif  
			break;

		case 22:
			P2PTextout("Recv 22, Agree..., STATE(%d) = %d", sit, p2pstream[sit].state);
            SetAudioInStatus(1);
            #ifdef SUPPORT_FM34
            SetAudioGongFang(TRUE);
            #endif			
			if ( bIsSameCalling(sit) )
			{
				switch(p2pstream[sit].state)
				{
					case PS_IDLE:
						P2PTextout("----UNEXPECT Code");
						break;
					/*
					case PS_WATCHING:
						Textout("----UNEXPECT Code");
						break;
					*/
					case PS_CALLING:
						P2PTextout("Start Video(%d)", sit);
						p2pstream[sit].state = PS_TALKING;
						p2pstream[sit].liveflag = 0x01;

						#ifdef	REMOVE_AUDIOSTREAM_CGI
						P2PTextout("Start Audio(%d)", sit);
						p2pstream[sit].audioflag = 1;
						#endif

                        /* BEGIN: 2G/3G/4G/WIFI, Change Video FrameRate */
                        /*        Added by wupm, 2014/12/9 */
                        #if defined	(FACTOP) || defined (KONX)
                        if ( nFrameRate != -1 )
                        {
                            Textout("SetAgreeFrameRate = %d", nFrameRate);
                            SetAgreeFrameRate(nFrameRate, sit, filename);
                        }
                        #endif
                        /* END:   Added by wupm, 2014/12/9 */

						StopAudioPlay();

						break;
					case PS_TALKING:
						P2PTextout("----UNEXPECT Code");
						break;
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].state);
						break;
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			break;
		case 25:
			P2PTextout("Recv 25, Refuse..., STATE(%d) = %d", sit, p2pstream[sit].state);
			if ( bIsSameCalling(sit) )
			{
				switch(p2pstream[sit].state)
				{
					case PS_IDLE:
						P2PTextout("----UNEXPECT Code");
						break;
					/*
					case PS_WATCHING:
						Textout("----UNEXPECT Code");
						break;
					*/
					case PS_CALLING:
						P2PTextout("Stop Video(%d)", sit);
						p2pstream[sit].state = PS_IDLE;
						p2pstream[sit].liveflag = 0x00;

						#ifdef	REMOVE_AUDIOSTREAM_CGI
						P2PTextout("Stop Audio(%d)", sit);
						p2pstream[sit].audioflag = 0x00;
						#endif
						break;
					case PS_TALKING:
						P2PTextout("----UNEXPECT Code");
						break;
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].state);
						break;
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			break;
		case 24:
			P2PTextout("Recv 24, Stop This Talking..., STATE(%d) = %d", sit, p2pstream[sit].state);
#ifdef	KONX
                    if ( bKonxScareing() )
                        StopAudioPlay(FALSE);
#endif
			if ( bIsSameCalling(sit) )
			{
				switch(p2pstream[sit].state)
				{
					case PS_IDLE:
						P2PTextout("----UNEXPECT Code");
						break;
					/*
					case PS_WATCHING:
						P2PTextout("----UNEXPECT Code");
						break;
					*/
					case PS_CALLING:
						P2PTextout("----UNEXPECT Code");
						break;
					case PS_TALKING:
						P2PTextout("Stop Video(%d)", sit);
						p2pstream[sit].state = PS_IDLE;
						StartAudioPlay(WF_TALK_OVER, 1, BaAudioTalkOverCallback);
						p2pstream[sit].liveflag = 0x00;

						#ifdef	REMOVE_AUDIOSTREAM_CGI
						P2PTextout("Stop Audio(%d)", sit);
						p2pstream[sit].audioflag = 0x00;
						#endif
						break;
					default:
						P2PTextout("UNKNOW STATE(%d) = %d", sit, p2pstream[sit].state);
						break;
				}		
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			break;

		case 23:
			P2PTextout("UnLock...");
			if ( bIsSameCalling(sit) )
			{
				#ifndef	USE_V10
				if ( p2pstream[sit].state == PS_TALKING )
				#endif
				{
					OnDoorOpen();
				}
			}
			else if ( bIsSameWatching(sit) )
			{
				if ( p2pstream[sit].stateEx == PS_WATCHING )
				{
					OnDoorOpen();
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			break;

		case 28:
			P2PTextout("UnLock2...");
			if ( bIsSameCalling(sit) )
			{
				#ifndef	USE_V10
				if ( p2pstream[sit].state == PS_TALKING )
				#endif
				{
					OnDoor2Open();
				}
			}
			else if ( bIsSameWatching(sit) )
			{
				if ( p2pstream[sit].stateEx == PS_WATCHING )
				{
					OnDoor2Open();
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			break;
			
		case 29:
			P2PTextout("UnLock1...");
			if ( bIsSameCalling(sit) )
			{
				if ( p2pstream[sit].state == PS_TALKING )
				{
					OnDoorUnlock(1,1);
					lock1Flag = 1;
				}
			}
			else if ( bIsSameWatching(sit) )
			{
				if ( p2pstream[sit].stateEx == PS_WATCHING )
				{
					OnDoorUnlock(1,1);
					lock1Flag = 1;
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			break;
			
		case 30:
			P2PTextout("Lock1...");
			if ( bIsSameCalling(sit) )
			{
				if ( p2pstream[sit].state == PS_TALKING )
				{
					OnDoorUnlock(1,0);
					lock1Flag = 0;
				}
			}
			else if ( bIsSameWatching(sit) )
			{
				if ( p2pstream[sit].stateEx == PS_WATCHING )
				{
					OnDoorUnlock(1,0);
					lock1Flag = 0;
				}
			}
			else
			{
				P2PTextout("----Garbage Code");
			}
			break;

		case 31:
		P2PTextout("UnLock2...");
		if ( bIsSameCalling(sit) )
		{
			if ( p2pstream[sit].state == PS_TALKING )
			{
				OnDoorUnlock(2,1);
				lock2Flag = 1;
			}
		}
		else if ( bIsSameWatching(sit) )
		{
			if ( p2pstream[sit].stateEx == PS_WATCHING )
			{
				OnDoorUnlock(2,1);
				lock2Flag = 1;
			}
		}
		else
		{
			P2PTextout("----Garbage Code");
		}
		break;

		case 32:
		P2PTextout("Lock2...");
		if ( bIsSameCalling(sit) )
		{
			if ( p2pstream[sit].state == PS_TALKING )
			{
				OnDoorUnlock(2,0);
				lock2Flag = 0;
			}
		}
		else if ( bIsSameWatching(sit) )
		{
			if ( p2pstream[sit].stateEx == PS_WATCHING )
			{
				OnDoorUnlock(2,0);
				lock2Flag = 0;
			}
		}
		else
		{
			P2PTextout("----Garbage Code");
		}
		break;
		
		default:
			P2PTextout("--------------Unknow streamid = %d", streamid);
            iRet = 0x00;
			break;
    }

    return iRet;
}

int p2plivestreamOld( int sit )
{
    int         iRet;
    int         streamid;
    int byPri = 0;
    unsigned char*   pbuf = p2pstream[sit].buffer + sizeof( CMDHEAD );
    byPri = CheckP2pPri( sit );
    if ( byPri == 0x00 )
    {
        return -1;
    }

    iRet = GetIntParamValue( pbuf, "streamid", &streamid );
    printf( "p2p streamid %d sit %d iRet %d\n", streamid, sit, iRet );

    if ( iRet != 0 )
    {
        return -1;
    }

    switch ( streamid )
    {
        case 0x00:
            iRet = 0x00;
            break;

        case 0x01:
            iRet = 0x00;
            break;

        case 0x02:
            iRet = -1;
            break;

        case 0x03:
            iRet = 0x00;
            break;

        case 0x04:
        
        break;

        case 0x0a:
            p2pstream[sit].liveflag = 0x01;
            iRet = 0x00;
            break;

        case 0x10:
            p2pstream[sit].liveflag = 0x00;
            iRet = 0x00;
            break;

        case 0x11:
            p2pstream[sit].recordflag = 0x00;
            p2pstream[sit].recordlen = 0x00;
            iRet = 0x00;
            break;

        default:
            iRet = -1;
            break;
    }

    return iRet;
}


/* BEGIN: Invalid Parameters, So System Halt!!! */
/*        Modified by wupm(2073111@qq.com), 2014/10/20 */
int p2paudiostream( int sit )
{
	return 0;
}

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
#if	1
int p2pgetparam( int sit, short cmd )
{
    unsigned char   temp[8192];
    char		temp1[64];
    char            temp2[64];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    char		i;
    memset( temp, 0x00, 8192 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
        iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    //date
    len += sprintf( temp + len, "var now=%d;\r\n", bparam.stDTimeParam.dwCurTime );
    len += sprintf( temp + len, "tz=%d;\r\n", bparam.stDTimeParam.byTzSel );
    len += sprintf( temp + len, "ntp_enable=%d;\r\n", bparam.stDTimeParam.byIsNTPServer );
    len += sprintf( temp + len, "ntp_svr=\"%s\";\r\n", bparam.stDTimeParam.szNtpSvr );
    //network param groups
    len += sprintf( temp + len, "dhcpen=%d;\r\n", bparam.stNetParam.byIsDhcp );
    len += sprintf( temp + len, "ip=\"%s\";\r\n", bparam.stNetParam.szIpAddr );
    len += sprintf( temp + len, "mask=\"%s\";\r\n", bparam.stNetParam.szMask );
    len += sprintf( temp + len, "gateway=\"%s\";\r\n", bparam.stNetParam.szGateway );
    len += sprintf( temp + len, "dns1=\"%s\";\r\n", bparam.stNetParam.szDns1 );
    len += sprintf( temp + len, "dns2=\"%s\";\r\n", bparam.stNetParam.szDns2 );
    len += sprintf( temp + len, "port=%d;\r\n", bparam.stNetParam.nPort );

    //if (byPri == 255){
    //user groups
    for ( i = 1; i < 4; i++ )
    {
        char temp1[9];
        len += sprintf( temp + len, "user%d_name=\"%s\";\r\n", i, bparam.stUserParam[i - 1].szUserName );
        len += sprintf( temp + len, "user%d_pwd=\"%s\";\r\n", i, bparam.stUserParam[i - 1].szPassword );
        //len += sprintf(temp+len,"user%d_pri=%d;\r\n",i,bparam.stUserParam[i-1].byPri);
    }

    //mult device groups
    for ( i = 2; i < 10; i++ )
    {
        len += sprintf( temp + len, "dev%d_host=\"%s\";\r\n", i, bparam.stMultDevice[i - 2].host );
        len += sprintf( temp + len, "dev%d_alias=\"%s\";\r\n", i, bparam.stMultDevice[i - 2].alias );
        len += sprintf( temp + len, "dev%d_user=\"%s\";\r\n", i, bparam.stMultDevice[i - 2].user );
        len += sprintf( temp + len, "dev%d_pwd=\"%s\";\r\n", i, bparam.stMultDevice[i - 2].passwd );
        len += sprintf( temp + len, "dev%d_port=%d;\r\n", i, bparam.stMultDevice[i - 2].port );
    }

    //wifi param groups
    len += sprintf( temp + len, "wifi_enable=%d;\r\n", bparam.stWifiParam.byEnable );
    memset( temp1, 0, sizeof( temp1 ) );
    memset( temp2, 0, sizeof( temp2 ) );
    MyReplace( temp1, bparam.stWifiParam.szSSID, "\"", "%22" );
    len += sprintf( temp + len, "wifi_ssid=\"%s\";\r\n", temp1 );
    len += sprintf( temp + len, "wifi_mode=%d;\r\n", bparam.stWifiParam.byWifiMode );
    len += sprintf( temp + len, "wifi_encrypt=%d;\r\n", bparam.stWifiParam.byEncrypt );
    len += sprintf( temp + len, "wifi_authtype=%d;\r\n", bparam.stWifiParam.byAuthType );
    len += sprintf( temp + len, "wifi_defkey=%d;\r\n", bparam.stWifiParam.byDefKeyType );
    len += sprintf( temp + len, "wifi_keyformat=%d;\r\n", bparam.stWifiParam.byKeyFormat );
    len += sprintf( temp + len, "wifi_key1=\"%s\";\r\n", bparam.stWifiParam.szKey1 );
    len += sprintf( temp + len, "wifi_key2=\"%s\";\r\n", bparam.stWifiParam.szKey2 );
    len += sprintf( temp + len, "wifi_key3=\"%s\";\r\n", bparam.stWifiParam.szKey3 );
    len += sprintf( temp + len, "wifi_key4=\"%s\";\r\n", bparam.stWifiParam.szKey4 );
    len += sprintf( temp + len, "wifi_key1_bits=%d;\r\n", bparam.stWifiParam.byKey1Bits );
    len += sprintf( temp + len, "wifi_key2_bits=%d;\r\n", bparam.stWifiParam.byKey2Bits );
    len += sprintf( temp + len, "wifi_key3_bits=%d;\r\n", bparam.stWifiParam.byKey3Bits );
    len += sprintf( temp + len, "wifi_key4_bits=%d;\r\n", bparam.stWifiParam.byKey4Bits );
    len += sprintf( temp + len, "wifi_wpa_psk=\"%s\";\r\n", bparam.stWifiParam.szShareKey );
    //adsl param groups
    len +=  sprintf( temp + len, "pppoe_enable=%d;\r\n", bparam.stPppoeParam.byEnable );
    len +=  sprintf( temp + len, "pppoe_user=\"%s\";\r\n", bparam.stPppoeParam.szUserName );
    len +=  sprintf( temp + len, "pppoe_pwd=\"%s\";\r\n", bparam.stPppoeParam.szPassword );
    /* BEGIN: Deleted by wupm, 2013/5/3 */
#if	0
    //rtsp param groups
    len +=  sprintf( temp + len, "rtsp_auth_enable=%d;\r\n", bparam.stRtspParam.byEnable );
    len +=  sprintf( temp + len, "rtsp_user=\"%s\";\r\n", bparam.stRtspParam.szUserName );
    len +=  sprintf( temp + len, "rtsp_pwd=\"%s\";\r\n", bparam.stRtspParam.szPassword );
#endif

    //upnp param groups
    len +=  sprintf( temp + len, "upnp_enable=%d;\r\n", bparam.stUpnpParam.byEnable );
    //ddns param groups
    len +=  sprintf( temp + len, "ddns_service=%d;\r\n", bparam.stDdnsParam.byDdnsSvr );
    len +=  sprintf( temp + len, "ddns_proxy_svr=\"%s\";\r\n", bparam.stDdnsParam.szProxySvr );
    len +=  sprintf( temp + len, "ddns_host=\"%s\";\r\n", bparam.stDdnsParam.szDdnsName );
    len +=  sprintf( temp + len, "ddns_user=\"%s\";\r\n", bparam.stDdnsParam.szUserName );
    len +=  sprintf( temp + len, "ddns_pwd=\"%s\";\r\n", bparam.stDdnsParam.szPassword );
    len +=  sprintf( temp + len, "ddns_proxy_port=%d;\r\n", bparam.stDdnsParam.nProxyPort );
    len +=  sprintf( temp + len, "ddns_mode=%d;\r\n", bparam.stDdnsParam.byMode );
    len +=  sprintf( temp + len, "ddns_status=%d;\r\n", bparam.stDdnsParam.dnsstatus );
    //email param groups
    len +=  sprintf( temp + len, "mail_sender=\"%s\";\r\n", bparam.stMailParam.szSender );
    len +=  sprintf( temp + len, "mail_receiver1=\"%s\";\r\n", bparam.stMailParam.szReceiver1 );
    len +=  sprintf( temp + len, "mail_receiver2=\"%s\";\r\n", bparam.stMailParam.szReceiver2 );
    len +=  sprintf( temp + len, "mail_receiver3=\"%s\";\r\n", bparam.stMailParam.szReceiver3 );
    len +=  sprintf( temp + len, "mail_receiver4=\"%s\";\r\n", bparam.stMailParam.szReceiver4 );
    len +=  sprintf( temp + len, "mailssl=%d;\r\n", bparam.stMailParam.byCheck );
    len +=  sprintf( temp + len, "mail_svr=\"%s\";\r\n", bparam.stMailParam.szSmtpSvr );
    len +=  sprintf( temp + len, "mail_user=\"%s\";\r\n", bparam.stMailParam.szSmtpUser );
    len +=  sprintf( temp + len, "mail_pwd=\"%s\";\r\n", bparam.stMailParam.szSmtpPwd );
    len +=  sprintf( temp + len, "mail_port=%d;\r\n", bparam.stMailParam.nSmtpPort );
    len +=  sprintf( temp + len, "mail_inet_ip=%d;\r\n", bparam.stMailParam.byNotify );
    //ftp param groups
    len +=  sprintf( temp + len, "ftp_svr=\"%s\";\r\n", bparam.stFtpParam.szFtpSvr );
    len +=  sprintf( temp + len, "ftp_user=\"%s\";\r\n", bparam.stFtpParam.szFtpUser );
    len +=  sprintf( temp + len, "ftp_pwd=\"%s\";\r\n", bparam.stFtpParam.szFtpPwd );
    len +=  sprintf( temp + len, "ftp_dir=\"%s\";\r\n", bparam.stFtpParam.szFtpDir );
    len +=  sprintf( temp + len, "ftp_port=%d;\r\n", bparam.stFtpParam.nFtpPort );
    len +=  sprintf( temp + len, "ftp_mode=%d;\r\n", bparam.stFtpParam.byMode );
    len +=  sprintf( temp + len, "ftp_upload_interval=%d;\r\n", ( unsigned short )bparam.stFtpParam.nInterTime );
    //alarm param groups
    len +=  sprintf( temp + len, "alarm_motion_armed=%d;\r\n", bparam.stAlarmParam.byMotionEnable );
    len +=  sprintf( temp + len, "alarm_motion_sensitivity=%d;\r\n", bparam.stAlarmParam.bySensitivity );
    len +=  sprintf( temp + len, "alarm_input_armed=%d;\r\n", bparam.stAlarmParam.byAlarmInEnable );
    len +=  sprintf( temp + len, "alarm_ioin_level=%d;\r\n", bparam.stAlarmParam.byAlarmInLevel );
    len +=  sprintf( temp + len, "alarm_mail=%d;\r\n", bparam.stAlarmParam.byAlarmEmail );
    len +=  sprintf( temp + len, "alarm_iolinkage=%d;\r\n", bparam.stAlarmParam.byLinkEnable );
    len +=  sprintf( temp + len, "alarm_ioout_level=%d;\r\n", bparam.stAlarmParam.byAlarmOutLevel );
    len +=  sprintf( temp + len, "alarm_upload_interval=%d;\r\n", bparam.stAlarmParam.byUploadInter );
    len +=  sprintf( temp + len, "alarm_presetsit=%d;\r\n", bparam.stAlarmParam.szPresetSit );
    len +=  sprintf( temp + len, "alarm_snapshot=%d;\r\n", bparam.stAlarmParam.bySnapshot );
    len +=  sprintf( temp + len, "alarm_record=%d;\r\n", bparam.stAlarmParam.byAlarmRecord );


    /* BEGIN: Added by wupm, 2013/6/27 */
#ifdef	PLAY_AUDIO_FILE_ON_ALARM
    len +=  sprintf( temp + len, "enable_alarm_audio=%d;\r\n", bparam.stAlarmParam.reserved & 0x01 );
#endif

    len +=  sprintf( temp + len, "alarm_schedule_enable=%d;\r\n", bparam.stAlarmParam.alarmen );
    len +=  sprintf( temp + len, "alarm_schedule_sun_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[0][0] );
    len +=  sprintf( temp + len, "alarm_schedule_sun_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[0][1] );
    len +=  sprintf( temp + len, "alarm_schedule_sun_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[0][2] );
    len +=  sprintf( temp + len, "alarm_schedule_mon_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[1][0] );
    len +=  sprintf( temp + len, "alarm_schedule_mon_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[1][1] );
    len +=  sprintf( temp + len, "alarm_schedule_mon_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[1][2] );
    len +=  sprintf( temp + len, "alarm_schedule_tue_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[2][0] );
    len +=  sprintf( temp + len, "alarm_schedule_tue_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[2][1] );
    len +=  sprintf( temp + len, "alarm_schedule_tue_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[2][2] );
    len +=  sprintf( temp + len, "alarm_schedule_wed_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[3][0] );
    len +=  sprintf( temp + len, "alarm_schedule_wed_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[3][1] );
    len +=  sprintf( temp + len, "alarm_schedule_wed_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[3][2] );
    len +=  sprintf( temp + len, "alarm_schedule_thu_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[4][0] );
    len +=  sprintf( temp + len, "alarm_schedule_thu_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[4][1] );
    len +=  sprintf( temp + len, "alarm_schedule_thu_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[4][2] );
    len +=  sprintf( temp + len, "alarm_schedule_fri_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[5][0] );
    len +=  sprintf( temp + len, "alarm_schedule_fri_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[5][1] );
    len +=  sprintf( temp + len, "alarm_schedule_fri_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[5][2] );
    len +=  sprintf( temp + len, "alarm_schedule_sat_0=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[6][0] );
    len +=  sprintf( temp + len, "alarm_schedule_sat_1=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[6][1] );
    len +=  sprintf( temp + len, "alarm_schedule_sat_2=%d;\r\n", bparam.stAlarmParam.stAlarmTime.time[6][2] );
#ifdef NEW_BRAOD_AES
	len +=  sprintf( temp + len, "lock1Flag=%d;\r\n", lock1Flag);
	len +=  sprintf( temp + len, "lock2Flag=%d;\r\n", lock2Flag);
	printf(  "lock1Flag=%d;\r\n", lock1Flag);
	printf(  "lock2Flag=%d;\r\n", lock2Flag);
#endif
    //}
    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd        = 0x6002;
    head.len       	= len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
#endif
int p2pgetcameraparams( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    len += sprintf( temp + len, "resolution=%d;\r\n", bparam.stVencParam.bysize );
    len += sprintf( temp + len, "vbright=%d;\r\n", ( bparam.stVencParam.brightness == 0 ) ? 1 : bparam.stVencParam.brightness );
    len += sprintf( temp + len, "vcontrast=%d;\r\n", bparam.stVencParam.contrast );
    len += sprintf( temp + len, "vhue=%d;\r\n", bparam.stVencParam.chroma );
    len += sprintf( temp + len, "vsaturation=%d;\r\n", bparam.stVencParam.saturation );
    len += sprintf( temp + len, "OSDEnable=%d;\r\n", bparam.stVencParam.OSDEnable );
    /* BEGIN: Modified by wupm, 2013/3/2 */
    //len += sprintf( temp + len, "mode=%d;\r\n", bparam.stVencParam.videomode );
    //len += sprintf( temp + len, "flip=%d;\r\n", bparam.stVencParam.videoenv );
    len += sprintf( temp + len, "mode=%d;\r\n", bparam.stVencParam.videoenv );
    len += sprintf( temp + len, "flip=%d;\r\n", bparam.stVencParam.videomode );
    len += sprintf( temp + len, "enc_framerate=%d;\r\n", bparam.stVencParam.byframerate );
    len += sprintf( temp + len, "sub_enc_framerate=%d;\r\n", bparam.stVencParam.byframeratesub );
    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd	= cmd;
    head.len       	= len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int p2pgetmisc( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    len += sprintf( temp + len, "ptz_patrol_rate=%d;\r\n", bparam.stPTZParam.byRate );
    len += sprintf( temp + len, "ptz_patrol_up_rate=%d;\r\n", bparam.stPTZParam.byUpRate );
    len += sprintf( temp + len, "ptz_patrol_down_rate=%d;\r\n", bparam.stPTZParam.byDownRate );
    len += sprintf( temp + len, "ptz_patrol_left_rate=%d;\r\n", bparam.stPTZParam.byLeftRate );
    len += sprintf( temp + len, "ptz_patrol_right_rate=%d;\r\n", bparam.stPTZParam.byRightRate );
    len += sprintf( temp + len, "ptz_center_onstart=%d;\r\n", bparam.stPTZParam.byCenterEnable );
    len += sprintf( temp + len, "ptz_disppreset=%d;\r\n", bparam.stPTZParam.byDisPresent );
    len += sprintf( temp + len, "led_mode=%d;\r\n", bparam.stPTZParam.byLedMode );
    len += sprintf( temp + len, "preset_onstart=%d;\r\n", bparam.stPTZParam.byOnStart );
    len += sprintf( temp + len, "ptruntimes=%d;\r\n", bparam.stPTZParam.byRunTimes );
    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd	= cmd;
    head.len       	= len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}

int p2pgetwifiscan( int sit, short cmd )
{
	Textout("p2pgetwifiscan");
	int num = 0;
	for(num=0; num<5;num++)
	{
		if(1 ==bScanEnd)
		{
			Textout("scanEnd");
			bScanEnd = 0;
			break;
		}
		sleep(1);
			
	}
    unsigned char   temp[1024 * 32];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;

    /* BEGIN: Modified by wupm, 2013/7/1 */
    //char i;
    int i;

    char    ssid[64];
    char    temp1[256];
    char    temp2[18];
    char    temp3[8];
    char    temp4[4];
    char    temp5[4];
    memset( temp, 0x00, 4096 );
    byPri = CheckP2pPri( sit );
	Textout("=======GetWifiResult,CheckP2PPrivage, OK=====, byPri = %d", byPri);

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    //Textout("==========Ready SendBack WifiResult========");
    len = sizeof( CMDHEAD );
    memset( temp, 0x00, 1024 * 32 );
    len += sprintf( temp + len, "var ap_number=%d;\r\n", wificnt );
    len += sprintf( temp + len, "var ap_ssid=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_mode=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_security=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_dbm0=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_dbm1=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_mac=new Array();\r\n" );
    len += sprintf( temp + len, "var ap_channel=new Array();\r\n" );

    for ( i = 0; i < wificnt; i++ )
    {
        if ( i > 31 )
        {
            break;
        }

		//Textout("========SendBack, [%d]===", i);

        memset( ssid, 0x00, 64 );
        memset( temp1, 0x00, 256 );
        memset( temp2, 0x00, 18 );
        memcpy( temp2, WifiResult[i].mac, 17 );
        memset( temp3, 0x00, 8 );
        memcpy( temp3, WifiResult[i].mode, 5 );
        memset( temp4, 0x00, 4 );
        memset( temp5, 0x00, 4 );
        memcpy( temp4, WifiResult[i].db0, strlen( WifiResult[i].db0 ) );
        memcpy( temp5, WifiResult[i].db1, strlen( WifiResult[i].db1 ) );
        //printf("temp4: %s,  temp5: %s\n", temp4, temp5);
        //printf( "chnanel:%d\n", WifiResult[i].channel );

        switch ( WifiResult[i].Authtype )
        {
            case 0x00:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=0;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\"%s\";\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\"%s\";\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;

            case 0x01:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=1;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\"%s\";\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\"%s\";\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;

            case 0x02:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=2;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\"%s\";\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\"%s\";\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;

            case 0x03:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=3;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\"%s\";\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\"%s\";\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;

            case 0x04:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=4;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\"%s\";\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\"%s\";\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;

            case 0x05:
                len += sprintf( temp + len, "ap_ssid[%d] =\"%s\";\r\n", i, WifiResult[i].ssid );
                len += sprintf( temp + len, "ap_mac[%d]=\"%s\";\r\n", i, temp2 );
                len += sprintf( temp + len, "ap_security[%d]=5;\r\n", i );
                len += sprintf( temp + len, "ap_dbm0[%d]=\"%s\";\r\n", i, temp4 );
                len += sprintf( temp + len, "ap_dbm1[%d]=\"%s\";\r\n", i, temp5 );
                len += sprintf( temp + len, "ap_mode[%d]=%d;\r\n", i, WifiResult[i].modetype );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                len += sprintf( temp + len, "ap_channel[%d]=%d;\r\n", i, WifiResult[i].channel );
                break;
        }
    }

    head.startcode 	= 0x0a01;
    head.cmd	= cmd;
    head.len       	= len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int p2pgetfactory( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd	= cmd;
    head.len       	= len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int p2psetirgpio( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		iValue;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    len = sizeof( CMDHEAD );
    iRet = GetIntParamValue( pparam, "val", &iValue );

    if ( iRet == 0 )
    {
        bparam.stPTZParam.gpioir = iValue;
        //SET_IR_GPIO(bparam.stPTZParam.gpioir);
        len += sprintf( temp + len, "result=0;\r\n" );
    }

    else
    {
        len += sprintf( temp + len, "result=-5;\r\n" );
    }

    //camera param groups
    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd	= cmd;
    head.len       	= len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	//iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}

int p2psetalarm( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		iValue;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//    printf("p2psetalarm....p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    memset( &bparam.stAlarmParam, 0x00, sizeof( ALARMPARAM ) );

    //motion
    if ( GetIntParamValue( pparam, "motion_armed", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byMotionEnable = iValue;
    }

    if ( GetIntParamValue( pparam, "motion_sensitivity", &iValue ) != -1 )
    {
        if ( iValue > 10 )
        {
            iValue = 10;
        }

        bparam.stAlarmParam.bySensitivity = iValue;
        //printf( "motion sensit\n" );
    }

    //gpio
    if ( GetIntParamValue( pparam, "input_armed", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byAlarmInEnable = iValue;
        //printf( "input armed\n" );
    }

    if ( GetIntParamValue( pparam, "ioin_level", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byAlarmInLevel = iValue;
        //printf( "in level\n" );
    }

    //gpio link
    if ( GetIntParamValue( pparam, "iolinkage", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byLinkEnable = iValue;
        //printf( "iolink\n" );
    }

#ifdef	ENABLE_TAKE_PICTURE

    /* BEGIN: Added by wupm, 2013/6/16 */
    if ( GetIntParamValue( pparam, "enable_take_pic", &iValue ) != -1 )
    {
        bparam.stAlarmParam.enable_take_pic = iValue;
    }

#endif

    /* BEGIN: Added by wupm, 2013/6/27 */
#ifdef	PLAY_AUDIO_FILE_ON_ALARM

    if ( GetIntParamValue( pparam, "enable_alarm_audio", &iValue ) != -1 )
    {
        if ( iValue == 1 )
        {
            bparam.stAlarmParam.reserved |= 0x01;
        }

        else
        {
            bparam.stAlarmParam.reserved &= 0xFE;
        }
    }

#endif

    if ( GetIntParamValue( pparam, "ioout_level", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byAlarmOutLevel = iValue;
        //printf( "ioout level\n" );
    }

    //alarm email
    if ( GetIntParamValue( pparam, "mail", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byAlarmEmail = iValue;
        //printf( "mail\n" );
    }

    //alarm ftp
    if ( GetIntParamValue( pparam, "upload_interval", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byUploadInter = iValue;
        //printf( "upload_interval\n" );
    }

    //alarm preset
    if ( GetIntParamValue( pparam, "preset", &iValue ) != -1 )
    {
        bparam.stAlarmParam.szPresetSit = iValue;
        //printf( "preset\n" );
    }

    //alarm snapshot
    if ( GetIntParamValue( pparam, "snapshot", &iValue ) != -1 )
    {
        bparam.stAlarmParam.bySnapshot = iValue;
        //printf( "snapshot\n" );
    }

    //alarm record
    if ( GetIntParamValue( pparam, "record", &iValue ) != -1 )
    {
        bparam.stAlarmParam.byAlarmRecord = iValue;
        //printf( "record\n" );
    }

    //alarm en
    if ( GetIntParamValue( pparam, "schedule_enable", &iValue ) != -1 )
    {
        bparam.stAlarmParam.alarmen = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sun_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[0][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sun_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[0][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sun_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[0][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_mon_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[1][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_mon_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[1][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_mon_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[1][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_tue_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[2][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_tue_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[2][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_tue_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[2][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_wed_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[3][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_wed_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[3][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_wed_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[3][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_thu_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[4][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_thu_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[4][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_thu_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[4][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_fri_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[5][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_fri_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[5][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_fri_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[5][2] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sat_0", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[6][0] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sat_1", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[6][1] = iValue;
    }

    if ( GetIntParamValue( pparam, "schedule_sat_2", &iValue ) != -1 )
    {
        bparam.stAlarmParam.stAlarmTime.time[6][2] = iValue;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_ALARM
    myWriteLogIp( 0, LOG_SET_ALARM );
#endif
#endif

    NoteSaveSem();
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd        = cmd;//CGI_IESET_ALARM;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//    printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}


/* BEGIN: Added by wupm, 2013/3/12 */
#if	0	//def	HONESTECH
extern THTCLASS_PARAMETERS	oHTParams;
int P2P_SetHTClassParams( char sit, short cmd )
{
    unsigned char   temp[2048];
    char            temp1[64];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet = 0;
    int				iValue;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
    P2PTextout( "P2P_SetHTClassParams = [%s]", pparam );

    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    memset( &oHTParams.ddns, 0x00, sizeof( THTCLASS_FACTORY_PARAMETERS ) );

    if ( GetIntParamValue( pparam, "ht_ddns_interval", &iValue ) != -1 )
    {
        oHTParams.ddns.ht_ddns_interval = iValue;
    }

    if ( GetIntParamValue( pparam, "ht_ddns_port", &iValue ) != -1 )
    {
        oHTParams.ddns.ht_ddns_port = iValue;
    }

    //oHTParams.ddns.ht_ddns_status = HT_DDNS_FAIL;
    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "deviceModel", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_pid, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_ddns_svr", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_ddns_svr, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_ddns_host", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_ddns_host, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_ddns_user", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_ddns_user, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_ddns_pwd", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_ddns_pwd, temp1 );
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_camerasn", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.ddns.ht_camerasn, temp1 );
    }

    WriteHTClassParams();
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd        = cmd;//CGI_IESET_ALARM;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );
    printf( "p2p send %d iRet %d\n", len, iRet );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2P_SetHTClassAlarm( char sit, short cmd )
{
    unsigned char   temp[2048];
    char            temp1[64];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int				iValue;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
    P2PTextout( "P2P_SetHTClassAlarm = [%s]", pparam );

    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    memset( &oHTParams.alarm, 0x00, sizeof( THTCLASS_ALARM_PARAMETERS ) );

    if ( GetIntParamValue( pparam, "ht_alarm_port", &iValue ) != -1 )
    {
        oHTParams.alarm.ht_alarm_port = iValue;
    }

    if ( GetIntParamValue( pparam, "alarm_enabled", &iValue ) != -1 )
    {
        oHTParams.alarm.alarm_enabled = iValue;
    }

    memset( temp1, 0x00, 64 );

    if ( GetStrParamValue( pparam, "ht_alarm_svr", temp1, 63 ) != -1 )
    {
        strcpy( oHTParams.alarm.ht_alarm_svr, temp1 );
    }

    WriteHTClassParams();
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd        = cmd;//CGI_IESET_ALARM;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );
    printf( "p2p send %d iRet %d\n", len, iRet );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2P_GetHTClass( char sit, short cmd )
{
    unsigned char   temp[2048];
    char            temp1[64];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
    P2PTextout( "P2P_GetHTClass" );

    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    len += sprintf( temp + len, "ht_pid=\"%s\";\r\n", oHTParams.ddns.ht_pid );
    len += sprintf( temp + len, "ht_ddns_svr=\"%s\";\r\n", oHTParams.ddns.ht_ddns_svr );
    len += sprintf( temp + len, "ht_ddns_port=%d;\r\n", oHTParams.ddns.ht_ddns_port );
    len += sprintf( temp + len, "ht_ddns_interval=%d;\r\n", oHTParams.ddns.ht_ddns_interval );
    len += sprintf( temp + len, "ht_ddns_user=\"%s\";\r\n", oHTParams.ddns.ht_ddns_user );
    len += sprintf( temp + len, "ht_ddns_pwd=\"%s\";\r\n", oHTParams.ddns.ht_ddns_pwd );
    len += sprintf( temp + len, "ht_ddns_host=\"%s\";\r\n", oHTParams.ddns.ht_ddns_host );
    len += sprintf( temp + len, "ht_camerasn=\"%s\";\r\n", oHTParams.ddns.ht_camerasn );
    len += sprintf( temp + len, "ht_ddns_status=%d;\r\n", oHTParams.ddns.ht_ddns_status );
    len += sprintf( temp + len, "ht_alarm_svr=\"%s\";\r\n", oHTParams.alarm.ht_alarm_svr );
    len += sprintf( temp + len, "ht_alarm_port=%d;\r\n", oHTParams.alarm.ht_alarm_port );
    len += sprintf( temp + len, "alarm_enabled=%d;\r\n", oHTParams.alarm.alarm_enabled );
    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd	= cmd;
    head.len    = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );
    printf( "p2p send %d iRet %d\n", len, iRet );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
#endif

/* BEGIN: Added by wupm, 2013/5/21 */
/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
#if	0
int P2P_GetOemLxmCfg( char sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );

//                iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );

    len += sprintf( temp + len, "svr0=%s:%d;\r\n", bparam.stExtraParam.szServerIP, bparam.stExtraParam.nServerPort );
    len += sprintf( temp + len, "CameraId=%s;\r\n", bparam.stExtraParam.szCameraID );
    len += sprintf( temp + len, "updateInterval=%d;\r\n", bparam.stExtraParam.nInterval );

    len += sprintf( temp + len, "staticip=%s;\r\n", bparam.stNetParam.szIpAddr );
    len += sprintf( temp + len, "port=%d;\r\n", bparam.stNetParam.nPort );
    len += sprintf( temp + len, "mask=%s;\r\n", bparam.stNetParam.szMask );
    len += sprintf( temp + len, "gateway=%s;\r\n", bparam.stNetParam.szGateway );
    len += sprintf( temp + len, "dns1=%s;\r\n", bparam.stNetParam.szDns1 );
    len += sprintf( temp + len, "dns2=%s;\r\n", bparam.stNetParam.szDns2 );

    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd	= cmd;
    head.len       	= len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );

//        iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}

/* BEGIN: Added by wupm, 2013/5/21 */
int P2P_SetOemLxmCfg( char sit, short cmd )
{
    unsigned char   temp[2048];
    char            temp1[64];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int				iValue;

    char                            nexturl[64];
    BOOL                            bSave = FALSE;
    BOOL							bContinue = FALSE;

    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
    P2PTextout( "P2P_SetOemLxmCfg = [%s]", pparam );

    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );

    //////////////
    iRet = GetIntParamValue( pparam, "updateInterval", &iValue );

    if ( iRet == 0 )
    {
        bparam.stExtraParam.nInterval = iValue;
        memset( nexturl, 0x00, 64 );
        iRet = GetStrParamValue( pparam, "CameraId", nexturl, 63 );

        if ( iRet == 0 && nexturl[0] != 0 )
        {
            strcpy( bparam.stExtraParam.szCameraID, nexturl );
            bContinue = TRUE;
        }

        memset( nexturl, 0x00, 64 );
        iRet = GetStrParamValue( pparam, "svr0", nexturl, 63 );

        if ( iRet == 0 && nexturl[0] != 0 && bContinue )
        {
            int nIndex = 0;
            int nLen = strlen( nexturl );

            for ( nIndex = 0; nIndex < nLen; nIndex++ )
            {
                if ( nexturl[nIndex] == ':' )
                {
                    memset( bparam.stExtraParam.szServerIP, 0, 16 );
                    memcpy( bparam.stExtraParam.szServerIP, nexturl, nIndex );
                    bparam.stExtraParam.nServerPort = atoi( nexturl + nIndex + 1 );
                    P2PTextout( "Server = [%s], Port = %d", bparam.stExtraParam.szServerIP, bparam.stExtraParam.nServerPort );
                    bSave = TRUE;
                    break;
                }
            }
        }
    }

    bContinue = FALSE;
    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "staticip", nexturl, 63 );

    if ( iRet == 0 && nexturl[0] != 0 )
    {
        iRet = CheckIPok( nexturl );

        if ( iRet == 0 )
        {
            bparam.stNetParam.byIsDhcp = 0;
            strcpy( bparam.stNetParam.szIpAddr, nexturl );
            bContinue = TRUE;
        }
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "mask", nexturl, 63 );

    if ( iRet == 0 && nexturl[0] != 0 && bContinue )
    {
        iRet = CheckIPok( nexturl );

        if ( iRet == 0 )
        {
            strcpy( bparam.stNetParam.szMask, nexturl );
            bContinue = TRUE;
        }

        else
        {
            bContinue = FALSE;
        }
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "gateway", nexturl, 63 );

    if ( iRet == 0 && nexturl[0] != 0 && bContinue )
    {
        iRet = CheckIPok( nexturl );

        if ( iRet == 0 )
        {
            strcpy( bparam.stNetParam.szGateway, nexturl );
            bContinue = TRUE;
        }

        else
        {
            bContinue = FALSE;
        }
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "dns1", nexturl, 63 );

    if ( iRet == 0 && nexturl[0] != 0 && bContinue )
    {
        iRet = CheckIPok( nexturl );

        if ( iRet == 0 )
        {
            strcpy( bparam.stNetParam.szDns1, nexturl );
            bContinue = TRUE;
        }

        else
        {
            bContinue = FALSE;
        }
    }

    memset( nexturl, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "dns2", nexturl, 63 );

    if ( iRet == 0 && nexturl[0] != 0 )
    {
        iRet = CheckIPok( nexturl );

        if ( iRet == 0 )
        {
            strcpy( bparam.stNetParam.szDns2, nexturl );
        }
    }

    iRet = GetIntParamValue( pparam, "port", &iValue );

    if ( iRet == 0 && bContinue )
    {
        if ( iValue >= 80 )
        {
            bparam.stNetParam.nPort = iValue;
            bSave = TRUE;
        }
    }

    if ( bSave )
    {
        NoteSaveSem();
    }

    //////////////
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd        = cmd;//CGI_IESET_ALARM;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );
    printf( "p2p send %d iRet %d\n", len, iRet );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
#endif

int p2psetlog( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd        = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
    DoSystem( "rm /param/systemindex.txt" );
    DoSystem( "rm /param/systemlog.txt" );

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_RESET_LOG
    myWriteLogIp( 0, LOG_RESET_LOG );
#endif
#endif

    return 0;
}

int p2psetusers( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    char            temp1[32];
    int             iRet = 0;
    char            passtemp[32];
    char            usertemp[32];
    char            nexturl[64];
    int             value;
    int             iValue;
    char            i;
    char				decoderbuf[128];
    char		temp2[32];
    char		temp3[32];
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//    printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    memset( temp1, 0x00, 32 );
    iRet = GetStrParamValue( pparam, "user1", temp1, 31 );
    memset( decoderbuf, 0x00, 128 );
    URLDecode( temp1, strlen( temp1 ), decoderbuf, 31 );
    memset( temp1, 0x00, 32 );
    strcpy( temp1, decoderbuf );
    //printf("temp1: %s\n", temp1);
    memset( temp2, 0x00, 32 );
    iRet += GetStrParamValue( pparam, "user2", temp2, 31 );
    memset( decoderbuf, 0x00, 128 );
    URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
    strcpy( temp2, decoderbuf );
    //printf("temp2: %s\n", temp2);
    memset( temp3, 0x00, 32 );
    iRet += GetStrParamValue( pparam, "user3", temp3, 31 );
    memset( decoderbuf, 0x00, 128 );
    URLDecode( temp3, strlen( temp3 ), decoderbuf, 63 );
    strcpy( temp3, decoderbuf );
    //printf("temp3: %s\n", temp3);

    if ( ( iRet == 0 ) && ( strlen( temp3 ) ) )
    {
        int iRet1;
        int iRet2;
        iRet = memcmp( temp3, temp1, 32 );

        //printf("cmp %d %d admin %s op %s vistor %s\n",iRet,iRet1,temp3,temp2,temp1);
        if ( iRet )
        {
            //printf("temp3:%s temp2:%s\n",temp3,temp2);
            iRet1 = memcmp( temp3, temp2, 32 );

            if ( iRet1 )
            {
                //printf("temp1:%s temp2:%s\n",temp1,temp2);
                iRet2 = memcmp( temp1, temp2, 32 );

                if ( ( iRet2 ) || ( ( iRet2 == 0 ) && ( strlen( temp1 ) == 0 ) ) )
                {
                    char passwd1[32];
                    char passwd2[32];
                    char passwd3[32];
                    //printf("temp1:%s\n",temp1);
                    memset( bparam.stUserParam[0].szUserName, 0x00, 32 );
                    memset( bparam.stUserParam[1].szUserName, 0x00, 32 );
                    memset( bparam.stUserParam[2].szUserName, 0x00, 32 );
                    strcpy( bparam.stUserParam[0].szUserName, temp1 );
                    strcpy( bparam.stUserParam[1].szUserName, temp2 );
                    strcpy( bparam.stUserParam[2].szUserName, temp3 );
                    memset( bparam.stUserParam[0].szPassword, 0x00, 32 );
                    memset( bparam.stUserParam[1].szPassword, 0x00, 32 );
                    memset( bparam.stUserParam[2].szPassword, 0x00, 32 );
                    memset( passwd1, 0x00, 32 );
                    GetStrParamValue( pparam, "pwd1", passwd1, 31 );
                    memset( decoderbuf, 0x00, 128 );
                    URLDecode( passwd1, strlen( passwd1 ), decoderbuf, 31 );
                    memset( passwd1, 0x00, 32 );
                    strcpy( passwd1, decoderbuf );
                    //printf("passwd1: %s\n", passwd1);
                    memset( passwd2, 0x00, 32 );
                    GetStrParamValue( pparam, "pwd2", passwd2, 31 );
                    memset( decoderbuf, 0x00, 128 );
                    URLDecode( passwd2, strlen( passwd2 ), decoderbuf, 31 );
                    memset( passwd2, 0x00, 32 );
                    strcpy( passwd2, decoderbuf );
                    //printf("passwd2: %s\n", passwd2);
                    memset( passwd3, 0x00, 32 );
                    GetStrParamValue( pparam, "pwd3", passwd3, 31 );
                    memset( decoderbuf, 0x00, 128 );
                    URLDecode( passwd3, strlen( passwd3 ), decoderbuf, 31 );
                    memset( passwd3, 0x00, 32 );
                    strcpy( passwd3, decoderbuf );
                    //printf("passwd3: %s\n", passwd3);
                    strcpy( bparam.stUserParam[0].szPassword, passwd1 );
                    strcpy( bparam.stUserParam[1].szPassword, passwd2 );
                    strcpy( bparam.stUserParam[2].szPassword, passwd3 );
                    NoteSaveSem();
                    len = sizeof( CMDHEAD );
                    len += sprintf( temp + len, "result=0;\r\n" );
                    head.startcode = 0x0a01;
                    //head.version   = 0x0100;
                    head.cmd       = cmd;
                    head.len       = len - sizeof( CMDHEAD );
                    memcpy( temp, &head, sizeof( CMDHEAD ) );
                    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
					iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//                    printf("p2p send %d iRet %d\n",len,iRet);
                    if ( iRet < 0 )
                    {
                        PopP2pSocket( p2pstream[sit].socket );
                        return -1;
                    }

                    return 0;
                }
            }
        }
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_USER
    myWriteLogIp( 0, LOG_SET_USER );
#endif
#endif

    //printf("bparam.stUserParam[0].szUserName: %s, bparam.stUserParam[0].szPassword: %s\n", bparam.stUserParam[0].szUserName, bparam.stUserParam[0].szPassword);
    //printf("bparam.stUserParam[1].szUserName: %s, bparam.stUserParam[1].szPassword: %s\n", bparam.stUserParam[1].szUserName, bparam.stUserParam[1].szPassword);
    //printf("bparam.stUserParam[2].szUserName: %s, bparam.stUserParam[2].szPassword: %s\n", bparam.stUserParam[2].szUserName, bparam.stUserParam[2].szPassword);
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=-1;\r\n" );
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//    printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int p2psetalias( int sit, short cmd )
{
    unsigned char   temp[2048];
    char            szDeviceName[128];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    memset( szDeviceName, 0x00, 128 );
    iRet = GetStrParamValue( pparam, "devalias", szDeviceName, 80 );

    if ( iRet == 0x00 )
    {
        szDeviceName[79] = 0;
        strcpy( bparam.stIEBaseParam.szDevName, szDeviceName );
    }

    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_ALIAS
    myWriteLogIp( 0, LOG_SET_ALIAS );
#endif
#endif

    return 0;
}

int p2psetmail( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		value;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    char				decoderbuf[128];
    char			temp2[64];
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }
    StartAudioPlay(WF_CONFIGOK, 1, NULL);

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    memset( &bparam.stMailParam, 0x00, sizeof( MAILPARAM ) );
    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "sender", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szSender, 0x00, 64 );
        strcpy( bparam.stMailParam.szSender, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet =  GetStrParamValue( pparam, "receiver1", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szReceiver1, 0x00, 64 );
        strcpy( bparam.stMailParam.szReceiver1, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "receiver2", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szReceiver2, 0x00, 64 );
        strcpy( bparam.stMailParam.szReceiver2, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "receiver3", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szReceiver3, 0x00, 64 );
        strcpy( bparam.stMailParam.szReceiver3, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "receiver4", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szReceiver4, 0x00, 64 );
        strcpy( bparam.stMailParam.szReceiver4, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "svr", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szSmtpSvr, 0x00, 64 );
        strcpy( bparam.stMailParam.szSmtpSvr, decoderbuf );
    }

    GetIntParamValue( pparam, "port", &value );
    bparam.stMailParam.nSmtpPort = value;
    memset( temp2, 0x00, 63 );
    iRet = GetStrParamValue( pparam, "user", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szSmtpUser, 0x00, 63 );
        strcpy( bparam.stMailParam.szSmtpUser, decoderbuf );
    }

    memset( temp2, 0x00, 63 );
    iRet = GetStrParamValue( pparam, "pwd", temp2, 63 );
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stMailParam.szSmtpPwd, 0x00, 63 );
        strcpy( bparam.stMailParam.szSmtpPwd, decoderbuf );
    }
    GetIntParamValue( pparam, "mail_inet_ip", &value );
    bparam.stMailParam.byNotify = value;
    GetIntParamValue( pparam, "ssl", &value );
    bparam.stMailParam.byCheck = value;
    NoteSaveSem();
    EmailConfig();
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_EMAIL
    myWriteLogIp( 0, LOG_SET_EMAIL );
#endif
#endif

    return 0;
}

void BaAudioWifiConfigCallback(void)
{
    if(!NowIsAPState())
    {   
        Textout("Sta mode wifi config ok");
        IpcStaModeReconn();
    }
    else
    {   
        Textout("Ap mode wifi config ok");
        SetAPState(FALSE);
    }
}


int p2psetwifi( int sit, short cmd )
{
    unsigned char   temp[2048];
    char            temp1[128];
    char            temp3[64];
    char            temp2[64];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		value;
    char		i, j;
    char		flag = 0;
    char				decoderbuf[128];
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    GetIntParamValue( pparam, "enable", &value );
    bparam.stWifiParam.byEnable = value;
    GetIntParamValue( pparam, "wps_enable", &value );
    bparam.stWifiParam.byWps = value;
    memset( temp2, 0x00, 64 );
    memset( bparam.stWifiParam.wpspin, 0x00, 8 );
    iRet = GetStrParamValue( pparam, "wps_pin", temp2, 8 ); //bparam.stWifiParam.szSSID);

    if ( iRet == 0x00 )
    {
        memset( bparam.stWifiParam.wpspin, 0x00, 8 );
        strcpy( bparam.stWifiParam.wpspin, temp2 );
    }

    memset( bparam.stWifiParam.szSSID, 0x00, 64 );
    memset( bparam.stWifiParam.szShareKey, 0x00, 64 );
    memset( bparam.stWifiParam.szKey1, 0x00, 64 );
    memset( bparam.stWifiParam.szKey2, 0x00, 64 );
    memset( bparam.stWifiParam.szKey3, 0x00, 64 );
    memset( bparam.stWifiParam.szKey4, 0x00, 64 );
    //memset(bparam.stWifiParam.szIpAddr,0x00,16);
    memset( temp1, 0x00, 128 );
    //printf( "pparam: %s\n", pparam );
    iRet = GetStrParamValue( pparam, "ssid", temp1, 63 ); //bparam.stWifiParam.szSSID);

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp1, strlen( temp1 ), decoderbuf, 63 );
        memset( temp3, 0x00, 64 );
        strcpy( temp3, decoderbuf );
        memset( bparam.stWifiParam.szSSID, 0x00, 64 );
        strcpy( bparam.stWifiParam.szSSID, temp3 );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "wpa_psk", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stWifiParam.szShareKey, 0x00, 64 );
        strcpy( bparam.stWifiParam.szShareKey, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "key1", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stWifiParam.szKey1, 0x00, 64 );
        strcpy( bparam.stWifiParam.szKey1, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "key2", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stWifiParam.szKey2, 0x00, 64 );
        strcpy( bparam.stWifiParam.szKey2, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "key3", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stWifiParam.szKey3, 0x00, 64 );
        strcpy( bparam.stWifiParam.szKey3, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "key4", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stWifiParam.szKey4, 0x00, 64 );
        strcpy( bparam.stWifiParam.szKey4, decoderbuf );
    }

    //GetStrParamValue(pparam, "ipaddr", bparam.stWifiParam.szIpAddr);
    GetIntParamValue( pparam, "mode", &value );
    bparam.stWifiParam.byWifiMode = value;
    GetIntParamValue( pparam, "channel", &value );
    bparam.stWifiParam.channel = value;
    //printf( "channel:%d\n", bparam.stWifiParam.channel );
    GetIntParamValue( pparam, "encrypt", &value );
    bparam.stWifiParam.byEncrypt = value;
    GetIntParamValue( pparam, "authtype", &value );
    bparam.stWifiParam.byAuthType = value;
    GetIntParamValue( pparam, "keyformat", &value );
    bparam.stWifiParam.byKeyFormat = value;
    //printf( "keyfromat:%d\n", value );
    GetIntParamValue( pparam, "defkey", &value );
    bparam.stWifiParam.byDefKeyType = value;
    GetIntParamValue( pparam, "key1_bits", &value );
    bparam.stWifiParam.byKey1Bits = value;
    GetIntParamValue( pparam, "key2_bits", &value );
    bparam.stWifiParam.byKey2Bits = value;
    GetIntParamValue( pparam, "key3_bits", &value );
    bparam.stWifiParam.byKey3Bits = value;
    GetIntParamValue( pparam, "key4_bits", &value );
    bparam.stWifiParam.byKey4Bits = value;


    /* BEGIN: Added by wupm, 2013/6/8 */
#if	1

    //authtype = 1 is WEP
    //defkey = 0-3 is KeyIndex
    //key1-4 is Key
    //KeyLen 5,13,16 = ASCII(1)
    //KeyLen 10,26,32 = HEX(0)
    if ( bparam.stWifiParam.byAuthType == 1 )
    {
        int nKeyIndex = bparam.stWifiParam.byDefKeyType;
        int nKeyLength = 0;

        switch ( nKeyIndex )
        {
            case 0:
                nKeyLength = strlen( bparam.stWifiParam.szKey1 );
                break;

            case 1:
                nKeyLength = strlen( bparam.stWifiParam.szKey2 );
                break;

            case 2:
                nKeyLength = strlen( bparam.stWifiParam.szKey3 );
                break;

            case 3:
                nKeyLength = strlen( bparam.stWifiParam.szKey4 );
                break;
        }

        switch ( nKeyLength )
        {
            case 5:
            case 13:
            case 16:
                bparam.stWifiParam.byKeyFormat = 1;	//Ascii
                break;

            case 10:
            case 26:
            case 32:
                bparam.stWifiParam.byKeyFormat = 0;	//Hex
                break;
        }
    }

#endif

    NoteSaveSem();
   
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//    printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_WIFI
    myWriteLogIp( 0, LOG_SET_WIFI );
#endif
#endif

    /*************Exit AP MODE*************/
    /* BEGIN: Added by Baggio.wu, 2014/11/29 */

    StartAudioPlay( WF_CONFIGOK, 1, BaAudioWifiConfigCallback);

	/*************Exit AP MODE*************/

    return 0;
}
int p2pcameracontrol( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;

    /* BEGIN: Modified by wupm, 2013/7/1 */
    //int             iRet, iRet1;
    int             iRet = 0, iRet1 = 0;

    int		iValue;
    unsigned char	camcmd, param;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    iRet = GetIntParamValue( pparam, "param", &iValue );
    camcmd = iValue;
    iRet += GetIntParamValue( pparam, "value", &iValue );
    param = iValue;
    //printf( "camcontrol iRet %d  iRet1 %d value %d param:%s len %d\n", iRet, iRet1, param, pparam, strlen( pparam ) );

    if ( iRet == 0x00 )
    {
        camera_control( camcmd, param );
    }

    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}

int p2psetdate( int sit, short cmd )
{
    unsigned char   temp[2048];
    char            szNtpServer[64];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int             dwCurTime = 0;
    int             bNtpEnable = 0;
    int             byTzSel = 0;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//    printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    memset( szNtpServer, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "ntp_svr", szNtpServer, 64 );

    if ( iRet == 0x00 )
    {
        memset( bparam.stDTimeParam.szNtpSvr, 0x00, 32 );
        memcpy( bparam.stDTimeParam.szNtpSvr, szNtpServer, 32 );
    }

    iRet = GetIntParamValue( pparam, "now", &dwCurTime );

    if ( iRet == 0x00 )
    {
        bparam.stDTimeParam.dwCurTime = ( unsigned int )dwCurTime;
        bparam.stDTimeParam.byIsPCSync = 1;
    }

    else
    {
        bparam.stDTimeParam.byIsPCSync = 0;
    }

    iRet = GetIntParamValue( pparam, "tz", &byTzSel );

    if ( iRet == 0x00 )
    {
        bparam.stDTimeParam.byTzSel = byTzSel;
    }

    iRet = GetIntParamValue( pparam, "ntp_enable", &bNtpEnable );

    if ( iRet == 0x00 )
    {
        bparam.stDTimeParam.byIsNTPServer = bNtpEnable;
        SetNtpRestart( bparam.stDTimeParam.byIsNTPServer );
    }

    PcDateSync();
    NoteSaveSem();
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//    printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_DATE
    myWriteLogIp( 0, LOG_SET_DATE );
#endif
#endif

    return 0;
}

int p2psetddns( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		iValue;
    char            temp1[24];
    char			temp2[64];
    char				decoderbuf[128];
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//    printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    iRet = GetIntParamValue( pparam, "service", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stDdnsParam.byDdnsSvr = iValue;
    }

    iRet = GetIntParamValue( pparam, "enable", &iValue );

    if ( iRet == 0 )
    {
        bparam.stDdnsParam.enable = iValue;
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "user", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stDdnsParam.szUserName, 0x00, 32 );
        strcpy( bparam.stDdnsParam.szUserName, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "pwd", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( bparam.stDdnsParam.szPassword, 0x00, 32 );
        strcpy( bparam.stDdnsParam.szPassword, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "host", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stDdnsParam.szDdnsName, 0x00, 64 );
        strcpy( bparam.stDdnsParam.szDdnsName, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "proxy_svr", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stDdnsParam.szProxySvr, 0x00, 64 );
        strcpy( bparam.stDdnsParam.szProxySvr, decoderbuf );
    }

    iRet = GetIntParamValue( pparam, "proxy_port", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stDdnsParam.nProxyPort = iValue;
    }

    iRet = GetIntParamValue( pparam, "ddns_mode", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stDdnsParam.byMode = iValue;
    }

    SetThirdStatus();
    SetDdnsStart();
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//    printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int p2psetmisc( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		iValue;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );

    if ( GetIntParamValue( pparam, "ptz_center_onstart", &iValue ) != -1 )
    {
        bparam.stPTZParam.byCenterEnable = iValue;
    }

    if ( GetIntParamValue( pparam, "ptz_patrol_rate", &iValue ) != -1 )
    {
        bparam.stPTZParam.byRate = iValue;
    }

    if ( GetIntParamValue( pparam, "ptz_patrol_up_rate", &iValue ) != -1 )
    {
        bparam.stPTZParam.byUpRate = iValue;
    }

    if ( GetIntParamValue( pparam, "ptz_patrol_down_rate", &iValue ) != -1 )
    {
        bparam.stPTZParam.byDownRate = iValue;
    }

    if ( GetIntParamValue( pparam, "ptz_patrol_left_rate", &iValue ) != -1 )
    {
        bparam.stPTZParam.byLeftRate = iValue;
    }

    if ( GetIntParamValue( pparam, "ptz_patrol_right_rate", &iValue ) != -1 )
    {
        bparam.stPTZParam.byRightRate = iValue;
    }

    if ( GetIntParamValue( pparam, "ptz_dispreset", &iValue ) != -1 )
    {
        bparam.stPTZParam.byDisPresent = iValue;
    }

    if ( GetIntParamValue( pparam, "ptz_run_times", &iValue ) != -1 )
    {
        bparam.stPTZParam.byRunTimes = iValue;
    }

    if ( GetIntParamValue( pparam, "led_mode", &iValue ) != -1 )
    {
        bparam.stPTZParam.byLedMode = iValue;
    }

    if ( GetIntParamValue( pparam, "ptz_preset", &iValue ) != -1 )
    {
        bparam.stPTZParam.byOnStart = iValue;
    }

    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int p2pdecodercontrol( int sit, short cmd )
{
	return 0;
}
int p2psetdefault( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    /* BEGIN: Modified by wupm, 2013/1/29   Version:117 */
    //DoSystem( "cp /param/system.ini /param/system-b.ini" );
    DoSystem( "cp /system/www/system.ini /system/www/system-b.ini" );
    DoSystem( "sync" );
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_BACKUP
    myWriteLogIp( 0, LOG_BACKUP );
#endif
	#endif

    return 0;
}

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
#if	0
int p2psetdevices( char sit, short cmd )
{
    unsigned char   temp[2048];
    char            temp1[32];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		iValue;
    char		i;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );

//                iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    memset( &bparam.stMultDevice, 0, sizeof( MULTDEVICE ) * 8 );

    for ( i = 0; i < 8; i++ )
    {
        memset( temp1, 0x00, 24 );
        sprintf( temp1, "dev%d_host", i + 2 );
        /* BEGIN: Modified by wupm, 2013/7/1 */
        //GetStrParamValue( pparam, temp1, &bparam.stMultDevice[i].host );
        GetStrParamValue( pparam, temp1, &bparam.stMultDevice[i].host, 63 );
        memset( temp1, 0x00, 24 );
        sprintf( temp1, "dev%d_alias", i + 2 );
        /* BEGIN: Modified by wupm, 2013/7/1 */
        //GetStrParamValue( pparam, temp1, &bparam.stMultDevice[i].alias );
        GetStrParamValue( pparam, temp1, &bparam.stMultDevice[i].alias, 79 );
        sprintf( temp1, "dev%d_port", i + 2 );
        GetIntParamValue( pparam, temp1, &iValue );
        bparam.stMultDevice[i].port = iValue;
        memset( temp1, 0x00, 24 );
        sprintf( temp1, "dev%d_user", i + 2 );
        /* BEGIN: Modified by wupm, 2013/7/1 */
        //GetStrParamValue( pparam, temp1, &bparam.stMultDevice[i].user );
        GetStrParamValue( pparam, temp1, &bparam.stMultDevice[i].user, 31 );
        memset( temp1, 0x00, 24 );
        sprintf( temp1, "dev%d_pwd", i + 2 );
        /* BEGIN: Modified by wupm, 2013/7/1 */
        //GetStrParamValue( pparam, temp1, &bparam.stMultDevice[i].passwd );
        GetStrParamValue( pparam, temp1, &bparam.stMultDevice[i].passwd, 31 );
    }

    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );

//        iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
#endif

int p2psetnetwork( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		value;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//    printf("p2pgetstatus:%d buf:%s\n",byPri,pparam);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    memset( &bparam.stNetParam, 0x00, sizeof( NETPARAM ) );
    GetStrParamValue( pparam, "ipaddr", bparam.stNetParam.szIpAddr, 15 );
    GetStrParamValue( pparam, "mask", bparam.stNetParam.szMask, 15 );
    GetStrParamValue( pparam, "gateway", bparam.stNetParam.szGateway, 15 );
    GetStrParamValue( pparam, "dns1", bparam.stNetParam.szDns1, 15 );
    GetStrParamValue( pparam, "dns2", bparam.stNetParam.szDns2, 15 );
    GetIntParamValue( pparam, "port", &value );
    bparam.stNetParam.nPort = value;
    GetIntParamValue( pparam, "dhcp", &value );
    bparam.stNetParam.byIsDhcp = value;
    //printf( "ip:%s\n", bparam.stNetParam.szIpAddr );
    //printf( "mask:%s\n", bparam.stNetParam.szMask );
    //printf( "gateway:%s\n", bparam.stNetParam.szGateway );
    P2PTextout( "dns1:%s\n", bparam.stNetParam.szDns1 );
    P2PTextout( "dns2:%s\n", bparam.stNetParam.szDns2 );

    NoteSaveSem();
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
    NoteSaveSem();

//    printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_NETWORK
    myWriteLogIp( 0, LOG_SET_NETWORK );
#endif
#endif

    return 0;
}
int p2psetdns( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		iValue;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//        printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    memset( &bparam.stDdnsParam, 0x00, sizeof( DDNSPARAM ) );
    iRet = GetIntParamValue( pparam, "service", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stDdnsParam.byDdnsSvr = iValue;
    }

    iRet = GetIntParamValue( pparam, "enable", &iValue );

    if ( iRet == 0 )
    {
        bparam.stDdnsParam.enable = iValue;
    }

    /* BEGIN: Modified by wupm, 2013/7/1 */
    /*
    GetStrParamValue( pparam, "user", bparam.stDdnsParam.szUserName );
    GetStrParamValue( pparam, "pwd", bparam.stDdnsParam.szPassword );
    GetStrParamValue( pparam, "host", bparam.stDdnsParam.szDdnsName );
    GetStrParamValue( pparam, "proxy_svr", bparam.stDdnsParam.szProxySvr );
    */
    GetStrParamValue( pparam, "user", bparam.stDdnsParam.szUserName, 31 );
    GetStrParamValue( pparam, "pwd", bparam.stDdnsParam.szPassword, 31 );
    GetStrParamValue( pparam, "host", bparam.stDdnsParam.szDdnsName, 63 );
    GetStrParamValue( pparam, "proxy_svr", bparam.stDdnsParam.szProxySvr, 63 );


    iRet = GetIntParamValue( pparam, "proxy_port", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stDdnsParam.nProxyPort = iValue;
    }

    iRet = GetIntParamValue( pparam, "ddns_mode", &iValue );

    if ( iRet == 0x00 )
    {
        bparam.stDdnsParam.byMode = iValue;
    }

    SetDdnsStart();
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}

void BaAudioRebootCallback(void)
{
	SetRebootCgi();
}

int p2preboot( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );

	/* BEGIN: Invalid Parameters, So System Halt!!! */
	/*        Deleted by wupm(2073111@qq.com), 2014/10/20 */
    //SetRebootCgi();

    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//        printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_REBOOT
    myWriteLogIp( 0, LOG_REBOOT );
#endif
#endif

    if(bPrevSetWifiCgi == TRUE)
    {
        Textout("############Set wifi should not reboot##############");
        bPrevSetWifiCgi = FALSE;
    }
    else     
		StartAudioPlay(WF_REBOOT, 1, BaAudioRebootCallback);
	return 0;
}
int p2psetwifiscan( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //GetWifiScan();
    //wifiScanSit = sit;
	//wifiScanCmd = cmd;
    StartWifiScan();
    //cgiwaittimeout(5);    
    p2pgetwifiscan( sit, cmd );
    
    return 0;
}

int p2pcheckuser( int sit, short cmd )
{
    unsigned char   	temp[2048];
    int             		len = 0;
    int             		byPri = 0;
    CMDHEAD         	head;
    int             		iRet;
    unsigned char*  	 pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
	//0=ERROR
	//-1=Already Used
	//1-8=Other, OK
	//101-108=Admin, OK

    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }
	
/* BEGIN: Added by yiqing, 2015/4/14 */
#ifndef BAFANGDIANZI 
	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/25 */
	if ( byPri == -1 )
	{
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-2" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }
	/*
		else
		{
            sleep(1);
			PopP2pSocket( p2pstream[sit].socket );
		}
	*/

        return 0;
    }
#endif


	if ( byPri == -3 )
	{
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-3" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }
        /* modify begin by yiqing, 2015-07-07, 原因: */
		//PopP2pSocket( p2pstream[sit].socket );
        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
	#ifdef NEW_BRAOD_AES
	len += sprintf( temp + len, "type=1;\r\n" );
	#endif
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
//    printf("p2p send %d iRet %d\n",len,iRet);
    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}

int p2prestorefactory( int sit, short cmd )
{
	#if	0
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//    printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );

//        iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_RESTORE
    myWriteLogIp( 0, LOG_RESTORE );
#endif
#endif

    //camera param groups
    /* BEGIN: Modified by wupm, 2013/1/29   Version:117 */
    //DoSystem( "rm /param/system.ini" );
    DoSystem( "rm /system/www/system.ini" );
    SetRebootCgi();
#endif
    /* BEGIN: Added by wupm, 2013/7/1 */
    return 0;
}

int p2psetftp( int sit, short cmd )
{
    unsigned char   temp[2048];
    char            temp1[64];
    char            temp2[32];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		iValue;
    char				decoderbuf[128];
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

//    printf("p2pgetstatus:%d\n",byPri);
    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_CMDCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    memset( &bparam.stFtpParam, 0x00, sizeof( FTPPARAM ) );
    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "svr", temp2, 63 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
        memset( bparam.stFtpParam.szFtpSvr, 0x00, 64 );
        strcpy( bparam.stFtpParam.szFtpSvr, decoderbuf );
    }

    GetIntParamValue( pparam, "port", &iValue );
    bparam.stFtpParam.nFtpPort = iValue;
    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "user", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( bparam.stFtpParam.szFtpUser, 0x00, 32 );
        strcpy( bparam.stFtpParam.szFtpUser, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "pwd", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( bparam.stFtpParam.szFtpPwd, 0x00, 32 );
        strcpy( bparam.stFtpParam.szFtpPwd, decoderbuf );
    }

    memset( temp2, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "dir", temp2, 31 );

    if ( iRet == 0x00 )
    {
        memset( decoderbuf, 0x00, 128 );
        URLDecode( temp2, strlen( temp2 ), decoderbuf, 31 );
        memset( bparam.stFtpParam.szFtpDir, 0x00, 32 );
        strcpy( bparam.stFtpParam.szFtpDir, decoderbuf );

        if ( decoderbuf[0] == 0 )
        {
            strcpy( bparam.stFtpParam.szFtpDir, "/" );
        }
    }

    GetIntParamValue( pparam, "mode", &iValue );
    bparam.stFtpParam.byMode = iValue;
    iValue = 0;
    GetIntParamValue( pparam, "interval", &iValue );
    bparam.stFtpParam.nInterTime = iValue;
    NoteSaveSem();
    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );
    //printf( "p2p send %d iRet %d\n", len, iRet );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_SET_FTP
    myWriteLogIp( 0, LOG_SET_FTP );
#endif
#endif

    return 0;
}

#if	1
/* BEGIN: Add New CGI: get_doorbelllogs.cgi */
/*        Added by wupm(2073111@qq.com), 2014/8/11 */
#ifdef	ENABLE_BELL_LOG
#define	MAX_BELL_LOG_COUNT	32
typedef	struct
{
	char szID[32];
	char szUser[32];
	int	 nStatus;
}BELL_LOGITEM;

#define	LOG_STATUS_TIMEOUT	0
#define	LOG_STATUS_AGREE	1
#define	LOG_STATUS_ALARM	2

typedef	struct
{
	int nCount;
	int nFirst;
	BELL_LOGITEM oItems[MAX_BELL_LOG_COUNT * 2];
}BELL_LOG;

/* END:   Added by wupm(2073111@qq.com), 2014/8/11 */
BELL_LOG BellLog;
static BOOL bBellLogInited = FALSE;
#define	BELL_LOG_FILENAME	"/system/www/BellLog.dat"
pthread_mutex_t	mutex_BellLog = PTHREAD_MUTEX_INITIALIZER;
static void LockBellLog()
{
	pthread_mutex_lock( &mutex_BellLog );
}
static void UnlockBellLog()
{
	pthread_mutex_unlock( &mutex_BellLog );
}
static void ReadBellFile()
{
	LockBellLog();
	if ( 1 )
	{
		FILE *f = fopen(BELL_LOG_FILENAME, "rb");
		if ( f != NULL )
		{
			fread(&BellLog, 1, sizeof(BELL_LOG), f);
			fclose(f);
		}
		else
		{
			memset(&BellLog, 0, sizeof(BELL_LOG));
		}
	}
	UnlockBellLog();
}
static void WriteBellFile()
{
	LockBellLog();
	if ( 1 )
	{
		FILE *f = fopen(BELL_LOG_FILENAME, "wb");
		if ( f != NULL )
		{
			fwrite(&BellLog, 1, sizeof(BELL_LOG), f);
			fclose(f);
		}
	}
	UnlockBellLog();
}
static void DeleteJPEGFile(const char *szTime)
{
	if ( szTime[0] != 0 )
	{
		char szCommand[256];
		memset(szCommand, 0, 256);
		sprintf(szCommand, "rm -f /tmp/%s.jpg", szTime);
		P2PTextout("Delete Temp JPEG File = [%s]", szCommand);
		DoSystem(szCommand);
	}
}
void AppendBellLog(const char *szTime)
{
	if ( !bBellLogInited )
	{
		bBellLogInited = TRUE;
		ReadBellFile();
	}
	
	if ( BellLog.nCount < MAX_BELL_LOG_COUNT )
	{
		memset(&BellLog.oItems[BellLog.nCount],0, sizeof(BELL_LOGITEM));
		strcpy(BellLog.oItems[BellLog.nCount].szID, szTime);
		BellLog.nCount++;
	}
	else if ( BellLog.nFirst < MAX_BELL_LOG_COUNT )
	{
		DeleteJPEGFile(BellLog.oItems[BellLog.nFirst].szID);
		memset(&BellLog.oItems[BellLog.nCount+BellLog.nFirst],0, sizeof(BELL_LOGITEM));
		strcpy(BellLog.oItems[BellLog.nCount+BellLog.nFirst].szID, szTime);
		BellLog.nFirst++;
	}
	else
	{
		int i=0;

		DeleteJPEGFile(BellLog.oItems[BellLog.nFirst].szID);
		BellLog.nFirst = 0;

		for(i=0;i<MAX_BELL_LOG_COUNT-1;i++)
		{
			memcpy(&BellLog.oItems[i], &BellLog.oItems[MAX_BELL_LOG_COUNT+1+i], sizeof(BELL_LOGITEM));
		}

		memset(&BellLog.oItems[MAX_BELL_LOG_COUNT-1], 0, sizeof(BELL_LOGITEM));
		strcpy(BellLog.oItems[MAX_BELL_LOG_COUNT-1].szID, szTime);
	}
	
	WriteBellFile();
	P2PTextout("OK, nCount = %d, nFirst = %d", BellLog.nCount, BellLog.nFirst);
}
void ChangeBellLog(const char *szTime, const char *szUser, int nStatus)
{
	//Find context szID
	//if changed, change it
	//save
	int i;
	for(i=BellLog.nCount-1;i>=0;i--)
	{
		if ( strcmp(szTime, BellLog.oItems[i+BellLog.nFirst].szID) == 0 )
		{
			strcpy(BellLog.oItems[i+BellLog.nFirst].szUser, szUser);
			BellLog.oItems[i+BellLog.nFirst].nStatus = nStatus;
			WriteBellFile();
			break;
		}
	}
}
/* add begin by yiqing, 2015-07-31, 原因: */
void DeleteAllBellLogs()
{
    int i = 0;
    int bDeleteFlag = 0;
    
    for ( i = 0 ; i< BellLog.nCount  + BellLog.nFirst; i++ )
	{
        DeleteJPEGFile(BellLog.oItems[i].szID);
	}

    memset(&BellLog,0,sizeof(BELL_LOG));
    WriteBellFile();

    
}
void DeleteBellLog(const char *szTime)
{
    Textout("szTime:%s",szTime);
    int i = 0;
    int bDeleteFlag = 0;

    for ( i = 0 ; i< BellLog.nCount  + BellLog.nFirst; i++ )
	{
		if( strcmp(szTime,BellLog.oItems[i].szID) == 0 )
		{
            DeleteJPEGFile(BellLog.oItems[i].szID);
            memset(&BellLog.oItems[i], 0, sizeof(BELL_LOGITEM));
            bDeleteFlag = 1;
            if(i < MAX_BELL_LOG_COUNT)
            {
                BellLog.nCount--;
            }
            else
            {
                BellLog.nFirst--;
            }
            break;
        }
	}
    

    for ( i = 0 ; i< BellLog.nCount  + BellLog.nFirst; i++ )
	{
		if( strcmp(szTime,BellLog.oItems[i].szID) == 0 )
		{
            DeleteJPEGFile(BellLog.oItems[i].szID);
            memset(&BellLog.oItems[i], 0, sizeof(BELL_LOGITEM));
            bDeleteFlag = 1;
            if(i < MAX_BELL_LOG_COUNT)
            {
                BellLog.nCount--;
            }
            else
            {
                BellLog.nFirst--;
            }
            break;
        }
	}
    if(1 == bDeleteFlag)
    {
        for( ;i< BellLog.nCount  + BellLog.nFirst -2; i++)
        {
            memcpy(&BellLog.oItems[i], &BellLog.oItems[1+i], sizeof(BELL_LOGITEM));
        }
        memset(&BellLog.oItems[i + 1], 0, sizeof(BELL_LOGITEM));
        WriteBellFile();
    }

}
/* add end by yiqing, 2015-07-31 */

void GetBellLog()
{
	//if not initialize, do it
	//copy all data to Cache-buffer in memory
	if ( !bBellLogInited )
	{
		bBellLogInited = TRUE;
		ReadBellFile();
	}
}
#else
#define	MAX_JPEG_LIST	64
typedef struct
{
	char szPath[64];
	int	t;
}JPEGLIST;
JPEGLIST oJpegList[MAX_JPEG_LIST];
#define	MAX_JPEG_RESERVED_TIME	300		//Reserved 10 Min
void* OnCheckJpegList( void* p )
{
	int i;
	char szCommand[256] = {0};
	while(1)
	{
		sleep(2);
		for(i=0;i<MAX_JPEG_LIST;i++)
		{
			if ( oJpegList[i].szPath[0] == 0 )
			{
				continue;
			}
			else
			{
				oJpegList[i].t ++;
				if ( oJpegList[i].t >= MAX_JPEG_RESERVED_TIME )
				{
					P2PTextout("Delete Temp JPEG File = [%s]", oJpegList[i].szPath);
					memset(szCommand, 0, 256);
					sprintf(szCommand, "rm -f %s", oJpegList[i].szPath);
					DoSystem(szCommand);
					oJpegList[i].szPath[0] = 0;
					oJpegList[i].t = 0;
				}
			}
		}
	}
}
void AppendJpegList(char *szPath)
{
	int i;
	static pthread_t threadid = 0;
	if ( threadid == 0 )
	{
		memset(oJpegList, 0, sizeof(JPEGLIST) * MAX_JPEG_LIST);
	}
	for ( i=0; i<MAX_JPEG_LIST; i++)
	{
		if ( oJpegList[i].szPath[0] == 0 )
		{
			strcpy(oJpegList[i].szPath, szPath);
			oJpegList[i].t = 0;
			break;
		}
	}
	if ( threadid == 0 )
	{
		pthread_create(&threadid, NULL, &OnCheckJpegList, NULL);
		pthread_detach(threadid);
	}
}
#endif
#endif


/* BEGIN: Change BIZ Server String */
/*        Added by wupm(2073111@qq.com), 2014/9/5 */
BOOL		bMsgPushing = FALSE;
char msg_dwDeviceID[64] = {0};
char msg_dwSzTitle[128] = {0};
char msg_sendData[128] = {0};
void *OnMsgPush(void *p)
{
	int iRet = 0;
    int nRet = 0;


     
#ifdef LDPUSH
    Textout("**************************GetDnsIP*****************************");
    nRet = GetDnsIp( JPUSH_HOST_NAME, jpush_address);
    Textout("Server = [%s], IPADDR = [%s]", JPUSH_HOST_NAME, jpush_address);
    if(nRet == -1){
        jpush_address[0] = 0;
    }

    nRet = GetDnsIp( YPUSH_HOST_NAME, ypush_address);
    Textout("Server = [%s], IPADDR = [%s]", YPUSH_HOST_NAME, ypush_address);
    if(nRet == -1){
        ypush_address[0] = 0;
    }

    nRet = GetDnsIp( XPUSH_HOST_NAME, xpush_address);
    Textout("Server = [%s], IPADDR = [%s]", XPUSH_HOST_NAME, xpush_address);
    if(nRet == -1){
        xpush_address[0] = 0;
    }

#endif
	


	#ifdef JPUSH
	nRet = GetDnsIp( JPUSH_HOST_NAME, jpush_address);
	Textout("Server = [%s], IPADDR = [%s]", JPUSH_HOST_NAME, jpush_address);
	if(nRet == -1){
		jpush_address[0] = 0;
	}

	//jPushInit();
	#endif	
	

	while(1)
	{
		if ( !bMsgPushing )
		{
			sleep(1);
			continue;
		}

		if ( msg_dwDeviceID[0] == 0 || msg_dwSzTitle[0] == 0 )
		{
			Textout("MSG Parameters ERROR!!!!");
			bMsgPushing = FALSE;
			sleep(1);
			continue;
		}

		bMsgPushing = FALSE;

#ifdef LDPUSH
		iRet = LdPush(msg_dwSzTitle,1);
#endif
     
#ifdef JPUSH
		memset(msg_sendData,0,sizeof(msg_sendData));
		sprintf(msg_sendData,"j_%s",msg_dwSzTitle);
		Textout("----------Call jPush--------------------");
		iRet = jPush(msg_sendData);
		Textout("----------jPush([%s], [%s]), Return = %d", msg_dwDeviceID, msg_sendData, iRet);
#endif

		/* modify begin by yiqing, 2016-06-08 */
		//去掉原因:在ntp没对时的情况下，调用Biz_MsgPush会堵塞不返回
		#if 0
		memset(msg_sendData,0,sizeof(msg_sendData));
		sprintf(msg_sendData,"b_%s",msg_dwSzTitle);
		Textout("----------Call Biz_MsgPush--------------");
		iRet = Biz_MsgPush(msg_dwDeviceID, msg_sendData, "Hello");
		Textout("----------Biz_MsgPush([%s], [%s]), Return = %d", msg_dwDeviceID, msg_sendData, iRet);
		#endif
		msg_dwDeviceID[0] = 0;
		msg_dwSzTitle[0] = 0;
		
	}

	return 0;
}
void MsgPush(char *dwDeviceID, char *szTitle)
{
	if ( !bMsgPushing )
	{
		strcpy(msg_dwDeviceID, dwDeviceID);
		strcpy(msg_dwSzTitle, szTitle);
		bMsgPushing = TRUE;
	}
	else
	{
		Textout("Msg NOW Pushing, So Lose this information!!!!");
	}
}

void MsgPushInit()
{
	pthread_t threadid;
	pthread_create(&threadid, NULL, &OnMsgPush, NULL);
	pthread_detach(threadid);
}

int OnDoorP2PAlarm(int sit, char *szid, int type)
{
    int writesize = 0;
    int readsize = 0;
    int i;
	int iRet;

    char temp[1024];
	memset(temp, 0, 1024);

    CMDHEAD head;
    memset( &head, 0, sizeof( head ) );
    head.startcode = 0x0a01;
    head.cmd  = CGI_ALARM_NOTIFY;

	sprintf(temp + sizeof( CMDHEAD ),
		"%s,%s,%d",
		szid,
		bparam.stIEBaseParam.dwDeviceID,
		type);

	head.len = strlen(temp + sizeof(CMDHEAD)) + 1;
	memcpy( temp, ( char* )&head, sizeof( CMDHEAD ) );
    for ( i = 0; i < MAX_P2P_CONNECT; i++ )
    {
		P2PTextout("i = %d, sit = %d, socket = %d, type = %d", i, sit, p2pstream[i].socket, type);
		if ( sit != -1 )
		{
			if ( i != sit )
			{
				continue;
			}
		}

        if ( p2pstream[i].socket == -1 )
        {
            continue;
        }

        iRet = PPPP_Check_Buffer( p2pstream[i].socket, P2P_ALARMCHANNEL, &writesize, &readsize );

        if ( iRet < 0 )
        {
            P2PTextout("PPPP_Check_Buffer socket=%d, iRet=%d", p2pstream[i].socket, iRet);
            PopP2pSocket( p2pstream[i].socket );
            continue;
        }

        if ( writesize >= 8 * 1024 )
        {
			P2PTextout("CheckBuffer AlarmChannel writesize = %d", writesize);
            continue;
        }

		switch(type)
		{
			case BELL_WATH_WAIT_TIMEOUT:
				P2PTextout("Watch Timeout [%d], State = %d, Set State to PS_IDLE", i, p2pstream[i].stateEx);
				p2pstream[i].stateEx = PS_IDLE;
				break;

			case BELL_CALL:
				P2PTextout("Call [%d], Set State to PS_CALLING", i);
				p2pstream[i].state = PS_CALLING;
				break;
			case BELL_ALARM:
				break;
			case BELL_AGREE:
				P2PTextout("Some one Agree, Notify Other [%d], State = %d, Set State to PS_IDLE", i, p2pstream[i].state);
				p2pstream[i].state = PS_IDLE;
				break;
			case BELL_CALL_WAIT_TIMEOUT:
				P2PTextout("Call Wait Timeout, Notify Remote [%d], State = %d, Set State to PS_IDLE", i, p2pstream[i].state);
				p2pstream[i].state = PS_IDLE;
				break;
			case BELL_CALL_TIMEOUT:
				P2PTextout("Call Timeout, Notify Remote [%d], State = %d, Set State to PS_IDLE", i, p2pstream[i].state);
				p2pstream[i].state = PS_IDLE;
				break;
			//case BELL_WATH_STOP:
			//	break;
			case BELL_RESET_TIMER:
				break;
			case BELL_AGREE_ME:
				break;
		}
		

		if ( type != BELL_AGREE_ME )
		{
        	iRet = PPPP_WriteEx(p2pstream[i].socket, P2P_ALARMCHANNEL, temp, head.len + sizeof( CMDHEAD ) );
			P2PTextout( "Send msg (%d) = [%s]", i, &temp[8] );
            if ( iRet < 0 )
            {
                PopP2pSocket( p2pstream[i].socket );
                continue;
            }			
		}
    }

	#ifdef	ENABLE_BELL_LOG
	switch(type)
	{
		case BELL_ALARM:
			AppendBellLog(szid);
			ChangeBellLog(szid, "", LOG_STATUS_ALARM);
			break;
		case BELL_AGREE_ME:
			P2PTextout("-----------Agree, User=[%s]", p2pstream[sit].szUser);
			ChangeBellLog(szid, p2pstream[sit].szUser, LOG_STATUS_AGREE);
			break;
	}
	#endif

	//Only Call and Alarm
	if ( type == BELL_ALARM || type == BELL_CALL )
	{
		char szTitle[128] = {0};
		if ( type == BELL_ALARM )
			sprintf(szTitle, "Alarming,%s", temp + sizeof(head));
		else
			sprintf(szTitle, "Calling,%s", temp + sizeof(head));

		//MsgPush(bparam.stIEBaseParam.dwDeviceID, szTitle);
	}

    return 1;
}

/* END:   Added by wupm, 2013/11/1 */

#ifndef	GET_RECORDLIST_PAGE_BY_PAGE
int p2pgetrecordfile( int sit, short cmd )
{
    unsigned char   	temp[1024 * 64];
    int             		len = 0;
    int             		byPri = 0;
    CMDHEAD         	head;
    int             		iRet;
    int				recordcnt = 0;
    FILE*           		 frecord = NULL;
    unsigned char   	temp1[128];
    char            		temp2[24];
    char            		temp3[128];
    unsigned int		filelen;
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
    //printf( "getrecordfile %d\n", byPri );

    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
        iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    len += sprintf( temp + len, "var record_name0=new Array();\r\n" );
    len += sprintf( temp + len, "var record_size0=new Array();\r\n" );
    DoSystem( "ls /mnt/*.h260 > /tmp/getfile.txt" );
    frecord = fopen( "/tmp/getfile.txt", "r" );

    if ( frecord != NULL )
    {
        while ( !feof( frecord ) )
        {
            memset( temp1, 0x00, 128 );
            fgets( temp1, 256, frecord );
            memset( temp2, 0x00, 24 );
            memcpy( temp2, temp1 + 5, 23 );
            memset( temp3, 0x00, 128 );
            filelen = 0x00;
            //printf("temp1:%s\n",temp1);
            temp1[28] = 0x00;
            filelen = GetFileSize( temp1 );
            len += sprintf( temp + len, "record_name0[%d]=\"%s\";\r\n", recordcnt, temp2 );
            len += sprintf( temp + len, "record_size0[%d]=%d;\r\n", recordcnt, filelen );
            recordcnt++;

            if ( len >= 1024 * 60 )
            {
                break;
            }
        }

        fclose( frecord );
    }

    if ( recordcnt > 0 )
    {
        recordcnt--;
    }

    len += sprintf( temp + len, "var record_num0=%d;\r\n", recordcnt );
    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd	= cmd;
    head.len       	= len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
    iRet = PPPP_WriteEx( p2pstream[sit].socket,P2P_CMDCHANNEL, temp, len );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
#else
int p2pgetrecordfile( int sit, short cmd )
{
    unsigned char   	temp[1024 * 128];
    int             		len = 0;
    int             		byPri = 0;
    CMDHEAD         	head;
    int             		iRet;
    int				recordcnt = 0;
    FILE*           		 frecord = NULL;
    unsigned char   	temp1[128];
    char            		temp2[24];
    char            		temp3[128];
    unsigned int		filelen;

    /* BEGIN: Added by wupm, 2013/1/31   Version:159 */
#define	MAX_PAGE_SIZE	4096
    int	m_nPageIndex = 0;			//IN(Default = 1), OUT
    int m_nPageSize = 0;			//IN(Default = 0), OUT
    int m_nPageCount = 0;			//OUT
    int m_nRecordCount = 0;			//OUT

    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    memset( temp, 0x00, 1024 * 64 );
    byPri = CheckP2pPri( sit );
    //printf( "getrecordfile %d\n", byPri );

    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
        iRet = PPPP_WriteEx( p2pstream[sit].socket,P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    len += sprintf( temp + len, "var record_name0=new Array();\r\n" );
    len += sprintf( temp + len, "var record_size0=new Array();\r\n" );

    /* BEGIN: Deleted by wupm, 2013/2/21 */
    /*
    Lock_GetRecordList();
    DoSystem( "ls /mnt/*.h264 > /tmp/getfile.txt" );
    UnLock_GetRecordList();
    */
    /* END: Deleted by wupm, 2013/2/21 */

    /* BEGIN: Added by wupm, 2013/1/31 */

    //Get PageSize

    iRet = GetIntParamValue( pparam, "PageSize", &m_nPageSize );

    if ( iRet != 0x00 )
    {
        m_nPageSize = 0;
    }

    else if ( m_nPageSize >= 0 && m_nPageSize <= MAX_PAGE_SIZE )
    {
    }
    else
    {
        m_nPageSize = 0;
    }

    //Get PageIndex

    iRet = GetIntParamValue( pparam, "PageIndex", &m_nPageIndex );

    if ( iRet != 0x00 )
    {
        m_nPageIndex = 0;
    }

    else if ( m_nPageIndex >= 0 )
    {

    }
    else
    {
        m_nPageIndex = 0;
    }

    //Set to Default Value

    if ( m_nPageSize == 0 )
    {
        m_nPageSize = MAX_PAGE_SIZE;
        m_nPageIndex = 0;
    }

    /* BEGIN: Added by wupm, 2013/2/19 */
    Lock_GetRecordFile();

    /* BEGIN: Deleted by wupm, 2013/2/21 */
    if ( 1 )
    {
        int nStartIndex = m_nPageIndex * m_nPageSize;
        int nStopIndex = nStartIndex + m_nPageSize;
        int nRecordCount = GetRecordFileCount();
        //int nRecordCount = gRecordFileList.nCount;
        int nIndex = 0;
        char* pFile = NULL;

        for ( nIndex = nStartIndex; nIndex < nRecordCount && nIndex < nStopIndex; nIndex++, recordcnt++ )
        {
            pFile = GetRecordFile( nIndex );

            if ( pFile == NULL )
            {
                continue;
            }

            len += sprintf( temp + len, "record_name0[%d]=\"%s\";\r\n",
                            nIndex - nStartIndex,
                            pFile
                            //gRecordFileList.oRecordFile[nIndex].szFile
                          );
            len += sprintf( temp + len, "record_size0[%d]=%d;\r\n",
                            nIndex - nStartIndex,
                            GetRecordFileSize( nIndex )
                            //gRecordFileList.oRecordFile[nIndex].nSize
                          );

            if ( len >= 1024 * 120 )
            {
                m_nPageSize = recordcnt;
                break;
            }
        }

        m_nRecordCount = nRecordCount;
        m_nPageCount = ( m_nRecordCount + m_nPageSize - 1 ) / m_nPageSize;
    }

    /* END: Deleted by wupm, 2013/2/21 */

    UnLock_GetRecordFile();

    P2PTextout( "m_nPageIndex = %d, m_nPageSize = %d, m_nRecordCount = %d, m_nPageCount = %d",
             m_nPageIndex, m_nPageSize, m_nRecordCount, m_nPageCount );

    /* END:   Added by wupm, 2013/1/31 */

    len += sprintf( temp + len, "var record_num0=%d;\r\n", recordcnt );
    len += sprintf( temp + len, "var PageIndex=%d;\r\n", m_nPageIndex );
    len += sprintf( temp + len, "var PageSize=%d;\r\n", m_nPageSize );
    len += sprintf( temp + len, "var RecordCount=%d;\r\n", m_nRecordCount );
    len += sprintf( temp + len, "var PageCount=%d;\r\n", m_nPageCount );

    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd		= cmd;
    head.len       	= len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
    iRet = PPPP_WriteEx( p2pstream[sit].socket,P2P_CMDCHANNEL, temp, len );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}

#endif


/* BEGIN: Added by wupm, 2013/1/11 */
int p2pgetrecord( int sit, short cmd )
{
    unsigned char   	temp[1024 * 64];
    int             		len = 0;
    int             		byPri = 0;
    CMDHEAD         	head;
    int             		iRet;
    int				recordcnt = 0;
    FILE*           		 frecord = NULL;
    unsigned char   	temp1[128];
    char            		temp2[24];
    char            		temp3[128];
    unsigned int		filelen;
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );
    //printf( "getrecordfile %d\n", byPri );

    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    //camera param groups
    len = sizeof( CMDHEAD );
    len += sprintf( temp + len, "result=0;\r\n" );
    len += sprintf( temp + len, "var enc_size=%d;\r\n", bparam.stVencParam.bysize );
    len += sprintf( temp + len, "var enc_framerate=%d;\r\n", bparam.stVencParam.byframerate );
    len += sprintf( temp + len, "var enc_keyframe=%d;\r\n", bparam.stVencParam.keyframe );
    len += sprintf( temp + len, "var enc_quant=%d;\r\n", bparam.stVencParam.quant );
    len += sprintf( temp + len, "var enc_ratemode=%d;\r\n", bparam.stVencParam.ratemode );
    len += sprintf( temp + len, "var enc_bitrate=%d;\r\n", bparam.stVencParam.bitrate );
    len += sprintf( temp + len, "var enc_main_mode=%d;\r\n", bparam.stVencParam.mainmode );
    //sub rate encoder param
    len += sprintf( temp + len, "var sub_enc_size=%d;\r\n", bparam.stVencParam.bysizesub );
    len += sprintf( temp + len, "var sub_enc_framerate=%d;\r\n", bparam.stVencParam.byframeratesub );
    len += sprintf( temp + len, "var sub_enc_keyframe=%d;\r\n", bparam.stVencParam.keyframesub );
    len += sprintf( temp + len, "var sub_enc_quant=%d;\r\n", bparam.stVencParam.quantsub );
    len += sprintf( temp + len, "var sub_enc_ratemode=%d;\r\n", bparam.stVencParam.ratemodesub );
    len += sprintf( temp + len, "var sub_enc_bitrate=%d;\r\n", bparam.stVencParam.bitratesub );
    len += sprintf( temp + len, "var enc_sub_mode=%d;\r\n", bparam.stVencParam.submode );
    //record param
    len += sprintf( temp + len, "var record_cover_enable=%d;\r\n", bparam.stRecordSet.recordover );
    len += sprintf( temp + len, "var record_timer=%d;\r\n", ( unsigned char )bparam.stRecordSet.timer );
    len += sprintf( temp + len, "var record_size=%d;\r\n", bparam.stRecordSet.size );
    len += sprintf( temp + len, "var record_time_enable=%d;\r\n", bparam.stRecordSet.timerenable );
    len += sprintf( temp + len, "var record_schedule_sun_0=%d;\r\n", bparam.stRecordSet.timerecord.time[0][0] );
    len += sprintf( temp + len, "var record_schedule_sun_1=%d;\r\n", bparam.stRecordSet.timerecord.time[0][1] );
    len += sprintf( temp + len, "var record_schedule_sun_2=%d;\r\n", bparam.stRecordSet.timerecord.time[0][2] );
    len += sprintf( temp + len, "var record_schedule_mon_0=%d;\r\n", bparam.stRecordSet.timerecord.time[1][0] );
    len += sprintf( temp + len, "var record_schedule_mon_1=%d;\r\n", bparam.stRecordSet.timerecord.time[1][1] );
    len += sprintf( temp + len, "var record_schedule_mon_2=%d;\r\n", bparam.stRecordSet.timerecord.time[1][2] );
    len += sprintf( temp + len, "var record_schedule_tue_0=%d;\r\n", bparam.stRecordSet.timerecord.time[2][0] );
    len += sprintf( temp + len, "var record_schedule_tue_1=%d;\r\n", bparam.stRecordSet.timerecord.time[2][1] );
    len += sprintf( temp + len, "var record_schedule_tue_2=%d;\r\n", bparam.stRecordSet.timerecord.time[2][2] );
    len += sprintf( temp + len, "var record_schedule_wed_0=%d;\r\n", bparam.stRecordSet.timerecord.time[3][0] );
    len += sprintf( temp + len, "var record_schedule_wed_1=%d;\r\n", bparam.stRecordSet.timerecord.time[3][1] );
    len += sprintf( temp + len, "var record_schedule_wed_2=%d;\r\n", bparam.stRecordSet.timerecord.time[3][2] );
    len += sprintf( temp + len, "var record_schedule_thu_0=%d;\r\n", bparam.stRecordSet.timerecord.time[4][0] );
    len += sprintf( temp + len, "var record_schedule_thu_1=%d;\r\n", bparam.stRecordSet.timerecord.time[4][1] );
    len += sprintf( temp + len, "var record_schedule_thu_2=%d;\r\n", bparam.stRecordSet.timerecord.time[4][2] );
    len += sprintf( temp + len, "var record_schedule_fri_0=%d;\r\n", bparam.stRecordSet.timerecord.time[5][0] );
    len += sprintf( temp + len, "var record_schedule_fri_1=%d;\r\n", bparam.stRecordSet.timerecord.time[5][1] );
    len += sprintf( temp + len, "var record_schedule_fri_2=%d;\r\n", bparam.stRecordSet.timerecord.time[5][2] );
    len += sprintf( temp + len, "var record_schedule_sat_0=%d;\r\n", bparam.stRecordSet.timerecord.time[6][0] );
    len += sprintf( temp + len, "var record_schedule_sat_1=%d;\r\n", bparam.stRecordSet.timerecord.time[6][1] );
    len += sprintf( temp + len, "var record_schedule_sat_2=%d;\r\n", bparam.stRecordSet.timerecord.time[6][2] );

    /* BEGIN: Added by wupm, 2013/4/11 */
    Lock_GetStorageSpace();
    len += sprintf( temp + len, "record_sd_status=%d;\r\n", bparam.stRecordSet.sdstatus );
    //len += sprintf( temp + len, "sdstatus=%d;\r\n", bparam.stStatusParam.sdstatus );
    len += sprintf( temp + len, "sdtotal=%d;\r\n", sdtotal );
    len += sprintf( temp + len, "sdfree=%d;\r\n", sdfree );
    UnLock_GetStorageSpace();

    /* BEGIN: Added by wupm, 2013/7/1 */
    len +=  sprintf( temp + len, "enable_record_audio=%d;\r\n", ( ( bparam.stAlarmParam.reserved & 0x02 ) == 0x02 ) ? 1 : 0 );
    len +=  sprintf( temp + len, "record_sensitivity=%d;\r\n", ( ( bparam.stAlarmParam.reserved & 0xF0 ) >> 4 ) & 0x0F );
    /* END:   Added by wupm, 2013/7/1 */

    head.startcode 	= 0x0a01;
    //head.version   	= 0x0100;
    head.cmd	= cmd;
    head.len       	= len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
    iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
/* END:   Added by wupm, 2013/1/11 */

/* BEGIN: Added by wupm, 2013/3/7 */
int p2pDeleteRecordFile( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		value;
    char            szFieldName[64];
    char			decoderbuf[128];
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );


    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 || byPri != ADMIN )
    {
        CMDHEAD head;
        char    result[64];

        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );

        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );

        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    memset( szFieldName, 0x00, 64 );
    iRet = GetStrParamValue( pparam, "name", szFieldName, 63 );

    if ( iRet == 0x00 )
    {
        char name[128];
        int  iRet;
        memset( decoderbuf, 0x00, 128 );
        URLDecode( szFieldName, strlen( szFieldName ), decoderbuf, 63 );
        memset( szFieldName, 0x00, 64 );
        strcpy( szFieldName, decoderbuf );

#ifdef	GET_RECORDLIST_PAGE_BY_PAGE
        DeleteRecordFile( szFieldName );
#else

        if ( szFieldName[0] != 0 )
        {
            char szCommand[128] = {0};
            sprintf( szCommand, "rm -f /mnt/%s", szFieldName );
            P2PTextout( "%s", szCommand );
            DoSystem( szCommand );
        }

#endif
    }

    if ( 1 )
    {
        int iValue = 0;
        len = sizeof( CMDHEAD );
        len += sprintf( temp + len, "result=0;\r\n" );
    }

    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );

    iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_DELETE_FILE
    myWriteLogIp( 0, LOG_DELETE_FILE );
#endif
#endif

    return 0;

}

/* BEGIN: Added by wupm, 2013/2/21 */
int p2pSDFormat( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		value;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 || byPri != ADMIN )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    NoteSDFormat();

    if ( 1 )
    {
        int iValue = 0;
        len = sizeof( CMDHEAD );
        len += sprintf( temp + len, "result=0;\r\n" );
    }

    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    /* BEGIN: Added by wupm, 2013/6/14 */
#ifdef	MORE_LOG
#ifdef	LOG_FORMAT_SD
    myWriteLogIp( 0, LOG_FORMAT_SD );
#endif
#endif

    return 0;
}

/* BEGIN: Added by wupm, 2013/1/12 */
int p2pSetRecordSchedue( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		value;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );
    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    if ( 1 )
    {
        int iValue = 0;
        len = sizeof( CMDHEAD );
        len += sprintf( temp + len, "result=0;\r\n" );

        if ( GetIntParamValue( pparam, "record_cover", &iValue ) != -1 )
        {
            bparam.stRecordSet.recordover = iValue;
        }

        if ( GetIntParamValue( pparam, "record_timer", &iValue ) != -1 )
        {
            bparam.stRecordSet.timer = iValue;
            ReadRecordSize();
        }

        if ( GetIntParamValue( pparam, "time_schedule_enable", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerenable = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_sun_0", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[0][0] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_sun_1", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[0][1] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_sun_2", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[0][2] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_mon_0", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[1][0] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_mon_1", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[1][1] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_mon_2", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[1][2] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_tue_0", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[2][0] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_tue_1", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[2][1] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_tue_2", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[2][2] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_wed_0", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[3][0] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_wed_1", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[3][1] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_wed_2", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[3][2] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_thu_0", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[4][0] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_thu_1", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[4][1] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_thu_2", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[4][2] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_fri_0", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[5][0] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_fri_1", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[5][1] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_fri_2", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[5][2] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_sat_0", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[6][0] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_sat_1", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[6][1] = iValue;
        }

        if ( GetIntParamValue( pparam, "schedule_sat_2", &iValue ) != -1 )
        {
            bparam.stRecordSet.timerecord.time[6][2] = iValue;
        }

        /* BEGIN: Added by wupm, 2013/7/1 */
        if ( GetIntParamValue( pparam, "enable_record_audio", &iValue ) != -1 )
        {
            if ( iValue == 1 )
            {
                bparam.stAlarmParam.reserved |= 0x02;
            }

            else
            {
                bparam.stAlarmParam.reserved &= 0xFD;
            }
        }

        if ( GetIntParamValue( pparam, "record_sensitivity", &iValue ) != -1 )
        {
            if ( iValue >= 0 && iValue <= 10 )
            {
                bparam.stAlarmParam.reserved &= 0x0F;
                bparam.stAlarmParam.reserved |= ( ( iValue << 4 ) & 0xF0 );
            }
        }

        /* END:   Added by wupm, 2013/7/1 */
        NoteSaveSem();
    }

    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
/* END:   Added by wupm, 2013/1/12 */

/* BEGIN: Added by wupm, 2013/6/16 */
#ifdef	AUTO_DOWNLOAD_FIRMWIRE
int P2PDownloadFile( int sit, short cmd )
{
    unsigned char   temp[2048];
    int             len = 0;
    int             byPri = 0;
    CMDHEAD         head;
    int             iRet;
    int		value;
    unsigned char*   pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );


    //unsigned char           		temp[2048];
    char                            temp1[64];
    char                            temp2[64];
    //int                             len  = 0;
    //int                             iRet = 0;
    char                            nexturl[64];
    //int                             value;
    int                             iValue;
    char                            i = 0;
    char                            flag = 0;
    char                            decoderbuf[128];

    char	server[64];
    int 	nPort = 80;
    char	file[64];
    int		type;

    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        CMDHEAD head;
        char    result[64];
        memset( result, 0x00, 64 );
        sprintf( result, "result=-1" );
        head.startcode = 0x0a01;
        //head.version   = 0x0100;
        head.cmd        = cmd;
        head.len       = strlen( result );
        sprintf( temp + sizeof( CMDHEAD ), "%s", result );
        memcpy( temp, &head, sizeof( CMDHEAD ) );
        len = head.len + sizeof( CMDHEAD );
        //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
		iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    if ( 1 )
    {
        P2PTextout( "CGI String = [%s]", pparam );

        //iRet = GetStrParamValue( pparam, "devalias", szDeviceName, 80 );
        memset( temp, 0x00, 2048 );
        memset( temp2, 0x00, 64 );
        iRet = GetStrParamValue( pparam, "server", temp2, 63 );

        if ( iRet == 0 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
            memset( server, 0x00, 64 );
            strcpy( server, decoderbuf );
        }

        memset( temp2, 0x00, 64 );
        iRet += GetStrParamValue2( pparam, "file", temp2, 63 );

        if ( iRet == 0 )
        {
            memset( decoderbuf, 0x00, 128 );
            URLDecode( temp2, strlen( temp2 ), decoderbuf, 63 );
            memset( file, 0x00, 64 );
            strcpy( file, decoderbuf );
        }

        if ( GetIntParamValue( pparam, "type", &type ) != 0 )
        {
            type = 0;
        }

        if ( GetIntParamValue( pparam, "port", &nPort ) != 0 )
        {
            nPort = 80;
        }

        if ( iRet == 0 )
        {
            P2PTextout( "CGI Success, server = [%s], nPort = %d, file = [%s], type = %d", server, nPort, file, type );
            downloadfile( server, nPort, file, type );
        }
    }

    head.startcode = 0x0a01;
    //head.version   = 0x0100;
    head.cmd       = cmd;
    head.len       = len - sizeof( CMDHEAD );
    memcpy( temp, &head, sizeof( CMDHEAD ) );
    //iRet = p2p_write( sit, P2P_CMDCHANNEL, temp, len );
	iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, temp, len );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
#endif

/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/10 */
#ifdef VERSION_RELEASE
int P2PSendBack( int sit, short cmd, char* buf, int len )
{
	printf("P2PSendBack\n");
	if ( len > 0 )
	{
	    int 	iRet = 0;
	    CMDHEAD* pHead = ( CMDHEAD* )( buf - sizeof( CMDHEAD ) );

		if ( len == 1 )
		{
			len = 0;
		}

	    pHead->startcode = 0x0a01;
	    //pHead->version   = 0x0100;
	    pHead->cmd       = cmd;
	    pHead->len       = len;
		iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, ( unsigned char* )pHead, pHead->len + sizeof( CMDHEAD ) );

	    return iRet;
	}
	else
	{
	    int 	iRet = 0;
		char	temp[1024] = {0};
	    CMDHEAD* pHead = ( CMDHEAD* )( temp );

	    pHead->startcode = 0x0a01;
	    //pHead->version   = 0x0100;
	    pHead->cmd       = cmd;
	    pHead->len       = strlen( buf );

		if ( pHead->len + sizeof(CMDHEAD) >= 1024 )
		{
			P2PTextout("---------------Paramater ERROR");
			return -1;
		}

		memcpy(temp + sizeof(CMDHEAD), buf, pHead->len);
		iRet = PPPP_WriteEx( p2pstream[sit].socket, P2P_CMDCHANNEL, ( unsigned char* )pHead, pHead->len + sizeof( CMDHEAD ) );
		Textout("P2PSendBack cmd:0X%x",cmd);

	    return iRet;
	}
}
void P2PAppendHead( char* *p )
{
	//P2PTextout("Before %04X", *p);
    int len = sprintf( *p, "result=0;\r\n" );
    *p += len;
	//P2PTextout("Before %04X", *p);
}
void P2PAppendString( char* *p, const char* name, const char* value )
{
    int len = sprintf( *p, "var %s=\"%s\";\r\n", name, value );
    *p += len;
}
void P2PAppendInt( char* *p, const char* name, int value )
{
	//P2PTextout("Before %04X", *p);
    int len = sprintf( *p, "var %s=%d;\r\n", name, value );
    *p += len;
	//P2PTextout("Before %04X", *p);
}
/* BEGIN: Add New CGI: get_doorbelllogs.cgi */
/*        Added by wupm(2073111@qq.com), 2014/8/11 */
void P2PAppendFormatString( char* *p, const char* name, int i, const char* value )
{
    int len = sprintf( *p, "var %s[%d]=\"%s\";\r\n", name, i, value );
    *p += len;
}
void P2PAppendFormatInt( char* *p, const char* name, int i, int value )
{
	//P2PTextout("Before %04X", *p);
    int len = sprintf( *p, "var %s[%d]=%d;\r\n", name, i, value );
    *p += len;
	//P2PTextout("Before %04X", *p);
}

typedef struct
{
	int nFileSize;
	char szFileName[32];
}SNAPSHOT_HEAD;

/* END:   Added by wupm(2073111@qq.com), 2014/8/11 */
int P2PAppendFile( char* *p, const char *filename)
{
	char *old_p = *p;
	FILE *fp = NULL;
	char filepath[256] = {0};

	int	nRetValue = 0;

	/* BEGIN: Append a HEAD for SnapShot */
	/*        Added by wupm(2073111@qq.com), 2014/8/12 */
	#ifdef	ENABLE_APPEND_SNAPSHOT_HEAD
		SNAPSHOT_HEAD *pSnapHead = (SNAPSHOT_HEAD *)*p;
		memset(pSnapHead, 0, sizeof(SNAPSHOT_HEAD));
		pSnapHead->nFileSize = 0;
		strcpy(pSnapHead->szFileName, filename);
		*p += sizeof(SNAPSHOT_HEAD);
		old_p = *p;
	#endif

	memset(filepath, 0, 256);
	sprintf( filepath, "/tmp/%s.jpg", filename );
	fp = fopen(filepath, "rb");
	if ( fp == NULL )
	{
		Textout("-------------Can not Open File = [%s], ERROR", filepath);
		return -1;
	}
	else
	{
		int i = 0;
		int nReadBytes = 0;
		int nTotalSize = 0;
		while(1)
		{
			#if	1
			if ( nTotalSize >= 510 * 1024 )	//Max Buffer = 512K
			{
				Textout("---------JPEG FILE TOO LARGE !!! nTotalSize = %d", nTotalSize);
				nTotalSize = 0;
				*p = old_p;
				nRetValue = -1;
				break;
			}
			#endif
			nReadBytes = fread(*p, 1, 2048, fp);
			if ( nReadBytes <= 0 )
			{
				break;
			}
			else if ( nReadBytes < 2048 )
			{
				*p += nReadBytes;
				nTotalSize += nReadBytes;
				//P2PTextout("Read File, nIndex = %d, nTotalSize = %d", ++i, nTotalSize);
				break;
			}
			else if ( nReadBytes == 2048 )
			{
				*p += nReadBytes;
				nTotalSize += nReadBytes;
				//P2PTextout("Read File, nIndex = %d, nTotalSize = %d", ++i, nTotalSize);
				continue;
			}
			else
			{
				Textout("-------------Read JPEG ERROR!!!");
				nTotalSize = 0;
				*p = old_p;
				nRetValue = -1;
				break;
			}
		}
		Textout("Read File OVER!!!, nTotalSize = %d, PointLen = %d", nTotalSize, *p - old_p);
		fclose(fp);

		/* BEGIN: Append a HEAD for SnapShot */
		/*        Added by wupm(2073111@qq.com), 2014/8/12 */
		#ifdef	ENABLE_APPEND_SNAPSHOT_HEAD
		pSnapHead->nFileSize = nTotalSize;
		#endif

		return nRetValue;
	}
}
int GetStrParamValueEx( const char* pszSrc, const char* pszParamName, char* szValue )
{
    const char* 		pos = pszSrc;
    const char*         pos1 = NULL;
    char sz1[64] = {0}, sz2[64] = {0};

    //check src and pszParam
    if ( !pszSrc || !pszParamName )
    {
        return -1;
    }

    sprintf( sz1, "?%s=", pszParamName );
    sprintf( sz2, "&%s=", pszParamName );

    //find param
    pos1 = strstr( pos, sz1 );

    if ( !pos1 )
    {
        pos1 = strstr( pos, sz2 );

        if ( !pos1 )
        {
			//Textout("NOT FOUND");
            return -1;
        }
    }

    //find param end
    pos = pos1 + strlen( pszParamName ) + 2;
    pos1 = strstr( pos, "&" );

    //find
    if ( pos1 )
    {
        if ( pos1 == pos )
        {
            return 1;
        }

        memcpy( szValue, pos, pos1 - pos );
        return 0;
    }

    //not find
    //find space
    pos1 = strstr( pos, " " );

    if ( pos1 != NULL )
    {
        if ( pos1 == pos )
        {
            return 1;
        }

        memcpy( szValue, pos, pos1 - pos );
        return 0;
    }

    if ( strlen( pos ) == pos )
    {
        return 1;
    }

    memcpy( szValue, pos, strlen( pos ) );

    return 0;
}
int GetIntParamValueEx( const char* pszSrc, const char* pszParamName, int* iValue )
{
    char  szParamValue[64] = {0};
    int iRet = GetStrParamValueEx( pszSrc, pszParamName, szParamValue );

    if ( iRet == 0 )
    {
        if ( szParamValue[0] != 0 )
        {
            *iValue = atoi( szParamValue );
        }
    }

    return iRet;
}
#endif
#ifdef	VERSION_RELEASE

/* BEGIN: Added by wupm(2073111@qq.com), 2014/7/2 */
void CloseP2PConnect(int nUserIndex)
{
	int i;
	for ( i = 0; i<MAX_P2P_CONNECT; i++)
	{
		if ( p2pstream[i].socket != -1 )
		{
			if ( p2pstream[i].nIndex == nUserIndex + 1 ||
				p2pstream[i].nIndex == nUserIndex + 101 )
			{
				P2PTextout("Close Connect (%d) !!!", i);
				PopP2pSocket(p2pstream[i].socket);
				break;
			}
		}
	}
}

/* BEGIN: Added by wupm(2073111@qq.com), 2014/7/2 */
void MoveP2PConnect(int nUserIndex )
{
	int i;
	for ( i = 0; i<MAX_P2P_CONNECT; i++)
	{
		if ( p2pstream[i].socket != -1 )
		{
			if ( p2pstream[i].nIndex == nUserIndex + 1 ||
				p2pstream[i].nIndex == nUserIndex + 101 )
			{
				p2pstream[i].nIndex --;
				break;
			}
		}
	}
}

void ResetAlarmConfig()
{
	P2PTextout("ResetAlarmConfig");
    bparam.stBell.alarm_on = 0;
	bparam.stBell.alarm_onok = 1;
    bparam.stBell.alarm_type = AT_MOTION;
    bparam.stBell.alarm_level = 3;
    bparam.stBell.alarm_delay = 20;
    bparam.stBell.alarm_start_hour = 0;
    bparam.stBell.alarm_stop_hour = 23;
    bparam.stBell.alarm_start_minute = 0;
    bparam.stBell.alarm_stop_minute = 59;
}

int ChangePassword( int sit, char* user, char* pwd )
{
    int i;

	if ( p2pstream[sit].socket == -1 )
	{
		P2PTextout("p2pstream[%d].socket = %d, Can not Allow Change Password", sit, p2pstream[sit].socket);
		return -1;
	}

	i = p2pstream[sit].nIndex;
	if ( i >= 1 && i <= 8 )
	{
		i--;
	}
	else
	{
		P2PTextout("-----------p2pstream[sit].nIndex = %d, Can not Allow Change Password", i);
		return -1;
	}

    //for ( i = 0; i < MAX_USER; i++ )
    {
        if ( strcmp( user, bparam.stBell.user[i] ) == 0 )
        {
            if ( pwd[0] == 0 )
            {
                bparam.stBell.pwd[i][0] = 0;
            }

            else
            {
                strcpy( bparam.stBell.pwd[i], pwd );
            }

			P2PTextout("Close Connect, because Change Password");
			CloseP2PConnect(i);

            return 0;
        }
		else
		{
			P2PTextout("---------Current = [%d]/[%d], New = [%d]/[%d], Can not Allow Change User",
				bparam.stBell.user[i],
				bparam.stBell.pwd[i],
				user,
				pwd
				);
			return -1;
		}
    }

    return -1;
}
int	AddUser( char* user, char* pwd )
{
    int i;
    Textout("Add New User, [%s] / [%s]", user, pwd);
    for ( i = 0; i < MAX_USER; i++ )
    {
        if ( bparam.stBell.user[i][0] == 0 )
        {
            memcpy( bparam.stBell.user[i], user, 32 );
            bparam.stBell.user[i][31] = 0;

            if ( pwd[0] == 0 )
            {
                bparam.stBell.pwd[i][0] = 0;
            }

            else
            {
                memcpy( bparam.stBell.pwd[i], pwd, 32 );
                 bparam.stBell.pwd[i][31] = 0;
            }

            return 0;
        }
    }

    return -1;
}

int DelUser( char* user )
{
    int i, k;

    for ( i = 0; i < MAX_USER; i++ )
    {
        if ( strcmp( user, bparam.stBell.user[i] ) == 0 )
        {
            CloseP2PConnect(i);
			bparam.stBell.user[i][0] = 0;
			bparam.stBell.pwd[i][0] = 0;

			/* BEGIN: Added by wupm(2073111@qq.com), 2014/7/2 */
			for(k=i+1;k<MAX_USER;k++)
			{
				if ( bparam.stBell.user[k][0] == 0 )
				{
					break;
				}
				memcpy(bparam.stBell.user[k-1], bparam.stBell.user[k], 32);
				memcpy(bparam.stBell.pwd[k-1], bparam.stBell.pwd[k], 32);

				bparam.stBell.user[k][0] = 0;
				bparam.stBell.pwd[k][0] = 0;

				if ( bparam.stBell.admin == k + 1)
				{
					bparam.stBell.admin = k;
				}

				MoveP2PConnect(k);
			}
			/* END:   Added by wupm(2073111@qq.com), 2014/7/2 */

            return 0;
        }
    }

    return -1;
}

void ChangeVideoHue( int value )
{
	camera_control(9, value);
}
void ChangeVideoSaturation( int value )
{
	camera_control(8, value);
}
void ChangeVideoContrast( int value )
{
	camera_control(2, value);
}
void ChangeVideoBrightness( int value )
{
	camera_control(1, value);
}

void ChangeVideoFrameRate( int value )
{
	camera_control(6, value);
}
void ChangeVideoBitRate( int value )
{
	camera_control(13, value);
}
void ChangeVideoResolution( int value )
{
	camera_control(0, value);
}

void ChangeVideo( int nIndex )
{
	if ( bparam.stBell.video_resolution[nIndex] != bparam.stBell.current_resolution )
    {
		bparam.stBell.current_resolution = bparam.stBell.video_resolution[nIndex];
        ChangeVideoResolution( bparam.stBell.video_resolution[nIndex] );
    }

    if ( bparam.stBell.video_bitrate[nIndex] != bparam.stBell.current_bitrate )
    {
		bparam.stBell.current_bitrate = bparam.stBell.video_bitrate[nIndex];
        ChangeVideoResolution( bparam.stBell.video_bitrate[nIndex] );
    }

    if ( bparam.stBell.video_framerate[nIndex] != bparam.stBell.current_framerate )
    {
		bparam.stBell.current_framerate = bparam.stBell.video_framerate[nIndex];
        ChangeVideoResolution( bparam.stBell.video_framerate[nIndex] );
    }
}

BOOL GetLocalTime(char *szTime)
{
	BOOL bOK = FALSE;
	char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	struct tm *p;
	
    #if	1
    time_t timep;
    time(&timep);
	//P2PTextout("bparam.stDTimeParam.byTzSel = %d", bparam.stDTimeParam.byTzSel);
	timep -= bparam.stDTimeParam.byTzSel;
    p = localtime(&timep); //取得当地时间
    #else
    int temptime = bparam.stDTimeParam.dwCurTime - bparam.stDTimeParam.byTzSel;
    p = localtime(&temptime);
	#endif
	
    sprintf(szTime,
		"%04d-%02d-%02d-%s-%02d-%02d-%02d",
		//"%04d-%02d-%02d_%02d:%02d:%02d",
		1900+p->tm_year,
		p->tm_mon + 1,
		p->tm_mday,
		wday[p->tm_wday],
		p->tm_hour,
		p->tm_min,
		p->tm_sec);

	if (
		(( p->tm_hour > bparam.stBell.alarm_start_hour ) || ( p->tm_hour == bparam.stBell.alarm_start_hour && p->tm_min >= bparam.stBell.alarm_start_minute ) ) &&
		(( p->tm_hour < bparam.stBell.alarm_stop_hour) || ( p->tm_hour == bparam.stBell.alarm_stop_hour && p->tm_min <= bparam.stBell.alarm_stop_minute)))
	{
		Textout("Inside Alarm Valid TimeArea!!");
		return TRUE;
	}

	return FALSE;
}

void OnFoundAlarm(BOOL bMotion, BOOL bPir, char *szTime)
{
	int sit = 0;
	int iRet = 0;
	//int i = 0;
	//char szTime[64] = {0};
	char szPath[64] = {0};
	//int t = 0;
	//GetLocalTime(szTime);

	sprintf(szPath, "/tmp/%s.jpg", szTime);

	EmailSend(FALSE, bMotion, bPir, szPath);
	//CaptureJpeg(szPath);
	//AppendJpegList(szPath);

	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/7/17 */
	//strcpy(szAlarmID, szTime);
	for(sit = 0; sit < MAX_P2P_CONNECT; sit++)
	{
		if ( p2pstream[sit].socket != -1 )
		{
			strcpy(p2pstream[sit].szAlarmID, szTime);
		}
	}

	//Alarm Image
	#if	0	//def	USE_CLOUND_SAVE
	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/14 */
	iRet = Biz_CS_File_Put(szTime, szPath, S3CannedAclPrivate);	//S3CannedAclPublicReadWrite);
	Textout("----------Biz_CS_File_Put Return %d", iRet);
	#endif

    #ifdef BAGGIO_API
    OnDoorNvrAlarm();
    #endif
    
	OnDoorP2PAlarm(-1, szTime, BELL_ALARM);
}

void SetBellAlarmOn(int iValue)
{
	//P2PTextout("SetBellAlarmOn, iValue = %d", iValue);
	if ( iValue == 0 )	//off
    {
        SetDoorOn( FALSE );
    }
	else
    {
		StartOnDelayThread();
		SetDoorOn( TRUE );
		/*
		if ( !GetOnDelayThread() )
		{
			StartOnDelayThread();
		}
		*/
	}
}

BOOL bStartCall = FALSE;
BOOL bResetCall = FALSE;
void ResetTalk(int sit)
{
	bResetCall = TRUE;
}
BOOL CheckStartCalling()
{
	return bStartCall;
}

/* BEGIN: Append new Protocol for IPhone Message */
/*        Added by wupm(2073111@qq.com), 2014/9/4 */
#ifdef	ENABLE_IOS_MESSAGE
static void SetSessionState(int nState)
{
	int sit = 0;
	switch(nState)
	{
		case TALK_OVER:
			for(sit = 0; sit < MAX_P2P_CONNECT; sit++)
			{
				if ( p2pstream[sit].socket != -1 )
				{
				    P2PTextout("sit[%d], szCallID(%s) clear", sit, p2pstream[sit].szCallID );
					p2pstream[sit].szCallID[0] = 0;
				}
			}
			break;
	    default:
            P2PTextout("Unkown state=%d", nState);
	        break;
	}
}
int GetSessionState(int nIndex, char *szid)
{
	/*
	#define	TALK_OVER		2	//ID=NULL
	#define	TALK_EXPIRED	1	//ID=INVALID
	#define	TALK_CALLING	0	//ID=OK, state=CALLING
	#define	TALK_NOBYAGREE	3	//ID=OK, state=IDLE
	#define	TALK_TAKING		4	//ID=OK, state=TALKING
	*/
	int sit = 0;
	int nState = PS_IDLE;
	BOOL bOver = FALSE;
	BOOL bExpired = FALSE;

    //=================================================
    //if only biz user
    int nOnline = 0;
    for(sit = 0; sit < MAX_P2P_CONNECT; sit++)
    {
        if ( p2pstream[sit].socket != -1 )
        {
            nOnline++;
        }
    }

    if(nOnline == 1)
    {
        if(CheckStartCalling())
        {
            strcpy(p2pstream[nIndex].szCallID, szid);
            p2pstream[nIndex].state = PS_CALLING;
            P2PTextout("Biz User(sit = %d)", nIndex);
            return TALK_CALLING;
        }
        else
        {
            P2PTextout("Biz User calling over");
            return TALK_EXPIRED;
        }
    }
    //=================================================
	for(sit = 0; sit != nIndex && sit < MAX_P2P_CONNECT; sit++)
	{
		Textout("sit[%d], socket=%d, szCallID=%s, state=%d", sit, p2pstream[sit].socket, p2pstream[sit].szCallID, p2pstream[sit].state);
		if ( p2pstream[sit].socket != -1 )
		{
			if ( p2pstream[sit].szCallID[0] != 0 )
			{
				if ( strcmp(p2pstream[sit].szCallID, szid) != 0 )
				{
					bExpired = TRUE;
					break;
				}
				else
				{
					if ( p2pstream[sit].state == PS_TALKING )
					{
						P2PTextout("TALK_TAKING(sit = %d)", sit);
						return TALK_TAKING;
					}
					else if( p2pstream[sit].state == PS_CALLING )
					{
						strcpy(p2pstream[nIndex].szCallID, szid);
						p2pstream[nIndex].state = PS_CALLING;
						P2PTextout("TALK_CALLING(sit = %d)", sit);
						return TALK_CALLING;
					}
				}
			}
			else
			{
                bOver = TRUE;
                break;
			}
		}
	}

    if(CheckStartCalling() && !bExpired && !bOver)
    {
        strcpy(p2pstream[nIndex].szCallID, szid);
        p2pstream[nIndex].state = PS_CALLING;
        P2PTextout("Still calling waiting, Biz User(sit = %d)", nIndex);
        return TALK_CALLING;
    }
	
	
	if ( bOver )
	{
		P2PTextout("TALK_OVER");
		return TALK_OVER;
	}
	else if ( bExpired )
	{
		P2PTextout("TALK_EXPIRED");
		return TALK_EXPIRED;
	}

	P2PTextout("TALK_NOBYAGREE");
	return TALK_NOBYAGREE;
}
#endif

/* BEGIN: KONX */
/*        Added by wupm(2073111@qq.com), 2014/10/25 */
#ifdef	KONX
static BOOL bKonxAlarmOn = FALSE;
void OnPressAlarm()
{
	//Off,Open LED(Default)
	//On,Close LED, Enable _ALARM_IN, then is ALARMIN, and Playing, STOP IT
	Textout("Press Alarm Key, now State = %d", bKonxAlarmOn);
	bKonxAlarmOn = !bKonxAlarmOn;
	ControlIO(BELL_ONOFF_LED, bKonxAlarmOn);
	if ( !bKonxAlarmOn )
	{
		StopAudioPlay();
	}
}
#endif


/* BEGIN: KONX, Change IO */
/*        Added by wupm(2073111@qq.com), 2014/11/7 */
void OnAlarmInPressDown()
{
#ifdef  KONX
    if ( !bKonxAlarmOn )
    {
        Textout("OnAlarmInPress, Invalid, Donothing");
        return;
    }
#endif

#ifdef  KONX
    if ( bKonxAlarmOn )
    {
        Textout("OnAlarmInPress, StopPlayAudioFile");
		StopAudioPlay();
    }
#endif
}

/* BEGIN: Zigbee */
/*        Added by wupm(2073111@qq.com), 2014/9/9 */
void OnAlarmIn()
{
	/* BEGIN: KONX */
	/*        Added by wupm(2073111@qq.com), 2014/10/25 */
	//Valid if Alarm ON
	//if Valid, Play Sound myself
	#ifdef	KONX
	if ( !bKonxAlarmOn )
	{
		Textout("OnAlarmIn, Invalid, Donothing");
		return;
	}
	#endif

    if(NowIsAPState() || NowIsWPSState())
    {
        P2PTextout("IPC config WIFI(in AP-%d or in WPS-%d), forbiden Alarm", NowIsAPState(), NowIsWPSState());
        return;
    }	

    
    #if defined (ZHENGSHOW) || defined (KONX) || defined (BELINK)
    
    EnableIR(9);
    #endif

	int iRet = 0;
	int i = 0;
	int sit = -1;
	char szTime[64] = {0};
	char szPath[64] = {0};
	int t = 0;
	GetLocalTime(szTime);

    Textout("OnAlarmIn photos:%s.jpg",szTime);
	sprintf(szPath, "/tmp/%s.jpg", szTime);
	CaptureJpeg(szPath);

	/* BEGIN: Add New CGI: get_doorbelllogs.cgi */
	/*        Modified by wupm(2073111@qq.com), 2014/8/11 */
	#ifdef	ENABLE_BELL_LOG
	AppendBellLog(szTime);
    Textout("-------------------------------szid: %s-----------------------------------",szTime);
	#else
	AppendJpegList(szPath);
	#endif

    #if defined (ZHENGSHOW) || defined (KONX)  || defined (BELINK)
    DisableIR(9);
    #endif

	OnDoorP2PAlarm(-1, szTime, BELL_ALARM);	//Extern Alarm IN(433 STUDY)

	/* BEGIN: KONX */
	/*        Added by wupm(2073111@qq.com), 2014/10/25 */
	#ifdef	KONX
	if ( bKonxAlarmOn )
	{
		Textout("OnAlarmIn, PlayAudioFile");
		StartAudioPlay(WF_ALARM_IN, 0, NULL);
	}
	#endif
}

/********************************Calling APP*********************************************/
void CallbackTalking(unsigned int sit);

static unsigned int nTimerCall = 0;
static char szCallTime[64];
static int nTalkSit = 0;

void BaAudioCallTimeoutCallback(void)
{
    Textout("BaAudioCallTimeoutCallback");
#if  defined( ZILINK ) || defined(ZHENGSHOW)
    /*  Close LED D7/8  */
    ControlBellLED(CB_OPEN);        
#endif

#if defined(FEEDDOG) || defined(FUHONG) || defined(YUELAN) || defined(BELINK) || defined(FOBJECT_FM34)
    /*  Close LED D7/8  */
    ControlBellLED(CB_CLOSE);        
#endif
}

int bMcuNotyceFlag = 0;
void SetMcuNotyceFlag(int flag)
{
    bMcuNotyceFlag = flag;
}
void CallbackWaitingAgree(unsigned int sit)
{
    int index = 0;
    int sit2 = 0;
    
    Textout("Call Waiting... t = %d", nTimerCall);
    nTimerCall++;
    ResetBellAlarm();

    //ReCall
    if ( bResetCall )
    {
        P2PTextout("Re-Call");
        //20141125, BIZ Again
        #ifdef  FACTOP
        OnDoorP2PAlarm(-1, szCallTime, BELL_CALL);
        #endif
        nTimerCall = 0;
        bResetCall = FALSE;
    }

    //Agree
    for(index=0;index<MAX_P2P_CONNECT;index++)
    {
        if ( p2pstream[index].socket != -1 && p2pstream[index].state == PS_TALKING )
        {
            nTalkSit = index;
            break;
        }
    }

    
    if ( index < MAX_P2P_CONNECT )
    {
#ifdef ZHENGSHOW
        ControlIO(BELL_NOTYCE_TO_MCU,0);   
#endif
        Textout("P2P[%d] Agree, Exit Wait, Enter Talking", index);

        UnRegisterBaSysCallback(0xFF, CallbackWaitingAgree);

		#ifdef	ENABLE_BELL_LOG
		P2PTextout("OK, %d Channel Agree, Send BELL_AGREE_ME", index);
		OnDoorP2PAlarm(index, szCallTime, BELL_AGREE_ME );
		#endif
#ifdef LDPUSH
		LdPush("Other clients have been answered",0);
#endif
		//Close Other
		for(sit2=0;sit2<MAX_P2P_CONNECT;sit2++)
		{
			if ( p2pstream[sit2].socket != -1 && sit2 != index )
			{
				p2pstream[sit2].state = PS_IDLE;
				OnDoorP2PAlarm(sit2, szCallTime, BELL_AGREE );
				P2PTextout("Close P2P[%d]", sit2);
			}
		}        

		#ifdef SUPPORT_FM34
				setMotoActionFlag(FALSE);
				fm34_check();
				setMotoActionFlag(TRUE);
				usleep(100*1000);
	    #endif

        nTimerCall = 0;
        RegisterBaSysCallback(0xFF, 1, CallbackTalking);
#ifdef ZHENGSHOW
        usleep(1000000);
        ControlIO(BELL_NOTYCE_TO_MCU,1);
#endif 
        P2PTextout("Exit Call(Agree)");
        return;
    }

    //Timeout
    if ( nTimerCall >= bparam.stBell.max_wait )
    {
        for(index=0;index<MAX_P2P_CONNECT;index++)
        {
            if ( p2pstream[index].socket != -1 )
            {
                if ( p2pstream[index].state == PS_CALLING )
                {
                    p2pstream[index].state = PS_IDLE;
                }
                
                OnDoorP2PAlarm(index, szCallTime, BELL_CALL_WAIT_TIMEOUT);
            }
        }
        
        UnRegisterBaSysCallback(0xFF, CallbackWaitingAgree);
        bStartCall = FALSE;


		#ifdef SUPPORT_FM34
	    setMotoActionFlag(FALSE);
	    fm34_check();
		setMotoActionFlag(TRUE);
		usleep(1000*1000);
	    #endif
		
        StartAudioPlay(WF_CALL_TIMEOUT, 1, BaAudioCallTimeoutCallback);

		
        #if defined (ZHENGSHOW) || defined (KONX) || defined (BELINK)
        DisableIR(0);
        #endif
		
        
        P2PTextout("Exit Call(Wait Timeout)");        
    }
    #ifdef ZHENGSHOW
    if( 1 == bMcuNotyceFlag)
    {
        bMcuNotyceFlag = 0;
        for(index=0;index<MAX_P2P_CONNECT;index++)
        {
            if ( p2pstream[index].socket != -1 )
            {
               if ( p2pstream[index].state == PS_CALLING )
               {
                   p2pstream[index].state = PS_IDLE;
               }
               OnDoorP2PAlarm(index, szCallTime, BELL_CALL_WAIT_TIMEOUT);
            }
        }
        UnRegisterBaSysCallback(0xFF, CallbackWaitingAgree);
       	bStartCall = FALSE;
       	#if defined (ZHENGSHOW)
       	DisableIR(0);
       	#endif

	  	#ifdef SUPPORT_FM34
		setMotoActionFlag(FALSE);
		fm34_check();
		setMotoActionFlag(TRUE);
		usleep(1000*1000);
		#endif
       Textout("************************************Exit Call(MCU HAD OPEN LOCK)"); 
    }
    #endif
}

void BaAudioTalkTimeoutCallback(void)
{
    //Textout("BaAudioTalkTimeoutCallback");
#if  defined( ZILINK ) || defined(ZHENGSHOW)
    /*Close LED D7/8*/
    ControlBellLED(CB_OPEN);
    return;
#endif

#if defined(FEEDDOG) || defined(FUHONG) || defined(YUELAN) || defined(BELINK)
    /*Close LED D7/8*/
    ControlBellLED(CB_CLOSE);
    return;
#endif

    ControlBellLED(CB_FLASH_STOP);
}

void CallbackTalking(unsigned int sit)
{
    nTimerCall++;
    P2PTextout("Talking... t = %d", nTimerCall);
    ResetBellAlarm();

    //Stop
    if ( p2pstream[nTalkSit].state != PS_TALKING )
    {
	#ifdef	ENABLE_IOS_MESSAGE
        SetSessionState(TALK_OVER);
	#endif

        p2pstream[nTalkSit].state = PS_IDLE;

        /************************Talk Over*************************/
        UnRegisterBaSysCallback(0xFF, CallbackTalking);

        #if defined (ZHENGSHOW) || defined (KONX) || defined (BELINK)
        DisableIR(0);
        #endif
        
        //P2PTextout("Enter Delay After Call");
        SetBellAlarmOn(bparam.stBell.alarm_on); //Alarm Delay After Call
        ResetBellAlarm();

        bStartCall = FALSE;

        /* add begin by yiqing, 2015-05-21, 原因: */
        BaAudioTalkTimeoutCallback();

        /************************Talk Over*************************/

        P2PTextout("Exit Talk(Remote Stop Talking)");
        return;
    }

    //ReCall
    if ( bResetCall )
    {
        P2PTextout("Re-Call");
        nTimerCall = 0;
        OnDoorP2PAlarm(nTalkSit, szCallTime, BELL_RESET_TIMER);
        bResetCall = FALSE;
    }

    //Timeout
    if ( nTimerCall >= bparam.stBell.max_talk )
    {
	#ifdef	ENABLE_IOS_MESSAGE
        SetSessionState(TALK_OVER);
	#endif

        p2pstream[nTalkSit].state = PS_IDLE;
        OnDoorP2PAlarm(nTalkSit, szCallTime, BELL_CALL_TIMEOUT);

        StartAudioPlay(WF_TALK_TIMEOUT, 1, BaAudioTalkTimeoutCallback);
    	
        /************************Talk Over*************************/
        UnRegisterBaSysCallback(0xFF, CallbackTalking);

        #if defined (ZHENGSHOW) || defined (KONX) || defined (BELINK)
        DisableIR(0);
        #endif
        
        //P2PTextout("Enter Delay After Call");
        SetBellAlarmOn(bparam.stBell.alarm_on); //Alarm Delay After Call
        ResetBellAlarm();

        bStartCall = FALSE;

        /************************Talk Over*************************/
        P2PTextout("Exit Talk(Timeout)");
    }
}

static void BaPlayCallingCallback(void)
{
    if( !DoorBellIsTalking())
		StartAudioPlay(WF_CALLING, 0, NULL);
}

static unsigned int nTimerCallLed = 0;
static void BaCallLedTimer(unsigned int sit)
{
#ifdef BELL_CALL_LED
    if(nTimerCallLed == 0)
    {
        ControlIO(BELL_CALL_LED, TRUE);
    }

    nTimerCallLed++;
    if(nTimerCallLed >= 6)
    {
        ControlIO(BELL_CALL_LED, FALSE);
        UnRegisterBaSysCallback(0xFF, BaCallLedTimer);
    }
#endif    
}

BOOL DoorBellIsIdle()
{
    BOOL bIdle = TRUE;
    int index=0;
    for(index=0;index<MAX_P2P_CONNECT;index++)
    {
        if ( p2pstream[index].socket != -1 && 
            ( p2pstream[index].state == PS_TALKING || p2pstream[index].state == PS_WATCHING ))
        {
            bIdle = FALSE;
            break;
        }
    }

    return bIdle;
}

BOOL DoorBellIsTalking()
{
    BOOL bTalking = FALSE;
    int index=0;
    for(index=0;index<MAX_P2P_CONNECT;index++)
    {
        if ( p2pstream[index].socket != -1 && p2pstream[index].state == PS_TALKING)
        {
            bTalking = TRUE;
            break;
        }
    }

    return bTalking;
}


void OnCall()
{
	int iRet = 0;
	int i = 0;
	int sit = -1;
	char szPath[64] = {0};
	int t = 0;

	#if defined (FEEDDOG) || defined (FUHONG)
    if(IsRedIrEffective())
        OnIROpen();
	#endif

    #if  defined (KONX) || defined (BELINK)
    EnableIR(0);
    #endif

	#ifdef ZHENGSHOW
	EnableIR(0);
	SetMcuNotyceFlag(0);
	#endif

#if 0//由于按按钮拍的时候马上拍照只拍到手，所以延迟一秒拍照
	GetLocalTime(szCallTime);
	for(sit = 0; sit < MAX_P2P_CONNECT; sit++)
	{
		if ( p2pstream[sit].socket != -1 )
		{
			strcpy(p2pstream[sit].szCallID, szCallTime);
		}
	}

	sprintf(szPath, "/tmp/%s.jpg", szCallTime);
	CaptureJpeg(szPath);

	EmailSend(TRUE, FALSE, FALSE, szPath);

	#ifdef	ENABLE_BELL_LOG
	AppendBellLog(szCallTime);
    Textout("-------------------------------szid: %s-----------------------------------",szCallTime);
	#else
	AppendJpegList(szPath);
	#endif
#endif
	GetLocalTime(szCallTime);
	
	#if 1//在抓拍延时2秒之前推送消息，提高速度
	char szTitle[128] = {0};
	sprintf(szTitle, "%s,%s,%d", szCallTime,bparam.stIEBaseParam.dwDeviceID,BELL_CALL);
	MsgPush(bparam.stIEBaseParam.dwDeviceID, szTitle);
	#endif
	
#ifdef  FACTOP
	/* BEGIN: ACTOP, When First Long-Press-Button(10s) = Reset  */
	/*        Modified by wupm(2073111@qq.com), 2014/10/17 */
	//ControlBellLED(CB_FLASH_SLOW);
	ControlBellLED(CB_OPEN);
#else
#ifdef  ZILINK
	/* Open LED D7/8 */
	ControlBellLED(CB_CLOSE);
#else
	ControlBellLED(CB_FLASH_SLOW);
#endif
#endif

    #ifdef BELL_CALL_LED
    nTimerCallLed = 0;
    RegisterBaSysCallback(0xFF, 1, BaCallLedTimer);
    #endif

	StartAudioPlay(WF_WELCOME, 1, BaPlayCallingCallback);

    #ifdef BAGGIO_API
    NvrAlarmSend(eDoorBell_Call);
    #endif

#if 1 //由于按按钮拍的时候马上拍照只拍到手，所以延迟一秒拍照
	sleep(2);
	//GetLocalTime(szCallTime);
	for(sit = 0; sit < MAX_P2P_CONNECT; sit++)
	{
		if ( p2pstream[sit].socket != -1 )
		{
			strcpy(p2pstream[sit].szCallID, szCallTime);
		}
	}

	sprintf(szPath, "/tmp/%s.jpg", szCallTime);
	CaptureJpeg(szPath);

	/* modify begin by yiqing, 2016-06-16 */
	//EmailSend(TRUE, FALSE, FALSE, szPath);

	#ifdef	ENABLE_BELL_LOG
	AppendBellLog(szCallTime);
    Textout("-------------------------------szid: %s-----------------------------------",szCallTime);
	#else
	AppendJpegList(szPath);
	#endif
#endif
	OnDoorP2PAlarm(-1, szCallTime, BELL_CALL);
	

    nTimerCall = 0;
	RegisterBaSysCallback(0xFF, 1, CallbackWaitingAgree);
	
	return ;
}

//20141126
#ifdef  FACTOP
BOOL bSetD0Low = FALSE;
void* ThreadSetD0Low( void* p )
{
    int t = 0;
    while(1)
    {
        if ( !bSetD0Low )
        {
            sleep(1);
            continue;
        }

        ControlIO(_MOTO_D0, 1);
        //sleep(1); //Release 20141128
        //sleep(2);
        sleep(3);   //Release 20141203
        ControlIO(_MOTO_D0, 0);
        bSetD0Low = FALSE;
    }
}
#endif

static void BaCtrl433Timer(unsigned int sit)
{
    #ifdef BELL_CTRL433
    ControlIO(BELL_CTRL433,FALSE);
    UnRegisterBaSysCallback(0xFF,BaCtrl433Timer);
    #endif
}

void StartCall()
{
    #ifdef BELL_CTRL433
    ControlIO(BELL_CTRL433,TRUE);
    RegisterBaSysCallback(0xFF,1,BaCtrl433Timer);
    #endif

    //20141126, When Call, Set MOTO_D0 3S LOW
    #ifdef  FACTOP
    if ( 1 )
    {
        static pthread_t    threadopendelay1 = 0;
        if ( threadopendelay1 == 0 )
        {
            pthread_create( &threadopendelay1, NULL, &ThreadSetD0Low, NULL );
            pthread_detach(threadopendelay1);
        }
    }
    bSetD0Low = TRUE;
    #endif

	if ( !bStartCall )
	{
		bStartCall = TRUE;
		
		
	    
		OnCall();
	}
	else
	{
		bResetCall = TRUE;
        #ifdef  FACTOP
        bSetD0Low = TRUE;
        #endif
		
	}
}


static BOOL bResetWatch[MAX_P2P_CONNECT] = {0,0,0,0,0,0,0,0};
static BOOL szWatchTime[MAX_P2P_CONNECT][1024];
static unsigned int nTimerWatch[MAX_P2P_CONNECT];
void ResetWatch(int sit)
{
	bResetWatch[sit] = TRUE;
}

void CallbackWatch(unsigned int sit)
{
    nTimerWatch[sit]++;
    P2PTextout("Watching....t[%d]=%d", sit, nTimerWatch[sit]);
    if ( p2pstream[sit].stateEx != PS_WATCHING || p2pstream[sit].socket == -1)
    {
        P2PTextout("sit[%d]Exit Watch(Break)", sit);

        #if defined (ZHENGSHOW) || defined (KONX) || defined (BELINK)
        DisableIR(sit+1);
        #endif


        /*****************WATCH END*********************/
        p2pstream[sit].szWatchID[0] = 0;
        UnRegisterBaSysCallback(sit, CallbackWatch);
        /*****************WATCH END*********************/
        return;
    }

    if ( bResetWatch[sit] )
    {
        nTimerWatch[sit] = 0;
        bResetWatch[sit] = FALSE;
        OnDoorP2PAlarm(sit, szWatchTime[sit], BELL_RESET_TIMER);
        P2PTextout("sit[%d]Reset Watch", sit);
    }

    if ( bparam.stBell.max_watch > 0 && nTimerWatch[sit] >= bparam.stBell.max_watch )
    {
        #if defined (ZHENGSHOW) || defined (KONX)|| defined (BELINK)
        DisableIR(sit+1);
        #endif
    
        if ( p2pstream[sit].stateEx == PS_WATCHING )
        {
            p2pstream[sit].stateEx = PS_IDLE;
        }
        OnDoorP2PAlarm(sit, szWatchTime[sit], BELL_WATH_WAIT_TIMEOUT);
        P2PTextout("sit[%d]Exit Watch(Timeout)", sit);

        /*****************WATCH END*********************/
        p2pstream[sit].szWatchID[0] = 0;
        UnRegisterBaSysCallback(sit, CallbackWatch);
        /*****************WATCH END*********************/        
    }
}


void StartWatch(int sit)
{

#if defined (FEEDDOG) || defined (FUHONG)
    if(IsRedIrEffective())
        OnIROpen();
#endif

	if ( sit >=0 && sit < 8 )
	{
        #if defined (ZHENGSHOW) || defined (KONX)|| defined (BELINK)
        EnableIR(sit+1);
        #endif
        
		P2PTextout("Send Notification to Watch[%d] Thread", sit);
		nTimerWatch[sit] = 0;
		GetLocalTime(szWatchTime[sit]);
		RegisterBaSysCallback(sit, 1, CallbackWatch);		
	}
	else
	{
		P2PTextout("StartWatch Parameters ERROR ( %d )", sit);
	}
}

#endif

#ifdef	VERSION_RELEASE
int P2PResetUserConnect( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

	memset(temp, 0, 4096);
    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

	ResetUserConnect(sit);

    P2PAppendHead( &p );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}

#ifdef	ENABLE_BELL_LOG
int P2PGetBellLogs( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

	memset(temp, 0, 4096);
    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );

	if ( 1 )
	{
		int i = 0;
		GetBellLog();
		if ( BellLog.nCount == 0 )
		{
			P2PAppendInt( &p, "ncount", 0 );
		}
		else
		{
			int nItemIndex = 0;
			P2PAppendInt( &p, "ncount", BellLog.nCount );
			for ( i = 0, nItemIndex = BellLog.nCount - 1 + BellLog.nFirst; i< BellLog.nCount; i++, nItemIndex-- )
			{
				P2PAppendFormatString( &p, "uuid", i, BellLog.oItems[nItemIndex].szID );
				P2PAppendFormatString( &p, "user", i, BellLog.oItems[nItemIndex].szUser );
				P2PAppendFormatInt( &p, "status", i, BellLog.oItems[nItemIndex].nStatus);
                Textout("%d: szid=%s,szUser=%s,nStatus=%d",
                    i,BellLog.oItems[nItemIndex].szID,BellLog.oItems[nItemIndex].szUser,BellLog.oItems[nItemIndex].nStatus);
			}
		}
	}

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}

/* add begin by yiqing, 2015-07-31, 原因: */
int P2PDeleteBellLogs( int sit, short cmd )
{
    
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;
    
    char	szValue[64] = {0};
    int nRetValue = 0;

	memset(temp, 0, 4096);
    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    memset(szValue, 0, 64);
    iRet = GetStrParamValueEx( pparam, "uuid", szValue );

    if ( iRet == 0 && szValue[0] != 0 )
    {
        if(strcmp(szValue,"all_delete") == 0 )
        {
            DeleteAllBellLogs();
        }
        else
        {
            Textout("delete uuid:%s",szValue);
            DeleteBellLog(szValue);
        }

    }
	else
	{
		//iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );
		nRetValue = -1;
	}

	sprintf(szValue, "result=%d;\r\n", nRetValue);
	
	iRet = P2PSendBack( sit, cmd, szValue, 0 );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}
/* add end by yiqing, 2015-07-31 */
#endif

/* BEGIN: Append new Protocol for IPhone Message */
/*        Added by wupm(2073111@qq.com), 2014/9/5 */
#ifdef	ENABLE_IOS_MESSAGE
int P2PCheckSessionState( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;
	char	szValue[64] = {0};
	int     len = 0;

	int nRetValue = 0;

	memset(temp, 0, 4096);
    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        nRetValue = -1;
    }

    memset(szValue, 0, 64);
    iRet = GetStrParamValueEx( pparam, "filename", szValue );
	P2PTextout("P2PCheckSessionState filename = [%s]", szValue);

    if ( iRet == 0 && szValue[0] != 0 )
    {
        nRetValue = GetSessionState(sit, szValue);
		P2PTextout("GetSessionState Return = %d", nRetValue);
    }
	else
	{
		//iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );
		nRetValue = -1;
	}

	len = sprintf(szValue, "result=%d;\r\n", nRetValue);
	len += sprintf(szValue+len, "calltime=%s;\r\n", szCallTime);
	Textout("%s",szValue);
	iRet = P2PSendBack( sit, cmd, szValue, 0 );

	if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
#endif

#ifdef LDPUSH
int pushRegister(int sit,short cmd)
{
    Textout("pushregister");

	int 	byPri = 0;
	int 	iRet = 0;
    char    szValue[64] = {0};
    int nRetValue = 0;
    
    char bUpdateFlag = 0;
    unsigned char index = MAX_PUSH_SIZE;

    int pushType = 0;
    int validity = 0;
    int environment = 0;
	int devicetype = 0;

    /********JPUsh***********/
    char	appKey[64] = {0};
	char	masterKey[64] = {0};
	char    receiverValue[64] = {0};

    /********YPush***********/
    char apikey[32] = {0};
    char ysecret_key[48] = {0};
    char channel_id[20] = {0};

    /************XPush***********/
    //int access_id;;
    char access_id[32] = {0};
    char xsecret_key[40] = {0};
    char device_token[68]={0};
    
	unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

	byPri = CheckP2pPri( sit );

	if ( byPri == 0x00 )
	{
		nRetValue = -1;
        goto loop;
	}

    iRet= GetIntParamValueEx(pparam,"pushType",&pushType);
    if( iRet != 0)
    {
        if( 0 == pushparamlist.pushType )
        {
            //pushparamlist.pushType = 1 +4 ;//JPUSY+XPUSH
            pushparamlist.pushType = 1 +4 +2 ;//JPUSY+XPUSH+baiduyun
        }
    }
    else 
    {
        pushparamlist.pushType = pushType;

    }
    Textout("pushType:%d",pushparamlist.pushType);
    

    iRet= GetIntParamValueEx(pparam,"validity",&validity);
    if(0 == iRet)
    {
        pushparamlist.validity = validity;
    }
    else
    {
        if( 0 == pushparamlist.validity)
        {
            pushparamlist.validity = 48;
        }
    }
    Textout("validity=%d",pushparamlist.validity);

    

    iRet = GetStrParamValueEx(pparam,"device_token",device_token);
    if(iRet ==0 && device_token[0]!= 0)
    {
        int i;
        for(i = 0;  i< MAX_PUSH_SIZE; i++)
        {
            if(strcmp(pushparamlist.stXPushParam[i].device_token,device_token) == 0)
            {
                Textout("this device has exit,update device on index:%d",i);

                bUpdateFlag = 1;
                index = i;
                break;
            }
        }
        
        if(0 == bUpdateFlag)
        {
            for(i = 0; i < MAX_PUSH_SIZE; i++)
            {
                if(pushparamlist.registerTime[i] + (pushparamlist.validity*3600) < time(NULL))
                {
                    Textout("this index has not device or the device has expired, regidter the index:%d",i);
                    index = i;
                    break;
                }
            }
            if(index >= MAX_PUSH_SIZE)
            {
                Textout("Full registration");
                nRetValue = -3;
                WritePushParams();
                goto loop;
            }
        }

        if(pushparamlist.registerTime[index] < time(NULL))
        {
            pushparamlist.registerTime[index] = time(NULL);
            Textout("time[%d]:%d",index,pushparamlist.registerTime[index]);
        }

        iRet= GetIntParamValueEx(pparam,"environment",&environment);
        if(0 == iRet)
        {
            pushparamlist.environment[index] = (char)environment;
        }
        else
        {
            if(0 ==  pushparamlist.environment[index])
            {
                pushparamlist.environment[index] = 1;
            }
        }
		
		iRet= GetIntParamValue(pparam,"devicetype",&devicetype);
	    if(0 == iRet)
	    {
			if(0 == devicetype)
			{
				pushparamlist.devicetype[index] = 1;
			}
			else
			{
				pushparamlist.devicetype[index] = 0;
			}
	    }
	    Textout("devicetype=%d",pushparamlist.devicetype[index]);

        //Textout("pushType2:%d",pushparamlist.pushType);
        
    	//if(pushparamlist.pushType & 0x01)//极光推送
    	{
            iRet = GetStrParamValueEx( pparam, "appkey", appKey );
        	if ( iRet == 0 && appKey[0] != 0 )
        	{
                strcpy(&pushparamlist.stJPushParam[index].appKey,appKey);
        	}

            iRet = GetStrParamValueEx( pparam, "master", masterKey );
        	if ( iRet == 0 && masterKey[0] != 0 )
        	{
        		strcpy(&pushparamlist.stJPushParam[index].masterKey,masterKey);
        	}

            iRet = GetStrParamValueEx( pparam, "alias", receiverValue );
        	if ( iRet == 0 && receiverValue[0] != 0 )
        	{
        		strcpy(&pushparamlist.stJPushParam[index].receiverValue,receiverValue);
        	}
    	}

        //if( (pushparamlist.pushType >>1) & 0x01)//百度云推送
    	{
            iRet = GetStrParamValueEx(pparam,"apikey",apikey);
            if(0 == iRet)
            {
                strcpy(&pushparamlist.stYPushParam[index].apikey,apikey);
            }

            iRet = GetStrParamValueEx(pparam,"ysecret_key",ysecret_key);
            if(0 == iRet)
            {
                strcpy(&pushparamlist.stYPushParam[index].secret_key,ysecret_key);
            }

            iRet = GetStrParamValueEx(pparam,"channel_id",channel_id);
            if(0 == iRet)
            {
                strcpy(&pushparamlist.stYPushParam[index].channel_id,channel_id);
            }
    	}

        strcpy(&pushparamlist.stXPushParam[index].device_token,device_token);
        //if( (pushparamlist.pushType >>2) & 0x01)//腾讯信鸽推送
    	{
            //Textout("xpush");
            iRet = GetStrParamValueEx(pparam,"access_id",access_id);
            if(0 == iRet)
            {
                strcpy(&pushparamlist.stXPushParam[index].access_id,access_id);
                //Textout("access_id:%s",pushparamlist.stXPushParam[index].access_id);
            }
            
            iRet = GetStrParamValueEx(pparam,"xsecret_key",xsecret_key);
            if(0 == iRet)
            {
                strcpy(&pushparamlist.stXPushParam[index].secret_key,xsecret_key);
                //Textout("xsecret_key:%s",pushparamlist.stXPushParam[index].secret_key);
            }

/*
            iRet = GetStrParamValueEx(pparam,"device_token",device_token);
            if(0 == iRet)
            {
                strcpy(&pushparamlist.stXPushParam[index].device_token,device_token); 
            }
*/
    	}
    }

/*
    int j = 0;
    for(j = 0; j < MAX_PUSH_SIZE; j ++)
    {
        Textout("time[%d]:%d",j,pushparamlist.registerTime[j]);
    }
*/

    WritePushParams();
    
    
loop:
    sprintf(szValue, "result=%d;\r\n", nRetValue);
	iRet = P2PSendBack( sit, cmd, szValue, 0 );

	if ( iRet < 0 )
	{
		PopP2pSocket( p2pstream[sit].socket );
		return -1;
	}

/*
   for(j = 0; j < MAX_PUSH_SIZE; j ++)
   {
       Textout("time[%d]:%d",j,pushparamlist.registerTime[j]);
   }
*/

	return 0;
    
}

int pushDelete(int sit,short cmd)
{
    Textout("pushDelete");
	int 	byPri = 0;
	int 	iRet = 0;
    char    szValue[64] = {0};
    int nRetValue = 0;

    char bFlag = 0;
    
    char device_token[68]={0};


    unsigned char*  pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        nRetValue = -1;
        goto loop;
    }

    iRet = GetStrParamValueEx(pparam,"device_token",device_token);
    if(iRet ==0 && device_token[0]!= 0)
    {
        int index;
        for(index = 0;  index< MAX_PUSH_SIZE; index++)
        {
            if(strcmp(pushparamlist.stXPushParam[index].device_token,device_token) == 0)
            {
                memset(&pushparamlist.stXPushParam[index],0,sizeof(XPUSHPARAM));
                memset(&pushparamlist.stYPushParam[index],0,sizeof(YPUSHPARAM));
                pushparamlist.environment[index] = 0;
                pushparamlist.registerTime[index] = 0;
                //pushparamlist.pushType[index] = 0;
                //pushparamlist.validity[index] = 0;
                Textout("delete push whith index:%d success",index);
                bFlag = 1;
                WritePushParams();
                break;
            }
        }
        if(0 == bFlag)
        {
            nRetValue = -3;
        }
    }
    else
    {
        nRetValue = -2;
    }

    
loop:
    sprintf(szValue, "result=%d;\r\n", nRetValue);
	iRet = P2PSendBack( sit, cmd, szValue, 0 );

	if ( iRet < 0 )
	{
		PopP2pSocket( p2pstream[sit].socket );
		return -1;
	}

	return 0;

}

int getPushParam(int sit,short cmd)
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

    int pushType = pushparamlist.pushType;
    int validity = pushparamlist.validity;

	memset(temp, 0, 4096);
    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );

    
    P2PAppendInt( &p, "pushType", pushType );
    P2PAppendInt( &p, "validity", validity );
    Textout("pushType:%d",pushType);
    Textout("validity:%d",validity);
    
     iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}

#endif

#ifdef JPUSH
int jPushRegister(int sit,short cmd)
{
	Textout("jPushRegister");
	int 	byPri = 0;
	int 	iRet = 0;

	
	char	appKey[64] = {0};
	char	masterKey[64] = {0};
	char    receiverValue[64] = {0};
	char    bRegisteFlag = 0;

	char	szValue[64] = {0};
	int nRetValue = 0;
    
	unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

	byPri = CheckP2pPri( sit );

	if ( byPri == 0x00 )
	{
		nRetValue = -1;
	}
	
	ReadJPushParams();

	memset(appKey, 0, 64);
	iRet = GetStrParamValueEx( pparam, "appkey", appKey );

	if ( iRet == 0 && appKey[0] != 0 )
	{
		P2PTextout("appkey = [%s]", appKey);
	}
	else
	{
		return -1;
	}

	memset(masterKey, 0, 64);
	iRet = GetStrParamValueEx( pparam, "master", masterKey );
	
	if ( iRet == 0 && appKey[0] != 0 )
	{
		P2PTextout("masterKey = [%s]", masterKey);
	}
	else {
		return -1;
	}

	memset(receiverValue, 0, 64);
	iRet = GetStrParamValueEx( pparam, "alias", receiverValue );

	if ( iRet == 0 && receiverValue[0] != 0 )
	{
		P2PTextout("receiverValue = [%s]", receiverValue);
	}
	else {
		return -1;
	}

	int i;
	for(i = 0 ; i < MAX_JPUSH_SIZE; i++ )
	{
		if((strcmp(jpushparamlist.stJpushParam[i].receiverValue,receiverValue) == 0)  &&
			(strcmp( jpushparamlist.stJpushParam[i].appKey,appKey) == 0))
			
		{
			jpushparamlist.registerTime[i] = time(NULL);
			strcpy(&jpushparamlist.stJpushParam[i].masterKey,masterKey);
			WriteJPushParams();
			//Textout("jpushparamlist.registerTime[%d]:%d",i,jpushparamlist.registerTime[i]);
			Textout("jPush update success");
			bRegisteFlag = 1;
            break;
		}
	}

	if(bRegisteFlag == 0){
		for(i = 0; i<MAX_JPUSH_SIZE; i++)
			{
				//if(jpushparamlist.registerTime[i] == 0)
                if(jpushparamlist.registerTime[i] + (2*24*3600) < time(NULL))
				{
					
					jpushparamlist.registerTime[i] = time(NULL);
					Textout("registerTime[%d]:%d",i,jpushparamlist.registerTime[i]);
					strcpy(&jpushparamlist.stJpushParam[i].appKey,appKey);
					strcpy(&jpushparamlist.stJpushParam[i].masterKey,masterKey);
					strcpy(&jpushparamlist.stJpushParam[i].receiverValue,receiverValue);
					WriteJPushParams();
					bRegisteFlag = 1;
					nRetValue = 0;
					Textout("jPush register success");
					break;
				}
			}

	}
	
	sprintf(szValue, "result=%d;\r\n", nRetValue);
	iRet = P2PSendBack( sit, cmd, szValue, 0 );

	if ( iRet < 0 )
	{
		PopP2pSocket( p2pstream[sit].socket );
		return -1;
	}

	return 0;
}

int jPushDelete(int sit,short cmd)
{
	Textout("jPushDelete");
	int 	byPri = 0;
	int 	iRet = 0;
	
	char	appKey[64] = {0};
	char    receiverValue[64] = {0};

	char    bDeleteFlag = 0;
	int 	nRetValue = -1;
	char	szValue[64] = {0};

	unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

	byPri = CheckP2pPri( sit );

	if ( byPri == 0x00 )
	{
		nRetValue = -1;
	}

	memset(appKey, 0, 64);
	iRet = GetStrParamValueEx( pparam, "appkey", appKey );
	P2PTextout("appkey = [%s]", appKey);

	if ( iRet == 0 && appKey[0] != 0 )
	{
		memset(receiverValue, 0, 64);
		iRet = GetStrParamValueEx( pparam, "alias", receiverValue );
		Textout("receiverValue = [%s]", receiverValue);

		if ( iRet == 0 && receiverValue[0] != 0 )
		{
			
		}
		else {
			Textout("receiverValue string error");
			return -1;
		}
	}
	else
	{
		Textout("appkey string error");
		return -1;
	}

	

	int i ;
	for(i = 0; i<MAX_JPUSH_SIZE; i++){
		
		if((strcmp( jpushparamlist.stJpushParam[i].appKey,appKey) == 0) &&
			(strcmp(jpushparamlist.stJpushParam[i].receiverValue,receiverValue)==0))
		{
			memset(&(jpushparamlist.stJpushParam[i]),0,sizeof(JPUSHPARAM));
			jpushparamlist.registerTime[i] = 0;
			nRetValue = 0;

			WriteJPushParams();
			Textout("delete jpush[%d] appKey:%s,receiverValue:%s  success!",i,appKey,receiverValue);
			break;
		}
	}

	sprintf(szValue, "result=%d;\r\n", nRetValue);
	iRet = P2PSendBack( sit, cmd, szValue, 0 );

	if ( iRet < 0 )
	{
		PopP2pSocket( p2pstream[sit].socket );
		return -1;
	}

	return 0;
}

#endif

int P2PGetBellTime( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

	memset(temp, 0, 4096);
    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );

    P2PAppendString( &p, "ntp_svr", bparam.stDTimeParam.szNtpSvr );
    P2PAppendInt( &p, "now", bparam.stDTimeParam.dwCurTime );
    P2PAppendInt( &p, "tz", bparam.stDTimeParam.byTzSel );
    P2PAppendInt( &p, "ntp_enable", bparam.stDTimeParam.byIsNTPServer );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2PSetBellTime( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;
    int		iValue = 0;
    char	szValue[64] = {0};

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/28 */
	StartAudioPlay(WF_CONFIGOK, 1, NULL);
	
    //
    szValue[0] = 0;
    iRet = GetStrParamValueEx( pparam, "ntp_svr", szValue );

    if ( iRet == 0 && szValue[0] != 0 )
    {
        strcpy(bparam.stDTimeParam.szNtpSvr, szValue);
    }

	
    iRet = GetIntParamValueEx( pparam, "now", &iValue );

    if ( iRet == 0 )
    {
        bparam.stDTimeParam.dwCurTime = ( unsigned int )iValue;
        bparam.stDTimeParam.byIsPCSync = 1;
    }
	else
    {
        bparam.stDTimeParam.byIsPCSync = 0;
    }
    

	//
    iRet = GetIntParamValueEx( pparam, "tz", &iValue );

    if ( iRet == 0 )
    {
        bparam.stDTimeParam.byTzSel = iValue;
    }

	//
	
    iRet = GetIntParamValueEx( pparam, "ntp_enable", &iValue );

    if ( iRet == 0 )
    {
        bparam.stDTimeParam.byIsNTPServer = iValue;
        SetNtpRestart( bparam.stDTimeParam.byIsNTPServer );
    }

	//
	PcDateSync();
	

    NoteSaveSem();

    P2PAppendHead( &p );	//result=0;\n\r

    P2PAppendString( &p, "ntp_svr", bparam.stDTimeParam.szNtpSvr );
    P2PAppendInt( &p, "now", bparam.stDTimeParam.dwCurTime );
    P2PAppendInt( &p, "tz", bparam.stDTimeParam.byTzSel );
    P2PAppendInt( &p, "ntp_enable", bparam.stDTimeParam.byIsNTPServer );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}
int P2PGetBellConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );
    /*
    bell_on=1/0 免打扰
    bell_audio=1/0 语音提示
    bell_mode=1/0 配置模式
    max_watch= 监视时长
    max_talk= 通话时长
    max_wait= 呼叫最大时间
    */
    P2PAppendInt( &p, "bell_on", bparam.stBell.bell_on );
    P2PAppendInt( &p, "bell_audio", bparam.stBell.bell_audio );
    P2PAppendInt( &p, "bell_mode", bparam.stBell.bell_mode );
    P2PAppendInt( &p, "max_watch", bparam.stBell.max_watch );
    P2PAppendInt( &p, "max_talk", bparam.stBell.max_talk );
    P2PAppendInt( &p, "max_wait", bparam.stBell.max_wait );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2PSetBellConfig( int sit, short cmd )
{
    char   	temp[2048];
    int     byPri = 0;
    int		iRet = 0;
    int		iValue = 0;
    char	szValue[64] = {0};

	memset(temp, 0, 2048);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );
    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/28 */
	StartAudioPlay(WF_CONFIGOK, 1, NULL);

	
    /*
    bell_on=1/0 免打扰
    bell_audio=1/0 语音提示
    bell_mode=1/0 配置模式
    max_watch= 监视时长
    max_talk= 通话时长
    max_wait= 呼叫最大时间
    */
    iRet = GetIntParamValueEx( pparam, "bell_on", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.bell_on = iValue;
    }

    iRet = GetIntParamValueEx( pparam, "bell_audio", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.bell_audio = iValue;
    }

    iRet = GetIntParamValueEx( pparam, "bell_mode", &iValue );

    if ( iRet == 0 )
    {
		P2PTextout("Only Adminitrator Can Set bell_mode(Enable AP), byPri = %d", byPri);
        if ( byPri > 100 )
        {
            bparam.stBell.bell_mode = iValue;
        }
		else
		{
			P2PTextout("Can not Change bell_mode Property, byPri = %d", byPri);
		}
    }

    iRet = GetIntParamValueEx( pparam, "max_watch", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.max_watch = iValue;
    }

    iRet = GetIntParamValueEx( pparam, "max_talk", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.max_talk = iValue;
    }

    iRet = GetIntParamValueEx( pparam, "max_wait", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.max_wait = iValue;
    }

    NoteSaveSem();

    P2PAppendHead( &p );	//result=0;\n\r
    P2PAppendInt( &p, "bell_on", bparam.stBell.bell_on );
    P2PAppendInt( &p, "bell_audio", bparam.stBell.bell_audio );
    P2PAppendInt( &p, "bell_mode", bparam.stBell.bell_mode );
    P2PAppendInt( &p, "max_watch", bparam.stBell.max_watch );
    P2PAppendInt( &p, "max_talk", bparam.stBell.max_talk );
    P2PAppendInt( &p, "max_wait", bparam.stBell.max_wait );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}
int P2PGetLockConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

	memset(temp, 0, 4096);
    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

	//P2PTextout("Before %04X", p);
    P2PAppendHead( &p );
	//P2PTextout("Endi %04X", p);
	/*
    lock_type=1/0 锁类型
    lock_delay=   延时
    */
    P2PAppendInt( &p, "lock_type", bparam.stBell.lock_type );
	P2PAppendInt( &p, "lock_delay", bparam.stBell.lock_delay );
	Textout("lock_type=%d", bparam.stBell.lock_type );
	//P2PTextout("%04X, %04X, %d", p, pData, p-pData);
	//P2PTextout("Return pData = [%s]", pData);
	iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2PSetLockConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;
    int		iValue = 0;
    char	szValue[64] = {0};

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/28 */
	StartAudioPlay(WF_CONFIGOK, 1, NULL);

	
    /*
    lock_type=1/0 锁类型
    lock_delay=   延时
    */
    //
    iRet = GetIntParamValueEx( pparam, "lock_type", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.lock_type = iValue;
    }

    //
    iRet = GetIntParamValueEx( pparam, "lock_delay", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.lock_delay = iValue;
    }

    NoteSaveSem();

    P2PAppendHead( &p );	//result=0;\n\r
    P2PAppendInt( &p, "lock_type", bparam.stBell.lock_type );
    P2PAppendInt( &p, "lock_delay", bparam.stBell.lock_delay );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}
int P2PGetPinConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );
    /*
    pin=1/0         输入类型
    pin_bind=0/1/2   输入关联项
    pout=1/0        输出类型
    pout_bind=0/1/2…输出关联项

    */
    P2PAppendInt( &p, "pin", bparam.stBell.pin );
    P2PAppendInt( &p, "pin_bind", bparam.stBell.pin_bind );
    P2PAppendInt( &p, "pout", bparam.stBell.pout );
    P2PAppendInt( &p, "pout_bind", bparam.stBell.pout_bind );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2PSetPinConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;
    int		iValue = 0;
    char	szValue[64] = {0};

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/28 */
	StartAudioPlay(WF_CONFIGOK, 1, NULL);

	
    /*
    pin=1/0         输入类型
    pin_bind=0/1/2   输入关联项
    pout=1/0        输出类型
    pout_bind=0/1/2…输出关联项
    */
    //
    iRet = GetIntParamValueEx( pparam, "pin", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.pin = iValue;
    }

    //
    iRet = GetIntParamValueEx( pparam, "pin_bind", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.pin_bind = iValue;
    }

    //
    iRet = GetIntParamValueEx( pparam, "pout", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.pout = iValue;
    }

    //
    iRet = GetIntParamValueEx( pparam, "pout_bind", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.pout_bind = iValue;
    }

    NoteSaveSem();

    P2PAppendHead( &p );	//result=0;\n\r
    P2PAppendInt( &p, "pin", bparam.stBell.pin );
    P2PAppendInt( &p, "pin_bind", bparam.stBell.pin_bind );
    P2PAppendInt( &p, "pout", bparam.stBell.pout );
    P2PAppendInt( &p, "pout_bind", bparam.stBell.pout_bind );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}
int P2PGetAlarmConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );
    /*
    alarm_on=0/1       是否开启
    alarm_type=0/1/2    报警类型
    alarm_level=1/2/3/4/5 灵敏度
    alarm_delay      报警延时
    alarm_start=    起始时间
    alarm_stop=     终止时间
    */
    P2PAppendInt( &p, "alarm_on", bparam.stBell.alarm_on );
    P2PAppendInt( &p, "alarm_onok", bparam.stBell.alarm_onok );
    P2PAppendInt( &p, "alarm_type", bparam.stBell.alarm_type );
    P2PAppendInt( &p, "alarm_level", bparam.stBell.alarm_level );
    P2PAppendInt( &p, "alarm_delay", bparam.stBell.alarm_delay );
    P2PAppendInt( &p, "alarm_start_hour", bparam.stBell.alarm_start_hour );
    P2PAppendInt( &p, "alarm_stop_hour", bparam.stBell.alarm_stop_hour );
    P2PAppendInt( &p, "alarm_start_minute", bparam.stBell.alarm_start_minute );
    P2PAppendInt( &p, "alarm_stop_minute", bparam.stBell.alarm_stop_minute );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2PSetAlarmConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;
    int		iValue = 0;
    char	szValue[64] = {0};

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/28 */
	StartAudioPlay(WF_CONFIGOK, 1, NULL);

	
    /*
    alarm_on=0/1       是否开启
    alarm_type=0/1/2    报警类型
    alarm_level=1/2/3/4/5 灵敏度
    alarm_delay      报警延时
    alarm_start=    起始时间
    alarm_stop=     终止时间
    */
    //
    iRet = GetIntParamValueEx( pparam, "alarm_on", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.alarm_on = iValue;
		bparam.stBell.alarm_onok = 0;
		SetBellAlarmOn(iValue);
    }

    //
    iRet = GetIntParamValueEx( pparam, "alarm_type", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.alarm_type = iValue;
    }

    //
    iRet = GetIntParamValueEx( pparam, "alarm_level", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.alarm_level = iValue;
        if(bparam.stBell.alarm_on)
        {
            MotionResutlClear();
            AlarmMotionInit(iValue);
        }
        Textout("alarm_level=%d", bparam.stBell.alarm_level );
    }

    //
    iRet = GetIntParamValueEx( pparam, "alarm_delay", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.alarm_delay = iValue;
    }

    //
    iRet = GetIntParamValueEx( pparam, "alarm_start_hour", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.alarm_start_hour = iValue;
    }

    //
    iRet = GetIntParamValueEx( pparam, "alarm_stop_hour", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.alarm_stop_hour = iValue;
    }

    //
    iRet = GetIntParamValueEx( pparam, "alarm_start_minute", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.alarm_start_minute = iValue;
    }

    //
    iRet = GetIntParamValueEx( pparam, "alarm_stop_minute", &iValue );

    if ( iRet == 0 )
    {
        bparam.stBell.alarm_stop_minute = iValue;
    }

    NoteSaveSem();

    P2PAppendHead( &p );	//result=0;\n\r
    P2PAppendInt( &p, "alarm_on", bparam.stBell.alarm_on );
    P2PAppendInt( &p, "alarm_onok", bparam.stBell.alarm_onok );
    P2PAppendInt( &p, "alarm_type", bparam.stBell.alarm_type );
    P2PAppendInt( &p, "alarm_level", bparam.stBell.alarm_level );
    P2PAppendInt( &p, "alarm_delay", bparam.stBell.alarm_delay );
    P2PAppendInt( &p, "alarm_start_hour", bparam.stBell.alarm_start_hour );
    P2PAppendInt( &p, "alarm_stop_hour", bparam.stBell.alarm_stop_hour );
    P2PAppendInt( &p, "alarm_start_minute", bparam.stBell.alarm_start_minute );
    P2PAppendInt( &p, "alarm_stop_minute", bparam.stBell.alarm_stop_minute );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}
int P2PResetAlarmConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;
    int		iValue = 0;
    char	szValue[64] = {0};

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

	//P2PTextout("Reset 1, alarm_stop_minute = %d", bparam.stBell.alarm_stop_minute);
    ResetAlarmConfig();
	//P2PTextout("Reset 2, alarm_stop_minute = %d", bparam.stBell.alarm_stop_minute);
    NoteSaveSem();

    P2PAppendHead( &p );	//result=0;\n\r
    P2PAppendInt( &p, "alarm_on", bparam.stBell.alarm_on );
    P2PAppendInt( &p, "alarm_onok", bparam.stBell.alarm_onok );
    P2PAppendInt( &p, "alarm_type", bparam.stBell.alarm_type );
    P2PAppendInt( &p, "alarm_level", bparam.stBell.alarm_level );
    P2PAppendInt( &p, "alarm_delay", bparam.stBell.alarm_delay );
    P2PAppendInt( &p, "alarm_start_hour", bparam.stBell.alarm_start_hour );
    P2PAppendInt( &p, "alarm_stop_hour", bparam.stBell.alarm_stop_hour );
    P2PAppendInt( &p, "alarm_start_minute", bparam.stBell.alarm_start_minute );
    P2PAppendInt( &p, "alarm_stop_minute", bparam.stBell.alarm_stop_minute );

	//P2PTextout("Reset 3, alarm_stop_minute = %d", bparam.stBell.alarm_stop_minute);

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}
int P2PGetUserConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );
    /*
    user1=    用户名（限定最大12位字串，不区分大小写）
    pwd1=    密码（限定最大12位字串，不区分大小写）
    …..
    user8=
    pwd8=
    admin=1/2/3/4/5/6/7/8  指明第几个用户具有管理权限
    alias=               设备别名，限定长度31
    */

    P2PAppendString( &p, "user1", bparam.stBell.user[0] );
    P2PAppendString( &p, "pwd1", bparam.stBell.pwd[0] );
    P2PAppendString( &p, "user2", bparam.stBell.user[1] );
    P2PAppendString( &p, "pwd2", bparam.stBell.pwd[1] );
    P2PAppendString( &p, "user3", bparam.stBell.user[2] );
    P2PAppendString( &p, "pwd3", bparam.stBell.pwd[2] );
    P2PAppendString( &p, "user4", bparam.stBell.user[3] );
    P2PAppendString( &p, "pwd4", bparam.stBell.pwd[3] );
    P2PAppendString( &p, "user5", bparam.stBell.user[4] );
    P2PAppendString( &p, "pwd5", bparam.stBell.pwd[4] );
    P2PAppendString( &p, "user6", bparam.stBell.user[5] );
    P2PAppendString( &p, "pwd6", bparam.stBell.pwd[5] );
    P2PAppendString( &p, "user7", bparam.stBell.user[6] );
    P2PAppendString( &p, "pwd7", bparam.stBell.pwd[6] );
    P2PAppendString( &p, "user8", bparam.stBell.user[7] );
    P2PAppendString( &p, "pwd8", bparam.stBell.pwd[7] );

    P2PAppendInt( &p, "admin", bparam.stBell.admin );
    //P2PAppendString( &p, "alias", bparam.stBell.alias );
    P2PAppendString( &p, "alias", bparam.stIEBaseParam.dwDeviceID);

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/25 */
	if ( 1 )
	{
		int i,k;
		int s[MAX_USER];
		for(i=0;i<MAX_USER;i++)
		{
			s[i] = US_OFFLINE;
			for(k=0;k<MAX_P2P_CONNECT;k++)
			{
				if ( p2pstream[k].socket != -1 && p2pstream[k].nIndex == i+1 )
				{
					switch(p2pstream[k].state)
					{
						/*
						case PS_WATCHING:
							s[i] = US_WATCHING;
							break;
						*/
						case PS_TALKING:
							s[i] = US_TALKING;
							break;
						default:
							s[i] = US_ONLINE;
							break;
					}
					/* BEGIN: Added by wupm(2073111@qq.com), 2014/7/17 */
					if ( s[i] == US_ONLINE )
					{
						switch(p2pstream[k].stateEx)
						{
							case PS_WATCHING:
								s[i] = US_WATCHING;
								break;
						}
					}
					break;
				}
			}
		}
		P2PAppendInt( &p, "s1", s[0] );
		P2PAppendInt( &p, "s2", s[1] );
		P2PAppendInt( &p, "s3", s[2] );
		P2PAppendInt( &p, "s4", s[3] );
		P2PAppendInt( &p, "s5", s[4] );
		P2PAppendInt( &p, "s6", s[5] );
		P2PAppendInt( &p, "s7", s[6] );
		P2PAppendInt( &p, "s8", s[7] );
	}

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2PSetUserConfig( int sit, short cmd )
{
    char   	temp[2048];
    int     byPri = 0;
    int		iRet = 0;
    int		iValue = 0;
    char	szValue[64];
    char	szValue2[64];

    memset(szValue, 0, 64);
    memset(szValue2, 0, 64);
	memset(temp, 0, 2048);

    int		iMod = -1;

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );
    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    /*
    mod=0/1/2     0=增加，1=删除，2=修改权限，3=修改设备别名，4=修改用户密码
    newuser=            用户名
    newpwd=            密码
    admin=1/2/3/4/5/6/7/8  mode=2时有效，具有管理权限的用户
    alias=               mode=3时有效，设备别名
    */
    //
    iRet = GetIntParamValueEx( pparam, "mod", &iMod );

	P2PTextout("SetUser, byPri = %d", byPri);

    switch ( iMod )
    {
        case UM_ADDUSER:
            if ( byPri > 100 )
            {
                iRet = GetStrParamValueEx( pparam, "newuser", szValue );

                if ( iRet == 0 && szValue[0] != 0 )
                {
                    szValue2[0] = 0;
                    iRet = GetStrParamValueEx( pparam, "newpwd", szValue2 );

                    if ( iRet == 0 )
                    {
                        iRet = AddUser( szValue, szValue2 );

                        if ( iRet == 0 )
                        {

							StartAudioPlay(WF_CONFIGOK, 1, NULL);
				
							NoteSaveSem();
                        }
                    }
                }
            }
			else
			{
				P2PTextout("DONT ALLOW AddUser");
			}

            break;

        case UM_DELUSER:
            if ( byPri > 100 )
            {
                iRet = GetStrParamValueEx( pparam, "newuser", szValue );

                if ( iRet == 0 && szValue[0] != 0 )
                {
                    iRet = DelUser( szValue );

                    if ( iRet == 0 )
                    {
						StartAudioPlay(WF_CONFIGOK, 1, NULL);
				
						NoteSaveSem();
                    }
                }
            }
			else
			{
				P2PTextout("DONT ALLOW DelUser");
			}

            break;

        case UM_UPPRI:
            if ( byPri > 100 )
            {
                iRet = GetIntParamValueEx( pparam, "admin", &iValue );

                if ( iRet == 0 && iValue >= 1 && iValue <= 8 )
                {
					StartAudioPlay(WF_CONFIGOK, 1, NULL);
				
                    bparam.stBell.admin = iValue;
                    NoteSaveSem();
                }
            }
			else
			{
				P2PTextout("DONT ALLOW Change Admin Index");
			}

            break;

        case UM_UPALIAS:
            iRet = GetStrParamValueEx( pparam, "alias", szValue );

            if ( iRet == 0 && szValue[0] != 0 )
            {
				StartAudioPlay(WF_CONFIGOK, 1, NULL);
		
				/* BEGIN: Modified by wupm(2073111@qq.com), 2014/6/11 */
                //strcpy( bparam.stBell.alias, szValue );
                strcpy( bparam.stIEBaseParam.dwDeviceID, szValue );
                NoteSaveSem();
            }

            break;

        case UM_UPPWD:
            iRet = GetStrParamValueEx( pparam, "newuser", szValue );

            if ( iRet == 0 && szValue[0] != 0 )
            {
                szValue2[0] = 0;
                iRet = GetStrParamValueEx( pparam, "newpwd", szValue2 );

                if ( iRet == 0 )
                {
                    iRet = ChangePassword( sit, szValue, szValue2 );

                    if ( iRet == 0 )
                    {
						StartAudioPlay(WF_CONFIGOK, 1, NULL);
			
                        NoteSaveSem();
                    }
                }
            }

            break;
    }

    P2PAppendHead( &p );	//result=0;\n\r
    P2PAppendString( &p, "user1", bparam.stBell.user[0] );
    P2PAppendString( &p, "pwd1", bparam.stBell.pwd[0] );
    P2PAppendString( &p, "user2", bparam.stBell.user[1] );
    P2PAppendString( &p, "pwd2", bparam.stBell.pwd[1] );
    P2PAppendString( &p, "user3", bparam.stBell.user[2] );
    P2PAppendString( &p, "pwd3", bparam.stBell.pwd[2] );
    P2PAppendString( &p, "user4", bparam.stBell.user[3] );
    P2PAppendString( &p, "pwd4", bparam.stBell.pwd[3] );
    P2PAppendString( &p, "user5", bparam.stBell.user[4] );
    P2PAppendString( &p, "pwd5", bparam.stBell.pwd[4] );
    P2PAppendString( &p, "user6", bparam.stBell.user[5] );
    P2PAppendString( &p, "pwd6", bparam.stBell.pwd[5] );
    P2PAppendString( &p, "user7", bparam.stBell.user[6] );
    P2PAppendString( &p, "pwd7", bparam.stBell.pwd[6] );
    P2PAppendString( &p, "user8", bparam.stBell.user[7] );
    P2PAppendString( &p, "pwd8", bparam.stBell.pwd[7] );

    P2PAppendInt( &p, "admin", bparam.stBell.admin );
    //P2PAppendString( &p, "alias", bparam.stBell.alias );
    P2PAppendString( &p, "alias", bparam.stIEBaseParam.dwDeviceID);

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/25 */
	if ( 1 )
	{
		int i,k;
		int s[MAX_USER];
		for(i=0;i<MAX_USER;i++)
		{
			s[i] = US_OFFLINE;
			for(k=0;k<MAX_P2P_CONNECT;k++)
			{
				if ( p2pstream[k].socket != -1 && p2pstream[k].nIndex == i+1 )
				{
					switch(p2pstream[k].state)
					{
						/*
						case PS_WATCHING:
							s[i] = US_WATCHING;
							break;
						*/
						case PS_TALKING:
							s[i] = US_TALKING;
							break;
						default:
							s[i] = US_ONLINE;
							break;
					}
					/* BEGIN: Added by wupm(2073111@qq.com), 2014/7/17 */
					if ( s[i] == US_ONLINE )
					{
						switch(p2pstream[k].stateEx)
						{
							case PS_WATCHING:
								s[i] = US_WATCHING;
								break;
						}
					}
					break;
				}
			}
		}
		P2PAppendInt( &p, "s1", s[0] );
		P2PAppendInt( &p, "s2", s[1] );
		P2PAppendInt( &p, "s3", s[2] );
		P2PAppendInt( &p, "s4", s[3] );
		P2PAppendInt( &p, "s5", s[4] );
		P2PAppendInt( &p, "s6", s[5] );
		P2PAppendInt( &p, "s7", s[6] );
		P2PAppendInt( &p, "s8", s[7] );
	}

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}
int P2PGetVideoConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

    int 	nIndex = 0;

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );
    /*
    mod=0/1/2/3/4
    入口参数：如果取0，表示获取设备端的当前模式
    出口参数
    mod=     速度优先=1、适中=2、质量优先=3、自定义=4
    resolution= 分辨率
    bitrate=    码流
    framerate=  帧率
    brightness= 亮度
    contrast=   对比
    saturation=  饱和
    hue=      色度
    */

    iRet = GetIntParamValueEx( pparam, "mod", &nIndex );

    if ( nIndex >= 0 || nIndex <= VM_COUNT )
    {
        if ( nIndex == 0 )
        {
            nIndex = bparam.stBell.video_mod;
        }
    }

    P2PAppendInt( &p, "video_mod", nIndex );
    P2PAppendInt( &p, "video_resolution", bparam.stBell.current_resolution );
    P2PAppendInt( &p, "video_framerate", bparam.stBell.current_framerate );
    P2PAppendInt( &p, "video_bitrate", bparam.stBell.current_bitrate );
    P2PAppendInt( &p, "video_brightness", bparam.stBell.current_brightness );
    P2PAppendInt( &p, "video_contrast", bparam.stBell.current_contrast );
    P2PAppendInt( &p, "video_saturation", bparam.stBell.current_saturation );
    P2PAppendInt( &p, "video_hue", bparam.stBell.current_hue );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2PSetVideoConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;
    int		iValue = 0;
    char	szValue[64] = {0};

    int		iType = 0;
	int		nIndex = 0;

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    /*
    入口参数：0=调节，1=保存
    */
    //
    iRet = GetIntParamValueEx( pparam, "type", &iType );

    if ( iRet == 0 )
    {
        iRet = GetIntParamValueEx( pparam, "video_mod", &iValue );

        if ( iRet == 0 && iValue >= 0 && iValue < VM_COUNT )
        {
            if ( iValue != bparam.stBell.video_mod && iValue >= VM_SPEED && iValue < VM_COUNT )
            {
                bparam.stBell.video_mod = iValue;
                NoteSaveSem();
                ChangeVideo( iValue );
            }

			int nIndex = bparam.stBell.video_mod;

            //
            iRet = GetIntParamValueEx( pparam, "video_resolution", &iValue );

            if ( iRet == 0 )
            {
                if ( iValue != bparam.stBell.current_resolution )
                {
                    bparam.stBell.current_resolution = iValue;
                    ChangeVideoResolution( iValue );
                }
            }

            //
            iRet = GetIntParamValueEx( pparam, "video_framerate", &iValue );

            if ( iRet == 0 )
            {
                if ( iValue != bparam.stBell.current_framerate )
                {
                    bparam.stBell.current_framerate = iValue;
                    ChangeVideoFrameRate( iValue );
                }
            }

            //
            iRet = GetIntParamValueEx( pparam, "video_bitrate", &iValue );

            if ( iRet == 0 )
            {
                if ( iValue != bparam.stBell.current_bitrate )
                {
                    bparam.stBell.current_bitrate = iValue;
                    ChangeVideoBitRate( iValue );
                }
            }

            ////
            iRet = GetIntParamValueEx( pparam, "video_brightness", &iValue );

            if ( iRet == 0 )
            {
                if ( iValue != bparam.stBell.current_brightness )
                {
                    bparam.stBell.current_brightness = iValue;
                    ChangeVideoBrightness( iValue );
                }
            }

            ////
            iRet = GetIntParamValueEx( pparam, "video_contrast", &iValue );

            if ( iRet == 0 )
            {
                if ( iValue != bparam.stBell.current_contrast )
                {
                    bparam.stBell.current_contrast = iValue;
                    ChangeVideoContrast( iValue );
                }
            }

            ////
            iRet = GetIntParamValueEx( pparam, "video_saturation", &iValue );

            if ( iRet == 0 )
            {
                if ( iValue != bparam.stBell.current_saturation )
                {
                    bparam.stBell.current_saturation = iValue;
                    ChangeVideoSaturation( iValue );
                }
            }

            ////
            iRet = GetIntParamValueEx( pparam, "video_hue", &iValue );

            if ( iRet == 0 )
            {
                if ( iValue != bparam.stBell.current_hue )
                {
                    bparam.stBell.current_hue = iValue;
                    ChangeVideoHue( iValue );
                }
            }

            /////////////
            if ( iType == 1 )	//Save
            {
                nIndex = bparam.stBell.video_mod;

                bparam.stBell.video_resolution[nIndex] = bparam.stBell.current_resolution;
                bparam.stBell.video_bitrate[nIndex] = bparam.stBell.current_bitrate;
                bparam.stBell.video_framerate[nIndex] = bparam.stBell.current_framerate;

                bparam.stBell.video_brightness = bparam.stBell.current_brightness;
                bparam.stBell.video_contrast = bparam.stBell.current_contrast;
                bparam.stBell.video_saturation = bparam.stBell.current_saturation;
                bparam.stBell.video_hue = bparam.stBell.current_hue;

				/*
				bparam.stVencParam.bitrate = bparam.stBell.video_bitrate[bparam.stBell.video_mod];
				bparam.stVencParam.byframerate = bparam.stBell.video_framerate[bparam.stBell.video_mod];
				bparam.stVencParam.bysize = bparam.stBell.video_resolution[bparam.stBell.video_mod];
				*/
                NoteSaveSem();
            }
        }
    }

    P2PAppendHead( &p );	//result=0;\n\r
    nIndex = bparam.stBell.video_mod;
    P2PAppendInt( &p, "video_mod", nIndex );
    P2PAppendInt( &p, "video_resolution", bparam.stBell.current_resolution );
    P2PAppendInt( &p, "video_framerate", bparam.stBell.current_framerate );
    P2PAppendInt( &p, "video_bitrate", bparam.stBell.current_bitrate );
    P2PAppendInt( &p, "video_brightness", bparam.stBell.current_brightness );
    P2PAppendInt( &p, "video_contrast", bparam.stBell.current_contrast );
    P2PAppendInt( &p, "video_saturation", bparam.stBell.current_saturation );
    P2PAppendInt( &p, "video_hue", bparam.stBell.current_hue );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}
int P2PResetVideoConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;
    int		iValue = 0;
    char	szValue[64] = {0};

	int		nIndex = 0;

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    memset( temp, 0x00, 2048 );
    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    ResetVideoConfig();
    NoteSaveSem();

    P2PAppendHead( &p );	//result=0;\n\r
    nIndex = bparam.stBell.video_mod;
    P2PAppendInt( &p, "video_mod", nIndex );
    P2PAppendInt( &p, "video_resolution", bparam.stBell.current_resolution );
    P2PAppendInt( &p, "video_framerate", bparam.stBell.current_framerate );
    P2PAppendInt( &p, "video_bitrate", bparam.stBell.current_bitrate );
    P2PAppendInt( &p, "video_brightness", bparam.stBell.current_brightness );
    P2PAppendInt( &p, "video_contrast", bparam.stBell.current_contrast );
    P2PAppendInt( &p, "video_saturation", bparam.stBell.current_saturation );
    P2PAppendInt( &p, "video_hue", bparam.stBell.current_hue );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;

}
int P2PGetSnapShot( int sit, short cmd )
{
    char   	temp[1024 * 512] = {0};
    int     byPri = 0;
    int		iRet = 0;
	char	filename[256];

	int 	nImageSize = 0;

	memset(temp, 0, 1024 * 512);
	memset(filename, 0, 256);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );
	//P2PTextout("byPri = %d", byPri);

	//P2PTextout("----------start");
    iRet = GetStrParamValueEx( pparam, "filename", filename );
	P2PTextout("filename = [%s], iRet = %d", filename, iRet);

    if ( byPri == 0x00 )
    {
		/*
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
        */
        nImageSize = 1;
    }

    else if ( iRet != 0 || filename[0] == 0)
    {
		/*
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
        */
        nImageSize = 1;
    }

	else
	{

		P2PTextout("Request Get JPEG File, name = [%s]", filename);

		capturelock();
		iRet = P2PAppendFile( &p, filename );
		//P2PTextout("Send Back JPEG File Size = %d", p - pData);
		captureunlock();

		if ( iRet == 0 )
		{
			nImageSize = p - pData;
		}
		else
		{
			nImageSize = 1;
		}
	}

    //iRet = P2PSendBack( sit, cmd, pData, p - pData );
    iRet = P2PSendBack( sit, cmd, pData, nImageSize );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}

int P2PGetVersionConfig( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );
    /*
    sw_ver=  以字串方式表示的软件版本
    hw_ver=  硬件版本
    */

	char SOFTWARE_VERSION[32];
	sprintf(SOFTWARE_VERSION, "%d.%d.%02d.%02d",
		GetMajorVersion(),
		GetMinorVersion(),
		CUSTOMER_ID,
		PRODUCT_ID * 10 + LANGUAGE_ID);
    P2PAppendString( &p, "sw_ver", SOFTWARE_VERSION );	//bparam.stBell.sw_ver );
    P2PAppendString( &p, "hw_ver", HARDWARE_VERSION );	//bparam.stBell.hw_ver );
    //P2PAppendString( &p, "company", COMPANY_NAME );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2PGetBellParams( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );

    /*
    时钟类
    ntp_enable=
    ntp_svr=
    now=
    tz=
    */
    P2PAppendInt( &p, "now", bparam.stDTimeParam.dwCurTime );
    P2PAppendInt( &p, "tz", bparam.stDTimeParam.byTzSel );
    P2PAppendInt( &p, "ntp_enable", bparam.stDTimeParam.byIsNTPServer );
    P2PAppendString( &p, "ntp_svr", bparam.stDTimeParam.szNtpSvr );
    /*
    系统类
    */
    P2PAppendInt( &p, "bell_on", bparam.stBell.bell_on );
    P2PAppendInt( &p, "bell_audio", bparam.stBell.bell_audio );
    P2PAppendInt( &p, "bell_mode", bparam.stBell.bell_mode );
    P2PAppendInt( &p, "max_watch", bparam.stBell.max_watch );
    P2PAppendInt( &p, "max_talk", bparam.stBell.max_talk );
    P2PAppendInt( &p, "max_wait", bparam.stBell.max_wait );
    /*
    锁控定义
    */
    P2PAppendInt( &p, "lock_type", bparam.stBell.lock_type );
    P2PAppendInt( &p, "lock_delay", bparam.stBell.lock_delay );
    /*
    接口定义
    */
    P2PAppendInt( &p, "enc_size", bparam.stBell.pin );
    P2PAppendInt( &p, "enc_size", bparam.stBell.pin_bind );
    P2PAppendInt( &p, "enc_size", bparam.stBell.pout );
    P2PAppendInt( &p, "enc_size", bparam.stBell.pout_bind );
    /*
    报警定义
    */
    P2PAppendInt( &p, "alarm_on", bparam.stBell.alarm_on );
    P2PAppendInt( &p, "alarm_onok", bparam.stBell.alarm_onok );
    P2PAppendInt( &p, "alarm_type", bparam.stBell.alarm_type );
    P2PAppendInt( &p, "alarm_level", bparam.stBell.alarm_level );
    P2PAppendInt( &p, "alarm_delay", bparam.stBell.alarm_delay );
    P2PAppendInt( &p, "alarm_start_hour", bparam.stBell.alarm_start_hour );
    P2PAppendInt( &p, "alarm_stop_hour", bparam.stBell.alarm_stop_hour );
    P2PAppendInt( &p, "alarm_start_minute", bparam.stBell.alarm_start_minute );
    P2PAppendInt( &p, "alarm_stop_minute", bparam.stBell.alarm_stop_minute );
    /*
    用户定义
    */
    P2PAppendString( &p, "user1", bparam.stBell.user[0] );
    P2PAppendString( &p, "pwd1", bparam.stBell.pwd[0] );
    P2PAppendString( &p, "user2", bparam.stBell.user[1] );
    P2PAppendString( &p, "pwd2", bparam.stBell.pwd[1] );
    P2PAppendString( &p, "user3", bparam.stBell.user[2] );
    P2PAppendString( &p, "pwd3", bparam.stBell.pwd[2] );
    P2PAppendString( &p, "user4", bparam.stBell.user[3] );
    P2PAppendString( &p, "pwd4", bparam.stBell.pwd[3] );
    P2PAppendString( &p, "user5", bparam.stBell.user[4] );
    P2PAppendString( &p, "pwd5", bparam.stBell.pwd[4] );
    P2PAppendString( &p, "user6", bparam.stBell.user[5] );
    P2PAppendString( &p, "pwd6", bparam.stBell.pwd[5] );
    P2PAppendString( &p, "user7", bparam.stBell.user[6] );
    P2PAppendString( &p, "pwd7", bparam.stBell.pwd[6] );
    P2PAppendString( &p, "user8", bparam.stBell.user[7] );
    P2PAppendString( &p, "pwd8", bparam.stBell.pwd[7] );

    P2PAppendInt( &p, "admin", bparam.stBell.admin );

    //P2PAppendString( &p, "alias", bparam.stBell.alias );
    P2PAppendString( &p, "alias", bparam.stIEBaseParam.dwDeviceID );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
int P2PGetBellStatus( int sit, short cmd )
{
    char   	temp[1024 * 64] = {0};
    int     byPri = 0;
    int		iRet = 0;

    char 	mac[32] = {0};

	memset(temp, 0, 4096);

    char* pData = ( char* )( temp + sizeof( CMDHEAD ) );
    char* p = pData;
    unsigned char*	pparam = p2pstream[sit].buffer + sizeof( CMDHEAD );

    byPri = CheckP2pPri( sit );

    if ( byPri == 0x00 )
    {
        iRet = P2PSendBack( sit, cmd, "result=-1;\r\n", 0 );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[sit].socket );
            return -1;
        }

        return 0;
    }

    P2PAppendHead( &p );
    /*
    网络
    mac=
    dhcp=0/1     是否启用DHCP
    ip=
    mask=
    gateway
    dns_auto=0/1  是否自动获取DNS
    dns1
    dns2
    port
    */
    memset( mac, 0x00, 32 );
    memcpy( mac, bparam.stIEBaseParam.szMac, 17 );
    P2PAppendString( &p, "mac", mac );
    P2PAppendInt( &p, "dhcp", bparam.stNetParam.byIsDhcp );
    P2PAppendString( &p, "ip", bparam.stNetParam.szIpAddr );
    P2PAppendString( &p, "mask", bparam.stNetParam.szMask );
    P2PAppendString( &p, "gateway", bparam.stNetParam.szGateway );
    P2PAppendInt( &p, "dns_auto", 1 );
    P2PAppendString( &p, "dns1", bparam.stNetParam.szDns1 );
    P2PAppendString( &p, "dns2", bparam.stNetParam.szDns2 );
    P2PAppendInt( &p, "port", bparam.stNetParam.nPort );
    /*
    WIFI
    wifi=0/1      是否启用WIFI
    wifi_mac=
    ssid=        已配置的SSID
    key=         已配置的Wifi密码
    */
    P2PAppendInt( &p, "wifi", bparam.stWifiParam.byEnable );
    memset( mac, 0x00, 32 );
    memcpy( mac, bparam.stIEBaseParam.szWifiMac, 17 );
    P2PAppendString( &p, "wifi_mac", mac );
    P2PAppendString( &p, "ssid", bparam.stWifiParam.szSSID );
    P2PAppendString( &p, "key", bparam.stWifiParam.szShareKey );

    /*
    版本等其它信息
    sw_ver=  以字串方式表示的软件版本
    hw_ver=  硬件版本
    device_id= P2P的ID号

    ap_mode=0/1  是否处于AP模式
    sensor=0/1    标清=0，高清=1
    status=    网络的连接状态
    	0=OK
    	1=DHCP不成功
    	2=连接路由器故障
    	3=连接互联网故障
    */
    char SOFTWARE_VERSION[32];
	sprintf(SOFTWARE_VERSION, "%d.%d.%02d.%02d",
		GetMajorVersion(),
		GetMinorVersion(),
		CUSTOMER_ID,
		PRODUCT_ID * 10 + LANGUAGE_ID);
    P2PAppendString( &p, "sw_ver", SOFTWARE_VERSION );	//bparam.stBell.sw_ver );
    P2PAppendString( &p, "hw_ver", HARDWARE_VERSION );	//bparam.stBell.hw_ver );
    //P2PAppendString( &p, "company", COMPANY_NAME );

    P2PAppendString( &p, "device_id", bparam.stIEBaseParam.dwDeviceID );
	
	

    P2PAppendInt( &p, "ap_mode", bparam.stBell.ap_mode );
    P2PAppendInt( &p, "sensor", bparam.stBell.sensor );
    P2PAppendInt( &p, "status", bparam.stBell.status );

    iRet = P2PSendBack( sit, cmd, pData, p - pData );

    if ( iRet < 0 )
    {
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    return 0;
}
#endif


int p2pcmdfind( int sit )
{
    char*	 pcmd = NULL;
    char*	 pdst = NULL;
    int	iRet;
    short   cmd = 0x00;
    pcmd = p2pstream[sit].buffer + sizeof( CMDHEAD );
    P2PTextout( "\n===============\nP2P = [%s]", pcmd );

    /**********************Mask set wifi reboot *********************************/
    pdst = strstr( pcmd, "/set_wifi.cgi" );
    if ( pdst != NULL ) bPrevSetWifiCgi = TRUE;

    pdst = strstr( pcmd, "/reboot.cgi" );
    if ( pdst == NULL && bPrevSetWifiCgi == TRUE) bPrevSetWifiCgi = FALSE;
    /**********************Mask set wifi reboot *********************************/

    pdst = strstr( pcmd, "/get_status.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEGET_STATUS;
        iRet = p2pgetstatus( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/get_params.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEGET_PARAM;
        iRet = p2pgetparam( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/get_camera_params.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEGET_CAM_PARAMS;
        iRet = p2pgetcameraparams( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/get_misc.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEGET_MISC;
        iRet = p2pgetmisc( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/get_wifi_scan_result.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEGET_WIFI_SCAN;
        iRet = p2pgetwifiscan( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/get_factory.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEGET_FACTORY;
        iRet = p2pgetfactory( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_ir_gpio.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_IR;
        iRet = p2psetirgpio( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_alarm.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_ALARM;
        iRet = p2psetalarm( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_log.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_LOG;
        iRet = p2psetlog( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_users.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_USER;
        iRet = p2psetusers( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_alias.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_ALIAS;
        iRet = p2psetalias( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_mail.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_MAIL;
        iRet = p2psetmail( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_wifi.cgi" );

    if ( pdst != NULL )
    {
        bPrevSetWifiCgi = TRUE;
        cmd = CGI_IESET_WIFI;
        iRet = p2psetwifi( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/camera_control.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_CAM_CONTROL;
        iRet = p2pcameracontrol( sit, cmd );
        return iRet;
    }

	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/14 */
	/*
    pdst = strstr( pcmd, "/set_datetime.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_DATE;
        iRet = p2psetdate( sit, cmd );
        return iRet;
    }
    */

    pdst = strstr( pcmd, "/snapshot.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_SNAPSHOT;
        iRet = P2PGetSnapShot(sit, cmd);

        return iRet;
    }

    pdst = strstr( pcmd, "/set_ddns.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_DDNS;
        iRet = p2psetddns( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_misc.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_MISC;
        iRet = p2psetmisc( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/decoder_control.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_DECODER_CONTROL;
        iRet = p2pdecodercontrol( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_default.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_DEFAULT;
        iRet = p2psetdefault( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_devices.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_DEVICE;
		/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
        //iRet = p2psetdevices( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_network.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_NETWORK;
        iRet = p2psetnetwork( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_dns.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_DNS;
        iRet = p2psetdns( sit, cmd );
    }

    pdst = strstr( pcmd, "/reboot.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEREBOOT;
        iRet = p2preboot( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/wifi_scan.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_WIFISCAN;
        iRet = p2psetwifiscan( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/restore_factory.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IERESTORE;
        iRet = p2prestorefactory( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/check_user.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEGET_CHECKUSER;
        iRet = p2pcheckuser( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_ftp.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_FTP;
        iRet = p2psetftp( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/upgrade_htmls.cgi" );

    if ( pdst != NULL )
    {
        return -1;
    }

    pdst = strstr( pcmd, "/upgrade_firmware.cgi" );

    if ( pdst != NULL )
    {
        return -1;
    }

    pdst = strstr( pcmd, "/livestream.cgi" );

    if ( pdst != NULL )
    {
        #if (defined(JINQIANXIANG) || defined(BAFANGDIANZI))
        iRet = p2plivestreamOld(sit);
        #else
        iRet = p2plivestream( sit );
        #endif
        return iRet;
    }

    pdst = strstr( pcmd, "/audiostream.cgi" );

    if ( pdst != NULL )
    {
        iRet = p2paudiostream( sit );
        return iRet;
    }

    pdst = strstr( pcmd, "/get_record_file.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEGET_RECORD_FILE;
        iRet = p2pgetrecordfile( sit, cmd );
        return iRet;
    }

    /* BEGIN: Added by wupm, 2013/1/11 */
    pdst = strstr( pcmd, "/get_record.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEGET_RECORD;
        iRet = p2pgetrecord( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/set_recordsch.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IESET_RECORDSCH;
        iRet = p2pSetRecordSchedue( sit, cmd );
        return iRet;
    }

    /* END:   Added by wupm, 2013/1/11 */
    /* BEGIN: Added by wupm, 2013/2/21 */
    pdst = strstr( pcmd, "/set_formatsd.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEFORMATSD;
        iRet = p2pSDFormat( sit, cmd );
        return iRet;
    }

    pdst = strstr( pcmd, "/del_file.cgi" );

    if ( pdst != NULL )
    {
        cmd = CGI_IEDEL_FILE;
        iRet = p2pDeleteRecordFile( sit, cmd );
        return iRet;
    }

    /* BEGIN: Added by wupm, 2013/3/12 */
#ifdef	HONESTECH
    pdst = strstr( pcmd, "/set_htclass_factory_params.cgi" );

    if ( pdst != NULL )
    {
        return P2P_SetHTClassParams( sit, CGI_SET_HTCLASS_PARAMS );
    }

    pdst = strstr( pcmd, "/set_htclass_alarm.cgi" );

    if ( pdst != NULL )
    {
        return P2P_SetHTClassAlarm( sit, CGI_SET_HTCLASS_ALARM );
    }

    pdst = strstr( pcmd, "/get_htclass.cgi" );

    if ( pdst != NULL )
    {
        return P2P_GetHTClass( sit, CGI_GET_HTCLASS );
    }

#endif

/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/10 */
#if	0
    /* BEGIN: Added by wupm, 2013/5/21 */
    pdst = strstr( pcmd, "/SetOemLxmCfg.cgi" );

    if ( pdst != NULL )
    {
        return P2P_SetOemLxmCfg( sit, CGI_SET_OEM_LXM );
    }

    pdst = strstr( pcmd, "/GetOemLxmCfg.cgi" );

    if ( pdst != NULL )
    {
        return P2P_GetOemLxmCfg( sit, CGI_GET_OEM_LXM );
    }
#endif

#ifdef	AUTO_DOWNLOAD_FIRMWIRE
    pdst = strstr( pcmd, "/auto_download_file.cgi" );

    if ( pdst != NULL )
    {
        return P2PDownloadFile( sit, CGI_AUTO_DOWNLOAD_FILE );
    }

#endif

	/* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/13 */
    /* BEGIN: Added by wupm, 2013/11/1 */
	/*
    pdst = strstr( pcmd, "/doorbell_control.cgi" );

    if ( pdst != NULL )
    {
        return P2PDoorBellControl( sit, CGI_DOORBELL_CONTROL );
    }

    pdst = strstr( pcmd, "/doorbell_setup.cgi" );

    if ( pdst != NULL )
    {
        return P2PDoorBellSetup( sit, CGI_DOORBELL_SETUP );
    }

    pdst = strstr( pcmd, "/doorbell_control_get.cgi" );

    if ( pdst != NULL )
    {
        return P2PDoorBellControlGet( sit, CGI_DOORBELL_CONTROL_GET );
    }

    pdst = strstr( pcmd, "/doorbell_setup_get.cgi" );

    if ( pdst != NULL )
    {
		return P2PDoorBellSetupGet( sit, CGI_DOORBELL_SETUP_GET );
    }
	*/

    /* END:   Added by wupm, 2013/11/1 */

    /* BEGIN: Added by wupm(2073111@qq.com), 2014/6/10 */
#ifdef	VERSION_RELEASE
    //
    pdst = strstr( pcmd, "/get_bell_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PGetBellConfig( sit, CGI_P2PGetBellConfig );
    }

    //
    pdst = strstr( pcmd, "/set_bell_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PSetBellConfig( sit, CGI_P2PSetBellConfig );
    }

    //
    pdst = strstr( pcmd, "/get_lock_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PGetLockConfig( sit, CGI_P2PGetLockConfig );
    }

    //
    pdst = strstr( pcmd, "/set_lock_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PSetLockConfig( sit, CGI_P2PSetLockConfig );
    }

    //
    pdst = strstr( pcmd, "/get_pin_config.cgi" );

    if ( pdst != NULL )
    {
		//return P2PCheckSessionState( sit, CGI_P2PCheckSessionState );
        return P2PGetPinConfig( sit, CGI_P2PGetPinConfig );
    }

    //
    pdst = strstr( pcmd, "/set_pin_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PSetPinConfig( sit, CGI_P2PSetPinConfig );
    }

    //
    pdst = strstr( pcmd, "/get_alarm_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PGetAlarmConfig( sit, CGI_P2PGetAlarmConfig );
    }

    //
    pdst = strstr( pcmd, "/set_alarm_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PSetAlarmConfig( sit, CGI_P2PSetAlarmConfig );
    }

    //
    pdst = strstr( pcmd, "/reset_alarm_config.cgi" );

    if ( pdst != NULL )
    {
		//P2PTextout("Call P2PResetAlarmConfig");
        return P2PResetAlarmConfig( sit, CGI_P2PResetAlarmConfig );
    }

    //
    pdst = strstr( pcmd, "/get_user_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PGetUserConfig( sit, CGI_P2PGetUserConfig );
    }

    //
    pdst = strstr( pcmd, "/set_user_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PSetUserConfig( sit, CGI_P2PSetUserConfig );
    }

    //
    pdst = strstr( pcmd, "/get_video_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PGetVideoConfig( sit, CGI_P2PGetVideoConfig );
    }

    //
    pdst = strstr( pcmd, "/set_video_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PSetVideoConfig( sit, CGI_P2PSetVideoConfig );
    }

    //
    pdst = strstr( pcmd, "/reset_video_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PResetVideoConfig( sit, CGI_P2PResetVideoConfig );
    }

    //
    pdst = strstr( pcmd, "/reset_user.cgi" );

    if ( pdst != NULL )
    {
        return P2PResetUserConnect( sit, CGI_P2PResetUserConnect );
    }

    //
    /* BEGIN: Deleted by wupm(2073111@qq.com), 2014/6/24 */
	/*
    pdst = strstr( pcmd, "/control_bell.cgi" );

    if ( pdst != NULL )
    {
        return P2PControlBell( sit, CGI_P2PControlBell );
    }
    */

    //
    pdst = strstr( pcmd, "/get_version_config.cgi" );

    if ( pdst != NULL )
    {
        return P2PGetVersionConfig( sit, CGI_P2PGetVersionConfig );
    }

    //
    /*
    pdst = strstr( pcmd, "/check_user.cgi" );
    if ( pdst != NULL )
    {
        return P2PCheckUser(sit, CGI_DOORBELL_SETUP_GET);
    }
    */
    //
    pdst = strstr( pcmd, "/get_bell_params.cgi" );

    if ( pdst != NULL )
    {
        return P2PGetBellParams( sit, CGI_P2PGetBellParams );
    }

    //
    pdst = strstr( pcmd, "/get_bell_status.cgi" );

    if ( pdst != NULL )
    {
        return P2PGetBellStatus( sit, CGI_P2PGetBellStatus );
    }

    //
    pdst = strstr( pcmd, "/get_datetime.cgi" );

    if ( pdst != NULL )
    {
        return P2PGetBellTime( sit, CGI_P2PGetBellTime );
    }

    //
    pdst = strstr( pcmd, "/set_datetime.cgi" );

    if ( pdst != NULL )
    {
        return P2PSetBellTime( sit, CGI_P2PSetBellTime );
    }

	/* BEGIN: Add New CGI: get_doorbelllogs.cgi */
	/*        Added by wupm(2073111@qq.com), 2014/8/11 */
	#ifdef	ENABLE_BELL_LOG
    pdst = strstr( pcmd, "/get_doorbelllogs.cgi" );

    if ( pdst != NULL )
    {
        return P2PGetBellLogs( sit, CGI_P2PGetBellLogs );
    }

    /* add begin by yiqing, 2015-07-31, 原因: */

    pdst = strstr( pcmd, "/delete_doorbelllogs.cgi" );
    
        if ( pdst != NULL )
        {
            return P2PDeleteBellLogs( sit, CGI_P2PDeleteBellLog );
        }

    /* add end by yiqing, 2015-07-31 */
    
	#endif

	/* BEGIN: Append new Protocol for IPhone Message */
	/*        Added by wupm(2073111@qq.com), 2014/9/5 */
	#ifdef	ENABLE_IOS_MESSAGE
    pdst = strstr( pcmd, "/check_session_state.cgi" );

    if ( pdst != NULL )
    {
        return P2PCheckSessionState( sit, CGI_P2PCheckSessionState );
    }
	#endif

#endif

/* BEGIN: Added by yiqing, 2015/3/18 */
#ifdef JPUSH
	pdst = strstr( pcmd, "/jpush_register.cgi" );
					
    if ( pdst != NULL )
    {
        return jPushRegister( sit, CGI_P2PRegisterJpush);
    }

	pdst = strstr( pcmd, "/jpush_delete.cgi" );

    if ( pdst != NULL )
    {
        return jPushDelete( sit, CGI_P2PDeleteJpush);
    }
#endif
/* END:   Added by yiqing, 2015/3/18 */

/* add begin by yiqing, 2015-05-28, 原因: */
#ifdef LDPUSH
	pdst = strstr( pcmd, "/push_register.cgi" );
					
    if ( pdst != NULL )
    {
        return pushRegister( sit, CGI_P2PRegisterPush);
    }

	pdst = strstr( pcmd, "/push_delete.cgi" );

    if ( pdst != NULL )
    {
        return pushDelete( sit, CGI_P2PDeletePush);
    }

    pdst = strstr( pcmd, "/getPushParam.cgi" );

    if ( pdst != NULL )
    {
        return getPushParam( sit, CGI_P2PGetPushParam);
    }
#endif
/* add end by yiqing, 2015-05-28 */

    return -1;
}

void p2pcmdproc( int sit )
{
    p2pcmdfind( sit );
    wifiok = 1;
}


void* p2plistenThreadProc( void* p )
{
    int handle = -1;

    P2P_init();
    while ( 1 )
    {
        if ( ( strlen( bparam.stIEBaseParam.dwDeviceID ) == 0 ) || ( bparam.stIEBaseParam.dwDeviceID[0] == 0x00 ) )
        {
            sleep( 3 );
            continue;
        }
#ifdef PPCS_API
        //handle = PPPP_Listen( bparam.stIEBaseParam.dwDeviceID, 300, 0, 1,"HRMPGZ" );//test
        handle = PPPP_Listen( bparam.stIEBaseParam.dwDeviceID, 300, 0, 1,bparam.stIEBaseParam.dwApiLisense);

		//Textout("ApiLisense = %s",bparam.stIEBaseParam.dwApiLisense);
#else
		handle = PPPP_Listen( bparam.stIEBaseParam.dwDeviceID, 300, 0, 1 );
#endif
		//Textout("DeviceId = %s",bparam.stIEBaseParam.dwDeviceID);
        if ( handle < ERROR_PPPP_SUCCESSFUL )
        {
            sleep( 3 );
            continue;
        }

        wifiok = 1;
        PushP2pSocket( handle );
    }
}


void p2pcmdrcv( int sit, unsigned int readsize )
{
    int 	iRet;
    int 	needread;
    int	readsize1 = readsize;

    //read cmdhead
    if ( p2pstream[sit].len < sizeof( CMDHEAD ) )
    {
        //printf("read cmdhead=%d\n",p2pstream[sit].len);
        //read cmdhead
        needread = sizeof( CMDHEAD );
        iRet = PPPP_Read( p2pstream[sit].socket, P2P_CMDCHANNEL,
                          p2pstream[sit].buffer + p2pstream[sit].len, &needread, 100000 );

        //printf("read cmdhead size:%d iRet %d\n",needread,iRet);
        //check cmdhead is rcv
        if ( ( iRet < 0 ) && ( iRet != ERROR_PPPP_TIME_OUT ) )
        {
            //printf( "cmdhead channel recive failed iRet=%d\n", iRet );
            P2P_close( p2pstream[sit].socket );
            return;
        }

        else
        {
            p2pstream[sit].len += needread;
        }
    }

    //read cmd param
    if ( p2pstream[sit].len >= sizeof( CMDHEAD ) )
    {
        CMDHEAD head;
        //printf("read cgi=%d\n",p2pstream[sit].len);
        //check cmdhead
        memcpy( &head, p2pstream[sit].buffer, sizeof( CMDHEAD ) );

        if ( head.len >= 1000 )
        {
            //printf( "cmd channel rcv size1 > 512\n" );
            P2P_close( p2pstream[sit].socket );
            return;
        }

        //check head
        needread = head.len + sizeof( CMDHEAD ) - p2pstream[sit].len;

        if ( needread > 0 )
        {
            iRet = PPPP_Read( p2pstream[sit].socket, P2P_CMDCHANNEL,
                              p2pstream[sit].buffer + p2pstream[sit].len, &needread, 100000 );
        }

        else
        {
            iRet = 0x00;
        }

        //printf("read cgi iRet %d needread %d\n",iRet,needread);
        //check iRet
        if ( ( ( iRet < 0 ) && ( iRet != ERROR_PPPP_TIME_OUT ) ) || ( needread >= 1000 ) )
        {
            //printf( "cmd channel recive failed iRet=%d\n", iRet );
            P2P_close( p2pstream[sit].socket );
            return;
        }

        else
        {
            p2pstream[sit].len += needread;
            //printf("p2pcmdproc headlen %d streamlen %d\n",head.len,p2pstream[sit].len);
            p2pcmdproc( sit );
            memset( p2pstream[sit].buffer, 0x00, 1024 );
            p2pstream[sit].len = 0;
        }
    }

    //readsize1 -= p2pstream[sit].len;
}

void* p2pcmdThreadProc( void* p )
{
    int 		iRet;
    int 		handle;
    int i = 0;
    unsigned int	readsize;
    unsigned int	writesize;

    while ( 1 )
    {
        usleep( 10*1000 );

        for ( i = 0; i < MAX_P2P_CONNECT; i++ )
        {
            if ( p2pstream[i].socket == -1 )
            {
                usleep( 50000 );
                continue;
            }

            iRet = PPPP_Check_Buffer( p2pstream[i].socket, P2P_CMDCHANNEL, NULL, &readsize );
            if ( iRet < 0 )
            {
                P2PTextout( "PPPP_Check_Buffer socket=%d, iRet=%d", p2pstream[i].socket, iRet );
                PopP2pSocket( p2pstream[i].socket );
                continue;
            }

            if ( readsize > 0 )
            {
                wifiok = 1;
                p2pcmdrcv( i, readsize );
            }
        }
    }
}

void p2ptalkrcv( int sit, unsigned int readsize )
{
    int 			iRet;
    int 			needread;
    int			readsize1 = readsize;
    LIVEHEAD       phead;

    //get talk data
    if ( p2pstream[sit].talklen < MAX_TALKLEN )
    {
        needread = MAX_TALKLEN - p2pstream[sit].talklen;
        iRet = PPPP_Read( p2pstream[sit].socket, P2P_TALKCHANNEL,
                          p2pstream[sit].talkbuffer + p2pstream[sit].talklen, &needread, 100000 );

        /* BEGIN: Modified by wupm(2073111@qq.com), 2014/4/16 */
        if ( 0 )
        {
            st_PPPP_Session SInfo;
            PPPP_Check( p2pstream[sit].socket, &SInfo );
            Textout("MODE=%d, Local=[%s], LocalWan = [%s], Remote=[%s]", SInfo.bMode, inet_ntoa(SInfo.MyLocalAddr.sin_addr), inet_ntoa(SInfo.MyWanAddr.sin_addr),inet_ntoa(SInfo.RemoteAddr.sin_addr));
            //P2PTextout( "MODE=%d, Local=[0x%8X], LocalWan = [0x%8X], Remote=[0x%8X]", SInfo.bMode, ( SInfo.MyLocalAddr.sin_addr.s_addr ), ( SInfo.MyWanAddr.sin_addr.s_addr ), ( SInfo.RemoteAddr.sin_addr.s_addr ) );
        }

        //check cmdhead is rcv
        if ( ( iRet < 0 ) && ( iRet != ERROR_PPPP_TIME_OUT ) )
        {
            //printf( "cmdhead channel recive failed iRet=%d\n", iRet );
            P2P_close( p2pstream[sit].socket );
            return;
        }

        else
        {
            p2pstream[sit].talklen += needread;
        }
    }

    //find start code
    memcpy( &phead, p2pstream[sit].talkbuffer, sizeof( LIVEHEAD ) );

	/* BEGIN: Modified by wupm(2073111@qq.com), 2014/7/10 */
    //printf("phead type %d phead len %d\n",phead.type,phead.len);
    //if ( ( phead.startcode != STARTCODE ) || ( phead.type != 0x08 ) || ( phead.len != MAX_TALKLEN_DATA ) )
    if ( ( phead.type != 0x08 ) || ( phead.len != MAX_TALKLEN_DATA ) )
    {
        short sit1 = 0;

        while ( p2pstream[sit].talklen > 0 )
        {
            memcpy( &phead, p2pstream[sit].talkbuffer + sit1, sizeof( LIVEHEAD ) );

            if ( ( phead.type == 0x08 ) && ( phead.len == MAX_TALKLEN_DATA ) )
            {
                //printf("Find head=================%d\n",p2pstream[sit].talklen);
                break;
            }

            else
            {
                p2pstream[sit].talklen--;
                sit1++;
            }
        }
    }

    //check stream head
    if ( p2pstream[sit].talklen >= MAX_TALKLEN )
    {
        //printf("talklen %d\n",p2pstream[sit].talklen);
        memcpy( &phead, p2pstream[sit].talkbuffer, sizeof( LIVEHEAD ) );

        if ( phead.type == 0x08 )
        {
            //send audio data
            //printf("send audio data\n");
            /* BEGIN: Added by Baggio.wu, 2013/9/23 */
            phead.adpcmsample = 0;
            phead.adpcmindex = 0;

            /* BEGIN: Added by wupm, 2013/5/29 */
            DecoderClr( phead.adpcmsample, phead.adpcmindex );

            HD_AudioOutSendDataNew(p2pstream[sit].talkbuffer + sizeof( LIVEHEAD ), MAX_TALKLEN - sizeof( LIVEHEAD ));
        }

        p2pstream[sit].talklen = 0x00;
    }
}

void* p2ptalkThreadProc( void* p )
{
    int 		iRet = 0;
    int 		i = 0;
    unsigned int	readsize = 0;

    while ( 1 )
    {
        usleep( 10*1000 );

        for ( i = 0; i < MAX_P2P_CONNECT; i++ )
        {
            if ( p2pstream[i].socket == -1 )
            {
                continue;
            }

            // BEGIN: Added by wupm(2073111@qq.com), 2014/4/16 
            readsize = 0;

            iRet = PPPP_Check_Buffer( p2pstream[i].socket, P2P_TALKCHANNEL, NULL, &readsize );
            if ( iRet < 0 )
            {
                P2PTextout("PPPP_Check_Buffer socket=%d, iRet=%d", p2pstream[i].socket, iRet);
                PopP2pSocket( p2pstream[i].socket );
                //bell_avstate.bAudioRecving[i] = FALSE;
                continue;
            }

            if ( readsize > 0 )
            {
                wifiok = 1;
                p2ptalkrcv( i, readsize );
            }
        }
		
    }
}

int p2pmediasend( int sit )
{
    int                     	iRet = 0;
    unsigned int           len     = 0;
    unsigned char*        pbuf   = NULL;
    LIVEHEAD              phead;
    unsigned int		offset;
    unsigned int		writesize;

    iRet = PPPP_Check_Buffer( p2pstream[sit].socket, P2P_VIDEOCHANNEL, &writesize, NULL );
    if ( iRet < 0 )
    {
		P2PTextout("PPPP_Check_Buffer socket=%d, iRet=%d", p2pstream[sit].socket, iRet);
        PopP2pSocket( p2pstream[sit].socket );
        usleep( 10000 );
        return 0;
    }

	if ( writesize >= 1024 * 64 )
    {
        usleep( 10*1000 );
        return 0;
    }

    mjpglock();
    offset = encbufj.prelen;
    memcpy( &phead, encbufj.pbuf + offset, sizeof( LIVEHEAD ) );
    pbuf = encbufj.pbuf + offset;
    len = phead.len + sizeof( LIVEHEAD );

    if ( ( len >= MAX_JPEG_BUFFER_SIZE ) || ( len <= 0x00 ) )
    {
        P2PTextout("VIDEO Size = %d, TOO LARGE---", len);
        mjpgunlock();
        return 0;
    }

    memcpy( videosendbuf, pbuf, len );
    mjpgunlock();

    if ( p2pstream[sit].socket >= 0 )
    {
		iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_VIDEOCHANNEL,videosendbuf,len);
        if ( iRet < 0 )
        {
            P2PTextout("Send Video ERROR!!");
            PopP2pSocket( p2pstream[sit].socket );
            return 0;
        }
    }

	return 0;
}

void p2paudiosend( int sit )
{
    int                     iRet;
    unsigned int            len     = 0;
    unsigned char*           pbuf   = NULL;
    LIVEHEAD                phead;
    unsigned int		offset;
    unsigned int		writesize;
    //unsigned char		sendbuf[1024 * 128];
	unsigned char		sendbuf[1024];

    iRet = PPPP_Check_Buffer( p2pstream[sit].socket, P2P_AUDIOCHANNEL, &writesize, NULL );
    if ( iRet < 0 )
    {
        P2PTextout( "PPPP_Check_Buffer(Audio) socket=%d, iRet=%d", p2pstream[sit].socket, iRet );
        PopP2pSocket( p2pstream[sit].socket );
        return;
    }

    if ( writesize >= 1024 * 64 )
    {
        usleep(10*1000);
        return;
    }

    audiolock();
    offset = encbufa.prelen;
    memcpy( &phead, encbufa.pbuf + offset , sizeof( LIVEHEAD ) );
    pbuf = encbufa.pbuf + offset;


    len = phead.len + sizeof( LIVEHEAD );

	if ( len >= 1024 )
    {
        P2PTextout("Read Audio DataSize ERROR = %d", len);
        audiounlock();
        return;
    }
    memcpy( sendbuf, pbuf, len );
    audiounlock();

    if ( p2pstream[sit].socket >= 0 )
    {
        iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_AUDIOCHANNEL,sendbuf,len);
        if ( iRet < 0 )
        {
            P2PTextout( "media send failed=%d, Clsoe Socket(%d)", iRet, sit );
            PopP2pSocket( p2pstream[sit].socket );
        }
    }
}

static unsigned int p2pprelen[MAX_P2P_CONNECT];
int p2pmediasendH264( int sit )
{
    int                     iRet = 0;
    unsigned int           len     = 0;
    LIVEHEAD            phead;
    unsigned int	    writesize = 0;
    char	    sendbuf[MAX_FRAME_LENGTH];
    int                         status = 0;

    int offset = 0;
    int prelen = 0;
    unsigned char* pbuf = NULL;

    iRet = PPPP_Check_Buffer( p2pstream[sit].socket, P2P_VIDEOCHANNEL, &writesize, NULL );
    if ( iRet < 0 )
    {
        printf( "check buffer p2p media send=%d\n", iRet );
        PopP2pSocket( p2pstream[sit].socket );
        return -1;
    }

    if ( writesize >= 1024 * 64 )
    {
        printf("writesize >= 1024 * 64\n");
        return -2;
    }

    //get prelen
    mainLock();

    if ( userindp2p[sit].flag > 0 )
    {
        if ( userindp2p[sit].flag == 0x01 )
        {
            userindp2p[sit].offset = 0;
        }

        else if ( userindp2p[sit].flag == 0x02 )
        {
            userindp2p[sit].offset = EBUF_MAX_MLEN / 2;
        }

        userindp2p[sit].flag = 0;
    }

    //printf("prelen %d prelen %d\n",p2pprelen[sit], encbufm.prelen);
    prelen = encbufm.prelen;
    if ( p2pprelen[sit] == prelen )
    {
        mainUnlock();
        return -3;
    }

    p2pprelen[sit] = prelen;
    pbuf = encbufm.pbuf;
    offset = userindp2p[sit].offset;
    
    memcpy( &phead, pbuf + offset, sizeof( LIVEHEAD ) );
    pbuf = pbuf + offset;
    len = phead.len + sizeof( LIVEHEAD );

    if ( len >= MAX_FRAME_LENGTH )
    {
        status = -1;
    }

    else
    {
        memcpy( sendbuf, pbuf, len );
        status = 0;
        userindp2p[sit].offset += len;
    }

    mainUnlock();

    if ( p2pstream[sit].socket >= 0 && status == 0 )
    {
        iRet = PPPP_WriteEx(p2pstream[sit].socket,P2P_VIDEOCHANNEL,sendbuf,len);
        if ( iRet < 0 )
        {
            printf( "media send failed=%d\n", iRet );
            PopP2pSocket( p2pstream[sit].socket );
            return  -5;
        }
    }

    return 0;
}

void NoteP2pUser( void )
{
    sem_post( &p2psem );
}

unsigned int nSnapshotVideo = 0;
BOOL bSnapshotVideo = FALSE;
void SetSnapshotVideo(char value)
{
	bSnapshotVideo = value;
	nSnapshotVideo = 0;
}
void* p2pmediaThreadProc( void* p )
{
    int            i = 0;

    if ( ( sem_init( &p2psem, 0, 0 ) ) != 0 )
    {
        //printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        sem_wait( &p2psem );
		
		#ifdef SENSOR_3861
		if(bSnapshotVideo)
		{
			nSnapshotVideo++;
			if(nSnapshotVideo == 25 )
			{
				nSnapshotVideo = 0;
				Textout("one frame by sent");
			}
			else
			{
				usleep(10*1000);
				continue;
			}
		}
        #endif

		
        for ( i = 0; i < MAX_P2P_CONNECT; i++ )
        {
            if ( ( p2pstream[i].socket == -1 ) || ( p2pstream[i].liveflag == 0x00 ) )
            {
                continue;
            }

			/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/14 */
			#if (!defined(JINQIANXIANG) && !defined(BAFANGDIANZI))
				if (!( p2pstream[i].state == PS_TALKING ||
					p2pstream[i].stateEx == PS_WATCHING ||
					p2pstream[i].state == PS_CALLING ))
				{
					P2PTextout("STATE is ERROR!! Stop Video(%d)", i);
					p2pstream[i].liveflag = 0;
					usleep(1000);
					continue;
				}
			#endif

            wifiok = 1;
            #ifdef SENSOR_3861
            p2pmediasend( i );
            #else
            p2pmediasendH264( i );
            #endif
        }

        usleep( 15 * 1000 );
    }
}

BOOL IsSendingVideo()
{
    int i=0;
    for(i=0; i<MAX_P2P_CONNECT; i++)
    {
        if ( ( p2pstream[i].socket != -1 ) && ( p2pstream[i].liveflag == 0x01 ) )
            return TRUE;
    }    

    return FALSE;
}

int P2PReadFileData( int usit, unsigned char* pbuf, unsigned int* length )
{
    FILE*			 fp;
    LIVEHEAD               phead;
    int				iRet;
    int 				status = LIVE_RECORD_END;
    unsigned int		len = 0;
    char				filename[64];;

    Textout1( "Read SDCard recorder file[%s]\n", p2pstream[usit].filename );
    fp = fopen( p2pstream[usit].filename, "rb" );

    if ( fp == NULL )
    {
        return LIVE_RECORD_NOT;
    }

    Textout1( "recordfirstflag=%d\n", p2pstream[usit].recordfirstflag );

    if ( p2pstream[usit].recordfirstflag == 0x01 )
    {
        //find startcode
        p2pstream[usit].recordfirstflag = 0x00;

        while ( !feof( fp ) )
        {
            fseek( fp, p2pstream[usit].recordlen, SEEK_SET );
            iRet = fread( &phead, 1, sizeof( LIVEHEAD ), fp );

            //find start key frame
            Textout1( "read file, [audio=0x06, mj=0x03, h264=0,1]type=%d, len=%d", phead.type, phead.len );

            if ( ( phead.startcode == STARTCODE ) && ( phead.type == 0x03 ) )
            {
                if ( ( phead.len + sizeof( LIVEHEAD ) ) >= MAX_JPEG_BUFFER_SIZE )
                {
                    //printf( "phead.len=%d, >= 256*1024\n", phead.len );
                    status =  LIVE_RECORD_BAD;
                }

                else
                {
                    //printf("find start head and len %d\n",phead.len);
                    phead.streamid = RECORDRATE;
                    phead.sessid = usit;
                    memcpy( pbuf, &phead, sizeof( LIVEHEAD ) );
                    fread( pbuf + sizeof( LIVEHEAD ), 1, phead.len, fp );
                    len =  phead.len + sizeof( LIVEHEAD );
                    status = LIVE_RECORD_OK;
                    p2pstream[usit].recordlen += len;
                }

                break;
            }

            p2pstream[usit].recordlen++;
        }
    }

    else
    {
        fseek( fp, p2pstream[usit].recordlen, SEEK_SET );
        iRet = fread( &phead, 1, sizeof( LIVEHEAD ), fp );

        Textout1( "read file, startcode=0x%08x, type=%d, len=%d", phead.startcode, phead.type, phead.len );

        if ( phead.startcode == STARTCODE )
        {
            if ( ( phead.len + sizeof( LIVEHEAD ) ) >= MAX_JPEG_BUFFER_SIZE )
            {
                //printf( "phead.len=%d, >= 256*1024\n", phead.len );
                status =  LIVE_RECORD_BAD;
            }

            else
            {
                phead.streamid = RECORDRATE;
                phead.sessid = usit;
                memcpy( pbuf, &phead, sizeof( LIVEHEAD ) );
                fread( pbuf + sizeof( LIVEHEAD ), 1, phead.len, fp );
                len =  phead.len + sizeof( LIVEHEAD );
                status = LIVE_RECORD_OK;
                p2pstream[usit].recordlen += len;
            }
        }

        else
        {
            Textout1( "set recordfirstflag = 1" );
            p2pstream[usit].recordfirstflag = 1;
            len = 1;
            status = LIVE_RECORD_OK;
        }
    }

    fclose( fp );
    *length = len;
    //printf("");
    //printf("offset %d len %d\n",userindr[usit].offset,len);
    return status;
}

void NoteP2pAudio( void )
{
    sem_post( &p2paudio );
}

void* p2paudioThreadProc( void* p )
{
    int i = 0;

    if ( ( sem_init( &p2paudio, 0, 0 ) ) != 0 )
    {
        //printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        sem_wait( &p2paudio );

        for ( i = 0; i < MAX_P2P_CONNECT; i++ )
        {
            if ( p2pstream[i].socket == -1 || p2pstream[i].audioflag == 0x00)
            {
                continue;
            }

			if (!( p2pstream[i].stateEx == PS_WATCHING || p2pstream[i].state == PS_TALKING ))
			{
                continue;
			}

			//if ( AlarmAudioPlaying() )
			if(GetPlayAlarmAudioState())
			{
                usleep( 1000*1000 );
				continue;
			}

            
            p2paudiosend( i );
        }

        usleep( 10*1000 );
    }
}


int sendP2PAlarm( int alarmType )
{
    int writesize = 0;
    int readsize = 0;
    int i;
    Textout( "sendP2PAlarm... alarmType: %d", alarmType );

    for ( i = 0; i < MAX_P2P_CONNECT; i++ )
    {
        if ( p2pstream[i].socket == -1 )
        {
            continue;
        }

        int iRet = PPPP_Check_Buffer( p2pstream[i].socket, P2P_ALARMCHANNEL, &writesize, &readsize );

        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[i].socket );
            continue;
        }

        if ( writesize >= 8 * 1024 )
        {
            continue;
        }

        CMDHEADOLD head;
        memset( &head, 0, sizeof( head ) );
        head.startcode = 0x0a01;
        head.version   = 0x0100;
        head.cmd  = CGI_ALARM_NOTIFY;
        head.len       = sizeof( int );
        char temp[128] ;
        int alarm = 0 ;

        if ( alarmType == MOTION_ALARM )
        {
            alarm = 1;
        }
        else if ( alarmType == NOTE_CLIENT_CAP_PIC )
        {
            alarm = 7;
        }
        else if ( alarmType == NOTE_CLIENT_RECORD )
        {
            alarm = 8;
        }

        else
        {
            alarm = 2;
        }

        memcpy( temp, ( char* )&head, sizeof( head ) );
        memcpy( temp + sizeof( head ), ( char* )&alarm, sizeof( int ) );
        int len = head.len + sizeof( CMDHEAD );

        iRet = PPPP_Write(p2pstream[i].socket,P2P_ALARMCHANNEL,temp,len);
        if ( iRet < 0 )
        {
            PopP2pSocket( p2pstream[i].socket );
            continue;
        }
    }

    return 1;
}

int P2PStart( void )
{
    pthread_t p2pinitthread;
    pthread_t p2plistenthread;
    pthread_t p2pcmdthread;
    pthread_t p2pmediathread;
    pthread_t p2paudiothread;
    pthread_t p2ptalkthread;
    

    P2p_streaminit();

    pthread_create( &p2plistenthread, 0, &p2plistenThreadProc, NULL );
	pthread_detach(p2plistenthread);

    pthread_create( &p2pinitthread, 0, &p2pInitThreadProc, NULL );
	pthread_detach(p2pinitthread);

    pthread_create( &p2pcmdthread, 0, &p2pcmdThreadProc, NULL );
	pthread_detach(p2pcmdthread);

    pthread_create( &p2pmediathread, 0, &p2pmediaThreadProc, NULL );
	pthread_detach(p2pmediathread);

    pthread_create( &p2paudiothread, 0, &p2paudioThreadProc, NULL );
	pthread_detach(p2paudiothread);

    pthread_create( &p2ptalkthread, 0, &p2ptalkThreadProc, NULL );
	pthread_detach(p2ptalkthread);

    return 0;
}



