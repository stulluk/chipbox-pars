
#ifndef	_TUNER_ERROR_H_
#define	_TUNER_ERROR_H_	

#ifdef __cplusplus
extern "C" {
#endif

//the error code bit	00~07:	module specify error
//				 		08~15:	module 		:BB
//				 		16~23:	error		:FF
//				 		24~31:	reserve

#define TUNER_ERROR_BASE			0x00FFCC00
#define TUNER_NO_ERROR				0x0
#define TUNER_NOT_SUPPORT_NOW		(TUNER_ERROR_BASE+0x0A)

#define TUNER_ERROR_DEV				(TUNER_ERROR_BASE+0x00)			
#define TUNER_ERROR_FD				(TUNER_ERROR_BASE+0x01)	
#define TUNER_ERROR_MANY_FILE		(TUNER_ERROR_BASE+0x02)	
#define TUNER_ERROR_LITTLE_FILE		(TUNER_ERROR_BASE+0x03)	

#define TUNER_ERROR_PSYMBOL_RATE	(TUNER_ERROR_BASE+0x10)	
#define TUNER_ERROR_PSTATUS			(TUNER_ERROR_BASE+0x11)	
#define TUNER_ERROR_PSNR			(TUNER_ERROR_BASE+0x12)	
#define TUNER_ERROR_PBER			(TUNER_ERROR_BASE+0x13)	

#define TUNER_ERROR_INVALID_PARAM	(TUNER_ERROR_BASE+0x20)	
#define TUNER_ERROR_LOCK_PLL		(TUNER_ERROR_BASE+0x21)	
#define TUNER_ERROR_BUSY			(TUNER_ERROR_BASE+0x22)	
#define TUNER_ERROR_NO_SIGNAL		(TUNER_ERROR_BASE+0x23)

#define TUNER_ERROR_INIT_SYSTEM		(TUNER_ERROR_BASE+0x30)	
#define TUNER_ERROR_GET_PARAMS		(TUNER_ERROR_BASE+0x31)	
#define TUNER_ERROR_GET_INFO		(TUNER_ERROR_BASE+0x32)	
	

#ifdef __cplusplus
}
#endif

#endif	/*	_TUNER_ERROR_H_	*/


