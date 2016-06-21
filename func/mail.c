#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include "cmdhead.h"
#include "param.h"
#include "alarm.h"
#include "debug.h"

#define FILE_MAIL_TEST_RESULT   "/tmp/mrt.txt"
char			maildns[64];

void emailtest( void )
{
    DoSystem( "echo \"mail test ok\"|/system/system/bin/mailx -v -s \"mail test\" 517929728@qq.com" );
#if 0
    system( "sendmail -s \"test\" -H 517929728:zengjiahao@smtp.qq.com:25 -f 517929728@qq.com < /tmp/dhcp.txt -t 517929728@qq.com -a /tmp/ssmtp.conf" );
#endif
}

int DoMailTest( void ) 	//email test
{
    int     iRet = -1;
    char    cmd[256];

    if ( bparam.stMailParam.szSender[0] == 0 )
    {
        return -1;
    }

    if ( bparam.stMailParam.szReceiver1[0] != 0x00 )
    {
        iRet = EmailConfig();

        if ( iRet )
        {
            return -1;
        }

        memset( cmd, 0x00, 256 );

        /* BEGIN: Modified by Baggio.wu, 2013/9/9 */
        sprintf( cmd, "echo \"mail test ok\" | /system/system/bin/mailx -r %s -s \"mail test\"  %s",
                 bparam.stMailParam.szSender, bparam.stMailParam.szReceiver1 );
        //sprintf( cmd, "echo \"mail test ok\" | /system/system/bin/mailx -v -s \"mail test\"  %s",
        //         bparam.stMailParam.szReceiver1 );

        printf( "start cmd:%s\n", cmd );
        EmailWrite( cmd, strlen( cmd ) );
        //emailtest();
        printf( "cmd:%s\n", cmd );

    }

    return iRet;
}

int GetMailTestResult()
{
    int iRet =0;
    FILE*	fp = NULL;
    unsigned char value = 0;

    int nIntervalTime=0;
    while(nIntervalTime < 5)
    {
        iRet = access( FILE_MAIL_TEST_RESULT, F_OK );
        if ( iRet == 0 )
        {
            Textout("Get the result file of mail test");
            break;
        }
        else
        {
            nIntervalTime++;
            sleep(1);
            Textout("nIntervalTime = %d", nIntervalTime);
        }
    }

    fp = fopen( FILE_MAIL_TEST_RESULT, "rb" );

    if ( fp == NULL )
    {
        return -1;
    }

    value = 0;
    fread( &value, 1, 1, fp );

    if ( value == 0x30 )
    {
        iRet = 0;
    }

    else
    {
        iRet = -1;
    }

    fclose( fp );

    /* BEGIN: Added by Baggio.wu, 2013/10/25 */

    //DoSystem( "rm /tmp/mrt.txt" );

    //rm result file
    {
        char cmd[32];
        memset(cmd, 0, 32);
        sprintf(cmd, "rm %s", FILE_MAIL_TEST_RESULT);
        DoSystem(cmd);
    }
    /* END:   Added by Baggio.wu, 2013/10/25 */

    Textout( "sed mail iRet=%d, get mrt.txt value=%d\n", iRet, value );
    return iRet;
}

int GetDnsName( void )
{
    int len;
    int len1;
    char* pdst = NULL;

    memset( maildns, 0x00, 64 );
    strcpy(maildns, "DoorBell find alarm");
    len = strlen( bparam.stDdnsParam.serveruser );
    if(len != 0)
    {
        strcat( maildns, bparam.stDdnsParam.serveruser );
    }

    return 0;
}

#if	0	//def	SEND_EMAIL_WITH_SIX_IMAGE
int EmailSendSixImage( char AlarmType,
	char* filename1,
	char* filename2,
	char* filename3,
	char* filename4,
	char* filename5,
	char* filename6
	)
{
    int	iRet = 0;
    char 	cmd[1024];
    char	type[128];

	char szTemp[512];
	char szTemp2[512];

	char szTemp3[512];
	char szTemp4[512];

    memset( type, 0, 128 );

    switch ( AlarmType )
    {
        case MOTION_TYPE:
			/* BEGIN: Modified by wupm, 2013/6/20 */
            //sprintf( type, "motion" );
            sprintf( type, "motion alarm mac=%c%c:%c%c:%c%c:%c%c:%c%c:%c%c ID=%s",
                     bparam.stIEBaseParam.szMac[0],
                     bparam.stIEBaseParam.szMac[1],
                     bparam.stIEBaseParam.szMac[3],
                     bparam.stIEBaseParam.szMac[4],
                     bparam.stIEBaseParam.szMac[6],
                     bparam.stIEBaseParam.szMac[7],
                     bparam.stIEBaseParam.szMac[9],
                     bparam.stIEBaseParam.szMac[10],
                     bparam.stIEBaseParam.szMac[12],
                     bparam.stIEBaseParam.szMac[13],
                     bparam.stIEBaseParam.szMac[15],
                     bparam.stIEBaseParam.szMac[16],
					bparam.stIEBaseParam.dwDeviceID
            	);
            break;

        case SENSOR_TYPE:
			/* BEGIN: Modified by wupm, 2013/6/20 */
            sprintf( type, "extern alarm mac=%c%c:%c%c:%c%c:%c%c:%c%c:%c%c ID=%s",
                     bparam.stIEBaseParam.szMac[0],
                     bparam.stIEBaseParam.szMac[1],
                     bparam.stIEBaseParam.szMac[3],
                     bparam.stIEBaseParam.szMac[4],
                     bparam.stIEBaseParam.szMac[6],
                     bparam.stIEBaseParam.szMac[7],
                     bparam.stIEBaseParam.szMac[9],
                     bparam.stIEBaseParam.szMac[10],
                     bparam.stIEBaseParam.szMac[12],
                     bparam.stIEBaseParam.szMac[13],
                     bparam.stIEBaseParam.szMac[15],
                     bparam.stIEBaseParam.szMac[16],
					bparam.stIEBaseParam.dwDeviceID
            	);
            break;
    }

    GetDnsName();

	memset(szTemp, 0, 512);
	memset(szTemp2, 0, 512);
	if (filename1[0] != 0 )
	{
		sprintf(szTemp2, "-a %s ", filename1);
		strcat(szTemp, szTemp2);
	}
	if (filename2[0] != 0 )
	{
		sprintf(szTemp2, "-a %s ", filename2);
		strcat(szTemp, szTemp2);
	}
	if (filename3[0] != 0 )
	{
		sprintf(szTemp2, "-a %s ", filename3);
		strcat(szTemp, szTemp2);
	}
	if (filename4[0] != 0 )
	{
		sprintf(szTemp2, "-a %s ", filename4);
		strcat(szTemp, szTemp2);
	}
	if (filename5[0] != 0 )
	{
		sprintf(szTemp2, "-a %s ", filename5);
		strcat(szTemp, szTemp2);
	}
	if (filename6[0] != 0 )
	{
		sprintf(szTemp2, "-a %s ", filename6);
		strcat(szTemp, szTemp2);
	}

#if	0
	memset(szTemp3, 0, 512);
	memset(szTemp4, 0, 512);
	if ( bparam.stMailParam.szReceiver1[0] != 0x00 )
	{
		sprintf(szTemp3,bparam.stMailParam.szReceiver1);
		strcat(szTemp4, szTemp3);
	}
	if ( bparam.stMailParam.szReceiver2[0] != 0x00 )
	{
		sprintf(szTemp3,bparam.stMailParam.szReceiver2);
		if ( szTemp4[0] != 0 )	strcat(szTemp4, ",");
		strcat(szTemp4, szTemp3);
	}
	if ( bparam.stMailParam.szReceiver3[0] != 0x00 )
	{
		sprintf(szTemp3,bparam.stMailParam.szReceiver3);
		if ( szTemp4[0] != 0 )	strcat(szTemp4, ",");
		strcat(szTemp4, szTemp3);
	}
	if ( bparam.stMailParam.szReceiver4[0] != 0x00 )
	{
		sprintf(szTemp3,bparam.stMailParam.szReceiver4);
		if ( szTemp4[0] != 0 )	strcat(szTemp4, ",");
		strcat(szTemp4, szTemp3);
	}

	if ( szTemp4[0] != 0 )
	{
		memset( cmd, 0x00, 1024 );
		sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx %s -s \"%s\" \"%s\"",
                 maildns, szTemp, type, szTemp4 );
        printf( "mail 1:%s\n", cmd );
        EmailWrite( cmd, strlen( cmd ) );
	}
#else
    if ( bparam.stMailParam.szReceiver1[0] != 0x00 )
    {
        /* BEGIN: Modified by wupm, 2013/4/22 */
        //memset( cmd, 0x00, sizeof( cmd ) );
        memset( cmd, 0x00, 1024 );
        //sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -a %s -s \"%s\" %s",
        //         maildns, filename, type, bparam.stMailParam.szReceiver1 );

		sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -r %s %s -s \"%s\" %s",
                 maildns, bparam.stMailParam.szSender, szTemp, type, bparam.stMailParam.szReceiver1 );
        printf( "mail 1:[%s], len = %d\n", cmd, strlen(cmd) );
        EmailWrite( cmd, strlen( cmd ) );
     }

    if ( bparam.stMailParam.szReceiver2[0] != 0x00 )
    {
		/* BEGIN: Modified by wupm, 2013/4/22 */
        //memset( cmd, 0x00, sizeof( cmd ) );
        memset( cmd, 0x00, 1024 );
        //sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -a %s -s \"%s\" %s",
        //         maildns, filename, type, bparam.stMailParam.szReceiver2 );
		sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -r %s %s -s \"%s\" %s",
                 maildns, bparam.stMailParam.szSender, szTemp, type, bparam.stMailParam.szReceiver2 );
        printf( "mail 2:%s\n", cmd );
        //iRet += system(cmd);
        EmailWrite( cmd, strlen( cmd ) );
    }

    if ( bparam.stMailParam.szReceiver3[0] != 0x00 )
    {
        /* BEGIN: Modified by wupm, 2013/4/22 */
        //memset( cmd, 0x00, sizeof( cmd ) );
        memset( cmd, 0x00, 1024 );
        //sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -a %s -s \"%s\" %s",
        //         maildns, filename, type, bparam.stMailParam.szReceiver3 );
		sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -r %s %s -s \"%s\" %s",
                 maildns, bparam.stMailParam.szSender, szTemp, type, bparam.stMailParam.szReceiver3 );
        printf( "mail 3:%s\n", cmd );
        //iRet += DoSystem(cmd);
        EmailWrite( cmd, strlen( cmd ) );
    }

    if ( bparam.stMailParam.szReceiver4[0] != 0x00 )
    {
        /* BEGIN: Modified by wupm, 2013/4/22 */
        //memset( cmd, 0x00, sizeof( cmd ) );
        memset( cmd, 0x00, 1024 );
        //sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -a %s -s \"%s\" %s",
        //         maildns, filename, type, bparam.stMailParam.szReceiver4 );
		sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -r %s %s -s \"%s\" %s",
                 maildns, bparam.stMailParam.szSender, szTemp, type, bparam.stMailParam.szReceiver4 );
        printf( "mail 4:%s\n", cmd );
        //iRet += DoSystem(cmd);
        EmailWrite( cmd, strlen( cmd ) );
    }
#endif
    return iRet;
}
#endif

int EmailSend(BOOL bCall, BOOL bMotion, BOOL bPir, char* filename ) 	//alarm email send
{
    int	iRet = 0;
    char 	cmd[1024];
    char	type[8];
    memset( type, 0, 8 );

    if(bCall)
        sprintf( type, "CALL" );
    else if(bPir)
        sprintf( type, "PIR" );
    else /*bMotion and else*/
        sprintf( type, "motion" );
    
    GetDnsName();

    if ( bparam.stMailParam.szReceiver1[0] != 0x00 )
    {
        memset( cmd, 0x00, sizeof( cmd ) );
        sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -r %s -a %s -s \"%s\" %s",
                 maildns, bparam.stMailParam.szSender, filename, type, bparam.stMailParam.szReceiver1 );
        printf( "mail 1:%s\n", cmd );
        EmailWrite( cmd, strlen( cmd ) );
    }

    if ( bparam.stMailParam.szReceiver2[0] != 0x00 )
    {
        memset( cmd, 0x00, sizeof( cmd ) );
        sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -r %s -a %s -s \"%s\" %s",
                 maildns, bparam.stMailParam.szSender, filename, type, bparam.stMailParam.szReceiver2 );
        printf( "mail 2:%s\n", cmd );
        //iRet += system(cmd);
        EmailWrite( cmd, strlen( cmd ) );
    }
    if ( bparam.stMailParam.szReceiver3[0] != 0x00 )
    {
        memset( cmd, 0x00, sizeof( cmd ) );
        sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -r %s -a %s -s \"%s\" %s",
                 maildns, bparam.stMailParam.szSender, filename, type, bparam.stMailParam.szReceiver3 );
        printf( "mail 3:%s\n", cmd );
        //iRet += DoSystem(cmd);
        EmailWrite( cmd, strlen( cmd ) );
    }
    if ( bparam.stMailParam.szReceiver4[0] != 0x00 )
    {
        memset( cmd, 0x00, sizeof( cmd ) );
        sprintf( cmd, "echo \"%s\"|/system/system/bin/mailx -r %s -a %s -s \"%s\" %s",
                 maildns, bparam.stMailParam.szSender, filename, type, bparam.stMailParam.szReceiver4 );
        printf( "mail 4:%s\n", cmd );
        //iRet += DoSystem(cmd);
        EmailWrite( cmd, strlen( cmd ) );
    }

    return iRet;
}

int EmailConfig1( void )
{
    FILE*    fp = NULL;
    char    temp[256];

    if ( bparam.stMailParam.szSmtpUser[0] == 0x00 )
    {
        return -1;
    }

    if ( bparam.stMailParam.szSmtpPwd[0] == 0x00 )
    {
        return -1;
    }

    memset( temp, 0x00, 256 );
    fp = fopen( "/tmp/revaliases", "wb" );

    if ( fp == NULL )
    {
        printf( "email config failed\n" );
        return -1;
    }

    memset( temp, 0x00, 256 );
    sprintf( temp, "root:%s:%s:%d\n", bparam.stMailParam.szSender, bparam.stMailParam.szSmtpSvr, bparam.stMailParam.nSmtpPort );
    fwrite( temp, 1, strlen( temp ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "mainuser:%s:%s:%d\n", bparam.stMailParam.szSender, bparam.stMailParam.szSmtpSvr, bparam.stMailParam.nSmtpPort );
    fwrite( temp, 1, strlen( temp ), fp );
    fclose( fp );
    return 0;
}

int EmailConfig( void )
{
    FILE*    fp = NULL;
    char    temp[256];

    if ( bparam.stMailParam.szSmtpUser[0] == 0x00 )
    {
        return -1;
    }

    if ( bparam.stMailParam.szSmtpPwd[0] == 0x00 )
    {
        return -1;
    }

    memset( temp, 0x00, 256 );
    fp = fopen( "/tmp/ssmtp.conf", "wb" );

    if ( fp == NULL )
    {
        printf( "email config failed\n" );
        return -1;
    }

    memset( temp, 0x00, 256 );
    sprintf( temp, "root=%s\n", bparam.stMailParam.szSender );
    fwrite( temp, 1, strlen( temp ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "mailhub=%s:%d\n", bparam.stMailParam.szSmtpSvr, bparam.stMailParam.nSmtpPort );
    fwrite( temp, 1, strlen( temp ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "rewriteDomain=\n" );
    fwrite( temp, 1, strlen( temp ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "hostname=%s:%d\n", bparam.stMailParam.szSmtpSvr, bparam.stMailParam.nSmtpPort );
    fwrite( temp, 1, strlen( temp ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "AuthUser=%s\n", bparam.stMailParam.szSmtpUser );
    fwrite( temp, 1, strlen( temp ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "AuthPass=%s\n", bparam.stMailParam.szSmtpPwd );
    fwrite( temp, 1, strlen( temp ), fp );
    memset( temp, 0x00, 256 );
    sprintf( temp, "FromLineOverride=YES\n" );
    fwrite( temp, 1, strlen( temp ), fp );

    if ( bparam.stMailParam.byCheck == 0x01 )
    {
        memset( temp, 0x00, 256 );
        sprintf( temp, "UseSTARTTLS=YES\n" );
        fwrite( temp, 1, strlen( temp ), fp );
    }

    if ( bparam.stMailParam.byCheck == 0x02 )
    {
        memset( temp, 0x00, 256 );
        sprintf( temp, "UseTLS=YES\n" );
        fwrite( temp, 1, strlen( temp ), fp );
    }

    fclose( fp );
    EmailConfig1();
    return 0;
}

void EmailInit( void )
{
    EmailConfig();
}

