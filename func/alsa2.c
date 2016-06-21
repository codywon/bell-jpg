#if	1
/* * Standard includes */
#include <ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>

/* * Mandatory variables. */
#define BUF_SIZE 4096
int audio_fd;
unsigned char audio_buffer[BUF_SIZE];

char* DEVICE_NAME = "/dev/dsp";
int open_mode = O_RDONLY;	//O_RDONLY, O_WRONLY, O_RDWR

int startRecord()
{
    //Open device
    if ( ( audio_fd = open( DEVICE_NAME, open_mode, 0 ) ) == -1 )
    {
        /* Open of device failed */
        printf("Can not Open Audio Device %s\n", DEVICE_NAME);
        return -1;
    }

	int mask;

    if ( ioctl( audio_fd, SNDCTL_DSP_GETFMTS, &mask ) == -1 )
    {
        /* Handle fatal error ... */
		printf("SNDCTL_DSP_GETFMTS ERROR\n");
		return -1;
    }

	printf("Support Audio Format = 0x%08X", (unsigned int)mask);

    if ( mask & AFMT_MPEG )
    {
        /* 本设备支持MPEG采样格式 ... */
    }

	int format;
    format = AFMT_S16_LE;

    if ( ioctl( audio_fd, SNDCTL_DSP_SETFMT, &format ) == -1 )
    {
        /* fatal error */
        perror( "SNDCTL_DSP_SETFMT" );
        exit( 1 );
    }

    if ( format != AFMT_S16_LE )
    {
        /* 本设备不支持选择的采样格式. */
    }

    int len;

    if ( ( len = read( audio_fd, audio_buffer, count ) ) == -1 )
    {
        perror( "audio read" );
        exit( 1 );
    }
	/*
    count为录音数据的字节个数（建议为2的指数），
		但不能超过audio_buffer的 大小。
		从读字节的个数可以精确的测量时间，
		例如8kHZ 16 - bit stereo的速率为8000 * 2 * 2 = 32000bytes / second，
		这是知道何时停止录音的唯一方法。

            在设置采样格式之前，可以先测试设备能够支持那些采样格式，方法如下：


    设置采样格式


    设置通道数目
    */
    int channels = 2;

    /* 1=mono, 2=stereo */
    if ( ioctl( audio_fd, SNDCTL_DSP_CHANNELS, &channels ) == -1 )
    {
        /* Fatal error */
        perror( "SNDCTL_DSP_CHANNELS" );
        exit( 1 );
    }

    if ( channels != 2 )
    {
        /* 本设备不支持立体声模式 ... */
    }

    //设置采样速率
    int speed = 11025;

    if ( ioctl( audio_fd, SNDCTL_DSP_SPEED, &speed ) == -1 )
    {
        /* Fatal error */
        perror( "SNDCTL_DSP_SPEED" );
        exit( Error code );
    }

    if ( /* 返回的速率（即硬件支持的速率）与需要的速率差别很大... */ )
    {
        /* 本设备不支持需要的速率... */
    }

    //音频设备通过分频的方法产生需要的采样时钟，
    //因此不可能产生所有的频率。驱动程序会计算出最接近要求的频率来，
    //用户程序要检查返回的速率值，如果误差较小，可以忽略，但误差不能太大。
#endif
