#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include	<signal.h>
#include <sys/stat.h>
#include<unistd.h>
#include "cmdhead.h"
#include "param.h"
#include "debug.h"

#include "alarm.h"
#include <wchar.h>

#define FILE_FTP_TEST_RESULT    "/tmp/ftpret.txt"

pthread_mutex_t         ftpmutext = PTHREAD_MUTEX_INITIALIZER;

void ftplock( void )
{
    pthread_mutex_lock( &ftpmutext );
}

void ftpunlock( void )
{
    pthread_mutex_unlock( &ftpmutext );
}

void FtpFileTest( void )
{
    FILE*    fp = NULL;
    char	temp[16];
    fp = fopen( "/tmp/ftptest.txt", "wb" );

    if ( fp == NULL )
    {
        printf( "ftp config failed\n" );
        return;
    }

    fwrite( "ftp test is ok by ", 1, 18, fp );
    memset( temp, 0x00, 16 );
    memcpy( temp, bparam.stNetParam.szIpAddr, 16 );
    fwrite( temp, 1, strlen( temp ), fp );
    fclose( fp );
}
int sub_dir(int fp, char * dir)
{
	char	cmd[128];
	char *p;
	int flag;
	p = strtok(dir, "/");
	if( p ){
		memset( cmd, 0x00, 128 );
		sprintf( cmd, "mkdir  %s\n", p);
		fwrite( cmd, 1, strlen( cmd ), fp );

		memset( cmd, 0x00, 128 );
		sprintf( cmd, "cd %s\n", p);
		fwrite( cmd, 1, strlen( cmd ), fp );
		flag = 0;
	}
	else 
		flag = 1;
	while( 1 )
	{
		p = strtok(NULL, "/");
		if( p ){
			memset( cmd, 0x00, 128 );
			sprintf( cmd, "mkdir  %s\n", p);
			fwrite( cmd, 1, strlen( cmd ), fp );

			memset( cmd, 0x00, 128 );
			sprintf( cmd, "cd %s\n", p);
			fwrite( cmd, 1, strlen( cmd ), fp );
		}
		else 
			break;
	}
	return flag;
}

int FtpConfig( char test, char* filename )
{
    char    cmd[128];
    FILE*	fp = NULL;
    struct stat     stStat;
    int iRet;
	int flag = 0;

    if ( bparam.stFtpParam.szFtpSvr[0] == 0x00 )
    {
        return -1;
    }

    if ( bparam.stFtpParam.szFtpUser[0] == 0x00 )
    {
        return -1;
    }

    if ( bparam.stFtpParam.szFtpPwd[0] == 0x00 )
    {
        return -1;
    }

    fp = fopen( "/tmp/ftpupdate1.sh", "wb" );

    if ( fp == NULL )
    {
        printf( "ftp config failed\n" );
        return -1;
    }

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "/system/system/bin/ftp -n<<!\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "open %s %d\n", bparam.stFtpParam.szFtpSvr, bparam.stFtpParam.nFtpPort );
    fwrite( cmd, 1, strlen( cmd ), fp );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "user %s %s\n", bparam.stFtpParam.szFtpUser, bparam.stFtpParam.szFtpPwd );
    fwrite( cmd, 1, strlen( cmd ), fp );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "binary\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

    if ( bparam.stFtpParam.byMode == 1 ) 	//passive
    {
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "pass\n" );
        fwrite( cmd, 1, strlen( cmd ), fp );
    }
#ifdef CUSTOM_DIR

	char sub_temp[ 128 ];
	memset(sub_temp, 0, 128);
	//strcpy(sub_temp, bparam.stFtpParam.szFtpDir);
	sprintf(sub_temp, "%s/%s", bparam.stFtpParam.szFtpDir,bparam.stIEBaseParam.dwDeviceID); 

    flag = sub_dir(fp,sub_temp);
	if(flag){
		memset( cmd, 0x00, 128 );
    	sprintf( cmd, "cd %s\n", bparam.stFtpParam.szFtpDir );
    	fwrite( cmd, 1, strlen( cmd ), fp );
	}
#else
	memset( cmd, 0x00, 128 );
    sprintf( cmd, "cd %s\n", bparam.stFtpParam.szFtpDir );
    fwrite( cmd, 1, strlen( cmd ), fp );

#endif
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "lcd /tmp\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

    if ( test == 0x01 )
    {
        FtpFileTest();
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "put ftptest.txt\n" );
        fwrite( cmd, 1, strlen( cmd ), fp );
    }

    else
    {
        char    filename1[128];
        memset( filename1, 0x00, 128 );
        memcpy( filename1, filename + 5, strlen( filename ) - 5 );
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "put %s\n", filename1 );
        fwrite( cmd, 1, strlen( cmd ), fp );
    }

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "close\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "bye\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "!\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );
    fclose( fp );
    iRet = access( "/tmp/ftpupdate1.sh", X_OK );

    if ( iRet )
    {
        DoSystem( "chmod a+x /tmp/ftpupdate1.sh" );
    }

    return 0;
}

int FtpConfigAlarm( char test, char* capfilename, char* filename )
{
    char    cmd[128];
    FILE*	fp = NULL;
    struct stat     stStat;
    int iRet;
	int flag = 0;

    if ( bparam.stFtpParam.szFtpSvr[0] == 0x00 )
    {
        return -1;
    }

    if ( bparam.stFtpParam.szFtpUser[0] == 0x00 )
    {
        return -1;
    }

    if ( bparam.stFtpParam.szFtpPwd[0] == 0x00 )
    {
        return -1;
    }

    fp = fopen( "/tmp/ftpupdate.sh", "wb" );

    if ( fp == NULL )
    {
        printf( "ftp config failed\n" );
        return -1;
    }

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "/system/system/bin/ftp -n<<!\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "open %s %d\n", bparam.stFtpParam.szFtpSvr, bparam.stFtpParam.nFtpPort );
    fwrite( cmd, 1, strlen( cmd ), fp );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "user %s %s\n", bparam.stFtpParam.szFtpUser, bparam.stFtpParam.szFtpPwd );
    fwrite( cmd, 1, strlen( cmd ), fp );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "binary\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

    if ( bparam.stFtpParam.byMode == 1 ) 	//passive
    {
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "pass\n" );
        fwrite( cmd, 1, strlen( cmd ), fp );
    }
#ifdef CUSTOM_DIR
	char sub_temp[ 128 ];
	memset(sub_temp, 0, 128);
	sprintf(sub_temp, "%s/%s", bparam.stFtpParam.szFtpDir,bparam.stIEBaseParam.dwDeviceID); 
	//strcpy(sub_temp, bparam.stFtpParam.szFtpDir);

    flag = sub_dir(fp,sub_temp);
	if(flag){
   		memset( cmd, 0x00, 128 );
    	sprintf( cmd, "cd %s\n", bparam.stFtpParam.szFtpDir );
    	fwrite( cmd, 1, strlen( cmd ), fp );
	}
#else
		memset( cmd, 0x00, 128 );
    	sprintf( cmd, "cd %s\n", bparam.stFtpParam.szFtpDir );
    	fwrite( cmd, 1, strlen( cmd ), fp );

#endif

#ifdef	CREATE_FTP_SUBDIRECTORY

    /* BEGIN: Added by wupm, 2013/2/1   Version:6 */
    //filename=[/tmp/00_3a_58_19_3E_00__1_20130131110813_4.jpg]
    //[         /tmp/00_02_2A_F3_08_DC_JWEV-007533-LVFBH_0_20130425053328_8.jpg
    if ( 1 )
    {
        int nFileNameLen = 0;
        char szTemp[128];
        memset( szTemp, 0, 128 );

	/* BEGIN: Added by wupm, 2013/6/14 */
	#ifdef	FTP_SUBDIRECTORY_UID_OR_MAC
		if ( bparam.stIEBaseParam.dwDeviceID[0] == 0 )
		{
			memcpy(szTemp, bparam.stIEBaseParam.szMac, 17);
			szTemp[2] = '-';
			szTemp[5] = '-';
			szTemp[8] = '-';
			szTemp[11] = '-';
			szTemp[14] = '-';
		}
		else
		{
			/* BEGIN: Modified by wupm, 2013/7/1 */
			//strcpy(szTemp, bparam.stIEBaseParam.dwDeviceID[0]);
			strcpy(szTemp, bparam.stIEBaseParam.dwDeviceID);
		}
        Textout( "FTP Sub Directory = [%s]", szTemp );
        memset( cmd, 0x00, 128 );
	#else
		/* BEGIN: Modified by wupm, 2013/4/25 */
        //nFileNameLen = strlen( szTemp );
        nFileNameLen = strlen( filename );

        memcpy( szTemp, filename + nFileNameLen - 20, 8 );
		Textout("FileName = {%s}", filename);
		Textout("cap File name = [%s]", capfilename);
        Textout( "FTP Sub Directory = [%s]", szTemp );
        memset( cmd, 0x00, 128 );
	#endif

		/* BEGIN: Modified by wupm, 2013/4/25 */
        //sprintf( cmd, "mkdir -p %s\n", szTemp );
        sprintf( cmd, "mkdir %s\n", szTemp );

        fwrite( cmd, 1, strlen( cmd ), fp );
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "cd %s\n", szTemp );
        fwrite( cmd, 1, strlen( cmd ), fp );
    }

#endif

    /* END:   Added by wupm, 2013/2/1 */
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "lcd /tmp\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

    if ( test == 0x01 )
    {
        FtpFileTest();
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "put ftptest.txt\n" );
        fwrite( cmd, 1, strlen( cmd ), fp );
    }

    else
    {
        char    filename1[128];
        char   filename2[32];
        memset( filename1, 0x00, 128 );
        memcpy( filename1, filename + 5, strlen( filename ) - 5 );
        memset( filename2, 0x00, 32 );
        memcpy( filename2, capfilename + 5, strlen( capfilename ) - 5 );
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "put %s %s\n", filename2, filename1 );
		Textout("COMMAND = [%s]", cmd);
        fwrite( cmd, 1, strlen( cmd ), fp );
    }

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "close\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "bye\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "!\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

	/* BEGIN: Added by wupm, 2013/4/25 */
    /* BEGIN: Deleted by Baggio.wu, 2013/7/19 */
    //move to where file create
    #if 0
    memset( cmd, 0x00, 128 );
	sprintf(cmd, "rm -f %s", capfilename);
    fwrite( cmd, 1, strlen( cmd ), fp );
    #endif
    
    fclose( fp );
    iRet = access( "/tmp/ftpupdate.sh", X_OK );

    if ( iRet )
    {
        DoSystem( "chmod a+x /tmp/ftpupdate.sh" );
    }

    return 0;
}

#ifdef CUSTOM_FTP_ALARM
void customFtpSend(char AlarmType,
	char* filename1,
	char* filename2,
	char* filename3)
{
	int     iRet = 0;
    char param[4];
	char target_filename1[256];
	char target_filename2[256];
	char target_filename3[256];
	int nFileNameLen = 0;

	char    cmd[128];
    FILE*	fp = NULL;
    struct stat     stStat;

    if ( bparam.stFtpParam.szFtpSvr[0] == 0x00 )
    {
        return -1;
    }

    if ( bparam.stFtpParam.szFtpUser[0] == 0x00 )
    {
        return -1;
    }

    if ( bparam.stFtpParam.szFtpPwd[0] == 0x00 )
    {
        return -1;
    }

    iRet = access( "/tmp/ftpupdate3.sh", X_OK );
    if ( iRet )
    {
        Textout("file exist, rm -f /tmp/ftpupdate3.sh");
        DoSystem( "rm -f /tmp/ftpupdate3.sh" );
    }

    fp = fopen( "/tmp/ftpupdate3.sh", "wb" );
    if ( fp == NULL )
    {
        printf( "ftp config failed\n" );
        return -1;
    }

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "/system/system/bin/ftp -n<<!\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "open %s %d\n", bparam.stFtpParam.szFtpSvr, bparam.stFtpParam.nFtpPort );
    fwrite( cmd, 1, strlen( cmd ), fp );

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "user %s %s\n", bparam.stFtpParam.szFtpUser, bparam.stFtpParam.szFtpPwd );
    fwrite( cmd, 1, strlen( cmd ), fp );

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "binary\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

    if ( bparam.stFtpParam.byMode == 1 ) 	//passive
    {
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "pass\n" );
        fwrite( cmd, 1, strlen( cmd ), fp );
    }


#if 1//def	CREATE_FTP_SUBDIRECTORY

    //filename1=[/tmp/20130131110813_4.jpg]
    if ( 1 )
    {
		char filename[256];
        char szTemp[128];
        nFileNameLen=strlen(filename1);
		if ( nFileNameLen == 0 )
		{
			Textout("No File, return");
			return 1;
		}

        memset(filename, 0, 256);
        strcpy(filename, filename1);

        memset( szTemp, 0, 128 );
        memcpy( szTemp, filename + 5, 8 );
		Textout("FileName = {%s}", filename);
        Textout( "FTP Sub Directory = [%s]", szTemp );
        memset( cmd, 0x00, 128 );

        sprintf( cmd, "mkdir %s\n", szTemp );

        fwrite( cmd, 1, strlen( cmd ), fp );
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "cd %s\n", szTemp );
        fwrite( cmd, 1, strlen( cmd ), fp );

		///////

        memset( szTemp, 0, 128 );
		sprintf(szTemp, "%s", bparam.stIEBaseParam.dwDeviceID);
        Textout( "FTP Sub Directory = [%s]", szTemp );
        memset( cmd, 0x00, 128 );

        sprintf( cmd, "mkdir %s\n", szTemp );

        fwrite( cmd, 1, strlen( cmd ), fp );
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "cd %s\n", szTemp );
        fwrite( cmd, 1, strlen( cmd ), fp );
    }

	memset(target_filename1, 0, 256);
	memset(target_filename2, 0, 256);
	memset(target_filename3, 0, 256);

    if ( filename1[0] != 0 )
    {
		sprintf(target_filename1,
			"%c"
			"%c%c%c%c%c%c"
			"%c%c%c%c%c%c"
			"%c"
			"%c.jpg",
			( AlarmType == GPIO_ALARM || AlarmType == MOTION_ALARM ) ? 'A' : 'P',
			filename1[7],
			filename1[8],
			filename1[9],
			filename1[10],
			filename1[11],
			filename1[12],
			filename1[13],
			filename1[14],
			filename1[15],
			filename1[16],
			filename1[17],
			filename1[18],
			'1',	//0:SD,1:FTP,2:EMAIL
			filename1[nFileNameLen - 5]);
		Textout("target_filename 1 = [%s]", target_filename1);
		//Textout("filename1 = [%s]", filename1);
		//Textout("", );
    }
	if ( filename2[0] != 0 )
    {
		sprintf(target_filename2,
			"%c"
			"%c%c%c%c%c%c"
			"%c%c%c%c%c%c"
			"%c"
			"%c.jpg",
			( AlarmType == GPIO_ALARM || AlarmType == MOTION_ALARM ) ? 'A' : 'P',
			filename2[7],
			filename2[8],
			filename2[9],
			filename2[10],
			filename2[11],
			filename2[12],
			filename2[13],
			filename2[14],
			filename2[15],
			filename2[16],
			filename2[17],
			filename2[18],
			'1',	//0:SD,1:FTP,2:EMAIL
			filename2[nFileNameLen - 5]);
		Textout("target_filename 2 = [%s]", target_filename2);
    }
	if ( filename3[0] != 0 )
    {
		sprintf(target_filename3,
			"%c"
			"%c%c%c%c%c%c"
			"%c%c%c%c%c%c"
			"%c"
			"%c.jpg",
			( AlarmType == GPIO_ALARM || AlarmType == MOTION_ALARM ) ? 'A' : 'P',
			filename3[7],
			filename3[8],
			filename3[9],
			filename3[10],
			filename3[11],
			filename3[12],
			filename3[13],
			filename3[14],
			filename3[15],
			filename3[16],
			filename3[17],
			filename3[18],
			'1',	//0:SD,1:FTP,2:EMAIL
			filename3[nFileNameLen - 5]);
		Textout("target_filename 3 = [%s]", target_filename3);
    }

#endif

    /* END:   Added by wupm, 2013/2/1 */
    memset( cmd, 0x00, 128 );
    sprintf( cmd, "lcd /tmp\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

	/*
    if ( test == 0x01 )
    {
        FtpFileTest();
        memset( cmd, 0x00, 128 );
        sprintf( cmd, "put ftptest.txt\n" );
        fwrite( cmd, 1, strlen( cmd ), fp );
    }

    else*/
    {
        char    localfile[128];
        //char   filename2[128];

		if ( filename1[0] != 0 )
		{
	        memset( localfile, 0x00, 128 );
	        memcpy( localfile, filename1 + 5, strlen( filename1 ) - 5 );
	        //memset( filename2, 0x00, 128 );
	        //memcpy( filename2, capfilename + 5, strlen( capfilename ) - 5 );

	        memset( cmd, 0x00, 128 );
	        sprintf( cmd, "put %s %s\n", localfile, target_filename1);
			Textout("COMMAND = [%s]", cmd);
	        fwrite( cmd, 1, strlen( cmd ), fp );
		}

		if ( filename2[0] != 0 )
		{
	        memset( localfile, 0x00, 128 );
	        memcpy( localfile, filename2 + 5, strlen( filename2 ) - 5 );
	        //memset( filename2, 0x00, 128 );
	        //memcpy( filename2, capfilename + 5, strlen( capfilename ) - 5 );

	        memset( cmd, 0x00, 128 );
	        sprintf( cmd, "put %s %s\n", localfile, target_filename2);
			Textout("COMMAND = [%s]", cmd);
	        fwrite( cmd, 1, strlen( cmd ), fp );
		}

		if ( filename3[0] != 0 )
		{
	        memset( localfile, 0x00, 128 );
	        memcpy( localfile, filename3 + 5, strlen( filename3 ) - 5 );
	        //memset( filename2, 0x00, 128 );
	        //memcpy( filename2, capfilename + 5, strlen( capfilename ) - 5 );

	        memset( cmd, 0x00, 128 );
	        sprintf( cmd, "put %s %s\n", localfile, target_filename3);
			Textout("COMMAND = [%s]", cmd);
	        fwrite( cmd, 1, strlen( cmd ), fp );
		}
    }

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "close\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "bye\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

    memset( cmd, 0x00, 128 );
    sprintf( cmd, "!\n" );
    fwrite( cmd, 1, strlen( cmd ), fp );

	/* BEGIN: Added by wupm, 2013/4/25 */
    memset( cmd, 0x00, 128 );
	//sprintf(cmd, "rm -f %s", capfilename);
	sprintf(cmd, "rm -f %s", filename1);
    fwrite( cmd, 1, strlen( cmd ), fp );

    memset( cmd, 0x00, 128 );
	//sprintf(cmd, "rm -f %s", capfilename);
	sprintf(cmd, "rm -f %s", filename2);
    fwrite( cmd, 1, strlen( cmd ), fp );

	memset( cmd, 0x00, 128 );
	//sprintf(cmd, "rm -f %s", capfilename);
	sprintf(cmd, "rm -f %s", filename3);
    fwrite( cmd, 1, strlen( cmd ), fp );

    fclose( fp );
    iRet = access( "/tmp/ftpupdate3.sh", X_OK );
    if ( iRet )
    {
        DoSystem( "chmod a+x /tmp/ftpupdate3.sh" );
    }

    /*return 0;
    if ( -1 == iRet )
    {
        return iRet;
    }*/

    // (iRet == 0){
    ftp_dbg( "alarm upload41\n", 0 );
    //gnal(SIGCHLD,SIG_IGN);
    //gnal(SIGCHLD,SIG_DFL);
    //FtpWrite(param,4);
    iRet = DoSystem( "/tmp/ftpupdate3.sh" );
    ftp_dbg( "alarm upload42\n", 0 );
    //}
    //ftpunlock();
    return iRet;
}

#endif

int FtpsendAlarm( char* capfilename, char* filename )
{
    int     iRet = 0;
    char param[4];
    //ftplock();
    Textout("7");
    ftp_dbg( "alarm upload40\n", 0 );
    iRet = FtpConfigAlarm( 0x00, capfilename, filename );

    if ( -1 == iRet )
    {
		Textout("FtpConfigAlarm failed");
        return iRet;
    }

    // (iRet == 0){
    ftp_dbg( "alarm upload41\n", 0 );
    //gnal(SIGCHLD,SIG_IGN);
    //gnal(SIGCHLD,SIG_DFL);
    //FtpWrite(param,4);
    Textout("8");
    iRet = DoSystem( "/tmp/ftpupdate.sh &" );
    ftp_dbg( "alarm upload42\n", 0 ); 
    //}
    //ftpunlock();
    Textout("9 - FTP Over");
    return iRet;
}
int Ftpsend( char* filename )
{
    int     iRet = 0;
    //ftplock();
    iRet = FtpConfig( 0x00, filename );

    if ( iRet == 0 )
    {
        //gnal(SIGCHLD,SIG_DFL);
        iRet = DoSystem( "/tmp/ftpupdate1.sh &" );
        //gnal(SIGCHLD,SIG_IGN);
    }

    //ftpunlock();
    return iRet;
}

int GetFtpResult( void )
{
    FILE*    fp = NULL;
    char*    pdst = NULL;
    char    temp[256];
    int	iRet = 0;
    fp = fopen( FILE_FTP_TEST_RESULT, "r" );

    if ( fp == NULL )
    {
        printf( "fopen failed\n" );
        return -1;
    }

    //fgets( temp, 256, fp );
    fread( temp, 1, 256, fp );
    fclose(fp);

    pdst = strstr( temp, "Not connected" );
    if ( pdst != NULL )
    {
        return -2;
    }

    pdst = strstr( temp, "Login" );
    if ( pdst != NULL )
    {
        return -3;
    }

	/* BEGIN: Added by wupm, 2013/7/4 */
	pdst = strstr( temp, "Not logged in" );
    if ( pdst != NULL )
    {
        return -4;
    }

	/* BEGIN: Added by wupm, 2013/7/4 */
	pdst = strstr( temp, "Login failed" );
    if ( pdst != NULL )
    {
        return -5;
    }

    return iRet;
}

int GetFtpTestResult()
{
    int iRet = 0;

    int nIntervalTime=0;
    while(nIntervalTime < 5)
    {
        iRet = access( FILE_FTP_TEST_RESULT, F_OK );
        if ( iRet == 0 )
        {
            Textout("Get the result file of FTP test");
            break;
        }
        else
        {
            nIntervalTime++;
            sleep(1);
            Textout("nIntervalTime = %d", nIntervalTime);
        }
    } 

    iRet = GetFtpResult();
    Textout( "Ftp result=%d", iRet );


    /* BEGIN: Added by Baggio.wu, 2013/10/25 */
    //DoSystem( "rm /tmp/ftpret.txt" );

    //rm result file
    if(1)
    {
        char cmd[32];
        memset(cmd, 0, 32);
        sprintf(cmd, "rm %s", FILE_FTP_TEST_RESULT);
        DoSystem(cmd);
    }


    return iRet;
}

int DoFtpTest( void )
{
    int     iRet = 0;
    iRet = FtpConfig( 0x01, NULL );

    if ( iRet == 0 )
    {
        char cmd[128];
        memset(cmd, 0, 128);
        sprintf(cmd, "/tmp/ftpupdate1.sh > %s", FILE_FTP_TEST_RESULT);
        iRet = DoSystem(cmd);
        //iRet = DoSystem( "/tmp/ftpupdate1.sh > /tmp/ftpret.txt" );
    }

    return iRet;
}

