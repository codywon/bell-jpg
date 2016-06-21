# Microsoft Developer Studio Project File - Name="webs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=webs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "webs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "webs.mak" CFG="webs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "webs - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "webs - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/etc/goAheadMocanaSSL/WIN", GPAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "webs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /W3 /O1 /D "NDEBUG" /D "__ENABLE_MOCANA_SSL_SERVER__" /D "__RTOS_WIN32__" /D "WEBS_SSL_SUPPORT" /D "WIN32" /D "_WINDOWS" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib /nologo /subsystem:windows /map /machine:I386

!ELSEIF  "$(CFG)" == "webs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /W3 /Gm /Zi /Od /D "_DEBUG" /D "ASSERT" /D "DEV" /D "__ENABLE_MOCANA_SSL_SERVER__" /D "__RTOS_WIN32__" /D "WEBS_SSL_SUPPORT" /D "WIN32" /D "_WINDOWS" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "WEBS" /d "WIN"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 advapi32.lib wsock32.lib kernel32.lib user32.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "webs - Win32 Release"
# Name "webs - Win32 Debug"
# Begin Group "mocana_ssl"

# PROP Default_Filter ""
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\mocana_ssl\common\asm_math.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\math_386.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\math_coldfire.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\mdefs.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\memory_debug.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\merrors.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\merrors.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\mocana.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\mocana.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\moptions.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\mrtos.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\mstdlib.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\mstdlib.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\mtcp.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\mtcp.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\mtypes.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\prime.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\prime.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\random.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\random.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\utils.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\utils.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\vlong.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\common\vlong.h
# End Source File
# End Group
# Begin Group "crypto"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\mocana_ssl\crypto\arc4.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\arc4.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\asn1cert.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\asn1cert.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\base64.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\base64m.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\blowfish.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\blowfish.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\des.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\des.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\dsa.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\dsa.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\md5.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\md5.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\rc4algo.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\rc4algo.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\rsa.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\rsa.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\sha1.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\sha1.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\three_des.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\crypto\three_des.h
# End Source File
# End Group
# Begin Group "platform"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\mocana_ssl\platform\linux_rtos.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\platform\linux_tcp.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\platform\solaris_rtos.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\platform\solaris_tcp.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\platform\vxworks_rtos.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\platform\vxworks_tcp.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\platform\win32_rtos.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\platform\win32_tcp.c
# End Source File
# End Group
# Begin Group "ssl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\mocana_ssl\ssl\sizedbuffer.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\ssl\sizedbuffer.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\ssl\ssl.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\ssl\ssl.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\ssl\ssl_util.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\ssl\ssl_util.h
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\ssl\sslsock.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl\ssl\sslsock.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=..\asp.c
# End Source File
# Begin Source File

SOURCE=..\balloc.c
# End Source File
# Begin Source File

SOURCE=..\base64.c
# End Source File
# Begin Source File

SOURCE=..\cgi.c
# End Source File
# Begin Source File

SOURCE=..\default.c
# End Source File
# Begin Source File

SOURCE=..\ejlex.c
# End Source File
# Begin Source File

SOURCE=..\ejparse.c
# End Source File
# Begin Source File

SOURCE=..\emfdb.c
# End Source File
# Begin Source File

SOURCE=..\form.c
# End Source File
# Begin Source File

SOURCE=..\h.c
# End Source File
# Begin Source File

SOURCE=..\handler.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=..\md5c.c
# End Source File
# Begin Source File

SOURCE=..\mime.c
# End Source File
# Begin Source File

SOURCE=..\misc.c
# End Source File
# Begin Source File

SOURCE=..\mocana_ssl.c
# End Source File
# Begin Source File

SOURCE=..\page.c
# End Source File
# Begin Source File

SOURCE=..\ringq.c
# End Source File
# Begin Source File

SOURCE=..\rom.c
# End Source File
# Begin Source File

SOURCE=..\security.c
# End Source File
# Begin Source File

SOURCE=..\sock.c
# End Source File
# Begin Source File

SOURCE=..\sockGen.c
# End Source File
# Begin Source File

SOURCE=..\sym.c
# End Source File
# Begin Source File

SOURCE=..\uemf.c
# End Source File
# Begin Source File

SOURCE=..\um.c
# End Source File
# Begin Source File

SOURCE=..\umui.c
# End Source File
# Begin Source File

SOURCE=..\url.c
# End Source File
# Begin Source File

SOURCE=..\value.c
# End Source File
# Begin Source File

SOURCE=..\webrom.c
# End Source File
# Begin Source File

SOURCE=..\webs.c
# End Source File
# Begin Source File

SOURCE=..\websda.c
# End Source File
# Begin Source File

SOURCE=..\websSSL.c
# End Source File
# Begin Source File

SOURCE=..\websuemf.c
# End Source File
# End Target
# End Project
