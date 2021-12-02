#include "linuxos.h"

#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "csnetsetting.h"
#include "ui_common.h"
#include <sys_setup.h>
#include <ctype.h>
#include "e2p.h"
#include "mvosapi.h" // by kb : 2010.07.22

//#define MAC_ADDR_CHANGE

#define UnSelect			1
#define Select				2
#define SHELL_SCRIP			"/usr/work0/app/getaddr.sh"
#define DHCP_SCRIP			"/usr/share/udhcpc/default.script"
#define DHCP_SCRIP_A		"/usr/share/udhcpc/default.script.auto"
#define DHCP_SCRIP_M		"/usr/share/udhcpc/default.script.manual"

static CSAPP_Applet_t		CSApp_Network_Applets;

static U32					NetSettingItemIdx[CSAPP_NET_ITEM_MAX]={
								CSAPP_STR_DHCP_SETTING,
								CSAPP_STR_DNS_TYPE,
								CSAPP_STR_IPADDR,
								CSAPP_STR_SUBNET_MASK,
								CSAPP_STR_GATEWAY,
								CSAPP_STR_DNS1,
								CSAPP_STR_DNS2,
								CSAPP_STR_DNS3,
								CSAPP_STR_MAC_ADDRESS,
								CSAPP_STR_PING
							};

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

static U32					DHCP_STR[CSAPP_DHCP_MAX] = {
								CSAPP_STR_USE,
								CSAPP_STR_NOT_USE
							};

static U32					DNS_STR[CSAPP_DNS_MAX] = {
								CSAPP_STR_SEARCH_AUTO,
								CSAPP_STR_SEARCH_MANUAL
							};

stIPStr_Struct				Default_Netmask[IP_ITEM] = {
								{"255"},
								{"255"},
								{"255"},
								{"0"}
							};

char	DHCP_Check_String[CSAPP_DHCP_IP_MAX+1][CSAPP_DHCP_IP_MAX+1] = {
								"ip",
								"mask",
								"gw",
								"dns"
							};

U8	Net_Arrow_Kind[CSAPP_NET_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC
};

U8	Net_Enter_Kind[CSAPP_NET_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_STATIC
};

static U8					u8Current_DHCP = CSAPP_DHCP_ON;
static U8					u8Current_DNS = CSAPP_DNS_AUTO;
static U8					u8Backup_DNS = 0;
static U16					Current_Item = 0;
BITMAP						btIPPad_cap;
BOOL						b8Keypad_Save = FALSE;
BOOL						b8Check_Change_DHCP = FALSE;
static BOOL					b8First_Key_in = TRUE;
static BOOL					b8Modify_flag = FALSE;
static U8					u8Check_DNS_Count = 0;

stIPStr_Struct				acIP_Addr[IP_ITEM];
stIPStr_Struct				acSubnet_Mask[IP_ITEM];
stIPStr_Struct				acGateway[IP_ITEM];
stIPStr_Struct				acDNS1[IP_ITEM];
stIPStr_Struct				acDNS2[IP_ITEM];
stIPStr_Struct				acDNS3[IP_ITEM];

static char					MacAddress[MAC_SIZE];
static char 				ReturnMAC[MAC_SIZE*2];

static char 				Udhcpc_Check_String[CSAPP_UDHCPC_IP_MAX][20] = {
									"ip=",
									"subnet=",
									"serverid="
								};

BOOL Check_Compare_IP(void)
{
	if ( memcmp(&acIP_Addr, &IP_Addr, sizeof(stIPStr_Struct)*IP_ITEM) != 0 )
		return FALSE;

	if ( memcmp(&acSubnet_Mask, &IP_SubNet, sizeof(stIPStr_Struct)*IP_ITEM) != 0 )
		return FALSE;

	if ( memcmp(&acGateway, &IP_Gateway, sizeof(stIPStr_Struct)*IP_ITEM) != 0 )
		return FALSE;

	if ( memcmp(&acDNS1, &IP_DNS1, sizeof(stIPStr_Struct)*IP_ITEM) != 0 )
		return FALSE;

	if ( memcmp(&acDNS2, &IP_DNS2, sizeof(stIPStr_Struct)*IP_ITEM) != 0 )
		return FALSE;

	if ( memcmp(&acDNS3, &IP_DNS3, sizeof(stIPStr_Struct)*IP_ITEM) != 0 )
		return FALSE;

	//printf("\n==== Check_Compare_IP : Return TRUE =====\n");
	return TRUE;
}

BOOL Check_Null_IP(void)
{
	if ( atoi(IP_Addr[0].Re_IP) == 0 && atoi(IP_Addr[1].Re_IP) == 0 && atoi(IP_Addr[2].Re_IP) == 0 )
		return FALSE;

	return TRUE;
}

void MV_Copy_IPData(void)
{
	memcpy(&acIP_Addr, &IP_Addr, sizeof(stIPStr_Struct)*IP_ITEM);
	memcpy(&acSubnet_Mask, &IP_SubNet, sizeof(stIPStr_Struct)*IP_ITEM);
	memcpy(&acGateway, &IP_Gateway, sizeof(stIPStr_Struct)*IP_ITEM);
	memcpy(&acDNS1, &IP_DNS1, sizeof(stIPStr_Struct)*IP_ITEM);
	memcpy(&acDNS2, &IP_DNS2, sizeof(stIPStr_Struct)*IP_ITEM);
	memcpy(&acDNS3, &IP_DNS3, sizeof(stIPStr_Struct)*IP_ITEM);
}

void MV_Copy_IPData_Backup(void)
{
	memcpy(&IP_Addr, &acIP_Addr, sizeof(stIPStr_Struct)*IP_ITEM);
	memcpy(&IP_SubNet, &acSubnet_Mask, sizeof(stIPStr_Struct)*IP_ITEM);
	memcpy(&IP_Gateway, &acGateway, sizeof(stIPStr_Struct)*IP_ITEM);
	memcpy(&IP_DNS1, &acDNS1, sizeof(stIPStr_Struct)*IP_ITEM);
	memcpy(&IP_DNS2, &acDNS2, sizeof(stIPStr_Struct)*IP_ITEM);
	memcpy(&IP_DNS3, &acDNS3, sizeof(stIPStr_Struct)*IP_ITEM);
}

void IPSelected_Key(HWND hwnd, U8 Select_Kind)
{
	char	temp_str[50];
	HDC		hdc;

	hdc = MV_BeginPaint(hwnd);
	
	SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	//printf("\nIPSelected_Key :: IPKeypad_X : %d =====\n\n", IPKeypad_X);
	sprintf(temp_str, "%d", IPKeypad_X );
	
	if ( Select_Kind == UnSelect )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (IPKeypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( INPUT_WINDOW_NUM_ITEMY ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (IPKeypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( INPUT_WINDOW_NUM_ITEMY ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
	}
	
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (IPKeypad_X*KEYBOARD_KEY_OFFSET) + 8),ScalerHeigthPixel(INPUT_WINDOW_NUM_ITEMY + 2), temp_str);
	MV_EndPaint(hwnd,hdc);
}

U8 IPPress_Key(void)
{
	U8	a=0;
	U8 	temp_str[3];

	sprintf(temp_str, "%d",(U8)IPKeypad_X);
	a = temp_str[0];
		
	return a;
}

BOOL UI_IPKeypad_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC				hdc;
	
	switch (u8Key)
    {
        case CSAPP_KEY_LEFT:
        case CSAPP_KEY_UP:
			b8First_Key_in = TRUE;
			
			hdc = MV_BeginPaint(hwnd);
			MV_Net_Draw_IP_Field(hdc, IPFocus_Index, atoi(IP_TempData[IPFocus_Index].Re_IP), UNFOCUS);
			MV_EndPaint(hwnd,hdc);
			
        	if ( IPFocus_Index == 0 )
				IPFocus_Index = IP_ITEM - 1;
    		else
    			IPFocus_Index -= 1;
			
			IPKeypad_Index = 0;
			hdc = MV_BeginPaint(hwnd);
			MV_Net_Draw_IP_Field(hdc, IPFocus_Index, atoi(IP_TempData[IPFocus_Index].Re_IP), FOCUS);
			MV_EndPaint(hwnd,hdc);
        	break;
        case CSAPP_KEY_RIGHT:
        case CSAPP_KEY_DOWN:
		case CSAPP_KEY_ENTER:
			b8First_Key_in = TRUE;
			
        	hdc = MV_BeginPaint(hwnd);
			MV_Net_Draw_IP_Field(hdc, IPFocus_Index, atoi(IP_TempData[IPFocus_Index].Re_IP), UNFOCUS);
			MV_EndPaint(hwnd,hdc);
			
        	if ( IPFocus_Index == IP_ITEM - 1 )
				IPFocus_Index = 0;
    		else
    			IPFocus_Index += 1;

			IPKeypad_Index = 0;
			hdc = MV_BeginPaint(hwnd);
			MV_Net_Draw_IP_Field(hdc, IPFocus_Index, atoi(IP_TempData[IPFocus_Index].Re_IP), FOCUS);
			MV_EndPaint(hwnd,hdc);
        	break;
        case CSAPP_KEY_RED:
        	if ( IPKeypad_Index > 0 && IPKeypad_Index <= u8max_IP_length )
        	{
	    		IP_TempData[IPFocus_Index].Re_IP[IPKeypad_Index-1] = '\0';

				hdc = MV_BeginPaint(hwnd);
				MV_Net_Draw_IP_Field(hdc, IPFocus_Index, atoi(IP_TempData[IPFocus_Index].Re_IP), FOCUS);
				MV_EndPaint(hwnd,hdc);

				IPKeypad_Index--;
	        }
        	break;
		case CSAPP_KEY_GREEN:
				memset(&IP_TempData[IPFocus_Index].Re_IP, 0, IP_LENGTH);

				hdc = MV_BeginPaint(hwnd);
				MV_Net_Draw_IP_Field(hdc, IPFocus_Index, atoi(IP_TempData[IPFocus_Index].Re_IP), FOCUS);
				MV_EndPaint(hwnd,hdc);

				IPKeypad_Index=0;
        	break;
		case CSAPP_KEY_BLUE:
			b8Keypad_Save = TRUE;
			MV_Close_IP_Keypad( hwnd );
        	break;
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			memset(IP_TempData, 0x00, ( sizeof(stIPStr_Struct) * IP_ITEM ));
			MV_Close_IP_Keypad( hwnd );
			return FALSE;
            break;
		case CSAPP_KEY_0:
		case CSAPP_KEY_1:
		case CSAPP_KEY_2:
		case CSAPP_KEY_3:
		case CSAPP_KEY_4:
		case CSAPP_KEY_5:
		case CSAPP_KEY_6:
		case CSAPP_KEY_7:
		case CSAPP_KEY_8:
		case CSAPP_KEY_9:
			{
				if( b8First_Key_in == TRUE )
				{
					memset(IP_TempData[IPFocus_Index].Re_IP, 0x00, IP_LENGTH);
					IPKeypad_Index = 0;
					b8First_Key_in = FALSE;
				}
					
				switch (u8Key)
				{
					case CSAPP_KEY_0:
						IPSelected_Key(hwnd, UnSelect);
						IPKeypad_X = 0;
						IPSelected_Key(hwnd, Select);
						break;
						
					case CSAPP_KEY_1:
						IPSelected_Key(hwnd, UnSelect);
						IPKeypad_X = 1;
						IPSelected_Key(hwnd, Select);
						break;
						
					case CSAPP_KEY_2:
						IPSelected_Key(hwnd, UnSelect);
						IPKeypad_X = 2;
						IPSelected_Key(hwnd, Select);
						break;
						
					case CSAPP_KEY_3:
						IPSelected_Key(hwnd, UnSelect);
						IPKeypad_X = 3;
						IPSelected_Key(hwnd, Select);
						break;
						
					case CSAPP_KEY_4:
						IPSelected_Key(hwnd, UnSelect);
						IPKeypad_X = 4;
						IPSelected_Key(hwnd, Select);
						break;
						
					case CSAPP_KEY_5:
						IPSelected_Key(hwnd, UnSelect);
						IPKeypad_X = 5;
						IPSelected_Key(hwnd, Select);
						break;
						
					case CSAPP_KEY_6:
						IPSelected_Key(hwnd, UnSelect);
						IPKeypad_X = 6;
						IPSelected_Key(hwnd, Select);
						break;
						
					case CSAPP_KEY_7:
						IPSelected_Key(hwnd, UnSelect);
						IPKeypad_X = 7;
						IPSelected_Key(hwnd, Select);
						break;
						
					case CSAPP_KEY_8:
						IPSelected_Key(hwnd, UnSelect);
						IPKeypad_X = 8;
						IPSelected_Key(hwnd, Select);
						break;
						
					case CSAPP_KEY_9:
						IPSelected_Key(hwnd, UnSelect);
						IPKeypad_X = 9;
						IPSelected_Key(hwnd, Select);
						break;   
				}	
				//printf("\n %d ==> %d =============\n", IPKeypad_Index, u8max_IP_length );
				if ( IPKeypad_Index < u8max_IP_length )
				{
					IP_TempData[IPFocus_Index].Re_IP[IPKeypad_Index] = IPPress_Key();
		        	IPKeypad_Index++;
					if ( IPKeypad_Index == u8max_IP_length && IPFocus_Index < IP_ITEM )
					{
						if ( atoi(IP_TempData[IPFocus_Index].Re_IP) > 255 )
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_INVALID_IP);
							EndPaint(hwnd,hdc);
							
							usleep( 1000 * 1000 );
							
							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);

							IPKeypad_Index--;
							IP_TempData[IPFocus_Index].Re_IP[IPKeypad_Index] = '\0';

							hdc = MV_BeginPaint(hwnd);
							MV_Net_Draw_IP_Field(hdc, IPFocus_Index, atoi(IP_TempData[IPFocus_Index].Re_IP), FOCUS);
							MV_EndPaint(hwnd,hdc);
						}
						else
						{
							hdc = MV_BeginPaint(hwnd);
							MV_Net_Draw_IP_Field(hdc, IPFocus_Index, atoi(IP_TempData[IPFocus_Index].Re_IP), UNFOCUS);
							MV_EndPaint(hwnd,hdc);

							IPFocus_Index += 1;
							IPKeypad_Index = 0;
							b8First_Key_in = TRUE;

							hdc = MV_BeginPaint(hwnd);
							MV_Net_Draw_IP_Field(hdc, IPFocus_Index, atoi(IP_TempData[IPFocus_Index].Re_IP), FOCUS);
							MV_EndPaint(hwnd,hdc);
						}
					}
				} else {
					memcpy( &IP_TempData[IPFocus_Index].Re_IP[0], &IP_TempData[IPFocus_Index].Re_IP[1], IP_LENGTH - 1 );
					IP_TempData[IPFocus_Index].Re_IP[IPKeypad_Index-1] = IPPress_Key();					
				}
				
				hdc = MV_BeginPaint(hwnd);
				MV_Net_Draw_IP_Field(hdc, IPFocus_Index, atoi(IP_TempData[IPFocus_Index].Re_IP), FOCUS);
				MV_EndPaint(hwnd,hdc);
			}
			break;
    }
	return TRUE;
}

void MV_Net_Draw_IP_Field(HDC hdc, U8 u8FiledNum, U16 u16IpNum, U8 u8Focus)
{
	RECT	TmpRect;
	char	temp_str[10];
	
	TmpRect.left	= ( INPUT_WINDOW_NUM_STARTX - 5 ) + ( 75 * u8FiledNum ) + 5;
	TmpRect.right	= TmpRect.left + 65;
	TmpRect.top		= INPUT_WINDOW_NUM_STARTY + 4 ;
	TmpRect.bottom	= TmpRect.top + INPUT_WINDOW_NUM_STARTDY;

	if ( u8Focus == FOCUS )
	{
		SetTextColor(hdc,MVAPP_BLACK_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
	} else {
		SetTextColor(hdc,MVAPP_YELLOW_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
	}
	
	MV_FillBox( hdc, ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + ( 75 * u8FiledNum )), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY), ScalerWidthPixel(65), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTDY) );
	sprintf(temp_str, "%d", u16IpNum );
	CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
}
		
void Draw_IPKeypad(HDC hdc)
{
	U16 	i;
	char	temp_str[10];
	
	for ( i = 0 ; i < NUM_COUNT ; i++ )
	{
		if ( i == IPKeypad_X )
		{
			SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (i*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( INPUT_WINDOW_NUM_ITEMY ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
		} else {
			SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (i*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( INPUT_WINDOW_NUM_ITEMY ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
		}
		sprintf(temp_str, "%d", i );
		MV_CS_MW_TextOut( hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (i*KEYBOARD_KEY_OFFSET) + 8),ScalerHeigthPixel( INPUT_WINDOW_NUM_ITEMY + 2 ), temp_str);
	}

	for ( i = 0 ; i < IP_ITEM ; i++ )
	{
		if ( i == 0 )
			MV_Net_Draw_IP_Field(hdc , i, atoi(IP_TempData[i].Re_IP), FOCUS);
		else
			MV_Net_Draw_IP_Field(hdc , i, atoi(IP_TempData[i].Re_IP), UNFOCUS);
	}
}

void MV_Draw_IP_KeyPad(HWND hwnd, U8 max_string_length, eMV_Nerwork_Items esItem)
{
	HDC		hdc;
	RECT	Temp_Rect;
	
	IPKeypad_X = 0;
	IPKeypad_Y = 0;
	IPFocus_Index = 0;
	IP_keypad_enable = TRUE;
	b8Keypad_Save = FALSE;
	b8First_Key_in = TRUE;
	u8max_IP_length = max_string_length;

	memset(&btIPPad_cap, 0x00, sizeof(BITMAP));
	
	Temp_Rect.left	=INPUT_WINDOW_NUM_STARTX;
	Temp_Rect.right	=INPUT_WINDOW_NUM_STARTX + INPUT_WINDOW_NUM_STARTDX;
	Temp_Rect.top		=INPUT_WINDOW_NUM_STARTY + 4 ;
	Temp_Rect.bottom	=INPUT_WINDOW_NUM_STARTY + INPUT_WINDOW_NUM_STARTDY;

	hdc = MV_BeginPaint(hwnd);

	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX), ScalerHeigthPixel(NUM_KEYPAD_STARTY), ScalerWidthPixel(NUM_KEYPAD_STARTDX), ScalerHeigthPixel(NUM_KEYPAD_STARTDY), &btIPPad_cap);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX), ScalerHeigthPixel(NUM_KEYPAD_STARTY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX + NUM_KEYPAD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(NUM_KEYPAD_STARTY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX), ScalerHeigthPixel(NUM_KEYPAD_STARTY + NUM_KEYPAD_STARTDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX + NUM_KEYPAD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(NUM_KEYPAD_STARTY + NUM_KEYPAD_STARTDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(NUM_KEYPAD_STARTX + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(NUM_KEYPAD_STARTY),ScalerWidthPixel(NUM_KEYPAD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(NUM_KEYPAD_STARTDY));
	FillBox(hdc,ScalerWidthPixel(NUM_KEYPAD_STARTX), ScalerHeigthPixel(NUM_KEYPAD_STARTY + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(NUM_KEYPAD_STARTDX),ScalerHeigthPixel(NUM_KEYPAD_STARTDY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	
	
	Temp_Rect.top 	= NUM_KEYPAD_STARTY + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= NUM_KEYPAD_STARTX + WINDOW_OUT_GAP;
	Temp_Rect.right	= Temp_Rect.left + NUM_KEYPAD_STARTDX - WINDOW_OUT_GAP*2;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, NetSettingItemIdx[esItem]);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX + WINDOW_OUT_GAP), ScalerHeigthPixel(NUM_KEYPAD_STARTY + 50), ScalerWidthPixel(NUM_KEYPAD_STARTDX - WINDOW_OUT_GAP*2), ScalerHeigthPixel(NUM_KEYPAD_STARTDY - 60) );
	
	Draw_IPKeypad(hdc);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 2 + 10), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2),	ScalerHeigthPixel( INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 2 + 12),	CS_MW_LoadStringByIdx(CSAPP_STR_BACKSPACE));
	FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 3 + 10), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth * 2),	ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 3 + 12),	CS_MW_LoadStringByIdx(CSAPP_STR_ALL_CLEAR));
	FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 4 + 10), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2),	ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 4 + 12),	CS_MW_LoadStringByIdx(CSAPP_STR_SAVE));
	
	MV_EndPaint(hwnd,hdc);
}

void MV_Close_IP_Keypad( HWND hwnd )
{
	HDC		hdc;

	IP_keypad_enable = FALSE;		
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX), ScalerHeigthPixel(NUM_KEYPAD_STARTY), ScalerWidthPixel(NUM_KEYPAD_STARTDX), ScalerHeigthPixel(NUM_KEYPAD_STARTDY), &btIPPad_cap);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&btIPPad_cap);
}

U8 MV_Udhcpc_Data_Namecheck(char *tempSection)
{
	int 	i;
	char	Temp[20];
	
	for ( i = 0 ; i < CSAPP_UDHCPC_IP_MAX ; i++ )
	{
		//printf("\n===== MV_Udhcpc_Data_Namecheck : %d :: %s =====\n", strlen(Udhcpc_Check_String[i]), Udhcpc_Check_String[i]);
		memset (Temp, 0, sizeof(char) * 20);
		strncpy(Temp, tempSection, strlen(Udhcpc_Check_String[i]));
		
		if ( strcmp ( Udhcpc_Check_String[i], Temp ) == 0 )
			break;
	}

	return i;
}

void MV_Parser_DHCP_Value( char *Temp, char *tempSection )
{
	U16		i, j;
	
	for ( i = 0 ; i < strlen(tempSection) ; i++ )
	{
		if ( isdigit(tempSection[i]) )
			break;
	}
	
	j = i;

	for ( i = 0 ; i < strlen(tempSection) - j ; i++ )
	{
		if ( tempSection[i+j] == '\n' )
			break;
		
		Temp[i] = tempSection[i+j];
	}
}

BOOL MV_DHCP_Namecheck(char *tempSection)
{
	char	Temp[20];
	
	memset (Temp, 0, sizeof(char) * 20);
	strncpy(Temp, tempSection, strlen("nameserver"));

	if ( strcmp ( "nameserver", Temp ) == 0 ){
		return TRUE;
	}

	return FALSE;
}

MV_File_Return MV_LoadDHCPFile(void)
{
	FILE* 				fp;
    char 				tempSection [DHCP_FILE_MAX_COL + 2];
	char				Temp[100];
	int					i = 0;
	
	if (!(fp = fopen(DHCP_FILE, "r")))
	{
        return FILE_NOFILE;
	}

	while (!feof(fp)) {
		memset (tempSection, 0, sizeof(char) * (DHCP_FILE_MAX_COL + 2));
		memset (Temp, 0, sizeof(char) * 100);
		
        if (!fgets(tempSection, DHCP_FILE_MAX_COL, fp)) {
			return FILE_READ_FAIL;
        }

//		if ( (parse_index = MV_Udhcpc_Data_Namecheck(tempSection)) < CSAPP_UDHCPC_IP_MAX )
//		{
			MV_Parser_DHCP_Value(Temp, tempSection);
			//printf("\n====== %s ========\n", tempSection);

			switch(i)
			{
				case CSAPP_UDHCPC_IPADDR:
					MV_Parser_IP_DataValue( CSAPP_IP_IPADDR, Temp );
					break;
				case CSAPP_UDHCPC_SUBNET:
					MV_Parser_IP_DataValue( CSAPP_IP_SUBNET, Temp );
					break;
				case CSAPP_UDHCPC_GATEWAY:
					MV_Parser_IP_DataValue( CSAPP_IP_GATEWAY, Temp );
					break;
				default:
					break;
			}
//		}

		i++;
    }
	
	fclose (fp);
	return FILE_OK;
}

void MV_Parser_DNS_Value( char *Temp, char *tempSection )
{
	U16		i, j;
	
	for ( i = 0 ; i < strlen(tempSection) ; i++ )
	{
		if ( isdigit(tempSection[i]) )
			break;
	}
	
	j = i;

	for ( i = 0 ; i < strlen(tempSection) - j ; i++ )
	{
		if ( tempSection[i+j] == '\n' )
			break;
		
		Temp[i] = tempSection[i+j];
	}
}

BOOL MV_DNS_Namecheck(char *tempSection)
{
	char	Temp[20];
	
	memset (Temp, 0, sizeof(char) * 20);
	strncpy(Temp, tempSection, strlen("nameserver"));

	if ( strcmp ( "nameserver", Temp ) == 0 ){
		return TRUE;
	}

	return FALSE;
}

MV_File_Return MV_LoadDNSFile(void)
{
	FILE* 		fp;
    char 		tempSection [DHCP_FILE_MAX_COL + 2];
	char		Temp[100];

	if (!(fp = fopen(DHCP_RESOLV_FILE, "r")))
         return FILE_NOFILE;

	while (!feof(fp)) {
		memset (tempSection, 0, sizeof(char) * (DHCP_FILE_MAX_COL + 2));
		memset (Temp, 0, sizeof(char) * 100);
		
        if (!fgets(tempSection, DHCP_FILE_MAX_COL, fp)) {
			return FILE_READ_FAIL;
        }

		MV_Parser_DNS_Value(Temp, tempSection);
		//printf("\n===== Temp : %s , tempSection : %s \n", Temp, tempSection);
		if ( MV_DNS_Namecheck(tempSection) == TRUE )
		{
			//printf("===== %d : MV_DNS_Namecheck : %s\n", u8Check_DNS_Count, Temp);
			switch(u8Check_DNS_Count)
			{
				case 0:
					MV_Parser_IP_DataValue( CSAPP_IP_DNS1, Temp );
					break;
				case 1:
					MV_Parser_IP_DataValue( CSAPP_IP_DNS2, Temp );
					break;
				case 2:
					MV_Parser_IP_DataValue( CSAPP_IP_DNS3, Temp );
					break;
				default:
					break;
			}
			u8Check_DNS_Count++;
		}
    }
	
	fclose (fp);
	return FILE_OK;
}

void MV_DHCP_Data_Read(void)
{
	//printf("\n========== FILE : %s =============\n", DHCP_FILE);
	MV_LoadDHCPFile();
	MV_LoadDNSFile();
}

void MV_DNS_Data_Read(void)
{
	MV_LoadDNSFile();
}

void MV_Draw_NetSelectBar(HDC hdc, int y_gap, eMV_Nerwork_Items esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(Net_Enter_Kind[esItem])
	{
		case MV_NUMERIC:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 - ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_NUMBER].bmHeight),&MV_BMP[MVBMP_Y_NUMBER]);
			break;
		case MV_SELECT:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 - ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight),&MV_BMP[MVBMP_Y_ENTER]);
			break;
		default:
			break;
	}
	
	if ( Net_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_NetMenuBar(HDC hdc, U8 u8Focuskind, eMV_Nerwork_Items esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;

	if ( u8Focuskind == FOCUS )
	{
		//printf("================== FOCUS %d ===========================\n", esItem);
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_Draw_NetSelectBar(hdc, y_gap, esItem);
	} else {
		//printf("================== UNFOCUS %d ===========================\n", esItem);
		if ( u8Current_DHCP == CSAPP_DHCP_ON && ( esItem > CSAPP_NET_DNS_TYPE && esItem < CSAPP_NET_DNS1))
		{
			SetTextColor(hdc,MVAPP_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);		
			MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		} else if ( u8Current_DHCP == CSAPP_DHCP_OFF && esItem == CSAPP_NET_DNS_TYPE ) {
			SetTextColor(hdc,MVAPP_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);		
			MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		} else {
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);		
			MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		}
	}					

	CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(NetSettingItemIdx[esItem]));

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
		char	temp_str[20];
		
		case CSAPP_NET_DHCP:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(DHCP_STR[u8Current_DHCP]), -1, &TmpRect, DT_CENTER);		
			break;
		case CSAPP_NET_DNS_TYPE:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(DNS_STR[u8Current_DNS]), -1, &TmpRect, DT_CENTER);		
			break;
		case CSAPP_NET_IPADDR:
			sprintf(temp_str, "%d.%d.%d.%d", atoi(acIP_Addr[0].Re_IP), atoi(acIP_Addr[1].Re_IP), atoi(acIP_Addr[2].Re_IP), atoi(acIP_Addr[3].Re_IP) );
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case CSAPP_NET_SUBNET:
			sprintf(temp_str, "%d.%d.%d.%d", atoi(acSubnet_Mask[0].Re_IP), atoi(acSubnet_Mask[1].Re_IP), atoi(acSubnet_Mask[2].Re_IP), atoi(acSubnet_Mask[3].Re_IP) );
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_NET_GATEWAY:
			sprintf(temp_str, "%d.%d.%d.%d", atoi(acGateway[0].Re_IP), atoi(acGateway[1].Re_IP), atoi(acGateway[2].Re_IP), atoi(acGateway[3].Re_IP) );
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case CSAPP_NET_DNS1:
			sprintf(temp_str, "%d.%d.%d.%d", atoi(acDNS1[0].Re_IP), atoi(acDNS1[1].Re_IP), atoi(acDNS1[2].Re_IP), atoi(acDNS1[3].Re_IP) );
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_NET_DNS2:
			sprintf(temp_str, "%d.%d.%d.%d", atoi(acDNS2[0].Re_IP), atoi(acDNS2[1].Re_IP), atoi(acDNS2[2].Re_IP), atoi(acDNS2[3].Re_IP) );
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_NET_DNS3:
			sprintf(temp_str, "%d.%d.%d.%d", atoi(acDNS3[0].Re_IP), atoi(acDNS3[1].Re_IP), atoi(acDNS3[2].Re_IP), atoi(acDNS3[3].Re_IP) );
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_NET_MAC:
			sprintf(temp_str, "%02x:%02x:%02x:%02x:%02x:%02x", MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5] );
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		default:
			break;
	}
}

void MV_Draw_Net_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < CSAPP_NET_ITEM_MAX ; i++ )
	{
		if( Current_Item == i )
		{
			MV_Draw_NetMenuBar(hdc, FOCUS, i);
		} else {
			MV_Draw_NetMenuBar(hdc, UNFOCUS, i);
		}
	}
}

void MV_Draw_Net_MenuBar_By_DHCP(HDC hdc)
{
	U16 	i = 1;
	
	for( i = 1 ; i < CSAPP_NET_DNS_TYPE ; i++ )
		MV_Draw_NetMenuBar(hdc, UNFOCUS, i);
}

void MV_Draw_Net_MenuBar_By_DHCP_CHANGE(HDC hdc)
{
	U16 	i = 1;
	
	for( i = CSAPP_NET_DNS_TYPE ; i < CSAPP_NET_DNS1 ; i++ )
		MV_Draw_NetMenuBar(hdc, UNFOCUS, i);
}

void Read_MAC(void)
{
	memset(MacAddress, 0x00, MAC_SIZE);
	E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), 0x114, MacAddress, 6);
//	printf("\n=== %x %x %x %x %x %x ========\n", User_Volume[0], User_Volume[1], User_Volume[2], User_Volume[3], User_Volume[4], User_Volume[5]);
}

void Write_MAC(void)
{
	char		command_shell[100];

	memset(command_shell, 0x00, 100);
	sprintf(command_shell, "ifconfig eth0 hw ether %02x:%02x:%02x:%02x:%02x:%02x", MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5]);
	//printf("Command : %s\n", command_shell);
	E2P_Write(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), 0x114, MacAddress, 6);
	system("ifconfig eth0 down");
	system(command_shell);
	system("ifconfig eth0 up");

	//system("ifconfig");
}


CSAPP_Applet_t CSApp_NetworkSetting(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Network_Applets = CSApp_Applet_Error;

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

	CreateInfo.dwStyle		= WS_VISIBLE;
	CreateInfo.dwExStyle	= WS_EX_NONE;
	CreateInfo.spCaption	= "network setting";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = Network_Msg_cb;
	CreateInfo.lx 			= BASE_X;
	CreateInfo.ty 			= BASE_Y;
	CreateInfo.rx 			= BASE_X+WIDTH;
	CreateInfo.by 			= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 	= COLOR_transparent;
	CreateInfo.dwAddData 	= 0;
	CreateInfo.hHosting 	= HWND_DESKTOP;

	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);

	return CSApp_Network_Applets;   
}

void MV_Memset_IP_Data(void)
{
	memset(&acIP_Addr, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	memset(&acSubnet_Mask, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	memset(&acGateway, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	memset(&acDNS1, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	memset(&acDNS2, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	memset(&acDNS3, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	
	memset(&IP_Addr, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	memset(&IP_SubNet, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	memset(&IP_Gateway, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	memset(&IP_DNS1, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	memset(&IP_DNS2, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
	memset(&IP_DNS3, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);
}

int Network_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 				hdc;
	char 				ShellCommand[64];
	int  				Result;
   
	switch(message)
	   	{
			case MSG_CREATE:				
				memset(ReturnMAC, 0x00, MAC_SIZE*2);
				b8Check_Change_DHCP = FALSE;
				u8Current_DHCP = CS_DBU_GetDHCP_Type();
				u8Backup_DNS = u8Current_DNS = CS_DBU_GetDNS_Type();
				b8Keypad_Save = FALSE;
				Current_Item = 0;
				u8Check_DNS_Count = 0;
				b8Modify_flag = FALSE;
				MV_Memset_IP_Data();

				Read_MAC();

				//system("ifconfig");
				break;
				
			case MSG_PAINT:
				if ( u8Current_DHCP == CSAPP_DHCP_ON )
				{
					hdc=BeginPaint(hwnd);
					MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
					EndPaint(hwnd,hdc);
				
					system(SHELL_SCRIP);
					MV_DHCP_Data_Read();
					
					hdc=BeginPaint(hwnd);
					Close_Msg_Window(hdc);
					EndPaint(hwnd,hdc);
				}
				else
					MV_Load_IPData_File();

				MV_DNS_Data_Read();				
				MV_Copy_IPData();

				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_SYSTEM, EN_ITEM_FOCUS_NETWORK);

				hdc=BeginPaint(hwnd);
				MV_Draw_Net_MenuBar(hdc);
				MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_NETWORK);
				EndPaint(hwnd,hdc);
				return 0;
			
			case MSG_KEYDOWN:
/************************************** Confirm Process *****************************************/
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
								
								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
								EndPaint(hwnd,hdc);
								MV_Copy_IPData_Backup();
								MV_Save_IPData_File();
								
								system("/etc/init.d/S40network restart");
									
								if ( u8Backup_DNS != u8Current_DNS )
								{
									char 	Temp_Str[255];

									memset(Temp_Str, 0x00, 255);
#if 0
									if ( u8Current_DNS == CSAPP_DNS_AUTO )
										sprintf(Temp_Str, "cp %s %s", AUTO_DNS_CFG, DNS_CFG);
									else
										sprintf(Temp_Str, "cp %s %s", MANU_DNS_CFG, DNS_CFG);
#endif
								}
								
								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							} else {
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);
								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							}
						} else {
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
					}
					
					if (wparam != CSAPP_KEY_IDLE)
					{
						break;
					}
				}

/**************************** Hexa Edit Process ******************************************************/
#ifdef MAC_ADDR_CHANGE
				if ( MV_Get_HexaKeypad_Status() == TRUE )
				{
					char		ReturnValue[18];
					char		Splite_Str[3];
					int			i, j = 0, k = 0;
					U8			TempU8;
					
					MV_HexaKeypad_Proc(hwnd, wparam);
					
					if ( wparam == CSAPP_KEY_ENTER )
					{
						strcpy(ReturnValue, MV_Get_HexaEdited_String());

						for ( i = 0 ; i < 18 ; i++ )
						{
							if ( ReturnValue[i] != ':' )
							{
								ReturnMAC[j] = ReturnValue[i];
								j++;
							}
						}

						//printf("%s =================\n", ReturnMAC);
						
						j = 0;
						k = strlen(ReturnMAC);
						memset(Splite_Str, 0x00, 3);
						for ( i = 0 ; i < k ; i++ )
						{
							Splite_Str[i%2] = ReturnMAC[i];
							if ( i%2 == 1 && i != 0 )
							{
								Str2Hex(Splite_Str, &TempU8);	
								//printf("i : %02d , j : %d ===== %s = %02x\n", i, j, Splite_Str, TempU8);
								MacAddress[j] = TempU8;
								j++;
							}					
						}
						//printf("%02x:%02x:%02x:%02x:%02x:%02x =================\n", MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5]);
						Write_MAC();
						b8Modify_flag = TRUE;

						hdc=BeginPaint(hwnd);
						MV_Draw_NetMenuBar(hdc, FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
					}
					break;
				}
#endif
/***************************** IP Key Edit Process ***********************************************/
				if ( IP_keypad_enable == TRUE )
				{
					UI_IPKeypad_Proc(hwnd, wparam);

					if ( wparam == CSAPP_KEY_BLUE )
					{
						switch(Current_Item)
						{
							case CSAPP_NET_IPADDR:
								memcpy(&acIP_Addr, &IP_TempData, sizeof(stIPStr_Struct) * IP_ITEM);

								if ( atoi(acSubnet_Mask[0].Re_IP) == 0 )
									memcpy(&acSubnet_Mask, &Default_Netmask, sizeof(stIPStr_Struct) * IP_ITEM);
								break;
								
							case CSAPP_NET_SUBNET:
								memcpy(&acSubnet_Mask, &IP_TempData, sizeof(stIPStr_Struct) * IP_ITEM);
								break;
								
							case CSAPP_NET_GATEWAY:
								memcpy(&acGateway, &IP_TempData, sizeof(stIPStr_Struct) * IP_ITEM);
								break;
								
							case CSAPP_NET_DNS1:
								memcpy(&acDNS1, &IP_TempData, sizeof(stIPStr_Struct) * IP_ITEM);
								break;
								
							case CSAPP_NET_DNS2:
								memcpy(&acDNS2, &IP_TempData, sizeof(stIPStr_Struct) * IP_ITEM);
								break;

							case CSAPP_NET_DNS3:
								memcpy(&acDNS3, &IP_TempData, sizeof(stIPStr_Struct) * IP_ITEM);
								break;
								
							default:
								break;
						}
						
						hdc=BeginPaint(hwnd);
						MV_Draw_NetMenuBar(hdc, FOCUS, Current_Item);
						
						if ( Current_Item == CSAPP_NET_IPADDR )
							MV_Draw_NetMenuBar(hdc, UNFOCUS, CSAPP_NET_SUBNET);
						
						EndPaint(hwnd,hdc);
						b8Modify_flag = TRUE;
					}
					break;
				}else if ( MV_Get_PopUp_Window_Status() == TRUE )
/***************************** Popup Process ***********************************************/
				{
					MV_PopUp_Proc(hwnd, wparam);

					if ( wparam == CSAPP_KEY_ENTER )
					{
						U8	u8Result_Value;

						u8Result_Value = MV_Get_PopUp_Window_Result();

						switch(Current_Item)
						{
							case CSAPP_NET_DHCP:
								u8Current_DHCP = u8Result_Value;
								hdc = BeginPaint(hwnd);
								CS_DBU_SetDHCP_Type(u8Current_DHCP);
								MV_Draw_NetMenuBar(hdc, FOCUS, Current_Item);
								MV_Draw_Net_MenuBar_By_DHCP_CHANGE(hdc);
								EndPaint(hwnd,hdc);
								break;
							case CSAPP_NET_DNS_TYPE:
								u8Current_DNS = u8Result_Value;
								hdc = BeginPaint(hwnd);
								CS_DBU_SetDNS_Type(u8Current_DNS);
								MV_Draw_NetMenuBar(hdc, FOCUS, Current_Item);
								EndPaint(hwnd,hdc);
								break;
							default:
								break;
						}
						b8Modify_flag = TRUE;
					}
					break;
				}
				
				switch(wparam)
				{
					case CSAPP_KEY_YELLOW:
						if( u8Current_DHCP == CSAPP_DHCP_ON )
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_GET_DHCP);
							EndPaint(hwnd,hdc);
							
							sprintf(ShellCommand,"udhcpc -n -t 5 -T 2");
							Result = system( ShellCommand );
							//printf("\n====== Result : %d ==========\n\n", Result);
							
							if( Result != 0 )
							{
								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_GET_DHCP_FAIL);
								EndPaint(hwnd,hdc);
								
								usleep( 5000 * 1000 );
								
								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
							}	
							else
							{
								system(SHELL_SCRIP);
								MV_DHCP_Data_Read();
								MV_Copy_IPData();
								
								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								MV_Draw_Net_MenuBar_By_DHCP_CHANGE(hdc);
								EndPaint(hwnd,hdc);
							}
						}else {
#if 0
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_SAVE);
							EndPaint(hwnd,hdc);

							MV_Copy_IPData_Backup();
							MV_Save_IPData_File();

							MV_Memset_IP_Data();
							Read_MAC();
							
							system(SHELL_SCRIP);
							
							MV_Load_IPData_File();
							MV_DNS_Data_Read();
							MV_Copy_IPData();

							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							MV_Draw_Net_MenuBar(hdc);
							MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_NETWORK);
							EndPaint(hwnd,hdc);
#endif
						}
						break;

					case CSAPP_KEY_UP:
						hdc=BeginPaint(hwnd);
						MV_Draw_NetMenuBar(hdc, UNFOCUS, Current_Item);

						if ( Current_Item == CSAPP_NET_DHCP )
							Current_Item = CSAPP_NET_ITEM_MAX - 1;
						else
							if ( u8Current_DHCP == CSAPP_DHCP_ON && Current_Item == CSAPP_NET_DNS1 )
								Current_Item = CSAPP_NET_DNS_TYPE;
							else if ( u8Current_DHCP == CSAPP_DHCP_OFF && Current_Item == CSAPP_NET_IPADDR )
								Current_Item = CSAPP_NET_DHCP;
							else
								Current_Item--;

						MV_Draw_NetMenuBar(hdc, FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
						break;
						
					case CSAPP_KEY_DOWN:
						hdc=BeginPaint(hwnd);
						MV_Draw_NetMenuBar(hdc, UNFOCUS, Current_Item);

						if ( Current_Item == CSAPP_NET_ITEM_MAX - 1 )
							Current_Item = CSAPP_NET_DHCP;
						else
							if ( u8Current_DHCP == CSAPP_DHCP_ON && Current_Item == CSAPP_NET_DNS_TYPE )
								Current_Item = CSAPP_NET_DNS1;
							else if ( u8Current_DHCP == CSAPP_DHCP_OFF && Current_Item == CSAPP_NET_DHCP )
								Current_Item = CSAPP_NET_IPADDR;
							else
								Current_Item++;

						MV_Draw_NetMenuBar(hdc, FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_LEFT:
						if ( Current_Item == CSAPP_NET_DHCP )
						{
							b8Check_Change_DHCP = TRUE;
							if ( u8Current_DHCP == CSAPP_DHCP_ON )
								u8Current_DHCP = CSAPP_DHCP_OFF;
							else
								u8Current_DHCP = CSAPP_DHCP_ON;
							
							CS_DBU_SetDHCP_Type(u8Current_DHCP);
#if 0
							if ( u8Current_DHCP == CSAPP_DHCP_ON )
								u8Current_DHCP = CSAPP_DHCP_OFF;
							else
							{
								if ( Check_Compare_IP() == FALSE )
								{
									MV_Copy_IPData_Backup();									
									MV_Save_IPData_File();									
								}
								u8Current_DHCP = CSAPP_DHCP_ON;
							}
#endif
							
							hdc=BeginPaint(hwnd);
							MV_Draw_NetMenuBar(hdc, FOCUS, Current_Item);
							MV_Draw_Net_MenuBar_By_DHCP_CHANGE(hdc);
							MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_NETWORK);
							EndPaint(hwnd,hdc);
						} else if ( Current_Item == CSAPP_NET_DNS_TYPE )
						{
							
							if ( u8Current_DNS == CSAPP_DNS_AUTO )
							{
								sprintf(ShellCommand,"cp %s %s", DHCP_SCRIP_M, DHCP_SCRIP);
								u8Current_DNS = CSAPP_DNS_MANUAL;
							}
							else
							{
								sprintf(ShellCommand,"cp %s %s", DHCP_SCRIP_A, DHCP_SCRIP);
								u8Current_DNS = CSAPP_DNS_AUTO;
							}

							if ( system(ShellCommand) != 0 )
								printf("=== Default.script Copy Fail : %s\n", ShellCommand);

							hdc=BeginPaint(hwnd);
							CS_DBU_SetDNS_Type(u8Current_DNS);
							MV_Draw_NetMenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
						}
						b8Modify_flag = TRUE;
						break;
						
					case CSAPP_KEY_RIGHT:
						if ( Current_Item == CSAPP_NET_DHCP )
						{
							b8Check_Change_DHCP = TRUE;
							if ( u8Current_DHCP == CSAPP_DHCP_ON )
								u8Current_DHCP = CSAPP_DHCP_OFF;
							else
								u8Current_DHCP = CSAPP_DHCP_ON;
							
							CS_DBU_SetDHCP_Type(u8Current_DHCP);
							
							hdc=BeginPaint(hwnd);
							MV_Draw_NetMenuBar(hdc, FOCUS, Current_Item);
							MV_Draw_Net_MenuBar_By_DHCP_CHANGE(hdc);
							MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_NETWORK);
							EndPaint(hwnd,hdc);
						} else if ( Current_Item == CSAPP_NET_DNS_TYPE )
						{
							if ( u8Current_DNS == CSAPP_DNS_AUTO )
							{
								sprintf(ShellCommand,"cp %s %s", DHCP_SCRIP_M, DHCP_SCRIP);
								u8Current_DNS = CSAPP_DNS_MANUAL;
							}
							else
							{
								sprintf(ShellCommand,"cp %s %s", DHCP_SCRIP_A, DHCP_SCRIP);
								u8Current_DNS = CSAPP_DNS_AUTO;
							}

							if ( system(ShellCommand) != 0 )
								printf("=== Default.script Copy Fail : %s\n", ShellCommand);

							hdc=BeginPaint(hwnd);
							CS_DBU_SetDNS_Type(u8Current_DNS);
							MV_Draw_NetMenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
						}
						b8Modify_flag = TRUE;
						break;
						
					case CSAPP_KEY_ENTER:
						if ( Current_Item == CSAPP_NET_DHCP )
						{
							RECT					smwRect;
							stPopUp_Window_Contents stContents;

							memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

							sprintf(stContents.Contents[0], "%s", CS_MW_LoadStringByIdx(DHCP_STR[0]));
							sprintf(stContents.Contents[1], "%s", CS_MW_LoadStringByIdx(DHCP_STR[1]));
							
							smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Current_Item + 1 );
							smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
							smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
							smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
							stContents.u8TotalCount = 2;
							MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
						} else if ( Current_Item == CSAPP_NET_DNS_TYPE ) {
							RECT					smwRect;
							stPopUp_Window_Contents stContents;

							memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

							sprintf(stContents.Contents[0], "%s", CS_MW_LoadStringByIdx(DNS_STR[0]));
							sprintf(stContents.Contents[1], "%s", CS_MW_LoadStringByIdx(DNS_STR[1]));
							
							smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Current_Item + 1 );
							smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
							smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
							smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
							stContents.u8TotalCount = 2;
							MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
						}
						else if ( Current_Item > CSAPP_NET_DHCP && Current_Item < CSAPP_NET_PING )
						{
							switch(Current_Item)
							{
								case CSAPP_NET_IPADDR:
									memcpy(&IP_TempData, &acIP_Addr, sizeof(stIPStr_Struct) * IP_ITEM);
									MV_Draw_IP_KeyPad(hwnd, IP_LENGTH, Current_Item);
									break;
									
								case CSAPP_NET_SUBNET:
									memcpy(&IP_TempData, &acSubnet_Mask, sizeof(stIPStr_Struct) * IP_ITEM);
									MV_Draw_IP_KeyPad(hwnd, IP_LENGTH, Current_Item);
									break;
									
								case CSAPP_NET_GATEWAY:
									memcpy(&IP_TempData, &acGateway, sizeof(stIPStr_Struct) * IP_ITEM);
									MV_Draw_IP_KeyPad(hwnd, IP_LENGTH, Current_Item);
									break;
									
								case CSAPP_NET_DNS1:
									memcpy(&IP_TempData, &acDNS1, sizeof(stIPStr_Struct) * IP_ITEM);
									MV_Draw_IP_KeyPad(hwnd, IP_LENGTH, Current_Item);
									break;
									
								case CSAPP_NET_DNS2:
									memcpy(&IP_TempData, &acDNS2, sizeof(stIPStr_Struct) * IP_ITEM);
									MV_Draw_IP_KeyPad(hwnd, IP_LENGTH, Current_Item);
									break;

								case CSAPP_NET_DNS3:
									memcpy(&IP_TempData, &acDNS3, sizeof(stIPStr_Struct) * IP_ITEM);
									MV_Draw_IP_KeyPad(hwnd, IP_LENGTH, Current_Item);
									break;

								case CSAPP_NET_MAC:
									{
#ifdef MAC_ADDR_CHANGE
										char	Temp_Mac[18];

										sprintf(Temp_Mac, "%02x:%02x:%02x:%02x:%02x:%02x", MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5]);
										MV_Draw_HexaKeypad(hwnd, Temp_Mac, 18);
#endif
									}
									break;
									
								default:
									break;
							}
						}else {
							if ( Current_Item == CSAPP_NET_PING )
							{
								U8 Result2;
								if ( u8Current_DHCP == CSAPP_DHCP_ON )
								{
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_PINGING);
									EndPaint(hwnd,hdc);

									MV_Save_DNS_File();

									sprintf(ShellCommand,"ping -c 3 www.yahoo.com");
									Result = system( ShellCommand );
									//printf("\n====== Result : %d ==========\n\n", Result);

									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_PINGING_IP);
									EndPaint(hwnd,hdc);

									sprintf(ShellCommand,"ping -c 3 -W 1 87.248.122.122");
									Result2 = system( ShellCommand );
									//printf("\n====== Result2 : %d ==========\n\n", Result2);

									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									EndPaint(hwnd,hdc);

									if ( Result != 0 && Result2 != 0)
									{
										hdc=BeginPaint(hwnd);
										MV_Draw_Msg_Window(hdc, CSAPP_STR_PING_FAIL);
										EndPaint(hwnd,hdc);
										
										usleep( 4000 * 1000 );
										
										hdc=BeginPaint(hwnd);
										Close_Msg_Window(hdc);
										EndPaint(hwnd,hdc);
									} else if ( Result != 0 && Result2 == 0) {
										hdc=BeginPaint(hwnd);
										MV_Draw_Msg_Window(hdc, CSAPP_STR_PING_IPOK);
										EndPaint(hwnd,hdc);
										
										usleep( 2000 * 1000 );
										
										hdc=BeginPaint(hwnd);
										Close_Msg_Window(hdc);
										EndPaint(hwnd,hdc);
									} else {
										hdc=BeginPaint(hwnd);
										MV_Draw_Msg_Window(hdc, CSAPP_STR_PING_OK);
										EndPaint(hwnd,hdc);
										
										usleep( 2000 * 1000 );
										
										hdc=BeginPaint(hwnd);
										Close_Msg_Window(hdc);
										EndPaint(hwnd,hdc);
									}
								} else {
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
									EndPaint(hwnd,hdc);

									MV_Copy_IPData_Backup();
									MV_Save_IPData_File();

									MV_Memset_IP_Data();
									Read_MAC();
									
									system(SHELL_SCRIP);
									
									MV_Load_IPData_File();									
									MV_DNS_Data_Read();				
									MV_Copy_IPData();

									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									MV_Draw_Net_MenuBar(hdc);
									MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_NETWORK);
									EndPaint(hwnd,hdc);
								}
							}
							b8Modify_flag = TRUE;
						}
						break;
						
					case CSAPP_KEY_ESC:
						/*
						if ( Check_Null_IP() == FALSE )
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_NO_DATA);
							EndPaint(hwnd,hdc);

							usleep( 2000 * 1000 );
									
							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);
							break;
						}
*/
						CSApp_Network_Applets=CSApp_Applet_Desktop;

						if ( b8Modify_flag == TRUE )
						{
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVE_OR_NOT);
						} else {
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;
						
					case CSAPP_KEY_MENU:
/*
						if ( Check_Null_IP() == FALSE )
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_NO_DATA);
							EndPaint(hwnd,hdc);

							usleep( 2000 * 1000 );
									
							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);
							break;
						}
*/
						CSApp_Network_Applets=b8Last_App_Status;

						if ( b8Modify_flag == TRUE )
						{
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVE_OR_NOT);
						} else {
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;

					case CSAPP_KEY_IDLE:
						CSApp_Network_Applets = CSApp_Applet_Sleep;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_TV_AV:
						ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
						break;
							
					default:
						break;
				}
				break;
			
		   	case MSG_CLOSE:
				if(CS_DBU_CheckIfUserSettingDataChanged())
				{
					CS_DBU_SaveUserSettingDataInHW();
				}
				
				DestroyMainWindow(hwnd);
				PostQuitMessage(hwnd);
				break;

		   	default:
				break;
	   	}
	
   return DefaultMainWinProc(hwnd,message,wparam,lparam);
}



