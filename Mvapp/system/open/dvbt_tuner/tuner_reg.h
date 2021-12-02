#ifndef __TUNER_REG_H
#define __TUNER_REG_H

//#include "csi2c.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	U8		Address;					/*	Address	*/
	U8		Default_Value;			/*	Default value	*/
	U8		Current_Value;			/*	Current value	*/
}tReg_Info_t;

typedef struct{
	U32                             Dev_Address;
	U16				Reg_Count;
}tReg_OpenParams_t;

int Reg_Init(tReg_OpenParams_t * params);
int Reg_AddDemodReg(U32 Id, U8 address, U8 def_value);
int Reg_SetOneDemodReg(U32 reg_id, U8 Data);
int Reg_GetOneDemodReg(U32 reg_id, U8 * Data);
int Reg_ApplyDefault(void);

int tuner_gpio_reset(void);


#ifdef __cplusplus
}
#endif

#endif

