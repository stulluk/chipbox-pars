/*
*
*Copyright 2006, Legend Silicon Corp. All rights reserved.
*
*File name: 	lgs_types.h
*Description : 	types for lgs driver
*
*/

#ifndef _LGS_TYPES_H_
#define _LGS_TYPES_H_

/**
 *	/file 	lgs_types.h
 *	/brief 	types for lgs driver
 */

#ifdef __cplusplus
extern "C" {
#endif

#define SBIT	unsigned char	/**< 1 bit */

#define INT8	char		/**< 8 bits, bit 7 is the signed bit */
#define INT16	short		/**< 16 bits, bit 15 is the signed bit */
#define INT32	long		/**< 32 bits, bit 31 is the signed bit */

#define UINT8	unsigned char	/**< 8 bits */
#define UINT16	unsigned short	/**< 16 bits */
#define UINT32	unsigned long	/**< 32 bits */

typedef int LGS_HANDLE;		/**< handle type */
typedef union 
{

   unsigned int	uiWord32;

   struct 
   {

	   	unsigned short   sLo16;
   		unsigned short   sHi16;
   } unShort;

} WORD2SHORT;
typedef	union
{
	short	sWord16;
	struct
	{
		unsigned char	ucByte0;	/* LSB */
		unsigned char	ucByte1; /* MSB */
   } byte;
} SHORT2BYTE;
typedef	union
{
	unsigned int	uiWord32;
	struct
	{
		unsigned char	ucByte0;	/* LSB */
		unsigned char	ucByte1;
		unsigned char	ucByte2;
		unsigned char	ucByte3;	/* MSB */
	} byte;
} WORD2BYTE;
#ifdef __cplusplus
}
#endif

#endif /*_LGS_TYPES_H_*/
