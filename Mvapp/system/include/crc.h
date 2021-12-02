#ifndef _CRC_H
#define _CRC_H

#include "mvosapi.h"

BOOL CS_CRC_Init(void);
void CS_CRC_16bCalculate(U8  *buffer, U32 size, U16 *CRC16);
void CS_CRC_32bCalculate(U8 *buffer, U32 size, U32 *CRC32);

#endif

