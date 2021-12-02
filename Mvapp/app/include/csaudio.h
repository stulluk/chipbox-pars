#ifndef  _CS_APP_AUDIO_H_
#define  _CS_APP_AUDIO_H_

#define CS_AUDIO_MODE_MAX_NUM		3

#define SM_WINDOW_X					160
#define SM_WINDOW_Y					130
#define	SM_WINDOW_DX				460
#define SM_WINDOW_DY				120
#define SM_WINDOW_TITLE_Y			140
#define SM_WINDOW_CONT_Y			180
#define SM_WINDOW_ITEM_X			( SM_WINDOW_X + SM_WINDOW_OUT_OFFSET )
#define SM_WINDOW_ITEM_DX			270
#define SM_WINDOW_ITEM_DY			30
#define SM_WINDOW_ITEM_NAME_X		170
#define SM_WINDOW_OUT_OFFSET		10
#define SM_WINDOW_CONT_DX			440

enum
{
	CS_APP_AUDIO_LANG = 0,
	CS_APP_AUDIO_TRACK,
	CS_APP_AUDIO_SETTING_MAX
};

enum
{
	CS_APP_AUDIO_STEREO = 0,
	CS_APP_AUDIO_LEFT,
	CS_APP_AUDIO_RIGHT,
	CS_APP_AUDIO_MONO,
	CS_APP_AUDIO_MODE_MAX
};

CSAPP_Applet_t	CSApp_Audio(void);
void MV_Draw_Audio_Bar(HDC hdc);
void MV_Draw_AudioMenuBar(HDC hdc, U8 u8Focuskind, U8 esItem);
void MV_Draw_AudioSelectBar(HDC hdc, int y_gap, U8 esItem);
void MV_Audio_SetWindow(HDC hdc);

#endif
