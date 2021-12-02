

#include "linuxos.h"

#include "crc.h"

#define CRC16_MAX_COEFFICIENTS	256
#define CRC32_MAX_COEFFICIENTS	256
#define CRC16_POLYNOMIAL		0x1021
#define CRC32_POLYNOMIAL		0x04C11DB7
#define CRC32_SIZE				4	/* size of the CRC 32 in bytes */
#define CRC16_SIZE				2	/* size in bytes of the CRC 16 */


/* this table array is created with the ccitt polynome : x^16+x^12+x^5+1 */

U16 CRC_crc16_table[CRC16_MAX_COEFFICIENTS];
U32 CRC_crc32_table[CRC32_MAX_COEFFICIENTS];


/********************************************************/
/*              Local Functions Declarations (LOCAL)    */
/********************************************************/


/********************************************************/
/*              Functions Definitions (LOCAL/GLOBAL)    */
/********************************************************/


 /******************************************************************************
 * Function Name : BL_CRC_Init
 *
 * Description   :
 *
 *		This function calculates all coefficients of CRC_crc16_table and CRC_crc32_table.
 *
 * Input parameters  :
 *
 *		pMsg	message to treat
 *
 * Output parameters  :
 *
 *		TRUE	OK
 *
 * Side effects  :
 *
 * Comment     :
 *
 *****************************************************************************/

BOOL CS_CRC_Init(void)
{
	U32 loopCntCoef;
	U32 loopCntBit;
	U16 coef16;
	U32 coef32;

	for (loopCntCoef = 0; loopCntCoef < CRC16_MAX_COEFFICIENTS; loopCntCoef++)
	{
		coef16 = (U16)loopCntCoef << 8;
		for (loopCntBit = 0; loopCntBit < 8; loopCntBit++)
		{
			if (coef16 & 0x8000)
				coef16 = ((coef16 << 1) ^ CRC16_POLYNOMIAL);
			else
				coef16 <<= 1;
		}
		CRC_crc16_table[loopCntCoef] = coef16;
	}

	for (loopCntCoef = 0; loopCntCoef < CRC32_MAX_COEFFICIENTS; loopCntCoef++)
	{
		coef32 = loopCntCoef << 24;
		for (loopCntBit=0; loopCntBit<8; loopCntBit++) 
		{
			if (coef32 & 0x80000000)
				coef32 = ((coef32 << 1) ^ CRC32_POLYNOMIAL);
			else
				coef32 <<= 1;
		}
		CRC_crc32_table[loopCntCoef] = coef32;
	}

	return(TRUE);
}


 /******************************************************************************
 * Function Name : CS_CRC_16bCalculate
 *
 * Description   :
 *
 *		Computes a 16 bits CRC coefficients on the buffer.
 *		The polynomial is x^16+x^12+x^5+x^1+1
 *
 * Input parameters  :
 *
 *		buffer 	: address of the input buffer
 *		size 		: size of the buffer
 *		crc16 	: 16 bits CRC coefficient that is calculated
 *
 * Output parameters  :
 *
 * Side effects  :
 *
 * Comment     :
 *
 *****************************************************************************/

void CS_CRC_16bCalculate(U8  *buffer, U32 size, U16 *CRC16)
{
	U16 crc16 = 0x0000;
	U32 cntByte;

	for (cntByte = 0; cntByte < size; cntByte++)
        crc16 = (crc16 << 8 ) ^ CRC_crc16_table[((crc16 >> 8) ^ *buffer++) & 0xFF];
	*CRC16 = crc16;
}

 /******************************************************************************
 * Function Name : CS_CRC_32bCalculate
 *
 * Description   :
 *
 *		Computes a 32 bits CRC coefficients on the buffer.
 *      The polynomial is :
 *		x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
 *
 * Input parameters  :
 *
 *		buffer : address of the input buffer
 *		size : size of the buffer
 *		CRC32Calculated : 32 bits CRC coefficient that is calculated
 *
 * Output parameters  :
 *
 * Side effects  :
 *
 * Comment     :
 *
 *****************************************************************************/

void CS_CRC_32bCalculate(U8 *buffer, U32 size, U32 *CRC32)
{
	U32 crc32 = 0xFFFFFFFF;
	U32 cntByte;

	for (cntByte = 0; cntByte < size; cntByte++)
	{
		crc32 = (crc32 << 8 ) ^ CRC_crc32_table[((crc32 >> 24) ^ *buffer++) & 0xFF];
	}
	*CRC32 = crc32;
}


