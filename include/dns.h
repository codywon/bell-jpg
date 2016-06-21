#ifndef __DNS_INTERFACE_H__
#define __DNS_INTERFACE_H__
#include<stdio.h>
#include<stdlib.h>

extern void ReadDnsParam( void );
extern void WriteDnsParam( unsigned char* pdns );
extern void* RegisterProc( void* p );
extern int DnsCgiConfig( unsigned char* pcgi );

#endif
