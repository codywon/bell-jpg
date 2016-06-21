
#include "wm8750.h"
#include "debug.h"

#if 1 //defined(CONFIG_I2S_WM8750)
					/* 8k  11.025k  12k   16k  22.05k  24k  32k   44.1k   48k  88.2k   96k*/
unsigned long i2s_codec_12p288Mhz[11]  = {0x0C,  0x00, 0x10, 0x14,  0x38, 0x38, 0x18,  0x20, 0x00,  0x00, 0x1C};
unsigned long i2s_codec_12Mhz[11]      = {0x0C,  0x32, 0x10, 0x14,  0x37, 0x38, 0x18,  0x22, 0x00,  0x3E, 0x1C};
unsigned long i2s_codec_24p576Mhz[11]  = {0x4C,  0x00, 0x50, 0x54,  0x00, 0x78, 0x58,  0x00, 0x40,  0x00, 0x5C};
#endif


typedef struct _wm8750_config_t
{
    int srate;
    int txvol;
    int rxvol;
    int slave_en;
    int wordlen_24b;
}wm8750_config_t, *pwm8750_config_t;

wm8750_config_t wm8750_config;
void SetWm8750_TX_VOL(int arg)
{
    if((int)arg > 127)
        wm8750_config.txvol = 127;
    else if((int)arg < 48)
        wm8750_config.txvol = 48;
    else
        wm8750_config.txvol = arg;
    
    audiohw_set_master_vol(arg,arg);
}

void SetWm8750LineoutVol(int Aout)
{
    audiohw_set_lineout_vol(Aout, wm8750_config.txvol, wm8750_config.txvol);
}

void SetWm8750_RX_VOL(int arg)
{
    if((int)arg > 63)
        wm8750_config.rxvol = 63;
    else if((int)arg < 0)
        wm8750_config.rxvol = 0;
    else
        wm8750_config.rxvol = arg;
}

void SetWm8750LineinVol()
{
    audiohw_set_linein_vol(wm8750_config.rxvol, wm8750_config.rxvol);
}

int i2s_codec_enable()
{
    /* Codec initialization */
    audiohw_preinit();

	audiohw_postinit(wm8750_config.slave_en, 1, 1, wm8750_config.wordlen_24b);
	return 0;	
}

int i2s_codec_disable()
{
	audiohw_close();

	return 0;
}	

int i2s_codec_frequency_config()
{
	unsigned long data;
	unsigned long* pTable;


	pTable = i2s_codec_12Mhz;
	data = pTable[wm8750_config.srate];

    audiohw_set_frequency(data|0x01);
	
	return 0;
}

void Wm8750ConfigInit()
{
    wm8750_config.srate = 0;    /*0-8khz*/
    wm8750_config.rxvol = 60;   //The bigger the value, the greater the volume max:127
    wm8750_config.txvol = 120;  //The bigger the value, the greater the volume      
    wm8750_config.slave_en = 0;
    wm8750_config.wordlen_24b = 0;
}

