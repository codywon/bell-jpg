#include<stdio.h>
#include<stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#include "param.h"
#include "debug.h"

#include "sensorapi.h"

unsigned char videocmd = 0;
unsigned char videoparam = 0;
unsigned char videoctrl = 0;
sem_t	savesem;
sem_t	camsem;

void NoteCamSem( void )
{
    sem_post( &camsem );
}


void NoteSaveSem()
{
    sem_post( &savesem );
}

void VideoParamInit_8433( void )
{
#ifdef USE_SENSOR_DEFAULT

    if ( GetUseSensorDefaultFlag() == 1 )
    {
		#ifdef BELINK
		bparam.stVencParam.bysize = 0;
		bparam.stVencParam.brightness = 128;
        bparam.stVencParam.contrast = 136;
        bparam.stVencParam.saturation = 134;
        bparam.stVencParam.chroma = 0;
		#else
		TSENSORDEFAULT param;
        GetSensorDefault( &param );
		
        SetUseSensorDefaultFlag( SENSOR_DEFAULT_DISABLE );

        bparam.stVencParam.brightness = param.brightness;
        bparam.stVencParam.contrast = param.contrast;
        bparam.stVencParam.saturation = param.saturation;
        bparam.stVencParam.chroma = param.hue;
		#endif
		
		
        Textout( "Sensor default,brightness=%d, contrast=%d, saturation=%d, hue=%d", bparam.stVencParam.brightness,
        bparam.stVencParam.contrast, bparam.stVencParam.saturation, bparam.stVencParam.chroma );

        NoteSaveSem();
    }
	/* add begin by yiqing, 2015-12-25*/
	H264SetSize(bparam.stVencParam.bysize, bparam.stVencParam.byframerate );

    H264SetBrightness( bparam.stVencParam.brightness );
    H264SetHue( bparam.stVencParam.chroma );
    H264SetSaturation( bparam.stVencParam.saturation );
    H264SetContrast( bparam.stVencParam.contrast );
#else
    H264SetBrightness( ( bparam.stVencParam.brightness - 128 ) >> 2 );
    H264SetHue( ( bparam.stVencParam.chroma - 128 ) >> 2 );
    H264SetSaturation( bparam.stVencParam.saturation >> 1 );
    H264SetContrast( bparam.stVencParam.contrast / 4 );
#endif

    H264SetMirr( bparam.stVencParam.videomode );

    H264SetGOP( H264_GOP );
    H264SetRateMode( H264_CBR_MODE );
    H264SetBitRate( bparam.stVencParam.bitrate );
}

void VideoParamInit_3861( void )
{
    H264SetBrightness(bparam.stVencParam.brightness);
    H264SetContrast(bparam.stVencParam.contrast >> 3);

    if ( bparam.stVencParam.videoenv > 2 )
    {
        bparam.stVencParam.videoenv = 0x00;
    }

    H264EnvMode( bparam.stVencParam.videoenv );

    char mirrflip = bparam.stVencParam.videomode;

#ifdef CHANGE_VIDEO_MIRROR
    if ( mirrflip == 0 ) 		mirrflip = 4; //normal
    else if ( mirrflip == 1 ) 	mirrflip = 3; //flip
    else if ( mirrflip == 2 ) 	mirrflip = 2; //mirr
    else if ( mirrflip == 3 ) 	mirrflip = 5; //mirr flip
#else
    if ( mirrflip == 0 ) 		mirrflip = 5; //normal
    else if ( mirrflip == 1 ) 	mirrflip = 3; //flip
    else if ( mirrflip == 2 ) 	mirrflip = 2; //mirr
    else if ( mirrflip == 3 ) 	mirrflip = 4; //mirr flip
#endif
	sleep(1);
    H264SetMirr( mirrflip );
}


void camera_control( unsigned char index, unsigned int param )
{
    char flag = 0;
    int nFlag=0;
	//Textout("========================================================");
	Textout("index:%d, param:%d",index,param);
    if ( videoctrl == 0x00 )
    {
        videoparam = param;

        switch ( index )
        {
            case 0:         //720p vga qvga
                if ( bparam.stVencParam.bysize == videoparam )
                {
                    flag = 0x01;
                    printf( "video size is same\n" );
                }

                else
                {
                    bparam.stVencParam.bysize = videoparam;
                    printf( "bysize %d\n", bparam.stVencParam.bysize );
                }

                break;

            case 1:         //bright
                bparam.stVencParam.brightness = videoparam;

                if ( bparam.stVencParam.brightness <= 0 )
                {
                    bparam.stVencParam.brightness = 0;
                }

                if ( bparam.stVencParam.brightness > 255 )
                {
                    bparam.stVencParam.brightness = 255;
                }

                break;

            case 2:         //contrast
                bparam.stVencParam.contrast = videoparam;
                break;

            case 3:         //mode
                bparam.stVencParam.videoenv = videoparam;
                break;

            case 5:         //mirr flip
                bparam.stVencParam.videomode = videoparam;
                break;

            case 6:         //framerate
                if ( videoparam > 0 && videoparam <= 60 )
                {
                    bparam.stVencParam.byframerate = videoparam;
                }

                break;

            case 7:         //default
                #if defined(SENSOR_3861)
                bparam.stVencParam.brightness = 0;
                bparam.stVencParam.contrast = 128;
				#elif defined(BELINK)
				bparam.stVencParam.bysize = 0;
				bparam.stVencParam.brightness = 128;
                bparam.stVencParam.contrast = 136;
                bparam.stVencParam.saturation = 134;
                bparam.stVencParam.chroma = 0;
                #elif defined(USE_SENSOR_DEFAULT)
                SetUseSensorDefaultFlag( SENSOR_DEFAULT_ENABLE );
                #else
                bparam.stVencParam.brightness = 128;
                bparam.stVencParam.contrast = 128;
                bparam.stVencParam.saturation = 128;
                bparam.stVencParam.chroma = 0;
                #endif
                
                break;

            case 8:         //saturation
                bparam.stVencParam.saturation = videoparam;
                break;

            case 9:         //hue
                bparam.stVencParam.chroma = videoparam;
                break;

            case 10:        //osd
                //bparam.stVencParam.OSDEnable = param;
                break;

            case 12:
                //bparam.stVencParam.byframeratesub = param;
                break;

                /* BEGIN: Added by wupm, 2013/6/6 */
            case 13:
                bparam.stVencParam.bitrate = videoparam;
                break;

            case 14:	//sharpness
                bparam.stVencParam.ircut  = videoparam;
                break;

            case 100:
                bparam.stPTZParam.byRate = videoparam;
                break;
				
            case 101:
             switch(videoparam)
             {
                 case 0:
                    Textout("livestream:snapshot video");
                    #ifdef SENSOR_3861
                    bparam.stVencParam.byframerate = 5;
                    #else
                    bparam.stVencParam.byframerate = 30;
                    #endif
                    bparam.stVencParam.bysize = 1;
                    SetSnapshotVideo(TRUE);
                    H264SetSize(bparam.stVencParam.bysize, bparam.stVencParam.byframerate );

                    #ifdef SENSOR_8433
                    MjpgSetSize(3);
                    #endif
                     break;
                 case 1:
                    Textout("livestream: 5fps QVGA");
                    #ifdef SENSOR_3861
                    bparam.stVencParam.byframerate = 5;
                    #else
                    bparam.stVencParam.byframerate = 10;
                    #endif
                    bparam.stVencParam.bysize = 1;
                    SetSnapshotVideo(FALSE);
                    H264SetSize(bparam.stVencParam.bysize, bparam.stVencParam.byframerate );

                    #ifdef SENSOR_8433
                    H264SetGOP( bparam.stVencParam.byframerate );
                    MjpgSetSize(3);
                    #endif
                     break;
                 case 2:
                    Textout("livestream: 15fps VGA");
                    bparam.stVencParam.byframerate = 15;
                    bparam.stVencParam.bysize = 0;
                    SetSnapshotVideo(FALSE);
                    H264SetSize(bparam.stVencParam.bysize, bparam.stVencParam.byframerate );

                    #ifdef SENSOR_8433
                    H264SetGOP( bparam.stVencParam.byframerate );
                    MjpgSetSize(3);
                    #endif
                     break;
                 case 3:
                    Textout("livestream: 10fps 720P");
                    bparam.stVencParam.byframerate = 10;
                    bparam.stVencParam.bysize = 3;
                    SetSnapshotVideo(FALSE);
                    H264SetSize(bparam.stVencParam.bysize, bparam.stVencParam.byframerate );
                    H264SetGOP(bparam.stVencParam.byframerate);
                    #ifdef SENSOR_8433
                    H264SetGOP( bparam.stVencParam.byframerate );
                    MjpgSetSize(3);
                    #endif
                     break;
                 default:
                     Textout("Unkown parameter 101, value=%d", videoparam);
                     break;
             }    
            default:
                break;
        }

        if ( flag == 0x00 )
        {
            videocmd = index;
            videoparam = param;
            videoctrl = 1;
            NoteCamSem();
			
			/* BEGIN: Added by yiqing, 2015/4/14 */
			NoteSaveSem();
        }
    }

    else
    {
        printf( "video process %d videocmd %d\n", videoctrl, videocmd );
    }
}

void VideoParamChange( void )
{
	//Textout("*************************************************************");
	Textout("videocmd;%d",videocmd);
    if ( videoctrl == 0x00 )
    {
        return;
    }

    switch ( videocmd )
    {
        case 0:         //720p vga qvga
            H264SetSize(bparam.stVencParam.bysize, bparam.stVencParam.byframerate );
            break;

        case 1:
            #if defined(SENSOR_3861) || defined(USE_SENSOR_DEFAULT)
            H264SetBrightness(bparam.stVencParam.brightness);
            #else
            H264SetBrightness( ( bparam.stVencParam.brightness - 128 ) >> 2 );
            #endif
            
            break;

        case 2:
            #if defined(SENSOR_3861)
            H264SetContrast(bparam.stVencParam.contrast >> 3);
            #elif defined(USE_SENSOR_DEFAULT)
            H264SetContrast( bparam.stVencParam.contrast );
            #else
            H264SetContrast( bparam.stVencParam.contrast / 4 );
            #endif
            break;

        case 3:         //mode
            H264EnvMode(videoparam);
            break;

        case 5:         //mirr flip
            #ifdef SENSOR_3861
                #ifdef CHANGE_VIDEO_MIRROR
                if ( videoparam == 0 ) 	videoparam = 3;	//normal
                else if ( videoparam == 1 ) 	videoparam = 5; //flip
                #else
                if ( videoparam == 0 ) 		videoparam = 5; //normal
                else if ( videoparam == 1 ) 	 videoparam = 3; //flip
                #endif
                else if ( videoparam == 2 ) 	 videoparam = 2; //mirr
                else if ( videoparam == 3 ) videoparam = 4; 	//mirr flip
            #endif
            
            H264SetMirr( videoparam );
            break;

        case 6:         //framerate
            H264SetFrameRate(bparam.stVencParam.byframerate);
            break;

        case 7:
            #ifdef SENSOR_3861
            VideoParamInit_3861();
            #else
            VideoParamInit_8433();
            #endif
            break;

        case 8:         //saturation
            #ifdef USE_SENSOR_DEFAULT
            H264SetSaturation( bparam.stVencParam.saturation );
            #else
            H264SetSaturation( bparam.stVencParam.saturation >> 1 );
            #endif 
            break;

        case 9:         //hue
            #ifdef USE_SENSOR_DEFAULT
            H264SetHue( bparam.stVencParam.chroma );
            #else
            H264SetHue( ( bparam.stVencParam.chroma - 128 ) >> 2 );
            #endif
            break;

        case 10:        //osd
            //bparam.stVencParam.OSDEnable = param;
            break;

        case 12:
            //bparam.stVencParam.byframeratesub = param;
            break;

        case 14:	//sharpness
            //bparam.stVencParam.sharpness = videoparam;
            //H264SetShapness();
            break;

        default:
            Textout("Unkown cmd=%d", videocmd);
            break;
    }

    videoctrl = 0;
}

void* videoparamProc( void* p )
{
    if ( ( sem_init( &camsem, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        sem_wait( &camsem );
        VideoParamChange();
        usleep( 500*1000 );

    }
}
void* FlashSaveProc( void* p )
{
    if ( ( sem_init( &savesem, 0, 0 ) ) != 0 )
    {
        printf( "sem init failed\n" );
    }

    while ( 1 )
    {
        sem_wait( &savesem );
        SaveSystemParam( &bparam );
    }
}
void StopVideoCaptureEx()
{
    StopVideoCapture();
}

void StartVideoCaptureEx()
{
    StartVideoCapture(bparam.stVencParam.bysize, bparam.stVencParam.byframerate);
#if defined(SENSOR_8433)
    VideoParamInit_8433();
#elif defined(SENSOR_3861)
    VideoParamInit_3861();
#endif
}

void NoteMainStreamSend()
{
    NoteP2pUser();
    NoteJpegUser();
    //NoteSDUser();
}

unsigned char GetMainFrameRate()
{
    return bparam.stVencParam.byframerate;
}

unsigned char GetSubFrameRate()
{
    return bparam.stVencParam.byframerate;
}

void VideoInInit( void )
{
    pthread_t 	videoparamthread;
    pthread_t		savethread;

    pthread_create( &savethread, 0, &FlashSaveProc, NULL );
    pthread_create( &videoparamthread, 0, &videoparamProc, NULL );

	pthread_detach(savethread);
	pthread_detach(videoparamthread);
}


