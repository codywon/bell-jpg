#if	0
//**********************************************************************************************
//源文件 recorder.c
//**********************************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sndtools.h"
//#define WAVOUTDEV FMT8K
typedef unsigned int DWORD;
typedef unsigned short WORD;
struct RIFF_HEADER
{
    char szRiffID[4]; // 'R','I','F','F'
    DWORD dwRiffSize;
    char szRiffFormat[4]; // 'W','A','V','E'
};
struct WAVE_FORMAT
{
    WORD wFormatTag;
    WORD wChannels;
    DWORD dwSamplesPerSec;
    DWORD dwAvgBytesPerSec;
    WORD wBlockAlign;
    WORD wBitsPerSample;
};
struct FMT_BLOCK
{
    char szFmtID[4]; // 'f','m','t',' '
    DWORD dwFmtSize;
    struct WAVE_FORMAT wavFormat;
};
struct FACT_BLOCK
{
    char szFactID[4]; // 'f','a','c','t'
    DWORD dwFactSize;
};
struct DATA_BLOCK
{
    char szDataID[4]; // 'd','a','t','a'
    DWORD dwDataSize;
};
int startRecord()
{
    char* buf;
    int dwSize;
    int i;
    struct RIFF_HEADER riffheader;
    struct FMT_BLOCK fmtblock;
    struct DATA_BLOCK datablock;
    FILE* fp;
    printf( "WORD %d \n", sizeof( WORD ) );
    printf( "DWORD %d\n", sizeof( DWORD ) );
    riffheader.szRiffID[0] = 'R';
    riffheader.szRiffID[1] = 'I';
    riffheader.szRiffID[2] = 'F';
    riffheader.szRiffID[3] = 'F';
    riffheader.dwRiffSize = 1024 * 50 + 8 + 16 + 8 + 4;
    riffheader.szRiffFormat[0] = 'W';
    riffheader.szRiffFormat[1] = 'A';
    riffheader.szRiffFormat[2] = 'V';
    riffheader.szRiffFormat[3] = 'E';
    fmtblock.szFmtID[0] = 'f';
    fmtblock.szFmtID[1] = 'm';
    fmtblock.szFmtID[2] = 't';
    fmtblock.szFmtID[3] = ' ';
    fmtblock.dwFmtSize = 16;
    fmtblock.wavFormat.wFormatTag = 0x0001;
    fmtblock.wavFormat.wChannels = 1;
    fmtblock.wavFormat.dwSamplesPerSec = 8000;
    fmtblock.wavFormat.dwAvgBytesPerSec = 8000 * 2; ////////////////////
    fmtblock.wavFormat.wBlockAlign = 2;
    fmtblock.wavFormat.wBitsPerSample = 16;
    datablock.szDataID[0] = 'd';
    datablock.szDataID[1] = 'a';
    datablock.szDataID[2] = 't';
    datablock.szDataID[3] = 'a';
    datablock.dwDataSize = 1024 * 50;

    if ( ( fp = fopen( "/tmp/test.wav", "wb" ) ) == NULL )
    {
        printf( "Cannot open test.wav" );
        return -1;
    }

    fwrite( &riffheader, sizeof( riffheader ), 1, fp );
    fwrite( &fmtblock, sizeof( fmtblock ), 1, fp );
    fwrite( &datablock, sizeof( datablock ), 1, fp );

    if ( !OpenSnd() )
    {
        printf( "Open sound device error!\\n" );
        //exit( -1 );
        return -1;
    }

	/*
	CheckRecSrc();
	GetMicGain();
	SetMicGain(100,100);
	CheckVolume();
	*/

    SetFormat( FMT16BITS, FMT8K );
    SetChannel( MONO );
    buf = ( char* )malloc( 1024 );

    if ( buf == NULL )
    {
		printf("malloc ERROR\n");
        //exit( -1 );
        return -1;
    }

    for ( i = 0; i < 50; i++ )
    {
        printf( "%d \n", i );
        dwSize = Record( buf, 1024 );
        fwrite( buf, dwSize, 1, fp );
		//dwSize = Play(buf, dwSize);
    }

    fclose( fp );

	printf("Record Over\n");
    //exit( 1 );
    return 0;
}
#else
/*
 * sound.c
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/soundcard.h>
#define LENGTH 3  /* 存储秒数 */
#define RATE 8000   /* 采样频率 */
#define SIZE 8      /* 量化位数 */
#define CHANNELS 1  /* 声道数目 */
/* 用于保存数字音频数据的内存缓冲区 */
unsigned char buf[LENGTH* RATE* SIZE* CHANNELS / 8];
//int main()
int startRecord()
{
    int fd;	/* 声音设备的文件描述符 */
    int arg;	/* 用于ioctl调用的参数 */
    int status;   /* 系统调用的返回值 */
    /* 打开声音设备 */
    fd = open( "/dev/dsp", O_RDWR );

    if ( fd < 0 )
    {
        perror( "open of /dev/dsp failed" );
        exit( 1 );
    }

    /* 设置采样时的量化位数 */
    arg = SIZE;
    status = ioctl( fd, SOUND_PCM_WRITE_BITS, &arg );

    if ( status == -1 )
    {
        perror( "SOUND_PCM_WRITE_BITS ioctl failed" );
    }

    if ( arg != SIZE )
    {
        perror( "unable to set sample size" );
    }

    /* 设置采样时的声道数目 */
    arg = CHANNELS;
    status = ioctl( fd, SOUND_PCM_WRITE_CHANNELS, &arg );

    if ( status == -1 )
    {
        perror( "SOUND_PCM_WRITE_CHANNELS ioctl failed" );
    }

    if ( arg != CHANNELS )
    {
        perror( "unable to set number of channels" );
    }

    /* 设置采样时的采样频率 */
    arg = RATE;
    status = ioctl( fd, SOUND_PCM_WRITE_RATE, &arg );

    if ( status == -1 )
    {
        perror( "SOUND_PCM_WRITE_WRITE ioctl failed" );
    }

    /* 循环，直到按下Control-C */
    while ( 1 )
    {
        printf( "Say something:\n" );
        status = read( fd, buf, sizeof( buf ) ); /* 录音 */

        if ( status != sizeof( buf ) )
        {
            perror( "read wrong number of bytes" );
        }
		else
		{
			printf("OK, status = %d\n", status);
		}

		#if	0
        printf( "You said:\n" );
        status = write( fd, buf, sizeof( buf ) ); /* 回放 */

        if ( status != sizeof( buf ) )
        {
            perror( "wrote wrong number of bytes" );
        }

        /* 在继续录音前等待回放结束 */
        status = ioctl( fd, SOUND_PCM_SYNC, 0 );

        if ( status == -1 )
        {
            perror( "SOUND_PCM_SYNC ioctl failed" );
        }
		#endif

		if ( 1 )
		{
			FILE *f = fopen("/tmp/hello.pcm", "wb");
			fwrite(buf, status, 1, f );
			fclose(f);
			perror("Write OK");
			break;
		}
    }
}
#endif

