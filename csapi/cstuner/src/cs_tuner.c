
#include	<stdio.h>
#include	"cs_tuner.h"

#ifdef DVBT_LG
#include "tuner_function_lg.h"
#endif

#ifdef DVBT_P10046
#include "tda10046.h"
#endif

#ifdef DVBC_P10023
#include "tda10023.h"
#endif

#ifdef DVBT_P10048
#include "tda10048.h"
#endif

#ifdef DMBT_LGS8913
#include "lgs8913.h"
#endif

#include	"tuner_error.h"
#include	"tuner_i2c.h"




static unsigned int tuner_use_count=0;


/** -----------------------------------------------------------------------
 * @Brief   : Open the tuner device and generate the handle
 * 
 * @Param   :  tuner_id -- Tuner service ID 
 * 
 * @Returns : The tuner devide handle or error code 
 --------------------------------------------------------------------------*/
int	cs_tuner_open(TUNER_ID_E tuner_id)
{
	int	fd = 0;

	//check the tuner id
	if(TUNER_ID_0 != tuner_id)
	{
		#ifdef TUNER_DEBUG
			printf("tuner device serive ID failure. \n");
		#endif
		return TUNER_ERROR_DEV;
	}
	
	//check the open count 
	if(0 != tuner_use_count)
	{
		#ifdef TUNER_DEBUG
			printf("tuner device has been opened, please close it first. \n");
		#endif
		return TUNER_ERROR_MANY_FILE;
	}
	tuner_use_count++;
	
	//make the handle 
	fd = TUNER_FD;

	//return the fd of the tuner for further use
	return fd;
}

/** -----------------------------------------------------------------------
 * @Brief   : Initialize the tuner tune and demodulate
 * 
 * @Param   : fd -- Handle of the tuner
 * 
 * @Returns : The error code
 --------------------------------------------------------------------------*/
int cs_tuner_init(void)
{
	int	retval = 0;
	
	printf("cs_tuner_init function called. \n");
		
	retval = c_tuner_i2c_init();	
	if(retval < 0) 
	{
		printf("Error: /dev/misc/orion_i2c.\n");
		return retval;
	}
	
#ifdef DVBT_P10046
	printf("222 into DVBT_P10046 cs_tuner_init \n");
	p10046_demodu_init();
#endif

#ifdef DVBC_P10023
	printf("222 into DVBC_P10023 cs_tuner_init \n");
	p10023_demodu_init();
#endif	

#ifdef DVBT_P10048
	printf("222 into DVBT_P10048 cs_tuner_init \n");
	p10048_demodu_init();
#endif	

#ifdef DMBT_LGS8913
	printf("222 into DVBT_P10048 cs_tuner_init \n");
	lgs8913_demodu_init();
#endif	
	return 0;
}

/** -----------------------------------------------------------------------
 * @Brief   : Close the tuner device and release the video handle 
 * 
 * @Param   : fd -- Handle of the tuner
 * 
 * @Returns : The error code
 --------------------------------------------------------------------------*/
int	cs_tuner_close(int fd)
{
	printf("cs_tuner_close function called. \n");

	//verify the handle
	if(TUNER_FD != fd)
	{
		printf("check tuner handle failure. \n");
		return -1;
	}

	//check the open count
	if(1 != tuner_use_count)
	{
		printf("tuner device hasn't been opened, please open it first. \n");
		return -2;
	}
	tuner_use_count--;

	return 0;
}

/** -----------------------------------------------------------------------
 * @Brief   : Set the tuner parameters for tune and demodulate
 * 
 * @Param   : fd -- Handle of the tuner
 * @Param   : params -- Point to the tune parameters: Frequency, 
 * 					Symbol Rate, Spectrum Inversion, QAM
 * 
 * @Returns : The error code
 --------------------------------------------------------------------------*/
int	cs_tuner_set_params(int fd, TUNER_PARAMS_S *params)
{
	
	printf("cs_tuner_set_params function called. \n");
	printf("\nset tuner param : Frequency:%d (M)  Symbol Rate:%d (K)\n                  Inversion:%d (0-NORMAL,1-INVERTED,2-AUTO)\n                  Qam:%d (1-16,2-32,3-64,4-128,5-256,6-AUTO,)\n",
				params->frequency,params->qam_params.symbol_rate,
				params->inversion,params->qam_params.modulation);
	
#ifdef DVBT_LG	
	lg_demodu_init();
   	lg_demodu_reset();
   	lg_demodu_set_ADC();
   	lg_demodu_set_AGC();
   	lg_demodu_set_OFDM(params->frequency);
   	lg_tuner_init(params->frequency);   
#endif     	

#ifdef DVBT_P10046
	p10046_demodu(params->frequency, BANDWIDTH_8_MHZ, params->inversion);
#endif

#ifdef DVBC_P10023
	p10023_demodu(params->frequency, params->qam_params.symbol_rate, params->qam_params.modulation, params->inversion);
#endif      

#ifdef DVBT_P10048
	p10048_demodu(params->frequency, BANDWIDTH_8_MHZ, params->inversion);
#endif   

#ifdef DMBT_LGS8913
	lgs8913_demodu(params->frequency, BANDWIDTH_8_MHZ, params->inversion);	
#endif

	return 0;
}

/** -----------------------------------------------------------------------
 * @Brief   : Get the tuner parameters
 * 
 * @Param   : fd -- Handle of the tuner
 * @Param   : params -- Point to the tune parameters: Frequency, 
 * 					Symbol Rate, Spectrum Inversion, QAM
 * 
 * @Returns : The error code
 --------------------------------------------------------------------------*/
int	cs_tuner_get_params(int fd, TUNER_PARAMS_S *params)
{
	/*int	retval = 0;

	#ifdef TUNER_DEBUG
		printf("cs_tuner_get_params function called. \n");
	#endif
		
	//verify  the fd
	if(TUNER_FD != fd)
	{
		#ifdef TUNER_DEBUG
			printf("check tuner handle failure. \n");
		#endif
		return -1;
	}

	retval = demodu_get_params(params);
	if(0 != retval)
	{
		return	TUNER_ERROR_GET_PARAMS;
	}

	#ifdef TUNER_DEBUG
		printf("get tuner param : Frequency:%d , Symbol Rate:%d \n                  Inversion:%d (0-NORMAL,1-INVERTED,2-AUTO)\n                  Qam:%d (0-16,1-32,2-64,3-128,4-256,5-AUTO,)\n",
				params->frequency,params->qam_params.symbol_rate,
				params->inversion,params->qam_params.modulation);
	#endif
	*/
		
	return 0;
}

/** -----------------------------------------------------------------------
 * @Brief   : Get the tuner information
 * 
 * @Param   : fd -- Handle of the tuner
 * @Param   : info -- Point to the tune information: Frequency Range, 
 * 					Symbol Rate Range, DVB/MCSN Mode 
 * 
 * @Returns : The error code
 --------------------------------------------------------------------------*/
int	cs_tuner_get_info(int fd, TUNER_INFO_S *info)
{
	/*int	retval = 0;

	unsigned int	frequency_stepsize = 0;
	unsigned int	symbol_rate_max = 0, symbol_rate_min = 0;
	unsigned int	name = 0, notifier_delay = 0;

	#ifdef TUNER_DEBUG
		printf("cs_tuner_get_info function called. \n");
	#endif
		
	//verify  the fd
	if(TUNER_FD != fd)
	{
		#ifdef TUNER_DEBUG
			printf("check tuner handle failure. \n");
		#endif
		return -1;
	}

	retval = demodu_info(info);
	if(0 != retval)
	{
		return	TUNER_ERROR_GET_INFO;
	}

	info->frequency_stepsize		= frequency_stepsize;
	info->name[0]				= name;
	info->notifier_delay			= notifier_delay;
	info->symbol_rate_max		= symbol_rate_max;
	info->symbol_rate_min			= symbol_rate_min;
	info->symbol_rate_tolerance	= symbol_rate_max-symbol_rate_min;

	#ifdef TUNER_DEBUG
		printf("Get tuner info is : frequency(min ~ max) %d ~ %d \n",info->frequency_min,info->frequency_max);
	#endif
	*/

	return 0;
}

/** -----------------------------------------------------------------------
 * @Brief   : Get the tuner lock status
 * 
 * @Param   : fd -- Handle of the tuner
 * @Param   : status -- Point to the status
 * 
 * @Returns : The error code
 --------------------------------------------------------------------------*/
int	cs_tuner_read_status(int fd, TUNER_STATUS_E *status)
{

	int err_code = 0;


#ifdef DVBT_LG	
	err_code = lg_lock_check();
#endif     	

#ifdef DVBT_P10046
	err_code = p10046_tuner_lock_check();
#endif	

#ifdef DVBC_P10023
	err_code = p10023_tuner_lock_check();
#endif

#ifdef DVBT_P10048
	err_code = p10048_tuner_lock_check();
#endif

#ifdef DMBT_LGS8913
	err_code = lgs8913_tuner_lock_check();
#endif
	if(err_code == 1)
	{
		
		printf("*********** tuner is locked **********!!!auddec\n");
		
		*status = TUNER_STATUS_FE_LOCKED;
	}
	else
	{
		printf("*********** tuner is not locked **********auddec!!!\n");
		
		*status = TUNER_STATUS_UNLOCKED;
		//cyg_thread_delay(10000);
	}
	
	
	//*status = TUNER_STATUS_FE_LOCKED;	
	return 0;
}

/** -----------------------------------------------------------------------
 * @Brief   : Get the tuner Bit Error Rate
 * 
 * @Param   : fd -- Handle of the tuner
 * @Param   : ber -- Point to the ber
 * 
 * @Returns : The error code
 --------------------------------------------------------------------------*/
int	cs_tuner_read_ber(int fd, unsigned int *ber)
{
	/*unsigned int	ber_depth = 0;
	unsigned int	uncor_block = 0;

	#ifdef TUNER_DEBUG
		printf("cs_tuner_read_ber function called. \n");
	#endif
		
	//verify  the fd
	if(TUNER_FD != fd)
	{
		#ifdef TUNER_DEBUG
			printf("check tuner handle failure. \n");
		#endif
		return -1;
	}

	demodu_ber(ber, &ber_depth, &uncor_block);

	#ifdef TUNER_DEBUG
		printf("Get Ber is : 0x%08x * e-%d\n",*ber, ber_depth);
		printf("Get uncorrected block  is : 0x%08x \n", uncor_block);
	#endif
	*/

	return 0;
}

/** -----------------------------------------------------------------------
 * @Brief   : Get the tuner Signal Noise Rate
 * 
 * @Param   : fd -- Handle of the tuner
 * @Param   : snr -- Point to the snr
 * 
 * @Returns : The error code
 --------------------------------------------------------------------------*/
int	cs_tuner_read_snr(int fd, unsigned int *snr)
{
	/*#ifdef TUNER_DEBUG
		printf("cs_tuner_read_snr function called. \n");
	#endif
		
	//verify  the fd
	if(TUNER_FD != fd)
	{
		#ifdef TUNER_DEBUG
			printf("check tuner handle failure. \n");
		#endif
		return -1;
	}

	demodu_snr(snr);

	#ifdef TUNER_DEBUG
		printf("Get SNR is : %d\n",*snr);
	#endif
	*/

	return 0;
}

/** -----------------------------------------------------------------------
 * @Brief   : Get the tuner Signal Strength
 * 
 * @Param   : fd -- Handle of the tuner
 * @Param   : signal_strength -- Point to the signal strength
 * 
 * @Returns : The error code
 --------------------------------------------------------------------------*/
int	cs_tuner_read_signal_strength(int fd, unsigned int *signal_strength)
{
	/*#ifdef TUNER_DEBUG
		printf("cs_tuner_read_signal_strength function called. \n");
	#endif
		
	//verify  the fd
	if(TUNER_FD != fd)
	{
		#ifdef TUNER_DEBUG
			printf("check tuner handle failure. \n");
		#endif
		return -1;
	}
	
	demodu_signal_strength(signal_strength);

	#ifdef TUNER_DEBUG
		printf("Get Signal Strength is : %d \n",*signal_strength);
	#endif
	*/

	return 0;
}
