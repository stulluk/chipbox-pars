
#ifndef	_CS_TUNER_H_
#define	_CS_TUNER_H_

#ifdef __cplusplus
extern "C" {
#endif


//DVBT_P10046
//DVBC_P10023
//DVBT_P10048
//DMBT_LGS8913

#define  DVBC_P10023

#define	TUNER_FD					0x0

typedef enum
{
	TUNER_ID_0,
	TUNER_ID_1
}TUNER_ID_E;



typedef enum TUNER_BANDWIDTH {
	BANDWIDTH_8_MHZ,
	BANDWIDTH_7_MHZ,
	BANDWIDTH_6_MHZ,
	BANDWIDTH_AUTO
} TUNER_BANDWIDTH_E;

typedef enum{
	TUNER_STATUS_PLL_LOCKED = 0,
	TUNER_STATUS_FE_LOCKED,
	TUNER_STATUS_UNLOCKED,
	TUNER_STATUS_SCANNING,
	TUNER_STATUS_NOT_FOUND
}TUNER_STATUS_E;

typedef enum{
	INVERSION_NORMAL = 0,
	INVERSION_INVERTED,
	INVERSION_AUTO
}TUNER_SPECTRAL_INVERSION_E;

typedef enum{
	QAM_4 = 0,
	QAM_16 = 1,
	QAM_32 = 2,
	QAM_64 = 3,
	QAM_128 = 4,
	QAM_256 = 5,
	QAM_AUTO = 6
}TUNER_QAM_MODULATION_E;

//struct define 
typedef struct{
	unsigned int	symbol_rate;
	TUNER_QAM_MODULATION_E modulation;
}TUNER_QAM_PARAMS_S;

typedef struct{
	unsigned int	frequency;
	TUNER_QAM_PARAMS_S	qam_params;
	TUNER_SPECTRAL_INVERSION_E	inversion;
}TUNER_PARAMS_S;

typedef struct{
	unsigned char	name[128];
	unsigned int	frequency_min;
	unsigned int	frequency_max;
	unsigned int	frequency_stepsize;
	unsigned int	frequency_tolerance;
	unsigned int	symbol_rate_max;
	unsigned int	symbol_rate_min;
	unsigned int	symbol_rate_tolerance;
	unsigned int	notifier_delay;
}TUNER_INFO_S;

int	cs_tuner_init(void);
int	cs_tuner_open(TUNER_ID_E tuner_id);
int	cs_tuner_close(int fd);
int	cs_tuner_set_params(int fd, TUNER_PARAMS_S *params);
int	cs_tuner_get_params(int fd, TUNER_PARAMS_S *params);
int	cs_tuner_get_info(int fd, TUNER_INFO_S *info);
int	cs_tuner_read_ber(int fd, unsigned int *ber);
int	cs_tuner_read_snr(int fd, unsigned int *snr);
int	cs_tuner_read_status(int fd, TUNER_STATUS_E *status);
int	cs_tuner_read_signal_strength(int fd, unsigned int *signal_strength);

#ifdef __cplusplus
}
#endif

#endif	/*	_CS_TUNER_LG_H_	*/
