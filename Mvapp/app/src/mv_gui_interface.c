#include "mv_gui_interface.h"


#ifdef MV_PROJECT_MINIGUI

int MV_BitmapFromFile (HDC hdc, PBITMAP bmp, const char* file_name)
{
	return LoadBitmap(hdc, bmp, file_name);
}

void MV_FillBox (HDC hdc, int x, int y, int w, int h)
{
	FillBox (hdc, x, y, w, h);
}

void MV_SetBrushColor (HDC hdc, DWORD mv_color)
{
	SetBrushColor(hdc, mv_color);
}

HDC MV_BeginPaint (HWND hWnd)
{
	return BeginPaint (hWnd);
}

void MV_EndPaint (HWND hWnd, HDC hdc)
{
	EndPaint (hWnd, hdc);
}

BOOL MV_FillBoxWithBitmap (HDC hdc, int x, int y, int w, int h, const BITMAP* bmp)
{
	return FillBoxWithBitmap (hdc, x, y, w, h, bmp);
}

void MV_CS_MW_TextOut( HDC hdc, U32 x, U32 y, char* text )
{
	CS_MW_TextOut( hdc, x, y, text );
}

BOOL MV_CS_MW_SetVideoDefinition(U16  definition_index)
{
	return CS_MW_SetVideoDefinition(definition_index);
}

BOOL MV_GetBitmapFromDC (HDC hdc, int x, int y, int w, int h, BITMAP* bmp)
{
	return GetBitmapFromDC (hdc, x, y, w, h, bmp);
}
#endif

