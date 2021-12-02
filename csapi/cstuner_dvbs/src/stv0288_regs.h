#ifndef __STV0288_REGS_H__
#define __STV0288_REGS_H__
    
#define R288_ID			0
#define F288_CHIP_IDENT		0
#define F288_RELEASE		1
    
/* I2CRPT  */ 
#define R288_I2CRPT		1
#define F288_I2CT_ON		2
#define F288_ENARPT_LEVEL	3
#define F288_SCLT_DELAY		4
#define F288_SCLT_VALUE		5
#define F288_STOP_ENABLE	6
#define F288_SDAT_VALUE		7
    
/* ACR */ 
#define R288_ACR		2
#define F288_PRESCALER		8
#define F288_DIVIDER		9
    
/* F22FR */
#define R288_F22FR		3
#define F288_FFR_REG		10
    
/* F22RX */
#define R288_F22RX		4
#define F288_FRX_REG		11
    
/*  DISEQC */ 
#define R288_DISEQC		5
#define F288_DIS_RESET		12
#define F288_TIM_OFF		13
#define F288_TIM_CMD		14
#define F288_DIS_PRECHARGE	15
#define F288_DISEQC_MODE	16
    
/* DISEQCFIFO */ 
#define R288_DISEQCFIFO		6
#define F288_DISEQC_FIFO1	17
    
/* DISEQCSTAT1 */ 
#define R288_DISEQCSTAT1	7
#define F288_TX_FAIL		18
#define F288_FIFO_FULL		19
#define F288_TX_IDDLE		20
#define F288_READ_WRITE_COUNTER	21
    
/* DISEQCSTAT2     */ 
#define R288_DISEQCSTAT2	8
#define F288_GAPBURST_FLAG	22
    
/* DISEQC2 */ 
#define R288_DISEQC2		9
#define F288_RECEIVER_ON	23
#define F288_IGNORE_SH22KHZ	24
#define F288_ONECHIP_TRX	25
#define F288_EXT_ENVEL		26
#define F288_PIN_SELECT		27
#define F288_IRQ_RXEND		28
#define F288_IRQ_HALF_FIFO	29
    
/* DISRX_ST0 */
#define R288_DISRX_ST0		10
#define F288_RX_END		30
#define F288_RX_ACTIVE		31
#define F288_SHORT_22KHZ	32
#define F288_RX_COUNT_TONE	33
#define F288_FIFO_8B_READY	34
#define F288_FIFO_EMPTY		35
#define F288_RX_NON_BYTE	36
#define F288_ABORT_DISEQC	37
    
/* DISRX_ST1 */
#define R288_DISRX_ST1		11
#define F288_RX_FAIL		38
#define F288_PARITY_FAIL	39
#define F288_FIFO_OVERFLOW	40
#define F288_FIFO_BYTENBR	41
    
/* DISTXWAIT */
#define R288_DISTXWAIT		12
#define F288_DISTXWAIT		42
    
/* TSREG   */ 
#define R288_TSREG		13
#define F288_ITSTATUS_MODE	43
#define F288_DEMOUNT_M		44
#define F288_SERIAL_OUT_D0	45
#define F288_OUTRS_HZ		46
    
/* AGC1C   */ 
#define R288_AGC1C		14
#define F288_ENA_DCADJ		47
#define F288_AVERAGE_ON		48
#define F288_AGC_OPDRAIN	49
#define F288_IAGC		50
#define F288_AGCIQ_BETA		51
    
/* AGC1R   */ 
#define R288_AGC1R		15
#define F288_AGC1R_REF		52
    
/* AGC1IN  */ 
#define R288_AGC1IN		16
#define F288_AGC1_VALUE		53
    
/* RTC     */ 
#define R288_RTC		17
#define F288_ALPHA_TMG		54
#define F288_BETA_TMG		55
    
/* AGC2C0  */ 
#define R288_AGC2C0		18
#define F288_AGC2COEFF		56
    
/* AGC2O   */ 
#define R288_AGC2O		19
#define F288_AGC2_REF		57
    
/* STEP1   */ 
#define R288_STEP1		20
#define F288_STEP_MINUS		58
#define F288_STEP_PLUS		59
    
/* CFD     */ 
#define R288_CFD		21
#define F288_CFD_ON		60
#define F288_BETA_FC		61
#define F288_FDCT		62
#define F288_LDL		63
    
/* ACLC    */ 
#define R288_ACLC		22
#define F288_DEROT_ON_OFF	64
#define F288_ACLC		65
#define F288_NOISE		66
#define F288_ALPHA		67
    
/* BCLC    */ 
#define R288_BCLC		23
#define F288_ALGO		68
#define F288_BETA		69
    
/* R8PSK   */ 
#define R288_R8PSK		24
#define F288_MODE_COEF		70
    
/* LDT     */ 
#define R288_LDT		25
#define F288_LOCK_THRESHOLD	71
    
/* LDT2    */ 
#define R288_LDT2		26
#define F288_LOCK_THRESHOLD2	72
    
/* DACR1   */ 
#define R288_DACR1		27
#define F288_DAC_MODE		73
#define F288_DACI_LSB		74
    
/* DACR2   */ 
#define R288_DACR2		28
#define F288_DACI_MSB		75
    
/* TLIRM   */ 
#define R288_TLIRM		29
#define F288_TMG_LOCK		76
#define F288_TMG_FINAL_IND_MSB	77
    
/* TLIRL   */ 
#define R288_TLIRL		30
#define F288_TMG_FINAL_IND_LSB	78
    
/* AGC2I1  */ 
#define R288_AGC2I1		31
#define F288_AGC2_INTEGRATOR_MSB 79
    
/* AGC2I2  */ 
#define R288_AGC2I2		32
#define F288_AGC2_INTEGRATOR_LSB 80
    
/* RTFM    */ 
#define R288_RTFM		33
#define F288_TIMING_LOOP_FREQ_MSB 81
    
/* RTFL    */ 
#define R288_RTFL		34
#define F288_TIMING_LOOP_FREQ_LSB 82
    
/* VSTATUS */ 
#define R288_VSTATUS		35
#define F288_CF			3
#define F288_VSTATUS_6		84
#define F288_VSTATUS_5		85
#define F288_PRF		86
#define F288_LK			7
#define F288_PR			8
    
/* LDI     */ 
#define R288_LDI		36
#define F288_LOCK_DET_INTEGR	89
    
/* ECNTM   */ 
#define R288_ECNTM		37
#define F288_ERROR_COUNT_MSB	90
    
/* ECNTL   */ 
#define R288_ECNTL		38
#define F288_ERROR_COUNT_LSB	91
    
/* SFRH    */ 
#define R288_SFRH		39
#define F288_SYMB_FREQ_HSB	92
    
/* SFRM    */ 
#define R288_SFRM		40
#define F288_SYMB_FREQ_MSB	93
    
/* SFRL    */ 
#define R288_SFRL		41
#define F288_SYMB_FREQ_LSB	94
    
/* CFRM    */ 
#define R288_CFRM		42
#define F288_CARRIER_FREQUENCY_MSB 5
    
/* CFRL    */ 
#define R288_CFRL		43
#define F288_CARRIER_FREQUENCY_LSB 96
    
/* NIRM    */ 
#define R288_NIRM		44
#define F288_NOISE_IND_MSB	97
    
/* NIRL    */ 
#define R288_NIRL		45
#define F288_NOISE_IND_LSB	98
    
/* VERROR  */ 
#define R288_VERROR		46
#define F288_ERROR_VAL		99
    
/* FECM    */ 
#define R288_FECM		47
#define F288_FECMODE		100
#define F288_FECM3		101
#define F288_VIT_DIFF		102
#define F288_SYNC		103
#define F288_SYM		104
    
/* VTH0    */ 
#define R288_VTH0		48
#define F288_VTH0		105
    
/* VTH1    */ 
#define R288_VTH1		49
#define F288_VTH1		106
    
/* VTH2    */ 
#define R288_VTH2		50
#define F288_VTH2		107
    
/* VTH3    */ 
#define R288_VTH3		51
#define F288_VTH3		108
    
/* VTH4    */ 
#define R288_VTH4		52
#define F288_VTH4		109
    
/* VTH5    */ 
#define R288_VTH5		53
#define F288_VTH5		110
    
/* PR      */ 
#define R288_PR			54
#define F288_E7			111
#define F288_E6			112
#define F288_PR_7_8		113
#define F288_PR_6_7		114
#define F288_PR_5_6		115
#define F288_PR_3_4		116
#define F288_PR_2_3		117
#define F288_PR_1_2		118
    
/* VAVSRCH */ 
#define R288_VAVSRCH		55
#define F288_AM			119
#define F288_F			120
#define F288_SN			121
#define F288_TO			122
#define F288_H			123
    
/* RS      */ 
#define R288_RS			56
#define F288_DEINT		124
#define F288_OUTRS_PS		125
#define F288_RS			126
#define F288_DESCRAM		127
#define F288_ERR_BIT		128
#define F288_MPEG		129
#define F288_CLK_POL		130
#define F288_CLK_CFG		131
    
/* RSOUT   */ 
#define R288_RSOUT		57
#define F288_INV_DVALID		132
#define F288_INV_DSTART		133
#define F288_INV_DERROR		134
#define F288_EN_STBACKEND	135
#define F288_ENA8_LEVEL		136
    
/* ERRCTRL */ 
#define R288_ERRCTRL		58
#define F288_ERRMODE		137
#define F288_TSTERS		138
#define F288_ERR_SOURCE		139
#define F288_ECOL3		140
#define F288_RESET_CNT		141
#define F288_NOE		142
    
/* VITPROG */ 
#define R288_VITPROG		59
#define F288_VITPROG_7		143
#define F288_VITPROG_6		144
#define F288_VITPROG_5		145
#define F288_SWAP_ENABLE	146
#define F288_VITPROG_3		147
#define F288_VITPROG_2		148
#define F288_MDIVIDER		149
    
/* ERRCTRL2  */
#define R288_ERRCTRL2		60
#define F288_ERRMODE2		150
#define F288_TSTERS2		151
#define F288_ERR_SOURCE2	152
#define F288_ECOL3_2		153
#define F288_RESET_CNT2		154
#define F288_NOE2		155
    
/* ECNTM2  */ 
#define R288_ECNTM2		61
#define F288_ERROR_COUNT2_MSB	156
    
/* ECNTL2  */ 
#define R288_ECNTL2		62
#define F288_ERROR_COUNT2_LSB	157
    
/* PLLCTRL */ 
#define R288_PLLCTRL		63
#define F288_PLL_MDIV		158
    
/* SYNTCTRL  */
#define R288_SYNTCTRL		64
#define F288_STANDBY		159
#define F288_PLL_STOP		160
#define F288_SEL_OSCI		161
#define F288_PLL_SELRATIO	162
#define F288_BYP_PLL_ADC	163
#define F288_BYPASS_PLL		164
    
/* TSTTNR1 */ 
#define R288_TSTTNR1		65
#define F288_ADC_INVCLK		165
#define F288_ADC_PON		166
#define F288_ADC_INMODE		167
#define F288_OSCI_STOP_I2C	168
#define F288_REGPOFF		169
    
/* IRQMSKM */ 
#define R288_IRQMSKM		66
#define F288_IRQ_MSK_MSB	170
    
/* IRQMSKL */ 
#define R288_IRQMSKL		67
#define F288_IRQ_MSK_LSB	171
    
/* IRQSTATM  */
#define R288_IRQSTATM		68
#define F288_IRQ_STATUS_MSB	172
    
/* IRQSTATL  */
#define R288_IRQSTATL		69
#define F288_IRQ_STATUS_LSB	173
    
/* IRQCFG  */ 
#define R288_IRQCFG		70
#define F288_INV1		174
#define F288_CHOICE1		175
#define F288_INV2		176
#define F288_CHOICE2		177
    
/* SYMBCTRL  */
#define R288_SYMBCTRL		71
#define F288_SYMB_CHOICE	178
    
/* ISYMB   */ 
#define R288_ISYMB		72
#define F288_I_SYMB		179
    
/* QSYMB   */ 
#define R288_QSYMB		73
#define F288_Q_SYMB		180
    
/* ASCTRL  */ 
#define R288_ASCTRL		74
#define F288_FROZE_LOCK		181
#define F288_KI			182
#define F288_AUTOCENTRE		183
#define F288_FINE		184
#define F288_COARSE		185
    
/* COARP1  */ 
#define R288_COARP1		75
#define F288_KT			186
    
/* COARP2  */ 
#define R288_COARP2		76
#define F288_KC			187
#define F288_KS			188
    
/* FMINM   */ 
#define R288_FMINM		77
#define F288_STOP_ON_FMIN	189
#define F288_FMIN_MSB		190
    
/* FMINL   */ 
#define R288_FMINL		78
#define F288_FMIN_LSB		191
    
/* FMAXM   */ 
#define R288_FMAXM		79
#define F288_STOP_ON_FMAX	192
#define F288_FMAX_MSB		193
    
/* FMAXL   */ 
#define R288_FMAXL		80
#define F288_FMAX_LSB		194
    
/* FINEINC */ 
#define R288_FINEINC		81
#define F288_FINE_INCR		195
    
/* STEP2   */ 
#define R288_STEP2		82
#define F288_STEP2_MINUS	196
#define F288_STEP2_PLUS		197
    
/* TH2     */ 
#define R288_TH2		83
#define F288_TH2_MSB		198
    
/* TH2_TH1 */ 
#define R288_TH2_TH1		84
#define F288_TH2_LSB		199
#define F288_TH1_MSB		200
    
/* TH1     */ 
#define R288_TH1		85
#define F288_TH1_LSB		201
    
/* THH     */ 
#define R288_THH		86
#define F288_THH		202
    
/* IND1MAX */ 
#define R288_IND1MAX		87
#define F288_IND1_TRESH		203
    
/* ACCU1VAL  */
#define R288_ACCU1VAL		88
#define F288_IND1_ACC		204
    
/* ACCU2VAL  */
#define R288_ACCU2VAL		89
#define F288_IND2_ACC		205
    
/* IOPGPIO0  */
#define R288_IOPGPIO0		90
#define F288_IOP0_HIGHZ		206
#define F288_IOP0_CFG		207
#define F288_IOP0_XOR		208
    
/* IOPGPIO1  */
#define R288_IOPGPIO1		91
#define F288_IOP1_HIGHZ		209
#define F288_IOP1_CFG		210
#define F288_IOP1_XOR		211
    
/* IOPGPIO2  */
#define R288_IOPGPIO2		92
#define F288_IOP2_HIGHZ		212
#define F288_IOP2_CFG		213
#define F288_IOP2_XOR		214
    
/* IOPGPIO3  */
#define R288_IOPGPIO3		93
#define F288_IOP3_HIGHZ		215
#define F288_IOP3_CFG		216
#define F288_IOP3_XOR		217
    
/* IOPGPIO4  */
#define R288_IOPGPIO4		94
#define F288_IOP4_HIGHZ		218
#define F288_IOP4_CFG		219
#define F288_IOP4_XOR		220
    
/* IOPGPIO5  */
#define R288_IOPGPIO5		95
#define F288_IOP5_HIGHZ		221
#define F288_IOP5_CFG		222
#define F288_IOP5_XOR		223
    
/* IOPGPIO6  */
#define R288_IOPGPIO6		96
#define F288_IOP6_HIGHZ		224
#define F288_IOP6_CFG		225
#define F288_IOP6_XOR		226
    
/* IOPGPIO7  */
#define R288_IOPGPIO7		97
#define F288_IOP7_HIGHZ		227
#define F288_IOP7_CFG		228
#define F288_IOP7_XOR		229
    
/* IOPGPIO8  */
#define R288_IOPGPIO8		98
#define F288_IOP8_HIGHZ		230
#define F288_IOP8_CFG		231
#define F288_IOP8_XOR		232
    
/* IOPGPIO9  */
#define R288_IOPGPIO9		99
#define F288_IOP9_HIGHZ		233
#define F288_IOP9_CFG		234
#define F288_IOP9_XOR		235
    
/* IOPVAL0 */ 
#define R288_IOPVAL0		100
#define F288_GPIO7		236
#define F288_GPIO6		237
#define F288_GPIO5		238
#define F288_GPIO4		239
#define F288_GPIO3		240
#define F288_GPIO2		241
#define F288_GPIO1		242
#define F288_GPIO0		243
    
/* IOPVAL1 */ 
#define R288_IOPVAL1		101
#define F288_CS1		244
#define F288_CS0		245
#define F288_STDBY		246
#define F288_AUXCLK		247
#define F288_DIRCLK		248
#define F288_AGC		249
#define F288_GPIO9		250
#define F288_GPIO8		251
    
/* IOPVAL2 */ 
#define R288_IOPVAL2		102
#define F288_DISEQCOUT		252
    
/* FREEDIS */ 
#define R288_FREEDIS		103
#define F288_DIV_XXX_FREE	253
    
/* FREES   */ 
#define R288_FREES		104
#define F288_SYSTEM_FREE1	254
    
/* FREESA  */ 
#define R288_FREESA		105
#define F288_SAMP_FREE		255
    
/* FREEVIT */ 
#define R288_FREEVIT		106
#define F288_VITERBI_FREE	256
    
/* FREERS  */ 
#define R288_FREERS		107
#define F288_REED_SOLO_FREE55	257
    
/* FREEQDM */ 
#define R288_FREEQDM		108
#define F288_DEMOD_FREE56	258
    
/* TAGC1   */ 
#define R288_TAGC1		109
#define F288_SEL_ADC_PLL	259
#define F288_AGC_BEF_DC		260
#define F288_EN_AGC_PWM		261
#define F288_EN_AGC_PLF		262
#define F288_EN_AGC_TST		263
    
/* IDCOFF  */ 
#define R288_IDCOFF		110
#define F288_I_DC_OFFSET_VALUE	264
    
/* QDCOFF  */ 
#define R288_QDCOFF		111
#define F288_Q_DC_OFFSET_VALUE	265
    
/* TSTR    */ 
#define R288_TSTR		112
#define F288_EN_STRST		266
#define F288_EN_TMG_LOC2	267
#define F288_EN_TMG_LOC1	268
#define F288_EN_STR_FRAC	269
#define F288_EN_STR_ERR		270
#define F288_EN_STR_GPD		271
    
/* TCTLT1  */ 
#define R288_TCTLT1		113
#define F288_SEL_COR		272
#define F288_NOISE_IND_CHOICE	273
#define F288_SEL_TETA		274
#define F288_SEL_CTL_PLF	275
    
/* TSTRM1  */ 
#define R288_TSTRM1		114
#define F288_SELOUTR1		276
#define F288_FSELRAM1		277
#define F288_FSELDEC		278
#define F288_FOEB		279
#define F288_FADR		280
    
/* TSTRATE */ 
#define R288_TSTRATE		115
#define F288_FORCEPHA		281
#define F288_TSTRATE6		282
#define F288_TSTRATE5		283
#define F288_FNEWALPHA		284
#define F288_FROT90		285
#define F288_FOFF		286
#define F288_FR1		287
#define F288_FR2		288
    
/* SELOUT  */ 
#define R288_SELOUT		116
#define F288_EN_VLOG		289
#define F288_SELVIT60		290
#define F288_SELSYN3		291
#define F288_SELSYN2		292
#define F288_SELSYN1		293
#define F288_SELLIFO		294
#define F288_SELFIFO		295
#define F288_SELERR		296
    
/* FORCEIN */ 
#define R288_FORCEIN		117
#define F288_SELVITDATAIN	297
#define F288_FORCE_ACS		298
#define F288_TSTSYN		299
#define F288_TSTRAM64		300
#define F288_TSTRAM		301
#define F288_TSTERR2		302
#define F288_TSTERR1		303
#define F288_TSTACS		304
    
/* TSTFIFOL  */
#define R288_TSTFIFOL		118
#define F288_TSTFIFO7		305
#define F288_TSTFIFO6		306
#define F288_TSTFIFO5		307
#define F288_TSTFIFO4		308
#define F288_TSTFIFO3		309
#define F288_FORMSB		310
#define F288_FORLSB		311
#define F288_TST_FIFO		312
    
/* TSTCK   */ 
#define R288_TSTCK		119
#define F288_TSTCKRST		313
#define F288_TSTCKDIL		314
#define F288_FORCERATE1		315
#define F288_FORCESYMHA		316
#define F288_FORSYMAX		317
#define F288_DIRCKINT		318
    
/* TSTRES  */ 
#define R288_TSTRES		120
#define F288_FRESYYY		319
#define F288_FREESRS		320
#define F288_FRESXXX		321
#define F288_FRESCAR		322
#define F288_FRESACS		323
#define F288_FRESYM		324
#define F288_FRESMAS		325
#define F288_FRESINT		326
    
/* TSTOUT  */ 
#define R288_TSTOUT		121
#define F288_RBACT		327
#define F288_TS			328
#define F288_CTEST		329
    
/* TSTIN   */ 
#define R288_TSTIN		122
#define F288_TEST_IN		330
#define F288_EN_ADC		331
#define F288_SGN_ADC		332
#define F288_BCLK_IN		333
#define F288_TP12		334
    
/* READREG */ 
#define R288_READREG		123
#define F288_READREG		335
    
/* TSTNR2  */ 
#define R288_TSTNR2		124
#define F288_DISEQC_IDDQ	336
#define F288_DISEQC_I2C		337
    
/* TSTDIS  */ 
#define R288_TSTDIS		125
#define F288_EN_DIS		338
#define F288_EN_PTRS		339
#define F288_TST_DIS_5		340
#define F288_TST_DIS_4		341
#define F288_EN_DIS_FIFOS	342
#define F288_TST_PRO		343
#define F288_TST_REG		344
#define F288_TST_PRE		345
    
/* TSTDISRX  */
#define R288_TSTDISRX		126
#define F288_EN_DISRX		346
#define F288_TST_CUR_SRC	347
#define F288_IN_DIGSIG		348
#define F288_HIZ_CUR_SRC	349
#define F288_PIN_SELECT_TST	350
#define F288_TST_DISRX		351
    
/* IOPSDAT */ 
#define R288_IOPSDAT		127
#define F288_SDAT_HIGHZ		352
#define F288_SDAT_CFG		353
#define F288_SDAT_XOR		354
    
/* IOPSCLT */ 
#define R288_IOPSCLT		128
#define F288_SCLT_HIGHZ		355
#define F288_SCLT_CFG		356
#define F288_SCLT_XOR		357
    
/* IOPAGC  */ 
#define R288_IOPAGC		129
#define F288_AGC_HIGHZ		358
#define F288_AGC_CFG		359
#define F288_AGC_XOR		360
    
/* IOPDIRCLK */
#define R288_IOPDIRCLK		130
#define F288_DIRCLK_HIGHZ	361
#define F288_DIRCLK_CFG		362
#define F288_DIRCLK_XOR		363
    
/* IOPAUX  */ 
#define R288_IOPAUX		131
#define F288_AUX_HIGHZ		364
#define F288_AUX_CFG		365
#define F288_AUX_XOR		366
    
/* IOPSTDBY  */
#define R288_IOPSTDBY		132
#define F288_STDBY_HIGHZ	367
#define F288_STDBY_CFG		368
#define F288_STDBY_XOR		369
    
/* IOPCS0  */ 
#define R288_IOPCS0		133
#define F288_CS0_HIGHZ		370
#define F288_CS0_CFG		371
#define F288_CS0_XOR		372
    
/* IOPCS1  */ 
#define R288_IOPCS1		134
#define F288_CS1_HIGHZ		373
#define F288_CS1_CFG		374
#define F288_CS1_XOR		375
    
/* IOPSDISEQC      */ 
#define R288_IOPSDISEQC		135
#define F288_DISEQC_HIGHZ	376
#define F288_DISEQC_CFG		377
#define F288_DISEQC_XOR		378
    
/* TBUSBIT */ 
#define R288_TBUSBIT		136
#define F288_BUS_CHOICE		379
#define F288_BUS_POSITION	380
    
/* TCOMP1  */ 
#define R288_TCOMP1		137
#define F288_SLEEPINHBT		381
#define F288_RA6SRC		382
#define F288_RA5SRC		383
#define F288_RA4SRC		384
#define F288_RA3SRC		385
#define F288_RA2SRC		386
#define F288_RA1SRC		387
#define F288_RA0SRC		388
    
/* TCOMP2  */ 
#define R288_TCOMP2		138
#define F288_ACCURATE		389
#define F288_COMPENS		390
#define F288_COMPTQ		391
#define F288_FREEZE		392
#define F288_CHIPSLEEP		393
    
/* TCOMPSTAT */
#define R288_TCOMPSTAT		139
#define F288_COMPOK		394
#define F288_A6SRC		395
#define F288_A5SRC		396
#define F288_A4SRC		397
#define F288_A3SRC		398
#define F288_A2SRC		399
#define F288_A1SRC		400
#define F288_A0SRC		401
    
/* Number of registers */ 
#define NBREG 			140
    
/* Number of fields */ 
#define STX288_NBFIELDS 	402

/* methods */ 
void stv0288_init(int reset_mod);
void stv0288_reset(void);

void stv0288_write(int addr, unsigned char dat);
int  stv0288_read(int addr);
void stv0288_field_write(int field, int val);
int  stv0288_field_read(int field);
void stv0288_set_field(int field, int val);

#endif

