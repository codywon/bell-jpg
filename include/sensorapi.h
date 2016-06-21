
#ifndef _291A_API_H_
#define _291A_API_H_

#define SENSOR_DEFAULT_ENABLE       1
#define SENSOR_DEFAULT_DISABLE      0

#define H264_GOP                    30

#define H264_CBR_MODE               1
#define H264_VBR_MODE               2       /*Unsupport*/

//**************************** Only for 291B ************************************
#define OSD_COUNTING_ENABLE     1
#define OSD_COUNTING_DISABLE    0
#define OSD_SHOW_LINE_ENABLE    1
#define OSD_SHOW_LINE_DISABLE   0
#define OSD_SHOW_BLOCK_ENABLE   1
#define OSD_SHOW_BLOCK_DISBALE  0
//0:Black  1:Red  2:Green  3:Blue  4:White
#define OSD_BLACK   0
#define OSD_RED     1
#define OSD_GREEN   2
#define OSD_BLUE    3
#define OSD_WHITE   4

#define MOTION_ENABLE   1
#define MOTION_DISABLE  0

#define MOTION_MASK_LEN 24      //16x12 = 192 window area, 24*8 bit = 192
#define MOTION_THRESHOLD_BASE   0x00000100     //0~65536

#define H264_RES_VGA                0
#define H264_RES_QVGA               1
#define H264_RES_QQVGA              2
#define H264_RES_HD                 3

#define H264_SIZE_HD				((1280<<16)|720)
#define H264_SIZE_VGA				((640<<16)|480)
#define H264_SIZE_QVGA				((320<<16)|240)
#define H264_SIZE_QQVGA				((160<<16)|112)

#define MULTI_STREAM_HD_QVGA		1
#define MULTI_STREAM_HD_QQVGA		2
#define MULTI_STREAM_HD_QVGA_QQVGA	4       /*Unsupport*/
#define MULTI_STREAM_HD_VGA			8       /*Unsupport*/
#define MULTI_STREAM_HD_VGA_QVGA	16       /*Unsupport*/

typedef struct
{
    int bUse;
    unsigned char brightness;
    unsigned char contrast;
    unsigned char saturation;
    unsigned char hue;
}TSENSORDEFAULT, *PTSENSORDEFAULT;

//**************************** Only for 291B ************************************
int GetH264Status();
int GetMJPEGStatus();
int StartVideoCapture(unsigned char resolution, unsigned char framerate);
int StopVideoCapture();
void InitSensorLib( unsigned char resolution, unsigned char framerate );
int H264EncThread( void );

char GetH264Flag();

int H264SetIFrame( void );
int H264SetFrameRate( unsigned int framerate );
int H264SetGOP( unsigned int gop );
int H264GetGOP( unsigned int* gop);

int SCC_SetQuant( unsigned int quant );
//int TUTK_SetQuant( unsigned int quant );
//int H264SetQuant( unsigned int quant );
int H264SetRateMode( unsigned int mode );
int H264SetBitRate( unsigned int bitrate );
int H264SetMultiBitrate(unsigned int streamid, unsigned int bitrate);
int H264GetMultiBitrate(unsigned int streamid, unsigned int* pbitrate);
int H264SetHue( int param );
int H264SetSaturation( int param );
int H264SetBrightness( int param );
int H264SetContrast( int param );

int H264EnvMode( unsigned char param );
int H264SetSize( unsigned char resolution, unsigned char framerate );

int H264SetMirr( unsigned char mode );
int H264GetMirr( unsigned char* mode );
int H264SetFlip( unsigned char mode );
int H264GetFlip( unsigned char* mode );

int ObjMotionInit( int threshold, int areas );


void GetSensorDefault(PTSENSORDEFAULT pparam);
int GetUseSensorDefaultFlag();
void SetUseSensorDefaultFlag(int flag);

//**************************** Only 8433 ****************************************
int H264SetIRCutMode( char mode ); /*
                                            [0] Default
                                            [1] Day mode
                                            [2] Night mode
                                            */

//After set STREAM_MODE_FRAMEBASE_H264L_MJPEG
int MjpgSetSize(unsigned char resolution); /* MJPEG Format [3] VGA [2] QVGA [1] QQVGA */

int MjpgSetQuality(unsigned char quality); /*             
                                                [0] High quality
                                                [1] Median quality
                                                [2] Low quality
                                                */                                            
void MotionResutlClear( void );
void ClrMotionValue( void );                                                
//**************************** Only 8433 ****************************************

//**************************** Only for 291B ************************************
int MJpgSetBitrate(unsigned int mjpg_bitrate);
int MJpgGetBitrate(unsigned int* mjpg_bitrate);

int H264GetMultiStatus( unsigned char *strm_type,   /* Get current stream number
                                                            0: N/A ( No H264 preview )
                                                            1: Single stream
                                                            2: 2 string
                                                            3: 3 string

                                                            when no h264 preview( single-stream or multi-stream), strm_type=0, when preview, strm_type=1-3
                                                            */
                                unsigned int* format);  /* Report current multistream format
                                                        Use bit map to report current resolution.
                                                        Report valid when ubMultiStrmStat = 2 or 3 only
                                                        Bit0: HD+QVGA
                                                        Bit1: HD+QQVGA
                                                        Bit2: HD+QQVGA+QVGA (292 only)
                                                        Bit3: HD+VGA (292 only)
                                                        Bit4: HD+QVGA+VGA (292 only)
                                                        Bit5: VGA+QVGA(291B only)

                                                        Other bits are reversed
                                                        */

int H264GetMultiInfo( unsigned char *strm_type,     /* Maximum multistream support
                                                            0: N/A
                                                            1: single stream only
                                                            2: maximum 2 stream
                                                            3: maximum 3 stream

                                                            */
                            unsigned int* format);      /* Supported Resolution:
                                                            Use bit map to list supported resolution
                                                            Bit0: HD+QVGA
                                                            Bit1: HD+QQVGA
                                                            Bit2: HD+QQVGA+QVGA
                                                            Bit3: HD+VGA
                                                            Bit4: HD+QVGA+VGA
                                                            Bit5: VGA+QVGA

                                                            For example:
                                                            291B will support 2 stream, so this byte will return 0x23(bit0 & bit1 = 1)

                                                            292A will support 3 stream, so this byte will 
                                                            Return 0x1f(bit0,bit1,bit2, bit3, bit4 = 1)


                                                            Other bits are reversed
                                                            */


int H264SetOsdTimerCounting(unsigned char enable); /* 0: disable timer counting
                                                                1: enable timer counting
                                                                */
int H264SetOsdRtc(unsigned int osd_rtc_year, 
                        unsigned char osd_rtc_month, 
                        unsigned char osd_rtc_day, 
                        unsigned char osd_rtc_hour, 
                        unsigned char osd_rtc_minute, 
                        unsigned char osd_rtc_second);
int H264GetOsdRtc(unsigned int* osd_rtc_year, 
                        unsigned char* osd_rtc_month, 
                        unsigned char* osd_rtc_day, 
                        unsigned char* osd_rtc_hour, 
                        unsigned char* osd_rtc_minute, 
                        unsigned char* osd_rtc_second);
int H264SetOsdSize(unsigned char osd_size_line, /* Line OSD Pixel Size: 1~4, 0: doesn't change osd size*/
                        unsigned char osd_size_block); /* Block OSD Pixel Size: 1~4, 0: doesn't change osd size*/
int H264GetOsdSize(unsigned char* osd_size_line, 
                        unsigned char* osd_size_block);
//#define OSD_BLACK   0
//#define OSD_RED     1
//#define OSD_GREEN   2
//#define OSD_BLUE    3
//#define OSD_WHITE   4
int H264SetOsdColor(unsigned char osd_color_font, /* 0:Black  1:Red  2:Green  3:Blue  4:White */
                        unsigned char osd_color_border); /* 0:Black  1:Red  2:Green  3:Blue  4:White */
int H264GetOsdColor(unsigned char* osd_color_font, 
                        unsigned char* osd_color_border);

int H264SetOsdEnable(unsigned char osd_show_line, /* 0: Line OSD Show Disable, 1: Line OSD Show Enable */
                        unsigned char osd_show_block);      /* 0: Block OSD Show Disable, 1: Block OSD Show Enable */
int H264GetOsdEnable(unsigned char* osd_show_line, 
                        unsigned char* osd_show_block);
/*
FHD(292A only) : OSD size = 3
HD            : OSD size = 2
VGA~QVGA    : OSD size = 1
< QVGA        : no OSD

*/
int H264SetOsdAutoScale(unsigned char osd_autoscale_line, /* 0: Line OSD Auto scale Disable, 1: Line OSD Auto scale Enable */
                        unsigned char osd_autoscale_block);         /* 0: Block OSD Auto scale Disable, 1: Block OSD Auto scale Enable */
int H264GetOsdAutoScale(unsigned char* osd_autoscale_line, 
                        unsigned char* osd_autoscale_block);

/*
multistream only
multistream active, disable AUTO scale
< QVGA        : no OSD
*/
int H264SetOsdMultiSize(unsigned char osd_ms_size_stream0, /* Stream0 OSD Pixel Size: 1~4, 0: doesn't change osd size */
                        unsigned char osd_ms_size_stream1,          /* Stream1 OSD Pixel Size: 1~4, 0: doesn't change osd size */
                        unsigned char osd_ms_size_stream2);         /* Stream2 OSD Pixel Size: 1~4, 0: doesn't change osd size */

int H264GetOsdMultiSize(unsigned char* osd_ms_size_stream0, 
                        unsigned char* osd_ms_size_stream1,
                        unsigned char* osd_ms_size_stream2);

int H264SetOsdPosition(unsigned char osd_type, /* OSD control type
                                                        0x01 :  set up Line base OSD
                                                        0x02:  set up Block base OSD
                                                        0x03:  Both OSD use same position
                                                        */
                        unsigned int osd_start_row,     /* OSD row start (unit : 16lines) */
                        unsigned int osd_start_col);    /* OSD col start(unit : 16 pixels) */

int H264GetOsdPosition(unsigned int* osd_line_start_row, 
                        unsigned int* osd_line_start_col,
                        unsigned int* osd_block_start_row,
                        unsigned int* osd_block_start_col);

int H264SetOsdMsPosition(unsigned char osd_ms_position_streamid, /* OSD stream number 
                                                                            0x00 :  stream 0 
                                                                            0x01 :  stream 1 
                                                                            0x02 :  stream 2 
                                                                            */
                        unsigned char osd_ms_start_row,     /* OSD row start (unit : 16lines) */
                        unsigned char osd_ms_start_col);    /* OSD col start(unit : 16 pixels) */

int H264GetOsdMsPosition(unsigned char* osd_ms_s0_start_row, 
                        unsigned char* osd_ms_s0_start_col,
                        unsigned char* osd_ms_s1_start_row,
                        unsigned char* osd_ms_s1_start_col,
                        unsigned char* osd_ms_s2_start_row,
                        unsigned char* osd_ms_s2_start_col);

int H264SetOsd2ndString(unsigned char group, /* To read/write OSD 2nd string group number. 
                                                        0x00: Group 0 : string 1~8
                                                        0x01: Group 1 : string 9~16
                                                        0x02: Group 2 : string 17~19
                                                        */
                                char* strstring);       /* The String.
                                                            Example: "ROOM" or "OFFICE"
                                                            */
int H264GetOsd2ndString(unsigned char group, char* strstring);

int H264SetMotionEnable( unsigned char enable );    /* D0: Motion Detect Enable/Disable.
                                                                0: MD Disable
                                                                1: MD Enable

                                                                D[7:1]: reserved
                                                                */
int H264GetMotionEnable( unsigned char* enable );
int H264SetMotionThreshold( unsigned int md_threshold ); /* Motion Detection Threshold
                                                                    Threshold to detect a block is moving
                                                                    0~65535
                                                                    */
int H264GetMotionThreshold( unsigned int* md_threshold );
int H264SetMotionMask( unsigned char* md_mask ); /* D[191:0] : Motion Detect Window Mask (16x12 window area, little endian), Please reference Figure 3-1.
                                                            D0: Motion detect window area 0
                                                            D1: Motion detect window area 1
                                                            D2: Motion detect window area 2
                                                            D3: Motion detect window area 3
                                                            D4: Motion detect window area 4
                                                            D5: Motion detect window area 5
                                                            D6: Motion detect window area 6
                                                            D7: Motion detect window area 7
                                                            ...
                                                            D(N): Motion detect window area(N)

                                                            1: Enable window area motion detect report (Mask Disable)
                                                            0: Disable window area motion detect report (Mask Enable)
                                                            */
int H264GetMotionMask( unsigned char* md_mask );
int H264SetMotionResult( unsigned char* md_mask );
int H264GetMotionResult( unsigned char* md_mask ); /* D[191:0]: Motion Detect Result. (little endian)
                                                            If  read result, firmware auto clear it.

                                                            D0: Window area 0 detect status is moving.
                                                            D1: Window area 1 detect status is moving
                                                            D2: Window area 2 detect status is moving
                                                            D3: Window area 3 detect status is moving
                                                            D4: Window area 4 detect status is moving
                                                            D5: Window area 5 detect status is moving
                                                            D6: Window area 6 detect status is moving
                                                            D7: Window area 7 detect status is moving
                                                            ...
                                                            D(N): Window area N
                                                            */

//**************************** Only for 291B ************************************



#endif

