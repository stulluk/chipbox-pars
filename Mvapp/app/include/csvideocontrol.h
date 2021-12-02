#ifndef  _CS_APP_VIDEOCONTROL_H_
#define  _CS_APP_VIDEOCONTROL_H_

#define CS_VIDEO_CONTROL_MAX_NUM		3

#define SM_VIDEO_WINDOW_X				160
#define SM_VIDEO_WINDOW_Y				130
#define	SM_VIDEO_WINDOW_DX				460
#define SM_VIDEO_WINDOW_DY				150
#define SM_VIDEO_WINDOW_TITLE_Y			140
#define SM_VIDEO_WINDOW_CONT_Y			180
#define SM_VIDEO_WINDOW_ITEM_X			( SM_VIDEO_WINDOW_X + SM_VIDEO_WINDOW_OUT_OFFSET )
#define SM_VIDEO_WINDOW_ITEM_DY			30
#define SM_VIDEO_WINDOW_ITEM_NAME_X		170
#define SM_VIDEO_WINDOW_OUT_OFFSET		10
#define SM_VIDEO_WINDOW_CONT_DX			440

enum
{
	CS_APP_VIDEO_BRIGHT = 0,
	CS_APP_VIDEO_CONTRAST,
	CS_APP_VIDEO_COLOR,
	CS_APP_VIDEO_SETTING_MAX
};

CSAPP_Applet_t	CSApp_Video_Set(void);
void MV_Draw_Audio_Bar(HDC hdc);
void MV_Draw_Video_SetMenuBar(HDC hdc, U8 u8Focuskind, U8 esItem);
void MV_Draw_Video_SetSelectBar(HDC hdc, int y_gap, U8 esItem);
void MV_Video_Set_SetWindow(HDC hdc);

#endif
