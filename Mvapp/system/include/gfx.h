#ifndef __GFX_H
#define __GFX_H

#ifdef __cplusplus
extern "C" {
#endif


 typedef enum
{
	CS_GFX_NO_ERROR = 0,
	CS_GFX_ERROR_BAD_PARAMETER,			  
	CS_GFX_ERROR,			 
}tCS_GFX_Error_t;

tCS_GFX_Error_t     CS_GFX_Init(void);
tCS_GFX_Error_t     CS_GFX_SetColor(U8 r, U8 g, U8 b, U8 a);
tCS_GFX_Error_t     CS_GFX_DrawLine(U16 x1, U16 y1, U16 x2, U16 y2);
tCS_GFX_Error_t     CS_GFX_FillRectangle(U16 x, U16 y, U16 w, U16 h);
tCS_GFX_Error_t     CS_GFX_DrawRectangle(U16 x, U16 y, U16 w, U16 h);
tCS_GFX_Error_t     CS_GFX_DrawDot(U16 x, U16 y);

#ifdef __cplusplus
}
#endif

#endif


