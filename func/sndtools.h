#ifndef SNDTOOLS_H
#define SNDTOOLS_H
#include <linux/soundcard.h>
#define FMT8BITS AFMT_S8_LE
#define FMT16BITS AFMT_S16_LE
#define FMT8K 8000
#define FMT16K 16000
#define FMT22K 22000
#define FMT44K 44000
#define MONO 1
#define STERO 2

#ifndef VAR_STATIC
extern int devfd;
//extern int CapMask;
#endif //ifndef VAR_STATIC

//Open sound device, return 1 if open success
//else return 0
int OpenSnd();
//Close sound device
int CloseSnd();
//Set record or playback format, return 1 if success
//else return 0
int SetFormat( int bits, int hz );
//Set record or playback channel, return 1 if success
//else return 1
int SetChannel( int chn );
//Record
int Record( char* buf, int size );
//Playback
int Play( char* buf, int size );

#endif //ifndef SNDTOOLS_H

