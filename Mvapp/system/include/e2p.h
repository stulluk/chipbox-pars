#ifndef __E2P_H_
#define __E2P_H_

typedef U32 E2P_Handle_t;

int E2P_Init(void); /* For EEPROM access problem by KB Kim 2011.09.13 */
int E2P_Open(unsigned int chipaddr); /* For EEPROM access problem by KB Kim 2011.09.13 */
CS_ErrorCode_t  E2P_Read(E2P_Handle_t Handle, U16 SubAddr, U8 *Data, U32 Size);
CS_ErrorCode_t  E2P_Write(E2P_Handle_t Handle, U16 SubAddr, U8 *Data, U32 Size);

#endif


