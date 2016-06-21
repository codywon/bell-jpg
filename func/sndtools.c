 //**********************************************************************************************
//源文件 sndtools.c
//**********************************************************************************************
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#define	DEBUG_WARN
#define VAR_STATIC
#include "sndtools.h"
int devfd = 0;
/*
* Open Sound device
* Return 1 if success, else return 0.
*/
int OpenSnd( )///* add by new version */int nWhich )
{
    if ( devfd > 0 )
    {
        close( devfd );
    }

    //devfd = open( "/dev/pcmC0D0c", O_RDWR );
    devfd = open( "/dev/dsp", O_RDWR);	//O_RDONLY );

    if ( devfd < 0 )
    {
        return 0;
    }

    return 1;
}
/*
* Close Sound device
* return 1 if success, else return 0.
*/
int CloseSnd( )	///* add by new version */int nWhich )
{
    close( devfd );
    devfd = 0;
    return 1;
}

/*
无论是输入增益还是输出增益，其取值范围都是从0到100
获取麦克风的输入增益
*/
int GetMicGain()
{
    int vol;
    ioctl( devfd, MIXER_READ( SOUND_MIXER_MIC ), &vol );
    printf( "Mic gain is at %d %%\n", vol );

    int left, right;
    left = vol & 0xff;
    right = ( vol & 0xff00 ) >> 8;
    printf( "Left gain is %d %%, Right gain is %d %%\n", left, right );

    return vol;
}

int SetMicGain( int left, int right )
{
    int vol;
    vol = ( right << 8 ) + left;
    ioctl( devfd, MIXER_WRITE( SOUND_MIXER_MIC ), &vol );
	printf( "SET---Left gain is %d %%, Right gain is %d %%, vol=%d\n", left, right, vol );
    return vol;
}

/*
通过SOUND_MIXER_READ_RECMAS返回的位掩码，
则可以查询出能够被当作录音源的每一个通道
*/
int CheckRecSrc()
{
    int recmask;
    ioctl( devfd, SOUND_MIXER_READ_RECMASK, &recmask );

    if ( recmask & SOUND_MIXER_CD )
    {
        printf( "The CD input can be a recording source\n" );
    }

    if ( recmask & SOUND_MIXER_LINE )
    {
        printf( "The Line input can be a recording source\n" );
    }

    if ( recmask & SOUND_MIXER_MIC )
    {
        printf( "The MIC input can be a recording source\n" );
    }

    return recmask;
}

int CheckVolume()
{
	int vol = 0;
    ioctl( devfd, MIXER_READ(SOUND_MIXER_VOLUME), &vol );
	printf("SOUND_MIXER_VOLUME = %d\n", vol);

	int left, right;
	left = 100;
	right = 100;
	vol = ( right << 8 ) + left;
	ioctl( devfd, MIXER_WRITE(SOUND_MIXER_VOLUME), &vol );
	printf("Set SOUND_MIXER_VOLUME = %d\n", vol);

	vol = 0;
    ioctl( devfd, MIXER_READ(SOUND_MIXER_IGAIN), &vol );
	printf("SOUND_MIXER_IGAIN = %d\n", vol);

	vol = 0;
    ioctl( devfd, MIXER_READ(SOUND_MIXER_OGAIN), &vol );
	printf("SOUND_MIXER_OGAIN = %d\n", vol);

	vol = 0;
    ioctl( devfd, MIXER_READ(SOUND_MIXER_RECLEV), &vol );
	printf("SOUND_MIXER_RECLEV = %d\n", vol);
	//

	vol = ( right << 8 ) + left;
    ioctl( devfd, MIXER_WRITE(SOUND_MIXER_IGAIN), &vol );
	printf("SOUND_MIXER_IGAIN = %d\n", vol);

	vol = ( right << 8 ) + left;
    ioctl( devfd, MIXER_WRITE(SOUND_MIXER_OGAIN), &vol );
	printf("SOUND_MIXER_OGAIN = %d\n", vol);

	vol = ( right << 8 ) + left;
    ioctl( devfd, MIXER_WRITE(SOUND_MIXER_RECLEV), &vol );
	printf("SOUND_MIXER_RECLEV = %d\n", vol);

    return vol;

}

/*
缓冲区大小的设置通常应紧跟在设备文件打开之后，
这是因为对声卡的其它操作有可能会导致驱动程序无法再修改其缓冲区的大小。
下面的代码示范了怎样设置声卡驱动程序中的内核缓冲区的大小：
在设置缓冲区大小时，参数setting实际上由两部分组成，
其低16位标明缓冲区的尺寸，相应的计算公式为buffer_size = 2^ssss，
即若参数setting低16位的值为16，那么相应的缓冲区的大小会被设置为65536字节。
参数setting的高16位则用来标明分片（fragment）的最大序号，
它的取值范围从2一直到0x7FFF，其中0x7FFF表示没有任何限制。
*/

int SetFragment()
{
    //int setting = 0xnnnnssss;
    int setting = 0x7FFF0010;
    int result = ioctl( devfd, SNDCTL_DSP_SETFRAGMENT, &setting );

    if ( result == -1 )
    {
        perror( "ioctl buffer size" );
        return -1;
    }

    // 检查设置值的正确性
    return 0;
}

/*
* Set Record an Playback format
* return 1 if success, else return 0.
* bits -- FMT8BITS(8bits), FMT16BITS(16bits)
* hz -- FMT8K(8000HZ), FMT16K(16000HZ), FMT22K(22000HZ), FMT44K(44000HZ)
*/
int SetFormat( int bits, int hz )
{
    int nWhich = 0;
    int tmp = bits;

    if ( -1 == ioctl( devfd, SNDCTL_DSP_SETFMT, &tmp ) )
    {
#ifdef DEBUG_WARN
        printf( "Set fmt to s16_little faile:%d\n", nWhich );
#endif
        return 0;
    }

    tmp = hz;

    if ( -1 == ioctl( devfd, SNDCTL_DSP_SPEED, &tmp ) )
    {
#ifdef DEBUG_WARN
        printf( "Set speed to %d:%d\n", hz, nWhich );
#endif
        return 0;
    }

    return 1;
}
/*
* Set Sound Card Channel
* return 1 if success, else return 0.
* chn -- MONO, STERO
*/
int SetChannel( int chn )
{
    int nWhich = 0;
    int tmp = chn;

    //if ( -1 == ioctl( devfd, SNDCTL_DSP_CHANNELS, &tmp ) )
    if ( -1 == ioctl( devfd, SNDCTL_DSP_STEREO, &tmp ) )
    {
#ifdef DEBUG_WARN
        printf( "Set Audio Channel faile:%d\n", nWhich );
#endif
        return 0;
    }

    return 1;
}
/*
* Record
* return numbers of byte for read.
*/
int Record( char* buf, int size )
{
    return read( devfd, buf, size );
}
/*
* Playback
* return numbers of byte for write.
*/
int Play( char* buf, int size )
{
    return write( devfd, buf, size );
}

