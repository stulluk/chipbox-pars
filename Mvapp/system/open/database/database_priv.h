#ifndef __DATABASE_PRIV_H
#define __DATABASE_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	eCS_DB_DISCARDSERVICE =0,
	eCS_DB_ADDSERVICE,
	eCS_DB_SWAPSERVICE	
}tCS_DB_ServcieAddedMode;

typedef enum 
{
	eCS_DB_INVALID = 0,
	eCS_DB_VALID
}tCS_DB_ValidStatus;

#define Ter_Freq_Max_Offset						2000

#define mTerFreqInRange(x,y) (((x == y) || ((x >= y - Ter_Freq_Max_Offset) && (x <= y + Ter_Freq_Max_Offset))) ? 1:0) 

#ifdef __cplusplus
}
#endif

#endif 

