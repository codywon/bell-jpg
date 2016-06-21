#ifndef __VIDEO_H__
#define __VIDEO_H__

int HD_VideoInOpen( void );
int HD_VideoInClose( int iChannelID );

int HD_VideoOutOpen( int iChnNumScreenDiv );
int HD_VideoOutClose( int iChnNumScreenDiv );

int HD_VideoInBindOutput( int iViChannelID, int iVoChannelID );
int HD_VideoInUnBindOutput( int iViChannelID, int iVoChannelID );

int GetBmp( char alarm, char* filename );

#endif //__VIDEO_H__
