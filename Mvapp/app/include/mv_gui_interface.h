#include "mvapp_def.h"
#include "linuxos.h"
#include "mwsetting.h"
#include "mwsvc.h"
#include "mwlayers.h"

#ifdef MV_PROJECT_MINIGUI
/***************************************************************************************/

int MV_BitmapFromFile (HDC hdc, PBITMAP bmp, const char* file_name);
void MV_FillBox (HDC hdc, int x, int y, int w, int h);
void MV_SetBrushColor (HDC hdc, DWORD mv_color);
HDC MV_BeginPaint (HWND hWnd);
void MV_EndPaint (HWND hWnd, HDC hdc);
BOOL MV_FillBoxWithBitmap (HDC hdc, int x, int y, int w, int h, const BITMAP* bmp);
void MV_CS_MW_TextOut( HDC hdc, U32 x, U32 y, char* text );
BOOL MV_CS_MW_SetVideoDefinition(U16  definition_index);
BOOL MV_GetBitmapFromDC (HDC hdc, int x, int y, int w, int h, BITMAP* bmp);
#endif

