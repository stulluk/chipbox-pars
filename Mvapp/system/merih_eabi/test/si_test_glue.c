/*
 * si_test_glue.c - symbols that si_ts_test needs from the rest of mvapp.
 */
#include <stdlib.h>

/* SI diagnostics follow MERIH_TUNER_DEBUG, as in mvapp (tuner.c). */
int tunerDebugEnabled(void)
{
	const char *value = getenv("MERIH_TUNER_DEBUG");

	return (value != NULL) && (value[0] == '1');
}

/* Clock / settings helpers referenced by mvos.c (OS time API, unused by the SI test). */
int CS_MW_GetTimeMode(void) { return 0; }
int CS_MW_GetTimeZone(void) { return 0; }
int CS_MW_GetTimeRegion(void) { return 0; }
int CS_DT_YMDtoMJD(void) { return 0; }
int CS_DT_HMtoUTC(void) { return 0; }
void CS_DT_ManualSetDateAndTime(void) {}
void CS_DBU_SaveUserSettingDataInHW(void) {}
