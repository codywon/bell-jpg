
#ifndef _BA_AUDIO_PLAY_H_
#define _BA_AUDIO_PLAY_H_

#define WAV_FILE_FRAME_SIZE 1024


typedef struct _WAVEFILE_HEADER
{
    char ChunkID[4];     //"RIFF"
    long ChunkSize;      //the length of file
    char Format[4];         //"WAVE"
    
    char Subchunk1ID[3]; //"fmt"
    long Subchunk1Size;  //16/18 bytes
    short AudioFormat;   //0x0001
    short NumChannels;   //mono/stereo
    long SampleRate;       //sample rate
    long ByteRate;          //bytes per second
    short BlockAlign;       //align bytes
    short BitsPerSample;    //16bit
    
    char Subchunk2ID[4];    //"data"
    long Subchunk2Size;     //data size
}WAVEFILE_HEADER;

typedef void (*BaAudioPlayCallback)(void);


#endif

