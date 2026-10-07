/*
 * ix2564.c - Sharp IX2564 satellite tuner PLL driver (port of Merih Ix2564.o).
 *
 * The IX2564 has no sub address: the 4 register bytes are written as one frame and are
 * told apart by their top bits. Accesses go through tunerSetPllResters(), i.e. through
 * the AVL2108 I2C repeater on the Chipbox.
 */
#include "merih_tuner_int.h"

/* Prescaler values selected by bit 4 of byte 3. */
static const U32 Ix2564Pval[2] = { 32, 16 };

/* Shadow of the 4 PLL bytes (+ spare), as in the original. */
static U8 Ix2564RegValue[5] = { 0x00, 0x00, 0x80, 0x00, 0x00 };

/*
 * Set the baseband low-pass filter (MHz, 10..34) and wait for the PLL lock flag.
 * Sequence: full frame with the filter bits cleared, byte 2 with the PLL reset bit,
 * 12 ms, bytes 2-3 with the filter code, then poll the status byte (FL bit 6).
 */
static TunerError Ix2564SetBandWidth(TunerObject_t *obj, U8 bandWidth)
{
	U8         code;
	U8         nibble = 0;
	U8         status = 0;
	U8         count;
	U8         bit;
	TunerError err;

	if (obj == NULL)
		return TUNER_NOT_FOUND;

	if (bandWidth <= 9)
		code = 0;
	else if (bandWidth <= 34)
		code = bandWidth - 10;
	else
		code = 24;
	code = (code >> 1) + 3;
	for (bit = 0; bit < 4; bit++)
		nibble = (U8)((nibble << 1) | ((code >> bit) & 1));

	Ix2564RegValue[2] &= ~0x1c;
	Ix2564RegValue[3] &= ~0x0c;
	err = tunerSetPllResters(obj, 0, Ix2564RegValue, 4);
	if (err != TUNER_NO_ERROR)
		return err;

	Ix2564RegValue[2] |= 0x04;
	err = tunerSetPllResters(obj, 0, &Ix2564RegValue[2], 1);
	if (err != TUNER_NO_ERROR)
		return err;
	OsWaitMillisecond(12);

	Ix2564RegValue[2] |= (nibble << 1) & 0x18;
	Ix2564RegValue[3] |= (nibble << 2) & 0x0c;
	Ix2564RegValue[2] |= 0x04;
	err = tunerSetPllResters(obj, 0, &Ix2564RegValue[2], 2);
	if (err != TUNER_NO_ERROR)
		return err;

	count = 0;
	do
	{
		OsWaitMillisecond(10);
		err = tunerGetPllOneResters(obj, 0, &status);
		count++;
		if (count > 14)
			return err;
	} while ((status & 0x40) == 0);
	return err;
}

/* Program the LO for freqMhz (IF, 950..2150 MHz) and then the filter bandwidth. */
static TunerError Ix2564SetFrequency(TunerObject_t *obj, U32 freqMhz, U8 bandWidth)
{
	U32 divider;
	U32 swallow;
	U8  band;
	U8  pIndex;
	U8  div2;

	if (obj == NULL)
		return TUNER_NOT_FOUND;
	if (freqMhz <= 0x3b5)
		return TUNER_DATA_ERROR;

	if (freqMhz <= 0x3d9)
	{
		div2 = 1; band = 5; pIndex = 1;
	}
	else if (freqMhz <= 0x430)
	{
		div2 = 1; band = 6; pIndex = 1;
	}
	else if (freqMhz <= 0x481)
	{
		div2 = 1; band = 7; pIndex = 0;
	}
	else if (freqMhz <= 0x50a)
	{
		div2 = 0; band = 1; pIndex = 0;
	}
	else if (freqMhz <= 0x5a6)
	{
		div2 = 0; band = 2; pIndex = 0;
	}
	else if (freqMhz <= 0x64e)
	{
		div2 = 0; band = 3; pIndex = 0;
	}
	else if (freqMhz <= 0x6fe)
	{
		div2 = 0; band = 4; pIndex = 0;
	}
	else if (freqMhz <= 0x7b3)
	{
		div2 = 0; band = 5; pIndex = 0;
	}
	else if (freqMhz <= 0x866)
	{
		div2 = 0; band = 6; pIndex = 0;
	}
	else
	{
		return TUNER_DATA_ERROR;
	}

	divider = freqMhz / Ix2564Pval[pIndex];
	swallow = freqMhz % Ix2564Pval[pIndex];
	Ix2564RegValue[3]  = (U8)((pIndex << 4) | (band << 5) | (div2 << 1));
	Ix2564RegValue[0]  = (U8)(((divider >> 3) & 0x1f) | 0x40);
	Ix2564RegValue[3] &= ~0x0c;
	Ix2564RegValue[1]  = (U8)((swallow & 0x1f) | (divider << 5));
	Ix2564RegValue[2]  = 0xe0;
	return Ix2564SetBandWidth(obj, bandWidth);
}

/* LO frequency (MHz) programmed in the shadow registers. */
static TunerError Ix2564GetFrequency(TunerObject_t *obj, U32 *freqMhz)
{
	U32 prescaler;
	U32 divider;

	if (obj == NULL)
		return TUNER_NOT_FOUND;
	prescaler = Ix2564Pval[(Ix2564RegValue[3] & 0x10) ? 1 : 0];
	divider   = ((Ix2564RegValue[0] & 0x1f) << 3) | (Ix2564RegValue[1] >> 5);
	*freqMhz  = ((divider * prescaler + (Ix2564RegValue[1] & 0x1f)) * 4) /
	            ((Ix2564RegValue[2] & 1) ? 8 : 4);
	return TUNER_NO_ERROR;
}

/* Power-up setting: 1550 MHz, widest filter. */
static TunerError Ix2564Init(TunerObject_t *obj, U32 *ifType)
{
	*ifType = 4;
	return Ix2564SetFrequency(obj, 0x60e, 34);
}

const TunerPllFunctions_t Ix2564PllFunctions =
{
	Ix2564Init,
	Ix2564SetFrequency,
	Ix2564GetFrequency,
	Ix2564SetBandWidth,
};
