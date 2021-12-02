#include "linuxos.h"

#include "database.h"
#include "timer.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "push.h"
#include "push_data.h"

#define MAX_LEVEL		49

#define S_X				150
#define S_Y				172
#define T_S_Y			( S_Y - 32 )
#define S_T_X			( S_X + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth + 10 )
#define S_T_Y			( S_Y + 340 )
#define BLOCK_XS		40
#define BLOCK_YS		30

U16			f_x = 0, f_y = 0;

U16 		board[16][16], bx[4], by[4], bnx[4], bny[4];
U16 		xmax, xmin, rotate, d, bn, bm, li, bot, max=19, ro=1, pre_bloc, interval, count;
U16 		block_color[8] = {0, 10, 20, 30, 40, 50, 60, 70};

U16 		Level_value=0;
U16 		Max_Level_value=0;
static U16 	move_count=0;
static U16 	clean_back=0;
static U16 	round;

static U16 	backup_count=0;
static U16 	back_x[3];
static U16 	back_y[3];
static U16 	back_value[3];
static U16 	back_kind;
						
static U16	block_location[10][11];
static U16	block_location_org[10][11];
BITMAP		btStage;
BITMAP		btMove;

U16 		sc;

static CSAPP_Applet_t	CSApp_Push_Applets;

static int Push_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void find_pusher(void)
{
	for ( f_y = 0 ; f_y < 10 ; f_y++ )
		for ( f_x = 0 ; f_x < 11 ; f_x++ )
			if ( block_location[f_y][f_x] == PUSH_BLOCK || block_location[f_y][f_x] == PUSH_LOCK_BLOCK )
				return;
}

static void init_data(void)
{
	Level_value = round = (U16)CS_DBU_Get_Push_Game_Level();
}

static void save_data(void)
{
	CS_DBU_Set_Push_Game_Level((U32)Level_value);
}

void gameover(HWND hwnd)
{
	HDC		hdc;

	hdc = BeginPaint(hwnd);
	MV_Draw_Msg_Window(hdc, CSAPP_STR_GOODBYE);
	EndPaint(hwnd,hdc);
	
	if(CS_DBU_CheckIfUserSettingDataChanged())
	{
		CS_DBU_SaveUserSettingDataInHW();
	}
	usleep(500*1000);

	hdc = BeginPaint(hwnd);
	Close_Msg_Window(hdc);
	EndPaint(hwnd,hdc);
}

void Level_loading(void)
{
	U16 i,j;

	for ( i = 0 ; i < 10 ; i++ )
		for ( j = 0 ; j < 11 ; j++ )
			block_location[i][j] = block_location_org[i][j] = block_location_value[Level_value][i][j];
			
	find_pusher();
}

void draw_block(HDC hdc)
{
	U16 i,j;
	
	for(i=0;i<10;i++){
		for ( j=0 ; j < 11 ; j++ )
		{
			switch(block_location[i][j])
			{
				case WALL_BLOCK:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X+(j*BLOCK_XS)), ScalerHeigthPixel(S_Y+(i*BLOCK_YS)), ScalerWidthPixel(BLOCK_XS), ScalerHeigthPixel(BLOCK_YS), &MV_BMP[MVBMP_PUSH_WALL]);
					break;
				case LOCK_BLOCK:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X+(j*BLOCK_XS)), ScalerHeigthPixel(S_Y+(i*BLOCK_YS)), ScalerWidthPixel(BLOCK_XS), ScalerHeigthPixel(BLOCK_YS), &MV_BMP[MVBMP_PUSH_LOCK]);
					break;
				case BOX_BLOCK:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X+(j*BLOCK_XS)), ScalerHeigthPixel(S_Y+(i*BLOCK_YS)), ScalerWidthPixel(BLOCK_XS), ScalerHeigthPixel(BLOCK_YS), &MV_BMP[MVBMP_PUSH_BOX]);
					break;
				case PUSH_BLOCK:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X+(j*BLOCK_XS)), ScalerHeigthPixel(S_Y+(i*BLOCK_YS)), ScalerWidthPixel(BLOCK_XS), ScalerHeigthPixel(BLOCK_YS), &MV_BMP[MVBMP_PUSH_PUSH]);
					break;
				case PUSH_LOCK_BLOCK:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X+(j*BLOCK_XS)), ScalerHeigthPixel(S_Y+(i*BLOCK_YS)), ScalerWidthPixel(BLOCK_XS), ScalerHeigthPixel(BLOCK_YS), &MV_BMP[MVBMP_PUSH_ONLOCK]);
					break;
				case LOCKING_BLOCK:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X+(j*BLOCK_XS)), ScalerHeigthPixel(S_Y+(i*BLOCK_YS)), ScalerWidthPixel(BLOCK_XS), ScalerHeigthPixel(BLOCK_YS), &MV_BMP[MVBMP_PUSH_LOCKED]);
					break;
				default :
					FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X+(j*BLOCK_XS)), ScalerHeigthPixel(S_Y+(i*BLOCK_YS)), ScalerWidthPixel(BLOCK_XS), ScalerHeigthPixel(BLOCK_YS), &MV_BMP[MVBMP_PUSH_SPACE]);
					break;
			}	
		}
	}/* for i */
}

void score_draw(HDC hdc)
{
	char 	text[50];
	RECT	lRect;
	
	MV_SetBrushColor( hdc, ROUND_COLOR );
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	lRect.left = S_X;
	lRect.right = S_X + 150;
	lRect.top = S_Y - 40;
	lRect.bottom = lRect.top + 30;

	if ( btStage.bmHeight == 0 )
	{
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(lRect.left - 4 ),ScalerHeigthPixel(lRect.top - 4), ScalerWidthPixel(lRect.right - lRect.left),ScalerHeigthPixel(30), &btStage);
	} else {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(lRect.left - 4 ),ScalerHeigthPixel(lRect.top - 4), ScalerWidthPixel(lRect.right - lRect.left),ScalerHeigthPixel(30), &btStage);
	}
//	MV_FillBox( hdc, ScalerWidthPixel(lRect.left - 4 ),ScalerHeigthPixel(lRect.top - 4), ScalerWidthPixel(lRect.right - lRect.left),ScalerHeigthPixel(30) );
	sprintf(text, "%s : %d", CS_MW_LoadStringByIdx(CSAPP_STR_SATGE), Level_value + 1);
	CS_MW_DrawText (hdc, text, -1, &lRect, DT_LEFT);
	
	lRect.left = S_X + 440 - 150;
	lRect.right = lRect.left + 150;
	lRect.top = S_Y - 40;
	lRect.bottom = lRect.top + 30;

	if ( btMove.bmHeight == 0 )
	{
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(lRect.left + 4),ScalerHeigthPixel(lRect.top - 4), ScalerWidthPixel(lRect.right - lRect.left),ScalerHeigthPixel(30), &btMove);
	} else {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(lRect.left + 4),ScalerHeigthPixel(lRect.top - 4), ScalerWidthPixel(lRect.right - lRect.left),ScalerHeigthPixel(30), &btMove);
	}
//	MV_FillBox( hdc, ScalerWidthPixel(lRect.left + 4),ScalerHeigthPixel(lRect.top - 4), ScalerWidthPixel(lRect.right - lRect.left),ScalerHeigthPixel(30) );
	sprintf(text, "%s : %d", CS_MW_LoadStringByIdx(CSAPP_STR_MOVE), move_count);
	CS_MW_DrawText (hdc, text, -1, &lRect, DT_RIGHT);
}

void board_draw(HDC hdc)
{
	draw_block(hdc);

	MV_SetBrushColor( hdc, ROUND_COLOR );
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X), ScalerHeigthPixel(S_T_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(S_T_X),ScalerHeigthPixel(S_T_Y), CS_MW_LoadStringByIdx(CSAPP_STR_RESTART));

	FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X), ScalerHeigthPixel(S_T_Y + BLOCK_YS), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_KEY_PREV_ICON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(S_T_X),ScalerHeigthPixel(S_T_Y + BLOCK_YS), CS_MW_LoadStringByIdx(CSAPP_STR_BACK));

	FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X), ScalerHeigthPixel(S_T_Y + BLOCK_YS * 2), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(S_T_X),ScalerHeigthPixel(S_T_Y + BLOCK_YS * 2), CS_MW_LoadStringByIdx(CSAPP_STR_NEXT_STAGE));

	FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X), ScalerHeigthPixel(S_T_Y + BLOCK_YS * 3), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(S_T_X),ScalerHeigthPixel(S_T_Y + BLOCK_YS * 3), CS_MW_LoadStringByIdx(CSAPP_STR_PREV_SATGE));

	score_draw(hdc);

}/* board_draw */

void backup_position(U16 kind)
{
	back_kind = kind;
	
	if ( kind == 1 )
	{
		back_x[0] =f_x;
		back_x[1] =f_x-1;
		back_x[2] =f_x-2;
		
		back_y[0] =f_y;
		back_y[1] =f_y;
		back_y[2] =f_y;
		
		back_value[0] = block_location[f_y][f_x];
		back_value[1] = block_location[f_y][f_x-1];
		back_value[2] = block_location[f_y][f_x-2];
	}
	else if ( kind == 2 )
	{
		back_x[0] =f_x;
		back_x[1] =f_x+1;
		back_x[2] =f_x+2;
		
		back_y[0] =f_y;
		back_y[1] =f_y;
		back_y[2] =f_y;
		
		back_value[0] = block_location[f_y][f_x];
		back_value[1] = block_location[f_y][f_x+1];
		back_value[2] = block_location[f_y][f_x+2];
	}
	else if ( kind == 3 )
	{
		back_x[0] =f_x;
		back_x[1] =f_x;
		back_x[2] =f_x;
		
		back_y[0] =f_y;
		back_y[1] =f_y-1;
		back_y[2] =f_y-2;
		
		back_value[0] = block_location[f_y][f_x];
		back_value[1] = block_location[f_y-1][f_x];
		back_value[2] = block_location[f_y-2][f_x];
	}
	else if ( kind == 4 )
	{
		back_x[0] =f_x;
		back_x[1] =f_x;
		back_x[2] =f_x;
		
		back_y[0] =f_y;
		back_y[1] =f_y+1;
		back_y[2] =f_y+2;
		
		back_value[0] = block_location[f_y][f_x];
		back_value[1] = block_location[f_y+1][f_x];
		back_value[2] = block_location[f_y+2][f_x];
	}
		
	backup_count=1;
}

U16 move_left(void)
{
	if ( f_x == 0 )
		return 0;
		
	if ( block_location[f_y][f_x-1] == SPACE_BLOCK || block_location[f_y][f_x-1] == LOCK_BLOCK )
	{
		backup_position(1);
		if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
			block_location[f_y][f_x] = block_location_org[f_y][f_x];
		else if ( block_location_org[f_y][f_x] == LOCKING_BLOCK )
			block_location[f_y][f_x] = LOCK_BLOCK;
		else
			block_location[f_y][f_x] = SPACE_BLOCK;
		
		if ( block_location[f_y][f_x-1] == LOCK_BLOCK )
			block_location[f_y][f_x-1] = PUSH_LOCK_BLOCK;
		else
			block_location[f_y][f_x-1] = PUSH_BLOCK;
		f_x--;
		return 1;
	} else if ( block_location[f_y][f_x-1] == WALL_BLOCK )
		return 0;
	else if ( block_location[f_y][f_x-1] == BOX_BLOCK )
	{
		if ( f_x-1 == 0 )
			return 0;
			
		if ( block_location[f_y][f_x-2] == SPACE_BLOCK || block_location[f_y][f_x-2] == LOCK_BLOCK )
		{
			backup_position(1);
			if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
				block_location[f_y][f_x] = block_location_org[f_y][f_x];
			else
				block_location[f_y][f_x] = SPACE_BLOCK;
				
			if ( block_location[f_y][f_x-1] == LOCK_BLOCK )
				block_location[f_y][f_x-1] = PUSH_LOCK_BLOCK;
			else
				block_location[f_y][f_x-1] = PUSH_BLOCK;
				
			if ( block_location[f_y][f_x-2] == LOCK_BLOCK )
				block_location[f_y][f_x-2] = LOCKING_BLOCK;
			else
				block_location[f_y][f_x-2] = BOX_BLOCK;
			f_x--;
			return 1;
		} else 
			return 0;
	}
	else if ( block_location[f_y][f_x-1] == LOCKING_BLOCK )
	{
		if ( f_x-1 == 0 )
			return 0;
			
		if ( block_location[f_y][f_x-2] == SPACE_BLOCK || block_location[f_y][f_x-2] == LOCK_BLOCK )
		{
			backup_position(1);
			if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
				block_location[f_y][f_x] = block_location_org[f_y][f_x];
			else if ( block_location_org[f_y][f_x] == LOCKING_BLOCK )
				block_location[f_y][f_x] = LOCK_BLOCK;
			else
				block_location[f_y][f_x] = SPACE_BLOCK;
				
			block_location[f_y][f_x-1] = PUSH_LOCK_BLOCK;
				
			if ( block_location[f_y][f_x-2] == LOCK_BLOCK )
				block_location[f_y][f_x-2] = LOCKING_BLOCK;
			else
				block_location[f_y][f_x-2] = BOX_BLOCK;
			f_x--;
			return 1;
		} else 
			return 0;
	}
	return 0;
}

U16 move_right(void)
{
	if ( f_x == 10 )
		return 0;
		
	if ( block_location[f_y][f_x+1] == SPACE_BLOCK || block_location[f_y][f_x+1] == LOCK_BLOCK )
	{
		backup_position(2);
		if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
			block_location[f_y][f_x] = block_location_org[f_y][f_x];
		else if ( block_location_org[f_y][f_x] == LOCKING_BLOCK )
			block_location[f_y][f_x] = LOCK_BLOCK;
		else
			block_location[f_y][f_x] = SPACE_BLOCK;
		
		if ( block_location[f_y][f_x+1] == LOCK_BLOCK )
			block_location[f_y][f_x+1] = PUSH_LOCK_BLOCK;
		else
			block_location[f_y][f_x+1] = PUSH_BLOCK;
		f_x++;
		return 1;
	} else if ( block_location[f_y][f_x+1] == WALL_BLOCK )
		return 0;
	else if ( block_location[f_y][f_x+1] == BOX_BLOCK )
	{
		if ( f_x+1 == 10 )
			return 0;
			
		if ( block_location[f_y][f_x+2] == SPACE_BLOCK || block_location[f_y][f_x+2] == LOCK_BLOCK )
		{
			backup_position(2);
			if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
				block_location[f_y][f_x] = block_location_org[f_y][f_x];
			else
				block_location[f_y][f_x] = SPACE_BLOCK;
				
			if ( block_location[f_y][f_x+1] == LOCK_BLOCK )
				block_location[f_y][f_x+1] = PUSH_LOCK_BLOCK;
			else
				block_location[f_y][f_x+1] = PUSH_BLOCK;
				
			if ( block_location[f_y][f_x+2] == LOCK_BLOCK )
				block_location[f_y][f_x+2] = LOCKING_BLOCK;
			else
				block_location[f_y][f_x+2] = BOX_BLOCK;
			f_x++;
			return 1;
		} else 
			return 0;
	}
	else if ( block_location[f_y][f_x+1] == LOCKING_BLOCK )
	{
		if ( f_x+1 == 10 )
			return 0;
			
		if ( block_location[f_y][f_x+2] == SPACE_BLOCK || block_location[f_y][f_x+2] == LOCK_BLOCK )
		{
			backup_position(2);
			if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
				block_location[f_y][f_x] = block_location_org[f_y][f_x];
			else if ( block_location_org[f_y][f_x] == LOCKING_BLOCK )
				block_location[f_y][f_x] = LOCK_BLOCK;
			else
				block_location[f_y][f_x] = SPACE_BLOCK;
				
			block_location[f_y][f_x+1] = PUSH_LOCK_BLOCK;
				
			if ( block_location[f_y][f_x+2] == LOCK_BLOCK )
				block_location[f_y][f_x+2] = LOCKING_BLOCK;
			else
				block_location[f_y][f_x+2] = BOX_BLOCK;
			f_x++;
			return 1;
		} else 
			return 0;
	}
	return 0;
}

U16 move_up(void)
{
	if ( f_y == 0 )
		return 0;
		
	if ( block_location[f_y-1][f_x] == SPACE_BLOCK || block_location[f_y-1][f_x] == LOCK_BLOCK )
	{
		backup_position(3);
		if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
			block_location[f_y][f_x] = block_location_org[f_y][f_x];
		else if ( block_location_org[f_y][f_x] == LOCKING_BLOCK )
			block_location[f_y][f_x] = LOCK_BLOCK;
		else
			block_location[f_y][f_x] = SPACE_BLOCK;
		
		if ( block_location[f_y-1][f_x] == LOCK_BLOCK )
			block_location[f_y-1][f_x] = PUSH_LOCK_BLOCK;
		else
			block_location[f_y-1][f_x] = PUSH_BLOCK;
		f_y--;
		return 1;
	} else if ( block_location[f_y-1][f_x] == WALL_BLOCK )
		return 0;
	else if ( block_location[f_y-1][f_x] == BOX_BLOCK )
	{
		if ( f_y-1 == 0 )
			return 0;
			
		if ( block_location[f_y-2][f_x] == SPACE_BLOCK || block_location[f_y-2][f_x] == LOCK_BLOCK )
		{
			backup_position(3);
			if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
				block_location[f_y][f_x] = block_location_org[f_y][f_x];
			else
				block_location[f_y][f_x] = SPACE_BLOCK;
				
			if ( block_location[f_y-1][f_x] == LOCK_BLOCK )
				block_location[f_y-1][f_x] = PUSH_LOCK_BLOCK;
			else
				block_location[f_y-1][f_x] = PUSH_BLOCK;
				
			if ( block_location[f_y-2][f_x] == LOCK_BLOCK )
				block_location[f_y-2][f_x] = LOCKING_BLOCK;
			else
				block_location[f_y-2][f_x] = BOX_BLOCK;
			f_y--;
			return 1;
		} else 
			return 0;
	}
	else if ( block_location[f_y-1][f_x] == LOCKING_BLOCK )
	{
		if ( f_y-1 == 0 )
			return 0;
			
		if ( block_location[f_y-2][f_x] == SPACE_BLOCK || block_location[f_y-2][f_x] == LOCK_BLOCK )
		{
			backup_position(3);
			if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
				block_location[f_y][f_x] = block_location_org[f_y][f_x];
			else if ( block_location_org[f_y][f_x] == LOCKING_BLOCK )
				block_location[f_y][f_x] = LOCK_BLOCK;
			else
				block_location[f_y][f_x] = SPACE_BLOCK;
				
			block_location[f_y-1][f_x] = PUSH_LOCK_BLOCK;
				
			if ( block_location[f_y-2][f_x] == LOCK_BLOCK )
				block_location[f_y-2][f_x] = LOCKING_BLOCK;
			else
				block_location[f_y-2][f_x] = BOX_BLOCK;
			f_y--;
			return 1;
		} else 
			return 0;
	}
	return 0;
}

U16 move_down(void)
{
	if ( f_y == 9 )
		return 0;
		
	if ( block_location[f_y+1][f_x] == SPACE_BLOCK || block_location[f_y+1][f_x] == LOCK_BLOCK )
	{
		backup_position(4);
		if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
			block_location[f_y][f_x] = block_location_org[f_y][f_x];
		else if ( block_location_org[f_y][f_x] == LOCKING_BLOCK )
			block_location[f_y][f_x] = LOCK_BLOCK;
		else
			block_location[f_y][f_x] = SPACE_BLOCK;
		
		if ( block_location[f_y+1][f_x] == LOCK_BLOCK )
			block_location[f_y+1][f_x] = PUSH_LOCK_BLOCK;
		else
			block_location[f_y+1][f_x] = PUSH_BLOCK;
		f_y++;
		return 1;
	} else if ( block_location[f_y+1][f_x] == WALL_BLOCK )
		return 0;
	else if ( block_location[f_y+1][f_x] == BOX_BLOCK )
	{
		if ( f_y+1 == 9 )
			return 0;
			
		if ( block_location[f_y+2][f_x] == SPACE_BLOCK || block_location[f_y+2][f_x] == LOCK_BLOCK )
		{
			backup_position(4);
			if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
				block_location[f_y][f_x] = block_location_org[f_y][f_x];
			else
				block_location[f_y][f_x] = SPACE_BLOCK;
				
			if ( block_location[f_y+1][f_x] == LOCK_BLOCK )
				block_location[f_y+1][f_x] = PUSH_LOCK_BLOCK;
			else
				block_location[f_y+1][f_x] = PUSH_BLOCK;
				
			if ( block_location[f_y+2][f_x] == LOCK_BLOCK )
				block_location[f_y+2][f_x] = LOCKING_BLOCK;
			else
				block_location[f_y+2][f_x] = BOX_BLOCK;
			f_y++;
			return 1;
		} else 
			return 0;
	}
	else if ( block_location[f_y+1][f_x] == LOCKING_BLOCK )
	{
		if ( f_y+1 == 9 )
			return 0;
			
		if ( block_location[f_y+2][f_x] == SPACE_BLOCK || block_location[f_y+2][f_x] == LOCK_BLOCK )
		{
			backup_position(4);
			if ( block_location_org[f_y][f_x] == LOCK_BLOCK )
				block_location[f_y][f_x] = block_location_org[f_y][f_x];
			else if ( block_location_org[f_y][f_x] == LOCKING_BLOCK )
				block_location[f_y][f_x] = LOCK_BLOCK;
			else
				block_location[f_y][f_x] = SPACE_BLOCK;
				
			block_location[f_y+1][f_x] = PUSH_LOCK_BLOCK;
				
			if ( block_location[f_y+2][f_x] == LOCK_BLOCK )
				block_location[f_y+2][f_x] = LOCKING_BLOCK;
			else
				block_location[f_y+2][f_x] = BOX_BLOCK;
			f_y++;
			return 1;
		} else 
			return 0;
	}
	return 0;
}

void back_step(void)
{
	clean_back = 1;
	
	block_location[back_y[0]][back_x[0]] = back_value[0];
	block_location[back_y[1]][back_x[1]] = back_value[1];
	block_location[back_y[2]][back_x[2]] = back_value[2];
	
	f_x = back_x[0];
	f_y = back_y[0];
	
	backup_count = 0;
	move_count--;
}

U16 ending_check(void)
{
	U16 i,j;
	
	for ( i = 0 ; i < 10 ; i++ )
		for ( j = 0 ; j < 11 ; j++ )
			if ( block_location[i][j] == LOCK_BLOCK || block_location[i][j] == PUSH_LOCK_BLOCK )
				return 0;
	return 1;
}

CSAPP_Applet_t	CSApp_Push(void)
{
	int						BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG						msg;
  	HWND					hwndMain;
	MAINWINCREATE			CreateInfo;

	CSApp_Push_Applets = CSApp_Applet_Error;
    
#ifdef  Screen_1080
    BASE_X = 0;
    BASE_Y = 0;
    WIDTH  = 1920;
    HEIGHT = 1080;
#else
    BASE_X = 0;
    BASE_Y = 0;
    WIDTH  = ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH);
    HEIGHT = ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT);
#endif
	
	CreateInfo.dwStyle	 = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "game push";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Push_Msg_cb;
	CreateInfo.lx = BASE_X;
	CreateInfo.ty = BASE_Y;
	CreateInfo.rx = BASE_X+WIDTH;
	CreateInfo.by = BASE_Y+HEIGHT;
	CreateInfo.iBkColor = CSAPP_BLACK_COLOR;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = HWND_DESKTOP;
	
	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);
	return CSApp_Push_Applets;
    
}


static int Push_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;
	int					i = 0, j = 0;

	switch(message)
	{
		case MSG_CREATE:
			memset(&btStage, 0x00, sizeof(BITMAP));
			memset(&btMove, 0x00, sizeof(BITMAP));
			init_data();
			Level_loading();
			Max_Level_value = Level_value;
			break;
			
		case MSG_PAINT:
			{
				CS_MW_SetSmallWindow(ScalerWidthPixel(MV_PIG_LEFT),ScalerHeigthPixel(MV_PIG_TOP),ScalerWidthPixel(MV_PIG_DX),ScalerHeigthPixel(MV_PIG_DY));
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_GAME, 0);
				
				hdc=BeginPaint(hwnd);
				MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
				MV_FillBox( hdc, ScalerWidthPixel(S_X - 10), ScalerHeigthPixel(S_Y - 10), ScalerWidthPixel(460), ScalerHeigthPixel(320) );	
				board_draw(hdc);
				EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_KEYDOWN:
			if ( MV_Check_Confirm_Window() == TRUE )
			{
				MV_Confirm_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_Check_YesNo() == TRUE )
						{
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
							
							if ( Level_value == MAX_LEVEL )
								Level_value = 0;
							else
								Level_value++;
					
							save_data();
							
							Level_loading();
							
							move_count = 0;
							backup_count = 0;

							hdc = BeginPaint(hwnd);
							draw_block(hdc);
							score_draw(hdc);
							EndPaint(hwnd,hdc);
							
							if ( Max_Level_value < Level_value )
								Max_Level_value = Level_value;
						} else {
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
							
							gameover(hwnd);
							
							CS_MW_SetNormalWindow();
							CSApp_Push_Applets = CSApp_Applet_PlugIn;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
					} else {
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
						
						if ( Level_value == MAX_LEVEL )
							Level_value = 0;
						else
							Level_value++;
				
						save_data();
						
						Level_loading();
						
						move_count = 0;
						backup_count = 0;

						hdc = BeginPaint(hwnd);
						draw_block(hdc);
						score_draw(hdc);
						EndPaint(hwnd,hdc);
						
						if ( Max_Level_value < Level_value )
							Max_Level_value = Level_value;
					}
				}
				
				if (wparam != CSAPP_KEY_IDLE)
				{
					break;
				}
			}
			
			switch(wparam)
			{
				case CSAPP_KEY_ESC:
					gameover(hwnd);
					CS_MW_SetNormalWindow();
					CSApp_Push_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_MENU:
					gameover(hwnd);
					CS_MW_SetNormalWindow();
					CSApp_Push_Applets = b8Last_App_Status;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_IDLE:
					CSApp_Push_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_ENTER:
					break;

				case CSAPP_KEY_RED:
					if ( Level_value > 0 )
					{
						Level_value--;
						Level_loading();
						
						move_count = 0;
						backup_count = 0;
						
						hdc = BeginPaint(hwnd);
						draw_block(hdc);
						score_draw(hdc);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_BLUE:
					if ( Level_value < MAX_LEVEL && Level_value < Max_Level_value) 
					{
						Level_value++;
						Level_loading();
						
						move_count = 0;
						backup_count = 0;

						hdc = BeginPaint(hwnd);
						draw_block(hdc);
						score_draw(hdc);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_RECALL:
					if ( backup_count == 1 )
					{
						back_step();
						hdc = BeginPaint(hwnd);
						draw_block(hdc);
						score_draw(hdc);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_GREEN:
					for ( i = 0 ; i < 10 ; i++ )
						for ( j = 0 ; j < 11 ; j++ )
							block_location[i][j] = block_location_org[i][j];
					move_count = 0;
					backup_count = 0;
					find_pusher();
					hdc = BeginPaint(hwnd);
					draw_block(hdc);
					score_draw(hdc);
					EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_UP:
					if ( move_up() )
					{
						move_count++;
						hdc = BeginPaint(hwnd);
						draw_block(hdc);
						score_draw(hdc);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_DOWN:
					if ( move_down() )
					{
						move_count++;
						hdc = BeginPaint(hwnd);
						draw_block(hdc);
						score_draw(hdc);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_LEFT:
					if ( move_left() )
					{
						move_count++;
						hdc = BeginPaint(hwnd);
						draw_block(hdc);
						score_draw(hdc);
						EndPaint(hwnd,hdc);
					}					
					break;

				case CSAPP_KEY_RIGHT:
					if ( move_right() )
					{
						move_count++;
						hdc = BeginPaint(hwnd);
						draw_block(hdc);
						score_draw(hdc);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_LIST:
					Level_value++;
					Level_loading();
					
					move_count = 0;
					backup_count = 0;

					hdc = BeginPaint(hwnd);
					draw_block(hdc);
					score_draw(hdc);
					EndPaint(hwnd,hdc);
					break;
					
				default:
					break;
			}

			if (ending_check())
			{
				MV_Set_Confirm(TRUE);
				MV_Draw_Confirm_Window(hwnd, CSAPP_STR_GOOD_NEXT);
			}
			break;

		case MSG_CLOSE:
			UnloadBitmap(&btStage);
			UnloadBitmap(&btMove);
			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			break;
			
		default:
			break;		
	}
	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

