
#include "i2c_if.h"
#include "stv0288_regs.h"

#define FIELD_TYPE_UNSIGNED 	0
#define FIELD_TYPE_SIGNED   	1

long RegExtClk;	/* MasterClock */
FIELD FieldMap[STX288_NBFIELDS];

extern int Csymbolrate;
extern Tuner_Register_t RegMap[NBREG];

unsigned char Def288Val[NBREG] = /* Default values for Sharp_DVBS_tuner registers */
{
	0x10, 0x15, 0x20, 0x8e, 0x8e, 0x12, 0x00, 0x20, 
	0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xc1, 0x54,
	0x40, 0x7a, 0x03, 0x48, 0x84, 0xc5, 0xb8, 0x9c, 
	0x00, 0xa6, 0x88, 0x8f, 0xf0, 0x80, 0x1a, 0x0b, 
	0x54, 0xff, 0x01, 0x9a, 0x7f, 0x00, 0x00, 0x46, 
	0x66, 0x90, 0xfa, 0xd9, 0x02, 0xb1, 0x00, 0x00,
	0x1e, 0x14, 0x0f, 0x09, 0x0c, 0x05, 0x2f, 0x16, 
	0xbd, 0x00, 0x13, 0x11, 0x30, 0x00, 0x00, 0x63, 
	0x04, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xd1, 0x33, 0x10, 0x36, 0x21, 0x94, 0xb2, 0x29,
	0x64, 0x2b, 0x54, 0x86, 0x00, 0x9b, 0x08, 0x7f, 
	0xff, 0x8d, 0x82, 0x82, 0x82, 0x02, 0x02, 0x02, 
	0x82, 0x82, 0x82, 0x82, 0x38, 0x0c, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x1c, 0x00, 0x48, 0x00, 0xb8, 
	0x3a, 0x10, 0x82, 0x80, 0x82, 0x82, 0x82, 0x20, 
	0x00, 0x00, 0x00, 0xc0
};

static sem_t sem_ic;

static unsigned char __field_create_mask(int field);
static void __add_field(int RegId, int FieldId, char Pos, char NbBits, char Type);
static void __add_register(int addr, char Address, char Reset);

void stv0288_write(int addr, unsigned char dat)
{
	unsigned char data_buffer[2];

	sem_wait(&sem_ic);

	data_buffer[0] = RegMap[addr].Addr;
	data_buffer[1] = dat;
	tuner_i2c_write(0xd0, 0, data_buffer, 2);

	sem_post(&sem_ic);

	RegMap[addr].Value = dat;

	return;
}

int  stv0288_read(int addr)
{
	unsigned char reg_addr;
	unsigned char value = 0xff;

	sem_wait(&sem_ic);

	reg_addr = RegMap[addr].Addr;
	tuner_i2c_read(0xd0, reg_addr, &value, 1);

	sem_post(&sem_ic);

	RegMap[addr].Value = value;

	return value;
}

void stv0288_reset(void)
{
	int i;

	for (i = 0; i < NBREG; i++)
		stv0288_write(i, RegMap[i].Reset);

	return;
}

void stv0288_field_write(int field, int fieldval)
{
	FIELD *pfield;

	pfield = &(FieldMap[field]);

	if (pfield->Type == FIELD_TYPE_SIGNED)
		fieldval = (fieldval > 0) ? fieldval : fieldval + (1 << pfield->Bits);
	fieldval = pfield->Mask & (fieldval << pfield->Pos);

	RegMap[pfield->Reg].Value = (RegMap[pfield->Reg].Value & (~pfield->Mask)) + fieldval;

	return;
}

void stv0288_set_field(int field, int val)
{
	FIELD *pfield;

	pfield = &(FieldMap[field]);

	stv0288_read(pfield->Reg);
	stv0288_field_write(field, val);
	stv0288_write(pfield->Reg, RegMap[pfield->Reg].Value);

	return;
}

int  stv0288_field_read(int field)
{
	int value;
	FIELD *pfield;

	pfield = &(FieldMap[field]); /* Just for code simplification */
	value = (RegMap[pfield->Reg].Value & pfield->Mask) >> pfield->Pos; /* Extract field */

	if ((pfield->Type == FIELD_TYPE_SIGNED) && (value >= (1 << (pfield->Bits - 1))))
		value = value - (1 << pfield->Bits);	/* Compute signed value */

	return value;
}

void stv0288_init(int reset_mod)
{
	unsigned int index = 0;

	sem_init(&sem_ic, 0, 1);

	/* REGISTER INITIALISATION */
	/* ID */
	__add_register(R288_ID, 0x00, Def288Val[index++]);
	__add_field(R288_ID, F288_CHIP_IDENT, 4, 4, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ID, F288_RELEASE, 0, 4, FIELD_TYPE_UNSIGNED);

	/* I2CRPT */
	__add_register(R288_I2CRPT, 0x01, Def288Val[index++]);
	__add_field(R288_I2CRPT, F288_I2CT_ON, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_I2CRPT, F288_ENARPT_LEVEL, 4, 3, FIELD_TYPE_UNSIGNED);
	__add_field(R288_I2CRPT, F288_SCLT_DELAY, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_I2CRPT, F288_SCLT_VALUE, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_I2CRPT, F288_STOP_ENABLE, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_I2CRPT, F288_SDAT_VALUE, 0, 1, FIELD_TYPE_UNSIGNED);

	/* ACR */
	__add_register(R288_ACR, 0x02, Def288Val[index++]);
	__add_field(R288_ACR, F288_PRESCALER, 5, 3, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ACR, F288_DIVIDER, 0, 5, FIELD_TYPE_UNSIGNED);

	/* F22FR  */
	__add_register(R288_F22FR, 0x03, Def288Val[index++]);
	__add_field(R288_F22FR, F288_FFR_REG, 0, 8, FIELD_TYPE_UNSIGNED);

	/* F22RX  */
	__add_register(R288_F22RX, 0x04, Def288Val[index++]);
	__add_field(R288_F22RX, F288_FRX_REG, 0, 8, FIELD_TYPE_UNSIGNED);

	/* DISEQC */
	__add_register(R288_DISEQC, 0x05, Def288Val[index++]);
	__add_field(R288_DISEQC, F288_DIS_RESET, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQC, F288_TIM_OFF, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQC, F288_TIM_CMD, 3, 3, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQC, F288_DIS_PRECHARGE, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQC, F288_DISEQC_MODE, 0, 2, FIELD_TYPE_UNSIGNED);

	/* DISEQCFIFO */
	__add_register(R288_DISEQCFIFO, 0x06, Def288Val[index++]);
	__add_field(R288_DISEQCFIFO, F288_DISEQC_FIFO1, 0, 8, FIELD_TYPE_UNSIGNED);

	/* DISEQCSTAT1 */
	__add_register(R288_DISEQCSTAT1, 0x07, Def288Val[index++]);
	__add_field(R288_DISEQCSTAT1, F288_TX_FAIL, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQCSTAT1, F288_FIFO_FULL, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQCSTAT1, F288_TX_IDDLE, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQCSTAT1, F288_READ_WRITE_COUNTER, 0, 5, FIELD_TYPE_UNSIGNED);

	/* DISEQCSTAT2 */
	__add_register(R288_DISEQCSTAT2, 0x08, Def288Val[index++]);
	__add_field(R288_DISEQCSTAT2, F288_GAPBURST_FLAG, 0, 1, FIELD_TYPE_UNSIGNED);

	/* DISEQC2 */
	__add_register(R288_DISEQC2, 0x09, Def288Val[index++]);
	__add_field(R288_DISEQC2, F288_RECEIVER_ON, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQC2, F288_IGNORE_SH22KHZ, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQC2, F288_ONECHIP_TRX, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQC2, F288_EXT_ENVEL, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQC2, F288_PIN_SELECT, 2, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQC2, F288_IRQ_RXEND, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISEQC2, F288_IRQ_HALF_FIFO, 0, 1, FIELD_TYPE_UNSIGNED);

	/* DISRX_ST0  */
	__add_register(R288_DISRX_ST0, 0x0a, Def288Val[index++]);
	__add_field(R288_DISRX_ST0, F288_RX_END, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISRX_ST0, F288_RX_ACTIVE, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISRX_ST0, F288_SHORT_22KHZ, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISRX_ST0, F288_RX_COUNT_TONE, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISRX_ST0, F288_FIFO_8B_READY, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISRX_ST0, F288_FIFO_EMPTY, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISRX_ST0, F288_RX_NON_BYTE, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISRX_ST0, F288_ABORT_DISEQC, 0, 1, FIELD_TYPE_UNSIGNED);

	/* DISRX_ST1  */
	__add_register(R288_DISRX_ST1, 0x0b, Def288Val[index++]);
	__add_field(R288_DISRX_ST1, F288_RX_FAIL, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISRX_ST1, F288_PARITY_FAIL, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISRX_ST1, F288_FIFO_OVERFLOW, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DISRX_ST1, F288_FIFO_BYTENBR, 0, 5, FIELD_TYPE_UNSIGNED);

	/* DISTXWAIT  */
	__add_register(R288_DISTXWAIT, 0x0c, Def288Val[index++]);
	__add_field(R288_DISTXWAIT, F288_DISTXWAIT, 0, 8, FIELD_TYPE_UNSIGNED);

	/* TSREG  */
	__add_register(R288_TSREG, 0x0d, Def288Val[index++]);
	__add_field(R288_TSREG, F288_ITSTATUS_MODE, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSREG, F288_DEMOUNT_M, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSREG, F288_SERIAL_OUT_D0, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSREG, F288_OUTRS_HZ, 0, 1, FIELD_TYPE_UNSIGNED);

	/* AGC1C  */
	__add_register(R288_AGC1C, 0x0e, Def288Val[index++]);
	__add_field(R288_AGC1C, F288_ENA_DCADJ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_AGC1C, F288_AVERAGE_ON, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_AGC1C, F288_AGC_OPDRAIN, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_AGC1C, F288_IAGC, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_AGC1C, F288_AGCIQ_BETA, 0, 4, FIELD_TYPE_UNSIGNED);

	/* AGC1R  */
	__add_register(R288_AGC1R, 0x0f, Def288Val[index++]);
	__add_field(R288_AGC1R, F288_AGC1R_REF, 0, 8, FIELD_TYPE_UNSIGNED);

	/* AGC1IN */
	__add_register(R288_AGC1IN, 0x10, Def288Val[index++]);
	__add_field(R288_AGC1IN, F288_AGC1_VALUE, 0, 8, FIELD_TYPE_SIGNED);

	/* RTC */
	__add_register(R288_RTC, 0x11, Def288Val[index++]);
	__add_field(R288_RTC, F288_ALPHA_TMG, 4, 4, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RTC, F288_BETA_TMG, 0, 4, FIELD_TYPE_UNSIGNED);

	/* AGC2C0 */
	__add_register(R288_AGC2C0, 0x12, Def288Val[index++]);
	__add_field(R288_AGC2C0, F288_AGC2COEFF, 0, 3, FIELD_TYPE_UNSIGNED);

	/* AGC2O  */
	__add_register(R288_AGC2O, 0x13, Def288Val[index++]);
	__add_field(R288_AGC2O, F288_AGC2_REF, 0, 7, FIELD_TYPE_UNSIGNED);

	/* STEP1  */
	__add_register(R288_STEP1, 0x14, Def288Val[index++]);
	__add_field(R288_STEP1, F288_STEP_MINUS, 4, 4, FIELD_TYPE_UNSIGNED);
	__add_field(R288_STEP1, F288_STEP_PLUS, 0, 4, FIELD_TYPE_UNSIGNED);

	/* CFD */
	__add_register(R288_CFD, 0x15, Def288Val[index++]);
	__add_field(R288_CFD, F288_CFD_ON, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_CFD, F288_BETA_FC, 4, 3, FIELD_TYPE_UNSIGNED);
	__add_field(R288_CFD, F288_FDCT, 2, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_CFD, F288_LDL, 0, 2, FIELD_TYPE_UNSIGNED);

	/* ACLC   */
	__add_register(R288_ACLC, 0x16, Def288Val[index++]);
	__add_field(R288_ACLC, F288_DEROT_ON_OFF, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ACLC, F288_ACLC, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ACLC, F288_NOISE, 4, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ACLC, F288_ALPHA, 0, 4, FIELD_TYPE_UNSIGNED);

	/* BCLC   */
	__add_register(R288_BCLC, 0x17, Def288Val[index++]);
	__add_field(R288_BCLC, F288_ALGO, 6, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_BCLC, F288_BETA, 0, 6, FIELD_TYPE_UNSIGNED);

	/* R8PSK  */
	__add_register(R288_R8PSK, 0x18, Def288Val[index++]);
	__add_field(R288_R8PSK, F288_MODE_COEF, 3, 1, FIELD_TYPE_UNSIGNED);

	/* LDT */
	__add_register(R288_LDT, 0x19, Def288Val[index++]);
	__add_field(R288_LDT, F288_LOCK_THRESHOLD, 0, 8, FIELD_TYPE_SIGNED);

	/* LDT2   */
	__add_register(R288_LDT2, 0x1a, Def288Val[index++]);
	__add_field(R288_LDT2, F288_LOCK_THRESHOLD2, 0, 8, FIELD_TYPE_SIGNED);

	/* DACR1  */
	__add_register(R288_DACR1, 0x1b, Def288Val[index++]);
	__add_field(R288_DACR1, F288_DAC_MODE, 5, 3, FIELD_TYPE_UNSIGNED);
	__add_field(R288_DACR1, F288_DACI_LSB, 0, 4, FIELD_TYPE_UNSIGNED);

	/* DACR2  */
	__add_register(R288_DACR2, 0x1c, Def288Val[index++]);
	__add_field(R288_DACR2, F288_DACI_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* TLIRM  */
	__add_register(R288_TLIRM, 0x1e, Def288Val[index++]);
	__add_field(R288_TLIRM, F288_TMG_LOCK, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TLIRM, F288_TMG_FINAL_IND_MSB, 0, 5, FIELD_TYPE_UNSIGNED);

	/* TLIRL  */
	__add_register(R288_TLIRL, 0x1f, Def288Val[index++]);
	__add_field(R288_TLIRL, F288_TMG_FINAL_IND_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* AGC2I1 */
	__add_register(R288_AGC2I1, 0x20, Def288Val[index++]);
	__add_field(R288_AGC2I1, F288_AGC2_INTEGRATOR_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* AGC2I2 */
	__add_register(R288_AGC2I2, 0x21, Def288Val[index++]);
	__add_field(R288_AGC2I2, F288_AGC2_INTEGRATOR_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* RTFM   */
	__add_register(R288_RTFM, 0x22, Def288Val[index++]);
	__add_field(R288_RTFM, F288_TIMING_LOOP_FREQ_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* RTFL   */
	__add_register(R288_RTFL, 0x23, Def288Val[index++]);
	__add_field(R288_RTFL, F288_TIMING_LOOP_FREQ_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* VSTATUS */
	__add_register(R288_VSTATUS, 0x24, Def288Val[index++]);
	__add_field(R288_VSTATUS, F288_CF, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VSTATUS, F288_VSTATUS_6, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VSTATUS, F288_VSTATUS_5, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VSTATUS, F288_PRF, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VSTATUS, F288_LK, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VSTATUS, F288_PR, 0, 3, FIELD_TYPE_UNSIGNED);

	/* LDI */
	__add_register(R288_LDI, 0x25, Def288Val[index++]);
	__add_field(R288_LDI, F288_LOCK_DET_INTEGR, 0, 8, FIELD_TYPE_SIGNED);

	/* ECNTM */
	__add_register(R288_ECNTM, 0x26, Def288Val[index++]);
	__add_field(R288_ECNTM, F288_ERROR_COUNT_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* ECNTL */
	__add_register(R288_ECNTL, 0x27, Def288Val[index++]);
	__add_field(R288_ECNTL, F288_ERROR_COUNT_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* SFRH */
	__add_register(R288_SFRH, 0x28, Def288Val[index++]);
	__add_field(R288_SFRH, F288_SYMB_FREQ_HSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* SFRM */
	__add_register(R288_SFRM, 0x29, Def288Val[index++]);
	__add_field(R288_SFRM, F288_SYMB_FREQ_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* SFRL */
	__add_register(R288_SFRL, 0x2a, Def288Val[index++]);
	__add_field(R288_SFRL, F288_SYMB_FREQ_LSB, 4, 4, FIELD_TYPE_UNSIGNED);

	/* CFRM */
	__add_register(R288_CFRM, 0x2b, Def288Val[index++]);
	__add_field(R288_CFRM, F288_CARRIER_FREQUENCY_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* CFRL */
	__add_register(R288_CFRL, 0x2c, Def288Val[index++]);
	__add_field(R288_CFRL, F288_CARRIER_FREQUENCY_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* NIRM */
	__add_register(R288_NIRM, 0x2d, Def288Val[index++]);
	__add_field(R288_NIRM, F288_NOISE_IND_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* NIRL */
	__add_register(R288_NIRL, 0x2e, Def288Val[index++]);
	__add_field(R288_NIRL, F288_NOISE_IND_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* VERROR */
	__add_register(R288_VERROR, 0x2f, Def288Val[index++]);
	__add_field(R288_VERROR, F288_ERROR_VAL, 0, 8, FIELD_TYPE_UNSIGNED);

	/* FECM */
	__add_register(R288_FECM, 0x30, Def288Val[index++]);
	__add_field(R288_FECM, F288_FECMODE, 4, 4, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FECM, F288_FECM3, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FECM, F288_VIT_DIFF, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FECM, F288_SYNC, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FECM, F288_SYM, 0, 1, FIELD_TYPE_UNSIGNED);

	/* VTH0 */
	__add_register(R288_VTH0, 0x31, Def288Val[index++]);
	__add_field(R288_VTH0, F288_VTH0, 0, 7, FIELD_TYPE_UNSIGNED);

	/* VTH1 */
	__add_register(R288_VTH1, 0x32, Def288Val[index++]);
	__add_field(R288_VTH1, F288_VTH1, 0, 7, FIELD_TYPE_UNSIGNED);

	/* VTH2 */
	__add_register(R288_VTH2, 0x33, Def288Val[index++]);
	__add_field(R288_VTH2, F288_VTH2, 0, 7, FIELD_TYPE_UNSIGNED);

	/* VTH3 */
	__add_register(R288_VTH3, 0x34, Def288Val[index++]);
	__add_field(R288_VTH3, F288_VTH3, 0, 7, FIELD_TYPE_UNSIGNED);

	/* VTH4 */
	__add_register(R288_VTH4, 0x35, Def288Val[index++]);
	__add_field(R288_VTH4, F288_VTH4, 0, 7, FIELD_TYPE_UNSIGNED);

	/* VTH5 */
	__add_register(R288_VTH5, 0x36, Def288Val[index++]);
	__add_field(R288_VTH5, F288_VTH5, 0, 7, FIELD_TYPE_UNSIGNED);

	/* PR */
	__add_register(R288_PR, 0x37, Def288Val[index++]);
	__add_field(R288_PR, F288_E7, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_PR, F288_E6, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_PR, F288_PR_7_8, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_PR, F288_PR_6_7, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_PR, F288_PR_5_6, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_PR, F288_PR_3_4, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_PR, F288_PR_2_3, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_PR, F288_PR_1_2, 0, 1, FIELD_TYPE_UNSIGNED);

	/* VAVSRCH */
	__add_register(R288_VAVSRCH, 0x38, Def288Val[index++]);
	__add_field(R288_VAVSRCH, F288_AM, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VAVSRCH, F288_F, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VAVSRCH, F288_SN, 4, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VAVSRCH, F288_TO, 2, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VAVSRCH, F288_H, 0, 2, FIELD_TYPE_UNSIGNED);

	/* RS */
	__add_register(R288_RS, 0x39, Def288Val[index++]);
	__add_field(R288_RS, F288_DEINT, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RS, F288_OUTRS_PS, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RS, F288_RS, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RS, F288_DESCRAM, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RS, F288_ERR_BIT, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RS, F288_MPEG, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RS, F288_CLK_POL, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RS, F288_CLK_CFG, 0, 1, FIELD_TYPE_UNSIGNED);

	/* RSOUT  */
	__add_register(R288_RSOUT, 0x3a, Def288Val[index++]);
	__add_field(R288_RSOUT, F288_INV_DVALID, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RSOUT, F288_INV_DSTART, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RSOUT, F288_INV_DERROR, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RSOUT, F288_EN_STBACKEND, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_RSOUT, F288_ENA8_LEVEL, 0, 4, FIELD_TYPE_UNSIGNED);

	/* ERRCTRL */
	__add_register(R288_ERRCTRL, 0x3b, Def288Val[index++]);
	__add_field(R288_ERRCTRL, F288_ERRMODE, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ERRCTRL, F288_TSTERS, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ERRCTRL, F288_ERR_SOURCE, 4, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ERRCTRL, F288_ECOL3, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ERRCTRL, F288_RESET_CNT, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ERRCTRL, F288_NOE, 0, 2, FIELD_TYPE_UNSIGNED);

	/* VITPROG */
	__add_register(R288_VITPROG, 0x3c, Def288Val[index++]);
	__add_field(R288_VITPROG, F288_VITPROG_7, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VITPROG, F288_VITPROG_6, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VITPROG, F288_VITPROG_5, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VITPROG, F288_SWAP_ENABLE, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VITPROG, F288_VITPROG_3, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VITPROG, F288_VITPROG_2, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_VITPROG, F288_MDIVIDER, 0, 2, FIELD_TYPE_UNSIGNED);

	/* ERRCTRL2 */
	__add_register(R288_ERRCTRL2, 0x3d, Def288Val[index++]);
	__add_field(R288_ERRCTRL2, F288_ERRMODE2, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ERRCTRL2, F288_TSTERS2, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ERRCTRL2, F288_ERR_SOURCE2, 4, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ERRCTRL2, F288_ECOL3_2, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ERRCTRL2, F288_RESET_CNT2, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ERRCTRL2, F288_NOE2, 0, 2, FIELD_TYPE_UNSIGNED);

	/* ECNTM2 */
	__add_register(R288_ECNTM2, 0x3e, Def288Val[index++]);
	__add_field(R288_ECNTM2, F288_ERROR_COUNT2_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* ECNTL2 */
	__add_register(R288_ECNTL2, 0x3f, Def288Val[index++]);
	__add_field(R288_ECNTL2, F288_ERROR_COUNT2_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* PLLCTRL */
	__add_register(R288_PLLCTRL, 0x40, Def288Val[index++]);
	__add_field(R288_PLLCTRL, F288_PLL_MDIV, 0, 8, FIELD_TYPE_UNSIGNED);

	/* SYNTCTRL   */
	__add_register(R288_SYNTCTRL, 0x41, Def288Val[index++]);
	__add_field(R288_SYNTCTRL, F288_STANDBY, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SYNTCTRL, F288_PLL_STOP, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SYNTCTRL, F288_SEL_OSCI, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SYNTCTRL, F288_PLL_SELRATIO, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SYNTCTRL, F288_BYP_PLL_ADC, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SYNTCTRL, F288_BYPASS_PLL, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TSTTNR1 */
	__add_register(R288_TSTTNR1, 0x42, Def288Val[index++]);
	__add_field(R288_TSTTNR1, F288_ADC_INVCLK, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTTNR1, F288_ADC_PON, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTTNR1, F288_ADC_INMODE, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTTNR1, F288_OSCI_STOP_I2C, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTTNR1, F288_REGPOFF, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IRQMSKM */
	__add_register(R288_IRQMSKM, 0x43, Def288Val[index++]);
	__add_field(R288_IRQMSKM, F288_IRQ_MSK_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* IRQMSKL */
	__add_register(R288_IRQMSKL, 0x44, Def288Val[index++]);
	__add_field(R288_IRQMSKL, F288_IRQ_MSK_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* IRQSTATM   */
	__add_register(R288_IRQSTATM, 0x45, Def288Val[index++]);
	__add_field(R288_IRQSTATM, F288_IRQ_STATUS_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* IRQSTATL   */
	__add_register(R288_IRQSTATL, 0x46, Def288Val[index++]);
	__add_field(R288_IRQSTATL, F288_IRQ_STATUS_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* IRQCFG */
	__add_register(R288_IRQCFG, 0x47, Def288Val[index++]);
	__add_field(R288_IRQCFG, F288_INV1, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IRQCFG, F288_CHOICE1, 4, 3, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IRQCFG, F288_INV2, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IRQCFG, F288_CHOICE2, 0, 3, FIELD_TYPE_UNSIGNED);

	/* SYMBCTRL   */
	__add_register(R288_SYMBCTRL, 0x4a, Def288Val[index++]);
	__add_field(R288_SYMBCTRL, F288_SYMB_CHOICE, 0, 2, FIELD_TYPE_UNSIGNED);

	/* ISYMB  */
	__add_register(R288_ISYMB, 0x4b, Def288Val[index++]);
	__add_field(R288_ISYMB, F288_I_SYMB, 0, 8, FIELD_TYPE_SIGNED);

	/* QSYMB  */
	__add_register(R288_QSYMB, 0x4c, Def288Val[index++]);
	__add_field(R288_QSYMB, F288_Q_SYMB, 0, 8, FIELD_TYPE_SIGNED);

	/* ASCTRL */
	__add_register(R288_ASCTRL, 0x50, Def288Val[index++]);
	__add_field(R288_ASCTRL, F288_FROZE_LOCK, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ASCTRL, F288_KI, 3, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ASCTRL, F288_AUTOCENTRE, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ASCTRL, F288_FINE, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_ASCTRL, F288_COARSE, 0, 1, FIELD_TYPE_UNSIGNED);

	/* COARP1 */
	__add_register(R288_COARP1, 0x51, Def288Val[index++]);
	__add_field(R288_COARP1, F288_KT, 0, 7, FIELD_TYPE_UNSIGNED);

	/* COARP2 */
	__add_register(R288_COARP2, 0x52, Def288Val[index++]);
	__add_field(R288_COARP2, F288_KC, 3, 3, FIELD_TYPE_UNSIGNED);
	__add_field(R288_COARP2, F288_KS, 0, 3, FIELD_TYPE_UNSIGNED);

	/* FMINM  */
	__add_register(R288_FMINM, 0x53, Def288Val[index++]);
	__add_field(R288_FMINM, F288_STOP_ON_FMIN, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FMINM, F288_FMIN_MSB, 0, 7, FIELD_TYPE_UNSIGNED);

	/* FMINL  */
	__add_register(R288_FMINL, 0x54, Def288Val[index++]);
	__add_field(R288_FMINL, F288_FMIN_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* FMAXM  */
	__add_register(R288_FMAXM, 0x55, Def288Val[index++]);
	__add_field(R288_FMAXM, F288_STOP_ON_FMAX, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FMAXM, F288_FMAX_MSB, 0, 7, FIELD_TYPE_UNSIGNED);

	/* FMAXL  */
	__add_register(R288_FMAXL, 0x56, Def288Val[index++]);
	__add_field(R288_FMAXL, F288_FMAX_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* FINEINC */
	__add_register(R288_FINEINC, 0x57, Def288Val[index++]);
	__add_field(R288_FINEINC, F288_FINE_INCR, 0, 8, FIELD_TYPE_SIGNED);

	/* STEP2  */
	__add_register(R288_STEP2, 0x58, Def288Val[index++]);
	__add_field(R288_STEP2, F288_STEP2_MINUS, 4, 4, FIELD_TYPE_UNSIGNED);
	__add_field(R288_STEP2, F288_STEP2_PLUS, 0, 4, FIELD_TYPE_UNSIGNED);

	/* TH2 */
	__add_register(R288_TH2, 0x59, Def288Val[index++]);
	__add_field(R288_TH2, F288_TH2_MSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* TH2_TH1 */
	__add_register(R288_TH2_TH1, 0x5a, Def288Val[index++]);
	__add_field(R288_TH2_TH1, F288_TH2_LSB, 6, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TH2_TH1, F288_TH1_MSB, 0, 2, FIELD_TYPE_UNSIGNED);

	/* TH1 */
	__add_register(R288_TH1, 0x5b, Def288Val[index++]);
	__add_field(R288_TH1, F288_TH1_LSB, 0, 8, FIELD_TYPE_UNSIGNED);

	/* THH */
	__add_register(R288_THH, 0x5c, Def288Val[index++]);
	__add_field(R288_THH, F288_THH, 0, 6, FIELD_TYPE_UNSIGNED);

	/* IND1MAX */
	__add_register(R288_IND1MAX, 0x5d, Def288Val[index++]);
	__add_field(R288_IND1MAX, F288_IND1_TRESH, 0, 8, FIELD_TYPE_UNSIGNED);

	/* ACCU1VAL   */
	__add_register(R288_ACCU1VAL, 0x5e, Def288Val[index++]);
	__add_field(R288_ACCU1VAL, F288_IND1_ACC, 0, 8, FIELD_TYPE_UNSIGNED);

	/* ACCU2VAL   */
	__add_register(R288_ACCU2VAL, 0x5f, Def288Val[index++]);
	__add_field(R288_ACCU2VAL, F288_IND2_ACC, 0, 8, FIELD_TYPE_UNSIGNED);

	/* IOPGPIO0   */
	__add_register(R288_IOPGPIO0, 0x60, Def288Val[index++]);
	__add_field(R288_IOPGPIO0, F288_IOP0_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO0, F288_IOP0_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO0, F288_IOP0_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPGPIO1   */
	__add_register(R288_IOPGPIO1, 0x61, Def288Val[index++]);
	__add_field(R288_IOPGPIO1, F288_IOP1_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO1, F288_IOP1_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO1, F288_IOP1_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPGPIO2   */
	__add_register(R288_IOPGPIO2, 0x62, Def288Val[index++]);
	__add_field(R288_IOPGPIO2, F288_IOP2_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO2, F288_IOP2_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO2, F288_IOP2_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPGPIO3   */
	__add_register(R288_IOPGPIO3, 0x63, Def288Val[index++]);
	__add_field(R288_IOPGPIO3, F288_IOP3_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO3, F288_IOP3_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO3, F288_IOP3_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPGPIO4   */
	__add_register(R288_IOPGPIO4, 0x64, Def288Val[index++]);
	__add_field(R288_IOPGPIO4, F288_IOP4_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO4, F288_IOP4_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO4, F288_IOP4_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPGPIO5   */
	__add_register(R288_IOPGPIO5, 0x65, Def288Val[index++]);
	__add_field(R288_IOPGPIO5, F288_IOP5_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO5, F288_IOP5_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO5, F288_IOP5_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPGPIO6   */
	__add_register(R288_IOPGPIO6, 0x66, Def288Val[index++]);
	__add_field(R288_IOPGPIO6, F288_IOP6_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO6, F288_IOP6_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO6, F288_IOP6_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPGPIO7   */
	__add_register(R288_IOPGPIO7, 0x67, Def288Val[index++]);
	__add_field(R288_IOPGPIO7, F288_IOP7_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO7, F288_IOP7_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO7, F288_IOP7_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPGPIO8   */
	__add_register(R288_IOPGPIO8, 0x68, Def288Val[index++]);
	__add_field(R288_IOPGPIO8, F288_IOP8_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO8, F288_IOP8_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO8, F288_IOP8_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPGPIO9   */
	__add_register(R288_IOPGPIO9, 0x69, Def288Val[index++]);
	__add_field(R288_IOPGPIO9, F288_IOP9_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO9, F288_IOP9_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPGPIO9, F288_IOP9_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPVAL0 */
	__add_register(R288_IOPVAL0, 0x6a, Def288Val[index++]);
	__add_field(R288_IOPVAL0, F288_GPIO7, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL0, F288_GPIO6, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL0, F288_GPIO5, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL0, F288_GPIO4, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL0, F288_GPIO3, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL0, F288_GPIO2, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL0, F288_GPIO1, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL0, F288_GPIO0, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPVAL1 */
	__add_register(R288_IOPVAL1, 0x6b, Def288Val[index++]);
	__add_field(R288_IOPVAL1, F288_CS1, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL1, F288_CS0, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL1, F288_STDBY, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL1, F288_AUXCLK, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL1, F288_DIRCLK, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL1, F288_AGC, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL1, F288_GPIO9, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPVAL1, F288_GPIO8, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPVAL2 */
	__add_register(R288_IOPVAL2, 0x6c, Def288Val[index++]);
	__add_field(R288_IOPVAL2, F288_DISEQCOUT, 0, 1, FIELD_TYPE_UNSIGNED);

	/* FREEDIS */
	__add_register(R288_FREEDIS, 0x70, Def288Val[index++]);
	__add_field(R288_FREEDIS, F288_DIV_XXX_FREE, 0, 8, FIELD_TYPE_UNSIGNED);

	/* FREES  */
	__add_register(R288_FREES, 0x71, Def288Val[index++]);
	__add_field(R288_FREES, F288_SYSTEM_FREE1, 0, 8, FIELD_TYPE_UNSIGNED);

	/* FREESA */
	__add_register(R288_FREESA, 0x72, Def288Val[index++]);
	__add_field(R288_FREESA, F288_SAMP_FREE, 0, 8, FIELD_TYPE_UNSIGNED);

	/* FREEVIT */
	__add_register(R288_FREEVIT, 0x74, Def288Val[index++]);
	__add_field(R288_FREEVIT, F288_VITERBI_FREE, 0, 8, FIELD_TYPE_UNSIGNED);

	/* FREERS */
	__add_register(R288_FREERS, 0x75, Def288Val[index++]);
	__add_field(R288_FREERS, F288_REED_SOLO_FREE55, 0, 8, FIELD_TYPE_UNSIGNED);

	/* FREEQDM */
	__add_register(R288_FREEQDM, 0x76, Def288Val[index++]);
	__add_field(R288_FREEQDM, F288_DEMOD_FREE56, 0, 8, FIELD_TYPE_UNSIGNED);

	/* TAGC1  */
	__add_register(R288_TAGC1, 0x81, Def288Val[index++]);
	__add_field(R288_TAGC1, F288_SEL_ADC_PLL, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TAGC1, F288_AGC_BEF_DC, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TAGC1, F288_EN_AGC_PWM, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TAGC1, F288_EN_AGC_PLF, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TAGC1, F288_EN_AGC_TST, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IDCOFF */
	__add_register(R288_IDCOFF, 0x82, Def288Val[index++]);
	__add_field(R288_IDCOFF, F288_I_DC_OFFSET_VALUE, 0, 6, FIELD_TYPE_UNSIGNED);

	/* QDCOFF */
	__add_register(R288_QDCOFF, 0x83, Def288Val[index++]);
	__add_field(R288_QDCOFF, F288_Q_DC_OFFSET_VALUE, 0, 6, FIELD_TYPE_UNSIGNED);

	/* TSTR   */
	__add_register(R288_TSTR, 0x84, Def288Val[index++]);
	__add_field(R288_TSTR, F288_EN_STRST, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTR, F288_EN_TMG_LOC2, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTR, F288_EN_TMG_LOC1, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTR, F288_EN_STR_FRAC, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTR, F288_EN_STR_ERR, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTR, F288_EN_STR_GPD, 1, 1, FIELD_TYPE_UNSIGNED);

	/* TCTLT1 */
	__add_register(R288_TCTLT1, 0x85, Def288Val[index++]);
	__add_field(R288_TCTLT1, F288_SEL_COR, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCTLT1, F288_NOISE_IND_CHOICE, 2, 2, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCTLT1, F288_SEL_TETA, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCTLT1, F288_SEL_CTL_PLF, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TSTRM1 */
	__add_register(R288_TSTRM1, 0x88, Def288Val[index++]);
	__add_field(R288_TSTRM1, F288_SELOUTR1, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRM1, F288_FSELRAM1, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRM1, F288_FSELDEC, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRM1, F288_FOEB, 2, 3, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRM1, F288_FADR, 0, 2, FIELD_TYPE_UNSIGNED);

	/* TSTRATE */
	__add_register(R288_TSTRATE, 0x89, Def288Val[index++]);
	__add_field(R288_TSTRATE, F288_FORCEPHA, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRATE, F288_TSTRATE6, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRATE, F288_TSTRATE5, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRATE, F288_FNEWALPHA, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRATE, F288_FROT90, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRATE, F288_FOFF, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRATE, F288_FR1, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRATE, F288_FR2, 0, 1, FIELD_TYPE_UNSIGNED);

	/* SELOUT */
	__add_register(R288_SELOUT, 0x8a, Def288Val[index++]);
	__add_field(R288_SELOUT, F288_EN_VLOG, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SELOUT, F288_SELVIT60, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SELOUT, F288_SELSYN3, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SELOUT, F288_SELSYN2, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SELOUT, F288_SELSYN1, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SELOUT, F288_SELLIFO, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SELOUT, F288_SELFIFO, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_SELOUT, F288_SELERR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* FORCEIN */
	__add_register(R288_FORCEIN, 0x8b, Def288Val[index++]);
	__add_field(R288_FORCEIN, F288_SELVITDATAIN, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FORCEIN, F288_FORCE_ACS, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FORCEIN, F288_TSTSYN, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FORCEIN, F288_TSTRAM64, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FORCEIN, F288_TSTRAM, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FORCEIN, F288_TSTERR2, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FORCEIN, F288_TSTERR1, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_FORCEIN, F288_TSTACS, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TSTFIFOL   */
	__add_register(R288_TSTFIFOL, 0x8c, Def288Val[index++]);
	__add_field(R288_TSTFIFOL, F288_TSTFIFO7, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTFIFOL, F288_TSTFIFO6, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTFIFOL, F288_TSTFIFO5, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTFIFOL, F288_TSTFIFO3, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTFIFOL, F288_FORMSB, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTFIFOL, F288_FORLSB, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTFIFOL, F288_TST_FIFO, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TSTCK  */
	__add_register(R288_TSTCK, 0x90, Def288Val[index++]);
	__add_field(R288_TSTCK, F288_TSTCKRST, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTCK, F288_TSTCKDIL, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTCK, F288_FORCERATE1, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTCK, F288_FORCESYMHA, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTCK, F288_FORSYMAX, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTCK, F288_DIRCKINT, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TSTRES */
	__add_register(R288_TSTRES, 0x91, Def288Val[index++]);
	__add_field(R288_TSTRES, F288_FRESYYY, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRES, F288_FREESRS, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRES, F288_FRESXXX, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRES, F288_FRESCAR, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRES, F288_FRESACS, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRES, F288_FRESYM, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRES, F288_FRESMAS, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTRES, F288_FRESINT, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TSTOUT */
	__add_register(R288_TSTOUT, 0x92, Def288Val[index++]);
	__add_field(R288_TSTOUT, F288_RBACT, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTOUT, F288_TS, 1, 3, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTOUT, F288_CTEST, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TSTIN  */
	__add_register(R288_TSTIN, 0x93, Def288Val[index++]);
	__add_field(R288_TSTIN, F288_TEST_IN, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTIN, F288_EN_ADC, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTIN, F288_SGN_ADC, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTIN, F288_BCLK_IN, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTIN, F288_TP12, 3, 1, FIELD_TYPE_UNSIGNED);

	/* READREG */
	__add_register(R288_READREG, 0x94, Def288Val[index++]);
	__add_field(R288_READREG, F288_READREG, 0, 8, FIELD_TYPE_UNSIGNED);

	/* TSTNR2 */
	__add_register(R288_TSTNR2, 0x97, Def288Val[index++]);
	__add_field(R288_TSTNR2, F288_DISEQC_IDDQ, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTNR2, F288_DISEQC_I2C, 0, 5, FIELD_TYPE_UNSIGNED);

	/* TSTDIS */
	__add_register(R288_TSTDIS, 0xa0, Def288Val[index++]);
	__add_field(R288_TSTDIS, F288_EN_DIS, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDIS, F288_EN_PTRS, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDIS, F288_TST_DIS_5, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDIS, F288_TST_DIS_4, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDIS, F288_EN_DIS_FIFOS, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDIS, F288_TST_PRO, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDIS, F288_TST_REG, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDIS, F288_TST_PRE, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TSTDISRX   */
	__add_register(R288_TSTDISRX, 0xa1, Def288Val[index++]);
	__add_field(R288_TSTDISRX, F288_EN_DISRX, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDISRX, F288_TST_CUR_SRC, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDISRX, F288_IN_DIGSIG, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDISRX, F288_HIZ_CUR_SRC, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDISRX, F288_PIN_SELECT_TST, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TSTDISRX, F288_TST_DISRX, 0, 3, FIELD_TYPE_UNSIGNED);

	/* IOPSDAT */
	__add_register(R288_IOPSDAT, 0xb0, Def288Val[index++]);
	__add_field(R288_IOPSDAT, F288_SDAT_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPSDAT, F288_SDAT_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPSDAT, F288_SDAT_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPSCLT */
	__add_register(R288_IOPSCLT, 0xb1, Def288Val[index++]);
	__add_field(R288_IOPSCLT, F288_SCLT_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPSCLT, F288_SCLT_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPSCLT, F288_SCLT_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPAGC */
	__add_register(R288_IOPAGC, 0xb2, Def288Val[index++]);
	__add_field(R288_IOPAGC, F288_AGC_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPAGC, F288_AGC_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPAGC, F288_AGC_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPDIRCLK  */
	__add_register(R288_IOPDIRCLK, 0xb3, Def288Val[index++]);
	__add_field(R288_IOPDIRCLK, F288_DIRCLK_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPDIRCLK, F288_DIRCLK_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPDIRCLK, F288_DIRCLK_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPAUX */
	__add_register(R288_IOPAUX, 0xb4, Def288Val[index++]);
	__add_field(R288_IOPAUX, F288_AUX_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPAUX, F288_AUX_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPAUX, F288_AUX_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPSTDBY   */
	__add_register(R288_IOPSTDBY, 0xb5, Def288Val[index++]);
	__add_field(R288_IOPSTDBY, F288_STDBY_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPSTDBY, F288_STDBY_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPSTDBY, F288_STDBY_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPCS0 */
	__add_register(R288_IOPCS0, 0xb6, Def288Val[index++]);
	__add_field(R288_IOPCS0, F288_CS0_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPCS0, F288_CS0_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPCS0, F288_CS0_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPCS1 */
	__add_register(R288_IOPCS1, 0xb7, Def288Val[index++]);
	__add_field(R288_IOPCS1, F288_CS1_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPCS1, F288_CS1_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPCS1, F288_CS1_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* IOPSDISEQC */
	__add_register(R288_IOPSDISEQC, 0xb8, Def288Val[index++]);
	__add_field(R288_IOPSDISEQC, F288_DISEQC_HIGHZ, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPSDISEQC, F288_DISEQC_CFG, 1, 6, FIELD_TYPE_UNSIGNED);
	__add_field(R288_IOPSDISEQC, F288_DISEQC_XOR, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TBUSBIT */
	__add_register(R288_TBUSBIT, 0xb9, Def288Val[index++]);
	__add_field(R288_TBUSBIT, F288_BUS_CHOICE, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TBUSBIT, F288_BUS_POSITION, 0, 6, FIELD_TYPE_UNSIGNED);

	/* TCOMP1 */
	__add_register(R288_TCOMP1, 0xf1, Def288Val[index++]);
	__add_field(R288_TCOMP1, F288_SLEEPINHBT, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP1, F288_RA6SRC, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP1, F288_RA5SRC, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP1, F288_RA4SRC, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP1, F288_RA3SRC, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP1, F288_RA2SRC, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP1, F288_RA1SRC, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP1, F288_RA0SRC, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TCOMP2 */
	__add_register(R288_TCOMP2, 0xf0, Def288Val[index++]);
	__add_field(R288_TCOMP2, F288_ACCURATE, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP2, F288_COMPENS, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP2, F288_COMPTQ, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP2, F288_FREEZE, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMP2, F288_CHIPSLEEP, 0, 1, FIELD_TYPE_UNSIGNED);

	/* TCOMPSTAT  */
	__add_register(R288_TCOMPSTAT, 0xf2, Def288Val[index++]);
	__add_field(R288_TCOMPSTAT, F288_COMPOK, 7, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMPSTAT, F288_A6SRC, 6, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMPSTAT, F288_A5SRC, 5, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMPSTAT, F288_A4SRC, 4, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMPSTAT, F288_A3SRC, 3, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMPSTAT, F288_A2SRC, 2, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMPSTAT, F288_A1SRC, 1, 1, FIELD_TYPE_UNSIGNED);
	__add_field(R288_TCOMPSTAT, F288_A0SRC, 0, 1, FIELD_TYPE_UNSIGNED);

	stv0288_reset();
}

static void __add_register(int addr, char Address, char Reset)
{
	Tuner_Register_t *pReg;

	pReg = &RegMap[addr];

	pReg->Addr = Address;
	pReg->Reset = Reset;
}

static unsigned char __field_create_mask(int field)
{
	int i;
	unsigned char mask = 0;

	for (i = 0; i < FieldMap[field].Bits; i++) {
		mask <<= 1;
		mask += 1;
	}
	mask = mask << FieldMap[field].Pos;

	return mask;
}

static void __add_field(int RegId, int FieldId, char Pos, char NbBits, char Type)
{
	FIELD *pField;

	pField = &FieldMap[FieldId];
	pField->Reg = RegId;
	pField->Pos = Pos;
	pField->Bits = NbBits;
	pField->Type = Type;
	pField->Mask = __field_create_mask(FieldId);
}

