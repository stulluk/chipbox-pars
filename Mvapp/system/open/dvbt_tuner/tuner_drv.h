#ifndef __TUNER_DRV_H
#define __TUNER_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#define TUNER_IF1_OUTPUT	0x01 /*output to MT352 is 36 or 44 MHz, not 4.57*/
#define TUNER_LO_INJECTION  0x02 /*1st LO is below RF input frequency*/
#define TUNER_SPECTRAL_INV	0x04 /* additional spectral inversion required */

#define Tuner_addr 0xc2

#define TDM1316AL_1		1

#if  TDM1316AL_1
#define PHILIPS_TD13111_TABLE \
            {51, 0xCA61,16667}, \
            {94, 0xCAA1,16667}, \
            {124, 0xCAC1,16667}, \
            {164, 0xCA62,16667}, \
            {254, 0xCAA2,16667}, \
            {384, 0xCAC2,16667}, \
            {444, 0xCA64,16667}, \
            {564, 0xCAA4,16667}, \
            {794, 0xCAE4,16667}		

#define PHILIPS_TD13111_INFO \
            Tuner_addr, \
            9, \
            TUNER_IF1_OUTPUT, \
            36130, \
            0x00, \
            0x00, \
            0x08
#else
#define PHILIPS_TD13113_TABLE \
            {48, 0xBC01,16667}, \
            {144, 0xF401,16667}, \
            {161, 0xBC02,16667}, \
            {330, 0xF402,16667}, \
            {447, 0xBC04,16667}, \
            {610, 0xF404,16667}, \
            {754, 0xFC04,16667}
		
#define PHILIPS_TD13113_INFO \
        Tuner_addr, \
        7,  \
        TUNER_IF1_OUTPUT,   \
        36130, \
        0x00, \
        0x00, \
        0x08
#endif

#define ZL10353_I2C_ADDRESS	0x0F//0x0E

#define  MPEG_OFF             		0
#define  MPEG_SERIAL		   	0
#define  MEPG_NoTEI		       	0
//#define  MPEG_NoBKERRinvert	  0
#define  MPEG_MDOswap	       	0
#define  MPEG_MPEGgap12    	0
#define  MPEG_clkinEn          		0
#define  MPEG_ClkInv           		1
#define  MPEG_serialLSB1st      	0
#define  MPEG_BKERRinvert     	1

#define  DOUBLE_AGC         		0


typedef enum 
{
	ZL10353_OFDM_STAT,        			//0x06  	
	ZL10353_FEC_STAT,				//0x07
	ZL10353_TUNER_STAT,        		//0x08
	ZL10353_TPS_INFO,        			//0x09
	ZL10353_RF_LEVEL,				//0x0E              
	ZL10353_SNR,					//0x10              
	ZL10353_TPS_RECEIVED_1,		//0x1D             
	ZL10353_TPS_RECEIVED_0,		//0x1E              
	ZL10353_TPS_CURRENT_1,		//0x1F             
	ZL10353_TPS_CURRENT_0,		//0x20             
	ZL10353_CONFIG,					//0x50             
	ZL10353_CLOCK_CTL_0,			//0x51            
	ZL10353_CLOCK_CTL_1,   			//0x52
	ZL10353_PLL_0,          				//0x53
	ZL10353_PLL_1,          				//0x54
	ZL10353_RESET,          				//0x55
	ZL10353_ADC_CTL,        			//0xEA
	ZL10353_AGC_GAIN_1,      			//0x0A
	ZL10353_AGC_GAIN_0,     			//0x0B
	ZL10353_AGC_TARGET,    			//0x56
	ZL10353_AGC_CTL,        			//0x89
	ZL10353_UK_REG1,        			//0x9C  /* unknown register */
	ZL10353_AGC_CTRL_5,     			//0x8E
	ZL10353_AGC_IF_LOLIM,   			//0x90
	ZL10353_AGC_RF_HILIM,   			//0x91
	ZL10353_AGC_IF_MAX,     			//0x92
	ZL10353_AGC_IF_MIN,     			//0x93
	ZL10353_AGC_RF_MAX,     			//0x94
	ZL10353_AGC_RF_MIN,     			//0x58
	ZL10353_AGC_KIF,         			//0x95
	ZL10353_AGC_KRF,         			//0x96
	ZL10353_AFC_CTL,        			//0x7C
	ZL10353_ACQ_CTL,        			//0x5E
	ZL10353_ACQ_DLY,       			//0xE9
	ZL10353_BW_CTL,         			//0x64
	ZL10353_TRL_NOMNIAL_RATE_1,  	//0x65
	ZL10353_TRL_NOMNIAL_RATE_0,  	//0x66
	ZL10353_MCLK_CTL,        			//0x5C
	ZL10353_INPUT_FREQ_1,   			//0x6C
	ZL10353_INPUT_FREQ_0,   			//0x6D
	ZL10353_TUNER_ADDR,    			//0x67  
	ZL10353_CHAN_START_1,    		//0x68  
	ZL10353_CHAN_START_0,    		//0x69  
	ZL10353_TUNER_CONT_1,    		//0x6A  
	ZL10353_TUNER_CONT_0,   		//0x6B  
	ZL10353_CHAN_STOP_1,     			//0xE2
	ZL10353_CHAN_STOP_0,    			//0xE3
	ZL10353_CAPT_RANGE,    			//0x5F
	ZL10353_TPS_GIVEN_1,    			//0x6E
	ZL10353_TPS_GIVEN_0,   			//0x6F
	ZL10353_TUNER_GO,      			//0x70
	ZL10353_FSM_GO,        				//0x71
	ZL10353_OP_CTRL_0,      			//0x5A
	ZL10353_OP_CTRL_1,      			//0x5B
	ZL10353_SCANCTL,         			//0x62
	ZL10353_UK_REG2,        			//0x9A  /* unknown register */
	ZL10353_UK_REG3,        			//0x9B  /* unknown register */
	ZL10353_UK_REG4,        			//0xCC  /* unknown register */	
	ZL10353_NO_OF_REGS   		
}eReg_ID;

#define  ZL10353_FECSTAT_LOCKED   0x30
#define  ZL10353_TUNER_GOAQUIRE   0x01

typedef enum 
{
	ZL10353_SELECT_TS_HP = 0x40,
	ZL10353_SELECT_TS_LP =  0xc0
}eReg_Value_HPLP;

typedef struct
{
    	U16 FreqMHz;    /* RF input frequency above which the following control word applies */
    	U16 ControlByte;    /* 16 bit synth control(charge pump, div ratio, band switch etc)*/
	U16 StepkHzx100;   /* comparison frequency(step size)*/
}tTUNER_PLL_CBTable;

/* Tuner control settings*/
typedef struct
{

	U8 	Address;            	/* bus address of tuner*/
	U8 	TableCount;         	/* number of entries in TunerTable*/
	U8 	Settings;			/* misc settings */
    	U16 	IF1kHz;             	/* tuner IF1 kHz for synth calcs (e.g. 36167)*/
   	U16 Control6MHz;        /* extra Control bits to set for 6MHz*/
    	U16 Control7MHz;        /* extra Control bits to set for 7MHz*/
    	U16 Control8MHz;        /* extra Control bits to set for 8MHz*/
}tTUNER_PLL_Info;

BOOL ZL10353_Init(void);
BOOL ZL10353_ProgramPLL(U32 pFrequency, U8 pBW);
BOOL ZL10353_Set_CaptRange(U8 pCaptRange);
BOOL ZL10353_SetBandWidth(U8 pBW);
BOOL ZL10353_IsLocked(void);
U16 ZL10353_ReadAGC(void);
U8 ZL10353_GetSNR(void);
U16 ZL10353_GetReceivedTPS(void);
void ZL10353_PartReset(void);
void ZL10353_SetTSPriority(U8 pValue);

#ifdef __cplusplus
}
#endif

#endif 

