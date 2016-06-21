/*
* copyright (C) 2013 XHX
* author: xiangqun
* date:2013.3.30
*/

#ifndef _XQ_TYPE_H_
#define _XQ_TYPE_H_

typedef int INT32;
typedef unsigned int UINT32;

typedef short INT16;
typedef unsigned short UINT16;

typedef char CHAR;
typedef signed char	 SCHAR;
typedef unsigned char UCHAR;

typedef long LONG;
typedef unsigned long ULONG;

#ifndef bool
#define bool	CHAR
#endif

#ifndef _ARC_COMPILER
#ifndef true
#define true	1
#endif

#ifndef false
#define false	0
#endif
#endif ////#ifndef _ARC_COMPILER

#endif

