#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <time.h>
#include <memory.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include "param.h"
#include "debug.h"

#define RALINK_GPIO_SET_DIR 	1
#define RALINK_GPIO_READ	2
#define RALINK_GPIO_WRITE 	3

#define ERROR_CODE_TRUE 0
#define ERROR_CODE_WRITE_DATA -1
#define ERROR_CODE_WRITE_ADDR -2
#define ERROR_CODE_READ_ADDR -3

static int gpiovalue = 0x06;
static int i2cfd = -1;
unsigned char encryptflag = 0x00;

#define	AIC_UNKNOW	0
#define	AIC_8388	1
#define	AIC_8388S	2
#define	AIC_8988	3

int AIC_nIndex = AIC_UNKNOW;


#define FILE_AUDIO_PARAM    "/system/www/audioparam.bin"

extern void ClearOnStartupAudioNoise();

void WS222DelayMs( unsigned delay )
{
    usleep( 1000 * delay );
}
void WS222_SDA( char value )
{
    int iRet;
    int i;   

    gpiovalue &= 0x00000004;

    #ifdef SUPPORT_FM34
    gpiovalue |= (1<<25);//复位脚

	/* add begin by yiqing, 2015-09-11 关闭功放*/
	gpiovalue |= BELL_AUDIO;
    #endif


#ifdef SUPPORT_FM34
		if ( bparam.stBell.lock_type == 0)//no
		{	
			gpiovalue |= BELL_OPENDOOR;
		}
#else
		if ( bparam.stBell.lock_type == 1 )//nc
		{
        	gpiovalue |= BELL_OPENDOOR;
		}
#endif



    if ( value )
    {
        gpiovalue |= 0x02;
    }

    
    iRet = ioctl( i2cfd , RALINK_GPIO_WRITE, gpiovalue );

    for ( i = 0; i < 1000; i++ );
}
void WS222_SCL( char value )
{
    int iRet;
    int i;

    gpiovalue &= 0x00000002;
    
    #ifdef SUPPORT_FM34
    gpiovalue |= (1<<25);//复位脚

	/* add begin by yiqing, 2015-09-11 关闭功放*/
	gpiovalue |= BELL_AUDIO;
    #endif
    

#ifdef SUPPORT_FM34
	if ( bparam.stBell.lock_type == 0)//no
	{
		gpiovalue |= BELL_OPENDOOR;
	}
#else
	if ( bparam.stBell.lock_type == 1 )//nc
	{
		gpiovalue |= BELL_OPENDOOR;
	}
#endif


    if ( value )
    {
        gpiovalue |= 0x04;
    }


    
    iRet = ioctl( i2cfd , RALINK_GPIO_WRITE, gpiovalue );

    for ( i = 0; i < 1000; i++ );
}

void gpioopen( void )
{
    i2cfd  = BaGpioOpen();
    //i2cfd = open( "/dev/gpio", O_RDWR );
    if ( i2cfd  < 0 )
    {
        perror( "/dev/gpio" );
        return;
    }
}
void gpioclose( void )
{
	sleep(3);
    //close( i2cfd );
}

void gpioinit( void )
{
    int iRet;
    #if defined (SUPPORT_FM34)


		#if defined (SUPPORT_IRCUT)
			/* add begin by yiqing, 2015-10-20设置第17脚MOTO_UP为输入脚PIR检测用*/
			iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x0fc07806 );
		#else
			/* modify begin by yiqing, 2015-07-17, 原因: 设置25脚为输出脚，控制音频芯片复位*/
			//iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x0dc27804 );
			iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x0fc27806 );
		#endif
    
    #elif defined(JINQIANXIANG)
    iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x0fc04006 );
    #else
    iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x0fc07806 );
	//iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x07c07806 );
    #endif
}

void gpioin( void )
{
    int iRet;

    #if defined (SUPPORT_FM34)

		#if defined (SUPPORT_IRCUT)
		/* add begin by yiqing, 2015-10-20设置第17脚MOTO_UP为输入脚PIR检测用*/
		iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x0dc07804 );
		#else
		iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x0dc27804 );
		#endif
    
    
    #elif defined(JINQIANXIANG)
    iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x0fc04004 );
    #else
    iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x0fc07804 );
    //iRet = ioctl( i2cfd , RALINK_GPIO_SET_DIR, 0x07c07806 );
    #endif
}

int gpioread( void )
{
    int iRet;
    int value;
    iRet = ioctl( i2cfd , RALINK_GPIO_READ, &value );
    //printf("read gpio status=0x%08x\n",value);
    value &= 0x02;

    if ( value )
    {
        return 1;
    }

    return 0;
}

void i2cdelay( int delay )
{
    int i;

    for ( i = 0; i < 1000; i++ );
}

#define SDA_HIGH	WS222_SDA(1)
#define SDA_LOW		WS222_SDA(0)
#define SCL_HIGH	WS222_SCL(1)
#define SCL_LOW		WS222_SCL(0)
#define SDA_IN		gpioin()
#define SDA_OUT		gpioinit()
#define SCL_OUT		gpioinit()
#define SDA_DETECT	gpioread();

#define I2C_DELAY				i2cdelay(1000);
#define I2C_DELAY_LONG			i2cdelay(100000);

void i2c_init( void )
{
    WS222DelayMs( 100 );
}
// I2C START
#define PRINTFMODE		1
void WS222_i2c_start( void )
{
    SDA_OUT;
    SDA_HIGH;
    SCL_HIGH;	//just in case default output value is 0, to avoid unexcept falling edge while set I/O as output
    SDA_HIGH;
    I2C_DELAY;
    SCL_HIGH;
    I2C_DELAY;
    I2C_DELAY;
    SDA_LOW;
    I2C_DELAY;
    I2C_DELAY;
    SCL_LOW;
    I2C_DELAY;
}

void WS222_i2c_stop( void )
{
    SDA_OUT;
    SDA_LOW;
    I2C_DELAY;
    I2C_DELAY;
    SCL_HIGH;
    I2C_DELAY;
    I2C_DELAY;
    SDA_HIGH;
    I2C_DELAY;
    I2C_DELAY;
    I2C_DELAY;
    I2C_DELAY;
}

unsigned char WS222_i2c_write_byte(  unsigned char data )
{
    unsigned char i, ack;
    int ret;
    SDA_OUT;
    I2C_DELAY;

    for ( i = 0; i < 8; i++ )
    {
        if ( ( data << i ) & 0x80 )
        {
            SDA_HIGH;
            I2C_DELAY;
        }

        else
        {
            SDA_LOW;
            I2C_DELAY;
        }

        SCL_HIGH;
        I2C_DELAY;
        SCL_LOW;
        I2C_DELAY;
    }

    SDA_IN;
    I2C_DELAY;
//	SDA_HIGH;
//	I2C_DELAY;
    SCL_HIGH;
    I2C_DELAY;
    ack = gpioread(); /// ack
    //printf(" _i2c_write_byte, ack = %d \r\n", ack);
    I2C_DELAY;
    SCL_LOW;
    SDA_OUT;
    return ack;
}

unsigned char WS222_i2c_read_byte_ack( void )
{
    unsigned short i, data;
    unsigned char v_return;
    data = 0;
    SDA_IN;
    I2C_DELAY;

    for ( i = 0; i < 8; i++ )
    {
        data <<= 1;
        I2C_DELAY;
        SCL_HIGH;
        I2C_DELAY;
        data |= gpioread();
        SCL_LOW;
        I2C_DELAY;
    }

    SDA_OUT;
    SDA_LOW;
    I2C_DELAY;
    SCL_HIGH;
    v_return = ( unsigned char )data & 0xFF;
    SCL_LOW;
    I2C_DELAY;
    return v_return;
}

unsigned char WS222_i2c_read_byte_noack( void )
{
    unsigned short i, data;
    unsigned char v_return;
    data = 0;
    SDA_IN;
    I2C_DELAY;

    for ( i = 0; i < 8; i++ )
    {
        data <<= 1;
        I2C_DELAY;
        SCL_HIGH;
        I2C_DELAY;
        data |= gpioread();
        SCL_LOW;
        I2C_DELAY;
    }

    SDA_OUT;
    SDA_HIGH;
    I2C_DELAY;
    SCL_HIGH;
    v_return = ( unsigned char )data & 0xFF;
    SCL_LOW;
    I2C_DELAY;
    return v_return;
}

unsigned char WS222_i2c_write( unsigned char device_addr, unsigned char sub_addr, unsigned char* buff, int ByteNo )
{
    unsigned char i;
    WS222_i2c_start();
    I2C_DELAY_LONG;

    if ( WS222_i2c_write_byte( device_addr & 0xFF ) ) //
    {
        WS222_i2c_stop();
#if PRINTFMODE
        //printf( "\n\rWRITE I2C : Write Error - Device Addr" );
        Textout( "\n\rWRITE I2C : Write Error - Device Addr, addr = %02X, sub = %02X", device_addr, sub_addr);
#endif
        return ERROR_CODE_WRITE_ADDR;
    }

    if ( WS222_i2c_write_byte( sub_addr ) )
    {
        WS222_i2c_stop();
#if PRINTFMODE
        printf( "\n\rWRITE I2C : Write Error - Sub Addr" );
#endif
        return ERROR_CODE_WRITE_ADDR;
    }

    for ( i = 0; i < ByteNo; i++ )
    {
        if ( WS222_i2c_write_byte( buff[i] ) )
        {
            WS222_i2c_stop();
#if PRINTFMODE
            printf( "\n\rWRITE I2C : Write Error - TX Data" );
#endif
            return ERROR_CODE_WRITE_DATA;
        }
    }

    I2C_DELAY;
    WS222_i2c_stop();
    I2C_DELAY_LONG;
    return ERROR_CODE_TRUE;
}

unsigned char WS222_i2c_writeEx( unsigned char* buff, int ByteNo )
{
    int i=0;
    
    WS222_i2c_start();
    I2C_DELAY_LONG;

    for ( i = 0; i < ByteNo; i++ )
    {
        if ( WS222_i2c_write_byte( buff[i] ) )
        {
            WS222_i2c_stop();
            return ERROR_CODE_WRITE_DATA;
        }
    }

    I2C_DELAY;
    WS222_i2c_stop();
    I2C_DELAY_LONG;
    return ERROR_CODE_TRUE;
}

unsigned char WS222_i2c_readEx( unsigned char device_addr, unsigned char* buff, int ByteNo )
{
    int i=0;

    WS222_i2c_start();
    I2C_DELAY_LONG;

    if ( WS222_i2c_write_byte( device_addr ) )
    {
        WS222_i2c_stop();
        return ERROR_CODE_READ_ADDR;
    }

    for ( i = 0; i < ByteNo; i++ )
    {
        if ( i < ByteNo - 1 )
        {
            buff[i] = WS222_i2c_read_byte_ack();
        }

        else
        {
            buff[i] = WS222_i2c_read_byte_noack();
        }
    }

    I2C_DELAY;
    I2C_DELAY_LONG;
    WS222_i2c_stop();
    I2C_DELAY_LONG;
    return ERROR_CODE_TRUE;
}


unsigned char WS222_i2c_read( unsigned char device_addr, unsigned char sub_addr, unsigned char* buff, int ByteNo )
{
    unsigned char i;
#if PRINTFMODE
    printf( "\r\n_i2c_read" );
#endif
    WS222_i2c_start();
    I2C_DELAY_LONG;

    if ( WS222_i2c_write_byte( 0xc0 ) )
    {
        WS222_i2c_stop();
#if PRINTFMODE
        printf( "\n\r Read I2C Write Error - device Addr\r\n" );
#endif
        return ERROR_CODE_READ_ADDR;
    }

    if ( WS222_i2c_write_byte( 0xa0 ) )
    {
        WS222_i2c_stop();
#if PRINTFMODE
        printf( "\n\rRead I2C Write Error - sub Addr \r\n" );
#endif
        return ERROR_CODE_READ_ADDR;
    }

    WS222_i2c_start();

    if ( WS222_i2c_write_byte( 0xc1 ) )
    {
        WS222_i2c_stop();
#if PRINTFMODE
        printf( "\n\rRead I2C Write Error - sub Addr \r\n" );
#endif
        return ERROR_CODE_READ_ADDR;
    }

    for ( i = 0; i < ByteNo; i++ )
    {
        if ( i < ByteNo - 1 )
        {
            buff[i] = WS222_i2c_read_byte_ack();
        }

        else
        {
            buff[i] = WS222_i2c_read_byte_noack();
        }
    }

    I2C_DELAY;
    I2C_DELAY_LONG;
    WS222_i2c_stop();
    I2C_DELAY_LONG;
    return ERROR_CODE_TRUE;
}


unsigned char my_WS222_i2c_read( unsigned char device_addr, unsigned char sub_addr, unsigned char* buff, int ByteNo )
{
    unsigned char i;
    WS222_i2c_start();
    I2C_DELAY_LONG;

    if ( WS222_i2c_write_byte( device_addr ) )
    {
		printf("1");
        WS222_i2c_stop();
        return ERROR_CODE_READ_ADDR;
    }

    if ( WS222_i2c_write_byte( sub_addr ) )
    {
		printf("2");
        WS222_i2c_stop();
        return ERROR_CODE_READ_ADDR;
    }

    WS222_i2c_start();

    if ( WS222_i2c_write_byte( device_addr|0x01 ) )
    {
		printf("3");
        WS222_i2c_stop();
        return ERROR_CODE_READ_ADDR;
    }

    for ( i = 0; i < ByteNo; i++ )
    {
        if ( i < ByteNo - 1 )
        {
            buff[i] = WS222_i2c_read_byte_ack();
        }

        else
        {
            buff[i] = WS222_i2c_read_byte_noack();
        }
    }

    I2C_DELAY;
    I2C_DELAY_LONG;
    WS222_i2c_stop();
    I2C_DELAY_LONG;
    return ERROR_CODE_TRUE;
}

/* BEGIN: Added by wupm, 2013/4/3 */
void CheckEncrypt()
{
	unsigned char error_code, i;
	int			status = 0;
	int			iRet = 0;

	unsigned char value = 5;
	unsigned char tx_data[8];
	unsigned char ex_data[8];
    unsigned char rx_data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

#ifdef	REMOVE_ENCRYPT
    encryptflag = 0;
	return;
#endif

    for ( i = 0; i < 8; i++ )
    {
        value = rand() & 0xff;
        tx_data[i] =  value;
    }

    iRet = WS222_i2c_write( 0XC0, 0xa0, tx_data , 8 );
    status = 0;
    if ( iRet  == 0x00 )
    {
        WS222DelayMs( 500 );
        WS222_i2c_read( 0XC1, 0xa0, rx_data, 10 );
        EDesEn_Crypt( rx_data, ex_data );

        for ( i = 0; i < 8; i++ )
        {
            if ( tx_data[i] != ex_data[i] )
            {
                status = 1;
                break;
            }
        }

        if(i != 8)
        {
            Textout("encrypt fail");
        }
        else
        {
            Textout("encrypt success");
        }
    }

    else
    {
        status = 1;
    }

    if ( status == 0x00 )
    {
        encryptflag = 0;
        Textout( "===========encryt is ok============\n" );
    }

    else
    {
        Textout( "==========encrypt is error===========\n" );
        encryptflag = 1;
		//ControlBellLED(CB_FLASH_FAST);
    }
}

void TestI2cModule()
{
	int			iRet = 0;

    unsigned char i2cData[32] = {0xA0,0x31,0x32,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
                                 0x66,0xA3,0x7E,0x3F,0xE1,0x73,0xC2,0x41,0x28,0x86,0x42,0xC1,0x00,0x00,0xCC,0xCC};

    {
        int i=0;
        printf("Write Data...\n");
        for(i=0; i<32; i++)
            printf("i2cData[%d]=0x%02x\n", i, i2cData[i]);
    }

    iRet = WS222_i2c_writeEx( i2cData, 32);
    if ( iRet  == 0x00 )
    {
        WS222DelayMs( 500 );
        memset(i2cData, 0, 32);
        WS222_i2c_readEx( 0xA1, i2cData, 31 );
    }

    {
        int i=0;
        printf("Read Data...\n");
        for(i=0; i<31; i++)
            printf("i2cData[%d]=0x%02x\n", i, i2cData[i]);
    }
}


unsigned char WS222_i2c_write_9byte( unsigned short data )
{
    unsigned char i, ack;
    int ret;

    SDA_OUT;
    I2C_DELAY;

    for ( i = 0; i < 9; i++ )
    {
        if ( ( data << i ) & 0x100 )
        {
            SDA_HIGH;
            I2C_DELAY;
        }

        else
        {
            SDA_LOW;
            I2C_DELAY;
        }

        SCL_HIGH;
        I2C_DELAY;
        SCL_LOW;
        I2C_DELAY;
    }

    SDA_IN;
    I2C_DELAY;

//	SDA_HIGH;
//	I2C_DELAY;
    SCL_HIGH;
    I2C_DELAY;
    ack = gpioread(); /// ack
    //printf( " _i2c_write_byte, ack = %d \r\n", ack );
    I2C_DELAY;
    SCL_LOW;
    SDA_OUT;
    return ack;
}

unsigned char i2c_WM8751_write( unsigned short sub_addr, unsigned short value )
{
    int i=0;
    unsigned char device_addr = 0x36;
    unsigned short sub_addr1 = (sub_addr<<1) | (value >> 8);
    //Textout( "Device Addr, addr = 0x%02X, sub = 0x%02X", device_addr, sub_addr);

    WS222_i2c_start();
    I2C_DELAY_LONG;


    if ( WS222_i2c_write_byte( device_addr ) )
    {
        WS222_i2c_stop();
#if PRINTFMODE
        printf( "WRITE I2C : Write Error - Device Addr=0x%02x\n", device_addr );
#endif
        return ERROR_CODE_WRITE_ADDR;
    }

    if ( WS222_i2c_write_byte( sub_addr1 ) )
    {
        WS222_i2c_stop();
#if PRINTFMODE
        printf( "WRITE I2C : Write Error - Sub Addr=%02x\n", sub_addr );
#endif
        return ERROR_CODE_WRITE_ADDR;
    }

    if ( WS222_i2c_write_byte( value & 0xFF) )
    {
        WS222_i2c_stop();
#if PRINTFMODE
        printf( "WRITE I2C : Write Error - TX Data\n" );
#endif
        return ERROR_CODE_WRITE_DATA;
    }

    I2C_DELAY;
    WS222_i2c_stop();
    I2C_DELAY_LONG;
    return ERROR_CODE_TRUE;
}

int CheckPowerEnc( void )
{
    FILE* fp = NULL;
    int  status = 0x00;
    fp = fopen( "/param/enc.bin", "rb" );

    if ( fp == NULL )
    {
        return -1;
    }

    fread( &status, 1, sizeof( int ), fp );
    fclose( fp );
    printf( "status %d\n", status );

    if ( status == 0x01 )
    {
        return 0;
    }

    return -1;
}

int SavePowerEnc( int flag )
{
    FILE* fp = NULL;
    int  status = flag;
    fp = fopen( "/param/enc.bin", "wb" );

    if ( fp == NULL )
    {
        return -1;
    }

    fwrite( &status, 1, sizeof( int ), fp );
    fclose( fp );
    return 0;
}

void Es8388Close( void )
{
    int iRet;
    unsigned char value = 0xf3;
    gpioopen();
    gpioinit();
    WS222DelayMs( 10 );
    i2c_init();
    WS222DelayMs( 200 );
    ClearOnStartupAudioNoise();
    iRet = WS222_i2c_write( 0x22, 0x02, &value, 1 );
    printf( "es8388 close\n" );
}

void Wm8988Close( void )
{
    int iRet;
    unsigned char value = 0xf3;
    gpioopen();
    gpioinit();
    WS222DelayMs( 10 );
    i2c_init();
    WS222DelayMs( 200 );

    i2s_codec_disable();
    printf( "wm8988 close\n" );
}


int GetI2cFd( void )
{
    return i2cfd;
}

#define	MAX_CHIP_COUNT			2
#define	MAX_SUBADDRESS_COUNT	40
typedef struct
{
    char szName[16];
    char nAddress[16];
    char nMicphoneValue[MAX_SUBADDRESS_COUNT * 2];
    char nLineInValue[MAX_SUBADDRESS_COUNT * 2];
} AUDIO_FILE_ITEM;
typedef struct
{
    AUDIO_FILE_ITEM oItem[MAX_CHIP_COUNT];
} AUDIO_FILE;

static char mStrAddress[MAX_CHIP_COUNT][16];

int CheckChip8388S()
{
    int iRet = 0;

	unsigned char rdat = 0x55;

	gpioopen();
    gpioinit();
    gpiovalue = 0x00000006;
    iRet = ioctl( i2cfd , RALINK_GPIO_WRITE, gpiovalue );
    WS222DelayMs( 10 );
    i2c_init();
    WS222DelayMs( 200 );
    WS222DelayMs( 20 );

	iRet = my_WS222_i2c_read( 0x22, 0x28, &rdat, 1 );
	Textout( "iRet=%d, rdat=0x%02x", iRet, rdat );

/* BEGIN: Added by yiqing, 2015/4/23 */
#ifdef ES8388S
	Textout( "=============Audio Chip = ES8388S!!!!!!" );
	gpioclose();
	return AIC_8388S;
#endif
/* END:   Added by yiqing, 2015/4/23 */

	if ( rdat == 0x28 )
    {
        Textout( "=============Audio Chip = ES8388!!!!!!" );
    	gpioclose();
		return AIC_8388;
    }

    else if(rdat == 0x00)
    {
        Textout( "=============Audio Chip = ES8388S!!!!!!" );
		gpioclose();
		return AIC_8388S;
    }
    
	return AIC_UNKNOW;

}
void WriteAudioParamFile()
{
    FILE* fp = NULL;
    AUDIO_FILE oFile;
    int i, k;
    unsigned char es8388table[35 * 2] =
    {
        0x08, 0x85,
        0x02, 0xf3,
        0x2b, 0xc0,
        0x00, 0x36,     //0x36 zqh
        0x01, 0x72,
        0x03, 0x00,
        0x04, 0x30,     //0x3c
        0x05, 0x00,
        0x06, 0x00,
        0x07, 0x7c,
        0x09, 0x88,
        0x0a, 0xf0,     //0x80 0xf0
        0x0b, 0x02,
        0x0c, 0x0c,     //0x0c
        0x0d, 0x0a,
        0x10, 0x00,
        0x11, 0x00,
        0x12, 0xd2,     //0x02
        0x13, 0xC0,
        0x14, 0x05,
        0x15, 0x06,
        0x16, 0xb3,     //0xb3
        0x17, 0x40,     //0x00
        0x18, 0x0a,
        0x19, 0xe2,
        0x1a, 0x00,
        0x1b, 0x00,
        0x26, 0x00,
        0x27, 0xb8,
        0x28, 0x38,
        0x29, 0x38,
        0x2a, 0xb8,
        0x2e, 0x1e,
        0x2f, 0x1e,
        0x02, 0x00
    };
    unsigned char Wm8988table[33 * 2] =
    {
        0x01, 0x17,
        0x03, 0x17,
        0x05, 0x79,
        0x07, 0x79,
        0x0a, 0x00,
        0x0e, 0x02,				//设置为主模式时请改为0x0e,0x42
        0x10, 0x0d,
        0x15, 0xff,
        0x17, 0xff,
        0x18, 0x0f,
        0x1a, 0x0f,
        0x20, 0x00,
        0x22, 0x7b,
        0x24, 0x00,
        0x26, 0x32,
        0x28, 0x00,
        0x2b, 0xc3,
        0x2d, 0xc3,
        0x2e, 0xc0,
        0x30, 0x04,
        0x36, 0x00,
        0x3e, 0x00,
        0x40, 0x00,
        0x42, 0x00,
        0x44, 0x52,
        0x46, 0x50,
        0x48, 0x52,
        0x4a, 0x50,
        0x51, 0x79,
        0x53, 0x79,
        0x86, 0x08,
        0x33, 0x7c,
        0x34, 0x00,
    };

    memset( &oFile, 0, sizeof( AUDIO_FILE ) );

    strcpy( oFile.oItem[0].szName, "ES8388" );
    oFile.oItem[0].nAddress[0] = 0x22;

    for ( i = 0; i < 70; i++ )
    {
        oFile.oItem[0].nMicphoneValue[i] = es8388table[i];
        oFile.oItem[0].nLineInValue[i] = es8388table[i];
    }

    for ( ; i < MAX_SUBADDRESS_COUNT * 2; i++ )
    {
        oFile.oItem[0].nMicphoneValue[i] = 0xFF;
        oFile.oItem[0].nLineInValue[i] = 0xFF;
    }

    strcpy( oFile.oItem[1].szName, "WM8988" );
    oFile.oItem[1].nAddress[0] = 0x36;

    for ( i = 0; i < 66; i++ )
    {
        oFile.oItem[1].nMicphoneValue[i] = Wm8988table[i];
        oFile.oItem[1].nLineInValue[i] = Wm8988table[i];
    }

    for ( ; i < MAX_SUBADDRESS_COUNT * 2; i++ )
    {
        oFile.oItem[1].nMicphoneValue[i] = 0xFF;
        oFile.oItem[1].nLineInValue[i] = 0xFF;
    }

    fp = fopen( FILE_AUDIO_PARAM, "wb" );
    fwrite( &oFile, 1, sizeof( AUDIO_FILE ), fp );
    fclose( fp );
}

void AudioPowerOpen( int value )
{
	gpiovalue &= ~_ENCPOWER;
	if( value )
	{
		gpiovalue |= _ENCPOWER;
	}

    #ifdef BELL_AUDIO_V2
    gpiovalue &= ~BELL_AUDIO_V2;
    #endif

    #ifdef BELL_AUDIO
    gpiovalue |= BELL_AUDIO;
    #endif

    #ifdef BELL_AUDIO_2ND
    gpiovalue &= ~BELL_AUDIO_2ND;
    #endif

	ioctl( i2cfd , RALINK_GPIO_WRITE, gpiovalue );
}


void CheckAudioChip()
{
    FILE* fp = NULL;
    int nIndex = 0;
    AUDIO_FILE oFile;

	/* BEGIN: Added by wupm(2073111@qq.com), 2014/6/18 */
    AIC_nIndex = CheckChip8388S();
    if(AIC_nIndex == AIC_UNKNOW)
    {
        AIC_nIndex = AIC_8988;
    }
	
    if (1)
    {
        unsigned char error_code, i;
        unsigned char tx_data[8];
        unsigned char ex_data[8];
        unsigned char rx_data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        unsigned char value = 5;
        int			iRet;
        int			status;

        srand( ( int )time( 0 ) );

        gpioopen();
        gpioinit();
        gpiovalue = 0x00000006;
        iRet = ioctl( i2cfd , RALINK_GPIO_WRITE, gpiovalue );
        WS222DelayMs( 10 );
        i2c_init();
        WS222DelayMs( 200 );

        /* BEGIN: Added by wupm, 2013/4/3 */
        CheckEncrypt();
        //TestI2cModule();

        AudioPowerOpen(0);
        

        WS222DelayMs( 20 );

        if ( 1 )
        {
			if ( AIC_nIndex == AIC_8388S )
			{
				const unsigned char es8388tablesss[] =
                {
                    53,	0x35, 0xa0,		//

                    56,	0x38, 0x02,     //

                    2,	0x02, 0xf3,     //CHIP POWER MANAGEMENT, DEFAULT 1100 0011

                    0,	0x00, 0x06,		//CHIP CONTROL 1, DEFAULT 0000 0110

                    1,	0x01, 0x72,		//CHIP CONTROL 2, DEFAULT 0001 1100

                    8,	0x08, 0xD5,		//MASTER MODE CONTROL, DEFAULT 1000 0000
                    					//D5 = 1101 0101
                    					//1 – master serial port mode
                    					//1 – MCLK divide by 2
                    					//0 – normal (default)
                    					//10101 – MCLK/10
                    					//
                    					//Change to 0x95, Can 16K !!!

                    45,	0x2d, 0x10,     //DAC CONTROL 23, DEFAULT 0000 0000
                    					//4		1 – 40k VREF to analog output resistance
                    					//		0 – 1.5k VREF to analog output resistance (default)

                    43,	0x2b, 0xc0,     //DAC CONTROL 21, DEFAULT 0011 1000
                    					//7		1 – DACLRC and ADCLRC same
                    					//6		1 – use ADC LRCK
                    					//5		0 – disable offset
                    					//4		1 – disable MCLK input from PAD
                    					//3		1 – power down ADC anaclk
                    					//2		1 – power down DAC anaclk

                    6,	0x06, 0xff,     //CHIP LOW POWER 2, DEFAULT 0000 0000

                    5,	0x05, 0x00,     //CHIP LOW POWER 1, DEFAULT 0000 0000

                    7,	0x07, 0x7a,     //ANALOG VOLTAGE MANAGEMENT, DEFAULT 0111 1100

                    //9,	0x09, 0x20,
                    //9,	0x09, 0x30,
                    9,	0x09, 0x00,
                    //9,	0x09, 0x50,     //ADC CONTROL 1, DEFAULT 0000 0000
                    					//7:4	Left channel PGA gain	0000 – 0 dB (default)
                    					//		0101 – +15 dB
                    					//		0000 – 0 dB (default)

                    10,	0x0a, 0xf1,     //ADC CONTROL 2, DEFAULT 0000 0000
                    					//7:6	Channel input select	11 – L-R differential input, L/R selection refer to DS (reg0xB[7])
                    					//0		0 – cap mode disabled (default)

                    11,	0x0b, 0x00,     //ADC CONTROL 3, DEFAULT 0000 0110

                    12,	0x0c, 0x0c,     //ADC CONTROL 4, DEFAULT 0000 0000
                    					//5		I2S or left justified mode
                    					//4-2	011 – 16-bit serial audio data word length
                    					//1-0	00 – I2S serial audio data format(default)

                    //13,	0x0d, 0x0A,
                    //13,	0x0d, 0x07,
                    13,	0x0d, 0x17,     //ADC CONTROL 5, DEFAULT 0000 0110
                    					//6		ADC ratio selection for slave mode
                    					//5		0 – single speed mode (default)
                    					//4-0	Master mode ADC MCLK to sampling frequency ratio
                    					//		10111 – 750
                    					//		00110 – 768
                    					//		00111 – 1024

                    23,	0x17, 0x18,     //DAC CONTROL 1, DEFAULT 0000 0000
                    					//7		1 – left and right channel data swap
                    					//6		I2S or left justified mode
                    					//5-3	011 – 16-bit serial audio data word length	110 - UNKNOW!!!
                    					//2-1	00 – I2S serial audio data format

                    //24,	0x18, 0x0A,
                    //24,	0x18, 0x07,
                    24,	0x18, 0x17,     //DAC CONTROL 2, DEFAULT 0000 0110
                    					//7
                    					//6
                    					//5
                    					//4-0	Master mode DAC MCLK to sampling frequency ratio
                    					//		10111 — 750
                    					//		00110 — 768(default)
                    					//		00111 — 1024
                    					//		01010 — 1536;

                    25,	0x19, 0x20,     //DAC CONTROL 3, DEFAULT 0011 0010
                    					//7-6	00 – 0.5 dB per 4 LRCK digital volume control ramp rate (default)
                    					//5		1 – enabled digital volume control soft ramp (default)
                    					//3		0 – normal (default)	1 – both channel gain control is set by DAC left gain control register
                    					//2		1 – mute analog outputs for both channels

                    26,	0x1a, 0x00,     //DAC CONTROL 4, DEFAULT 1100 0000
                    					//7-0	Digital volume control attenuates the signal in 0.5 dB incremental from 0 to –96 dB
                    					//		00000000 – 0 dB
                    					//		11000000 – -96 dB (default)

                    27,	0x1b, 0x00,     //DAC CONTROL 5, DEFAULT 1100 0000
                    					//7-0	Digital volume control attenuates the signal in 0.5 dB incremental from 0 to –96 dB.

                    39,	0x27, 0x80,     //DAC CONTROL 17, DEFAULT 0011 1000
                    					//7		0 – left DAC to left mixer disable (default)
                    					//6		0 – LIN signal to left mixer disable (default)
                    					//5-3	LIN signal to left mixer gain	000 – 6 dB
                    					//2		1 – RI2ROVOL use LI2LOVOL

                    42,	0x2a, 0x80,     //DAC CONTROL 20, DEFAULT 0011 1000
                    					//7		1 – right DAC to right mixer enable
                    					//6		0 – RIN signal to right mixer disable
                    					//5-3	RIN signal to right mixer gain	000 – 6 dB

                    3,	0x03, 0x00,     //ADC POWER MANAGEMENT, DEFAULT 1111 1100

                    //	2,	0x02, 0x30,     //
                    2,	0x02, 0x00,     //

                    16,	0x10, 0x00,     //ADC CONTROL 8, DEFAULT 1100 0000
                    					//7-0	Digital volume control attenuates the signal in 0.5 dB incremental from 0 to –96 dB
                    					//		00000001 – -0.5 dB
                    					//		11000000 – -96 dB (default)

                    17,	0x11, 0x00,     //

                    14,	0x0e, 0x2C,
                    //14,	0x0e, 0x24,
                    //14,	0x0e, 0x28,     //ADC CONTROL 6, DEFAULT 0011 0000
                    					//7		1 – left channel polarity inverted
                    					//5		0 – disable ADC left channel high pass filter
                    					//3-2	ALC MAXGAIN[1:0] for PGA max gain
                    					//1-0	ALC MINGAIN[1:0] for PGA min gain

                    //18,	0x12, 0xd2,  //ADC CONTROL 10, DEFAULT 0011 1000   
                    //18,	0x12, 0xda,     // 11011010 MIC Volume
                    18,	0x12, 0xe2,  // 11100010   
                    					//7-6	00 – ALC off
                    					//5-3	Set maximum gain of PGA 11011010 
                    					//2-0	Set minimum gain of PGA 11100010

                    19,	0x13, 0x90,     //ADC CONTROL 11, DEFAULT 1011 0000
                    					//7-4	ALC target	1001 – -3 dB	1010-1111 – -1.5 dB
                    					//3-0	ALC hold time before gain is increased	0000 – 0ms

                    20,	0x14, 0x05,     //ADC CONTROL 12, DEFAULT 0011 0010
                    					//7-4	ALC decay (gain ramp up) time, ALC mode/limiter mode:	0000 – 410 us/90.8 us
                    					//3-0	ALC attack (gain ramp down) time, ALC mode/limiter mode

                    21,	0x15, 0x06,     //ADC CONTROL 13, DEFAULT 0000 0110

                    22,	0x16, 0x50,     //ADC CONTROL 14, DEFAULT 0000 0000
                    					//7-3	Noise gate threshold		00000 – -76.5 dBFS		00001 – -75 dBFS
                    					//2-1	Noise gate type
                    					//0		Noise gate function enable

                    4,	0x04, 0x2c,     //DAC POWER MANAGEMENT, DEFAULT 1100 0000

                    48,	0x30, 0x1e,     //DAC CONTROL 26, DEFAULT 0000 0000
                    					//6		0 – normal (default)
                    					//5-0	LOUT volume
                    					//		000000 – -30dB (default)
                    					//		011110 – 0dB

                    49,	0x31, 0x1e,     //DAC CONTROL 27, DEFAULT 0000 0000
                    					//5-0	ROUT volume
                    					//		000000 – -30dB (default)
                    					//		011110 – 0dB

                    50,	0x32, 0x04,     //

                    //24,	0x18, 0x8A
                    24,	0x18, 0x97,     //DAC CONTROL 2, DEFAULT 0000 0110
                    					//7		Notch mode, there is 2Fs OSR for input data
                    					//		0 – notch mode disable (default)
                    					//		1 – notch mode enable
                    					//6
                    					//5
                    					//4-0	Master mode DAC MCLK to sampling frequency ratio
                    					//		10111 — 750
                    					//		00110 — 768(default)
                    					//		00111 — 1024
                    					//		01010 – 1536

                };
                
				int nCount = sizeof(es8388tablesss) / 3;
				Textout("8388S INIT...., nCount = %d", nCount);
				for ( i = 0; i < nCount; i++ )
                {
                    unsigned char addr = es8388tablesss[i * 3 + 1];
                    unsigned char value = es8388tablesss[i * 3 + 2];
                    iRet = WS222_i2c_write( 0x22, addr, &value, 1 );

                    if ( iRet )
                    {
                        Textout( "write i2c not ack\n" );
                        break;
                    }
                }
				Textout("8388S INIT OVER....");
			}
			else if ( AIC_nIndex == AIC_8388 )
			{
                #ifdef INCREASE_TALKING_SOUND
                unsigned char es8388table[35 * 2] =
                {
                    0x08, 0x85,
                    0x02, 0xf3,
                    0x2b, 0xc0,
                    0x00, 0x36,     //0x36 zqh
                    0x01, 0x72,
                    0x03, 0x00,
                    0x04, 0x30,     //0x3c
                    0x05, 0x00,
                    0x06, 0x00,
                    0x07, 0x7c,

            		/* BEGIN: Modified by wupm, 2013/5/29 */
            		//Increase Volume
                    //0x09, 0x88,
                    //0x09, 0x12,
            		//0x09, 0x99,
                    /* BEGIN: Modified by wupm, 2013/7/11 */
            		0x09, 0xc0, //0xb0

                    0x0a, 0xf0,     //0x80 0xf0
                    0x0b, 0x02,
                    0x0c, 0x0c,     //0x0c
                    0x0d, 0x0a,
                    0x10, 0x00,
                    0x11, 0x00,

            		/* BEGIN: Modified by wupm, 2013/5/29 */
            		//Increase Volume
                    //0x12, 0xd2,     //0x02
                    //0x12, 0xe2,
            		//0x12, 0xEA,
                    /* BEGIN: Modified by wupm, 2013/7/11 */
            		0x12, 0xeA,

                    0x13, 0xC0,
                    0x14, 0x05,
                    0x15, 0x06,

                    /* BEGIN: Modified by wupm, 2013/7/11 */
                    //0x16, 0xb3,     //0xb3
            		//0x16, 0x93,		//v1
            		//0x16, 0x73,		//v2
            		/* END:   Added by Baggio.wu, 2013/10/24 */
            		0x16, 0xb3,			//v3

                    0x17, 0x40,     //0x00
                    0x18, 0x0a,
                    0x19, 0xe2,
                    0x1a, 0x00,
                    0x1b, 0x00,
                    0x26, 0x00,
                    0x27, 0xb8,
                    0x28, 0x38,
                    0x29, 0x38,
                    0x2a, 0xb8,
                    0x2e, 0x1e,
                    0x2f, 0x1e,
                    0x02, 0x00
                };
                #else
                unsigned char es8388table[35 * 2] =
                {
                    0x08, 0x85,
                    0x02, 0xf3,
                    0x2b, 0xc0,
                    0x00, 0x36,     //0x36 zqh
                    0x01, 0x72,
                    0x03, 0x00,
                    0x04, 0x30,     //0x3c
                    0x05, 0x00,
                    0x06, 0x00,
                    0x07, 0x7c,
                    0x09, 0x88,
                    0x0a, 0xf0,     //0x80 0xf0
                    0x0b, 0x02,
                    0x0c, 0x0c,     //0x0c
                    0x0d, 0x0a,
                    0x10, 0x00,
                    0x11, 0x00,
                    0x12, 0xd2,     //0x02
                    0x13, 0xC0,
                    0x14, 0x05,
                    0x15, 0x06,
                    0x16, 0xb3,     //0xb3
                    0x17, 0x40,     //0x00
                    0x18, 0x0a,
                    0x19, 0xe2,
                    0x1a, 0x00,
                    0x1b, 0x00,
                    0x26, 0x00,
                    0x27, 0xb8,
                    0x28, 0x38,
                    0x29, 0x38,
                    0x2a, 0xb8,
                    0x2e, 0x1e,
                    0x2f, 0x1e,
                    0x02, 0x00
                };
                #endif

                for ( i = 0; i < 35; i++ )
                {
                    unsigned char addr = es8388table[i * 2];
                    unsigned char value = es8388table[i * 2 + 1];
                    iRet = WS222_i2c_write( 0x22, addr, &value, 1 );

                    if ( iRet )
                    {
                        Textout( "write i2c not ack\n" );
                        break;
                    }
                }
			}
			else if ( AIC_nIndex == AIC_8988 )
            {
                Wm8750ConfigInit();
                //SetWm8750_RX_VOL(36);
                //SetWm8750_TX_VOL(96);
                i2s_codec_enable();
                i2s_codec_frequency_config();
                SetWm8750LineoutVol(1);
                SetWm8750LineinVol();

				Textout("8988 Init Over...");
            }
			else
			{
				Textout("Audio Chip UNKNOW, So....");
			}
        }

        if ( iRet == 0 )
        {
            Textout( "=============Audio Chip is ok\n" );
        }

        else
        {
            Textout( "=============Audio Chip isn't exist...\n" );
        }

        gpioclose();
    }
}

#define UREG_8388_LEN 25
const unsigned char es8388UReg[UREG_8388_LEN*2]=
{
    0x2e,0x1b,
    0x2f,0x1b,
    0x2e,0x1a,
    0x2f,0x1a,
    0x2e,0x19,
    0x2f,0x19,
    0x2e,0x18,
    0x2f,0x18,
    0x2e,0x17,
    0x2f,0x17,
    0x2e,0x16,
    0x2f,0x16,
    0x2e,0x15,
    0x2f,0x15,
    0x2e,0x14,
    0x2f,0x14,
    0x2e,0x13,
    0x2f,0x13,
    0x2e,0x12,
    0x2f,0x12,
    0x2e,0x0,
    0x2f,0x0,
    0x04,0xc0,
    0x3,0xff,
    0x2,0xc3
};
#define UREG_8388S_LEN 25
const unsigned char es8388SUReg[UREG_8388S_LEN*2]=
{
    0x30,0x1b,
    0x31,0x1b,
    0x30,0x1a,
    0x31,0x1a,
    0x30,0x19,
    0x31,0x19,
    0x30,0x18,
    0x31,0x18,
    0x30,0x17,
    0x31,0x17,
    0x30,0x16,
    0x31,0x16,
    0x30,0x15,
    0x31,0x15,
    0x30,0x14,
    0x31,0x14,
    0x30,0x13,
    0x31,0x13,
    0x30,0x12,
    0x31,0x12,
    0x30,0x0,
    0x31,0x0,
    0x04,0xc0,
    0x3,0xff,
    0x2,0xc3
};
void ClearOnStartupAudioNoise()
{
    unsigned char addr = 0;
    unsigned char value = 0;
    int i = 0;
    int iRet = 0;

    Textout( "Check Audio chip=%d--[0--8988, 1--8388, 2--8388s]", AIC_nIndex );

    if ( AIC_nIndex == AIC_8388 )
    {

        for ( i = 0; i < UREG_8388_LEN; i++ )
        {
            addr = es8388UReg[i * 2];
            value = es8388UReg[i * 2 + 1];
            //iRet = BaI2CWriteBuffer( 0x11/*0x22*/, addr, &value, 1 ,I2C_ENABLE_SUB_ADDRESS);
            iRet = WS222_i2c_write( 0x22, addr, &value, 1 );

            if ( iRet )
            {
                Textout( "write i2c not ack\n" );
                break;
            }
        }
    }

    else if(AIC_nIndex == AIC_8388S)
    {


        for ( i = 0; i < UREG_8388S_LEN; i++ )
        {
            addr = es8388SUReg[i * 2];
            value = es8388SUReg[i * 2 + 1];
            //iRet = BaI2CWriteBuffer( 0x11/*0x22*/, addr, &value, 1 ,I2C_ENABLE_SUB_ADDRESS);
            iRet = WS222_i2c_write( 0x22, addr, &value, 1 );

            if ( iRet )
            {
                Textout( "write i2c not ack\n" );
                break;
            }
        }
    }

    else if ( AIC_nIndex == AIC_8988 )
    {

    }
}

void Es838Close( void )
{
    if ( AIC_nIndex == AIC_8388 || AIC_nIndex == AIC_8388S )
    {
        Es8388Close();
    }

    else if ( AIC_nIndex == AIC_8988 )
    {
        Wm8988Close();
    }
}

//============================================================================================

/* BEGIN: Added by Baggio.wu, 2014/11/7 */
#ifdef	SUPPORT_FM34
typedef struct
{
    unsigned char addrHi;
    unsigned char addrLo;
    unsigned char dataHi;
    unsigned char dataLo;
}Fm34Driver_t;

//{0x23,0x0F,0xFF,0xFF},  /*MIC Mute: 0x0000-mute / 0xFFFF-not mute*/
//{0x23,0x0E,0xFF,0xFF},  /*Speaker Mute: 0x0000-mute / 0xFFFF-not mute*/
#define	FMDSP_I2C_ADDR      0xC0
#define MAX_FM34_DRIVER     67
//{0x23,0x0E,0x00,0x03} //0x0003 bypass, 0x0000 AEC
Fm34Driver_t fm34Driver[MAX_FM34_DRIVER] = 
{
    {0x22,0xF2,0x00,0x44},  /*Bit[6-0]: This is to set the MIPS count(Default is 70 MIPS-0x0044)*/
    {0x23,0x03,0x19,0xF9},
    //{0x23,0x04,0x83,0x12},    //AGC MIC DISTANCE > 5M
    {0x23,0x04,0x03,0x12},
    {0x23,0x05,0x00,0x01},
    {0x23,0x01,0x00,0x00},  /*0x0000:Running 8k sample rate
                                                        0x0001:Tablet PC mode, SRC turn on for CHI/I2S set at 44.1khz or 48khz data rate
                                                        0x0002:Running 16k sample rate
                                                        0x0012:16k PDM DMIC sample rate, PCM/I2S 8k sample rate, Use for BWE feature
                                                     */
    //{0x23,0x0C,0x04,0x00},  /*MIC volume
    {0x23,0x0C,0x0F,0x00},  /*MIC volume
                                                0x0000~0x7FFF
                                                0x0100 is unit gain
                                                */
    //{0x23,0x0F,0x00,0x00},   
		
#ifdef VOLUME_INCREASE                                                
    {0x23,0x0D,0x26,0x00},
#else
	/* add begin by yiqing, 2015-10-15*/
    //{0x23,0x0D,0x12,0x00},
    {0x23,0x0D,0x32,0x00},
#endif
                                        /*Speaker volume
                                                    0x0000~0x7FFF
                                                    0x0100 is unit gain
                                                */
    {0x22,0xF6,0x00,0x03},  /*DAC mode select
                                                        0-DAC is disabled
                                                        1-DAC can be used
                                                        2-Use DAC through headphone pin
                                                        3-Use DAC through lineout pin
                                                    */
    {0x23,0x07,0xF4,0xF5},  /*0xF4F5----MIC0/1 PGAGAIN*/
    {0x23,0x08,0x0C,0xB0},
    {0x23,0x09,0x08,0x00},
    {0x23,0x0A,0x1A,0x00},
    {0x23,0x8C,0x10,0x00},
    {0x23,0x10,0x48,0x48},
    {0x23,0xA3,0x14,0x00},
    {0x23,0x25,0x20,0x00},
    {0x23,0xA5,0x00,0x00},
    {0x23,0x28,0x7F,0xFF},
    {0x23,0x2F,0x06,0x00},
    {0x23,0x32,0x00,0xA0},
    {0x23,0x33,0x00,0x0C},
    {0x23,0xB3,0x00,0x09},  //0x0009-------fei xian xing ya zhi
    {0x23,0xB4,0x00,0x03},  //0x0003-------fei xian xing ya zhi
    {0x23,0xB5,0x00,0x00},
    {0x23,0xB7,0x00,0x05},
    {0x23,0x39,0x00,0x01},
    {0x23,0xB9,0x20,0x00},
    {0x23,0xBA,0x03,0x00},
    {0x23,0xBB,0x00,0x00},
    {0x23,0xBD,0x02,0x00},
    {0x23,0xBC,0x03,0x00},
    {0x23,0xBE,0x50,0x00},
    {0x23,0x48,0x06,0x0D},  //0x060d-------20150113 mic GAIN
    {0x23,0x49,0x06,0x5B},
    {0x23,0xCE,0x80,0x00},
    {0x23,0xD0,0x03,0x00},
    {0x23,0xD5,0x60,0x00},
    {0x23,0x6E,0x20,0x00},
    {0x22,0xEE,0x00,0x00},
    {0x23,0x02,0x01,0x01},
    {0x23,0xEE,0x20,0x00},
    {0x23,0x84,0x00,0x07},
    {0x23,0xCF,0x04,0x00},
    {0x23,0xB8,0x24,0x00},
    {0x23,0xE7,0x07,0x06},
    {0x23,0xE8,0x13,0x83},
    {0x23,0xEA,0x7F,0xFF},
    {0x23,0xEB,0x00,0x20},
    {0x23,0xEC,0x00,0x80},
    {0x23,0xED,0x15,0x00},
    {0x22,0xF8,0x80,0x05},  /*Forte mockup
                                                        0x8000:handset 2 MIC 8K P.835 with A-filter
                                                        0x8001:hands-free 2 MIC 8K ACUQA
                                                        0x8002:handset 2 MIC 16K P.835 W/O A-filter
                                                        0x8003:hands-free 2 MIC 16K ACUQA
                                                        0x8004:hands-free BF 2 MIC 16K ACUQA 
                                                        0x8005:hands-free O+E 16K ACUQA 
                                                    */
    {0x22,0xF9,0x20,0xFF},  /*Bit[13]:I2S internal serial clock source 1-internal / 0-external
                                                        Bit[7]:I2S internal left_right select clock source 1-internal / 0-external
                                                        Bit[6]:I2S RXsi data latch edge 1-clock rising edge / 0-clock falling edge
                                                        Bit[5]:I2S channel data latch during LCLK=1, 1-left channel / 0-right channel
                                                        Bit[4]:Delay of data, 1:MSB valid one clock delay after frame sync / 0-MSB valid same cycle as frame sync
                                                        Bit[3-0]:Serial length of serial port, 0xF-16bit / 0x7-8bit
                                                    */
    {0x22,0xC6,0x00,0x7D},
    {0x22,0xC7,0x00,0x00},
    {0x22,0xC8,0x00,0x0C},
    {0x22,0xFA,0x22,0x8B},  /*Bit[8]: 0-I2S; 1-CHI
                                                        Bit[3]:DPLL tracking, 1-enable / 0-disable
                                                        Bit[2]:PDMCLK output, 1-cut off PDM_CLK output / 0-turn on PDM_CLK output
                                                        Bit[1]:MIC1 1-enable / 0-disable
                                                        Bit[0]:1-MIC0 enable / 0-MIC0~1 disable
                                                    */
    {0x22,0xD0,0x00,0x07},
    {0x22,0xD1,0x00,0x0F},
    {0x23,0x60,0x02,0x88},
    {0x23,0x61,0x19,0x4C},
    {0x23,0x62,0x73,0x33},
    {0x23,0x63,0x59,0x99},
    {0x23,0x64,0x00,0x74},
    {0x23,0xA6,0x00,0x28},
    {0x23,0xD3,0x14,0x00},
    {0x23,0xD4,0x2A,0x00},
    {0x22,0xFB,0x00,0x00}   /*
                                                        0x8000(read), DSP ready for external parameter downloading
                                                        0x0000(write), set by external host when parameter downloading is finished
                                                        0x5A5A(read), DSP is running
                                                    */
                                                    
};

void i2c_start()
{
    WS222_i2c_start();
    I2C_DELAY_LONG;
}
void i2c_stop()
{
	I2C_DELAY;
    WS222_i2c_stop();
    I2C_DELAY_LONG;
}
void i2c_write( unsigned char value)
{
	if ( WS222_i2c_write_byte( value ) )
    {
		Textout("Write (%02X) I2C ERROR", value);
        WS222_i2c_stop();
    }
}
unsigned char i2c_read()
{
	unsigned char ch;
	//ch = WS222_i2c_read_byte_ack();
	ch = WS222_i2c_read_byte_noack();
	return ch;
}


void i2c_open()
{
	gpioopen();
    gpioinit();


    #ifdef SUPPORT_FM34
    gpiovalue &= ~(1<<25);
    ioctl( i2cfd , RALINK_GPIO_WRITE, gpiovalue );
    usleep(1000);
    
    gpiovalue |= (1<<25);
    ioctl( i2cfd , RALINK_GPIO_WRITE, gpiovalue );
    usleep(15000);
    #endif
  
    gpiovalue |= 0x00000006;
    ioctl( i2cfd , RALINK_GPIO_WRITE, gpiovalue );

	//功放
	//gpiovalue &= ~(1<<17);
    //ioctl( i2cfd , RALINK_GPIO_WRITE, gpiovalue );
    //usleep(1000);
    
    WS222DelayMs( 10 );
    i2c_init();
    WS222DelayMs( 200 );
    Textout("------>gpiovalue=0x%08x", gpiovalue);
}
void i2c_close()
{
	gpioclose();
}

void fm_WriteMem(char AdrHi,char AdrLo, char DataHi,char DataLo)
{
	i2c_start();
	i2c_write(FMDSP_I2C_ADDR);
	i2c_write(0xFC); // send sync byte 1
	i2c_write(0xF3); // send sync byte 2
	i2c_write(0x3B); /*Bit[7-4]:0011-accessing Data memory; 0110-reading the data port
	                                Bit[3]:1-write / 0-read
	                                Bit[2-1]: Number of data bytes 
	                                11-no data
	                                10-3 bytes
	                                01-2 bytes
	                                00-1 byte
	                                Bit[0]:1-two address byte for accessing data memory / 0-one address byte for reading data port(0x25-low order byte / 0x26-high order byte)
	                                
	                             */
	i2c_write((unsigned char)AdrHi); // send address word
	i2c_write((unsigned char)AdrLo);
	i2c_write((unsigned char)DataHi); // send parameter data word
	i2c_write((unsigned char)DataLo);
	i2c_stop(); // stop
}

unsigned int fm_ReadMem(unsigned char AdrHi,unsigned char AdrLo)
{
	unsigned char hi_byte, low_byte;

	i2c_start(); // start
	i2c_write(FMDSP_I2C_ADDR);
	i2c_write(0xFC); // send sync byte 1
	i2c_write(0xF3); // send sync byte 2
	i2c_write(0x37); /*reference of fm_WriteMem*/
	i2c_write(AdrHi); 
	i2c_write(AdrLo);
	i2c_stop(); 
	
	// read low byte data (register 0x25) from bus
	i2c_start(); // start
	i2c_write(FMDSP_I2C_ADDR);
	i2c_write(0xFC); // send sync byte 1
	i2c_write(0xF3); // send sync byte 2
	i2c_write(0x60); // send command 0x60 to read register 0x25
	i2c_write(0x25);
	i2c_start(); // re-start
	i2c_write(FMDSP_I2C_ADDR+1);
	low_byte = i2c_read(); // get low byte from bus
	i2c_stop(); // stop

	// read high byte data (register 0x26) from bus
	i2c_start(); // start
	i2c_write(FMDSP_I2C_ADDR);
	i2c_write(0xFC); // send sync byte 1
	i2c_write(0xF3); // send sync byte 2
	i2c_write(0x60); // send command 0x60 to read register 0x26
	i2c_write(0x26);
	i2c_start(); // re-start
	i2c_write(FMDSP_I2C_ADDR+1);
	hi_byte = i2c_read(); // get high byte from bus
	i2c_stop(); // stop
	
	return ((unsigned int)hi_byte<<8)+(unsigned int)low_byte;
}

void dsp_Download(Fm34Driver_t* fm34, int len)
{
	int i;
	unsigned int value = 0;
	for (i=0; i<len; i++)
	{
		/*Textout("Write Config Data = [%02X %02X = %02X %02X]",
			fm34[i].addrHi,
			fm34[i].addrLo,
			fm34[i].dataHi,
			fm34[i].dataLo);
	        */
	        
		fm_WriteMem(
			fm34[i].addrHi,
			fm34[i].addrLo,
			fm34[i].dataHi,
			fm34[i].dataLo);
	    }

    //sleep(1);

    //value = fm_ReadMem(0x22, 0xFB);
    //Textout("FM34 Address(22FB)....DSP Running(0x%04x)", value);

    /*Read Only register*/
    #if 0
    while(1)
    {
        sleep(1);
        value = fm_ReadMem(0x22, 0xD3);
        Textout("Ready Test Read(22D3)....FM34 MIC(0x%04x)", value);

        value = fm_ReadMem(0x22, 0xD5);
        Textout("Ready Test Read(22D5)....5350 SPK(0x%04x)", value);

        value = fm_ReadMem(0x22, 0xD6);
        Textout("Ready Test Read(22D6)....5350 MIC(0x%04x)", value);

        value = fm_ReadMem(0x22, 0xD7);
        Textout("Ready Test Read(22D7)....FM34 SPK(0x%04x)", value);
    }
    #endif    
}


void fm_Init()
{
    Textout("*************************************fm_Init*********************************");
	i2c_open();
	dsp_Download(&fm34Driver[0], MAX_FM34_DRIVER );

	i2c_close();
}

void fm34_check()
{
    unsigned int value = 0;

    value = fm_ReadMem(0x22, 0xFB);
    Textout("FM34 Address(22FB)....DSP Running(0x%04x)", value);
	//i2c_open();
	//dsp_Download(&fm34Driver[0], MAX_FM34_DRIVER );

    if(0x5a5a != value)
    {
        Textout("FM34 not running,reset!");
        fm_Init();
    }
	
}

#endif


