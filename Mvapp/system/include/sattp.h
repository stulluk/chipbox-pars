#ifndef __SATDATA_H
#define __SATDATA_H

#ifdef __cplusplus
extern "C" {
#endif //#ifdef __cplusplus

#include "mvapp_def.h"

#define UCChar 											unsigned short

#define	MAX_TP_COUNT									5000

#ifndef NEW_LNB_TYPE_DEFINE
#define DEFAULT_NORMAL_C_LOCAL_FREQUAL					(5150)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL					(11300)
#define DEFAULT_UNIVERSAL_LOW_LOCAL_FREQUAL				(9750)
#define DEFAULT_UNIVERSAL_HIGH_LOCAL_FREQUAL			(10600)
#define DEFAULT_WIDE_LOW_LOCAL_FREQUAL					(9750)
#define DEFAULT_WIDE_HIGH_LOCAL_FREQUAL					(10750)
#define DEFAULT_OCSDP_LOW_LOCAL_FREQUAL					(11250)
#define DEFAULT_OCSDP_HIGH_LOCAL_FREQUAL				(11250)
#define DEFAULT_OCS_LOW_LOCAL_FREQUAL					(5150)
#define DEFAULT_OCS_HIGH_LOCAL_FREQUAL					(5750)
#define DEFAULT_LEGACY_STND_LOW_LOCAL_FREQUAL			(11250)
#define DEFAULT_LEGACY_STND_HIGH_LOCAL_FREQUAL			(11250)
#define DEFAULT_LEGACY_TWIN_LOW_LOCAL_FREQUAL  			(9750)
#define DEFAULT_LEGACY_TWIN_HIGH_LOCAL_FREQUAL			(10600)
#define DEFAULT_LEGACY_QUAD_LOW_LOCAL_FREQUAL			(9750)
#define DEFAULT_LEGACY_QUAD_HIGH_LOCAL_FREQUAL			(10600)
#else  // #ifndef NEW_LNB_TYPE_DEFINE
#define DEFAULT_NORMAL_C_LOCAL_FREQUAL_5150				(5150)
#define DEFAULT_NORMAL_C_LOCAL_FREQUAL_5750				(5750)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL_9750			(9750)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL_10000			(10000)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL_10600			(10600)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL_10678			(10678)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL_10700			(10700)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL_10750			(10750)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL_11000			(11000)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL_11200			(11200)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL_11250			(11250)
#define DEFAULT_NORMAL_KU_LOCAL_FREQUAL_11300			(11300)
#define DEFAULT_UNIVERSAL_LOW_LOCAL_FREQUAL_5150		(5150)
#define DEFAULT_UNIVERSAL_HIGH_LOCAL_FREQUAL_5750		(5750)
#define DEFAULT_UNIVERSAL_LOW_LOCAL_FREQUAL_9750		(9750)
#define DEFAULT_UNIVERSAL_HIGH_LOCAL_FREQUAL_10600		(10600)
#define DEFAULT_UNIVERSAL_HIGH_LOCAL_FREQUAL_10700		(10700)
#define DEFAULT_UNIVERSAL_HIGH_LOCAL_FREQUAL_10750		(10750)
#define DEFAULT_OCSDP_LOW_LOCAL_FREQUAL					(11250)
#define DEFAULT_OCSDP_HIGH_LOCAL_FREQUAL				(11250)
#define DEFAULT_OCS_LOW_LOCAL_FREQUAL					(5150)
#define DEFAULT_OCS_HIGH_LOCAL_FREQUAL					(5750)
#define DEFAULT_LEGACY_STND_LOW_LOCAL_FREQUAL			(11250)
#define DEFAULT_LEGACY_STND_HIGH_LOCAL_FREQUAL			(11250)
#define DEFAULT_LEGACY_TWIN_LOW_LOCAL_FREQUAL  			(9750)
#define DEFAULT_LEGACY_TWIN_HIGH_LOCAL_FREQUAL			(10600)
#define DEFAULT_LEGACY_QUAD_LOW_LOCAL_FREQUAL			(9750)
#define DEFAULT_LEGACY_QUAD_HIGH_LOCAL_FREQUAL			(10600)
#endif  // #ifndef NEW_LNB_TYPE_DEFINE

/* Define for u8LNBPower */
#define 	LNB_POWER_OFF       						0   		/**< LNB power off. */	
#define 	LNB_POWER_ON       							1   		/**< LNB power off. */	
#define 	LNB_POWER_13V18V    						2   		/**< LNB power can be 13V or 18V. */	
#define 	LNB_POWER_18V       						3  			/**< LNB power is 18V, Horizontal. */	
#define 	LNB_POWER_13V       						4   		/**< LNB power is 13V, Vertical, */	

#ifdef ENABLE_LNB_LONGLENGTH
#define 	LNB_POWER_14V19V    						5
#define 	LNB_POWER_19V       						6
#define 	LNB_POWER_14V       						7
#endif

#define		P_V											0
#define 	P_H											1
#define		K22_OFF										0
#define 	K22_ON										1
#define		LNB_0										0
#define		LNB_12										1
#define		LNB_OFF										0
#define 	LNB_ON										1

#define		Diseqc_OFF									0
#define 	Diseqc_4_1									1
#define 	Diseqc_4_2									2
#define 	Diseqc_4_3									3
#define 	Diseqc_4_4									4
#define 	Diseqc_16_1									5
#define 	Diseqc_16_2									6
#define 	Diseqc_16_3									7
#define 	Diseqc_16_4									8
#define 	Diseqc_16_5									9
#define 	Diseqc_16_6									10
#define 	Diseqc_16_7									11
#define 	Diseqc_16_8									12
#define 	Diseqc_16_9									13
#define 	Diseqc_16_10								14
#define 	Diseqc_16_11								15
#define 	Diseqc_16_12								16
#define 	Diseqc_16_13								17
#define 	Diseqc_16_14								18
#define 	Diseqc_16_15								19
#define 	Diseqc_16_16								20


typedef enum
{
	EN_LNB_TYPE_C_BAND_5150,
	EN_LNB_TYPE_C_BAND_5750,
	EN_LNB_TYPE_KU_BAND_9750,
	EN_LNB_TYPE_KU_BAND_10000,
	EN_LNB_TYPE_KU_BAND_10600,
	EN_LNB_TYPE_KU_BAND_10678,
	EN_LNB_TYPE_KU_BAND_10700,
	EN_LNB_TYPE_KU_BAND_10750,
	EN_LNB_TYPE_KU_BAND_11000,
	EN_LNB_TYPE_KU_BAND_11200,
	EN_LNB_TYPE_KU_BAND_11250,
	EN_LNB_TYPE_KU_BAND_11300,
	EN_LNB_TYPE_UNIVERSAL_5150_5750,
	EN_LNB_TYPE_UNIVERSAL_5750_5150,
	EN_LNB_TYPE_UNIVERSAL_9750_10600,
	EN_LNB_TYPE_UNIVERSAL_9750_10700,
	EN_LNB_TYPE_UNIVERSAL_9750_10750,	
	EN_LNB_TYPE_DIGITURK1,
	EN_LNB_TYPE_DIGITURK2,
	EN_LNB_TYPE_DIGITURK3,
	EN_LNB_TYPE_DIGITURK4,
#ifdef FOR_USA
	EN_LNB_TYPE_OCS_DP,
	EN_LNB_TYPE_OCS,
	EN_LNB_TYPE_LEGACY_TWIN,
	EN_LNB_TYPE_LEGACY_QUAD,
#endif
	EN_LNB_TYPE_OTHERS,
	EN_LNB_TYPE_DEFINE_MAX

}LNB_TYPE_DEFINE;

typedef enum {
	EN_LONGITUDE_TURKSAT_1C__2A_,
	EN_LONGITUDE_HOTBIRD_6_7A_8,
	EN_LONGITUDE_EUROBIRD_9,
	EN_LONGITUDE_TORR,
	EN_SAT_MAX_LONGITUDE
} EN_SAT_LST;

/************************* Global Variable **********************/



/************************* Global Function **********************/

void	MV_DB_Get_Default_SatTPData(void);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus
#endif  // #ifndef __SATDATA_H

