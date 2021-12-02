#include "linuxos.h"

#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "database.h"
#include "cskeyedit.h"
#include "casdrv_def.h"
#include "caskey.h"

#define	USE_ADD

static CSAPP_Applet_t		CSApp_KeyEdit_Applets;
static CasDbInfo_t			stCasDBInfo;
static U8					u8Key_length = 0;
static eMV_KeyFocus_Status	u8Window_Status = KEYEDIT_CAS_WINDOW;
static eMV_KeyEdit_Status	u8Add_or_Edit = KEYEDIT_EDIT;
static U8					u8Cas_Focus = 0;
static U8					u8Provider_Focus = 0;
static U8					u8Provider_Current_Item = 0;
static U16					u16Provider_Current_Page = 0;
static U16					u16Provider_Prev_Page = 0;
static U8					u8Key_Focus = 0;
static U8                   u8AddKeyNumber = CAS_KEYTYPE_INVALID;
static U8                   u8AddKeyLength = 0;
static KeyTempData_t		stKeyTemp_Data[KEY_ITEM_MAX];

static CasData_t			*stCurrentCas = NULL;
static ProviderData_t		*stCurrentProvider = NULL;

static int KeyEdit_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);


void MV_Get_Cas_Data_ByIndex(CasData_t **stCasData, U8 u8Index)
{
	int			i;
	CasData_t	*stTemp_Cas = NULL;

	stTemp_Cas = stCasDBInfo.CasData_P;
	for ( i = 0 ; i < u8Index ; i++ )
		stTemp_Cas = stTemp_Cas->NextCasData;

	*stCasData = stTemp_Cas;
}

void MV_Get_Provider_Data_ByIndex(ProviderData_t **stProviderData, U8 u8Cas_index, U8 u8Prov_Index)
{
	int				i;
	CasData_t		*stTemp_Cas = NULL;
	ProviderData_t	*stTemp_Provider = NULL;
	
	MV_Get_Cas_Data_ByIndex(&stTemp_Cas, u8Cas_index);

	if ((stTemp_Cas->TotalNumberOfProvider == 0) || ( u8Prov_Index > stTemp_Cas->TotalNumberOfProvider - 1))
	{
		printf("======= TOTAL Provider Number over ==> index : %d, Total : %d\n", u8Prov_Index, stTemp_Cas->TotalNumberOfProvider);
		return;
	}

	stTemp_Provider = stTemp_Cas->Provider;
	for ( i = 0 ; i < u8Prov_Index ; i++ )
		stTemp_Provider = stTemp_Provider->NextProviderData;

	*stProviderData = stTemp_Provider;
}

void MV_Set_Cas_KeyData_Reform(U8 u8Cas_index, U8 u8Prov_Index)
{
	// CasData_t		*stTemp_Cas = NULL;
	ProviderData_t	*stTemp_Provider = NULL;
	KeyData_t		*stTemp_Key = NULL;
	int				i = 0, j, k, l = 0;

	memset(&stKeyTemp_Data, 0x00, sizeof(KeyTempData_t) * KEY_ITEM_MAX);
		
	MV_Get_Provider_Data_ByIndex(&stTemp_Provider, u8Cas_index, u8Prov_Index);

	stTemp_Key = stTemp_Provider->Key;
	u8NumberOfKey = 255;
	if (stTemp_Key == NULL)
	{
		return;
	}

	//printf("MV_Set_Cas_KeyData_Reform : %d : %d\n", stTemp_Provider->TotalNumberOfKey, stTemp_Key->KeyLength);
	
	do
	{
		if ( stTemp_Key->KeyNumber >= 0x40 )
		{
			stTemp_Key = stTemp_Key->NextKeyData;
			l++;
			continue;
		}
		
		if ( stTemp_Key->KeyLength > 16 )
		{
			for ( j = 0 ; j < (stTemp_Key->KeyLength/KEY_DATA_MAX_LENGTH) ; j++ )
			{
				stKeyTemp_Data[i].KeyNumber = stTemp_Key->KeyNumber + j;
				stKeyTemp_Data[i].KeyLength = KEY_DATA_MAX_LENGTH;
				
				for ( k = 0 ; k < KEY_DATA_MAX_LENGTH ; k++ )
					sprintf(&stKeyTemp_Data[i].KeyData[k*2], "%02x", stTemp_Key->KeyData[(j*KEY_DATA_MAX_LENGTH) + k]);

				//printf("BIG %d KeyData : %02x : %d : %d : %s\n", i, stTemp_Key->KeyNumber, stTemp_Key->KeyLength, stKeyTemp_Data[i].KeyLength, stKeyTemp_Data[i].KeyData);
				i++;
			}
			l++;
		}
		else 
		{
			stKeyTemp_Data[i].KeyNumber = stTemp_Key->KeyNumber;
			stKeyTemp_Data[i].KeyLength = stTemp_Key->KeyLength;
					
			for ( j = 0 ; j < stTemp_Key->KeyLength ; j++ )
				sprintf(&stKeyTemp_Data[i].KeyData[j*2], "%02x", stTemp_Key->KeyData[j]);

			//printf("SMALL : %d : %02x KeyData : %s\n", i, stTemp_Key->KeyNumber, stKeyTemp_Data[i].KeyData);
			
			i++;
			l++;
		}
		stTemp_Key = stTemp_Key->NextKeyData;
	} while ((l < stTemp_Provider->TotalNumberOfKey) && (stTemp_Key != NULL));

	/* For 0 key case by KB Kim 2012.06.07 */
	if (i > 0)
	{
		u8NumberOfKey = i - 1 ;
	}
	else
	{
		u8NumberOfKey = 255;
	}
	printf("====== %d Number of Key : %d\n", l, u8NumberOfKey);
}

BOOL MV_Cas_KeyData_Update(ProviderData_t *provider, U8 u8Key_Index)
{
	KeyData_t		*stTemp_Key = NULL;
	U8              keyPointer;
	U8              tmpKeyNumber;
	U8              tmpKeyIndex = 0;
	U8              keyCount;
	U8              point;
	U8              TempU8;

	if (u8NumberOfKey == 255)
	{
		return FALSE;
	}
	
	stTemp_Key = provider->Key;
	if (stTemp_Key == NULL)
	{
		return FALSE;
	}

	tmpKeyNumber = stKeyTemp_Data[u8Key_Index].KeyNumber;
	if (tmpKeyNumber > CAS_KEY_TYPE_PK0)
	{
		tmpKeyNumber &= 0xF0;
		tmpKeyIndex = tmpKeyNumber & 0x0F;
	}

	printf("MV_Cas_KeyData_Update : KeyIndexNumber[0x%02X] KeyNumber[0x%02X] KeyIndex[%d]\n",
		stKeyTemp_Data[u8Key_Index].KeyNumber, tmpKeyNumber, tmpKeyIndex);

	keyCount = 0;
	do
	{
		if ( stTemp_Key->KeyNumber < CAS_KEYTYPE_ECM_06)
		{
			if (stTemp_Key->KeyNumber == tmpKeyNumber)
			{
				keyPointer = tmpKeyIndex * KEY_DATA_MAX_LENGTH;
				for ( point = 0 ; point < stKeyTemp_Data[u8Key_Index].KeyLength ; point++ )
				{
					Str2Hex(stKeyTemp_Data[u8Key_Index].KeyData + (point*2), &TempU8);
					stTemp_Key->KeyData[keyPointer + point] = TempU8;
				}

				return TRUE;
			}
		}

		stTemp_Key = stTemp_Key->NextKeyData;
		keyCount++;
	}while ((keyCount < provider->TotalNumberOfKey) && (stTemp_Key != NULL));

	/* Can't find Key in dB */

	return FALSE;
}

BOOL MV_Get_Cas_KeyNumber(U8 *casId, U8 *data, U8 length, U8 *keyNumber, U8 *keuLength)
{
	U16 u16CasId;
	U8  val;

	*keyNumber = CAS_KEYTYPE_INVALID;
	*keuLength = 0;
	
	if (length == 2)
	{
		if (Str2Hex(data, &val) == FALSE)
		{
			return FALSE;
		}
	}
	else if (length == 1)
	{
		if (Char2Hex(data[0], &val) == FALSE)
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	u16CasId = (U16)Array2Word(casId, 2);
	switch(u16CasId)
	{
		case SECA_CAS_ID        :
		case VIACCESS_CAS_ID    :
		case VIACCESS2_CAS_ID   :
			if (val >= CAS_KEY_TYPE_PK0)
			{
				return FALSE;
			}
			break;
		case IRDETO_CAS_ID      :
			if (val >= CAS_KEYTYPE_ECM_MOD)
			{
				return FALSE;
			}
			break;
		case NAGRA_CAS_ID       :
		case NAGRA2_CAS_ID      :
		case CRYPTOWORKS_CAS_ID :
			if (val > 0x01)
			{
				return FALSE;
			}
			break;
		case BISS_CAS_ID        :
		case DCW_CAS_ID         :
			if (val != 0x00)
			{
				return FALSE;
			}
			break;
		case TPS_CAS_ID         :
			if (val != 0x01)
			{
				return FALSE;
			}
			break;
		default                 :
			if (val >= CAS_KEY_TYPE_PK0)
			{
				return FALSE;
			}
			break;
	}

	*keyNumber = val;
	switch(u16CasId)
	{
		case SECA_CAS_ID        :
		case VIACCESS_CAS_ID    :
		case IRDETO_CAS_ID      :
		case NAGRA_CAS_ID       :
			*keuLength = 8;
			break;
		case VIACCESS2_CAS_ID   :
		case NAGRA2_CAS_ID      :
		case CRYPTOWORKS_CAS_ID :
		case BISS_CAS_ID        :
		case DCW_CAS_ID         :
		case TPS_CAS_ID         :
		default                 :
			*keuLength = 16;
			break;
	}
	
	return TRUE;
}

BOOL MV_Cas_KeyData_Add(CasData_t *cas, ProviderData_t *provider, U8 keyNumber, U8 *keyStr)
{
	U8              keyPointer;
	U8              dataLength;
	U8              point;
	U8              TempU8;
	U8              keyData[KEY_DATA_MAX_LENGTH];

	dataLength = (U8)strlen(keyStr);
	if (dataLength > KEY_DATA_ARRAY_SIZE)
	{
		return FALSE;
	}
	keyPointer = 0;
	for (point = 0; point < dataLength; point += 2)
	{
		Str2Hex(keyStr + point, &TempU8);
		keyData[keyPointer] = TempU8;
		keyPointer++;
	}

	/* Can't find Key in dB */

	return CasDrvUpdateKey (cas->CasId, provider->ProviderId, provider->ProviderName, keyNumber, keyPointer, keyData);
}

CSAPP_Applet_t CSApp_KeyEdit(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_KeyEdit_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption	= "keyedit";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = KeyEdit_Msg_cb;
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

	return CSApp_KeyEdit_Applets;   
}

void MV_Draw_Window_Title(HDC hdc)
{
	RECT	rc1;
	
	SetBrushColor(hdc, MVAPP_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(CAS_WINDOW_X), ScalerHeigthPixel(TOP_WINDOW_Y),ScalerWidthPixel(CAS_WINDOW_DX),ScalerHeigthPixel(TOP_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(PROVIDER_WINDOW_X), ScalerHeigthPixel(TOP_WINDOW_Y),ScalerWidthPixel(PROVIDER_WINDOW_DX),ScalerHeigthPixel(TOP_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(KEY_WINDOW_X), ScalerHeigthPixel(TOP_WINDOW_Y),ScalerWidthPixel(KEY_WINDOW_DX),ScalerHeigthPixel(TOP_WINDOW_DY));
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	
	rc1.left = CAS_WINDOW_X;
	rc1.top = TOP_WINDOW_Y + 4;
	rc1.right = rc1.left + CAS_WINDOW_DX;
	rc1.bottom = rc1.top + TOP_WINDOW_DY;
	CS_MW_DrawText(hdc, "CAS", -1, &rc1, DT_CENTER);

	rc1.left = PROVIDER_WINDOW_X;
	rc1.right = rc1.left + PROVIDER_WINDOW_DX;
	CS_MW_DrawText(hdc, "Provider", -1, &rc1, DT_CENTER);

	rc1.left = KEY_WINDOW_X;
	rc1.right = rc1.left + KEY_WINDOW_DX;
	CS_MW_DrawText(hdc, "Key Data", -1, &rc1, DT_CENTER);
}

void MV_Draw_Cas_Item(HDC hdc, int Cas_Item, BOOL bFocus)
{
	char		TempStr[64];
	RECT		rc1;
	CasData_t	*stTemp_Cas = NULL;
	
	MV_Get_Cas_Data_ByIndex(&stTemp_Cas, Cas_Item);

	rc1.left = CAS_WINDOW_X;
	rc1.top = CAS_WINDOW_Y + ( TOP_WINDOW_DY * Cas_Item );
	rc1.right = rc1.left + CAS_WINDOW_DX;
	rc1.bottom = rc1.top + TOP_WINDOW_DY;

	//printf("CasItem : %d , Cas_Focus : %d , bFocus : %d , u8Window_Status : %d\n", Cas_Item, u8Cas_Focus, bFocus, u8Window_Status);

	if ( bFocus == FOCUS && u8Window_Status == KEYEDIT_CAS_WINDOW )
	{
		SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
		FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));
		SetTextColor(hdc,CSAPP_BLACK_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
	}
	else if ( Cas_Item == u8Cas_Focus && u8Window_Status != KEYEDIT_CAS_WINDOW && bFocus == UNFOCUS )
	{
		//printf("======== TEST DRAW ============>>>>>>>>> \n");
		SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
		FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

		if ( Cas_Item%2 == 1 )
			SetBrushColor(hdc, MVAPP_GRAY_COLOR);
		else
			SetBrushColor(hdc, MVAPP_DARKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(rc1.left + 2), ScalerHeigthPixel(rc1.top + 2),ScalerWidthPixel(rc1.right - rc1.left - 4),ScalerHeigthPixel(rc1.bottom - rc1.top - 4));
		
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
	} 
	else 
	{
		if ( Cas_Item%2 == 1 )
			SetBrushColor(hdc, MVAPP_GRAY_COLOR);
		else
			SetBrushColor(hdc, MVAPP_DARKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
	}

	rc1.top += 4;
	sprintf(TempStr, "%s", stTemp_Cas->CasName);
//	printf("============ %02x, %02x ============\n", stTemp_Cas->CasId[0], stTemp_Cas->CasId[1]);
	CS_MW_DrawText(hdc, TempStr, -1, &rc1, DT_CENTER | DT_VCENTER);
}

void MV_Draw_Cas_Window(HDC hdc)
{
	int			i;
	
	SetBrushColor(hdc, MVAPP_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(CAS_WINDOW_X), ScalerHeigthPixel(CAS_WINDOW_Y),ScalerWidthPixel(CAS_WINDOW_DX),ScalerHeigthPixel(CAS_WINDOW_DY));
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	for ( i = 0 ; i < stCasDBInfo.NumberOfCas ; i++ )
	{
		if ( i == u8Cas_Focus && u8Window_Status == KEYEDIT_CAS_WINDOW )
			MV_Draw_Cas_Item(hdc, i, FOCUS);
		else
			MV_Draw_Cas_Item(hdc, i, UNFOCUS);
	}
	//printf("MV_Draw_Cas_Window : ==================\n");
}

void MV_Draw_Provider_Item(HDC hdc, int Count_index, U8 u8Provider_Item, BOOL bFocus)
{
	char			TempStr1[30];
	char			TempStr2[60];
	RECT			rc1;
	RECT			Scroll_Rect;
	ProviderData_t  *stTempProvider = NULL;
	
	/* For 0 provider case by KB Kim 2012.06.07 */
	if (u8NumberOfProvider == 255)
	{
		return;
	}
	
	rc1.left = PROVIDER_WINDOW_X;
	rc1.top = PROVIDER_WINDOW_Y + ( PROVIDER_ITEM_DY * Count_index );
	rc1.right = rc1.left + PROVIDER_WINDOW_DX;
	rc1.bottom = rc1.top + PROVIDER_ITEM_DY;
	
	if ( bFocus == FOCUS && u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
	{	
		SetTextColor(hdc,CSAPP_BLACK_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
		FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));
	}
	else if ( bFocus == UNFOCUS && u8Window_Status == KEYEDIT_KEY_WINDOW && u8Provider_Item == u8Provider_Current_Item )
	{
		SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
		FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

		if ( Count_index%2 == 1 )
			SetBrushColor(hdc, MVAPP_GRAY_COLOR);
		else
			SetBrushColor(hdc, MVAPP_DARKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(rc1.left + 2), ScalerHeigthPixel(rc1.top + 2),ScalerWidthPixel(rc1.right - rc1.left - 4),ScalerHeigthPixel(rc1.bottom - rc1.top - 4));
		
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
	}  
	else 
	{	
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		if ( Count_index%2 == 1 )
			SetBrushColor(hdc, MVAPP_GRAY_COLOR);
		else
			SetBrushColor(hdc, MVAPP_DARKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));
	}
	//printf("%d : Provider  =====>\n", u8Provider_Item);
	MV_Get_Provider_Data_ByIndex(&stTempProvider, u8Cas_Focus, u8Provider_Item);

	rc1.left += 6;
	rc1.right -= 6;
	rc1.top += 4;
	rc1.bottom = rc1.top + TOP_WINDOW_DY;
	sprintf(TempStr1, "%02x%02x%02x%02x", stTempProvider->ProviderId[0], stTempProvider->ProviderId[1], stTempProvider->ProviderId[2], stTempProvider->ProviderId[3] ); 
	CS_MW_DrawText(hdc, TempStr1, -1, &rc1, DT_CENTER | DT_VCENTER);
	
	rc1.top += TOP_WINDOW_DY;
	rc1.bottom = rc1.top + TOP_WINDOW_DY;
	sprintf(TempStr2, "%s", stTempProvider->ProviderName);
	//printf("%d : Provider %s : %s =====>\n", u8Provider_Item, TempStr1, TempStr2);
	CS_MW_DrawText(hdc, TempStr2, -1, &rc1, DT_LEFT | DT_VCENTER);

	if ( bFocus == FOCUS || u8Window_Status != KEYEDIT_PROVIDER_WINDOW )
	{
		Scroll_Rect.top = PROVIDER_WINDOW_Y;
		Scroll_Rect.left = PROVIDER_WINDOW_X + PROVIDER_WINDOW_DX;
		Scroll_Rect.right = Scroll_Rect.left + SCROLL_BAR_DX;
		Scroll_Rect.bottom = PROVIDER_WINDOW_Y + PROVIDER_WINDOW_DY;
		MV_Draw_ScrollBar(hdc, Scroll_Rect, u8Provider_Item, u8NumberOfProvider, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_Porvider_Window(HDC hdc)
{
	int			i = 0;
	int			index = 0;
	
	SetBrushColor(hdc, MVAPP_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(PROVIDER_WINDOW_X), ScalerHeigthPixel(PROVIDER_WINDOW_Y),ScalerWidthPixel(PROVIDER_WINDOW_DX),ScalerHeigthPixel(PROVIDER_WINDOW_DY));
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	/* For 0 provider case by KB Kim 2012.06.07 */
	if (u8NumberOfProvider == 255)
	{
		return;
	}
	
	for ( i = 0 ; i < PROVIDER_ITEM_MAX ; i++ )
	{
		index = u16Provider_Current_Page * PROVIDER_ITEM_MAX + i;

		if ( u8NumberOfProvider < index )
			break;
		
		if ( i == u8Provider_Focus && u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
			MV_Draw_Provider_Item(hdc, i, index, FOCUS);
		else
			MV_Draw_Provider_Item(hdc, i, index, UNFOCUS);
	}

	//printf("MV_Draw_Porvider_Window : ==================\n");
}

void MV_Draw_Provider_Focus(HWND hwnd, U8 u8Focusindex, U8 FocusKind)
{
	HDC		hdc;
	int 	Count_index;
	
	/* For 0 provider case by KB Kim 2012.06.07 */
	if (u8NumberOfProvider == 255)
	{
		return;
	}
	
	hdc = BeginPaint(hwnd);
	
	Count_index = u8Focusindex%PROVIDER_ITEM_MAX;

	if ( u16Provider_Prev_Page != u16Provider_Current_Page && FocusKind == FOCUS )
		MV_Draw_Porvider_Window(hdc);
	else
		MV_Draw_Provider_Item(hdc, Count_index, u8Focusindex, FocusKind);
	
	EndPaint(hwnd,hdc);
}

void MV_Draw_Key_Item(HDC hdc, int Key_Item, BOOL bFocus)
{	
	char		TempStr[512];
	RECT		rc1;

	/* For 0 key case by KB Kim 2012.06.07 */
	if (u8NumberOfKey == 255)
	{
		return;
	}
	
	rc1.left = KEY_WINDOW_X;
	rc1.top = KEY_WINDOW_Y + ( TOP_WINDOW_DY * Key_Item );
	rc1.right = rc1.left + KEY_WINDOW_DX;
	rc1.bottom = rc1.top + TOP_WINDOW_DY;

	if ( bFocus == FOCUS && u8Window_Status == KEYEDIT_KEY_WINDOW )
	{	
		SetTextColor(hdc,CSAPP_BLACK_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
		FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));
	} else {	
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		
		if ( Key_Item%2 == 1 )
			SetBrushColor(hdc, MVAPP_GRAY_COLOR);
		else
			SetBrushColor(hdc, MVAPP_DARKBLUE_COLOR);
		
		FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));
	}

	rc1.left = KEY_WINDOW_X;
	rc1.top = KEY_WINDOW_Y + ( TOP_WINDOW_DY * Key_Item );
	rc1.right = rc1.left + 50;
	rc1.bottom = rc1.top + TOP_WINDOW_DY;

	if ( stKeyTemp_Data[Key_Item].KeyNumber == 0x20 )
		sprintf(TempStr, "Mod");
	else if ( stKeyTemp_Data[Key_Item].KeyNumber == 0x30 )
		sprintf(TempStr, "Exp");
	else if ( stKeyTemp_Data[Key_Item].KeyNumber > 0x20 && stKeyTemp_Data[Key_Item].KeyNumber < 0x30 )
		memset(TempStr, 0x00, 512);
	else 
		sprintf(TempStr, "%02x", stKeyTemp_Data[Key_Item].KeyNumber);
	
	CS_MW_DrawText(hdc, TempStr, -1, &rc1, DT_LEFT | DT_VCENTER);
	
	rc1.left = KEY_WINDOW_X + 50;
	rc1.top = KEY_WINDOW_Y + ( TOP_WINDOW_DY * Key_Item ) + 4;
	rc1.right = rc1.left + KEY_WINDOW_DX - 50;
	rc1.bottom = rc1.top + TOP_WINDOW_DY;
	
	sprintf(TempStr, "%s", stKeyTemp_Data[Key_Item].KeyData);
	
	CS_MW_DrawText(hdc, TempStr, -1, &rc1, DT_CENTER | DT_VCENTER);
}

void MV_Draw_Key_window(HDC hdc)
{
	int			i;
//	CasData_t	*stTemp_Cas = NULL;
//	KeyData_t	*stTemp_Key = NULL;
	
	SetBrushColor(hdc, MVAPP_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(KEY_WINDOW_X), ScalerHeigthPixel(KEY_WINDOW_Y),ScalerWidthPixel(KEY_WINDOW_DX),ScalerHeigthPixel(KEY_WINDOW_DY));
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	/* For 0 key case by KB Kim 2012.06.07 */
	if (u8NumberOfKey == 255)
	{
		return;
	}
	
	for ( i = 0 ; i < KEY_ITEM_MAX ; i++ )
	{
		if ( i > u8NumberOfKey )
			break;

		if ( i == u8Key_Focus && u8Window_Status == KEYEDIT_KEY_WINDOW )
			MV_Draw_Key_Item(hdc, i, FOCUS);
		else
			MV_Draw_Key_Item(hdc, i, UNFOCUS);
	}
}

void MV_Draw_Cas_Help(HDC hdc)
{
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y),ScalerWidthPixel(1280 - MV_HELP_ICON_X*2),ScalerHeigthPixel(TOP_WINDOW_DY));
	
	switch((U8)u8Window_Status)
	{
		case KEYEDIT_CAS_WINDOW:
/*
			{
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY));
			}
*/
			break;
			
		case KEYEDIT_KEY_WINDOW:
		case KEYEDIT_PROVIDER_WINDOW:
			{
#ifdef USE_ADD
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY));
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_ADD));
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_OK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_OK_ICON].bmHeight), &MV_BMP[MVBMP_OK_ICON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 3), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_EDIT));
#else
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_OK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_OK_ICON].bmHeight), &MV_BMP[MVBMP_OK_ICON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 3), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_EDIT));
#endif
			}
			break;
		default:
			break;
	}
}

void MV_Draw_Full_Cas_Window(HDC hdc)
{
	MV_Draw_Window_Title(hdc);
	MV_Draw_Cas_Window(hdc);
	MV_Draw_Porvider_Window(hdc);
	MV_Draw_Key_window(hdc);
}

static int KeyEdit_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 				hdc;
	char				acKey_Value[256];
 	ProviderData_t		nullProvider;
  
	switch(message)
	   	{
			case MSG_CREATE:
				memset (&stCasDBInfo, 0x00, sizeof(CasDbInfo_t));
				memset(acKey_Value, 0x00, 256);
/*				
				//CasDrvGetKeyDbInfo(&stCasDBInfo);

				u8NumberOfCas = stCasDBInfo.NumberOfCas;
				u8NumberOfProvider = stCasDBInfo.CasData_P->TotalNumberOfProvider;
				u8KindOfKey = stCasDBInfo.CasData_P->Provider->TotalNumberOfKey;
				u8Key_length = stCasDBInfo.CasData_P->Provider->Key->KeyLength;
*/
				CasDrvGetKeyDbInfo(&stCasDBInfo);
				u8Provider_Focus = 0;
				u8Provider_Current_Item = 0;
				u16Provider_Current_Page = 0;
				u16Provider_Prev_Page = 0;
				u8Key_Focus = 0;
				u8Cas_Focus = 0;
				u8Add_or_Edit = KEYEDIT_EDIT;

				MV_Get_Cas_Data_ByIndex(&stCurrentCas, u8Cas_Focus);
				/* For 0 provider case by KB Kim 2012.06.07 */
				if (stCurrentCas->TotalNumberOfProvider > 0)
				{
					u8NumberOfProvider = stCurrentCas->TotalNumberOfProvider - 1;
					MV_Get_Provider_Data_ByIndex(&stCurrentProvider, u8Cas_Focus, u8Provider_Current_Item);
					MV_Set_Cas_KeyData_Reform(u8Cas_Focus, u8Provider_Current_Item);
				}
				else
				{
					u8NumberOfProvider = 255;
					u8NumberOfKey = 255;
					stCurrentProvider = &nullProvider;
				}
							
//				MV_Get_Provider_Data_ByIndex(&stCurrentProvider, u8Cas_Focus, u8Provider_Focus);
				//u8NumberOfKey = stCurrentProvider->TotalNumberOfKey - 1;
				u8Window_Status = KEYEDIT_CAS_WINDOW;
				break;
				
			case MSG_PAINT:
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_TOOL, EN_ITEM_FOCUS_CAS);
				hdc=BeginPaint(hwnd);
				MV_Draw_Full_Cas_Window(hdc);
				MV_Draw_Cas_Help(hdc);
				EndPaint(hwnd,hdc);
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
								BOOL		u8Return_Bool = FALSE;
								U8          u8TmpKeyNumber;
								
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);
										
								if ( u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
								{
									/* For Provider Delete Problem By KB Kim 2012.06.07 */
									u8Return_Bool = CasDrvDeleteProvider (stCurrentCas->CasId, stCurrentProvider->ProviderId);
								}
								else if ( u8Window_Status == KEYEDIT_KEY_WINDOW )
								{
									/* For Key Delete Problem By KB Kim 2012.06.07 */
									u8TmpKeyNumber = stKeyTemp_Data[u8Key_Focus].KeyNumber;
									if (u8TmpKeyNumber > CAS_KEY_TYPE_PK0)
									{
										u8TmpKeyNumber &= 0xF0;
									}

									u8Return_Bool = CasDrvDeleteKey (stCurrentCas->CasId, stCurrentProvider->ProviderId, u8TmpKeyNumber, 0);
									printf("Delete Key : Cas[0x%02X], Provider[0x%02X], Key[0x%02X], Result[%d]\n", 
										stCurrentCas->CasId[0], stCurrentProvider->ProviderId[0], u8TmpKeyNumber, u8Return_Bool);
								}

								if ( u8Return_Bool == TRUE )
								{
									CasDrvSaveKey2Flase();
									
									if ( u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
									{
										/* For Provider Delete by KB Kim 2012.06.07 */
										if (u8NumberOfProvider > 0)
										{
											u8NumberOfProvider--;
											
											if ( u8Provider_Current_Item > u8NumberOfProvider )
												u8Provider_Current_Item = u8NumberOfProvider;

											u8Provider_Focus = get_focus_line(&u16Provider_Current_Page, u8Provider_Current_Item, PROVIDER_ITEM_MAX);
											u8Key_Focus = 0;
											MV_Get_Provider_Data_ByIndex(&stCurrentProvider, u8Cas_Focus, u8Provider_Current_Item);
											MV_Set_Cas_KeyData_Reform(u8Cas_Focus, u8Provider_Current_Item);
										}
										else
										{
											u8NumberOfProvider = 255;
											u8NumberOfKey = 255;
											u8Provider_Focus = 0;
											stCurrentProvider = &nullProvider;
										}
										
										hdc = BeginPaint(hwnd);
										MV_Draw_Porvider_Window(hdc);
										MV_Draw_Key_window(hdc);
										EndPaint(hwnd,hdc);
									}
									else if ( u8Window_Status == KEYEDIT_KEY_WINDOW )
									{
										/* For 0 key case by KB Kim 2012.06.07 */
										MV_Set_Cas_KeyData_Reform(u8Cas_Focus, u8Provider_Current_Item);
										if ( u8NumberOfKey < 255)
										{
											if ( u8Key_Focus > u8NumberOfKey )
											{
												u8Key_Focus = u8NumberOfKey;
											}
											/* For Key Delete by KB Kim 2012.06.07 */
										}
										hdc=BeginPaint(hwnd);
										MV_Draw_Key_window(hdc);
										EndPaint(hwnd,hdc);
									}
								}
							}
							else
							{
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);								
							}
						}
						else
						{
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);								
						}
					}

					if (wparam != CSAPP_KEY_IDLE)
					{
						break;
					}
					else
					{
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);								
					}

				}
				
				if ( MV_Get_HexaKeypad_Status() == TRUE )
				{
					MV_HexaKeypad_Proc(hwnd, wparam);
					
					if ( wparam == CSAPP_KEY_ENTER )
					{
						strcpy(acKey_Value, MV_Get_HexaEdited_String());
						
						if ( u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
						{
							char		Splite_Str[4];
							int			i, j = 0, k = 0;
							U8			TempU8;

							if (u8Add_or_Edit == KEYEDIT_ADD)
							{
								/* Add Mode */
								stCurrentProvider = &nullProvider;
								memset( stCurrentProvider->ProviderName, 0x00, strlen(stCurrentProvider->ProviderName));
							}

							k = strlen(acKey_Value);
							for ( i = 0 ; i < k ; i++ )
							{
								Splite_Str[i%2] = acKey_Value[i];
								if ( i%2 == 1 && i != 0 )
								{
									Str2Hex(Splite_Str, &TempU8);	
									//printf("i : %02d , j : %d ===== %s = %02x\n", i, j, Splite_Str, TempU8);
									stCurrentProvider->ProviderId[j] = TempU8;
									j++;
								}					
							}

							if (u8Add_or_Edit == KEYEDIT_ADD)
							{
								if (u8NumberOfProvider == 255)
								{
									u8NumberOfProvider = 0;
								}
								else
								{
									u8NumberOfProvider++;
								}
								u8NumberOfKey = 255;
								u8Provider_Current_Item = u8NumberOfProvider;
								u8Provider_Focus = get_focus_line(&u16Provider_Current_Page, u8Provider_Current_Item, PROVIDER_ITEM_MAX);
								CasDrvUpdateProvider (stCurrentCas->CasId,  stCurrentProvider->ProviderId, stCurrentProvider->ProviderName);
								MV_Get_Provider_Data_ByIndex(&stCurrentProvider, u8Cas_Focus, u8Provider_Current_Item); /* Reload Current Provicer pointer */
								memset(nullProvider.ProviderId, 0x00, CAS_PROVIDER_ID_LENGTH);
								u8Add_or_Edit = KEYEDIT_EDIT;
							}

							printf("Provider ID : %02x%02x%02x%02x===========\n", stCurrentProvider->ProviderId[0], stCurrentProvider->ProviderId[1], stCurrentProvider->ProviderId[2], stCurrentProvider->ProviderId[3]);
							CasDrvSaveKey2Flase();
							hdc = BeginPaint(hwnd);
							MV_Draw_Porvider_Window(hdc);
							MV_Draw_Key_window(hdc);
							EndPaint(hwnd,hdc);
							
							/* For Provider Name Edit problem by KB Kim 2012.06.07 */
							sprintf(acKey_Value, "%s", stCurrentProvider->ProviderName);
							u8Key_length = CAS_PROVIDER_NAME_LENGTH - 1;
							MV_Draw_StringKeypad(hwnd, acKey_Value, u8Key_length);
						} 
						else if ( u8Window_Status == KEYEDIT_KEY_WINDOW )
						{
							U32  dataLength;
							BOOL u8Return_Bool = FALSE;
	
							dataLength = strlen(acKey_Value);
							if (u8Add_or_Edit == KEYEDIT_ADD)
							{
								if (u8AddKeyNumber == CAS_KEYTYPE_INVALID)
								{
									if ((dataLength > 0) && (dataLength <= 2))
									{
										if (MV_Get_Cas_KeyNumber(stCurrentCas->CasId, acKey_Value, (U8)dataLength, &u8AddKeyNumber, &u8AddKeyLength))
										{
											memset(acKey_Value, 0x00, sizeof(acKey_Value));
											u8Add_or_Edit = KEYEDIT_ADD;
											MV_Draw_HexaKeypad(hwnd, acKey_Value, (u8AddKeyLength * 2));
										}
									}
								}
								else
								{
									printf("== %s \n", acKey_Value );
									u8Return_Bool = MV_Cas_KeyData_Add(stCurrentCas, stCurrentProvider, u8AddKeyNumber, acKey_Value);
									if (u8Return_Bool == TRUE)
									{
										MV_Set_Cas_KeyData_Reform(u8Cas_Focus, u8Provider_Current_Item);
										u8Key_Focus = u8NumberOfKey;
										hdc = BeginPaint(hwnd);
										MV_Draw_Key_window(hdc);
										EndPaint(hwnd,hdc);
									}
								}
							}
							if (u8Add_or_Edit == KEYEDIT_EDIT)
							{
								strncpy( stKeyTemp_Data[u8Key_Focus].KeyData, acKey_Value, dataLength);
								printf("== %s \n", acKey_Value );
								printf("== %s \n", stKeyTemp_Data[u8Key_Focus].KeyData );
								u8Return_Bool = MV_Cas_KeyData_Update(stCurrentProvider, u8Key_Focus);
								hdc=BeginPaint(hwnd);
								MV_Draw_Key_Item(hdc, u8Key_Focus, FOCUS);
								EndPaint(hwnd,hdc);
							}
							if ( u8Return_Bool == TRUE )
							{
								CasDrvSaveKey2Flase();
							}
						}
					}
					
					break;
				}

				if ( MV_Get_StringKeypad_Status() == TRUE )
				{
					MV_StringKeypad_Proc(hwnd, wparam);
					
					if ( wparam == CSAPP_KEY_ENTER )
					{
						memset( acKey_Value , 0x00, 256);
						strcpy(acKey_Value, MV_Get_StringEdited_String());

						if ( u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
						{
							MV_Get_Provider_Data_ByIndex(&stCurrentProvider, u8Cas_Focus, u8Provider_Current_Item);
							memset( stCurrentProvider->ProviderName, 0x00, strlen(stCurrentProvider->ProviderName));
							strncpy( stCurrentProvider->ProviderName, acKey_Value, strlen(acKey_Value));
							MV_Draw_Provider_Focus(hwnd, u8Provider_Current_Item, FOCUS);
							CasDrvSaveKey2Flase();
						}
					}
					break;
				}
				
				switch(wparam)
				{
#ifdef USE_ADD
					case CSAPP_KEY_RED:
						if (((u8Window_Status == KEYEDIT_PROVIDER_WINDOW) && (u8NumberOfProvider != 255)) ||
							((u8Window_Status == KEYEDIT_KEY_WINDOW) && (u8NumberOfKey != 255)))
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SURE);
						break;
						
					case CSAPP_KEY_GREEN:
						if ( u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
						{
							sprintf(acKey_Value, "00000000");
							printf("=== %s ===\n", acKey_Value);
							u8Add_or_Edit = KEYEDIT_ADD;
							MV_Draw_HexaKeypad(hwnd, acKey_Value, strlen(acKey_Value));
						} else if ( u8Window_Status == KEYEDIT_KEY_WINDOW ){
							if (stCurrentCas->CasId[0] != CAS_ID_BISS)
							{
								sprintf(acKey_Value, "00");
								printf("=== %s ===\n", acKey_Value);
								u8AddKeyNumber = CAS_KEYTYPE_INVALID;
								u8AddKeyLength = 0;
								u8Add_or_Edit = KEYEDIT_ADD;
								MV_Draw_HexaKeypad(hwnd, acKey_Value, strlen(acKey_Value));
							}
						}
						break;
#endif
					case CSAPP_KEY_UP:
						if ( u8Window_Status == KEYEDIT_CAS_WINDOW )
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Cas_Item(hdc, u8Cas_Focus, UNFOCUS);
							EndPaint(hwnd,hdc);
							
							if ( u8Cas_Focus == 0 )
								u8Cas_Focus = stCasDBInfo.NumberOfCas - 1;
							else
								u8Cas_Focus--;

							u8Provider_Focus = 0;
							u8Provider_Current_Item = 0;
							u16Provider_Current_Page = 0;
							u16Provider_Prev_Page = 0;
							u8Key_Focus = 0;

							MV_Get_Cas_Data_ByIndex(&stCurrentCas, u8Cas_Focus);
							
							//printf(" KEY UP ======> %d : Provider : %d\n", u8Cas_Focus, stCurrentCas->TotalNumberOfProvider);
							
							/* For 0 provider case by KB Kim 2012.06.07 */
							if (stCurrentCas->TotalNumberOfProvider > 0)
							{
								u8NumberOfProvider = stCurrentCas->TotalNumberOfProvider - 1;
								MV_Get_Provider_Data_ByIndex(&stCurrentProvider, u8Cas_Focus, u8Provider_Current_Item);
								MV_Set_Cas_KeyData_Reform(u8Cas_Focus, u8Provider_Current_Item);
							}
							else
							{
								u8NumberOfProvider = 255;
								u8NumberOfKey = 255;
								stCurrentProvider = &nullProvider;
							}
							
							hdc=BeginPaint(hwnd);
							MV_Draw_Cas_Item(hdc, u8Cas_Focus, FOCUS);
							MV_Draw_Porvider_Window(hdc);
							MV_Draw_Key_window(hdc);
							EndPaint(hwnd,hdc);
						}
						else if ( u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
						{
							/* For 0 provider case by KB Kim 2012.06.07 */
							if (u8NumberOfProvider != 255)
							{
								MV_Draw_Provider_Focus(hwnd, u8Provider_Current_Item, UNFOCUS);

								u16Provider_Prev_Page = u16Provider_Current_Page;

								if(u8Provider_Current_Item == 0)
									u8Provider_Current_Item = u8NumberOfProvider;
								else
									u8Provider_Current_Item--;

								u8Provider_Focus = get_focus_line(&u16Provider_Current_Page, u8Provider_Current_Item, PROVIDER_ITEM_MAX);
								
								u8Key_Focus = 0;

								MV_Draw_Provider_Focus(hwnd, u8Provider_Current_Item, FOCUS);

								MV_Get_Provider_Data_ByIndex(&stCurrentProvider, u8Cas_Focus, u8Provider_Current_Item);
								MV_Set_Cas_KeyData_Reform(u8Cas_Focus, u8Provider_Current_Item);
								//u8NumberOfKey = stCurrentProvider->TotalNumberOfKey - 1;
								
								hdc=BeginPaint(hwnd);
								MV_Draw_Key_window(hdc);
								EndPaint(hwnd,hdc);
							}
						}
						else 
						{
							/* For 0 key case by KB Kim 2012.06.07 */
							if (u8NumberOfKey != 255)
							{
								hdc=BeginPaint(hwnd);
								MV_Draw_Key_Item(hdc, u8Key_Focus, UNFOCUS);
								EndPaint(hwnd,hdc);

								if ( u8Key_Focus == 0 )
									u8Key_Focus = u8NumberOfKey;
								else
									u8Key_Focus--;

								hdc=BeginPaint(hwnd);
								MV_Draw_Key_Item(hdc, u8Key_Focus, FOCUS);
								EndPaint(hwnd,hdc);
							}
						}
						break;
						
					case CSAPP_KEY_DOWN:
						if ( u8Window_Status == KEYEDIT_CAS_WINDOW )
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Cas_Item(hdc, u8Cas_Focus, UNFOCUS);
							EndPaint(hwnd,hdc);

							if ( u8Cas_Focus == stCasDBInfo.NumberOfCas - 1 )
								u8Cas_Focus = 0;
							else
								u8Cas_Focus++;

							u8Provider_Focus = 0;
							u8Provider_Current_Item = 0;
							u16Provider_Current_Page = 0;
							u16Provider_Prev_Page = 0;
							u8Key_Focus = 0;

							MV_Get_Cas_Data_ByIndex(&stCurrentCas, u8Cas_Focus);
							
							//printf(" KEY DOWN ======> %d : Provider : %d\n", u8Cas_Focus, stCurrentCas->TotalNumberOfProvider);
							
							/* For 0 provider case by KB Kim 2012.06.07 */
							if (stCurrentCas->TotalNumberOfProvider > 0)
							{
								u8NumberOfProvider = stCurrentCas->TotalNumberOfProvider - 1;
								MV_Get_Provider_Data_ByIndex(&stCurrentProvider, u8Cas_Focus, u8Provider_Current_Item);
								MV_Set_Cas_KeyData_Reform(u8Cas_Focus, u8Provider_Current_Item);
							}
							else
							{
								u8NumberOfProvider = 255;
								u8NumberOfKey = 255;
								stCurrentProvider = &nullProvider;
							}

							hdc=BeginPaint(hwnd);
							MV_Draw_Cas_Item(hdc, u8Cas_Focus, FOCUS);
							MV_Draw_Porvider_Window(hdc);
							MV_Draw_Key_window(hdc);
							EndPaint(hwnd,hdc);
						}
						else if ( u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
						{
							/* For 0 provider case by KB Kim 2012.06.07 */
							if (u8NumberOfProvider != 255)
							{
								//printf("=========== %d ===============\n", u8Provider_Current_Item);
								MV_Draw_Provider_Focus(hwnd, u8Provider_Current_Item, UNFOCUS);

								u16Provider_Prev_Page = u16Provider_Current_Page;

								if(u8Provider_Current_Item == u8NumberOfProvider)
									u8Provider_Current_Item = 0;
								else
									u8Provider_Current_Item++;

								u8Provider_Focus = get_focus_line(&u16Provider_Current_Page, u8Provider_Current_Item, PROVIDER_ITEM_MAX);
								
								u8Key_Focus = 0;

								MV_Draw_Provider_Focus(hwnd, u8Provider_Current_Item, FOCUS);

								MV_Get_Provider_Data_ByIndex(&stCurrentProvider, u8Cas_Focus, u8Provider_Current_Item);
								//u8NumberOfKey = stCurrentProvider->TotalNumberOfKey - 1;
								MV_Set_Cas_KeyData_Reform(u8Cas_Focus, u8Provider_Current_Item);
								
								hdc=BeginPaint(hwnd);
								MV_Draw_Key_window(hdc);
								EndPaint(hwnd,hdc);
							}
						}
						else 
						{
							/* For 0 key case by KB Kim 2012.06.07 */
							if (u8NumberOfKey != 255)
							{
								hdc=BeginPaint(hwnd);
								MV_Draw_Key_Item(hdc, u8Key_Focus, UNFOCUS);
								EndPaint(hwnd,hdc);

								if ( u8Key_Focus == u8NumberOfKey )
									u8Key_Focus = 0;
								else
									u8Key_Focus++;

								hdc=BeginPaint(hwnd);
								MV_Draw_Key_Item(hdc, u8Key_Focus, FOCUS);
								EndPaint(hwnd,hdc);
							}
						}
						break;

					case CSAPP_KEY_LEFT:
						if ( u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
						{							
							u8Window_Status = KEYEDIT_CAS_WINDOW;
							
							MV_Draw_Provider_Focus(hwnd, u8Provider_Current_Item, UNFOCUS);

							hdc=BeginPaint(hwnd);
							MV_Draw_Cas_Item(hdc, u8Cas_Focus, FOCUS);
							MV_Draw_Cas_Help(hdc);
							EndPaint(hwnd,hdc);							
						}
						else if ( u8Window_Status == KEYEDIT_KEY_WINDOW )
						{
							u8Window_Status = KEYEDIT_PROVIDER_WINDOW;
							
							hdc=BeginPaint(hwnd);
							MV_Draw_Key_Item(hdc, u8Key_Focus, UNFOCUS);
							EndPaint(hwnd,hdc);	

							MV_Draw_Provider_Focus(hwnd, u8Provider_Current_Item, FOCUS);
							
							hdc=BeginPaint(hwnd);
							MV_Draw_Cas_Help(hdc);
							EndPaint(hwnd,hdc);
						}
						break;
						
					case CSAPP_KEY_RIGHT:
						if ( u8Window_Status == KEYEDIT_CAS_WINDOW )
						{
							u8Window_Status = KEYEDIT_PROVIDER_WINDOW;
							
							hdc=BeginPaint(hwnd);
							MV_Draw_Cas_Item(hdc, u8Cas_Focus, UNFOCUS);
							EndPaint(hwnd,hdc);	

							MV_Draw_Provider_Focus(hwnd, u8Provider_Current_Item, FOCUS);
							hdc=BeginPaint(hwnd);
							MV_Draw_Cas_Help(hdc);
							EndPaint(hwnd,hdc);
						}
						else if ( u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
						{
							u8Window_Status = KEYEDIT_KEY_WINDOW;
							
							MV_Draw_Provider_Focus(hwnd, u8Provider_Current_Item, UNFOCUS);
							
							//printf("CSAPP_KEY_RIGHT : %d =====> \n", u8Key_Focus);
							hdc=BeginPaint(hwnd);
							MV_Draw_Key_Item(hdc, u8Key_Focus, FOCUS);
							MV_Draw_Cas_Help(hdc);
							EndPaint(hwnd,hdc);	
						}
						break;
#ifdef USE_ADD
					case CSAPP_KEY_YELLOW:
#else
					case CSAPP_KEY_RED:
#endif
					case CSAPP_KEY_ENTER:
						if ( u8Window_Status == KEYEDIT_PROVIDER_WINDOW )
						{
							/* For 0 provider case by KB Kim 2012.06.07 */
							if (u8NumberOfProvider != 255)
							{
								sprintf( acKey_Value, "%02x%02x%02x%02x", stCurrentProvider->ProviderId[0], stCurrentProvider->ProviderId[1], stCurrentProvider->ProviderId[2], stCurrentProvider->ProviderId[3]);
								printf("PROVIDER === %s ===\n", acKey_Value);

								u8Add_or_Edit = KEYEDIT_EDIT;
								MV_Draw_HexaKeypad(hwnd, acKey_Value, strlen(acKey_Value));
							}
						}
						else if( u8Window_Status == KEYEDIT_KEY_WINDOW )
						{
							/* For 0 key case by KB Kim 2012.06.07 */
							if (u8NumberOfKey != 255)
							{
								sprintf( acKey_Value, stKeyTemp_Data[u8Key_Focus].KeyData );
								printf("KEY === %s ===\n", acKey_Value);

								u8Add_or_Edit = KEYEDIT_EDIT;
								MV_Draw_HexaKeypad(hwnd, acKey_Value, strlen(acKey_Value));
							}
						}
						break;
						
					case CSAPP_KEY_ESC:
						CSApp_KeyEdit_Applets=CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_MENU:
						CSApp_KeyEdit_Applets=b8Last_App_Status;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_IDLE:
						CSApp_KeyEdit_Applets = CSApp_Applet_Sleep;
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
				DestroyMainWindow(hwnd);
				PostQuitMessage(hwnd);
				break;

		   	default:
				break;
	   	}
	
   return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

