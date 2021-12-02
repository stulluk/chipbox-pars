/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : orion_gfx_2dlib.c
// MODULE NAME      : 
// AUTHOR           : Jiasheng Chen
// AUTHOR'S EMAIL   : jschen@celestialsemi.com
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 06-11-23
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.1      06-11-23   jiasheng Chen           Original
// 0.2      07-05-14   Jiasheng Chen           Modified to support DirectFB
// ---------------------------------------------------------------------------
// [DESCRIPTION]
// Host 2d Operation Lib icluding Fill, COPY, Scalor, ColorKey, Compositer and ROP
// And Some Para Initial, For example: Clut4/8 Table and Scalor FIR Coeff Update
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/
#include "orion_gfx_test_vect.h"
#include "orion_gfx_2dlib.h"
#include "orion_gfx_loglib.h"
#include <assert.h>
#include <stdlib.h>
#include <memory.h>


enum
{
	GFX_INTERRUPT_MODE   = (0), //1 Enable, 0 Disable
};
//Clut4&8 Table
//Gfx Initial


HG2D_RESULT hGfx2DInit()
{
	//Invalid para Check
	//Reset Only
	hGfxAPIHardWareReset();
	hGfxAPISWReset();
	return HG2D_SUCCESS;
	
}
HG2D_RESULT hGfx2DInitClut4Table ( GFX_DEVICE_CONTEXT * hGfxDC, U32 *Clut4Table)
{
	HG2D_RESULT hResult = HG2D_UNKNOW;
	if (hGfxDC == NULL || Clut4Table == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else
	{
		U32 *p = NULL;
		for (p = hGfxDC->Clut4Table; p < hGfxDC->Clut4Table + CLUT4_TABLE_LEN; p++)
		{
			*p = *Clut4Table++;
		}
	}
	//Send CMD to Gfx HW
	hGfxDC->UpdateClut4Table = 1;
	hGfxDC->UpdateClut4TableMode = 0;
	if (!hGfxAPIActiveDeviceContext(hGfxDC, GFX_DC_CLUT4_TABLE))
	{
		hResult = HG2D_FAILED_ACTIVE_DEVICE_CONTEXT;
	}
	else
	{	
		hResult = HG2D_SUCCESS;
	}
	hGfxDC->UpdateClut4Table = 0;
	hGfxDC->UpdateClut4TableMode = 0;
	return hResult;

}
static U32 _GfxCLUT4Pal[CLUT4_TABLE_LEN] = 
{
    // from 000-015 
      0xff000000  ,   //  000
      0xff000080  ,   //  001
      0xff008000  ,   //  002
      0xff008080  ,   //  003
      0xff800000  ,   //  004
      0xff800080  ,   //  005
      0xff808000  ,   //  006
      0xff808080  ,   //  007
      0xff404040  ,   //  008
      0xff0000ff  ,   //  009
      0xff00ff00  ,   //  010
      0xff00ffff  ,   //  011
      0xffff0000  ,   //  012
      0xffff00ff  ,   //  013
      0xffffff00  ,   //  014
      0xffffffff  ,   //  015
};

U32 *hGfx2DGetDefaultClut4Table()
{
	return _GfxCLUT4Pal;
}
HG2D_RESULT hGfx2DSetDefaultClut4Table(GFX_DEVICE_CONTEXT * hGfxDC)
{
	HG2D_RESULT hResult = HG2D_UNKNOW;
	U32 *Clut4Table = NULL;
	if (hGfxDC == NULL) return HG2D_INVALID_PARA;
	//Initial Clut4 and Clut8 Table

	Clut4Table = hGfx2DGetDefaultClut4Table();
	if (Clut4Table == NULL)
	{
		return HG2D_FAILED_TO_GET_DEFAULT_CLUT4_TABLE;
	}
	hResult = hGfx2DInitClut4Table(hGfxDC, Clut4Table);
	return hResult;
	
}

HG2D_RESULT hGfx2DInitClut8Table ( GFX_DEVICE_CONTEXT * hGfxDC, U32 *Clut8Table)
{
	HG2D_RESULT hResult = HG2D_UNKNOW;
	if (hGfxDC == NULL || Clut8Table == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	{
		U32 *p = NULL;
		for (p = hGfxDC->Clut8Table; p < hGfxDC->Clut8Table + CLUT8_TABLE_LEN; p++)
		{
			*p = *Clut8Table++;
		}
	}
	//Send CMD to Gfx HW
	hGfxDC->UpdateClut8Table = 1;
	hGfxDC->UpdateClut8TableMode = 0;
	if (!hGfxAPIActiveDeviceContext(hGfxDC, GFX_DC_CLUT8_TABLE))
	{
		hResult = HG2D_FAILED_ACTIVE_DEVICE_CONTEXT;
	}
	else
	{	
		hResult = HG2D_SUCCESS;
	}
	hGfxDC->UpdateClut8Table = 0;
	hGfxDC->UpdateClut8TableMode = 0;
	return hResult;
	
}
static  U32  _GfxCLUT8Pal[CLUT8_TABLE_LEN] = 
{
    // from 000-215   216 color web safe palette
      0xff000000  ,   //  000
      0xff000033  ,   //  001
      0xff000066  ,   //  002
      0xff000099  ,   //  003
      0xff0000cc  ,   //  004
      0xff0000ff  ,   //  005
      0xff003300  ,   //  006
      0xff003333  ,   //  007
      0xff003366  ,   //  008
      0xff003399  ,   //  009
      0xff0033cc  ,   //  010
      0xff0033ff  ,   //  011
      0xff006600  ,   //  012
      0xff006633  ,   //  013
      0xff006666  ,   //  014
      0xff006699  ,   //  015
      0xff0066cc  ,   //  016
      0xff0066ff  ,   //  017
      0xff009900  ,   //  018
      0xff009933  ,   //  019
      0xff009966  ,   //  020
      0xff009999  ,   //  021
      0xff0099cc  ,   //  022
      0xff0099ff  ,   //  023
      0xff00cc00  ,   //  024
      0xff00cc33  ,   //  025
      0xff00cc66  ,   //  026
      0xff00cc99  ,   //  027
      0xff00cccc  ,   //  028
      0xff00ccff  ,   //  029
      0xff00ff00  ,   //  030
      0xff00ff33  ,   //  031
      0xff00ff66  ,   //  032
      0xff00ff99  ,   //  033
      0xff00ffcc  ,   //  034
      0xff00ffff  ,   //  035
      0xff330000  ,   //  036
      0xff330033  ,   //  037
      0xff330066  ,   //  038
      0xff330099  ,   //  039
      0xff3300cc  ,   //  040
      0xff3300ff  ,   //  041
      0xff333300  ,   //  042
      0xff333333  ,   //  043
      0xff333366  ,   //  044
      0xff333399  ,   //  045
      0xff3333cc  ,   //  046
      0xff3333ff  ,   //  047
      0xff336600  ,   //  048
      0xff336633  ,   //  049
      0xff336666  ,   //  050
      0xff336699  ,   //  051
      0xff3366cc  ,   //  052
      0xff3366ff  ,   //  053
      0xff339900  ,   //  054
      0xff339933  ,   //  055
      0xff339966  ,   //  056
      0xff339999  ,   //  057
      0xff3399cc  ,   //  058
      0xff3399ff  ,   //  059
      0xff33cc00  ,   //  060
      0xff33cc33  ,   //  061
      0xff33cc66  ,   //  062
      0xff33cc99  ,   //  063
      0xff33cccc  ,   //  064
      0xff33ccff  ,   //  065
      0xff33ff00  ,   //  066
      0xff33ff33  ,   //  067
      0xff33ff66  ,   //  068
      0xff33ff99  ,   //  069
      0xff33ffcc  ,   //  070
      0xff33ffff  ,   //  071
      0xff660000  ,   //  072
      0xff660033  ,   //  073
      0xff660066  ,   //  074
      0xff660099  ,   //  075
      0xff6600cc  ,   //  076
      0xff6600ff  ,   //  077
      0xff663300  ,   //  078
      0xff663333  ,   //  079
      0xff663366  ,   //  080
      0xff663399  ,   //  081
      0xff6633cc  ,   //  082
      0xff6633ff  ,   //  083
      0xff666600  ,   //  084
      0xff666633  ,   //  085
      0xff666666  ,   //  086
      0xff666699  ,   //  087
      0xff6666cc  ,   //  088
      0xff6666ff  ,   //  089
      0xff669900  ,   //  090
      0xff669933  ,   //  091
      0xff669966  ,   //  092
      0xff669999  ,   //  093
      0xff6699cc  ,   //  094
      0xff6699ff  ,   //  095
      0xff66cc00  ,   //  096
      0xff66cc33  ,   //  097
      0xff66cc66  ,   //  098
      0xff66cc99  ,   //  099
      0xff66cccc  ,   //  100
      0xff66ccff  ,   //  101
      0xff66ff00  ,   //  102
      0xff66ff33  ,   //  103
      0xff66ff66  ,   //  104
      0xff66ff99  ,   //  105
      0xff66ffcc  ,   //  106
      0xff66ffff  ,   //  107
      0xff990000  ,   //  108
      0xff990033  ,   //  109
      0xff990066  ,   //  110
      0xff990099  ,   //  111
      0xff9900cc  ,   //  112
      0xff9900ff  ,   //  113
      0xff993300  ,   //  114
      0xff993333  ,   //  115
      0xff993366  ,   //  116
      0xff993399  ,   //  117
      0xff9933cc  ,   //  118
      0xff9933ff  ,   //  119
      0xff996600  ,   //  120
      0xff996633  ,   //  121
      0xff996666  ,   //  122
      0xff996699  ,   //  123
      0xff9966cc  ,   //  124
      0xff9966ff  ,   //  125
      0xff999900  ,   //  126
      0xff999933  ,   //  127
      0xff999966  ,   //  128
      0xff999999  ,   //  129
      0xff9999cc  ,   //  130
      0xff9999ff  ,   //  131
      0xff99cc00  ,   //  132
      0xff99cc33  ,   //  133
      0xff99cc66  ,   //  134
      0xff99cc99  ,   //  135
      0xff99cccc  ,   //  136
      0xff99ccff  ,   //  137
      0xff99ff00  ,   //  138
      0xff99ff33  ,   //  139
      0xff99ff66  ,   //  140
      0xff99ff99  ,   //  141
      0xff99ffcc  ,   //  142
      0xff99ffff  ,   //  143
      0xffcc0000  ,   //  144
      0xffcc0033  ,   //  145
      0xffcc0066  ,   //  146
      0xffcc0099  ,   //  147
      0xffcc00cc  ,   //  148
      0xffcc00ff  ,   //  149
      0xffcc3300  ,   //  150
      0xffcc3333  ,   //  151
      0xffcc3366  ,   //  152
      0xffcc3399  ,   //  153
      0xffcc33cc  ,   //  154
      0xffcc33ff  ,   //  155
      0xffcc6600  ,   //  156
      0xffcc6633  ,   //  157
      0xffcc6666  ,   //  158
      0xffcc6699  ,   //  159
      0xffcc66cc  ,   //  160
      0xffcc66ff  ,   //  161
      0xffcc9900  ,   //  162
      0xffcc9933  ,   //  163
      0xffcc9966  ,   //  164
      0xffcc9999  ,   //  165
      0xffcc99cc  ,   //  166
      0xffcc99ff  ,   //  167
      0xffcccc00  ,   //  168
      0xffcccc33  ,   //  169
      0xffcccc66  ,   //  170
      0xffcccc99  ,   //  171
      0xffcccccc  ,   //  172
      0xffccccff  ,   //  173
      0xffccff00  ,   //  174
      0xffccff33  ,   //  175
      0xffccff66  ,   //  176
      0xffccff99  ,   //  177
      0xffccffcc  ,   //  178
      0xffccffff  ,   //  179
      0xffff0000  ,   //  180
      0xffff0033  ,   //  181
      0xffff0066  ,   //  182
      0xffff0099  ,   //  183
      0xffff00cc  ,   //  184
      0xffff00ff  ,   //  185
      0xffff3300  ,   //  186
      0xffff3333  ,   //  187
      0xffff3366  ,   //  188
      0xffff3399  ,   //  189
      0xffff33cc  ,   //  190
      0xffff33ff  ,   //  191
      0xffff6600  ,   //  192
      0xffff6633  ,   //  193
      0xffff6666  ,   //  194
      0xffff6699  ,   //  195
      0xffff66cc  ,   //  196
      0xffff66ff  ,   //  197
      0xffff9900  ,   //  198
      0xffff9933  ,   //  199
      0xffff9966  ,   //  200
      0xffff9999  ,   //  201
      0xffff99cc  ,   //  202
      0xffff99ff  ,   //  203
      0xffffcc00  ,   //  204
      0xffffcc33  ,   //  205
      0xffffcc66  ,   //  206
      0xffffcc99  ,   //  207
      0xffffcccc  ,   //  208
      0xffffccff  ,   //  209
      0xffffff00  ,   //  210
      0xffffff33  ,   //  211
      0xffffff66  ,   //  212
      0xffffff99  ,   //  213
      0xffffffcc  ,   //  214
      0xffffffff  ,   //  215

    // from 216 to 248,  32 level grey scale
      0xff000000  ,   //  216
      0xff080808  ,   //  217
      0xff101010  ,   //  218
      0xff181818  ,   //  219
      0xff202020  ,   //  220
      0xff282828  ,   //  221
      0xff303030  ,   //  222
      0xff383838  ,   //  223
      0xff404040  ,   //  224
      0xff484848  ,   //  225
      0xff505050  ,   //  226
      0xff585858  ,   //  227
      0xff606060  ,   //  228
      0xff686868  ,   //  229
      0xff707070  ,   //  230
      0xff787878  ,   //  231
      0xff808080  ,   //  232
      0xff888888  ,   //  233
      0xff909090  ,   //  234
      0xff989898  ,   //  235
      0xffa0a0a0  ,   //  236
      0xffa8a8a8  ,   //  237
      0xffb0b0b0  ,   //  238
      0xffb8b8b8  ,   //  239
      0xffc0c0c0  ,   //  240
      0xffc8c8c8  ,   //  241
      0xffd0d0d0  ,   //  242
      0xffd8d8d8  ,   //  243
      0xffe0e0e0  ,   //  244
      0xffe8e8e8  ,   //  245
      0xfff0f0f0  ,   //  246
      0xfff8f8f8  ,   //  247
      0xffffffff  ,   //  248

    // from 249 to 255, 7 color rainbow
      0xff0000ff  ,   //  249
      0xff00ff00  ,   //  250
      0xff00ffff  ,   //  251
      0xffff0000  ,   //  252
      0xffff00ff  ,   //  253
      0xffffff00  ,   //  254
      0xffffffff  ,   //  255
};    
U32 *hGfx2DGetDefaultClut8Table()
{
	return _GfxCLUT8Pal;
}

HG2D_RESULT hGfx2DSetDefaultClut8Table(GFX_DEVICE_CONTEXT * hGfxDC)
{
	HG2D_RESULT hResult = HG2D_UNKNOW;
	U32 *Clut8Table = NULL;
	
	if (hGfxDC == NULL) return HG2D_INVALID_PARA;
	//Initial Clut4 and Clut8 Table
		
	Clut8Table = hGfx2DGetDefaultClut8Table();
	if (Clut8Table == NULL)
	{
		return HG2D_FAILED_TO_GET_DEFAULT_CLUT8_TABLE;
	}
	hResult = hGfx2DInitClut8Table(hGfxDC, Clut8Table);
	return hResult;	
}

I32 IsRectValid( COLOR_IMG *Img, RECT *Rect) //Boolean
{
	if (Img == NULL || Rect == NULL) 
	{
		return 0;
	}
	else if (Rect->left   < 0 || Rect->left   > Img->PixelWidth  ||
		     Rect->top    < 0 || Rect->top    > Img->PixelHeight ||
		     Rect->right  < 0 || Rect->right  > Img->PixelWidth  ||
		     Rect->bottom < 0 || Rect->bottom > Img->PixelHeight )
	{
		return 0;
	}
	else if (!(Rect->left  <= Rect->right) || 
		     !(Rect->top   <= Rect->bottom) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
I32 CmpRect( RECT *Rect0, RECT *Rect1) //Equal == 1 not Equal return 0
{
	if (!(Rect0->left  <= Rect0->right) || 
		!(Rect0->top   <= Rect0->bottom) )
	{
		return 0;
	}
	else if (!(Rect1->left  <= Rect1->right) || 
		     !(Rect1->top   <= Rect1->bottom) )
	{
		return 0;
	}
	else if ((Rect0->right - Rect0->left) != 
		     (Rect1->right - Rect1->left) )	
	{
		return 0;
	}
	else if ((Rect0->bottom - Rect0->top) != 
		     (Rect1->bottom - Rect1->top) )	
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
static I32 IsColorFormatValid( GFX_COLOR_FORMAT ColorFormat)
{
	switch (ColorFormat)
	{
		case GFX_CF_RGB565:			
		case GFX_CF_ARGB4444:
		case GFX_CF_A0:
		case GFX_CF_CLUT8:
		case GFX_CF_CLUT4:
			return 1;
			break;
		default:
			return 0;
			break;
	}
}
void CalcuDramPara
(
    COLOR_IMG *DesImg, 
    RECT      *DesRect,	
    I32        IsVReverse,
	U32       *pBaseAddr,                        //Bytes Addr
	U32       *pLinePitch,                       //Bytes Addr
	U32       *pSkipPixelLine               //Valid Only When Clut4
)
{
	U32 PixelBitWidth = 0;
	U32 BaseAddr      = 0;                        //Bytes Addr
	U32 LinePitch     = 0;                       //Bytes Addr
	U32 SkipPixelLine = 0;              //Valid Only When Clut4
	
	switch (DesImg->ColorFormat)
	{
		case GFX_CF_RGB565:			
		case GFX_CF_ARGB4444:
		case GFX_CF_A0:
			PixelBitWidth = 16;
			break;
		case GFX_CF_CLUT8:
			PixelBitWidth =  8;
			break;
		case GFX_CF_CLUT4:
			PixelBitWidth =  4;
			break;
		default:
			PixelBitWidth =  0;
			break;
	}
	assert(PixelBitWidth != 0);
	if (PixelBitWidth == 0) return;
	//if Clut4 and Width is odd, the Line width should be Bytes Aligned
	//LinePitch = (DesImg->PixelWidth * PixelBitWidth + 8 - 1) / 8;
	LinePitch = DesImg->LinePitch;
	if (LinePitch < (DesImg->PixelWidth * PixelBitWidth + 8 - 1) / 8)
	{
		ERROR_D3("##ORION_GFX: Warning, DesImg LinePitch(%d) < DesImg Width(%d) in Format(%d)\n", LinePitch,DesImg->PixelWidth,DesImg->ColorFormat);
	}
	if (!IsVReverse)
	{
		BaseAddr = DesImg->StartAddress + 
	               DesRect->top * LinePitch +                
	               (DesRect->left * PixelBitWidth) / 8;
	}
	else
	{
		U32 LineIdx = 0;
		LineIdx  = DesRect->bottom;
		if (LineIdx > 0) LineIdx--;
		BaseAddr = DesImg->StartAddress + 
	               LineIdx* LinePitch +                
	               (DesRect->left * PixelBitWidth) / 8;
	}
	#if (ORION_VERSION == ORION_130)
	{		
		BaseAddr = BaseAddr & (~(0x3));	
	}
	#endif
	SkipPixelLine   = ((DesRect->left * PixelBitWidth) % 8) / PixelBitWidth;
	*pBaseAddr      = BaseAddr;                        //Bytes Addr
	*pLinePitch     = LinePitch;                       //Bytes Addr
	*pSkipPixelLine = SkipPixelLine;              //Valid Only When Clut4

}
//Fill 
HG2D_RESULT hGfx2DFill
( 
    GFX_DEVICE_CONTEXT *hGfxDC, 
    I32                 GfxSrcId,
    COLOR_IMG          *DesImg, 
    RECT               *DesRect, 
    GFX_ARGB_COLOR     *FillColor,
    ROP_OPT             RopValue 
)
{
	if (hGfxDC  == NULL ||
		GfxSrcId > 1    ||
		DesImg  == NULL ||
		DesRect == NULL ||
		FillColor == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else if (!IsColorFormatValid(DesImg->ColorFormat))
	{
		return HG2D_UNSUPPORTED_COLOR_FORMAT;
	}
	else if (!IsRectValid(DesImg, DesRect))
	{
		return HG2D_INVALID_DES_REC_VALUE;
	}
	else
	{
		HG2D_RESULT hResult = HG2D_UNKNOW;
	//S0 Control
		if (GfxSrcId == 0)
		{
			hGfxDC->S0Enable       = 1;
			hGfxDC->S0VReverseScan = 0;
			hGfxDC->S0FetchDram    = 0;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			                                     // if S0 Using Default Color then S0ColorFormat should be ARGB565 or A0
			hGfxDC->S0ColorFormat  = GFX_CF_A0;  //Must
			hGfxDC->S0DefaultColor.Alpha = FillColor->Alpha;     //ARGB8888 Format
			hGfxDC->S0DefaultColor.Red   = FillColor->Red;       //ARGB8888 Format
			hGfxDC->S0DefaultColor.Green = FillColor->Green;     //ARGB8888 Format
			hGfxDC->S0DefaultColor.Blue  = FillColor->Blue;      //ARGB8888 Format
			/*
			hGfxDC->S0BaseAddr;                      //Bytes Addr
			hGfxDC->S0LinePitch;                     //Bytes Addr
			hGfxDC->S0SkipPixelLine;            //Valid Only When Clut4
			hGfxDC->S0PixelNumOneLine;
			hGfxDC->S0TotalLineNum;	
			hGfxDC->S0ByteEndian;
			hGfxDC->S0NibbleEndian;
			*/
			hGfxDC->S0ClutEnable     = 0;
			hGfxDC->S0ColorKeyEnable = 0;
			/*
			hGfxDC->S0ColorKeyMin;
			hGfxDC->S0ColorKeyMax;
			*/
		}
		else
		{
			hGfxDC->S0Enable       = 0;
			hGfxDC->S0FetchDram    = 0;
		}
	//S1 Control
		if (GfxSrcId == 1)
		{
			hGfxDC->S1Enable       = 1;
			hGfxDC->S1VReverseScan = 0;
			hGfxDC->S1FetchDram    = 0;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			                                     // if S1 Using Default Color then S1ColorFormat should be ARGB565 or A0
			hGfxDC->S1ColorFormat  = GFX_CF_A0;  //Must
			hGfxDC->S1DefaultColor.Alpha = FillColor->Alpha;     //ARGB8888 Format
			hGfxDC->S1DefaultColor.Red   = FillColor->Red;       //ARGB8888 Format
			hGfxDC->S1DefaultColor.Green = FillColor->Green;     //ARGB8888 Format
			hGfxDC->S1DefaultColor.Blue  = FillColor->Blue;      //ARGB8888 Format
			/*
			hGfxDC->S1BaseAddr;                      //Bytes Addr
			hGfxDC->S1LinePitch;                     //Bytes Addr
			hGfxDC->S1SkipPixelLine;            //Valid Only When Clut4
			hGfxDC->S1PixelNumOneLine;
			hGfxDC->S1TotalLineNum;	
			hGfxDC->S1ByteEndian;
			hGfxDC->S1NibbleEndian;
			*/
			hGfxDC->S1ClutEnable     = 0;
			hGfxDC->S1ColorKeyEnable = 0;
			/*
			hGfxDC->S1ColorKeyMin;
			hGfxDC->S1ColorKeyMax;
			*/
		}
		else
		{
			hGfxDC->S1Enable       = 0;
			hGfxDC->S1FetchDram    = 0; 			
		}

	    //ROP and Compositor
		hGfxDC->CompositorEnable   = 0;
		hGfxDC->ROPAlphaCtrl       = 1;
		hGfxDC->RopValue = RopValue;
		//hGfxDC->S0OnTopS1;                        // 1 S0 On Top S1
		//D Dram Para		
		CalcuDramPara
		(
		    DesImg, 
		    DesRect,	
		    hGfxDC->DVReverseScan = 0,
			&hGfxDC->DBaseAddr,              //Bytes Addr
			&hGfxDC->DLinePitch,             //Bytes Addr
			&hGfxDC->DSkipPixelLine         //Valid Only When Clut4
		);
		hGfxDC->DPixelNumOneLine    = DesRect->right  - DesRect->left;;
		hGfxDC->DTotalLineNum       = DesRect->bottom - DesRect->top;
		
		hGfxDC->DColorFormat        = DesImg->ColorFormat;
		hGfxDC->DByteEndian         = DesImg->ByteEndian;
		hGfxDC->DNibbleEndian       = DesImg->NibbleEndian;	
		hGfxDC->D16BitEndian        = DesImg->TwoBytesEndian;

		//Scalor Control
		hGfxDC->S0ScalorEnable  = 0;
		//hGfxDC->HInitialPhase   = 0;
		//hGfxDC->VInitialPhase   = 0;
		hGfxDC->UpdateHFIRCoeff = 0;
		hGfxDC->UpdateVFIRCoeff = 0;

		//Clut para
		hGfxDC->UpdateClut4Table = 0;
		hGfxDC->UpdateClut8Table = 0;
		//RunTimeControl
		hGfxDC->InterruptEnable  = GFX_INTERRUPT_MODE;
		
		GfxTVDImgInitDramData(DesImg);
		//active DC
		if (!hGfxAPIActiveDeviceContext(hGfxDC, GFX_DC_ALL))
		{
			hResult = HG2D_FAILED_ACTIVE_DEVICE_CONTEXT;
		}
		else
		{
			GfxTVS0ImgDramData(NULL);
			GfxTVS1ImgDramData(NULL);
			GfxTVDImgDramData(DesImg);
			hResult = HG2D_SUCCESS;
		}
		
		//Waitting for Idiel
		return hResult;
	}
}
//Copy 
HG2D_RESULT hGfx2DCopy
( 
    GFX_DEVICE_CONTEXT  *hGfxDC, 
    I32                  GfxSrcId,
    COLOR_IMG           *SrcImg, 
    RECT                *SrcRect, 
    I32                  SrcVReverse,
    COLOR_IMG           *DesImg, 
    RECT                *DesRect, 
    I32                  DesVReverse,
    ROP_OPT              RopValue 
)
{
	if (hGfxDC  == NULL ||
		SrcImg  == NULL ||
		SrcRect == NULL ||
		DesImg  == NULL ||
		DesRect == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else if (!IsColorFormatValid(SrcImg->ColorFormat) || //Scalor RTL S0 Support ARGB4444 and RGB565 Only
		     !IsColorFormatValid(SrcImg->ColorFormat)  )
	{
		return HG2D_UNSUPPORTED_COLOR_FORMAT;
	}
	else if (!IsRectValid(SrcImg, SrcRect))
	{
		return HG2D_INVALID_SRC_REC_VALUE;
	}
	else if (!IsRectValid(DesImg, DesRect))
	{
		return HG2D_INVALID_DES_REC_VALUE;
	}
	else if (!CmpRect(SrcRect, DesRect))//Check SrcRect and DesRect
	{
		return HG2D_DIFFERENT_SRC_DES_RECT;
	}
	else
	{
		HG2D_RESULT hResult = HG2D_UNKNOW;
	//S0 Control
		if (GfxSrcId == 0)
		{
			hGfxDC->S0Enable       = 1;
			hGfxDC->S0VReverseScan = SrcVReverse;
			hGfxDC->S0FetchDram    = 1;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			                                     // if S0 Using Default Color then S0ColorFormat should be ARGB565 or A0
			hGfxDC->S0ColorFormat  = SrcImg->ColorFormat;  //Must
			hGfxDC->S0DefaultColor.Alpha = 0xFF;     //ARGB8888 Format, For Copy Don't change Source Alpha
			/*
			hGfxDC->S0DefaultColor.Red   = SrcImg->DefaultColor->Red;       //ARGB8888 Format
			hGfxDC->S0DefaultColor.Green = SrcImg->DefaultColor->Green;     //ARGB8888 Format
			hGfxDC->S0DefaultColor.Blue  = SrcImg->DefaultColor->Blue;      //ARGB8888 Format
			*/
			CalcuDramPara
			(
			    SrcImg, 
			    SrcRect,	
			    hGfxDC->S0VReverseScan,
				&hGfxDC->S0BaseAddr,              //Bytes Addr
				&hGfxDC->S0LinePitch,             //Bytes Addr
				&hGfxDC->S0SkipPixelLine         //Valid Only When Clut4
			);
			hGfxDC->S0PixelNumOneLine    = SrcRect->right  - SrcRect->left;;
			hGfxDC->S0TotalLineNum       = SrcRect->bottom - SrcRect->top;
			hGfxDC->S0ByteEndian   = SrcImg->ByteEndian;
			hGfxDC->S0NibbleEndian = SrcImg->NibbleEndian;
			hGfxDC->S016BitEndian  = SrcImg->TwoBytesEndian;

			hGfxDC->S0ClutEnable     = (SrcImg->ColorFormat == GFX_CF_CLUT4 ||
				                        SrcImg->ColorFormat == GFX_CF_CLUT8 );
			hGfxDC->S0ColorKeyEnable = 0;
			/*
			hGfxDC->S0ColorKeyMin;
			hGfxDC->S0ColorKeyMax;
			*/
		}
		else
		{
			hGfxDC->S0Enable       = 0;
			hGfxDC->S0FetchDram    = 0;
		}
	//S1 Control
		if (GfxSrcId == 1)
		{
			hGfxDC->S1Enable       = 1;
			hGfxDC->S1VReverseScan = SrcVReverse;
			hGfxDC->S1FetchDram    = 1;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			                                     // if S1 Using Default Color then S1ColorFormat should be ARGB565 or A0
			hGfxDC->S1ColorFormat  = SrcImg->ColorFormat;  //Must
			hGfxDC->S1DefaultColor.Alpha = 0xFF;     //ARGB8888 Format, For Copy Don't change Source Alpha
			/*
			hGfxDC->S1DefaultColor.Red   = SrcImg->DefaultColor->Red;       //ARGB8888 Format
			hGfxDC->S1DefaultColor.Green = SrcImg->DefaultColor->Green;     //ARGB8888 Format
			hGfxDC->S1DefaultColor.Blue  = SrcImg->DefaultColor->Blue;      //ARGB8888 Format
			*/
			CalcuDramPara
			(
			    SrcImg, 
			    SrcRect,	
			    hGfxDC->S1VReverseScan,
				&hGfxDC->S1BaseAddr,              //Bytes Addr
				&hGfxDC->S1LinePitch,             //Bytes Addr
				&hGfxDC->S1SkipPixelLine         //Valid Only When Clut4
			);
			hGfxDC->S1PixelNumOneLine    = SrcRect->right  - SrcRect->left;;
			hGfxDC->S1TotalLineNum       = SrcRect->bottom - SrcRect->top;
			hGfxDC->S1ByteEndian   = SrcImg->ByteEndian;
			hGfxDC->S1NibbleEndian = SrcImg->NibbleEndian;
			hGfxDC->S116BitEndian  = SrcImg->TwoBytesEndian;

			hGfxDC->S1ClutEnable     = (SrcImg->ColorFormat == GFX_CF_CLUT4 ||
				                        SrcImg->ColorFormat == GFX_CF_CLUT8 );
			hGfxDC->S1ColorKeyEnable = 0;
			/*
			hGfxDC->S1ColorKeyMin;
			hGfxDC->S1ColorKeyMax;
			*/
		}
		else
		{
			hGfxDC->S1Enable       = 0;
			hGfxDC->S1FetchDram    = 0;
		}

	    //ROP and Compositor
		hGfxDC->CompositorEnable   = 0;
		hGfxDC->ROPAlphaCtrl       = 1;
		hGfxDC->RopValue           = RopValue;
		//hGfxDC->S0OnTopS1;                        // 1 S0 On Top S1
		//D Dram Para		
		CalcuDramPara
		(
		    DesImg, 
		    DesRect,	
		    hGfxDC->DVReverseScan = DesVReverse,
			&hGfxDC->DBaseAddr,              //Bytes Addr
			&hGfxDC->DLinePitch,             //Bytes Addr
			&hGfxDC->DSkipPixelLine         //Valid Only When Clut4
		);
		hGfxDC->DPixelNumOneLine    = DesRect->right  - DesRect->left;;
		hGfxDC->DTotalLineNum       = DesRect->bottom - DesRect->top;
		
		hGfxDC->DColorFormat        = DesImg->ColorFormat;
		hGfxDC->DByteEndian         = DesImg->ByteEndian;
		hGfxDC->DNibbleEndian       = DesImg->NibbleEndian;
		hGfxDC->D16BitEndian        = DesImg->TwoBytesEndian;

		//Scalor Control
		hGfxDC->S0ScalorEnable   = 0;
		//hGfxDC->HInitialPhase   = 0;
		//hGfxDC->VInitialPhase   = 0;
		hGfxDC->UpdateHFIRCoeff  = 0;
		hGfxDC->UpdateVFIRCoeff  = 0;

		//Clut para
		hGfxDC->UpdateClut4Table = 0;
		hGfxDC->UpdateClut8Table = 0;
		//RunTimeControl
		hGfxDC->InterruptEnable  = GFX_INTERRUPT_MODE;
		
		//active DC
		GfxTVDImgInitDramData(DesImg);
		
		if (!hGfxAPIActiveDeviceContext(hGfxDC, GFX_DC_ALL))
		{
			hResult = HG2D_FAILED_ACTIVE_DEVICE_CONTEXT;
		}
		else
		{
			if (GfxSrcId == 0)
			{
				GfxTVS0ImgDramData(SrcImg);
				GfxTVS1ImgDramData(NULL);
			}
			else
			{
				GfxTVS0ImgDramData(NULL);
				GfxTVS1ImgDramData(SrcImg);
			}
			GfxTVDImgDramData(DesImg);
			hResult = HG2D_SUCCESS;
		}
		return hResult;
	}	
}

//Scalor and Anti-flicker
//Coeff

U32 SCALOR_COEFF( U32 SignBit, U32 Nmerator, U32 denominator) 
{
	return (((SignBit) << SCALOR_COEFF_POLARITY_BIT)  | \
		   ((((Nmerator)) << SCALOR_COEFF_FRACTION_BIT_WIDTH ) / \
		   (denominator)));
}

HG2D_RESULT hGfx2DInitScalorVFIRCoeff(GFX_DEVICE_CONTEXT *hGfxDC, U32 *VFIRCoeff)
{
	if (hGfxDC == NULL || VFIRCoeff == NULL) 
	{
		return HG2D_INVALID_PARA;
	}
	else
	{
		HG2D_RESULT hResult = HG2D_UNKNOW;
		U32 *p = NULL;
		for (p = hGfxDC->VFIRCoeffTable[0];
		     p < hGfxDC->VFIRCoeffTable[0] + SCALOR_PAHSE_NUM * SCALOR_V_FIR_TAP_NUM;
			 p++)
		{
			*p = *VFIRCoeff++;
		}
		hGfxDC->UpdateVFIRCoeff = 1;
		if (!hGfxAPIActiveDeviceContext(hGfxDC, GFX_DC_SCALOR_VFIR_COFF))
		{
			hResult = HG2D_FAILED_ACTIVE_DEVICE_CONTEXT;
		}
		else
		{	
			hResult = HG2D_SUCCESS;
		}
		hGfxDC->UpdateVFIRCoeff = 0;

		return hResult;
	}	
}
U32 *hGfx2DGetDefaultScalorVFIRCoeff()
{
	static U32 VFIRCoeffTable[SCALOR_PAHSE_NUM][SCALOR_V_FIR_TAP_NUM];
	U32 Phase = 0;
	assert(SCALOR_V_FIR_TAP_NUM == 2);
	for (Phase = 0 ; Phase < SCALOR_PAHSE_NUM; Phase++)
	{
		VFIRCoeffTable[Phase][0] = SCALOR_COEFF( 0, (SCALOR_PAHSE_NUM - Phase), SCALOR_PAHSE_NUM);
		VFIRCoeffTable[Phase][1] = SCALOR_COEFF( 0, (Phase), SCALOR_PAHSE_NUM);
	}
	return VFIRCoeffTable[0];	
}

HG2D_RESULT hGfx2DSetDefaultScalorVFIRCoeff(GFX_DEVICE_CONTEXT *hGfxDC)
{	
	#if (ORION_VERSION == ORION_130)
	return HG2D_SUCCESS;
	#endif

	if (hGfxDC == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else//Scalor Coeffecient Setting
	{
		HG2D_RESULT hResult = HG2D_UNKNOW;
		U32 *VFIRCoeff = NULL;
		VFIRCoeff = hGfx2DGetDefaultScalorVFIRCoeff();
		if (VFIRCoeff == NULL)
		{
			return HG2D_FAILED_TO_GET_DEFAULT_VFIR_COEFF;
		}
		hResult   = hGfx2DInitScalorVFIRCoeff(hGfxDC, VFIRCoeff);
		return hResult;
	}
}
HG2D_RESULT hGfx2DInitScalorHFIRCoeff(GFX_DEVICE_CONTEXT *hGfxDC, U32 *HFIRCoeff)
{
	if (hGfxDC == NULL || HFIRCoeff == NULL) 
	{
		return HG2D_INVALID_PARA;
	}
	else
	{
		HG2D_RESULT hResult = HG2D_UNKNOW;
		U32 *p = NULL;
		for (p = hGfxDC->HFIRCoeffTable[0];
		     p < hGfxDC->HFIRCoeffTable[0] + SCALOR_PAHSE_NUM * SCALOR_H_FIR_TAP_NUM;
			 p++)
		{
			*p = *HFIRCoeff++;
		}
		hGfxDC->UpdateHFIRCoeff = 1;
		if (!hGfxAPIActiveDeviceContext(hGfxDC, GFX_DC_SCALOR_HFIR_COFF))
		{
			hResult = HG2D_FAILED_ACTIVE_DEVICE_CONTEXT;
		}
		else
		{	
			hResult = HG2D_SUCCESS;
		}
		hGfxDC->UpdateHFIRCoeff = 0;
		return hResult;
	}
}
U32 *hGfx2DGetDefaultScalorHFIRCoeff()
{
	static U32 HFIRCoeffTable[SCALOR_PAHSE_NUM][SCALOR_H_FIR_TAP_NUM];
	U32 Phase = 0;
	assert(SCALOR_H_FIR_TAP_NUM == 4);
	for (Phase = 0 ; Phase < SCALOR_PAHSE_NUM; Phase++)
	{
		HFIRCoeffTable[Phase][0] = 0;
		HFIRCoeffTable[Phase][1] = SCALOR_COEFF( 0, (SCALOR_PAHSE_NUM - Phase), SCALOR_PAHSE_NUM);
		HFIRCoeffTable[Phase][2] = SCALOR_COEFF( 0, (Phase), SCALOR_PAHSE_NUM);
		HFIRCoeffTable[Phase][3] = 0;
	}
	return HFIRCoeffTable[0];
}

HG2D_RESULT hGfx2DSetDefaultScalorHFIRCoeff(GFX_DEVICE_CONTEXT *hGfxDC)
{
	#if (ORION_VERSION == ORION_130)
	return HG2D_SUCCESS;
	#endif
	
	if (hGfxDC == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else //Scalor Coeffecient Setting
	{
		HG2D_RESULT hResult = HG2D_UNKNOW;
		U32 *HFIRCoeff = NULL;
		HFIRCoeff = hGfx2DGetDefaultScalorHFIRCoeff();
		if (HFIRCoeff == NULL)
		{
			return HG2D_FAILED_TO_GET_DEFAULT_HFIR_COEFF;
		}
		hResult   = hGfx2DInitScalorHFIRCoeff(hGfxDC, HFIRCoeff);
		
		return hResult;
	}
}


HG2D_RESULT hGfx2DScalor
( 
    GFX_DEVICE_CONTEXT  *hGfxDC, 
    COLOR_IMG           *SrcImg, 
    RECT                *SrcRect, 
    I32                  SrcVReverse,
    COLOR_IMG           *DesImg, 
    RECT                *DesRect, 
    I32                  DesVReverse,
    ROP_OPT              RopValue, 
    U32                  VInitialPhase, 
    U32                  HInitialPhase
)
{
	if (hGfxDC  == NULL ||
		SrcImg  == NULL ||
		SrcRect == NULL ||
		DesImg  == NULL ||
		DesRect == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else if ((SrcImg->ColorFormat != GFX_CF_ARGB1555) &&
		     (SrcImg->ColorFormat != GFX_CF_RGB565) &&
		     (SrcImg->ColorFormat != GFX_CF_ARGB4444))
	{
		return HG2D_UNSUPPORTED_COLOR_FORMAT;
	}
	else if ((DesImg->ColorFormat != GFX_CF_ARGB1555) &&
		     (DesImg->ColorFormat != GFX_CF_RGB565) &&
		     (DesImg->ColorFormat != GFX_CF_ARGB4444))
	{
		return HG2D_UNSUPPORTED_COLOR_FORMAT;
	}
	else if (!IsRectValid(SrcImg, SrcRect))
	{
		return HG2D_INVALID_SRC_REC_VALUE;
	}
	else if (!IsRectValid(DesImg, DesRect))
	{
		return HG2D_INVALID_DES_REC_VALUE;
	}
	else
	{
		HG2D_RESULT hResult = HG2D_UNKNOW;
		//S0 Control
		{
			hGfxDC->S0Enable       = 1;
			hGfxDC->S0VReverseScan = SrcVReverse;
			hGfxDC->S0FetchDram    = 1;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			                                     // if S0 Using Default Color then S0ColorFormat should be ARGB565 or A0
			hGfxDC->S0ColorFormat  = SrcImg->ColorFormat;  //Must
			hGfxDC->S0DefaultColor.Alpha = SrcImg->DefaultColor.Alpha;     //ARGB8888 Format
			/*
			hGfxDC->S0DefaultColor.Red   = SrcImg->DefaultColor->Red;       //ARGB8888 Format
			hGfxDC->S0DefaultColor.Green = SrcImg->DefaultColor->Green;     //ARGB8888 Format
			hGfxDC->S0DefaultColor.Blue  = SrcImg->DefaultColor->Blue;      //ARGB8888 Format
			*/
			CalcuDramPara
			(
			    SrcImg, 
			    SrcRect,	
			    hGfxDC->S0VReverseScan,
				&hGfxDC->S0BaseAddr,              //Bytes Addr
				&hGfxDC->S0LinePitch,             //Bytes Addr
				&hGfxDC->S0SkipPixelLine         //Valid Only When Clut4
			);
			hGfxDC->S0PixelNumOneLine    = SrcRect->right  - SrcRect->left;;
			hGfxDC->S0TotalLineNum       = SrcRect->bottom - SrcRect->top;
			hGfxDC->S0ByteEndian   = SrcImg->ByteEndian;
			hGfxDC->S0NibbleEndian = SrcImg->NibbleEndian;
			hGfxDC->S016BitEndian  = SrcImg->TwoBytesEndian;
			
			hGfxDC->S0ClutEnable     = 0; //Do not support Clut Type In Scalor
			hGfxDC->S0ColorKeyEnable = 0; 
			/*
			hGfxDC->S0ColorKeyMin;
			hGfxDC->S0ColorKeyMax;
			*/
		}
		//S1 Control
		{
			hGfxDC->S1Enable       = 0;
			hGfxDC->S1FetchDram    = 0;
		}

	    //ROP and Compositor
		hGfxDC->CompositorEnable   = 0;
		hGfxDC->ROPAlphaCtrl       = 1;
		hGfxDC->RopValue = RopValue;		
		//hGfxDC->S0OnTopS1;                        // 1 S0 On Top S1
		//D Dram Para		
		CalcuDramPara
		(
		    DesImg, 
		    DesRect,	
		    hGfxDC->DVReverseScan = DesVReverse,
			&hGfxDC->DBaseAddr,              //Bytes Addr
			&hGfxDC->DLinePitch,             //Bytes Addr
			&hGfxDC->DSkipPixelLine          //Valid Only When Clut4
		);
		hGfxDC->DPixelNumOneLine    = DesRect->right  - DesRect->left;;
		hGfxDC->DTotalLineNum       = DesRect->bottom - DesRect->top;
		
		hGfxDC->DColorFormat        = DesImg->ColorFormat;
		hGfxDC->DByteEndian         = DesImg->ByteEndian;
		hGfxDC->DNibbleEndian       = DesImg->NibbleEndian;		
		hGfxDC->D16BitEndian        = DesImg->TwoBytesEndian;
		//Scalor Control
		hGfxDC->S0ScalorEnable   = 1;
		hGfxDC->HInitialPhase    = HInitialPhase;
		hGfxDC->VInitialPhase    = VInitialPhase;
		hGfxDC->UpdateHFIRCoeff  = 0;
		hGfxDC->UpdateVFIRCoeff  = 0;

		//Clut para
		hGfxDC->UpdateClut4Table = 0;
		hGfxDC->UpdateClut8Table = 0;
		//RunTimeControl
		hGfxDC->InterruptEnable  = GFX_INTERRUPT_MODE;
		GfxTVDImgInitDramData(DesImg);
		//active DC
		hGfxAPIWaitGfxToBeIdle(NULL); //Wait the Gfx layer Idel Before Scalor, Must
		if (!hGfxAPIActiveDeviceContext(hGfxDC, GFX_DC_ALL))
		{
			hResult = HG2D_FAILED_ACTIVE_DEVICE_CONTEXT;
		}
		else
		{
			GfxTVS0ImgDramData(SrcImg);
			GfxTVS1ImgDramData(NULL);
			GfxTVDImgDramData(DesImg);
			hResult = HG2D_SUCCESS;
		}
		return hResult;	
	}	
}

HG2D_RESULT hGfx2DBLT
(
	 GFX_DEVICE_CONTEXT *hGfxDC, 
     HGFX_BLT_SRC_PARA  *Src0Para,
     HGFX_BLT_SRC_PARA  *Src1Para, 
     HGFX_BLT_DES_PARA  *DesPara
)
{
	if (hGfxDC  == NULL ||
		Src0Para  == NULL ||
		Src1Para  == NULL ||
		DesPara == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else if ( Src0Para->Img  == NULL || 
		      Src0Para->Rect == NULL ||
		      Src1Para->Img  == NULL || 
		      Src1Para->Rect == NULL ||
		      DesPara->Img   == NULL ||
		      DesPara->Rect  == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else if (!IsColorFormatValid(Src0Para->Img->ColorFormat) || 
	         !IsColorFormatValid(Src1Para->Img->ColorFormat) || 
	         !IsColorFormatValid(DesPara->Img->ColorFormat)  )
	{
		return HG2D_UNSUPPORTED_COLOR_FORMAT;
	}
	else if (!IsRectValid(Src0Para->Img, Src0Para->Rect) || 
		     !IsRectValid(Src1Para->Img, Src1Para->Rect) )
	{
		return HG2D_INVALID_SRC_REC_VALUE;
	}
	else if (!IsRectValid(DesPara->Img,  DesPara->Rect) )
	{
		return HG2D_INVALID_DES_REC_VALUE;
	}
	else if (!CmpRect(Src0Para->Rect, Src1Para->Rect) || 
		     !CmpRect(Src0Para->Rect, DesPara->Rect)  )
	{
		return HG2D_DIFFERENT_SRC_DES_RECT;
	}
	else
	{
		HG2D_RESULT hResult = HG2D_UNKNOW;
		COLOR_IMG *S0Img = NULL, *S1Img = NULL, *DImg = NULL;
		S0Img = Src0Para->Img;
		S1Img = Src1Para->Img;
		DImg  = DesPara->Img;
	//S0 Control
		{
			hGfxDC->S0Enable       = 1;
			hGfxDC->S0VReverseScan = Src0Para->VReverse;
			hGfxDC->S0FetchDram    = 1;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			                                     // if S0 Using Default Color then S0ColorFormat should be ARGB565 or A0
			hGfxDC->S0ColorFormat  = S0Img->ColorFormat;  //Must
			hGfxDC->S0DefaultColor.Alpha = S0Img->DefaultColor.Alpha;     //ARGB8888 Format
			/*
			hGfxDC->S0DefaultColor.Red   = SrcImg->DefaultColor->Red;       //ARGB8888 Format
			hGfxDC->S0DefaultColor.Green = SrcImg->DefaultColor->Green;     //ARGB8888 Format
			hGfxDC->S0DefaultColor.Blue  = SrcImg->DefaultColor->Blue;      //ARGB8888 Format
			*/
			CalcuDramPara
			(
			    S0Img, 
			    Src0Para->Rect,	
			    hGfxDC->S0VReverseScan,
				&hGfxDC->S0BaseAddr,              //Bytes Addr
				&hGfxDC->S0LinePitch,             //Bytes Addr
				&hGfxDC->S0SkipPixelLine         //Valid Only When Clut4
			);
			hGfxDC->S0PixelNumOneLine    = Src0Para->Rect->right  - Src0Para->Rect->left;;
			hGfxDC->S0TotalLineNum       = Src0Para->Rect->bottom - Src0Para->Rect->top;
			hGfxDC->S0ByteEndian   = S0Img->ByteEndian;
			hGfxDC->S0NibbleEndian = S0Img->NibbleEndian;
			hGfxDC->S016BitEndian  = S0Img->TwoBytesEndian;

			hGfxDC->S0ClutEnable     = (S0Img->ColorFormat == GFX_CF_CLUT4 || 
				                        S0Img->ColorFormat == GFX_CF_CLUT8 );
			hGfxDC->S0ColorKeyEnable = Src0Para->ColorKeyEnable;
			memcpy(&hGfxDC->S0ColorKeyMin, &Src0Para->MinColor, sizeof(GFX_ARGB_COLOR));
			memcpy(&hGfxDC->S0ColorKeyMax, &Src0Para->MaxColor, sizeof(GFX_ARGB_COLOR));
		}
	//S1 Control
		{
			hGfxDC->S1Enable       = 1;
			hGfxDC->S1VReverseScan = Src1Para->VReverse;
			hGfxDC->S1FetchDram    = 1;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			                                     // if S1 Using Default Color then S1ColorFormat should be ARGB565 or A0
			hGfxDC->S1ColorFormat  = S1Img->ColorFormat;  //Must
			hGfxDC->S1DefaultColor.Alpha = S1Img->DefaultColor.Alpha;     //ARGB8888 Format
			/*
			hGfxDC->S1DefaultColor.Red   = SrcImg->DefaultColor->Red;       //ARGB8888 Format
			hGfxDC->S1DefaultColor.Green = SrcImg->DefaultColor->Green;     //ARGB8888 Format
			hGfxDC->S1DefaultColor.Blue  = SrcImg->DefaultColor->Blue;      //ARGB8888 Format
			*/
			CalcuDramPara
			(
			    S1Img, 
			    Src1Para->Rect,	
			    hGfxDC->S1VReverseScan,
				&hGfxDC->S1BaseAddr,              //Bytes Addr
				&hGfxDC->S1LinePitch,             //Bytes Addr
				&hGfxDC->S1SkipPixelLine         //Valid Only When Clut4
			);
			hGfxDC->S1PixelNumOneLine    = Src1Para->Rect->right  - Src1Para->Rect->left;;
			hGfxDC->S1TotalLineNum       = Src1Para->Rect->bottom - Src1Para->Rect->top;
			hGfxDC->S1ByteEndian   = S1Img->ByteEndian;
			hGfxDC->S1NibbleEndian = S1Img->NibbleEndian;
			hGfxDC->S116BitEndian  = S1Img->TwoBytesEndian;

			hGfxDC->S1ClutEnable     = (S1Img->ColorFormat == GFX_CF_CLUT4 || 
				                        S1Img->ColorFormat == GFX_CF_CLUT8 );
			hGfxDC->S1ColorKeyEnable = Src1Para->ColorKeyEnable;
			memcpy(&hGfxDC->S1ColorKeyMin, &Src1Para->MinColor, sizeof(GFX_ARGB_COLOR));
			memcpy(&hGfxDC->S1ColorKeyMax, &Src1Para->MaxColor, sizeof(GFX_ARGB_COLOR));
		}
	    //ROP and Compositor
		hGfxDC->CompositorEnable   = DesPara->BlendEnable;
		hGfxDC->ROPAlphaCtrl       = DesPara->ROPAlphaCtrl;
		hGfxDC->RopValue           = DesPara->RopValue;		
		hGfxDC->S0OnTopS1          = DesPara->IsS0OnTopS1;                        // 1 S0 On Top S1
		//D Dram Para		
		CalcuDramPara
		(
		    DImg, 
		    DesPara->Rect,	
		    hGfxDC->DVReverseScan = DesPara->VReverse,
			&hGfxDC->DBaseAddr,              //Bytes Addr
			&hGfxDC->DLinePitch,             //Bytes Addr
			&hGfxDC->DSkipPixelLine          //Valid Only When Clut4
		);
		hGfxDC->DPixelNumOneLine    = DesPara->Rect->right  - DesPara->Rect->left;;
		hGfxDC->DTotalLineNum       = DesPara->Rect->bottom - DesPara->Rect->top;
		
		hGfxDC->DColorFormat        = DImg->ColorFormat;
		hGfxDC->DByteEndian         = DImg->ByteEndian;
		hGfxDC->DNibbleEndian       = DImg->NibbleEndian;
		hGfxDC->D16BitEndian       = DImg->TwoBytesEndian;

		//Scalor Control
		hGfxDC->S0ScalorEnable   = 0;
		//hGfxDC->HInitialPhase   = 0;
		//hGfxDC->VInitialPhase   = 0;
		hGfxDC->UpdateHFIRCoeff  = 0;
		hGfxDC->UpdateVFIRCoeff  = 0;

		//Clut para
		hGfxDC->UpdateClut4Table = 0;
		hGfxDC->UpdateClut8Table = 0;
		//RunTimeControl
		hGfxDC->InterruptEnable  = GFX_INTERRUPT_MODE;
		GfxTVDImgInitDramData(DesPara->Img);
		//active DC
		if (!hGfxAPIActiveDeviceContext(hGfxDC, GFX_DC_ALL))
		{
			hResult = HG2D_FAILED_ACTIVE_DEVICE_CONTEXT;
		}
		else
		{
			GfxTVS0ImgDramData(Src0Para->Img);
			GfxTVS1ImgDramData(Src1Para->Img);
			GfxTVDImgDramData(DesPara->Img);
			hResult = HG2D_SUCCESS;
		}
		return hResult;
	}
}

HG2D_RESULT hGfx2DBLTSrc0
(
    GFX_DEVICE_CONTEXT *hGfxDC, 
    HGFX_BLT_SRC_PARA  *Src0Para,
    I32 Src1Enable,
    GFX_ARGB_COLOR     *Src1DefaultColor, 
    HGFX_BLT_DES_PARA  *DesPara
)
{
	if (hGfxDC  == NULL ||
		Src0Para  == NULL ||
		DesPara == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else if ( Src0Para->Img  == NULL || 
		      Src0Para->Rect == NULL ||
		      DesPara->Img   == NULL ||
		      DesPara->Rect  == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else if (!IsColorFormatValid(Src0Para->Img->ColorFormat) || 
		     !IsColorFormatValid(DesPara->Img->ColorFormat)  )
	{
		return HG2D_UNSUPPORTED_COLOR_FORMAT;
	}
	else if (!IsRectValid(Src0Para->Img, Src0Para->Rect))
	{
		return HG2D_INVALID_SRC_REC_VALUE;
	}
	else if (!IsRectValid(DesPara->Img,  DesPara->Rect) )
	{
		return HG2D_INVALID_DES_REC_VALUE;
	}
	else if (!CmpRect(Src0Para->Rect, DesPara->Rect)  )
	{
		return HG2D_DIFFERENT_SRC_DES_RECT;
	}
	else
	{
		HG2D_RESULT hResult = HG2D_UNKNOW;
		COLOR_IMG *S0Img = NULL, *DImg = NULL;
		S0Img = Src0Para->Img;
		DImg  = DesPara->Img;
	//S0 Control
		{
			hGfxDC->S0Enable       = 1;
			hGfxDC->S0VReverseScan = Src0Para->VReverse;
			hGfxDC->S0FetchDram    = 1;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			                                     // if S0 Using Default Color then S0ColorFormat should be ARGB565 or A0
			hGfxDC->S0ColorFormat  = S0Img->ColorFormat;  //Must
			hGfxDC->S0DefaultColor.Alpha = S0Img->DefaultColor.Alpha;     //ARGB8888 Format
			/*
			hGfxDC->S0DefaultColor.Red   = SrcImg->DefaultColor->Red;       //ARGB8888 Format
			hGfxDC->S0DefaultColor.Green = SrcImg->DefaultColor->Green;     //ARGB8888 Format
			hGfxDC->S0DefaultColor.Blue  = SrcImg->DefaultColor->Blue;      //ARGB8888 Format
			*/
			CalcuDramPara
			(
			    S0Img, 
			    Src0Para->Rect,	
			    hGfxDC->S0VReverseScan,
				&hGfxDC->S0BaseAddr,              //Bytes Addr
				&hGfxDC->S0LinePitch,             //Bytes Addr
				&hGfxDC->S0SkipPixelLine         //Valid Only When Clut4
			);
			hGfxDC->S0PixelNumOneLine    = Src0Para->Rect->right  - Src0Para->Rect->left;;
			hGfxDC->S0TotalLineNum       = Src0Para->Rect->bottom - Src0Para->Rect->top;
			hGfxDC->S0ByteEndian   = S0Img->ByteEndian;
			hGfxDC->S0NibbleEndian = S0Img->NibbleEndian;
			hGfxDC->S016BitEndian  = S0Img->TwoBytesEndian;

			hGfxDC->S0ClutEnable     = (S0Img->ColorFormat == GFX_CF_CLUT4 || 
				                        S0Img->ColorFormat == GFX_CF_CLUT8 );
			hGfxDC->S0ColorKeyEnable = Src0Para->ColorKeyEnable;
			memcpy(&hGfxDC->S0ColorKeyMin, &Src0Para->MinColor, sizeof(GFX_ARGB_COLOR));
			memcpy(&hGfxDC->S0ColorKeyMax, &Src0Para->MaxColor, sizeof(GFX_ARGB_COLOR));
		}
	//S1 Control
		{
			hGfxDC->S1Enable       = Src1Enable;
			hGfxDC->S1FetchDram    = 0;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			hGfxDC->S1ColorFormat  = GFX_CF_A0;                                     // if S1 Using Default Color then S1ColorFormat should be ARGB565 or A0
			if (Src1DefaultColor != NULL)
			{
				memcpy(&hGfxDC->S1DefaultColor, Src1DefaultColor, sizeof(GFX_ARGB_COLOR));
			}
			hGfxDC->S1ClutEnable     = 0;
			hGfxDC->S1ColorKeyEnable = 0;
		}
	    //ROP and Compositor
		hGfxDC->CompositorEnable   = DesPara->BlendEnable;
		hGfxDC->ROPAlphaCtrl       = DesPara->ROPAlphaCtrl;
		hGfxDC->RopValue           = DesPara->RopValue;		
		hGfxDC->S0OnTopS1          = DesPara->IsS0OnTopS1;                        // 1 S0 On Top S1
		//D Dram Para		
		CalcuDramPara
		(
		    DImg, 
		    DesPara->Rect,	
		    hGfxDC->DVReverseScan = DesPara->VReverse,
			&hGfxDC->DBaseAddr,              //Bytes Addr
			&hGfxDC->DLinePitch,             //Bytes Addr
			&hGfxDC->DSkipPixelLine         //Valid Only When Clut4
		);
		hGfxDC->DPixelNumOneLine    = DesPara->Rect->right  - DesPara->Rect->left;;
		hGfxDC->DTotalLineNum       = DesPara->Rect->bottom - DesPara->Rect->top;
		
		hGfxDC->DColorFormat        = DImg->ColorFormat;
		hGfxDC->DByteEndian         = DImg->ByteEndian;
		hGfxDC->DNibbleEndian       = DImg->NibbleEndian;		
		hGfxDC->D16BitEndian        = DImg->TwoBytesEndian;

		//Scalor Control
		hGfxDC->S0ScalorEnable   = 0;
		//hGfxDC->HInitialPhase   = 0;
		//hGfxDC->VInitialPhase   = 0;
		hGfxDC->UpdateHFIRCoeff  = 0;
		hGfxDC->UpdateVFIRCoeff  = 0;

		//Clut para
		hGfxDC->UpdateClut4Table = 0;
		hGfxDC->UpdateClut8Table = 0;
		//RunTimeControl
		hGfxDC->InterruptEnable  = GFX_INTERRUPT_MODE;
		
		GfxTVDImgInitDramData(DesPara->Img);
		//active DC
		if (!hGfxAPIActiveDeviceContext(hGfxDC, GFX_DC_ALL))
		{
			hResult = HG2D_FAILED_ACTIVE_DEVICE_CONTEXT;
		}
		else
		{
			GfxTVS0ImgDramData(Src0Para->Img);
			GfxTVS1ImgDramData(NULL);
			GfxTVDImgDramData(DesPara->Img);
			hResult = HG2D_SUCCESS;
		}
		return hResult;
	}

}

HG2D_RESULT hGfx2DBLTSrc1
(
    GFX_DEVICE_CONTEXT *hGfxDC, 
    HGFX_BLT_SRC_PARA  *Src1Para,
    I32 Src0Enable,
    GFX_ARGB_COLOR     *Src0DefaultColor, 
    HGFX_BLT_DES_PARA  *DesPara
)
{
	if (hGfxDC  == NULL ||
		Src1Para  == NULL ||
		DesPara == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else if ( Src1Para->Img  == NULL || 
		      Src1Para->Rect == NULL ||
		      DesPara->Img   == NULL ||
		      DesPara->Rect  == NULL)
	{
		return HG2D_INVALID_PARA;
	}
	else if (!IsColorFormatValid(Src1Para->Img->ColorFormat) || 
		     !IsColorFormatValid(DesPara->Img->ColorFormat)  )
	{
		return HG2D_UNSUPPORTED_COLOR_FORMAT;
	}
	else if (!IsRectValid(Src1Para->Img, Src1Para->Rect))
	{
		return HG2D_INVALID_SRC_REC_VALUE;
	}
	else if (!IsRectValid(DesPara->Img,  DesPara->Rect) )
	{
		return HG2D_INVALID_DES_REC_VALUE;
	}
	else if (!CmpRect(Src1Para->Rect, DesPara->Rect)  )
	{
		return HG2D_DIFFERENT_SRC_DES_RECT;
	}
	else
	{
		HG2D_RESULT hResult = HG2D_UNKNOW;
		COLOR_IMG *S1Img = NULL, *DImg = NULL;
		S1Img = Src1Para->Img;
		DImg  = DesPara->Img;
	//S0 Control
		{
			hGfxDC->S0Enable       = Src0Enable;
			hGfxDC->S0FetchDram    = 0;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			                                     // if S0 Using Default Color then S0ColorFormat should be ARGB565 or A0
			hGfxDC->S0ColorFormat  = GFX_CF_A0;
			if (Src0DefaultColor != NULL)
			{
				memcpy(&hGfxDC->S0DefaultColor, Src0DefaultColor, sizeof(GFX_ARGB_COLOR));
			}
			hGfxDC->S0ClutEnable     = 0;
			hGfxDC->S0ColorKeyEnable = 0;
		}

	//S1 Control
		{
			hGfxDC->S1Enable       = 1;
			hGfxDC->S1VReverseScan = Src1Para->VReverse;
			hGfxDC->S1FetchDram    = 1;          // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
			                                     // if S1 Using Default Color then S1ColorFormat should be ARGB565 or A0
			hGfxDC->S1ColorFormat  = S1Img->ColorFormat;  //Must
			hGfxDC->S1DefaultColor.Alpha = S1Img->DefaultColor.Alpha;     //ARGB8888 Format
			/*
			hGfxDC->S1DefaultColor.Red   = SrcImg->DefaultColor->Red;       //ARGB8888 Format
			hGfxDC->S1DefaultColor.Green = SrcImg->DefaultColor->Green;     //ARGB8888 Format
			hGfxDC->S1DefaultColor.Blue  = SrcImg->DefaultColor->Blue;      //ARGB8888 Format
			*/
			CalcuDramPara
			(
			    S1Img, 
			    Src1Para->Rect,	
			    hGfxDC->S1VReverseScan,
				&hGfxDC->S1BaseAddr,              //Bytes Addr
				&hGfxDC->S1LinePitch,             //Bytes Addr
				&hGfxDC->S1SkipPixelLine         //Valid Only When Clut4
			);
			hGfxDC->S1PixelNumOneLine    = Src1Para->Rect->right  - Src1Para->Rect->left;;
			hGfxDC->S1TotalLineNum       = Src1Para->Rect->bottom - Src1Para->Rect->top;
			hGfxDC->S1ByteEndian   = S1Img->ByteEndian;
			hGfxDC->S1NibbleEndian = S1Img->NibbleEndian;
			hGfxDC->S116BitEndian  = S1Img->TwoBytesEndian;

			hGfxDC->S1ClutEnable     = (S1Img->ColorFormat == GFX_CF_CLUT4 || 
				                        S1Img->ColorFormat == GFX_CF_CLUT8 );
			hGfxDC->S1ColorKeyEnable = Src1Para->ColorKeyEnable;
			memcpy(&hGfxDC->S1ColorKeyMin, &Src1Para->MinColor, sizeof(GFX_ARGB_COLOR));
			memcpy(&hGfxDC->S1ColorKeyMax, &Src1Para->MaxColor, sizeof(GFX_ARGB_COLOR));
		}
	    //ROP and Compositor
		hGfxDC->CompositorEnable   = DesPara->BlendEnable;
		hGfxDC->ROPAlphaCtrl       = DesPara->ROPAlphaCtrl;
		hGfxDC->RopValue           = DesPara->RopValue;		
		hGfxDC->S0OnTopS1          = DesPara->IsS0OnTopS1;                        // 1 S0 On Top S1
		//D Dram Para		
		CalcuDramPara
		(
		    DImg, 
		    DesPara->Rect,	
		    hGfxDC->DVReverseScan = DesPara->VReverse,
			&hGfxDC->DBaseAddr,              //Bytes Addr
			&hGfxDC->DLinePitch,             //Bytes Addr
			&hGfxDC->DSkipPixelLine         //Valid Only When Clut4
		);
		hGfxDC->DPixelNumOneLine    = DesPara->Rect->right  - DesPara->Rect->left;;
		hGfxDC->DTotalLineNum       = DesPara->Rect->bottom - DesPara->Rect->top;
		
		hGfxDC->DColorFormat        = DImg->ColorFormat;
		hGfxDC->DByteEndian         = DImg->ByteEndian;
		hGfxDC->DNibbleEndian       = DImg->NibbleEndian;
		hGfxDC->D16BitEndian        = DImg->TwoBytesEndian;

		//Scalor Control
		hGfxDC->S0ScalorEnable   = 0;
		//hGfxDC->HInitialPhase   = 0;
		//hGfxDC->VInitialPhase   = 0;
		hGfxDC->UpdateHFIRCoeff  = 0;
		hGfxDC->UpdateVFIRCoeff  = 0;

		//Clut para
		hGfxDC->UpdateClut4Table = 0;
		hGfxDC->UpdateClut8Table = 0;
		//RunTimeControl
		hGfxDC->InterruptEnable  = GFX_INTERRUPT_MODE;

		GfxTVDImgInitDramData(DesPara->Img);
		//active DC
		if (!hGfxAPIActiveDeviceContext(hGfxDC, GFX_DC_ALL))
		{
			hResult = HG2D_FAILED_ACTIVE_DEVICE_CONTEXT;
		}
		else
		{
			GfxTVS0ImgDramData(NULL);
			GfxTVS1ImgDramData(Src1Para->Img);
			GfxTVDImgDramData(DesPara->Img);
			hResult = HG2D_SUCCESS;
		}
		return hResult;
	}
}


