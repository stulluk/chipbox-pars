/*
 * avl2108.c - AVL2108 DVB-S/S2 demodulator driver (port of Merih Avl2108.o).
 *
 * Written from the disassembly of the OABI object in libsystem_merih.a so that mvapp can be
 * built as an EABI binary. Register addresses, sequences, retry counts and delays follow the
 * original. The demodulator is a 24-bit addressed, big-endian I2C device; the IX2564 PLL
 * sits behind its I2C repeater.
 *
 * Driver error codes (as in the original): 0 ok, 1 no handle / not initialised,
 * 3 bad parameter, 4 wrong functional mode, 6 busy / not ready.
 */
#include <string.h>

#include "merih_tuner_int.h"
#include "mvutil.h"

#define AVL_OK                 0
#define AVL_ERR_HANDLE         1
#define AVL_ERR_PARAM          3
#define AVL_ERR_MODE           4
#define AVL_ERR_BUSY           6

#define AVL_PATCH_DEMOD        0
#define AVL_PATCH_BLIND        1
#define AVL_NOT_INITIALIZED    2

#define AVL_PLCONF_ENTRIES     10
#define AVL_LEVEL_ENTRIES      47

/* One PlConf entry: PLL setup, reference key and DSP clock values. */
typedef struct
{
	U16 Pll80;      /* -> 0x6c0080 */
	U16 PllC0;      /* -> 0x6c00c0 */
	U16 Pll100;     /* -> 0x6c0100 */
	U16 Pll140;     /* -> 0x6c0140 */
	U16 Pll180;     /* -> 0x6c0180 */
	U16 RefKey;     /* reference clock key (kHz) */
	U16 RefHz;      /* -> 0x580, DiSEqC reference */
	U16 Clk2;       /* -> 0x582 */
	U16 Clk3;       /* -> 0x584 */
	U16 Pad;
} Avl2108PlConf_t;

/* Signal level table: first entry whose level >= measured value gives the percent. */
typedef struct
{
	U16 Level;
	U8  Percent;
	U8  Pad;
} Avl2108Level_t;

/* Blind scan request block (written to 0x5d0..0x5e2). */
typedef struct
{
	U32 CenterFreq;   /* 100 kHz units */
	U32 StartFreq;    /* 100 kHz units */
	U32 StopFreq;     /* 100 kHz units */
	U32 MinSymbol;
} Avl2108BlindParam_t;

static const Avl2108PlConf_t PlConf[AVL_PLCONF_ENTRIES] =
{
	{ 10, 0, 335, 6, 5,  4000, 11200, 16800, 19200, 0 },
	{ 10, 0, 299, 6, 5,  4500, 11250, 16875, 19286, 0 },
	{ 10, 1, 269, 6, 5, 10000, 11250, 16875, 19286, 0 },
	{ 10, 0,  83, 6, 5, 16000, 11200, 16800, 19200, 0 },
	{ 10, 0,  49, 6, 5, 27000, 11250, 16875, 19286, 0 },
	{ 10, 0, 335, 6, 4,  4000, 11200, 16800, 22400, 0 },
	{ 10, 0, 299, 6, 4,  4500, 11250, 16875, 22500, 0 },
	{ 10, 1, 269, 6, 4, 10000, 11250, 16875, 22500, 0 },
	{ 10, 0,  83, 6, 4, 16000, 11200, 16800, 22400, 0 },
	{ 10, 0,  49, 6, 4, 27000, 11250, 16875, 22500, 0 },
};

static const Avl2108Level_t SignalLevelTable[AVL_LEVEL_ENTRIES] =
{
	{  8285,   8, 0 }, { 10224,  10, 0 }, { 12538,  12, 0 }, { 14890,  14, 0 },
	{ 17343,  16, 0 }, { 19767,  18, 0 }, { 22178,  20, 0 }, { 24618,  22, 0 },
	{ 27006,  24, 0 }, { 29106,  26, 0 }, { 30853,  28, 0 }, { 32289,  30, 0 },
	{ 33577,  32, 0 }, { 34625,  34, 0 }, { 35632,  36, 0 }, { 36552,  38, 0 },
	{ 37467,  40, 0 }, { 38520,  42, 0 }, { 39643,  44, 0 }, { 40972,  46, 0 },
	{ 42351,  48, 0 }, { 43659,  50, 0 }, { 44812,  52, 0 }, { 45811,  54, 0 },
	{ 46703,  56, 0 }, { 47501,  58, 0 }, { 48331,  60, 0 }, { 49116,  62, 0 },
	{ 49894,  64, 0 }, { 50684,  66, 0 }, { 51543,  68, 0 }, { 52442,  70, 0 },
	{ 53407,  72, 0 }, { 54314,  74, 0 }, { 55208,  76, 0 }, { 56000,  78, 0 },
	{ 56789,  80, 0 }, { 57544,  82, 0 }, { 58253,  84, 0 }, { 58959,  86, 0 },
	{ 59657,  88, 0 }, { 60404,  90, 0 }, { 61181,  92, 0 }, { 62008,  94, 0 },
	{ 63032,  96, 0 }, { 65483,  98, 0 }, { 65535, 100, 0 },
};

/* State shared by all functions (single demodulator, as in the original). */
static U32 Avl2108Initialized = AVL_NOT_INITIALIZED;
static U32 Avl2108DiseqcOutputStatus;
static struct
{
	U32 PatchType;
	U16 RefKey;
	U16 RefHz;
	U16 Clk2;
	U16 Clk3;
} Avl2108CurrentData;

/* Read len bytes from a demodulator register (odd length: last byte via a 2-byte read). */
static int Avl2108I2cRead(TunerObject_t *obj, U32 addr, U8 *data, U32 len)
{
	U8  tmp[2];
	U32 even;
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	if ((len == 0) || (data == NULL))
		return AVL_ERR_PARAM;

	even = len & ~1u;
	tunerAccessLock(obj);
	err = (even > 0) ? tunerGetDemodResters(obj, addr, data, even) : AVL_OK;
	if ((err == AVL_OK) && (len & 1))
	{
		err = tunerGetDemodResters(obj, addr + even, tmp, 2);
		if (err == AVL_OK)
			data[even] = tmp[0];
	}
	tunerAccessRelease(obj);
	return err;
}

/* Write len bytes to a demodulator register (odd length: read-modify-write of last word). */
static int Avl2108I2cWrite(TunerObject_t *obj, U32 addr, const U8 *data, U32 len)
{
	U8  tmp[2];
	U32 even;
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	if ((len == 0) || (data == NULL))
		return AVL_ERR_PARAM;

	even = len & ~1u;
	tunerAccessLock(obj);
	err = (even > 0) ? tunerSetDemodResters(obj, addr, (U8 *)data, even) : AVL_OK;
	if ((err == AVL_OK) && (len & 1))
	{
		err = tunerGetDemodResters(obj, addr + even, tmp, 2);
		if (err == AVL_OK)
		{
			tmp[0] = data[even];
			err = tunerSetDemodResters(obj, addr + even, tmp, 2);
		}
	}
	tunerAccessRelease(obj);
	return err;
}

/* Read a big-endian 16-bit register. */
static int Avl2108I2cRead16(TunerObject_t *obj, U32 addr, U16 *value)
{
	U8  b[2];
	int err;

	*value = 0;
	if (obj == NULL)
		return AVL_ERR_HANDLE;
	err = Avl2108I2cRead(obj, addr, b, 2);
	if (err == AVL_OK)
		*value = (U16)Array2Word(b, 2);
	return err;
}

/* Read a big-endian 32-bit register. */
static int Avl2108I2cRead32(TunerObject_t *obj, U32 addr, U32 *value)
{
	U8  b[4];
	int err;

	*value = 0;
	if (obj == NULL)
		return AVL_ERR_HANDLE;
	err = Avl2108I2cRead(obj, addr, b, 4);
	if (err == AVL_OK)
		*value = Array2Word(b, 4);
	return err;
}

/* Write a big-endian 16-bit register. */
static int Avl2108I2cWrite16(TunerObject_t *obj, U32 addr, U16 value)
{
	U8 b[2];

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	Word2Array(b, value, 2);
	return Avl2108I2cWrite(obj, addr, b, 2);
}

/* Write a big-endian 32-bit register. */
static int Avl2108I2cWrite32(TunerObject_t *obj, U32 addr, U32 value)
{
	U8 b[4];

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	Word2Array(b, value, 4);
	return Avl2108I2cWrite(obj, addr, b, 4);
}

/* I2C repeater mailbox status (0x416): busy while byte 1 != 0. Caller holds the lock. */
static int Avl2108I2cRepeaterGetOPStatus(TunerObject_t *obj)
{
	U8  st[2];
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	err = tunerGetDemodResters(obj, 0x416, st, 2);
	if (err != AVL_OK)
		return err;
	return st[1] ? AVL_ERR_BUSY : AVL_OK;
}

/* Post a repeater operation: wait until idle, then write it just below 0x418. */
static int Avl2108I2cRepeaterSendOp(TunerObject_t *obj, U8 *op, U32 size)
{
	U8  count = 0;
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	if (size > 20)
		return AVL_ERR_PARAM;

	do
	{
		OsWaitMillisecond(100);
		err = Avl2108I2cRepeaterGetOPStatus(obj);
		count++;
	} while ((err != AVL_OK) && (count <= 19));
	if (err != AVL_OK)
		return err;

	return tunerSetDemodResters(obj, 0x418 - size, op, size);
}

/* Configure the I2C repeater (0x5cc) and start it. */
static int Avl2108I2cRepeateInit(TunerObject_t *obj)
{
	U8  op[2] = { 1, 1 };
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	err = Avl2108I2cWrite16(obj, 0x5cc, 200);
	if (err != AVL_OK)
		return err;

	tunerAccessLock(obj);
	err = Avl2108I2cRepeaterSendOp(obj, op, 2);
	tunerAccessRelease(obj);
	return err;
}

/* Read from the PLL through the repeater (op 3), result appears at 0x418. */
static TunerError Avl2108I2cRepeateRead(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len)
{
	U8  op[6];
	U8  buf[20];
	U32 size;
	U8  count;
	int err;

	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return (TunerError)AVL_ERR_HANDLE;
	if (len > 20)
		return (TunerError)AVL_ERR_PARAM;

	if (obj->PllSubAddrLen == 1)
	{
		op[0] = 0;
		op[1] = (U8)subAddr;
		op[2] = 1;
		op[3] = (U8)len;
		op[4] = obj->PllAddress;
		op[5] = 3;
		size  = 6;
	}
	else if (obj->PllSubAddrLen == 0)
	{
		op[0] = 0;
		op[1] = (U8)len;
		op[2] = obj->PllAddress;
		op[3] = 3;
		size  = 4;
	}
	else
	{
		return (TunerError)AVL_ERR_PARAM;
	}

	tunerAccessLock(obj);
	err = Avl2108I2cRepeaterSendOp(obj, op, size);
	if (err == AVL_OK)
	{
		err = Avl2108I2cRepeaterGetOPStatus(obj);
		count = 0;
		while ((err != AVL_OK) && (count <= 99))
		{
			OsWaitMillisecond(100);
			err = Avl2108I2cRepeaterGetOPStatus(obj);
			count++;
		}
		if (err == AVL_OK)
		{
			err = tunerGetDemodResters(obj, 0x418, buf, (len & 1) ? len + 1 : len);
			memcpy(data, buf, len);
		}
	}
	tunerAccessRelease(obj);
	return (TunerError)err;
}

/* Write to the PLL through the repeater (op 2). The PLL has no sub address. */
static TunerError Avl2108I2cRepeateWrite(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len)
{
	U8  op[20];
	U32 size;
	U32 offset;
	int err;

	(void)subAddr;
	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return (TunerError)AVL_ERR_HANDLE;
	size = len + 3;
	if (20 - size < len)
		return (TunerError)AVL_ERR_PARAM;

	memset(op, 0, sizeof(op));
	/* The mailbox is word aligned: an odd packet gets a leading pad byte. */
	offset = size & 1;
	if (offset)
		size = len + 4;
	memcpy(&op[offset], data, len);
	op[offset + len]     = (U8)len;
	op[offset + len + 1] = obj->PllAddress;
	op[offset + len + 2] = 2;

	tunerAccessLock(obj);
	err = Avl2108I2cRepeaterSendOp(obj, op, size);
	tunerAccessRelease(obj);
	return (TunerError)err;
}

/* Whether the DiSEqC block may be reprogrammed (previous transmission finished). */
static int Avl2108IDiseqcIsSafeToSwitchMode(TunerObject_t *obj)
{
	U32 value;
	int err;

	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return AVL_ERR_HANDLE;

	switch (Avl2108DiseqcOutputStatus)
	{
		case 1:
		case 2:
			return AVL_OK;
		case 3:
		case 4:
			err = Avl2108I2cRead32(obj, 0x70000c, &value);
			if (err != AVL_OK)
				return err;
			return (value & 0x40) ? AVL_OK : AVL_ERR_BUSY;
		default:
			return AVL_ERR_PARAM;
	}
}

/* DiSEqC transmit status: status[0] = finished bit, status[1] = FIFO count. */
static int Avl2108IDiseqcGetTxStatus(TunerObject_t *obj, U8 *status)
{
	U32 value;
	int err;

	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return AVL_ERR_HANDLE;
	err = Avl2108I2cRead32(obj, 0x70000c, &value);
	if (err != AVL_OK)
	{
		status[1] = 15;
		status[0] = 0;
		return err;
	}
	status[1] = (value >> 2) & 15;
	status[0] = (value >> 6) & 1;
	return AVL_OK;
}

/* Wait (up to ~150 ms) for the DiSEqC transmission to finish. */
static int Avl2108IDiseqcCheckTxFinished(TunerObject_t *obj)
{
	U8  status[2];
	U8  count = 0;
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	err = Avl2108IDiseqcGetTxStatus(obj, status);
	if (err != AVL_OK)
		return err;
	while (status[0] != 1)
	{
		OsWaitMillisecond(10);
		err = Avl2108IDiseqcGetTxStatus(obj, status);
		count++;
		if (err != AVL_OK)
			return err;
		if (count > 14)
			break;
	}
	return AVL_OK;
}

/* Switch the continuous 22 kHz tone on or off. */
static TunerError Avl2108Set22K(TunerObject_t *obj, U8 on)
{
	U32 value;
	int err;

	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return (TunerError)AVL_ERR_HANDLE;
	err = Avl2108IDiseqcIsSafeToSwitchMode(obj);
	if (err != AVL_OK)
		return (TunerError)err;
	err = Avl2108I2cRead32(obj, 0x700000, &value);
	if (err != AVL_OK)
		return (TunerError)err;

	if (on)
	{
		value = (value & ~7u) | 3u;
		err = Avl2108I2cWrite32(obj, 0x700000, value);
		if (err != AVL_OK)
			return (TunerError)err;
		value |= 0x400;
		err = Avl2108I2cWrite32(obj, 0x700000, value);
		if (err != AVL_OK)
			return (TunerError)err;
		Avl2108DiseqcOutputStatus = 2;
	}
	else
	{
		value &= ~0xc00u;
		err = Avl2108I2cWrite32(obj, 0x700000, value);
		if (err != AVL_OK)
			return (TunerError)err;
		Avl2108DiseqcOutputStatus = 1;
	}
	return (TunerError)AVL_OK;
}

/* Send a tone burst (1 = SatA, 2 = SatB). */
static TunerError Avl2108SetToneBurst(TunerObject_t *obj, U8 toneBurst)
{
	U8  fifo[4] = { 0, 0, 0, 1 };
	U32 value;
	U8  count;
	int err;

	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return (TunerError)AVL_ERR_HANDLE;
	err = Avl2108IDiseqcIsSafeToSwitchMode(obj);
	if (err != AVL_OK)
		return (TunerError)err;
	err = Avl2108I2cRead32(obj, 0x700000, &value);
	if (err != AVL_OK)
		return (TunerError)err;

	value &= ~7u;
	if (toneBurst == 1)
		value |= 1;
	else if (toneBurst == 2)
		value |= 2;
	err = Avl2108I2cWrite32(obj, 0x700000, value);
	if (err != AVL_OK)
		return (TunerError)err;

	for (count = 0; count < 8; count++)
	{
		err = Avl2108I2cWrite(obj, 0x700080, fifo, 4);
		if (err != AVL_OK)
			return (TunerError)err;
	}

	value |= 4;
	err = Avl2108I2cWrite32(obj, 0x700000, value);
	if (err != AVL_OK)
		return (TunerError)err;
	Avl2108DiseqcOutputStatus = 3;
	return (TunerError)Avl2108IDiseqcCheckTxFinished(obj);
}

/* Send a DiSEqC message of up to 8 bytes. */
static TunerError Avl2108SendDiseqcData(TunerObject_t *obj, U8 *data, U32 len)
{
	U8  fifo[4] = { 0, 0, 0, 0 };
	U32 value;
	U32 count;
	int err;

	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return (TunerError)AVL_ERR_HANDLE;
	if (len > 8)
		return (TunerError)AVL_ERR_PARAM;
	err = Avl2108IDiseqcIsSafeToSwitchMode(obj);
	if (err != AVL_OK)
		return (TunerError)err;

	/* Flush the transmit FIFO. */
	err = Avl2108I2cRead32(obj, 0x70001c, &value);
	if (err != AVL_OK)
		return (TunerError)err;
	value |= 1;
	Avl2108I2cWrite32(obj, 0x70001c, value);
	value &= ~1u;
	Avl2108I2cWrite32(obj, 0x70001c, value);

	err = Avl2108I2cRead32(obj, 0x700000, &value);
	if (err != AVL_OK)
		return (TunerError)err;
	value &= ~7u;
	err = Avl2108I2cWrite32(obj, 0x700000, value);
	if (err != AVL_OK)
		return (TunerError)err;

	for (count = 0; count < len; count++)
	{
		fifo[3] = data[count];
		err = Avl2108I2cWrite(obj, 0x700080, fifo, 4);
		if (err != AVL_OK)
			return (TunerError)err;
	}

	value |= 4;
	err = Avl2108I2cWrite32(obj, 0x700000, value);
	if (err != AVL_OK)
		return (TunerError)err;
	Avl2108DiseqcOutputStatus = 4;
	return (TunerError)Avl2108IDiseqcCheckTxFinished(obj);
}

/* DiSEqC block setup (reference clock from the selected PlConf entry). */
static int Avl2108IDiseqcInitialize(TunerObject_t *obj)
{
	U32 value;
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	if ((err = Avl2108I2cWrite32(obj, 0x700020, 1)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x700028, 200)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x70002c, Avl2108CurrentData.RefHz)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x700004, 44)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x700008, Avl2108CurrentData.RefHz * 10)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cRead32(obj, 0x700000, &value)) != AVL_OK)
		return err;
	value = (value & 0x300) | 0x28;
	if ((err = Avl2108I2cWrite32(obj, 0x700000, value)) != AVL_OK)
		return err;
	value &= ~0x20u;
	if ((err = Avl2108I2cWrite32(obj, 0x700000, value)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x70001c, 10)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x700014, 0)) != AVL_OK)
		return err;
	return Avl2108I2cWrite32(obj, 0x700020, 0);
}

/* Halt the DSP and download a patch (0 = demod, 1 = blind scan), then restart it. */
static int Avl2108IBaseDownloadFirmware(TunerObject_t *obj, U32 patchType)
{
	const U8 *patch;
	U32       total;
	U32       offset;
	U32       len;
	U32       addr;
	int       err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	if (patchType == AVL_PATCH_DEMOD)
		patch = DemodPatchData;
	else if (patchType == AVL_PATCH_BLIND)
		patch = BlindScanPatchData;
	else
		return AVL_ERR_PARAM;

	Avl2108I2cWrite32(obj, 0x600000, 0);
	total = Array2Word((U8 *)patch, 4) + 4;
	if (total <= 11)
		return AVL_ERR_PARAM;

	offset = 4;
	while (offset < total)
	{
		len     = Array2Word((U8 *)&patch[offset], 4);
		offset += 4;
		addr    = Array2Word((U8 *)&patch[offset], 4) & 0x00ffffff;
		offset += 4;
		err     = Avl2108I2cWrite(obj, addr, &patch[offset], len);
		offset += len;
		if (err != AVL_OK)
			return err;
	}

	if ((err = Avl2108I2cWrite32(obj, 0x3ffc, 0)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x434, 0)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x42c, 0)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x430, 0)) != AVL_OK)
		return err;
	return Avl2108I2cWrite32(obj, 0x600000, 1);
}

/* Functional mode register 0x2476 bit 0: 0 = demodulator, 1 = blind scan. */
static int Avl2108IBaseGetFunctionalMode(TunerObject_t *obj, U32 *mode)
{
	U16 value;
	int err;

	*mode = 2;
	if (obj == NULL)
		return AVL_ERR_HANDLE;
	err = Avl2108I2cRead16(obj, 0x2476, &value);
	if (err == AVL_OK)
		*mode = value & 1;
	return err;
}

/* Receiver mailbox (0x400) status: busy while byte 1 != 0. */
static int Avl2108IBaseGetRxOPStatus(TunerObject_t *obj)
{
	U8  st[2];
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	err = Avl2108I2cRead(obj, 0x400, st, 2);
	if (err != AVL_OK)
		return err;
	return st[1] ? AVL_ERR_BUSY : AVL_OK;
}

/* Poll the receiver mailbox every 10 ms, up to maxCount times. */
static int Avl2108IBaseCheckRxOPStatus(TunerObject_t *obj, U8 maxCount)
{
	U8  count = 0;
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	do
	{
		OsWaitMillisecond(10);
		err = Avl2108IBaseGetRxOPStatus(obj);
		count++;
		if (err != AVL_ERR_BUSY)
			return err;
	} while (count < maxCount);
	return err;
}

/* Post a receiver operation (2 = lock, 3 = reset error stat, 4 = halt, 8 = blind scan). */
static int Avl2108IBaseSendRxOP(TunerObject_t *obj, U8 op)
{
	U8  buf[2];
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	err = Avl2108IBaseGetRxOPStatus(obj);
	if (err != AVL_OK)
		return err;
	buf[0] = 0;
	buf[1] = op;
	return Avl2108I2cWrite(obj, 0x400, buf, 2);
}

/* Stop the receiver DSP before retuning. */
static int Avl2108IBaseHaltCpu(TunerObject_t *obj)
{
	int err;

	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return AVL_ERR_HANDLE;
	err = Avl2108IBaseSendRxOP(obj, 4);
	if (err != AVL_OK)
		return err;
	return Avl2108IBaseCheckRxOPStatus(obj, 20);
}

/* DSP running and patch signature 0xa55a present? */
static int Avl2108IBaseGetStatus(TunerObject_t *obj)
{
	U32 cpu;
	U16 magic;
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	if ((err = Avl2108I2cRead32(obj, 0x600000, &cpu)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cRead16(obj, 0x434, &magic)) != AVL_OK)
		return err;
	if ((cpu != 0) && (magic == 0xa55a))
		return AVL_OK;
	return AVL_ERR_BUSY;
}

/* Wait for the DSP after a patch download (gives up silently after ~400 ms). */
static int Avl2108IBaseCheckStatus(TunerObject_t *obj)
{
	U8  count;
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	OsWaitMillisecond(100);
	for (count = 0; count <= 29; count++)
	{
		err = Avl2108IBaseGetStatus(obj);
		if (err != AVL_ERR_BUSY)
			return err;
		OsWaitMillisecond(10);
	}
	return AVL_OK;
}

/* Program the demodulator PLL from a PlConf entry and pulse its reset. */
static int Avl2108IBaseSetPll(TunerObject_t *obj, const Avl2108PlConf_t *conf)
{
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	if ((err = Avl2108I2cWrite32(obj, 0x6c0100, conf->Pll100)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x6c00c0, conf->PllC0)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x6c0080, conf->Pll80)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x6c0140, conf->Pll140)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x6c0180, conf->Pll180)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x6c0200, 1)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x6c0000, 0)) != AVL_OK)
		return err;
	Avl2108I2cWrite32(obj, 0x6c0000, 1);
	return AVL_OK;
}

/* PLL setup + patch download for the given reference clock key. */
static int Avl2108IBaseInitialize(TunerObject_t *obj, U32 refKey, U32 patchType)
{
	const Avl2108PlConf_t *conf = NULL;
	U8                     index;
	int                    err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	for (index = 0; index < AVL_PLCONF_ENTRIES; index++)
	{
		if (PlConf[index].RefKey == refKey)
		{
			conf = &PlConf[index];
			break;
		}
	}
	if (conf == NULL)
		return AVL_ERR_PARAM;

	err = Avl2108IBaseSetPll(obj, conf);
	if (err != AVL_OK)
		return err;
	OsWaitMillisecond(1);
	err = Avl2108IBaseDownloadFirmware(obj, patchType);
	if (err != AVL_OK)
		return err;

	Avl2108CurrentData.Clk3      = conf->Clk3;
	Avl2108CurrentData.RefKey    = (U16)refKey;
	Avl2108CurrentData.PatchType = patchType;
	Avl2108CurrentData.RefHz     = conf->RefHz;
	Avl2108CurrentData.Clk2      = conf->Clk2;
	return AVL_OK;
}

/* TS output: bits 0-1 parallel/serial mode, bit 2 rising clock edge. */
static int Avl2108IDVBSxRxSetMpegOutput(TunerObject_t *obj, U8 mode)
{
	U32 parallel;
	U16 serialCont;
	U16 rising;
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	switch (mode & 3)
	{
		case 1:  parallel = 0; serialCont = 0; break;
		case 2:  parallel = 1; serialCont = 1; break;
		case 3:  parallel = 0; serialCont = 1; break;
		default: parallel = 1; serialCont = 0; break;
	}
	rising = (mode & 4) ? 1 : 0;

	if ((err = Avl2108I2cWrite32(obj, 0x45c, parallel)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x4de, serialCont)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x4dc, rising)) != AVL_OK)
		return err;
	return Avl2108I2cWrite32(obj, 0x6c0024, 2);
}

/* RF AGC polarity (the Chipbox needs 1). */
static int Avl2108IDVBSxRxSetRFAGCPola(TunerObject_t *obj, U32 polarity)
{
	if (obj == NULL)
		return AVL_ERR_HANDLE;
	return Avl2108I2cWrite32(obj, 0x43c, polarity);
}

/* Carrier frequency sweep range in 100 kHz units, max 500. */
static int Avl2108IDVBSxRxSetFreqSweepRange(TunerObject_t *obj, U16 range)
{
	if (obj == NULL)
		return AVL_ERR_HANDLE;
	if (range >= 500)
		range = 500;
	return Avl2108I2cWrite16(obj, 0x58a, range);
}

/* StreamType -> AVL2108 standard code (20 = auto detect). */
static U16 Avl2108GetStandard(StreamType type)
{
	switch (type)
	{
		case STREAM_DVBS2:       return 1;
		case STREAM_DVBS1:       return 0;
		case STREAM_DSS:         return 2;
		case STREAM_S_TURBOCODE: return 3;
		default:                 return 20;
	}
}

/* Start a lock attempt: mode 1 = sweep, 2 = fixed symbol rate. */
static int Avl2108IDVBSxRxLockChannel(TunerObject_t *obj, TunerTuneParam_t *tune, U8 mode)
{
	U32 functional;
	U16 standard;
	int err;

	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return AVL_ERR_HANDLE;
	if (tune->Symbolrate - 800 > 0xc030)
		return AVL_ERR_PARAM;
	err = Avl2108IBaseGetFunctionalMode(obj, &functional);
	if (err != AVL_OK)
		return err;
	if (functional == 1)
		return AVL_ERR_PARAM;

	if (mode == 1)
	{
		err = Avl2108I2cWrite16(obj, 0x2478, 1);
		if (err == AVL_OK)
			err = Avl2108IDVBSxRxSetFreqSweepRange(obj, (tune->Symbolrate <= 2999) ? 300 : 500);
	}
	else if (mode == 2)
	{
		err = Avl2108I2cWrite16(obj, 0x2478, 1);
	}
	else
	{
		return AVL_ERR_PARAM;
	}
	if (err == AVL_OK)
		err = Avl2108I2cWrite32(obj, 0x470, 0);

	standard = Avl2108GetStandard(tune->DvbType);
	if (err != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x248a, standard)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x2488, 1)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite32(obj, 0x490, tune->Symbolrate * 1000)) != AVL_OK)
		return err;
	return Avl2108IBaseSendRxOP(obj, 2);
}

/* Lock register 0x790 (1 = locked). */
static int AvL2108IDVBSxRxGetLockStatus(TunerObject_t *obj, U16 *lock)
{
	if (obj == NULL)
	{
		*lock = 0;
		return AVL_ERR_HANDLE;
	}
	return Avl2108I2cRead16(obj, 0x790, lock);
}

/* Poll for lock every 10 ms; the number of polls depends on the symbol rate. */
static U8 AvL2108IDVBSxRxCheckLockStatus(TunerObject_t *obj, U32 symbolRate)
{
	U32 maxCount;
	U32 count = 0;
	U16 lock;

	if (obj == NULL)
		return 0;
	if (symbolRate <= 4999)
		maxCount = 200;
	else if (symbolRate <= 9999)
		maxCount = 100;
	else
		maxCount = 50;

	for (;;)
	{
		if (AvL2108IDVBSxRxGetLockStatus(obj, &lock) != AVL_OK)
			return 0;
		count++;
		if (lock == 1)
			return 1;
		OsWaitMillisecond(10);
		if (count >= maxCount)
			return 0;
		if (TunerGetSearchStop((U32)obj) == 0)
			return 0;
	}
}

/* Reset BER / error counters after lock. */
static int Avl2108IDVBSxRxResetErrorStat(TunerObject_t *obj)
{
	U32 functional;
	int err;

	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return AVL_ERR_HANDLE;
	err = Avl2108IBaseGetFunctionalMode(obj, &functional);
	if (err != AVL_OK)
		return err;
	if (functional != 0)
		return AVL_ERR_PARAM;
	return Avl2108IBaseSendRxOP(obj, 3);
}

/* Carrier offset of the locked signal, rounded to MHz (register in 100 kHz). */
static int Avl2108IDVBSxRxGetRFOffset(TunerObject_t *obj, S32 *offset)
{
	U16 raw;
	S32 value;
	int err;

	*offset = 0;
	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return AVL_ERR_HANDLE;
	err = Avl2108I2cRead16(obj, 0x76e, &raw);
	if (err != AVL_OK)
		return err;
	value   = (S16)raw;
	*offset = (value < 0) ? (value - 5) / 10 : (value + 5) / 10;
	return AVL_OK;
}

/* Signal level in percent from the AGC (SignalLevelTable). */
static int Avl2108IDVBSxRxGetSignalLevel(TunerObject_t *obj, U8 *percent)
{
	U32 raw;
	U32 agc;
	U32 level;
	U8  index;
	int err;

	*percent = 0;
	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return AVL_ERR_HANDLE;
	err = Avl2108I2cRead32(obj, 0x40004c, &raw);
	if (err != AVL_OK)
		return err;
	Avl2108I2cRead32(obj, 0x43c, &agc);
	level = ((raw + 0x800000) & 0x00ffffff) >> 8;

	for (index = 0; index < AVL_LEVEL_ENTRIES; index++)
	{
		if (SignalLevelTable[index].Level >= level)
		{
			*percent = SignalLevelTable[index].Percent;
			return AVL_OK;
		}
	}
	*percent = 100;
	return AVL_OK;
}

/* SNR in 0.1 dB / 2 steps (register 0x680 in 0.01 dB, clamped). */
static int Avl2108IDVBSxRxGetSNR(TunerObject_t *obj, U8 *snr)
{
	U32 raw;
	int err;

	*snr = 0;
	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return AVL_ERR_HANDLE;
	err = Avl2108I2cRead32(obj, 0x680, &raw);
	if (err != AVL_OK)
		return err;
	if (raw > 10000)
		raw = 100;
	else if (raw > 2000)
		raw = 2000;
	*snr = (U8)((raw + 10) / 20);
	return AVL_OK;
}

/* Fill the detected stream type, code rate and roll-off of a locked signal. */
static int Avl2108IDVBSxRxGetSignalInfo(TunerObject_t *obj, TunerTuneParam_t *tune)
{
	U32 modulation;
	U32 code;
	U32 value;
	int err;

	tune->DvbType      = STREAM_UNKNOWN;
	tune->ModCode      = 0xff;
	tune->RollOff      = 0;
	tune->PunctureRate = 0xff;
	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return AVL_ERR_HANDLE;
	if ((err = Avl2108I2cRead32(obj, 0x650, &modulation)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cRead32(obj, 0x648, &code)) != AVL_OK)
		return err;

	if (code > 5)
	{
		tune->DvbType = STREAM_DVBS2;
		tune->ModCode = (U8)(code - 5);
	}
	else
	{
		tune->DvbType      = STREAM_DVBS1;
		tune->PunctureRate = (U8)((code > 2) ? code + 2 : code + 1);
	}

	if ((err = Avl2108I2cRead32(obj, 0x64c, &value)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cRead32(obj, 0x400030, &value)) != AVL_OK)
		return err;
	switch ((value >> 22) & 3)
	{
		case 0:  tune->RollOff = ROLL_OFF_20; break;
		case 1:  tune->RollOff = ROLL_OFF_25; break;
		default: tune->RollOff = ROLL_OFF_35; break;
	}
	return AVL_OK;
}

/* DVB-S receiver clock setup from the current PlConf entry. */
static int Avl2108IDVBSxRxInitialize(TunerObject_t *obj)
{
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	if ((err = Avl2108I2cWrite16(obj, 0x580, Avl2108CurrentData.RefHz)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x582, Avl2108CurrentData.Clk2)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x584, Avl2108CurrentData.Clk3)) != AVL_OK)
		return err;
	return Avl2108I2cWrite32(obj, 0x44c, 1);
}

/* Lock state, strength and quality as shown by mvapp (10/10 when unlocked). */
static TunerError Avl2108GetSignalInfo(TunerObject_t *obj, StreamType dvbType,
                                       TunerSignalState_t *state)
{
	U16 lock;
	U8  level;
	U8  snr;
	int err;

	(void)dvbType;
	state->Quality   = 0;
	state->LockState = SIGNAL_UNLOCK;
	state->Strength  = 0;
	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return (TunerError)AVL_ERR_HANDLE;
	if ((err = AvL2108IDVBSxRxGetLockStatus(obj, &lock)) != AVL_OK)
		return (TunerError)err;
	if ((err = Avl2108IDVBSxRxGetSignalLevel(obj, &level)) != AVL_OK)
		return (TunerError)err;

	if (lock == 0)
	{
		state->Strength = 10;
		state->Quality  = 10;
		return (TunerError)AVL_OK;
	}

	state->LockState = SIGNAL_LOCK;
	if ((err = Avl2108IDVBSxRxGetSNR(obj, &snr)) != AVL_OK)
		return (TunerError)err;

	if (snr <= 43)
		state->Quality = 80;
	else if (snr <= 80)
		state->Quality = (U8)(80 + (snr - 44) / 2);
	else
		state->Quality = 99;

	if (level <= 42)
		state->Strength = 90;
	else if (level <= 70)
		state->Strength = (U8)(90 + (level - 43) / 3);
	else
		state->Strength = 99;
	return (TunerError)AVL_OK;
}

/* Blind scan status words: progress, found TPs, next start frequency, spare. */
static int Avl2108GetBlindScanStatus(TunerObject_t *obj, U16 *status)
{
	int err;

	if ((obj == NULL) || (Avl2108Initialized != AVL_PATCH_BLIND))
		return AVL_ERR_HANDLE;
	if ((err = Avl2108I2cRead16(obj, 0x7a8, &status[0])) != AVL_OK)
		return err;
	if ((err = Avl2108I2cRead16(obj, 0x7a6, &status[1])) != AVL_OK)
		return err;
	if ((err = Avl2108I2cRead16(obj, 0x5dc, &status[2])) != AVL_OK)
		return err;
	return Avl2108I2cRead16(obj, 0x7aa, &status[3]);
}

/* Wait (max 40 s) for one blind scan segment to reach 100 %. */
static int Avl2108CheckBlindScanResult(TunerObject_t *obj, U16 *status)
{
	U16 count = 0;
	int err;

	for (;;)
	{
		OsWaitMillisecond(100);
		err = Avl2108GetBlindScanStatus(obj, status);
		count++;
		if (err != AVL_OK)
			return err;
		if (status[0] == 100)
			return AVL_OK;
		if (count >= 400)
			return AVL_ERR_BUSY;
		if (TunerGetSearchStop((U32)obj) == 0)
			return AVL_ERR_BUSY;
	}
}

/* Start one blind scan segment. */
static int Avl2108BlindScan(TunerObject_t *obj, Avl2108BlindParam_t *param)
{
	U32 functional;
	U16 start;
	U16 stop;
	int err;

	if ((obj == NULL) || (Avl2108Initialized != AVL_PATCH_BLIND))
		return AVL_ERR_HANDLE;
	if ((err = Avl2108IBaseGetFunctionalMode(obj, &functional)) != AVL_OK)
		return err;
	if (functional != 1)
		return AVL_ERR_MODE;

	start = (U16)param->StartFreq;
	stop  = (U16)param->StopFreq;
	if (start >= stop)
		return AVL_ERR_PARAM;
	if ((err = Avl2108I2cWrite16(obj, 0x5d2, (U16)param->MinSymbol)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x5d0, (U16)param->CenterFreq)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x5e0, 800)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x5e2, 0xb090)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x5dc, start)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x5de, stop)) != AVL_OK)
		return err;
	return Avl2108IBaseSendRxOP(obj, 8);
}

/* Read up to count found transponders starting at start, sorted by frequency. */
static U16 Avl2108ReadBlindTpInfo(TunerObject_t *obj, U16 start, U16 count, TunerTuneParam_t *out)
{
	TunerTuneParam_t swap;
	U32              raw[3];
	U16              total;
	U16              base;
	U16              number;
	U16              i;
	U16              j;
	U16              min;
	U32              addr;

	if ((obj == NULL) || (Avl2108Initialized != AVL_PATCH_BLIND))
		return 0;
	if (Avl2108I2cRead16(obj, 0x7a6, &total) != AVL_OK)
		return 0;
	number = ((U32)start + count > total) ? (U16)(total - start) : count;
	if (number == 0)
		return 0;
	if (Avl2108I2cRead16(obj, 0x5e4, &base) != AVL_OK)
		return 0;

	addr = base + start * 12 + 0x870;
	for (i = 0; i < number; i++)
	{
		if ((Avl2108I2cRead32(obj, addr, &raw[0]) != AVL_OK) ||
		    (Avl2108I2cRead32(obj, addr + 4, &raw[1]) != AVL_OK) ||
		    (Avl2108I2cRead32(obj, addr + 8, &raw[2]) != AVL_OK))
			return 0;
		addr += 12;
		out[i].TpFrequency = (raw[0] + 500) / 1000;
		out[i].Symbolrate  = (raw[1] + 500) / 1000;
		switch ((raw[2] & 0x1c) >> 2)
		{
			case 0: out[i].DvbType = STREAM_DVBS1;       break;
			case 1: out[i].DvbType = STREAM_DVBS2;       break;
			case 2: out[i].DvbType = STREAM_DSS;         break;
			case 3: out[i].DvbType = STREAM_S_TURBOCODE; break;
			case 4: out[i].DvbType = STREAM_UNKNOWN;     break;
			default: break;
		}
	}

	if (number <= 1)
		return number;
	for (i = 0; i < number - 1; i++)
	{
		min = i;
		for (j = i + 1; j < number; j++)
		{
			if (out[j].TpFrequency < out[min].TpFrequency)
				min = j;
		}
		if (min != i)
		{
			swap     = out[i];
			out[i]   = out[min];
			out[min] = swap;
		}
	}
	return number;
}

/* Reset the blind scan engine. */
static int Avl210BlindScanReset(TunerObject_t *obj)
{
	if ((obj == NULL) || (Avl2108Initialized != AVL_PATCH_BLIND))
		return AVL_ERR_HANDLE;
	return Avl2108I2cWrite16(obj, 0x2470, 1);
}

/* Spectrum inversion for the blind scan. */
static int Avl210BlindScanSetSpectrumInversion(TunerObject_t *obj, U16 inversion)
{
	if ((obj == NULL) || (Avl2108Initialized != AVL_PATCH_BLIND))
		return AVL_ERR_HANDLE;
	return Avl2108I2cWrite16(obj, 0x2484, inversion);
}

/*
 * Lock algorithm. searchMode != 0: tune tuneIn and return 1 when locked (tuneOut[0] gets
 * the measured frequency and stream parameters). searchMode == 0: blind scan from
 * FrequencyMin to FrequencyMax, returns the number of transponders in tuneOut[].
 */
static U16 Avl2108LockAlgo(TunerObject_t *obj, TunerTuneParam_t *tuneIn,
                           TunerTuneParam_t *tuneOut, TunerDemodInfo_t *info, U8 searchMode)
{
	const TunerPllFunctions_t *pll;
	Avl2108BlindParam_t        blind;
	U16                        status[4];
	U32                        functional;
	U32                        pllFreq;
	U32                        bandWidth;
	U32                        current;
	U32                        first;
	U32                        last;
	U32                        range;
	U32                        more;
	S32                        offset;
	int                        err;

	if ((obj == NULL) || (Avl2108Initialized == AVL_NOT_INITIALIZED))
		return 0;
	err = Avl2108IBaseGetFunctionalMode(obj, &functional);
	if (err != AVL_OK)
		return (U16)err;

	if (searchMode != 0)
	{
		if ((Avl2108Initialized != AVL_PATCH_DEMOD) || (functional != 0))
			return 0;

		tuneOut->Symbolrate   = tuneIn->Symbolrate;
		tuneOut->RollOff      = 0;
		tuneOut->ModCode      = 0xff;
		tuneOut->PunctureRate = 0xff;
		tuneOut->TpFrequency  = tuneIn->TpFrequency;
		tuneOut->DvbType      = STREAM_UNKNOWN;
		tuneOut->TpNumber     = tuneIn->TpNumber;

		bandWidth = tunerGetBandWidth(tuneIn->Symbolrate, tuneIn->RollOff) + 5;
		if (bandWidth > 34)
			bandWidth = 34;
		else if (bandWidth <= 9)
			bandWidth = 10;

		pll = info->Pll;
		if (Avl2108IBaseHaltCpu(obj) != AVL_OK)
			return 0;
		if (pll->SetFrequency(obj, tuneIn->TpFrequency, (U8)bandWidth) != TUNER_NO_ERROR)
			return 0;
		pll->GetFrequency(obj, &pllFreq);
		if (Avl2108IDVBSxRxLockChannel(obj, tuneIn, searchMode) != AVL_OK)
			return 0;
		OsWaitMillisecond(10);
		if (!AvL2108IDVBSxRxCheckLockStatus(obj, tuneIn->Symbolrate))
			return 0;
		if (Avl2108IDVBSxRxResetErrorStat(obj) != AVL_OK)
			return 1;
		if (Avl2108IDVBSxRxGetRFOffset(obj, &offset) != AVL_OK)
			return 1;
		tuneOut->TpFrequency = tuneIn->TpFrequency - offset;
		Avl2108IDVBSxRxGetSignalInfo(obj, tuneOut);
		return 1;
	}

	/* Blind scan in segments of 20.7 MHz (frequencies in 100 kHz units). */
	TunerSetBlindProcess((U32)obj, 0);
	if ((Avl2108Initialized != AVL_PATCH_BLIND) || (functional != 1))
		return 0;
	err = Avl210BlindScanSetSpectrumInversion(obj, 0);
	if (err != AVL_OK)
		return (U16)err;
	pll = info->Pll;
	if (Avl2108IBaseHaltCpu(obj) != AVL_OK)
		return 0;
	if (Avl210BlindScanReset(obj) != AVL_OK)
		return 0;
	if (tuneIn->FrequencyMin >= tuneIn->FrequencyMax)
		return 0;

	first   = tuneIn->FrequencyMin * 10;
	last    = tuneIn->FrequencyMax * 10;
	range   = last - first;
	current = first;
	more    = 1;
	do
	{
		if (TunerGetSearchStop((U32)obj) == 0)
			return 0;
		TunerSetBlindProcess((U32)obj, (U8)(((current - first) * 100) / range));
		blind.StartFreq = current;
		if (current + 207 > last)
		{
			more           = 0;
			blind.StopFreq = last;
		}
		else
		{
			blind.StopFreq = current + 207;
		}
		blind.CenterFreq = (blind.StartFreq + blind.StopFreq + 1) / 2;
		blind.MinSymbol  = 340;
		pllFreq          = (blind.CenterFreq + 5) / 10;
		if (pll->SetFrequency(obj, pllFreq, 34) != TUNER_NO_ERROR)
			return 0;
		pll->GetFrequency(obj, &pllFreq);
		blind.CenterFreq = pllFreq * 10;
		if (Avl2108BlindScan(obj, &blind) != AVL_OK)
			return 0;
		if (Avl2108CheckBlindScanResult(obj, status) != AVL_OK)
			return 0;
		current = status[2];
	} while (more);

	TunerSetBlindProcess((U32)obj, 100);
	return Avl2108ReadBlindTpInfo(obj, 0, status[1], tuneOut);
}

/* Point the repeater at the IX2564 and start it. */
static int Avl2108ExtTunerInitialize(TunerObject_t *obj)
{
	int err;

	if (obj == NULL)
		return AVL_ERR_HANDLE;
	if ((err = Avl2108I2cWrite16(obj, 0x5d4, obj->PllAddress)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x5da, 0)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x5d8, 0)) != AVL_OK)
		return err;
	if ((err = Avl2108I2cWrite16(obj, 0x5d6, 0x140)) != AVL_OK)
		return err;
	return Avl2108I2cRepeateInit(obj);
}

/* Load a patch (0 = demod, 1 = blind scan) and configure the receiver around it. */
static int Avl2108InitMode(TunerObject_t *obj, TunerDemodInfo_t *info, U32 patchType)
{
	int err;

	if ((patchType > AVL_PATCH_BLIND) || (obj == NULL))
		return AVL_ERR_HANDLE;
	err = Avl2108IBaseInitialize(obj, info->RefClock, patchType);
	if (err != AVL_OK)
	{
		Avl2108I2cWrite32(obj, 0x6c0000, 1);
		OsWaitMillisecond(10);
		err = Avl2108IBaseInitialize(obj, info->RefClock, patchType);
		if (err != AVL_OK)
			return err;
	}
	if ((err = Avl2108IBaseCheckStatus(obj)) != AVL_OK)
		return err;
	if ((err = Avl2108IDVBSxRxInitialize(obj)) != AVL_OK)
		return err;
	if ((err = Avl2108IDVBSxRxSetRFAGCPola(obj, 1)) != AVL_OK)
		return err;
	if (patchType == AVL_PATCH_DEMOD)
	{
		if ((err = Avl2108IDVBSxRxSetMpegOutput(obj, info->MpegMode)) != AVL_OK)
			return err;
	}
	if ((err = Avl2108ExtTunerInitialize(obj)) != AVL_OK)
		return err;
	return Avl2108IDiseqcInitialize(obj);
}

/* Demodulator open: fill info and bring the chip up with the normal demod patch. */
static TunerError Avl2108Init(TunerObject_t *obj, TunerDemodInitParam_t *param,
                              TunerDemodInfo_t *info)
{
	int err;

	info->ChipId   = 0x2108;
	info->RefClock = param->RefClock;
	info->Reserved = param->Reserved2;
	info->Pll      = param->Pll;
	info->MpegMode = param->MpegMode;

	Avl2108Initialized        = AVL_NOT_INITIALIZED;
	Avl2108DiseqcOutputStatus = 0;
	err = Avl2108InitMode(obj, info, AVL_PATCH_DEMOD);
	TUNER_DEBUG("avl2108 init (ref %u, mpeg %u): %d\n", (unsigned)info->RefClock,
	            info->MpegMode, err);
	if (err != AVL_OK)
	{
		memset(info, 0, sizeof(*info));
		return (TunerError)err;
	}
	Avl2108Initialized        = AVL_PATCH_DEMOD;
	Avl2108DiseqcOutputStatus = 1;
	return TUNER_NO_ERROR;
}

/* Switch between the normal (normal != 0) and the blind scan patch. */
static TunerError Avl2108ChangeSearchMode(TunerObject_t *obj, TunerDemodInfo_t *info, U8 normal)
{
	U32 wanted = normal ? AVL_PATCH_DEMOD : AVL_PATCH_BLIND;
	int err;

	if (Avl2108Initialized == wanted)
		return TUNER_NO_ERROR;
	err = Avl2108InitMode(obj, info, wanted);
	if (err == AVL_OK)
		Avl2108Initialized = wanted;
	return (TunerError)err;
}

const TunerDemodFunctions_t Avl2108DemodFunctions =
{
	Avl2108LockAlgo,
	Avl2108I2cRepeateRead,
	Avl2108I2cRepeateWrite,
	Avl2108SendDiseqcData,
	Avl2108Set22K,
	Avl2108SetToneBurst,
	Avl2108GetSignalInfo,
	Avl2108ChangeSearchMode,
	Avl2108Init,
};
