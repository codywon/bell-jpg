//=={{audio: ADPCM codec==============================================================
int g_nEnAudioPreSample = 0;
int g_nEnAudioIndex = 0;
int g_nDeAudioPreSample[4];
int g_nDeAudioIndex = 0;

static int gs_index_adjust[8] = { -1, -1, -1, -1, 2, 4, 6, 8};
static int gs_step_table[89] =
{
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371,
    408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493,
    10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

/* BEGIN: Modified by wupm, 2013/5/29 */
/*
void EncoderClr( void )
{
    g_nEnAudioPreSample = 0;
    g_nEnAudioIndex = 0;
}

void DecoderClr( void )
{
    char i;

    for ( i = 0; i < 4; i++ )
    {
        g_nDeAudioPreSample[i] = 0;
    }

    g_nDeAudioIndex = 0;
}
*/
void EncoderClr(int sample,int index)
{
	g_nEnAudioPreSample = sample;
	g_nEnAudioIndex = index;
}

void DecoderClr( int sample,int index )
{
	//g_nDeAudioPreSample = sample;
	//g_nDeAudioIndex = index;
	char i;

    for ( i = 0; i < 4; i++ )
    {
        g_nDeAudioPreSample[i] = sample;
    }

    g_nDeAudioIndex = index;
}

/* BEGIN: Added by wupm, 2013/5/29 */
int GetEncoderAduioPreSample()
{
	return g_nEnAudioPreSample;
}
int GetEncoderAudioIndex()
{
	return g_nEnAudioIndex;
}

void ADPCMEncode( unsigned char* pRaw, int nLenRaw, unsigned char* pBufEncoded )
{
    short* pcm = ( short* )pRaw;
    int cur_sample;
    int i;
    int delta;
    int sb;
    int code;
    nLenRaw >>= 1;

    for ( i = 0; i < nLenRaw; i++ )
    {
        cur_sample = pcm[i];
        delta = cur_sample - g_nEnAudioPreSample;

        if ( delta < 0 )
        {
            delta = -delta;
            sb = 8;
        }

        else
        {
            sb = 0;
        }

        code = 4 * delta / gs_step_table[g_nEnAudioIndex];

        if ( code > 7 )
        {
            code = 7;
        }

        delta = ( gs_step_table[g_nEnAudioIndex] * code ) / 4 + gs_step_table[g_nEnAudioIndex] / 8;

        if ( sb )
        {
            delta = -delta;
        }

        g_nEnAudioPreSample += delta;

        if ( g_nEnAudioPreSample > 32767 )
        {
            g_nEnAudioPreSample = 32767;
        }

        else if ( g_nEnAudioPreSample < -32768 )
        {
            g_nEnAudioPreSample = -32768;
        }

        g_nEnAudioIndex += gs_index_adjust[code];

        if ( g_nEnAudioIndex < 0 )
        {
            g_nEnAudioIndex = 0;
        }

        else if ( g_nEnAudioIndex > 88 )
        {
            g_nEnAudioIndex = 88;
        }

        if ( i & 0x01 )
        {
            pBufEncoded[i >> 1] |= code | sb;
        }

        else
        {
            pBufEncoded[i >> 1] = ( code | sb ) << 4;
        }
    }
}

void ADPCMDecode( char* pDataCompressed, int nLenData, char* pDecoded, char usit )
{
    int i;
    int code;
    int sb;
    int delta;
    short* pcm = ( short* )pDecoded;
    nLenData <<= 1;

    for ( i = 0; i < nLenData; i++ )
    {
        if ( i & 0x01 )
        {
            code = pDataCompressed[i >> 1] & 0x0f;
        }

        else
        {
            code = pDataCompressed[i >> 1] >> 4;
        }

        if ( ( code & 8 ) != 0 )
        {
            sb = 1;
        }

        else
        {
            sb = 0;
        }

        code &= 7;
        delta = ( gs_step_table[g_nDeAudioIndex] * code ) / 4 + gs_step_table[g_nDeAudioIndex] / 8;

        if ( sb )
        {
            delta = -delta;
        }

        g_nDeAudioPreSample[usit] += delta;

        if ( g_nDeAudioPreSample[usit] > 32767 )
        {
            g_nDeAudioPreSample[usit] = 32767;
        }

        else if ( g_nDeAudioPreSample[usit] < -32768 )
        {
            g_nDeAudioPreSample[usit] = -32768;
        }

        pcm[i] = g_nDeAudioPreSample[usit];
        g_nDeAudioIndex += gs_index_adjust[code];

        if ( g_nDeAudioIndex < 0 )
        {
            g_nDeAudioIndex = 0;
        }

        if ( g_nDeAudioIndex > 88 )
        {
            g_nDeAudioIndex = 88;
        }
    }
}
//==}}audio: ADPCM codec==============================================================


