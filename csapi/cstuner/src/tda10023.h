
#ifndef		_TUNER_FUNCTION_P10023_H_
#define		_TUNER_FUNCTION_P10023_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cs_tuner.h"

/******************************** Read Only Registers ***********************************/
/******* Register Name ************** Address ******* Function **************************/

#define	P10023_DEMOD_I2C_ADDR	               0x0C        /*  define the demodulation I2C address G152D*/
#define	P10023_TUNER_I2C_ADDR	               0x60        /*  define the tuner I2C address G152D*/


int p10023_tuner_set_pll(unsigned int frequency);
int p10023_demodu(unsigned int	frequency, unsigned int	symbol_rate, TUNER_QAM_MODULATION_E modulation, TUNER_SPECTRAL_INVERSION_E inversion);
int p10023_tuner_lock_check(void);
int p10023_demodu_init(void);



#ifdef __cplusplus
}
#endif

#endif



