
/*
*
*Copyright 2006, Legend Silicon Corp. All rights reserved.
*
*File name: 	lgs_8913.h
*Description : 	Driver code for lgs 8913 demodulator
*
*/

#ifndef _LGS_8913_H_
#define _LGS_8913_H_

/**
 *	/file 	lgs_8913.h
 *	/brief 	Driver code for lgs 8913 demodulator
 */

#ifdef __cplusplus
extern "C" {
#endif 

#include "lgs_types.h"

#define USE_SHARP_TUNER
//#define USE_CH_TUNER


#define RX_SC_MASK	0x1C	/**< sub-carrier modulation mask */
#define RX_SC_QAM64	0x10	/**< 64QAM modulation */
#define RX_SC_QAM16	0x08	/**< 16QAM modulation */
#define RX_SC_QAM4	0x00	/**< 4QAM modulation */

#define RX_FEC_MASK	0x03	/**< FEC rate mask */
#define RX_FEC_0_4	0x00	/**< FEC rate is 0.4 */
#define RX_FEC_0_6	0x01	/**< FEC rate is 0.6 */
#define RX_FEC_0_8	0x02	/**< FEC rate is 0.8 */

#define RX_TIM_MASK	  0x20	/**< time interleave length mask */
#define RX_TIM_LONG	  0x00	/**< time interleave length is 720 */
#define RX_TIM_MIDDLE     0x20   /**< time interleave length is 240 */

#define RX_CF_MASK	0x80	/**< control frame mask */
#define RX_CF_EN	0x80	/**< control frame on */

#define RX_GI_MASK	0x03	/**< guard interval mask */
#define RX_GI_420	0x00	/**< guard interval is 1/9 */
#define RX_GI_945	0x02	/**< guard interval is 1/4 */

#define	MPEG_OUTPUT_PARALLEL	0x00	/**< MPEG2 parallel output */
#define MPEG_OUTPUT_SERIAL	0x01	/**< MPEG2 serial output */
#define	MPEG_CLK_NORMAL		0x00	/**< MPEG2 normal clock */
#define MPEG_CLK_INVERTED	0x02	/**< MPEG2 inverted clock */
#define	MPEG_CLK_GATED		0x00	/**< MPEG2 clock gated */
#define	MPEG_CLK_FREE		0x04	/**< MPEG2 clock free */



#define LGS_NO_ERROR		0	/**< no error */
#define LGS_REGISTER_ERROR	0xFF	/**< register error */
#define LGS_I2C_OPEN_ERROR	0xFE	/**< I2C open error */
#define LGS_I2C_READ_ERROR	0xFD	/**< I2C read error */
#define LGS_I2C_WRITE_ERROR	0xFC	/**< I2C write error */
#define LGS_I2C_CLOSE_ERROR	0xFB	/**< I2C close error */
#define LGS_NO_LOCKED		0xFA	/**< not locked */
#define LGS_AUTO_DETECT_FAILED	0xF9	/**< auto detect failed */
#define LGS_FREQUENCY_ERROR	0xF8	/**< frequency error */
#define LGS_PAYLOAD_ERROR	0xF7	/**< compute payload error */


/*#define GI_AUTODETECT*/		/**< whether use guard interval auto detect */
#define BY_PASS_TUNER_I2C	/**< whether does tuner's I2C bypass the demodulation chip */
#define USE_LGS_SPECIFIED_TUNER	/**< whether use the tuner specifed by lgs */


typedef INT8 (*LGS_OPEN_I2C)(LGS_HANDLE *phandleI2c, 
			     const UINT8 sectionAddr, 
			     INT8 timeoutFlag);		/**< I2C open function pointer */
typedef INT8 (*LGS_READ_I2C)(LGS_HANDLE handleI2c,
			     UINT8 *pbuffer, 
			     UINT8 bufferLen, 
			     UINT8 timeout);		/**< I2C read function pointer */
typedef INT8 (*LGS_WRITE_I2C)(LGS_HANDLE handleI2c,
			     const UINT8 *pbuffer, 
			     UINT8 bufferLen, 
			     UINT8 timeout);		/**< I2C write function pointer */
typedef INT8 (*LGS_CLOSE_I2C)(LGS_HANDLE handleI2c);	/**< I2C close function pointer */
typedef void (*LGS_WAIT)(UINT16 millisecond);		/**< delay function pointer */

int tuner_i2c_init(void);

/**
 *register I2C entrance function
 *
 *@param	popen	I2C open
 *@param	pread 	I2C read
 *@param 	pwrite	I2C write
 *@param	pclose	I2C close
 *@return 	void
 */
void LGS_RegisterI2C(LGS_OPEN_I2C popen, 
		     LGS_READ_I2C pread, 
		     LGS_WRITE_I2C pwrite, 
		     LGS_CLOSE_I2C pclose);
		     
/**
 *register delay entrance function
 *
 *@param 	pwait	delay function
 *@return 	void
 */
void LGS_RegisterWait(LGS_WAIT pwait);

/**
 *confirm demodulator I2C base address
 *
 *@param	CE_A2	0	pin CE_A2 is low
 *			1	pin CE_A2 is high
 *@param	CE_A1	0	pin CE_A1 is low
 *			1	pin CE_A1 is high
 *@param	CE_A0	0	pin CE_A0 is low
 *			1	pin CE_A0 is high
 *@return 	void
 */
void LGS_DemodulatorBaseAddress(UINT8 CE_A2,
				UINT8 CE_A1,
				UINT8 CE_A0);

/**
 *write the registers of demodulator or tuner
 *
 *@param	registerAddress	register addresss
 *@param	registerData	register data
 *@return	macros defined in lgs_8913.h
 */ 
INT8 LGS_WriteRegister(UINT8 registerAddress, UINT8 registerData);

/**
 *read the registers of demodulator or tuner
 *
 *@param	registerAddress	register addresss
 *@param	pregisterData	register data
 *@return	macros defined in lgs_8913.h
 */ 
INT8 LGS_ReadRegister(UINT8 registerAddress, UINT8 *pregisterData);

/**
 *soft reset demodulator
 *
 *@param	void
 *@return	void
 */
INT8 LGS_SoftReset(void);

/**
 *get the status of demodulator
 *
 *@param	pcontrolFram		control frame
 *@param 	pmode			demodulation mode
 *@param	prate			FEC rate
 *@param	pinterleaveLength	interleave length
 *@param 	pguardInterval		guard interval
 *@return 	macros defined in lgs_8913.h
 */
INT8 LGS_GetStatus(UINT8 *pcontrolFrame,
		   UINT8 *pmode, 
		   UINT8 *prate, 
		   UINT8 *pinterleaverLength, 		    
		   UINT8 *pguardInterval);

/**
 *set demodulator to manual mode
 *
 *@param 	mode			demodulation mode
 *@param	rate			FEC rate
 *@param	interleaveLength	interleave length
 *@param	pguardInterval		guard interval
 *@return	macros defined in lgs_8913.h
 */
INT8 LGS_SetManualMode(UINT8 mode, UINT8 rate, UINT8 interleaverLength, UINT8 guardInterval);

/**
 *auto detect demodulator parameters
 *
 *@param	pcontrolFram		control frame
 *@param	pmode			demodulation mode
 *@param	prate			FEC rate
 *@param	pinterleaveLength	interleave length
 *@param	pguardInterval		guard interval
 *@return	INT8
 */
INT8 LGS_LoopAutoDetect(UINT8 *pcontrolFrame, UINT8 *pmode, UINT8 *prate, UINT8 *pinterleaverLength, UINT8 *pguardInterval);

/**
 *compute actual payload
 *
 *@param	ppayload	actual payload
 *@param	mode		demodulation mode
 *@param	rate		FEC rate
 *@param	guardInterval	guard interval
 *@return	macros defined in lgs_8913.h
 */
INT8 LGS_ComputePayload(UINT32 *ppayload, UINT8 mode, UINT8 rate, UINT8 guardInterval);

/**
 *get MPEG2 output mode
 *
 *@param	pserial		serial or parallel
 *@param	pclkPolarity	clock normal or inverted
 *@param	pclkGated	clock gated or free
 *@return	macros defined in lgs_8913.h
 */
INT8 LGS_GetMpegMode(UINT8 *pserial, UINT8 *pclkPolarity, UINT8 *pclkGated);

/**
 *Set MPEG2 output mode
 *
 *@param	serial		serial or parallel
 *@param	clkPolarity	clock normal or inverted
 *@param	clkGated	clock gated or free
 *@return	macros defined in lgs_8913.h
 */
INT8 LGS_SetMpegMode(UINT8 serial, 
		     UINT8 clkPolarity, 
		     UINT8 clkGated) ;

INT8 LGS_SetAutoMode(void);

INT8 LGS_GetAutoParameters(UINT8 *pcontrolFrame, UINT8 *pmode, UINT8 *prate, UINT8 *pinterleaverLength);

INT8 LGS_AutoDetectDone(UINT8 *presult);	

INT8 LGS_CheckLocked(UINT8 *presult);

#ifdef	BY_PASS_TUNER_I2C
#ifdef	USE_LGS_SPECIFIED_TUNER

/**
 *make tuner work
 *
 *@param	frequency	channel frequency, MHz
 *@return	macros defined in lgs_8913.h
 */
INT8 LGS_StartTuner(UINT16 frequency);

#endif	/*USE_LGS_SPECIFIED_TUNER*/

/**
 *open tuner I2C
 *
 *@param 	tunerAddress	tuner address
 *@return	macros defined in lgs_8913.h
 */
INT8 LGS_OpenTunerI2C(UINT8 tunerAddress);

/**
 *close tuner I2C
 *
 *@param	void
 *@return	macros defined in lgs_8913.h
 */
INT8 LGS_CloseTunerI2C(void);
#endif	/*BY_PASS_TUNER_I2C*/


#ifdef __cplusplus
}
#endif                          

#endif /*_LGS_8913_H_*/

