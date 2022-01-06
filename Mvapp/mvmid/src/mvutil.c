/****************************************************************
*
* FILENAME
*	mvutil.c
*
* PURPOSE 
*	Middle ware for utility functions
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2010.08.19           KB
*
****************************************************************/

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mvos.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/

/****************************************************************
 *                       Type define                            *
 ****************************************************************/

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/

/****************************************************************
 *                         Functions                            *
 ****************************************************************/

/* Sertac: 6.1.2022: below is definitely incorrect, TODO: need to fix this */
U32 BcdStr2Word(U8 *data){
	return atoi(data);
}

/* Sertac: 6.1.2022: added below */
void StrNcpy(char *destination, const char *source, int num){

	strncpy(destination,source,num);

}

/* Sertac: 6.1.2022: added below */
void Str2Lower(U8 *src, U8 *dest){

	U8 i=0;
	while(src[i])
	{
		dest[i] = tolower(src[i]);
		i++;
	}
}


/*
 *  Name         : Char2Hex
 *  Description
 *     Convert 1 Byte Char Data to Hex value
 *  INPUT Arg
 *     U8  data                   : Char Data
 *  OUTPUT Arg
 *     U8 *value                  : result value
 *  RETURN : BOOL
 *     TRUE  : convert success
 *     FALSE : convert fail
 */
BOOL Char2Hex(U8 data, U8 *value)
{
	U8 result;

	result = 0;

	if ((data >= '0') && (data <= '9'))
	{
		result = data - '0';
	}
	else if ((data>= 'A') && (data <= 'F'))
	{
		result = data - 'A' + 10;
	}
	else if ((data >= 'a') && (data <= 'f'))
	{
		result = data - 'a' + 10;
	}
	else
	{
		/* Data is not hex number */
		*value = 0;
		return FALSE;
	}

	*value = result;
	return TRUE;
}

/*
 *  Name         : Bcd2Dec
 *  Description
 *     Convert 1 Byte Char Data to Decimal value
 *  INPUT Arg
 *     U8  data                   : Char Data
 *  OUTPUT Arg
 *     U8 *value                  : result value
 *  RETURN : BOOL
 *     TRUE  : convert success
 *     FALSE : convert fail
 */
BOOL Bcd2Dec(U8 data, U8 *value)
{
	U8 result;

	result = 0;

	if ((data >= '0') && (data <= '9'))
	{
		result = data - '0';
	}
	else
	{
		/* Data is not hex number */
		*value = 0;
		return FALSE;
	}

	*value = result;
	return TRUE;
}

/*
 *  Name         : Str2Hex
 *  Description
 *     Convert 2 Bytes String to Hex value
 *  INPUT Arg
 *     U8 *data                   : Data array pointer
 *  OUTPUT Arg
 *     U8 *value                  : result value
 *  RETURN : BOOL
 *     TRUE  : convert success
 *     FALSE : convert fail
 */
BOOL Str2Hex(U8 *data, U8 *value)
{
	U8 result;
	U8 val;

	result = 0;
	*value = 0;

	if (Char2Hex(data[0], &val))
	{
		result |= val;
		if (Char2Hex(data[1], &val))
		{
			result <<= 4;
			result |= val;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	*value = result;
	return TRUE;
}

/*
 *  Name         : Array2Word
 *  Description
 *     Convert char array data to Unsigned int
 *  INPUT Arg
 *     U8 *data                   : Data array pointer
 *     U8 len                     : length of data
 *  OUTPUT Arg
 *     U32 *deviceId              : Opened device ID
 *  RETURN : U32
 *     unsigned int result
 */
U32 Array2Word(U8 *data, U8 len)
{
	U8   count;
	U32  val;
	
	if ((len > 4) || (len < 1))
	{
		return 0;
	}

	val = 0;
	for (count = 0; count < len; count++)
	{
		val = (val << 8) | ((U32)data[count]);
	}

	return val;
}

void Word2Array(U8 *buff, U32 data, U32 size)
{
	U32 count;

	if ((size < 1) || (size > 4))
	{
		return;
	}

	for (count = size; count > 0; count--)
	{
		buff[count - 1] = (U8)(data & 0xFF);
		data >>= 8;
	}
}

/*
 *  Name         : Bcd2Word
 *  Description
 *     Convert BCD value to unsigned
 *  INPUT Arg
 *     U32 bcdValue               : Data array pointer
 *  OUTPUT Arg
 *     NONE
 *  RETURN : U32
 *     Converted Value
 */
U32 Bcd2Word(U32 bcdValue)
{
	U32 value;
	U32 oneNibble;
	U32 mult;
	U8  counter;

	value = 0;
	mult  = 1;
	for (counter = 0; counter < 8; counter++)
	{
		oneNibble = bcdValue & 0x0000000F;
		if(oneNibble > 9)
		{
			/* Error in the value */
			return 0;
		}
		value = value + (oneNibble * mult);
		bcdValue >>= 4;
		mult *= 10;
	}

	return value;
}

/*
 *  Name         : BcdArray2Word
 *  Description
 *     Convert BCD coded array data to Unsigned int
 *  INPUT Arg
 *     U8 *data                   : Data array pointer
 *     U8 len                     : length of data
 *  OUTPUT Arg
 *     U32 *deviceId              : Opened device ID
 *  RETURN : U32
 *     unsigned int result
 */
U32 BcdArray2Word(U8 *data, U8 len)
{
	U8   count;
	U8   nibbleH;
	U8   nibbleL;
	U32  val;
	
	if ((len > 5) || (len < 1))
	{
		return 0;
	}

	val = 0;
	for (count = 0; count < len; count++)
	{
		nibbleH = ((data[count] & 0xF0) >> 4) * 10;
		nibbleL = data[count] & 0x0F;
		if ((nibbleH > 90) || (nibbleL > 9))
		{
			return 0;
		}
		val = (val * 100) + ((U32)(nibbleH + nibbleL));
	}

	return val;
}

/*
 *  Name         : XtoPowerY
 *  Description
 *     Compute  x^y (where x and y are integers) 
 *  INPUT Arg
 *     S32 Number                 : Base Number  -> x
 *     U32 power                  : Power        -> y
 *  OUTPUT Arg
 *     NOME
 *  RETURN : S32
 *     Result value
 */
S32 XtoPowerY(S32 number, U32 power)
{
	U32 count;
	S32 result = 1;
	
	for(count = 0; count < power; count++)
	{
		result *= number;
	}
		
	return result;
}

