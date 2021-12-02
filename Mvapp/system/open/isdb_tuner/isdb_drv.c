#include "linuxos.h"
#include "isdb_i2capi.h"
#include "isdb_drv.h"

//#define SAMUNG

static unsigned char TDHL5_44MHz_InitReg[] =
{
#ifdef SAMUNG
	0x01, 0x04,			// Wakeup condition setting -> 1: Do not restore to demodulation operation at TS error (rlock="0¡±)
	0x13,0x2c,
	0x14,0x22,
	0x15,0x00,
	0x16, 0x0A,			// For XCKO spurious prevention, stop output be setting register ixckosl(0x16[3]) to "1".
	0x1C, 0x20,			/// used STSFLG
	0x1D, 0xA2,		/// signal output -> Parallel(0xA2), Serial(0xAA)
	0x1E, 0xA8,		/// signal output -> Parallel(0xA8), Serial(0xA2)
	0x20, 0x00,			// Delay point (RF/IF changeover point) setting -> IF_AGC lower limit
	0x22, 0x8C,			// AGC control method changeover (rfif[7]) : 1: IF_AGC control mode, 0x80 -> 0x8C (Ver.0.80, yskoo)
	//0x23, 0x38,		/// IFAGC setting
	0x30, 0x20,		/// Digital IF frequency inversion(f_inv=0, 0x30[3]), if IF=44MHz
	0x31, 0x11,		/// Carrier frequency offset value setting, cpld_dt[13:8], if IF=44MHz
	0x32, 0x2B,		/// Carrier frequency offset value setting, cpld_dt[7:0], if IF=44MHz
	0x50, 0x04,			// Frequency CPE removal ON/OFF setting -> 1:OFF
	0x51, 0x58,			// C/N threshold value for CVI processing
	0x58, 0x60,			// Pilot interference detection level threshold
	//0x75, 0x20,		/// rlocksw[5] -> For RS error monitoring in units of TS packet, (Ver.0.40, yskoo) 
	0xC2, 0x10,			// ADC power save setting -> 1: Power save (can be set with differential input only)
	0xEF, 0x01,			// ISIC(inter-symbol interference suppressing circuit)/Continuous FFT window search changeover setting -> 1: ISIC ON
#else
	0x01, 0x04,			// Wakeup condition setting -> 1: Do not restore to demodulation operation at TS error (rlock="0¡±)
	0x16, 0x0A,			// For XCKO spurious prevention, stop output be setting register ixckosl(0x16[3]) to "1".
	0x1C, 0x20,			/// used STSFLG
	0x1D, 0xA2,		/// signal output -> Parallel(0xA2), Serial(0xAA)
	0x1E, 0xA8,		/// signal output -> Parallel(0xA8), Serial(0xA2)
	0x20, 0x00,			// Delay point (RF/IF changeover point) setting -> IF_AGC lower limit
	0x22, 0x8C,			// AGC control method changeover (rfif[7]) : 1: IF_AGC control mode, 0x80 -> 0x8C (Ver.0.80, yskoo)
	0x23, 0x38,		/// IFAGC setting
	0x30, 0x20,		/// Digital IF frequency inversion(f_inv=0, 0x30[3]), if IF=44MHz
	0x31, 0x11,		/// Carrier frequency offset value setting, cpld_dt[13:8], if IF=44MHz
	0x32, 0x2B,		/// Carrier frequency offset value setting, cpld_dt[7:0], if IF=44MHz
	0x50, 0x04,			// Frequency CPE removal ON/OFF setting -> 1:OFF
	0x51, 0x58,			// C/N threshold value for CVI processing
	0x58, 0x60,			// Pilot interference detection level threshold
	0x75, 0x20,		/// rlocksw[5] -> For RS error monitoring in units of TS packet, (Ver.0.40, yskoo) 
	0xC2, 0x10,			// ADC power save setting -> 1: Power save (can be set with differential input only)
	0xEF, 0x01,			// ISIC(inter-symbol interference suppressing circuit)/Continuous FFT window search changeover setting -> 1: ISIC ON
#endif
};

static U8 ALP514_pucConfigData[ALP514_DATA_SIZE];
static U8 ALP514_ucMOPReadData;

static U8 TC90507_ucFFTSize;
static char FFTSize_pcStr[4][10] = {"2K", "4K", "8K", "?"};

static U8 TC90507_ucGI;		// Guard ratio(interval)
static char GI_pcStr[4][10] = {"1/32", "1/16", "1/8", "1/4"};  // Guard Interval


static U8 TC90507_ucSystemID;
static char SysID_pcStr[3][10] = {"TV", "Sound", "?"};// System ID

// TMCC partial reception
static char ParReceptionTV_pcStr[2][10] = {"OFF", "ON"};		/// ON and OFF is changed (Ver.0.30, yskoo) 
static char ParReceptionSound_pcStr[2][10] = {"1-SEG", "3-SEG"};

// Combined transmission phase correction volume
static char PhaseCorrect_pcStr[8][10] = {"-¥ð/8", "-2¥ð/8", "-3¥ð/8", "-4¥ð/8", "-5¥ð/8", "-6¥ð/8", "-7¥ð/8", "OFF"};

// TMCC layer X carrier modulation system
static char Modulation_pcStr[8][10] = {"DQPSK", "QPSK", "16QAM", "64QAM", "?", "?", "?", "None"};

// TMCC layer X convolution coding rate
static char CR_pcStr[8][10] = {"1/2", "2/3", "3/4", "5/6", "7/8", "?", "?", "None"};

// TMCC layer X time interleave system
static char TI1_pcStr[8][10] = {"0", "4", "8", "16", "32", "?", "?", "None"};
static char TI2_pcStr[8][10] = {"0", "2", "4", "8", "16", "?", "?", "None"};
static char TI3_pcStr[8][10] = {"0", "1", "2", "4", "8", "?", "?", "None"};

// Number of segments used by layer X
static char NumSeg_pcStr[16][10] = {"0", "1", "2", "3", "4", "5", "6", 
										"7", "8", "9", "10", "11", "12", "13", "14", "None"};

static void Sleep(unsigned int count)
{
	usleep(count*1000);
}

// Register(System) Reset(Auto Recovery) 
// This function is included Sleep(1), 1ms Delay
static int  TC90507_Set_RegisterReset(void)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	
	iRetVal=TC90507_RegWriteMaskBit8(0x01, TC90507_BIT_7, TC90507_BIT_7);		// isyrst

	Sleep(1);		// Minimum Delay 4us after System reset
	
	return iRetVal;
}

/**************************************************************************
 *	DTV Receiver Initialization
 *************************************************************************/
// This function is included TC90507_Set_RegisterReset()
int TC90507_Set_RegInit(void)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	int iRegSize;
	int iRegCnt=0;
	U8 ucRegAddr, ucRegData;


	iRetVal=TC90507_Set_RegisterReset();

	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		iRegSize=sizeof(TDHL5_44MHz_InitReg);		// DATA8=1
		while(iRegCnt<iRegSize)
		{
			ucRegAddr=TDHL5_44MHz_InitReg[iRegCnt];
			ucRegData=TDHL5_44MHz_InitReg[iRegCnt+1];
			iRetVal=TC90507_RegWrite8(ucRegAddr, ucRegData);
			if (iRetVal!=ALPS_I2C_ERR_NON)
				break;
			else
				iRegCnt=iRegCnt+2;
		}
	}

	return iRetVal;
}

// Demodulator(Operation(Sequence)) Reset (Auto Recovery)
// This function is called when a channel is changed, LNA is on/off and unlock flag is set,
// But the register values are not reset.
int TC90507_Set_DemodReset(void)
{
	int iRetVal=ALPS_I2C_ERR_NON;

	// OFDM Demodulator Reset
	if (iRetVal==ALPS_I2C_ERR_NON)
		iRetVal=TC90507_RegWriteMaskBit8(0x01, TC90507_BIT_6, TC90507_BIT_6);	// imsrst
	
	return iRetVal;
}


// ucMode : TC90507_CH_SELECTION_MODE(0), TC90507_CH_SEARCH_MODE(1)
int TC90507_Set_ChannelMode(U8 ucMode)
{
	int iRetVal=ALPS_I2C_ERR_NON;

	if (ucMode==TC90507_CH_SELECTION_MODE)
	{
		// If the transmission mode is already known, the synchronization time can be shortened by
		// setting ¡°1¡± in register mdetsel and the FFT size and guard length in registers ffsize[1:0] and
		// gdleng[1:0]. In this case, the transmission mode is automatically searched if the set transmission
		// mode does not match the transmitted signal.

		/* mdetsel ->
			Whether mode search is performed upon demodulation operation initial pull-in is selected.
				0: Mode search ON upon initial pull-in
				1: Mode search OFF upon initial pull-in.
			However, mode search is performed upon re-pull-in.
		*/

		/* mlocksel -> 
			fulock flag output changeover
				0: Frame synchronization flag
				1: SP detection flag
			Valid for fulock only. Not reflected to FLOCK.
		*/
		iRetVal=TC90507_RegWriteMaskBit8(0x47, TC90507_BIT_7 | TC90507_BIT_6, TC90507_BIT_7);		// mdetsel(1), mlocksel(0)
	}
	else
	{
		iRetVal=TC90507_RegWriteMaskBit8(0x47, TC90507_BIT_7 | TC90507_BIT_6, TC90507_BIT_6);		// mdetsel(0), mlocksel(1)
	}

	return iRetVal;
}


/**************************************************************************
 *	Power Save(Sleep Mode, Standby Mode)
 *************************************************************************/
// ucMode : TC90507_ENABLE(0), TC90507_DISABLE(1)
// TC90507_DISABLE -> included TC90507_Set_DemodReset()
int TC90507_Set_SleepMode(U8 ucMode)
{
	int iRetVal=ALPS_I2C_ERR_NON;

	// slptim[6:4] : Sleep time -> 1-7: Sleep time ((2slptim-|1) ¡¿ Wakeup time)
	// wuptim[3:0] : Wakeup time -> 0: No demodulation
	if (ucMode==TC90507_ENABLE)
	{
		/// Add, IF AGC Ouput 0V when sleepmode is enable (Ver.0.70, yskoo)
		// IF_AGC gain control manual setting selection
		iRetVal|=TC90507_RegWriteMaskBit8(0x23, TC90507_BIT_0, TC90507_BIT_0);	// ifmgcon
		// IF gain manual setting, Sets the IFMGC level output from the AGCCNTI pin in SB format when ifmgcon = 1.
//		iRetVal|=TC90507_RegWrite8(0x25, 0x00);		// ifmgc
		
		iRetVal=TC90507_RegWrite8(0x03, 0x40);		// slptim, wuptim -> 0.14A
		iRetVal|=TC90507_RegWriteMaskBit8(0x04, TC90507_BIT_5, TC90507_BIT_5);	// adcpwn -> 0.02A
	}
	else
	{
		// IF_AGC gain control manual setting selection
		iRetVal|=TC90507_RegWriteMaskBit8(0x23, TC90507_BIT_0, !TC90507_BIT_0);	// ifmgcon
		
		iRetVal=TC90507_RegWrite8(0x03, 0x00);
		iRetVal|=TC90507_RegWriteMaskBit8(0x04, TC90507_BIT_5, 0);	// adcpwn

		if (iRetVal==ALPS_I2C_ERR_NON)
			iRetVal=TC90507_Set_DemodReset();
	}

	return iRetVal;
}



void CTDA6651_Init( float RFFREQUENCY )
{
	int MOPLL_IC = 1;	
	unsigned char Data[6], Read;
	unsigned int T_AD, T_DB1, T_DB2, T_CB1_0, T_CB1_1, T_CB2;
	float IF;
	float CH_FREQ;
	float STEP_SIZE;
	float OSC;

	//AfxMessageBox("checking");  // CHECKING FUNC CALL
	
	CH_FREQ = RFFREQUENCY/1000;//557.143;//RFFREQUENCY;
	printf("CH_FREQ=%f\n",CH_FREQ);
	// TDA6651 I2C ADDRESS
	T_AD = 0xc2;

	IF = 44.0;

	STEP_SIZE = 142.86;

	int DIVIDER;
	float temp;

	OSC= IF + CH_FREQ;
    temp=((OSC*1000)/STEP_SIZE);
	printf("OSC frequency=%f\n",OSC);
//    temp=((OSC*1000+(STEP_SIZE/2))/STEP_SIZE);
	DIVIDER = (int)(temp+0.6);
	//printf("DIVIDER=0x%x\n",DIVIDER);
//	DIVIDER = int(temp); 

	//DIVIDER=0x1070;

	T_DB1 = ((int)DIVIDER>>8)&0x7f;
	T_DB2 = ((int)DIVIDER)&0x00ff;


	
	// CONTROL BYTE (CB1)
	// m_ckATC = 1
	T_CB1_1 = 0xC1;
	// m_ckATC = 0
	T_CB1_0 = 0x81;
	
	// BAND SWITCH BYTE (BB)
	if(( CH_FREQ >= 50.0 ) && ( CH_FREQ <= 168.0 ))
	{
		T_CB2 = 0x41;
	}
	else if(( CH_FREQ >= 168.0 ) && ( CH_FREQ <= 470.0 ))
	{
		T_CB2 = 0xC1;
	}
	else if((CH_FREQ >= 470.0 ) && (CH_FREQ <= 867.0 ))
	{
		T_CB2 = 0xC8;//0x68;
	}
	
	// TDA6651 REGISTER DATA
	Data[0] = T_AD;							// I2C address b 0011 0010
	
	Data[0] = Data[0]&0xfe;					// omit bit#0
	Data[1] = T_DB1&0xff;					// save input T_DB1 8bit into dataout[1]
	Data[2] = T_DB2&0xff;					// save input T_DB2 8bit into dataout[2]
	Data[3] = T_CB1_1&0xff;					// save input T_CB 8bit data into dataout[3]
	Data[4] = T_CB2&0xff;					// save input T_BB 8bit data into dataout[4]
	Data[5] = T_CB1_0&0xff;					// save input T_BB 8bit data into dataout[4]

	//printf("0x%.2x,0x%.2x,0x%.2x,0x%.2x,0x%.2x,0x%.2x\n",Data[0],Data[1],Data[2],Data[3],Data[4],Data[5]);

	int i;
	for( i = 0; i < 5; i++ )
	{
		TC90507_Bypass_WriteArray(6,Data);
		Sleep(25);
		TC90507_Bypass_ReadArray(Data[0]|0x01,1,&Read);
		if(((Read)&0x40) != 0 )break;
	}
	

}


/**************************************************************************
 * Demodulator - Tuner Frequency Change
 *************************************************************************/
// TC90507_Set_DemodReset() must be called after this function
int TC90507_Send_BypassData(unsigned int uiFreqKHz)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucMOPReadAddress;
	U16 usDividerRatio;
	double dLOFreqHz;
	double	ALP514_dRefDividerHz=142857;
	double	ALP514_dIFHz=44.0*1.0e6;
	float		FrequencyTemp;

	FrequencyTemp=uiFreqKHz;
	//printf("FrequencyTemp=%f\n",FrequencyTemp);
#ifdef SAMUNG
	CTDA6651_Init(FrequencyTemp);

	return ALPS_I2C_ERR_NON;

#endif

	ALP514_pucConfigData[ALP514_INDEX_ADDRESS]=0xC2;
	ALP514_pucConfigData[ALP514_INDEX_CONTROL1]=0x89;
	ALP514_pucConfigData[ALP514_INDEX_CONTROL3]=0xF6;

	// (Input_Freq + IF) / (X'tal_Freq/Reference_Counter)
	dLOFreqHz = (uiFreqKHz*1.0e3) + ALP514_dIFHz;
	//printf("dLOFreqHz=%f\n",dLOFreqHz);
	usDividerRatio = (int) floor(0.5 + (dLOFreqHz / ALP514_dRefDividerHz));
	ALP514_pucConfigData[ALP514_INDEX_DIVIDER1] = (U8) (0x7f & (usDividerRatio >> 8));
	ALP514_pucConfigData[ALP514_INDEX_DIVIDER2] = (U8) (0xff & usDividerRatio);

	if (uiFreqKHz <= 216000)
		ALP514_pucConfigData[ALP514_INDEX_CONTROL2] = 0xE2;
	else if (uiFreqKHz <= 530000) 
		ALP514_pucConfigData[ALP514_INDEX_CONTROL2] = 0xEA;
	else 
		ALP514_pucConfigData[ALP514_INDEX_CONTROL2] = 0x2A;

	if (iRetVal==ALPS_I2C_ERR_NON)
		iRetVal=TC90507_Bypass_WriteArray(ALP514_DATA_SIZE, ALP514_pucConfigData);

	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		Sleep(50);

		ucMOPReadAddress=ALP514_pucConfigData[0]|0x01;
		iRetVal=TC90507_Bypass_ReadArray(ucMOPReadAddress, 1, &ALP514_ucMOPReadData);
	}

	return iRetVal;
}


void TC90507_Get_MOPConfigData(unsigned char *pucWriteData, unsigned char *pucReadData)
{
	int i;
		
	for (i=0; i<6; i++)
		pucWriteData[i]=ALP514_pucConfigData[i];

	*pucReadData=ALP514_ucMOPReadData;
}


/**************************************************************************
 *	I/O Setting & Error Correction (BER Monitor) 
 *************************************************************************/
// RS decoding can be turned ON and OFF by setting the register rsoff.
// ucMode : TC90507_ENABLE(0), TC90507_DISABLE(1)
int TC90507_Set_RSDecoding(U8 ucMode)
{
	int iRetVal=ALPS_I2C_ERR_NON;

	if (ucMode==TC90507_ENABLE)
		iRetVal=TC90507_RegWriteMaskBit8(0x71, TC90507_BIT_7, 0);	// rsoff
	else
		iRetVal=TC90507_RegWriteMaskBit8(0x71, TC90507_BIT_7, TC90507_BIT_7);

	return iRetVal;
}


// Serial Output for BER Measurement.
// Setting register beron to ¡°1¡± outputs the serial data and the serial clock for BER measurement instead of
// TS serial output. The data to be output is TS synchronization byte 47 and 187-byte data excluding parity.
// (Excluding OFDM multiplexed null packets.)
// It is necessary to set PRBS on SG for BER measurement.
// ucMode => TC90507_TS_SERIALOUT_NORMAL(0), TC90507_TS_SERIALOUT_BER(1)
int TC90507_Set_SerialBEROut(U8 ucMode)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	
	iRetVal=TC90507_RegWriteMaskBit8(0x75, TC90507_BIT_6, (ucMode&0x01)<<6);	// beron[6]
	
	return iRetVal;
}


// TC90507 is provided with independent TS parallel and TS serial pins, allowing separate output control.
// If both are enabled, simultaneous output is also possible.
// ucMode => TC90507_ENABLE(0), TC90507_DISABLE(1)
int TC90507_Set_TSSerialOut(U8 ucMode)
{
	int iRetVal=ALPS_I2C_ERR_NON;

	if (ucMode==TC90507_DISABLE)
		ucMode=TC90507_DISABLE+1;	// 2: Fixed to ¡°0
		
	iRetVal=TC90507_RegWriteMaskBit8(0x1E, TC90507_BIT_3 | TC90507_BIT_2, (ucMode&0x03)<<2);	// Clock&Data, sroen[3:2]
	
	return iRetVal;
}


// TC90507 is provided with independent TS parallel and TS serial pins, allowing separate output control.
// If both are enabled, simultaneous output is also possible.
// ucMode => TC90507_ENABLE(0), TC90507_DISABLE(1)
int TC90507_Set_TSParallelOut(U8 ucMode)
{
	int iRetVal=ALPS_I2C_ERR_NON;

	if (ucMode==TC90507_DISABLE)
		ucMode+=TC90507_DISABLE;	// 2: Fixed to ¡°0
	
	/// Add TS byte clock output disable when TS is serial (Ver.0.70, yskoo)
	iRetVal|=TC90507_RegWriteMaskBit8(0x1D, TC90507_BIT_1 | TC90507_BIT_0, (ucMode&0x03)<<0);	// Clock, rsckooen[1:0]
	iRetVal|=TC90507_RegWriteMaskBit8(0x1D, TC90507_BIT_3 | TC90507_BIT_2, (ucMode&0x03)<<2);	// Data, rsoutoen[3:2]
	
	return iRetVal;
}


// Replaces the specified layer with null packets.
// ucLayerA, ucLayerB, ucLayerC : TC90507_ENABLE(0), TC90507_DISABLE(1)
// TC90507_DISABLE(1) => NULL Packet Set
int TC90507_Set_TSOutputLayer(U8 ucLayerA, U8 ucLayerB, U8 ucLayerC)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	// [2]:A [1]:B [0]:C, TRUE => Null packet
	ucTemp=((ucLayerA&0x01)<<2)|((ucLayerB&0x01)<<1)|((ucLayerC&0x01)<<0);
	iRetVal=TC90507_RegWriteMaskBit8(0x71, TC90507_BIT_2 | TC90507_BIT_1 | TC90507_BIT_0, ucTemp);	// laysel[2:0]
	
	return iRetVal;
}


// For BER Measurement on equipment.
// Error measurement mode setting.
// ucMode => TC90507_BER_AFTER_RS(0), TC90507_BER_AFTER_VITERBI(1)
// 0: Number of error packets count mode after RS decoding, 
// 1: Number of error bits count mode after Viterbi decoding
int TC90507_Set_BERMornitorType(U8 ucMode)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	
	iRetVal=TC90507_RegWriteMaskBit8(0x77, TC90507_BIT_0, (ucMode&0x01)<<0);	// cor[0]
	
	return iRetVal;
}

int Set_TSOutputLayer(U8 Layer)
{
	int iRetVal=ALPS_I2C_ERR_NON;

	// All Layer = 0,   Layer A = 3,   Layer B = 5,   Layer C = 6
	if (Layer==TC90507_TS_OUTPUT_LAYER_ALL)
		iRetVal=TC90507_Set_TSOutputLayer(0, 0, 0);	// TC90507_DISABLE(1) => NULL Packet Set
	else if (Layer==TC90507_TS_OUTPUT_LAYER_A)
		iRetVal=TC90507_Set_TSOutputLayer(0, 1, 1);
	else if (Layer==TC90507_TS_OUTPUT_LAYER_B)
		iRetVal=TC90507_Set_TSOutputLayer(1, 0, 1);
	else
		iRetVal=TC90507_Set_TSOutputLayer(1, 1, 0);

	return iRetVal;
}


// At least one packet error presence/absence check in layer X frames
// Output value => TC90507_RS_ERROR(0), TC90507_RS_NONERROR(1)
int TC90507_Get_RSErrorStatus(TC90507_ucRSERROR_t *ptStatus)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0x96, &ucTemp);	// rlocka[7], rlockb[6], rlockc[5]
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		ptStatus->ucStatus_LayerA=(ucTemp>>7)&0x01;
		ptStatus->ucStatus_LayerB=(ucTemp>>6)&0x01;
		ptStatus->ucStatus_LayerC=(ucTemp>>5)&0x01;
	}
	
	return iRetVal;
}


// Output value => (x.xx)
int TC90507_Get_BER(TC90507_LAYER_dBER_t *ptBER)
{
	int 	iRetVal=ALPS_I2C_ERR_NON;
	U8 	ucBerMode;
	U32 	uiBERCount;
	U16 	usBERCycle;
	U8 	pucTemp[3];

	iRetVal=TC90507_RegRead8(0x77, &pucTemp[0]);	// cor
	if (iRetVal==ALPS_I2C_ERR_NON)
		ucBerMode=pucTemp[0] & 0x01;

	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		// Error Numbers
		iRetVal=TC90507_RegRead8Array(0x9D, 3, pucTemp);	// perra
		uiBERCount  = (pucTemp[0]<<16);
		uiBERCount |= (pucTemp[1]<<8);
		uiBERCount |= (pucTemp[2]<<0);

		// Packet Numbers
		iRetVal|=TC90507_RegRead8Array(0xA6, 2, pucTemp);	// pecya
		usBERCycle  = (pucTemp[0]<<8);
		usBERCycle |= (pucTemp[1]<<0);
	}
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		if (ucBerMode==TC90507_BER_AFTER_RS)
			ptBER->dBER_LayerA=((uiBERCount*8*0.5*9)/(usBERCycle*204*8));
		else
			ptBER->dBER_LayerA=((double)uiBERCount/(usBERCycle*204*8));
	}

	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		// Error Numbers
		iRetVal=TC90507_RegRead8Array(0xA0, 3, pucTemp);	// perrb
		uiBERCount  = (pucTemp[0]<<16);
		uiBERCount |= (pucTemp[1]<<8);
		uiBERCount |= (pucTemp[2]<<0);

		// Packet Numbers
		iRetVal|=TC90507_RegRead8Array(0xA8, 2, pucTemp);	// pecyb
		usBERCycle  = (pucTemp[0]<<8);
		usBERCycle |= (pucTemp[1]<<0);
	}
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		if (ucBerMode==TC90507_BER_AFTER_RS)
			ptBER->dBER_LayerB=((uiBERCount*8*0.5*9)/(usBERCycle*204*8));
		else
			ptBER->dBER_LayerB=((double)uiBERCount/(usBERCycle*204*8));
	}

	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		// Error Numbers
		iRetVal=TC90507_RegRead8Array(0xA3, 3, pucTemp);	// perrc
		uiBERCount  = (pucTemp[0]<<16);
		uiBERCount |= (pucTemp[1]<<8);
		uiBERCount |= (pucTemp[2]<<0);

		// Packet Numbers
		iRetVal|=TC90507_RegRead8Array(0xAA, 2, pucTemp);	// pecyc
		usBERCycle  = (pucTemp[0]<<8);
		usBERCycle |= (pucTemp[1]<<0);
	}
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		if (ucBerMode==TC90507_BER_AFTER_RS)
			ptBER->dBER_LayerC=((uiBERCount*8*0.5*9)/(usBERCycle*204*8));
		else
			ptBER->dBER_LayerC=((double)uiBERCount/(usBERCycle*204*8));
	}


	return iRetVal;
}


/**************************************************************************
 *	Monitor Output (The receiving mode (FFT size, guard length), 
 *	FFT window position, TMCC information, and various receiving states
 *	can be monitored. Further, constellation and spectrum can be observed.
 *************************************************************************/
// IF_AGC control level monitor output (*pucAGC => 0~255)
int TC90507_Get_IFAGC(U8 *pucAGC)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0x82, &ucTemp);	// ifagc_dt[7:0]
	if (iRetVal==ALPS_I2C_ERR_NON)
		*pucAGC=ucTemp;
	
	return iRetVal;
}

// Digital AGC gain monitor (*pcAGC => 2¡¯s complement format.)
int TC90507_Get_DAGC(char *pcAGC)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0xDA, &ucTemp);	// dagc_dt[7:0]
	if (iRetVal==ALPS_I2C_ERR_NON)
		*pcAGC= ~ucTemp+1;	// Fix Bug - 2¡¯s complement format
	
	return iRetVal;
}


// RF_AGC control level monitor output (*pucAGC => 0~255)
int TC90507_Get_RFAGC(U8 *pucAGC)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0x83, &ucTemp);	// rfagc_dt[7:0]
	if (iRetVal==ALPS_I2C_ERR_NON)
		*pucAGC=ucTemp;
	
	return iRetVal;
}


// Synchronization acquisition retry over flag.
// Outputs an error flag when the number of retries exceeds the retrycnt setting value.
// *pucResult => TC90507_LOCKED(0), TC90507_UNLOCKED(1)
int TC90507_Get_SyncRetryError(U8 *pucResult)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;

	iRetVal=TC90507_RegRead8(0x80, &ucTemp);	// retryov
	if (iRetVal==ALPS_I2C_ERR_NON)
		*pucResult=(ucTemp>>7)&0x01;

	return iRetVal;
}


// Synchronization sequence status information (*pucStatus => 0~F)
// 0: Transmission mode detection
// 1: FFT window position initial pull-in
// 2: Carrier AFC coarse synchronization
// 3: Carrier AFC fine synchronization clock AFC (first gain)
// 4: Carrier AFC fine synchronization clock AFC (second gain)
// 5: Clock PLL£¨first gain)
// 6: Clock PLL£¨second gain) -> 3/1 Segment
// 7: Clock PLL£¨third gain) -> 3/1 Segment
// 8: Synchronization sequence completion
// 9: FFT window position detection completion
// A¢¦F£º Not defined
int TC90507_Get_SequenceStatus(U8 *pucStatus)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0xB0, &ucTemp);	// seqen[3:0]
	if (iRetVal==ALPS_I2C_ERR_NON)
		*pucStatus=ucTemp&0x0F;					// 4bit
	else
		*pucStatus=0;
	
	return iRetVal;
}


// *pucResult => TC90507_LOCKED(0), TC90507_UNLOCKED(1)
// Reception level error flag
int TC90507_Get_ReceptionLevelError(U8 *pucResult)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0x80, &ucTemp);	// alarm[6]
	if (iRetVal==ALPS_I2C_ERR_NON)
		*pucResult=(ucTemp>>6)&0x01;
	
	return iRetVal;
}

// *pucResult => TC90507_LOCKED(0), TC90507_UNLOCKED(1)
// TMCC non-detection flag
int TC90507_Get_TMCC_Detect(U8 *pucResult)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0x80, &ucTemp);	// tmunvld [5]
	if (iRetVal==ALPS_I2C_ERR_NON)
		*pucResult=(ucTemp>>5)&0x01;
	
	return iRetVal;
}

// *pucResult => TC90507_LOCKED(0), TC90507_UNLOCKED(1)
// Frame non-synchronization flag
int TC90507_Get_FrameSynch(U8 *pucResult)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0x80, &ucTemp);	// fulock[3]
	if (iRetVal==ALPS_I2C_ERR_NON)
		*pucResult=(ucTemp>>3)&0x01;
	
	return iRetVal;
}

// Carrier frequency error monitor output
// When the receiver frequency is high, a negative (-) frequency error is output.
// When the receiver frequency is low, a positive (+) frequency error is output.
int TC90507_Get_CarrierOffsetFreq(double *pdFreqHz)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 pucTemp[2];
	U16 usTemp;
	short sTemp;

	/// Carrier frequency error = carafc_dt * 7.75 * (MD/FS) * (4/3) [Hz]
	iRetVal=TC90507_RegRead8Array(0x84, 2, pucTemp);	// carafc_dt
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		usTemp = (pucTemp[0]<<8) | pucTemp[1];
		sTemp= ~usTemp+1;	// Fix Bug - 2¡¯s complement format (Ver.0.60, yskoo)
		*pdFreqHz= sTemp * 7.75 * (76.224/65.015873) * (4.0/3.0);
	}

	return iRetVal;
}

// FFT size monitor output => (2K), (4K), (8K)
int TC90507_Get_FFTSize(char *pcOutput)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0xB0, &ucTemp);	// ffsize[7:6]
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		TC90507_ucFFTSize=(ucTemp>>6)&0x03;		// 2bit
		strcpy(pcOutput, FFTSize_pcStr[TC90507_ucFFTSize]);
	}
	
	return iRetVal;
}

// Guard Interval monitor output,  => (1/32), (1/16), (1/8), (1/4)
int TC90507_Get_GuardInterval(char *pcOutput)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0xB0, &ucTemp);	// gdleng[5:4]
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		TC90507_ucGI=(ucTemp>>4)&0x03;			// 2bit
		strcpy(pcOutput, GI_pcStr[TC90507_ucGI]);
	}
	
	return iRetVal;
}

// FFT demodulation window position setting (*pdPosition => x.xxx)
// should execute after TC90507_Get_FFT(), TC90507_Get_GuardInterval()
int TC90507_Get_FFTPosition(double *pdPosition)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	int Ng;
	int No;
	
	iRetVal=TC90507_RegRead8(0xB1, &ucTemp);	// woffset[7:0]
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		Ng = (1 << (6 + TC90507_ucFFTSize+TC90507_ucGI));
		No = ucTemp << (2+TC90507_ucFFTSize);
		*pdPosition=(double)No / (double)Ng;
	}
	
	return iRetVal;
}

// TMCC system identification (*piID => 0:TV, 1:Sound)
int TC90507_Get_TMCC_SystemID(char *pcOutput)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0xB2, &ucTemp);	// sysid[7:6]
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		TC90507_ucSystemID=(ucTemp>>6)&0x03;
		strcpy(pcOutput, SysID_pcStr[TC90507_ucSystemID]);		// 2bit
	}
	
	return iRetVal;
}

// TMCC parameter changeover indicator
// 0xF: No changeover, Other: Number of frames until changeover
// TMCC parameters are updated at the frame following pachg=0
int TC90507_Get_TMCC_ChangeoverCount(U8 *pucValue)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0xB2, &ucTemp);	// pachg[5:2]
	if (iRetVal==ALPS_I2C_ERR_NON)
		*pucValue=(ucTemp>>2)&0x0F;				// 4bit
	
	return iRetVal;
}

// TMCC emergency alarm broadcasting start flag £¨same as emerg£©
// *pucResult => TC90507_DATA_OFF(0), TC90507_DATA_ON(1)
int TC90507_Get_TMCC_EmergencyFlag(U8 *pucValue)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0xB2, &ucTemp);	// emeflg[1]
	if (iRetVal==ALPS_I2C_ERR_NON)
		*pucValue=(ucTemp>>1)&0x01;				// 1bit
	
	return iRetVal;
}

// TMCC partial reception flag, should execute after TC90507_Get_TMCC_SystemID()
// *piValue => TV mode => No partial reception, Partial reception provided
//			   Sound mode => 1-segment,  3-segment
// should execute after TC90507_Get_TMCC_SystemID()
int TC90507_Get_TMCC_PartialReception(char *pcOutput)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	
	iRetVal=TC90507_RegRead8(0xB2, &ucTemp);	// part[0]
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		if (TC90507_ucSystemID==0)
			strcpy(pcOutput, ParReceptionTV_pcStr[ucTemp&0x01]);
		else if (TC90507_ucSystemID==1)
			strcpy(pcOutput, ParReceptionSound_pcStr[ucTemp&0x01]);
	}
	
	return iRetVal;
}

// Combined transmission phase correction volume
// *pcOutput =>	0:-¥ð/8,  1:-2¥ð/8,  2:-3¥ð/8,  3:-4¥ð/8
//				4:-5¥ð/8, 5:-6¥ð/8,  6:-7¥ð/8,  7: No correction
int TC90507_Get_TMCC_PhaseCorrection(char *pcOutput)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp_1, ucTemp_2;

	iRetVal=TC90507_RegRead8(0xB7, &ucTemp_1);		// phcomp[2]
	ucTemp_2 = (ucTemp_1&0x01) << 2;
	iRetVal|=TC90507_RegRead8(0xB8, &ucTemp_1);	// phcomp[1:0]
	ucTemp_2 |=(ucTemp_1>>6) & 0x03;

	if (iRetVal==ALPS_I2C_ERR_NON)
		strcpy(pcOutput, PhaseCorrect_pcStr[ucTemp_2&0x07]);
	
	return iRetVal;
}

// tMod	=> TMCC layer X carrier modulation system
//				0:DQPSK, 1:QPSK, 2:16QAM, 3:64QAM, 7:No layer
// tCR	=> TMCC layer X convolution coding rate
//				0:1/2, 1:2/3, 2:3/4, 3:5/6, 4:7/8, 7:No layer
// tTI	=> TMCC layer X time interleave system
//		MODE1(when ffsize==0) => 0:I=0, 1:I=4, 2:I=8, 3:I=16, 4:I=32, 7:No layer
//		MODE2(when ffsize==1) => 0:I=0, 1:I=2, 2:I=4, 3:I=8, 4:I=16, 7:No layer
//		MODE3(when ffsize==2) => 0:I=0, 1:I=1, 2:I=2, 3:I=4, 4:I=8, 7:No layer
// tNumSeg	=> Number of segments used by layer X
//				0xf: No layer
int TC90507_Get_TMCC_LayerInfor(TC90507_pcLAYER_INFOR_t *ptInfor)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucB3, ucB4, ucB5, ucB6, ucB7, ucTemp;

	iRetVal=TC90507_RegRead8(0xB3, &ucB3);
	iRetVal|=TC90507_RegRead8(0xB4, &ucB4);
	iRetVal|=TC90507_RegRead8(0xB5, &ucB5);
	iRetVal|=TC90507_RegRead8(0xB6, &ucB6);
	iRetVal|=TC90507_RegRead8(0xB7, &ucB7);
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		ptInfor->tMod.pcLayerA=Modulation_pcStr[(ucB3>>5)&0x07];	// 3bit

		ptInfor->tCR.pcLayerA=CR_pcStr[(ucB3>>2)&0x07];		// 3bit

		ucTemp=((ucB3&0x03)<<1) | ((ucB4>>7)&0x01);
		if (TC90507_ucFFTSize==0)
			ptInfor->tTI.pcLayerA=TI1_pcStr[ucTemp];	// 3bit
		else if (TC90507_ucFFTSize==1)
			ptInfor->tTI.pcLayerA=TI2_pcStr[ucTemp];	// 3bit
		else
			ptInfor->tTI.pcLayerA=TI3_pcStr[ucTemp];	// 3bit

		ptInfor->tNumSeg.pcLayerA=NumSeg_pcStr[(ucB4>>3)&0x0F];	// 4bit


		ptInfor->tMod.pcLayerB=Modulation_pcStr[(ucB4>>0)&0x07];	// 3bit

		ptInfor->tCR.pcLayerB=CR_pcStr[(ucB5>>5)&0x07];		// 3bit

		ucTemp=(ucB5>>2)&0x07;
		if (TC90507_ucFFTSize==0)
			ptInfor->tTI.pcLayerB=TI1_pcStr[ucTemp];	// 3bit
		else if (TC90507_ucFFTSize==1)
			ptInfor->tTI.pcLayerB=TI2_pcStr[ucTemp];	// 3bit
		else
			ptInfor->tTI.pcLayerB=TI3_pcStr[ucTemp];	// 3bit

		ucTemp=((ucB5&0x03)<<2) | ((ucB6>>6)&0x03);
		ptInfor->tNumSeg.pcLayerB=NumSeg_pcStr[ucTemp];	// 4bit


		ptInfor->tMod.pcLayerC=Modulation_pcStr[(ucB6>>3)&0x07];	// 3bit

		ptInfor->tCR.pcLayerC=CR_pcStr[(ucB6>>0)&0x07];		// 3bit

		ucTemp=(ucB7>>5)&0x07;
		if (TC90507_ucFFTSize==0)
			ptInfor->tTI.pcLayerC=TI1_pcStr[ucTemp];	// 3bit
		else if (TC90507_ucFFTSize==1)
			ptInfor->tTI.pcLayerC=TI2_pcStr[ucTemp];	// 3bit
		else
			ptInfor->tTI.pcLayerC=TI3_pcStr[ucTemp];	// 3bit
		
		ptInfor->tNumSeg.pcLayerC=NumSeg_pcStr[(ucB7>>1)&0x0F];	// 4bit
	}

	return iRetVal;
}

// CNR monitor output (*pdCNR => xx.x)
int TC90507_Get_CNR(double *pdCNR)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	double P;
	U8 pucTemp[3];
	U32 uiCNData=0;

	// Outputs the average of differential value from the reference point to the constellation after equalization
	iRetVal=TC90507_RegRead8Array(0x8B, 3, pucTemp);	// cndat, 3 Bytes
	
	if (iRetVal==ALPS_I2C_ERR_NON)
	{
		uiCNData|=(pucTemp[0]<<16);	
		uiCNData|=(pucTemp[1]<<8);
		uiCNData|=(pucTemp[2]<<0);

		if (iRetVal==ALPS_I2C_ERR_NON)
		{
			if (uiCNData!=0)
			{
				P=10*log10(((double)5505024/(double)uiCNData));
				*pdCNR=(0.000024*pow(P,4))-(0.0016*pow(P,3))+(0.0398*pow(P,2))
					+(0.5491*P)+3.0965;
			}
			else
				*pdCNR=0;
		}
	}

	return iRetVal;
}

// For ALPS Tuner design team (ROHDE&SCHWARZ SFQ BER Measurement)
// Turns the valid signal of the null packet OFF. (Excluding null packets actually transmitted) 
// (0: Valid flag on, 1: Valid flag off)
// ucMode => TC90507_ENABLE(0), TC90507_DISABLE(1)
int TC90507_Set_SFQBERMeasurement(U8 ucMode)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	
	if (ucMode==TC90507_ENABLE)
		iRetVal=TC90507_RegWriteMaskBit8(0x76, TC90507_BIT_3, TC90507_BIT_3);	// nuval[3]
	else
		iRetVal=TC90507_RegWriteMaskBit8(0x76, TC90507_BIT_3, 0);
	
	return iRetVal;
}
static void ISDB_Log(void);
int MOPIC_SendData(U32 FrequencykHz,U8 mode)
{
	int iRetVal=ALPS_I2C_ERR_NON;	

	//printf("FrequencykHz=%d\n",FrequencykHz);
	iRetVal=TC90507_Send_BypassData(FrequencykHz);

	if (mode==TC90507_CONFIG_DIGITAL)
		iRetVal|=TC90507_Set_DemodReset();

	//Sleep(300);   //Remove By River 2008.06.10
	//ISDB_Log();

	return iRetVal;
}

#define TC90507_LOCKED					0
#define TC90507_UNLOCKED				1

#define TC90507_DATA_OFF				0
#define TC90507_DATA_ON					1
static void ISDB_Log(void)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucTemp;
	char pcTemp[10];
	double dTemp;
	TC90507_ucRSERROR_t tOFDM_RSError;
	TC90507_LAYER_dBER_t tOFDM_BER;
	TC90507_pcLAYER_INFOR_t tOFDM_LayerInfor;


			iRetVal|=TC90507_Get_SequenceStatus(&ucTemp);
			printf("\n - Current Sequence	: %d", ucTemp);

			iRetVal|=TC90507_Get_CNR(&dTemp);
			printf("\n - SNR			: %.1f", dTemp);

			iRetVal|=TC90507_Get_IFAGC(&ucTemp);
			printf("\n - AGC			: %d", ucTemp);

			iRetVal|=TC90507_Get_ReceptionLevelError(&ucTemp);
			if (ucTemp==TC90507_LOCKED)
				printf("\n - Reception Alarm	: LOCKED");
			else
				printf("\n - Reception Alarm	: UNLOCKED");

			iRetVal|=TC90507_Get_FrameSynch(&ucTemp);
			if (ucTemp==TC90507_LOCKED)
				printf("\n - Frame Sync		: LOCKED");
			else
				printf("\n - Frame Sync		: UNLOCKED");

			iRetVal|=TC90507_Get_TMCC_Detect(&ucTemp);
			if (ucTemp==TC90507_LOCKED)
				printf("\n - TMCC Detection	: LOCKED");
			else
				printf("\n - TMCC Detection	: UNLOCKED");

			iRetVal|=TC90507_Get_FFTSize(pcTemp);
			printf("\n\n - FFT Size		: %s", pcTemp);
			
			iRetVal|=TC90507_Get_GuardInterval(pcTemp);
			printf("\n - GuardLength		: %s", pcTemp);

			iRetVal|=TC90507_Get_TMCC_SystemID(pcTemp);
			printf("\n - System ID		: %s", pcTemp);

			iRetVal|=TC90507_Get_TMCC_ChangeoverCount(&ucTemp);
			printf("\n - Count Down		: %d", ucTemp);

			iRetVal|=TC90507_Get_TMCC_PartialReception(pcTemp);
			printf("\n - Partial Reception	: %s", pcTemp);

			iRetVal|=TC90507_Get_TMCC_PhaseCorrection(pcTemp);
			printf("\n - Phase Correction	: %s", pcTemp);

			iRetVal|=TC90507_Get_FFTPosition(&dTemp);
			printf("\n - FFT Position		: %.3f", dTemp);

			iRetVal|=TC90507_Get_TMCC_EmergencyFlag(&ucTemp);
			if (ucTemp==TC90507_DATA_OFF)
				printf("\n - Emergency Flag	: OFF");
			else
				printf("\n - Emergency Flag	: ON");

			printf("\n\n			LayerA		LayerB		LayerC");

			iRetVal|=TC90507_Get_TMCC_LayerInfor(&tOFDM_LayerInfor);

			printf("\n - Modulation		%s		%s		%s", 
				tOFDM_LayerInfor.tMod.pcLayerA, tOFDM_LayerInfor.tMod.pcLayerB, tOFDM_LayerInfor.tMod.pcLayerC );
			printf("\n - Coding Rate		%s		%s		%s", 
				tOFDM_LayerInfor.tCR.pcLayerA, tOFDM_LayerInfor.tCR.pcLayerB, tOFDM_LayerInfor.tCR.pcLayerC );
			printf("\n - Time Interleave	%s		%s		%s", 
				tOFDM_LayerInfor.tTI.pcLayerA, tOFDM_LayerInfor.tTI.pcLayerB, tOFDM_LayerInfor.tTI.pcLayerC );
			printf("\n - Number of Segment	%s		%s		%s", 
				tOFDM_LayerInfor.tNumSeg.pcLayerA, tOFDM_LayerInfor.tNumSeg.pcLayerB, tOFDM_LayerInfor.tNumSeg.pcLayerC );

			iRetVal|=TC90507_Get_RSErrorStatus(&tOFDM_RSError);
			if (tOFDM_RSError.ucStatus_LayerA==TC90507_RS_NONERROR)
				printf("\n\n - RS Error		LOCKED		");
			else
				printf("\n - RS Error		UNLOCKED	");
			if (tOFDM_RSError.ucStatus_LayerB==TC90507_RS_NONERROR)
				printf("LOCKED		");
			else
				printf("UNLOCKED	");
			if (tOFDM_RSError.ucStatus_LayerC==TC90507_RS_NONERROR)
				printf("LOCKED");
			else
				printf("UNLOCKED");

			iRetVal|=TC90507_Get_BER(&tOFDM_BER);
			printf("\n - BER			%.2e	", (float)tOFDM_BER.dBER_LayerA);
			printf("%.2e	", (float)tOFDM_BER.dBER_LayerB);
			printf("%.2e", (float)tOFDM_BER.dBER_LayerC);

#if 0
			if (TC90507_ucBERType==TC90507_BER_AFTER_RS)
				printf("\n\n - BER Monitor Type	: After RS");
			else
				printf("\n\n - BER Monitor Type	: After Viterbi");

			if (TC90507_ucTSOutputLayer==TC90507_TS_OUTPUT_LAYER_ALL)
				printf("\n\n - TS Output Layer	: Layer All");
			else if (TC90507_ucTSOutputLayer==TC90507_TS_OUTPUT_LAYER_A)
				printf("\n\n - TS Output Layer	: Layer A");
			else if (TC90507_ucTSOutputLayer==TC90507_TS_OUTPUT_LAYER_B)
				printf("\n\n - TS Output Layer	: Layer B");
			else
				printf("\n\n - TS Output Layer	: Layer C");		
#endif
		}

