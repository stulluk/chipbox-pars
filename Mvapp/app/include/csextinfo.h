#ifndef  _CS_APP_EXTEND_INFO_H_
#define  _CS_APP_EXTEND_INFO_H_

#define	EXTINFO_LEFT			200
#define	EXTINFO_TOP				200
#define	EXTINFO_WIDTH			880
#define	EXTINFO_HEIGHT			350
#define EXTINFO_TITLE_X			EXTINFO_LEFT
#define EXTINFO_TITLE_Y			EXTINFO_TOP
#define EXTINFO_TITLE_DX		EXTINFO_WIDTH
#define EXTINFO_TITLE_DY		MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmHeight
#define EXTINFO_MAIN_X			(EXTINFO_LEFT + 40)
#define EXTINFO_MAIN_Y			(EXTINFO_TOP + EXTINFO_TITLE_DY)
#define EXTINFO_MAIN_DX			(EXTINFO_TITLE_DX - 80)
#define EXTINFO_MAIN_DY			(EXTINFO_HEIGHT - EXTINFO_TITLE_DY)

#define EXTINFO_CH_NAME_X		(EXTINFO_MAIN_X + 20)
#define EXTINFO_CH_NAME_Y		(EXTINFO_MAIN_Y + 20)
#define EXTINFO_CH_ICON_X		EXTINFO_CH_NAME_X
#define EXTINFO_CH_ICON_Y		(EXTINFO_CH_NAME_Y + MV_INSTALL_MENU_BAR_H)
#define EXTINFO_CH_CAS_Y		(EXTINFO_CH_ICON_Y + MV_INSTALL_MENU_BAR_H + 10)
#define EXTINFO_CH_SAT_Y		(EXTINFO_CH_CAS_Y + MV_INSTALL_MENU_BAR_H)
#define EXTINFO_CH_TP_Y			(EXTINFO_CH_SAT_Y + MV_INSTALL_MENU_BAR_H)
#define EXTINFO_CH_VPID_Y		(EXTINFO_CH_TP_Y + MV_INSTALL_MENU_BAR_H)
#define EXTINFO_CH_APID_Y		(EXTINFO_CH_VPID_Y + MV_INSTALL_MENU_BAR_H)
#define EXTINFO_CH_PPID_Y		(EXTINFO_CH_APID_Y + MV_INSTALL_MENU_BAR_H)
#define EXTINFO_CH_PGMPID_Y		(EXTINFO_CH_PPID_Y + MV_INSTALL_MENU_BAR_H)

#define EXTINFO_SIGNAL_X		(EXTINFO_LEFT + 350)
#define EXTINFO_SIGNAL_Y1		(EXTINFO_CH_PPID_Y + 4)
#define EXTINFO_SIGNAL_Y2		(EXTINFO_SIGNAL_Y1 + MV_INSTALL_MENU_BAR_H)
#define EXTINFO_SIGNAL_DX		450
#define EXTINFO_SIGNAL_DY		(MV_BMP[MVBMP_RED_SIGNAL].bmHeight + 10)

#define BIG_SIGNAL_BACK_X		EXTINFO_CH_NAME_X
#define BIG_SIGNAL_BACK_Y		EXTINFO_CH_NAME_Y
#define BIG_SIGNAL_BACK_DX		(EXTINFO_MAIN_DX - 40)
#define BIG_SIGNAL_BACK_DY		(EXTINFO_MAIN_DY - 40)
#define BIG_SIGNAL_X			(BIG_SIGNAL_BACK_X + 30)
#define BIG_SIGNAL_DX			(BIG_SIGNAL_BACK_DX - 180)
#define BIG_SIGNAL_Y1			(BIG_SIGNAL_BACK_Y + 50)
#define BIG_SIGNAL_DY			50
#define BIG_SIGNAL_Y2			(BIG_SIGNAL_Y1 + BIG_SIGNAL_DY + 50)
#define BIG_SIGNAL_VALUE_X		(BIG_SIGNAL_X + BIG_SIGNAL_DX + 20)

CSAPP_Applet_t CSApp_ExtInfo(void);
void Show_ExtInfo_BIG_Signal(HDC hdc);
void MV_Draw_Extend_Information_Back(HDC hdc);
void MV_Draw_Extend_Information_Channel(HDC hdc);
void Show_ExtInfo_Signal(HDC hdc);
void Show_ExtInfo_BIG_Signal(HDC hdc);
void MV_Draw_Big_Signal_Back(HDC hdc);
void MV_Draw_Big_Signal_Close(HDC hdc);
#endif
/*   E O F  */

