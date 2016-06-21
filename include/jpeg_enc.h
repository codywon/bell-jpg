#ifndef _JPEG_ENC_
#define _JPEG_ENC_
#include "enc_type.h"
#include "type.h"
#include "JpgencLibApi.h"

extern int JpegEnc( void* pBufOut, int* bufSize, int addrY, int addrC, int width, int height, int pixelFmt );
#endif