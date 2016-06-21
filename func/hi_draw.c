/******************************************************************************

  Copyright (C), 2005-2006, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_draw.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2005/5/21
  Last Modified :
  Description   : osd interface implementation for GUI
  Function List : too many!... :P
  History       :
  1.Date        : 2005/5/21
    Author      : Z42136
    Modification: Created file

  2.Date        : 2005/7/4
    Author      : Z42136
    Modification: 修改视频输入输出接口，简化Video API接口

  3.Date        : 2006/4/7
    Author      : q60002125
    Modification: modify for HI3510 Demo
******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if 0
#include "hd_common.h"
#include "hi_draw.h"

#include "hi_ascii_8_12.h"
#include "hi_ascii_20_24.h"


#define GET_R_COMPONENT(RGBColor)  (HI_U8)((RGBColor>>10)&0x1f)
#define GET_G_COMPONENT(RGBColor)  (HI_U8)((RGBColor>>5)&0x1f)
#define GET_B_COMPONENT(RGBColor)  (HI_U8)(RGBColor&0x1f)

#define GET_Y_COMPONENT(R,G,B)   (HI_U8)(((R*66+G*129+B*25)/256)+16)
#define GET_Cb_COMPONENT(R,G,B)  (HI_U8)((((R*112-G*94)-B*18)/256)+128)
#define GET_Cr_COMPONENT(R,G,B)  (HI_U8)((((B*112-R*38)-G*74)/256)+128)


    /* global variables */

    /* font class list-chain */
    typedef struct tag_OSD_FONT_LIST
    {
        OSD_FONT 			OsdFont;
        struct tag_OSD_FONT_LIST*	 pNext;
    } OSD_FONT_LIST;

    static OSD_FONT_LIST*	 g_pOsdFontList = NULL;
    static OSD_FONT 	g_CurrentOsdFont = {"\0", NULL, 0, 0};

    /* ============= text setting ============= */
    static HI_U16 g_OsdTextColor = 0xFFFF;//0xffff;//
    /* [0...2] = [B, G, R] */

//static OSD_BG_MODE_T g_OsdTextBgMode = OSD_BG_MODE_OPAQUE;
    static OSD_BG_MODE_T g_OsdTextBgMode = OSD_BG_MODE_TRANSPARENT;

    static HI_U16 g_OsdTextBgColor = 0x8000;//0xFFFF;//0x801f;//0xfc00;
    /* [0...2] = [Cr, Cb, Y] */

    int OSD_FontInit( void )
    {
        int ret = 0;
        ret |= OSD_RegisterFont( "8_12", ascii_table_8_12, 8, 12 );
        ret |= OSD_RegisterFont( "20_24", ascii_table_20_24, 20, 24 );

        if ( 0 != ret )
        {
            return -1;
        }

        //ret = OSD_SetFont("20_24");
        ret = OSD_SetFont( "8_12" );

        if ( 0 != ret )
        {
            return -1;
        }

        return 0;
    }

    int OSD_SetFont( const char* FontName )
    {
        OSD_FONT_LIST*	 pOsdFontNode;

        if ( NULL == FontName )
        {
            return -1;
        }

        pOsdFontNode = g_pOsdFontList;

        while ( pOsdFontNode )
        {
            if ( strcmp( pOsdFontNode->OsdFont.FontName, FontName ) == 0 )
            {
                memcpy( &g_CurrentOsdFont, &( pOsdFontNode->OsdFont ), sizeof( OSD_FONT ) );
                return 0;
            }

            pOsdFontNode = pOsdFontNode->pNext;
        }

        return -1;
    }

    OSD_FONT* OSD_GetFont( void )
    {
        return &g_CurrentOsdFont;
    }

    int OSD_RegisterFont( const char* FontName, const HI_U8** FontEntry, HI_U32 FontWidth, HI_U32 FontHeight )
    {
        OSD_FONT_LIST*	 pNewOsdFontNode;
        OSD_FONT_LIST*	 pLastOsdFontNode;
        HI_U32 		FontNameLen;

        if ( ( NULL == FontName ) || ( NULL == FontEntry ) || ( 0 == FontWidth ) || ( 0 == FontHeight ) )
        {
            return -1;
        }

        pNewOsdFontNode = ( OSD_FONT_LIST* )malloc( sizeof( OSD_FONT_LIST ) );

        if ( NULL == pNewOsdFontNode )
        {
            PRINTF( "not enough memory to malloc!\n" );
            return -1;
        }

        FontNameLen = strlen( FontName );

        if ( FontNameLen >= MAX_FONT_NAME )
        {
            strncpy( pNewOsdFontNode->OsdFont.FontName, FontName, MAX_FONT_NAME - 1 );
            pNewOsdFontNode->OsdFont.FontName[MAX_FONT_NAME - 1] = '\0';
        }

        else
        {
            strcpy( pNewOsdFontNode->OsdFont.FontName, FontName );
        }

        pNewOsdFontNode->OsdFont.FontEntry = ( HI_U8** )FontEntry;
        pNewOsdFontNode->OsdFont.FontWidth = FontWidth;
        pNewOsdFontNode->OsdFont.FontHeight = FontHeight;
        pNewOsdFontNode->pNext = NULL;

        if ( g_pOsdFontList == NULL )
        {
            g_pOsdFontList = pNewOsdFontNode;
            return 0;
        }

        pLastOsdFontNode = g_pOsdFontList;

        for ( ; ; )
        {
            if ( pLastOsdFontNode->pNext )
            {
                pLastOsdFontNode = pLastOsdFontNode->pNext;
            }

            else
            {
                pLastOsdFontNode->pNext = pNewOsdFontNode;
                return 0;
            }
        }
    }

    int OSD_TextOut( OSD_TEXT_T* pstText, HI_U32 ulXStart, HI_U32 ulYStart, const HI_PCHAR pString )
    {
        HI_U8  	lineData;
        HI_U32 	lineDataNum;
        HI_U8*	 pLineData;
        HI_U32 	charIndex, charLen, charLine, charRow;
        HI_U32 	charValidWidth;

        //HI_U32 	charMemOffset;

        if ( ( NULL == pstText ) || ( NULL == pString ) )
        {
            return -1;
        }

        lineDataNum = ( g_CurrentOsdFont.FontWidth - 1 ) / 8 + 1;
        charLen = strlen( pString );
#if 0
        pstText->height 	= g_CurrentOsdFont.FontHeight;
        pstText->width 	= g_CurrentOsdFont.FontWidth * charLen;
        pstText->pRGBBuffer = ( HI_U8* )malloc( pstText->width * pstText->height );
#endif

        if ( pstText->pRGBBuffer == NULL )
        {
            PRINTF( "not enough memory to malloc!\n" );
            return -1;
        }

        //charMemOffset = 0;
        for ( charIndex = 0; charIndex < charLen; charIndex++ )
        {
            charValidWidth = g_CurrentOsdFont.FontWidth;

            /* Font Height */
            for ( charLine = 0; charLine < g_CurrentOsdFont.FontHeight; charLine++ )
            {
                //pLineData = pstText->pRGBBuffer + charLine * pstText->width + charMemOffset;
                pLineData = pstText->pRGBBuffer + charLine * pstText->width * 2 + charIndex * g_CurrentOsdFont.FontWidth * 2;

                /* Font Width */
                for ( charRow = 0; charRow < charValidWidth; charRow++ )
                {
                    lineData = g_CurrentOsdFont.FontEntry[( HI_U8 )pString[charIndex]][charLine * lineDataNum + charRow / 8];

                    if ( lineData & ( 0x80 >> ( charRow % 8 ) ) )
                    {
                        memcpy( ( void* )pLineData, ( void* )( &g_OsdTextColor ), 2 );
                    }

                    else if ( g_OsdTextBgMode )
                    {
                        memcpy( ( void* )pLineData, ( void* )( &g_OsdTextBgColor ), 2 );
                    }

                    pLineData += 2;
                }
            }

            //charMemOffset += g_CurrentOsdFont.FontWidth*2;
        }

        return 0;
    }

    int OSD_SetTextColor( HI_U32 ulTextColor )
    {
        HI_U16 r, g, b;
        r = GET_R_COMPONENT( ulTextColor );
        g = GET_G_COMPONENT( ulTextColor );
        b = GET_B_COMPONENT( ulTextColor );
        g_OsdTextColor = 0x8000 | ( ( r << 10 ) & 0x7c00 ) | ( ( g << 5 ) & 0x3e0 ) | ( b & 0x1f );
        return 0;
    }

    int OSD_SetTextBgMode( OSD_BG_MODE_T BgMode )
    {
        if ( OSD_BG_MODE_BUTT <= BgMode )
        {
            return -1;
        }

        g_OsdTextBgMode = BgMode;
        return 0;
    }

    int OSD_SetTextBgColor( HI_U32 ulBgColor )
    {
        HI_U16 r, g, b;
        r = GET_R_COMPONENT( ulBgColor );
        g = GET_G_COMPONENT( ulBgColor );
        b = GET_B_COMPONENT( ulBgColor );
        g_OsdTextBgColor = 0x8000 | ( ( r << 10 ) & 0x7c00 ) | ( ( g << 5 ) & 0x3e0 ) | ( b & 0x1f );
        return 0;
    }
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

