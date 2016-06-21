#ifndef __AUDIO_H__
#define __AUDIO_H__

int HD_AudioInOpen( int iAiDevID, int iAiChnID, unsigned char byIs16KSample );
int HD_AudioInClose( int iAiDevID, int iAiChnID );

int HD_AudioOutOpen( int iAoDevID, int iAoChnID );
int HD_AudioOutClose( int iAoDevID, int iAoChnID );

int HD_AudioStart( int iAiChannelID, int iAoChannelID );
int HD_AudioStop( int iAiChannelID, int iAoChannelID );

#endif //__AUDIO_H__
