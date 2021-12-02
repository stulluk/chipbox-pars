#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include "mvosapi.h"
#include "mvmiscapi.h"
#include "dvbtuner.h"
#include "tunerinit.h"
#include "fe_mngr.h"
#include "mwsvc.h"
#include "cs_app_common.h"

#define CONFIG_FILE  "/usr/work0/app/tunercfg.cfg"

// static long x = LONG_MIN;

U32					*Tuner_HandleId;

U32 TuneNotifySem;
extern tCS_FE_State	Current_FE_state;

U32 char2val(char *data)
{
	U32    len;
	U32    i;
	U32    result;
	U32    tmpVal;

	len = (U32)strlen(data);
	if (len > MAX_UNSIGNED_DIGIT)
	{
		len = MAX_UNSIGNED_DIGIT;
	}

	if (len == 0)
	{
		return (U32)MAX_UNSIGNED_VAL;
	}

	result = 0;

	for (i = 0; i < len; i++)
	{
		result  *= 10;
		if ((data[i] < ASCII_0) || (data[i] > ASCII_9))
		{
			/* Not a Decimal number */
			// OsDebugPrintf("0x%02X is not a Decimal number!\n", data[i]);
			return (U32)MAX_UNSIGNED_VAL;
		}
		tmpVal = (U32)(data[i] - ASCII_0);
		result += tmpVal;
	}

	return result;
}

U32 Str2Word(char *data)
{
	U32    len;
	U32    i;
	U32    result;
	U8     tmpVal;

	len = (U32)strlen(data);
	if (len > MAX_HEX_DIGIT)
	{
		len = MAX_HEX_DIGIT;
	}

	if (len == 0)
	{
		return (U32)MAX_UNSIGNED_VAL;
	}

	result = 0;

	for (i = 0; i < len; i++)
	{
		if (Char2Hex(data[i], &tmpVal))
		{
			result  <<= 4;
			result |= (U32)tmpVal;
		}
		else
		{
			// OsDebugPrintf("Str2Word Data Error : 0x%02X\n", data[i]);
			return (U32)MAX_UNSIGNED_VAL;
		}
	}

	return result;
}

BOOL TuneResultNotify(TunerResult_t *result)
{
	TunerSignalState_t   *siganlState;    /* Signal tune status */
	TunerTuneParam_t     *tuneResult;     /* Signal tune result */

	siganlState = result->SiganlState;
	tuneResult  = result->TuneResult;

	switch (siganlState->LockState)
	{
		case SIGNAL_UNLOCK  :
			if (Current_FE_state == eCS_FE_STATE_LOCKED)
			{
				FE_Notify(eCS_FE_SIGNAL_LOST);
			}
			else
			{
				FE_Notify(eCS_FE_UNLOCKED);
			}
			Current_FE_state = eCS_FE_STATE_UNLOCKED;
#ifdef TUNER_DEBUG_ON
			OsDebugPrtf("Signal Unlocked [%d] [%d]\n", siganlState->Strength, siganlState->Quality);
#endif // #ifdef TUNER_DEBUG_ON
			break;
		case SIGNAL_CARRIER :
			if (Current_FE_state == eCS_FE_STATE_LOCKED)
			{
				FE_Notify(eCS_FE_SIGNAL_LOST);
			}
			else
			{
				FE_Notify(eCS_FE_UNLOCKED);
			}
			Current_FE_state = eCS_FE_STATE_UNLOCKED;
			// OsDebugPrtf("Signal Carrier [%d] [%d]\n", siganlState->Strength, siganlState->Quality);
			break;
		case SIGNAL_LOCK    :
			FE_Notify(eCS_FE_LOCKED);
			Current_FE_state = eCS_FE_STATE_LOCKED;
#ifdef TUNER_DEBUG_ON
			OsDebugPrtf("Signal LOCK!!! [%d] [%d]\n", siganlState->Strength, siganlState->Quality);
			// OsDebugPrtf("\tLocked [%d] Frequency[%d], Symbol[%d]\n", tuneResult->DvbType, tuneResult->TpFrequency, tuneResult->Symbolrate);
			// OsDebugPrtf("\tRollOFf[%d], PunctureRate[%d], ModCode[%d]\n", tuneResult->RollOff, tuneResult->PunctureRate, tuneResult->ModCode);
#endif // #ifdef TUNER_DEBUG_ON
			break;
	}

	// OsSemaphoreSignal(TuneNotifySem);
	return 0;
}

void SkipSpace(U8 *lineData, U32 length, U32 *dataPointer)
{
	U32 pointer;

	pointer = *dataPointer;
	while ((pointer < length) && (lineData[pointer] == ASCII_SPACE))
	{
		pointer++;
	}

	*dataPointer = pointer;
}

U8 GetCapitalCharacter(U8 data)
{
	if ((ASCII_a <= data) && (data <= ASCII_z))
	{
		/* change Capital charactor */

		return (data - 0x20);
	}

	return data;
}

void ReadItem(U8 *lineData, U32 length, U32 initPointer, U8 *name, U8 *val)
{
	U32 pointer;
	U32 dataLength;

	pointer = initPointer;
	dataLength = 0;
	while ((pointer < length) && (dataLength < ITEM_NAME_LENGTH) && (lineData[pointer] != ASCII_EQUAL))
	{
		name[dataLength] = lineData[pointer];
		pointer++;
		dataLength++;
	}
	// OsDebugPrintf("\tName2[%s]\n", name);
	
	if (lineData[pointer] == ASCII_EQUAL)
	{
		pointer++;
		SkipSpace(lineData, length, &pointer);
		dataLength = 0;
		while ((pointer < length) && (dataLength < ITEM_VALUE_LENGTH) &&
			   (lineData[pointer] != ASCII_SPACE) && (lineData[pointer] != ASCII_LF))
		{
			val[dataLength] = GetCapitalCharacter(lineData[pointer]);
			pointer++;
			dataLength++;
		}
	}
}

void ReadTitle(U8 *lineData, U32 length, U32 initPointer, U8 *name)
{
	U32 pointer;
	U32 dataLength;

	pointer = initPointer;
	dataLength = 0;
	while ((pointer < length) && (dataLength < ITEM_NAME_LENGTH) && (lineData[pointer] != ASCII_BRACKET_R))
	{
		name[dataLength] = lineData[pointer];
		pointer++;
		dataLength++;
	}
	// OsDebugPrintf("\tName1[%s]\n", name);
}

ConfigDataType ProcessOneLine(FILE* cfgFile, U8 *name, U8 *value)
{
	static U32 lineCount = 0;
	U32  pointer;
	U32  lineLength;
	char *readStatus;
	U8   readBuffer[MAX_LINE_LENGTH];

	memset(readBuffer, 0, MAX_LINE_LENGTH);
	readStatus = fgets(readBuffer, MAX_LINE_LENGTH, cfgFile);
	if (readStatus == NULL)
	{
		return DATA_EOF;
	}

	// OsDebugPrintf("%03d: %s\n", lineCount, readBuffer);
	lineCount++;
	
 	pointer = 0;
	lineLength = strlen(readBuffer);
	SkipSpace (readBuffer, lineLength, &pointer);
	switch (readBuffer[pointer])
	{
		case ASCII_SHARP :
			return DATA_COMMENT;
		case ASCII_BRACKET_L :
			pointer++;
			ReadTitle(readBuffer, lineLength, pointer, name);
			return DATA_TITLE;
		default              :
			ReadItem(readBuffer, lineLength, pointer, name, value);
			break;
	}
	
	// OsDebugPrintf("\tname[%s], Value[%s]\n", name, value);

	return DATA_ITEM;
}

BOOL CompareName(U8 *refName, U8 *getName)
{
	U32 nameLen;
	U32 gnameLen;
	U32 count;

	// OsDebugPrintf("\tCompare Name[%s] with Name[%s] \n", refName, getName);
	nameLen = strlen (refName);
	gnameLen = strlen (getName);
	if (nameLen != gnameLen)
	{
		// OsDebugPrintf("\tCompare Name Len[%d] with Name[%d] \n", nameLen, gnameLen);
		return TRUE;
	}

	for (count = 0; count < nameLen; count++)
	{
		if (GetCapitalCharacter(refName[count]) != GetCapitalCharacter(getName[count]))
		{
			return TRUE;
		}
	}

	return FALSE;
}

ConfigDataType GetInitParameter(FILE* cfgFile, void *initData, U8 *nextTitle)
{
	TunerInitParam_t *initParam;
	ConfigDataType    dataType;
	U8                name[ITEM_NAME_LENGTH];
	U8                value[ITEM_VALUE_LENGTH];

	initParam = (TunerInitParam_t *)initData;

	do
	{
		memset(name, 0, ITEM_NAME_LENGTH);
		memset(value, 0, ITEM_VALUE_LENGTH);

		dataType = ProcessOneLine(cfgFile, name, value);
		switch (dataType)
		{
			case DATA_COMMENT :
				// OsDebugPrintf("\tComment\n");
				break;
			case DATA_ITEM    :
				if (CompareName(ITEM_NUMBER_OF_TUNER, name) == FALSE)
				{
					initParam->NumberOfTuner = (U8)char2val(value);
					// OsDebugPrintf("\tNumberOfTuner = %d\n", initParam->NumberOfTuner);
				}
				break;
			case DATA_TITLE  :
				strcpy ((char *)nextTitle, (char *)name);
				break;
			default          :
				break;
		}
	} while ((dataType == DATA_ITEM) || (dataType == DATA_COMMENT));

	return dataType;
}

ConfigDataType GetOpenParameter(FILE* cfgFile, void *openData, U8 *nextTitle)
{
	TunerOpenParam_t *openParam;
	ConfigDataType    dataType;
	U8                name[ITEM_NAME_LENGTH];
	U8                value[ITEM_VALUE_LENGTH];

	openParam = (TunerOpenParam_t *)openData;
	openParam->TunerOutputMode = 0;

	do
	{
		memset(name, 0, ITEM_NAME_LENGTH);
		memset(value, 0, ITEM_VALUE_LENGTH);

		dataType = ProcessOneLine(cfgFile, name, value);
		switch (dataType)
		{
			case DATA_COMMENT :
				// OsDebugPrintf("\tComment\n");
				break;
			case DATA_ITEM    :
				if (CompareName(ITEM_TUNER_NUMBER, name) == FALSE)
				{
					openParam->TunerNumber = (U8)char2val(value);
					// OsDebugPrintf("\tOPEN TunerNumber = %d\n", openParam->TunerNumber);
				}
				else if (CompareName(ITEM_PLL_REPEATE_MODE, name) == FALSE)
				{
					if (CompareName(VALUE_ON, value) == TRUE)
					{
						openParam->PllI2CRepeate = 0;
					}
					else
					{
						openParam->PllI2CRepeate = 1;
					}
					
					// OsDebugPrintf("\tOPEN PllI2CRepeate = %d\n", openParam->PllI2CRepeate);
				}
				else if (CompareName(ITEM_OUTPUT_MODE, name) == FALSE)
				{
					if (CompareName(VALUE_TUNER_PARALLEL, value) == FALSE)
					{
						openParam->TunerOutputMode |= TUNER_OUT_PARALLEL;
					}
					else if (CompareName(VALUE_TUNER_PARALLEL_CI, value) == FALSE)
					{
						openParam->TunerOutputMode |= TUNER_OUT_PARALLEL_CI;
					}
					else if (CompareName(VALUE_TUNER_SERIAL, value) == FALSE)
					{
						openParam->TunerOutputMode |= TUNER_OUT_SERIAL;
					}
					else if (CompareName(VALUE_TUNER_SERIAL_CONT, value) == FALSE)
					{
						openParam->TunerOutputMode |= TUNER_OUT_SERIAL_CONT;
					}
					else
					{
						/* Default Output mode is TUNER_OUT_PARALLEL_CI */
						openParam->TunerOutputMode |= TUNER_OUT_PARALLEL_CI;
					}
					
					// OsDebugPrintf("\tOPEN TunerOutputMode = %d\n", openParam->TunerOutputMode);
				}
				else if (CompareName(ITEM_OUTPUT_CLK, name) == FALSE)
				{
					if (CompareName(VALUE_TUNER_CLK_FALLING, value) == FALSE)
					{
						openParam->TunerOutputMode |= TUNER_OUT_FALLING;
					}
					else if (CompareName(VALUE_TUNER_CLK_RISING, value) == FALSE)
					{
						openParam->TunerOutputMode |= TUNER_OUT_RISING;
					}
					else
					{
						/* Default Output mode is TUNER_OUT_PARALLEL_CI */
						openParam->TunerOutputMode |= TUNER_OUT_RISING;
					}
					
					// OsDebugPrintf("\tOPEN TunerOutputMode = %d\n", openParam->TunerOutputMode);
				}
				else if (CompareName(ITEM_TUNER_TYPE, name) == FALSE)
				{
					if (CompareName(VALUE_TUNER_DVB_S, value) == FALSE)
					{
						openParam->Type = TUNER_SATELLITE;
					}
					else if (CompareName(VALUE_TUNER_DVB_C, value) == FALSE)
					{
						openParam->Type = TUNER_CABLE;
					}
					else if (CompareName(VALUE_TUNER_DVB_T, value) == FALSE)
					{
						openParam->Type = TUNER_TERRESTRIAL;
					}
					// OsDebugPrintf("\tOPEN Type = %d\n", openParam->Type);
				}
				else if (CompareName(ITEM_DEMODULATOR_DEVICE, name) == FALSE)
				{
					if (CompareName(VALUE_STV0903, value) == FALSE)
					{
						openParam->Demodulator = STV0903;
					}
					else if (CompareName(VALUE_AVL2108, value) == FALSE)
					{
						openParam->Demodulator = AVL2108;
					}
					else
					{
						openParam->Demodulator = AVL2108;
					}
					// OsDebugPrintf("\tOPEN Demodulator = %d\n", openParam->Demodulator);
				}
				else if (CompareName(ITEM_PLL_DEVICE, name) == FALSE)
				{
					if (CompareName(VALUE_STV6110, value) == FALSE)
					{
						openParam->TunerPll = STV6110;
					}
					else if (CompareName(VALUE_IX2564, value) == FALSE)
					{
						openParam->TunerPll = IX2564;
					}
					else
					{
						openParam->TunerPll = IX2564;
					}
					// OsDebugPrintf("\tOPEN TunerPll = %d\n", openParam->TunerPll);
				}
				break;
			case DATA_TITLE  :
				strcpy ((char *)nextTitle, (char *)name);
				break;
			default          :
				break;
		}
	} while ((dataType == DATA_ITEM) || (dataType == DATA_COMMENT));

	return dataType;
}

ConfigDataType GetI2CParameter(FILE* cfgFile, void *i2cData, U8 *nextTitle)
{
	I2cOpenParam_t   *i2cParam;
	ConfigDataType    dataType;
	U8                name[ITEM_NAME_LENGTH];
	U8                value[ITEM_VALUE_LENGTH];

	i2cParam = (I2cOpenParam_t *)i2cData;

	do
	{
		memset(name, 0, ITEM_NAME_LENGTH);
		memset(value, 0, ITEM_VALUE_LENGTH);

		dataType = ProcessOneLine(cfgFile, name, value);
		switch (dataType)
		{
			case DATA_COMMENT :
				// OsDebugPrintf("\tComment\n");
				break;
			case DATA_ITEM    :
				if (CompareName(ITEM_I2C_NUMBER, name) == FALSE)
				{
					i2cParam->DevideNo = (U8)char2val(value);
					// OsDebugPrintf("\tI2C DevideNo = %d\n", i2cParam->DevideNo);
				}
				else if (CompareName(ITEM_I2C_ADDRESS, name) == FALSE)
				{
					i2cParam->Address = (U8)Str2Word(value);
					// OsDebugPrintf("\tI2C Address = 0x%02X\n", i2cParam->Address);
				}
				else if (CompareName(ITEM_I2C_ADDRESS_LEN, name) == FALSE)
				{
					i2cParam->SubAddrLen = (U8)char2val(value);
					// OsDebugPrintf("\tI2C SubAddrLen = %d\n", i2cParam->SubAddrLen);
				}
				else if (CompareName(ITEM_I2C_SPEED, name) == FALSE)
				{
					i2cParam->Speed = (U8)char2val(value);
					// OsDebugPrintf("\tI2C Speed = %d\n", i2cParam->Speed);
				}
				break;
			case DATA_TITLE  :
				strcpy ((char *)nextTitle, (char *)name);
				break;
			default          :
				break;
		}
	} while ((dataType == DATA_ITEM) || (dataType == DATA_COMMENT));

	return dataType;
}

ConfigDataType GetGpioParameter(FILE* cfgFile, void *gpioData, U8 *nextTitle)
{
	GpioOpenParam_t  *gpioParam;
	ConfigDataType    dataType;
	U8                name[ITEM_NAME_LENGTH];
	U8                value[ITEM_VALUE_LENGTH];

	gpioParam = (GpioOpenParam_t *)gpioData;

	do
	{
		memset(name, 0, ITEM_NAME_LENGTH);
		memset(value, 0, ITEM_VALUE_LENGTH);
		dataType = ProcessOneLine(cfgFile, name, value);
		switch (dataType)
		{
			case DATA_COMMENT :
				// OsDebugPrintf("\tComment\n");
				break;
			case DATA_ITEM    :
				if (CompareName(ITEM_GPIO_PORT, name) == FALSE)
				{
					gpioParam->PortNumber = (U8)char2val(value);
					// OsDebugPrintf("\tGPIO PortNumber = %d\n", gpioParam->PortNumber);
				}
				else if (CompareName(ITEM_GPIO_BIT, name) == FALSE)
				{
					gpioParam->BitNumber = (U8)char2val(value);
					// OsDebugPrintf("\tGPIO BitNumber = %d\n", gpioParam->BitNumber);
				}
				else if (CompareName(ITEM_GPIO_MODE, name) == FALSE)
				{
					if (CompareName(VALUE_GPIO_OUTPUT, value) == FALSE)
					{
						gpioParam->Mode = GPIO_WRITE_MODE;
					}
					else if (CompareName(VALUE_GPIO_INPUT, value) == FALSE)
					{
						gpioParam->Mode = GPIO_READ_MODE;
					}
					else if (CompareName(VALUE_GPIO_IO, value) == FALSE)
					{
						gpioParam->Mode = GPIO_IO_MODE;
					}
					else
					{
						gpioParam->Mode = GPIO_WRITE_MODE;
					}
					// OsDebugPrintf("\tGPIO Mode = %d\n", gpioParam->Mode);
				}
				else if (CompareName(ITEM_GPIO_INIT_VALUE, name) == FALSE)
				{
					gpioParam->Value = (U8)char2val(value);
					// OsDebugPrintf("\tGPIO Init Value = %d\n", gpioParam->Value);
				}
				break;
			case DATA_TITLE  :
				strcpy ((char *)nextTitle, (char *)name);
				break;
			default          :
				break;
		}
	} while ((dataType == DATA_ITEM) || (dataType == DATA_COMMENT));

	return dataType;
}

void WriteDefaultConfigData(void)
{
	FILE*		      tunerCfg;

	tunerCfg = fopen(CONFIG_FILE, "wt");

	/* Init Param */
	fprintf(tunerCfg, "# Tuner Initial parameter\n");
	fprintf(tunerCfg, "[%s]\n", TITLE_INIT_DATA);
	fprintf(tunerCfg, "# Total number of tuner : %d\n", NUMBER_OF_TUNER);
	fprintf(tunerCfg, "%s=%d\n", ITEM_NUMBER_OF_TUNER, NUMBER_OF_TUNER);
	fprintf(tunerCfg, "\n");
	
	/* Open Param */
	fprintf(tunerCfg, "# Tuner Open parameter\n");
	fprintf(tunerCfg, "[%s]\n", TITLE_OPENDATA);
	fprintf(tunerCfg, "%s=%d\n", ITEM_TUNER_NUMBER, 0);
	fprintf(tunerCfg, "%s=%s\n", ITEM_PLL_REPEATE_MODE, VALUE_ON);
	fprintf(tunerCfg, "%s=%s\n", ITEM_OUTPUT_MODE, VALUE_TUNER_PARALLEL);
	fprintf(tunerCfg, "%s=%s\n", ITEM_OUTPUT_CLK, VALUE_TUNER_CLK_RISING);
	fprintf(tunerCfg, "%s=%s\n", ITEM_TUNER_TYPE, VALUE_TUNER_DVB_S);
	fprintf(tunerCfg, "%s=%s\n", ITEM_DEMODULATOR_DEVICE, VALUE_AVL2108);
	fprintf(tunerCfg, "%s=%s\n", ITEM_PLL_DEVICE, VALUE_IX2564);
	
	/* Demodulator I2C */
	fprintf(tunerCfg, "[%s]\n", TITLE_DEMODULATOR_I2C);
	fprintf(tunerCfg, "%s=%d\n", ITEM_I2C_NUMBER, 0);
	fprintf(tunerCfg, "%s=%02X\n", ITEM_I2C_ADDRESS, 0x18);
	fprintf(tunerCfg, "%s=%d\n", ITEM_I2C_ADDRESS_LEN, 3);
	fprintf(tunerCfg, "%s=%d\n", ITEM_I2C_SPEED, 2);
	
	/* PLL I2C */
	fprintf(tunerCfg, "[%s]\n", TITLE_PLL_I2C);
	fprintf(tunerCfg, "%s=%d\n", ITEM_I2C_NUMBER, 0);
	fprintf(tunerCfg, "%s=%02X\n", ITEM_I2C_ADDRESS, 0xC0);
	fprintf(tunerCfg, "%s=%d\n", ITEM_I2C_ADDRESS_LEN, 0);
	fprintf(tunerCfg, "%s=%d\n", ITEM_I2C_SPEED, 1);
	
	/* Tuner Reset GPIO */
	fprintf(tunerCfg, "[%s]\n", TITLE_TUNER_RESET);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_PORT, 0);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_BIT, 6);
	fprintf(tunerCfg, "%s=%s\n", ITEM_GPIO_MODE, VALUE_GPIO_OUTPUT);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_INIT_VALUE, 0);
	
	/* LNB Power GPIO */
	fprintf(tunerCfg, "[%s]\n", TITLE_LNB_POWER);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_PORT, 1);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_BIT, 51);
	fprintf(tunerCfg, "%s=%s\n", ITEM_GPIO_MODE, VALUE_GPIO_OUTPUT);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_INIT_VALUE, 1);
	
	/* Hor/Ver GPIO */
	fprintf(tunerCfg, "[%s]\n", TITLE_HOR_VER);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_PORT, 1);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_BIT, 49);
	fprintf(tunerCfg, "%s=%s\n", ITEM_GPIO_MODE, VALUE_GPIO_OUTPUT);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_INIT_VALUE, 1);
	
	/* 0/12V GPIO */
	fprintf(tunerCfg, "[%s]\n", TITLE_V12);
	fprintf(tunerCfg, "#%s is not used in this H/W.\n", TITLE_V12);
	fprintf(tunerCfg, "%s=%u\n", ITEM_GPIO_PORT, 255);
	fprintf(tunerCfg, "%s=%u\n", ITEM_GPIO_BIT, 255);
	fprintf(tunerCfg, "%s=%s\n", ITEM_GPIO_MODE, VALUE_GPIO_OUTPUT);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_INIT_VALUE, 1);
	
	/* Skew GPIO */
	fprintf(tunerCfg, "[%s]\n", TITLE_SKEW);
	fprintf(tunerCfg, "#%s is not used in this H/W.\n", TITLE_SKEW);
	fprintf(tunerCfg, "%s=%u\n", ITEM_GPIO_PORT, 255);
	fprintf(tunerCfg, "%s=%u\n", ITEM_GPIO_BIT, 255);
	fprintf(tunerCfg, "%s=%s\n", ITEM_GPIO_MODE, VALUE_GPIO_OUTPUT);
	fprintf(tunerCfg, "%s=%d\n", ITEM_GPIO_INIT_VALUE, 1);

	fclose(tunerCfg);
}

void SetDefaultInitParam(TunerInitParam_t *initParam)
{
	initParam->NumberOfTuner = NUMBER_OF_TUNER;
}

void SetSefaultI2C(I2cOpenParam_t *i2cParam, U8 address, U8 subLen, U8 speed)
{
	i2cParam->DevideNo   = 0;
	i2cParam->Address    = address;
	i2cParam->SubAddrLen = subLen;
	i2cParam->Speed      = speed;
}

void SetSefaultGpio(GpioOpenParam_t *gpioParam, U8 port, U8 bit, U8 mode, U8 value)
{
	gpioParam->PortNumber = port;
	gpioParam->BitNumber  = bit;
	gpioParam->Mode       = mode;
	gpioParam->Value      = value;
}

void SetDefaultOpenParam(TunerOpenParam_t *openParam, U8 tunerNumber)
{
	U8 i2cAddress;
	
	openParam->TunerNumber = tunerNumber;
	openParam->PllI2CRepeate = 1;/* PLL I2C Repeate mode ON */
	openParam->TunerOutputMode = TUNER_OUT_PARALLEL_CI | TUNER_OUT_RISING;/* Output mode Paralle CI mod & Rising */
	openParam->Type = 0;/* Tuner Type DVB-S/S2 */
	openParam->Demodulator = AVL2108; /* Demodulator AVL2108 */
	openParam->TunerPll    = IX2564; /* PLL Device IX2564 */
	i2cAddress = 0x18 + tunerNumber*2;
	// OsDebugPrintf("SetDefaultOpenParam Info : Demodulator I2C Address [0x%02X]\n", i2cAddress);
	SetSefaultI2C(&openParam->DemodI2cParam, i2cAddress, 3, 2);
	i2cAddress = 0xC0 + tunerNumber*2;
	// OsDebugPrintf("SetDefaultOpenParam Info : PLL I2C Address [0x%02X]\n", i2cAddress);
	SetSefaultI2C(&openParam->PllI2cParam, i2cAddress, 0, 1);
	SetSefaultGpio(&openParam->TunerReset, 0,    6, GPIO_WRITE_MODE, 1);
	SetSefaultGpio(&openParam->LnbPower,   1,   51, GPIO_WRITE_MODE, 1);
	SetSefaultGpio(&openParam->HorVer,     1,   49, GPIO_WRITE_MODE, 1);
	SetSefaultGpio(&openParam->V12,     0xFF, 0xFF, GPIO_WRITE_MODE, 1);
	SetSefaultGpio(&openParam->Skew,    0xFF, 0xFF, GPIO_WRITE_MODE, 1);
	openParam->CallBackNotify = TuneResultNotify;
}

U8 SetDefaultTunerParameter(U32 **tunerHandleId_p)
{
	TunerInitParam_t  tunerInitParam;
	TunerOpenParam_t  tunerOpenParam;
	U32              *tunerHandleId;
	int               handleSize;
	U8                count;

	SetDefaultInitParam(&tunerInitParam);
	handleSize = sizeof(U32) * tunerInitParam.NumberOfTuner;
	// OsDebugPrintf("####  SetDefaultTunerParameter Info : Tuner Init for %d Tuner(s)\n", tunerInitParam.NumberOfTuner);
	tunerHandleId = (U32 *)OsMemoryAllocate(handleSize);
	if (tunerHandleId == NULL)
	{
		// OsDebugPrintf("####  SetDefaultTunerParameter Error : Can not allocate Tuner handle ID memory\n");

		return 0;
	}
	TunerInit(tunerInitParam);

	count = 0;
	do
	{
		SetDefaultOpenParam(&tunerOpenParam, count);
		// OsDebugPrintf("####  SetDefaultTunerParameter Info : Tuner Open for Tuner [%d]\n", count);
		TunerOpen(&tunerHandleId[count], &tunerOpenParam);
		count++;
	} while (count < tunerInitParam.NumberOfTuner);
	
	*tunerHandleId_p = tunerHandleId;

	return count;
}

U8 SetTunerParameter(FILE* tunerCfg, U32 **tunerHandleId_p)
{
	TunerInitParam_t  tunerInitParam;
	TunerOpenParam_t  tunerOpenParam;
	ConfigDataType    dataType;
	BOOL              compare;
	int               handleSize;
	U32              *tunerHandleId;
	U32               count;
	U8                numberOftuner;
	U8                tunerInited;
	U8                currentTuner;
	U8                name[ITEM_NAME_LENGTH];
	U8                value[ITEM_VALUE_LENGTH];
	ConfigData tunerConfig[NUMBER_OF_TITLE] =
	{
		{TITLE_INIT_DATA, &tunerInitParam, GetInitParameter},
		{TITLE_OPENDATA, &tunerOpenParam, GetOpenParameter},
		{TITLE_DEMODULATOR_I2C, &tunerOpenParam.DemodI2cParam, GetI2CParameter},
		{TITLE_PLL_I2C, &tunerOpenParam.PllI2cParam, GetI2CParameter},
		{TITLE_TUNER_RESET, &tunerOpenParam.TunerReset, GetGpioParameter},
		{TITLE_LNB_POWER, &tunerOpenParam.LnbPower, GetGpioParameter},
		{TITLE_HOR_VER, &tunerOpenParam.HorVer, GetGpioParameter},
		{TITLE_V12, &tunerOpenParam.V12, GetGpioParameter},
		{TITLE_SKEW, &tunerOpenParam.Skew, GetGpioParameter},
	};

	tunerHandleId = NULL;
	*tunerHandleId_p = tunerHandleId;

	tunerInited   = 0;
	numberOftuner = 0;
	currentTuner  = 0;
	tunerOpenParam.TunerNumber = 0xff;

	do
	{
		memset(name, 0, ITEM_NAME_LENGTH);
		dataType = ProcessOneLine(tunerCfg, name, value);
		while (dataType == DATA_TITLE)
		{
			dataType = DATA_UNKNOWN;
			count = 0;
			do
			{
				compare = CompareName(tunerConfig[count].Name, name);
				if (compare == FALSE)
				{
					if (count == 1)
					{
						/* Tuner open Prameter found */
						if (tunerInited)
						{
							if (currentTuner == tunerOpenParam.TunerNumber)
							{
								/* All open paremeter setted for current tuner and get next open parameter header */
								/* Now time to open for current tuner and after continue to set next tuner        */
								// OsDebugPrintf("####  SetTunerParameter Info : Tuner Open for Tuner [%d]\n", currentTuner);
								TunerOpen(&tunerHandleId[currentTuner], &tunerOpenParam);
								currentTuner++;
								if (currentTuner >= numberOftuner)
								{
									/* All tuner opened */
									*tunerHandleId_p = tunerHandleId;

									return currentTuner;
								}
							}

							/* Set Defalut open parameter */
							SetDefaultOpenParam(&tunerOpenParam, currentTuner);
						}
						else
						{
							/* Tuner Nor initialize and get just open parameter */
							// OsDebugPrintf("####  SetTunerParameter Error : Tuner not Initialized\n");
							return 0;
						}
					}

					// OsDebugPrintf("\t%d : Name[%s] found\n", count, tunerConfig[count].Name);
					memset(name, 0, ITEM_NAME_LENGTH);
					dataType = tunerConfig[count].Function(tunerCfg, tunerConfig[count].ConfigData, name);
					if (count == 0)
					{
						/* Tuner Init Param found and processed */
						/* Tuner Initialize                     */
						if (tunerInited > 0)
						{
							// OsDebugPrintf("####  SetTunerParameter Error : Tuner Initialized before\n");
						}
						else
						{
							numberOftuner = tunerInitParam.NumberOfTuner;
							if (numberOftuner == 0)
							{
								// OsDebugPrintf("####  SetTunerParameter Error : Number of tuner is 0. Use default setting\n");
								return 0;
							}
							tunerOpenParam.TunerNumber = numberOftuner; /* Init NULL tuner for open Parameter tunernumber */
							// OsDebugPrintf("####  SetTunerParameter Info : Tuner Init for %d Tuner(s)\n", numberOftuner);
							TunerInit(tunerInitParam);
							tunerInited = 1;
							handleSize  = sizeof(U32) * numberOftuner;
							tunerHandleId = (U32 *)OsMemoryAllocate(handleSize);
						}
					}

				}

				count++;
			} while(compare && (count < NUMBER_OF_TITLE));
		}
	} while (dataType != DATA_EOF);

	if (currentTuner == tunerOpenParam.TunerNumber)
	{
		/* End of Config file but unopened tunerleft */
		// OsDebugPrintf("####  SetTunerParameter Info : Tuner Open for Tuner [%d] after EOF\n", currentTuner);
		tunerOpenParam.CallBackNotify = TuneResultNotify;
		TunerOpen(&tunerHandleId[currentTuner], &tunerOpenParam);
		currentTuner++;
	}

	*tunerHandleId_p = tunerHandleId;
	return currentTuner;
}

void GetDefaultSearchParam(TunerSearchParam_t *searchParam)
{
	searchParam->TunerNumber         = 0;
	searchParam->SearchMode          = 1;
	searchParam->TpNumber            = 0;
	searchParam->TpFrequency         = 11919;
	searchParam->Symbolrate          = 24444;
	searchParam->SignalType          = STREAM_AUTO;
	searchParam->RollOff             = ROLL_OFF_35;
	searchParam->FecCode             = DVB_FEC_AUTO;
	searchParam->Power               = 1;
	searchParam->HorVer              = 0;
	searchParam->On22khz             = 0;
	searchParam->DiseqcPort          = 1; /* Port A */
	searchParam->ToneBurst           = TONEBURST_NONE;
	searchParam->Sw12V               = 0;
	searchParam->SkewValue           = 0xFF;
	searchParam->LnbType             = LNB_UNIVERSAL;
	searchParam->LnbLocalHi          = 10600;
	searchParam->LnbLocalLow         = 9750;
	searchParam->ConnectionMode      = TUNER_CONECTION_NONE;
	searchParam->MotorPosition       = 0;
	searchParam->MotorAngle          = 0;
	searchParam->UnicableSatPosition = 0;
	searchParam->UniCableUBand       = 0;
	searchParam->UniCableBandFreq    = 0;
#ifdef USA_SUPPORT /* USA Support Version */
	searchParam->LagacySwitch        = 0;
#endif /* #ifdef USA_SUPPORT */
}

U32 GetValueFromConsole(U32 value, U32 min, U32 max)
{
	U8  count;
	U8  inBuffer[MAX_UNSIGNED_DIGIT + 1];
	int inChar;
	U32 convertVal;

	count = 0;
	memset(inBuffer, 0x00, (MAX_UNSIGNED_DIGIT + 1));
	do
	{
		inChar = getchar();
		if (inChar == ASCII_q)
		{
			inChar = ASCII_Q;
		}
		else if ((inChar == ASCII_LF) || (inChar == ASCII_CR))
		{
			inChar = ASCII_LF;
		}
		else
		{
			inBuffer[count] = inChar;
			count++;
		}
	} while((count < MAX_UNSIGNED_DIGIT) && (inChar != ASCII_LF));

	if ((inBuffer[0] == ASCII_LF) || (inBuffer[0] == ASCII_Q))
	{
		/* Default Value or Quit*/
		return value;
	}

	convertVal = char2val(inBuffer);
	if ((convertVal < min) || (max < convertVal))
	{
		/* Out of Range : use default value */
		return value;
	}

	return convertVal;
}

void TuneOperation(U32 tunerHandleId)
{
	TunerSearchParam_t searchParam;
	U8                 waitInput;
	int                key;

	waitInput = 1;
	GetDefaultSearchParam(&searchParam);
	
	do
	{
		OsDebugPrintf("/**************************************************/\n");
		OsDebugPrintf("\tTuner Search Parameter Setting Start\n");
		OsDebugPrintf("\tInput TP Frequency [%d MHz] : ", searchParam.TpFrequency);
		searchParam.TpFrequency = (U16)GetValueFromConsole(searchParam.TpFrequency, 0, 15000);
		OsDebugPrintf("\tInput Symbol Rate [%d Ksps] : ", searchParam.Symbolrate);
		searchParam.Symbolrate = (U16)GetValueFromConsole(searchParam.Symbolrate, 0, 65000);
		OsDebugPrintf("\tSelect LNB Type(Universal : 0, Signle : 1) [%d]", searchParam.LnbType);
		searchParam.LnbType = (U8)GetValueFromConsole(searchParam.LnbType, 0, 1);
		if (searchParam.LnbType == LNB_UNIVERSAL)
		{
			OsDebugPrintf("\tSelect LNB Local High Frequency [%d MHz]", searchParam.LnbLocalHi);
			searchParam.LnbLocalHi = (U16)GetValueFromConsole(searchParam.LnbLocalHi, 0, 15000);
			OsDebugPrintf("\tSelect LNB Local Low Frequency  [%d MHz]", searchParam.LnbLocalLow);
			searchParam.LnbLocalLow = (U16)GetValueFromConsole(searchParam.LnbLocalLow, 0, 15000);
			searchParam.On22khz = 0;
		}
		else
		{
			OsDebugPrintf("\tSelect LNB Local Frequency [%d MHz]", searchParam.LnbLocalHi);
			searchParam.LnbLocalHi = (U16)GetValueFromConsole(searchParam.LnbLocalHi, 0, 15000);
			OsDebugPrintf("\tSelect 22KHz On (ON : 1, OFF : 0)  [%d]", searchParam.On22khz);
			searchParam.On22khz = (U16)GetValueFromConsole(searchParam.On22khz, 0, 15000);
			searchParam.LnbLocalLow = searchParam.LnbLocalHi;
		}
		OsDebugPrintf("\tSelect LNB Power On/Off (ON : 1, Off : 0) [%d]", searchParam.Power);
		searchParam.Power = (U8)GetValueFromConsole(searchParam.Power, 0, 1);
		OsDebugPrintf("\tSelect Hor/Ver(Hor : 1, Ver : 0) [%d]", searchParam.HorVer);
		searchParam.HorVer = (U8)GetValueFromConsole(searchParam.HorVer, 0, 1);
		OsDebugPrintf("\tSelect DiSEqC(NONE : 0, A : 1, B : 2, C : 3, D : 4) [%d]", searchParam.DiseqcPort);
		searchParam.DiseqcPort = (U8)GetValueFromConsole(searchParam.DiseqcPort, 0, 4);
		TunerSearchStart(tunerHandleId, &searchParam);
		OsSemaphoreWait(TuneNotifySem, TIMEOUT_FOREVER); /* Wait result */
		OsDebugPrintf("\tDo you Want to continue? (Y/N)");
		key = getchar();
		if ((key == ASCII_n) || (key == ASCII_N))
		{
			waitInput = 0;
		}
		key = getchar();
	} while (waitInput);
}

int TunerInitial(void)
{
	FILE*		      tunerCfg;
	U8                tunerNumber;

	OsDebugPrintf("/*****************************************************************/\n");
	OsDebugPrintf("\tTuner Driver Init By KB Kim\n");
	OsDebugPrintf("\tCopyright (C) 2010 OTE\n");
	OsDebugPrintf("/*****************************************************************/\n");

	OsCreateSemaphore(&TuneNotifySem, 0);
	tunerCfg = fopen(CONFIG_FILE, "r");
	if (tunerCfg == NULL)
	{
		WriteDefaultConfigData();
		tunerCfg = fopen(CONFIG_FILE, "r");
		if (tunerCfg == NULL)
		{
			OsDebugPrintf("####  main Error : Can't open configulation file\n");
			tunerNumber = SetDefaultTunerParameter(&Tuner_HandleId);

			return (-1);
		}
	}

	tunerNumber = SetTunerParameter(tunerCfg, &Tuner_HandleId);
	if (tunerNumber == 0)
	{
		/* Error to get Tuner parameter */
		/* Use default setting          */
		tunerNumber = SetDefaultTunerParameter(&Tuner_HandleId);
		if (tunerNumber == 0)
		{
			OsDebugPrintf("####  main Error : Can't Init and open tuner\n");
		}
	}
	fclose(tunerCfg);

#if 0
	/* Tune */
	TuneOperation(tunerHandleId[0]);

	if (tunerNumber > 0)
	{
		for (counter = 0; counter < tunerNumber; counter++)
		{
			if (TunerClose(tunerHandleId[counter]) == TUNER_NO_ERROR)
			{
				OsDebugPrintf("####  main info : Tuner %d is Closed\n", counter);
			}
			else
			{
				OsDebugPrintf("####  main Error : Can't Close Tuner %d\n", counter);
			}
		}
	}

	TunerTerm();

	if (TuneNotifySem != 0)
	{
		OsDeleteSemaphore(TuneNotifySem);
	}
#endif
	return 0;
}
