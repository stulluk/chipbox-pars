/*****************************************************************************

File name   : panel.c

Description : Front Panel Driver API


History:
-----------------------------------------------------------------------------
14/09/07	Created 			by 	Simon
24/12/07	Updated 			by 	Simon
			Add support DSD418 Front Panel
			Because IC CH450 is not a full I2C Compatible device. Sti2c is not suit for it,
			So we use two other GPIOs as Serial bus
			
18/01/08	Updated 			by 	Simon
			Add support DSD418 Front Panel
*****************************************************************************/

#include "linuxos.h"
#include "fpanel.h"


#define ERROR_PANEL		
#define DEBBG_PANEL		


#ifdef DEBBG_PANEL
#define DBG_PANEL(x)		printf x
#else
#define DBG_PANEL(x)
#endif

#ifdef ERROR_PANEL
#define ERR_PANEL(x)		printf x
#else
#define ERR_PANEL(x)
#endif





#define PANEL_REPEAT_GUARD	10	// 10*20=200ms

static void  (*panelf_KeyNotify)(PanelT_KeyEvent iKeyEvent, unsigned char bKeyCode);

static const char*  g_Revision  = "Panel-REL_1.2.0";

static BOOL panelv_bInitialized = FALSE;

typedef struct _Seg8Encode
{
	unsigned char bChar;
	unsigned char bEnc;
}PanelT_Seg8Encode;

static const PanelT_Seg8Encode panelv_pbSeg8Encode[] = {
    {'0', 0x3F},
	{'1', 0x06},
	{'2', 0x5B},
	{'3', 0x4F},
	{'4', 0x66},
	{'5', 0x6D},
	{'6', 0x7D},
	{'7', 0x07},
	{'8', 0x7F},
	{'9', 0x6F},
	{'A', 0x77},
	{'a', 0x5F},
	{'B', 0x7C},
	{'b', 0x7C},
	{'C', 0x39},
	{'c', 0x58},
	{'D', 0x5E},
	{'d', 0x5E},
	{'E', 0x79},
	{'e', 0x79},
	{'F', 0x71},
	{'f', 0x71},
	{'G', 0x3D},
	{'g', 0x6F},
	{'H', 0x76},
	{'h', 0x74},
	{'I', 0x06},
	{'i', 0x10},
	{'J', 0x0E},
	{'j', 0x0E},
	{'L', 0x38},
	{'l', 0x06},
	{'N', 0x37},
	{'n', 0x54},
	{'O', 0x3F},
	{'o', 0x5C},
	{'P', 0x73},
	{'p', 0x73},
	{'Q', 0x67},
	{'q', 0x67},
	{'R', 0x70},
	{'r', 0x70},
	{'S', 0x6D},
	{'s', 0x6D},
	{'T', 0x78},
	{'t', 0x78},
	{'U', 0x3E},
	{'u', 0x1C},
	{'V', 0x3E},
	{'v', 0x1C},
	{'Y', 0x6E},
	{'y', 0x6E},
	{' ', 0x00},
	{'-', 0x40},
	{'_', 0x08}
};




#define TM1618_CMD_DISPMODE		0x00
#define TM1618_CMD_DATAWR		0x40
#define TM1618_BIT_FIXEDADDR	0x04

#define TM1618_CMD_DISPCTRL		0x80
#define TM1618_CMD_SETADDR		0xC0

static CSGPIO_HANDLE	 Diohandle=0,Clkhandle=0,Stbhandle=0;

#define DIO_PIO_BIT			Diohandle//(1<<4)
#define CLK_PIO_BIT			Clkhandle//(1<<5)
#define STB_PIO_BIT			Stbhandle//(1)

static CSGPIO_HANDLE	 KeyDatahandle=0,KeyClkhandle=0,KeyPLhandle=0;

#define KEYDATA_PIO_BIT		KeyDatahandle//(1<<14)
#define KEYCLK_PIO_BIT			KeyClkhandle//(1<<9)
#define KEYPL_PIO_BIT			KeyPLhandle//(1<<1)

static PanelT_LedColor panelv_iBiColor = PANELT_LED_DARK;

static unsigned char panelv_pbRegValue[8];

static unsigned short panelv_wCom5Value = 0;

static void panelf_task(_IN_ void *pTaskParams);

static void GPIOF_WritePIO(CSGPIO_HANDLE gpio,BOOL value)
{
	CSGPIO_Write(gpio,value);
}

static void GPIOF_ReadPIO(CSGPIO_HANDLE gpio,char* value)
{
	CSGPIO_Read(gpio,value);
}

static void OS_delayus(int us)
{
	int i=0,j=0,ii=0;

	//us=20;
	for(ii=0;ii<us;ii++)
		for(i=0;i<2000;i++)j=i;
	
}

static void panelf_SPI_STB(BOOL bLevel)
{
	if (bLevel)
		GPIOF_WritePIO(STB_PIO_BIT, TRUE);
	else
		GPIOF_WritePIO(STB_PIO_BIT, FALSE);
}

static void panelf_SPI_CLK(BOOL bLevel)
{
	if (bLevel)
		GPIOF_WritePIO(CLK_PIO_BIT, TRUE);
	else
		GPIOF_WritePIO(CLK_PIO_BIT, FALSE);
}

static void panelf_SPI_DIO(BOOL bLevel)
{
	if (bLevel)
		GPIOF_WritePIO(DIO_PIO_BIT, TRUE);
	else
		GPIOF_WritePIO(DIO_PIO_BIT, FALSE);
}

static ERR_CODE panelf_SPI_Write(_IN_ unsigned char *pbData, _IN_ unsigned char bLen)
{
	unsigned char i, j;
	unsigned char bData;
	
	if (NULL == pbData)
		return FAIL;

	//OS_delayus(10);
	OS_delayus(10);
	panelf_SPI_STB(FALSE);
	for (i=0; i<bLen; i++)
	{
		OS_delayus(10);
		bData = *pbData++;
		for(j=0; j<8; j++)
		{
			panelf_SPI_CLK(FALSE);
 			if(bData & 0x01)
				panelf_SPI_DIO(TRUE);
 			else
				panelf_SPI_DIO(FALSE);
 			bData >>= 1;
			OS_delayus(10);
			panelf_SPI_CLK(TRUE);
			OS_delayus(10);
		}
	}
	panelf_SPI_STB(TRUE);
	
	return OK;
}

static unsigned short panelf_segenc(unsigned char b7Seg)
{
	unsigned short w16Seg = 0;

	if(b7Seg & 0x01)
		w16Seg |= 0x1000;	// Seg13		'a'
	if(b7Seg & 0x02)
		w16Seg |= 0x0800;	// Seg12		'b'
	if(b7Seg & 0x04)
		w16Seg |= 0x0010;	// Seg5		'c'
	if(b7Seg & 0x08)
		w16Seg |= 0x0008;	// Seg4		'd'
	if(b7Seg & 0x10)
		w16Seg |= 0x0004;	// Seg3		'e'
	if(b7Seg & 0x20)
		w16Seg |= 0x0002;	// Seg2		'f'
	if(b7Seg & 0x40)
		w16Seg |= 0x0001;	// Seg1		'g'
	if(b7Seg & 0x80)
		w16Seg |= 0x2000;	// Seg14		'dot'
//	printf("panelf_segenc %x ----> %x\n", b7Seg, w16Seg);
	
	return w16Seg;
}

ERR_CODE PanelF_DisplayOFF(void)
{
	unsigned char bCommand[6];

	bCommand[0]= TM1618_CMD_DISPCTRL;
	return panelf_SPI_Write(bCommand, 1);
}

ERR_CODE PanelF_DisplayON(void)
{
	unsigned char bCommand[6];

	bCommand[0]= TM1618_CMD_DISPCTRL | 0x08 | 0x07;
	return panelf_SPI_Write(bCommand, 1);
}


ERR_CODE  PanelF_DisplayString(_IN_ const char *pString)
{
	int i, j, iCharNum;
	char ch;
	unsigned short w16SegEnc;
	unsigned char pbData[12];

	if (pString == NULL)
		return FAIL;
	PanelF_DisplayOFF();

	printf("Led:%s   \n",pString);

	pbData[0] = TM1618_CMD_DISPMODE;
	panelf_SPI_Write(pbData, 1);	
	pbData[0] =TM1618_CMD_DATAWR;
	panelf_SPI_Write(pbData, 1);	
	pbData[0] = TM1618_CMD_SETADDR;
	iCharNum = sizeof(panelv_pbSeg8Encode) / sizeof(PanelT_Seg8Encode);
	for(i=0; i<4; i++)
	{
		ch = *pString++;
			
		for(j=0; j<iCharNum; j++)
		{
			if(panelv_pbSeg8Encode[j].bChar == ch)
			{
				w16SegEnc = panelf_segenc(panelv_pbSeg8Encode[j].bEnc);
				pbData[2*i+1] = (unsigned char)(w16SegEnc & 0xFF);
				pbData[2*i+2] = (unsigned char)((w16SegEnc>>8) & 0xFF);
				break;
			}
		}
		if (j==iCharNum)
		{
			w16SegEnc = panelf_segenc(0);
			pbData[2*i+1] = (unsigned char)(w16SegEnc & 0xFF);
			pbData[2*i+2] = (unsigned char)((w16SegEnc>>8) & 0xFF);
		}
	}
	
	for(i=0; i<8; i++)
		panelv_pbRegValue[i] = pbData[i+1];

	if(PANELT_LED_GREEN == panelv_iBiColor)
	{
		pbData[5] |= 0x01;
	}
	else if(PANELT_LED_RED == panelv_iBiColor)
	{
		pbData[5] |= 0x01;
	}
	else if(PANELT_LED_YELLOW == panelv_iBiColor)
	{
		pbData[5] |= 0x01;
		pbData[7] |= 0x01;
	}

	panelf_SPI_Write(pbData, 9);	

	PanelF_DisplayON();
	return OK;
 }


ERR_CODE  PanelF_DisplayTime(_IN_ const unsigned char bHour, _IN_ const unsigned char bMin)
{
	int i, j, iCharNum;
	char ch[4];
	unsigned short w16SegEnc;
	unsigned char pbData[12];

	if (bHour >= 24)
		return FAIL;

	if (bMin >= 60)
		return FAIL;

	ch[0] = '0' + (bHour / 10);
	ch[1] = '0' + (bHour % 10);
	ch[2] = '0' + (bMin / 10);
	ch[3] = '0' + (bMin % 10);

	pbData[0] = TM1618_CMD_DISPMODE;
	panelf_SPI_Write(pbData, 1);	
	pbData[0] = TM1618_CMD_SETADDR;
	panelf_SPI_Write(pbData, 1);	// Set Register 0	
	pbData[0] = TM1618_CMD_DATAWR;
	
	iCharNum = sizeof(panelv_pbSeg8Encode) / sizeof(PanelT_Seg8Encode);
	for(i=0; i<4; i++)
	{
		for(j=0; j<iCharNum; j++)
		{
			if(panelv_pbSeg8Encode[j].bChar == ch[i])
			{
				w16SegEnc = panelf_segenc(panelv_pbSeg8Encode[j].bEnc);
				pbData[2*i+1] = (unsigned char)(w16SegEnc & 0xFF);
				pbData[2*i+2] = (unsigned char)((w16SegEnc>>8) & 0xFF);
				break;
			}
		}
		if (j==iCharNum)
		{
			w16SegEnc = panelf_segenc(0);
			pbData[2*i+1] = (unsigned char)(w16SegEnc & 0xFF);
			pbData[2*i+2] = (unsigned char)((w16SegEnc>>8) & 0xFF);
		}
	}

	for(i=0; i<8; i++)
		panelv_pbRegValue[i] = pbData[i+1];

	pbData[3] |= 0x01;
	if(PANELT_LED_GREEN == panelv_iBiColor)
	{
		pbData[7] |= 0x01;
	}
	else if(PANELT_LED_RED == panelv_iBiColor)
	{
		pbData[5] |= 0x01;
	}
	else if(PANELT_LED_YELLOW == panelv_iBiColor)
	{
		pbData[5] |= 0x01;
		pbData[7] |= 0x01;
	}

	panelf_SPI_Write(pbData, 9);	

	PanelF_DisplayON();

	return OK;
}

ERR_CODE  PanelF_SetBiColorLED(_IN_ PanelT_LedColor iLedColor)
{
	unsigned char pbData[12];
	
	pbData[0] = TM1618_CMD_DISPMODE;
	panelf_SPI_Write(pbData, 1);	

	switch(iLedColor)
	{
		case PANELT_LED_GREEN:
			panelv_iBiColor = PANELT_LED_GREEN;
			break;

		case PANELT_LED_RED:
			panelv_iBiColor = PANELT_LED_RED;
			break;

		case PANELT_LED_YELLOW:
			panelv_iBiColor = PANELT_LED_YELLOW;
			break;

		case PANELT_LED_DARK:
			panelv_iBiColor = PANELT_LED_DARK;
			break;

		default:
			ERR_PANEL(("[PanelF_SetBiColorLED]Unknown Color(%d).\n", iLedColor));
			return FAIL;
	}
	
	if(PANELT_LED_GREEN == panelv_iBiColor)
	{
		pbData[0] = TM1618_CMD_SETADDR | 0x04;
		panelf_SPI_Write(pbData, 1);	
		pbData[0] = TM1618_CMD_DATAWR | TM1618_BIT_FIXEDADDR;
		pbData[1] = panelv_pbRegValue[4] & ~0x01;
		panelf_SPI_Write(pbData, 2);	

		pbData[0] = TM1618_CMD_SETADDR | 0x06;
		panelf_SPI_Write(pbData, 1);	
		pbData[0] = TM1618_CMD_DATAWR | TM1618_BIT_FIXEDADDR;
		pbData[1] = panelv_pbRegValue[6] | 0x01;
		panelf_SPI_Write(pbData, 2);	
	}
	else if(PANELT_LED_RED == panelv_iBiColor)
	{
		pbData[0] = TM1618_CMD_SETADDR | 0x04;
		panelf_SPI_Write(pbData, 1);	
		pbData[0] = TM1618_CMD_DATAWR | TM1618_BIT_FIXEDADDR;
		pbData[1] = panelv_pbRegValue[4] |= 0x01;
		panelf_SPI_Write(pbData, 2);	
		pbData[0] = TM1618_CMD_SETADDR | 0x06;
		panelf_SPI_Write(pbData, 1);	
		pbData[0] = TM1618_CMD_DATAWR | TM1618_BIT_FIXEDADDR;
		pbData[1] = panelv_pbRegValue[6] & ~0x01;
		panelf_SPI_Write(pbData, 2);	
	}
	else if(PANELT_LED_YELLOW == panelv_iBiColor)
	{
		pbData[0] = TM1618_CMD_SETADDR | 0x04;
		panelf_SPI_Write(pbData, 1);	
		pbData[0] = TM1618_CMD_DATAWR | TM1618_BIT_FIXEDADDR;
		pbData[1] = panelv_pbRegValue[4] | 0x01;
		panelf_SPI_Write(pbData, 2);	
		pbData[0] = TM1618_CMD_SETADDR | 0x06;
		panelf_SPI_Write(pbData, 1);	
		pbData[0] = TM1618_CMD_DATAWR | TM1618_BIT_FIXEDADDR;
		pbData[1] = panelv_pbRegValue[6] | 0x01;
		panelf_SPI_Write(pbData, 2);	
	}
	else
	{
		pbData[0] = TM1618_CMD_SETADDR | 0x04;
		panelf_SPI_Write(pbData, 1);	
		pbData[0] = TM1618_CMD_DATAWR | TM1618_BIT_FIXEDADDR;
		pbData[1] = panelv_pbRegValue[4] & ~0x01;
		panelf_SPI_Write(pbData, 2);	
		pbData[0] = TM1618_CMD_SETADDR | 0x06;
		panelf_SPI_Write(pbData, 1);	
		pbData[0] = TM1618_CMD_DATAWR | TM1618_BIT_FIXEDADDR;
		pbData[1] = panelv_pbRegValue[6] & ~0x01;
		panelf_SPI_Write(pbData, 2);	
	}

	PanelF_DisplayON();

	return OK;
}


#define FPANEL_STACK_SIZE	   	1024*8
static U8							FpanelStack[FPANEL_STACK_SIZE];
static CSOS_Task_Handle_t 		         FpanelTaskHandle;
static CSOS_TaskDesc_t*			FpanelTaskDesc;
static CSOS_TaskFlag_t				FpanelTaskFlag=0;
ERR_CODE  PanelF_Initialize(_IN_ PanelT_InitParams *pInitParams)
{
	CS_ErrorCode_t iErrCode;
	unsigned char bValue;
	CSAPI_RESULT	Status;

	if (panelv_bInitialized)	// Already initialized before
	{
		ERR_PANEL(("[PanelF_Initialize]Panel module already be initialized before\n"));
		return FAIL;
	}
	
	if (NULL == pInitParams)
	{
		ERR_PANEL(("[PanelF_Initialize]Parameter Invalid.\n"));
		return FAIL;
	}
	
	if (NULL == pInitParams->PanelKeyProc)
	{
		ERR_PANEL(("[PanelF_Initialize]Parameter Invalid.\n"));
		return FAIL;
	}

	DIO_PIO_BIT = CSGPIO_Open(4);
	if(DIO_PIO_BIT==0)
		{
			printf("DIO_PIO_BIT =%x\n",DIO_PIO_BIT);
		}
	Status=CSGPIO_SetDirection(DIO_PIO_BIT,GPIO_DIRECTION_WRITE);
	if(Status==CSAPI_FAILED)
		{
			printf("DIO_PIO_BIT direction error\n");
		}

	CLK_PIO_BIT = CSGPIO_Open(5);
	if(CLK_PIO_BIT==0)
		{
			printf("CLK_PIO_BIT =%x\n",CLK_PIO_BIT);
		}
	Status=CSGPIO_SetDirection(CLK_PIO_BIT,GPIO_DIRECTION_WRITE);
	if(Status==CSAPI_FAILED)
		{
			printf("CLK_PIO_BIT direction error\n");
		}
	
	STB_PIO_BIT = CSGPIO_Open(0);
	if(STB_PIO_BIT==0)
		{
			printf("STB_PIO_BIT =%x\n",STB_PIO_BIT);
		}
	Status=CSGPIO_SetDirection(STB_PIO_BIT,GPIO_DIRECTION_WRITE);
	if(Status==CSAPI_FAILED)
		{
			printf("STB_PIO_BIT direction error\n");
		}
	
	
	panelf_SPI_STB(TRUE); 
	bValue =TM1618_CMD_DISPMODE;
	panelf_SPI_Write(&bValue, 1);	
#if 0
		{char pbData[4];

		
				pbData[0] =0x48;//TM1618_CMD_DATAWR;
				panelf_SPI_Write(pbData, 1);	


				while(1);

			}
#endif	

	PanelF_DisplayString("CSM ");
	
	PanelF_DisplayON();


	// keyboard
	KEYDATA_PIO_BIT = CSGPIO_Open(14);
	if(KEYDATA_PIO_BIT==0)
		{
			printf("KEYDATA_PIO_BIT =%x\n",KEYDATA_PIO_BIT);
		}
	Status=CSGPIO_SetDirection(KEYDATA_PIO_BIT,GPIO_DIRECTION_READ);
	if(Status==CSAPI_FAILED)
		{
			printf("KEYDATA_PIO_BIT direction error\n");
		}

	KEYCLK_PIO_BIT = CSGPIO_Open(9);
	if(KEYCLK_PIO_BIT==0)
		{
			printf("KEYCLK_PIO_BIT =%x\n",KEYCLK_PIO_BIT);
		}
	Status=CSGPIO_SetDirection(KEYCLK_PIO_BIT,GPIO_DIRECTION_WRITE);
	if(Status==CSAPI_FAILED)
		{
			printf("KEYCLK_PIO_BIT direction error\n");
		}
	GPIOF_WritePIO(KEYCLK_PIO_BIT, TRUE);
	
	KEYPL_PIO_BIT = CSGPIO_Open(1);
	if(KEYPL_PIO_BIT==0)
		{
			printf("KEYPL_PIO_BIT =%x\n",KEYPL_PIO_BIT);
		}
	Status=CSGPIO_SetDirection(KEYPL_PIO_BIT,GPIO_DIRECTION_WRITE);
	if(Status==CSAPI_FAILED)
		{
			printf("KEYPL_PIO_BIT direction error\n");
		}
	GPIOF_WritePIO(KEYPL_PIO_BIT, TRUE);

#if 1  
	iErrCode=CSOS_CreateTask((void *)(panelf_task),
								NULL,
								NULL,
								FPANEL_STACK_SIZE,
								FpanelStack,
								NULL,
								&FpanelTaskHandle,
								&FpanelTaskDesc,
								10,
								"FPANEL",
								FpanelTaskFlag);
	if(iErrCode!=CS_NO_ERROR)
		return FAIL;
	iErrCode=CSOS_StartTask(FpanelTaskHandle);
	if(iErrCode!=CS_NO_ERROR)
		return FAIL;
 #endif

	panelf_KeyNotify = pInitParams->PanelKeyProc;

	panelv_bInitialized = TRUE;
	
	return OK;
}

const char*   EK_PanelF_GetRevision( void )
{
	return g_Revision;
}

static void panelf_KEY_PL(BOOL bLevel)
{
	if (bLevel)
		GPIOF_WritePIO(KEYPL_PIO_BIT, TRUE);
	else
		GPIOF_WritePIO(KEYPL_PIO_BIT, FALSE);
}

static void panelf_KEY_Clk(BOOL bLevel)
{
	if (bLevel)
		GPIOF_WritePIO(KEYCLK_PIO_BIT, TRUE);
	else
		GPIOF_WritePIO(KEYCLK_PIO_BIT, FALSE);
}

static void panelf_KEY_Data(char * bLevel)
{	
	GPIOF_ReadPIO(KEYDATA_PIO_BIT, bLevel);	
}

static ERR_CODE panelf_ScanKeyboard(_IN_ unsigned char *pbKeyCode)
{
	unsigned char bKeyCode = 0,i,Value=0;

	//parallel loading
	panelf_KEY_PL(FALSE);
	OS_delayus(10);
	panelf_KEY_PL(TRUE);

	//serial shift
	panelf_KEY_Clk(FALSE);
	OS_delayus(5);
	for(i=0;i<8;i++)
		{
			panelf_KEY_Clk(TRUE);
			OS_delayus(5);
			panelf_KEY_Data(&Value);
			printf("Value=%x   ",Value);
			bKeyCode+=(Value<<i);
			panelf_KEY_Clk(FALSE);
			OS_delayus(5);
		}


	*pbKeyCode = bKeyCode;
	printf("bKeyCode=0x%x\n",bKeyCode);
	return OK;
	
}

static void panelf_task(_IN_ void *pTaskParams)
{
	unsigned char bKeyCode = 0;
	unsigned char bLastKeyCode = 0;
	unsigned int dwRepeatCounter = 0;
	BOOL bKeyDownFlag = FALSE;
	
	
	while(TRUE)
	{
		if (panelf_ScanKeyboard(&bKeyCode) == OK)
		{
			if (bKeyCode != 0)	// some key down
			{
				bLastKeyCode = bKeyCode;
				if (!bKeyDownFlag)
				{
					bKeyDownFlag = TRUE;
					dwRepeatCounter = 0;
					if (panelf_KeyNotify)
					{
						panelf_KeyNotify(PANELT_KEY_DOWN, bKeyCode);
						//panelf_KeyNotify(0xfd,bKeyCode,TRUE);
					}
				}
				else
				{
					if (dwRepeatCounter++ > PANEL_REPEAT_GUARD)
					{
						dwRepeatCounter = 0;
						if (panelf_KeyNotify)
							panelf_KeyNotify(PANELT_KEY_REPEAT, bKeyCode);
							//panelf_KeyNotify(0xfd,bKeyCode,TRUE);
					}
				}
			}
			else	// No key detect 
			{
				if (bKeyDownFlag)	// key not released before
				{
					if (panelf_KeyNotify)
						panelf_KeyNotify(PANELT_KEY_UP, bLastKeyCode);
						//panelf_KeyNotify(0xfd,bKeyCode,FALSE);
					bKeyDownFlag = FALSE;
					dwRepeatCounter = 0;
				}
			}
		}
		CSOS_DelayTaskMs(100);	// 20ms
#if 0
{char pdata[5];

		pdata[0]='0';
		pdata[1]='1';
		pdata[2]='2';
		pdata[3]='3';
		pdata[4]=0;
		
		PanelF_DisplayString(pdata);
}
#endif

		
	}
}


// EOF


