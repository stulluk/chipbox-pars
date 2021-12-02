
#include "usb_defs.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

#ifndef NULL
#define NULL	(void*)0
#endif

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
#define CFG_PCLK_FREQ		55000000
#define CFG_HZ                  CFG_PCLK_FREQ

#define CFG_TIMERBASE           0x101E2000	/* Timer 0 and 1 base */
#define TIMER_LOAD_VAL 		0xffffffff

#define CFG_TIMER_INTERVAL	10000

#define CFG_TIMER_RELOAD	(CFG_TIMER_INTERVAL >> 4)	/* Divide by 16 */
#define CFG_TIMER_CTRL          0x84	/* Enable, Clock / 16 */
#define MUSB_HSDMA_CHANNELS		2

#define MGC_Read8(_pBase, _offset) *(volatile uint8_t*)(_pBase + _offset)
#define MGC_Read16(_pBase, _offset) *(volatile uint16_t*)(_pBase + _offset)
#define MGC_Read32(_pBase, _offset) *(volatile uint32_t*)(_pBase + _offset)

#define MGC_Write8(_pBase, _offset, _data) MGC_Read8(_pBase, _offset) = _data
#define MGC_Write16(_pBase, _offset, _data) MGC_Read16(_pBase, _offset) = _data
#define MGC_Write32(_pBase, _offset, _data) MGC_Read32(_pBase, _offset) = _data

#if 1 /* for orion_1.3 */
#define MGC_EXTRA_INIT()  do {							\
        volatile unsigned short *gpio_base;					\
        volatile unsigned short *idscs_base;					\
										\
        gpio_base = (volatile unsigned short *)0x101E4000;			\
        gpio_base[0x008 >> 1] = gpio_base[0x008 >> 1] & 0xfff7;			\
        gpio_base[0x004 >> 1] = gpio_base[0x004 >> 1] | 0x0008;			\
        gpio_base[0x000 >> 1] = gpio_base[0x000 >> 1] & 0xfff7;			\
										\
        idscs_base = (volatile unsigned short *)0x10171000;			\
        idscs_base[0x400 >> 1] = idscs_base[0x400 >> 1] | 0x0002;		\
        idscs_base[0x200 >> 1] = idscs_base[0x200 >> 1] & 0xfdff;		\
        udelay(20);								\
        idscs_base[0x200 >> 1] = idscs_base[0x200 >> 1] | 0x0200;		\
}while(0)
#else /* for orion_1.4 */
#define MGC_EXTRA_INIT()  do {							\
        volatile unsigned short *idscs_base;					\
        idscs_base = (volatile unsigned short *)0x10171000;			\
        idscs_base[0x200 >> 1] = idscs_base[0x200 >> 1] & 0xfdff;		\
        udelay(20);								\
        idscs_base[0x200 >> 1] = idscs_base[0x200 >> 1] | 0x0200;		\
	udelay(100);								\
}while(0)
#endif

static unsigned char *BASE_ADDR = 0x10200000;
static unsigned char get_desc_cmd[] = { 0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x12, 0x00 };
static unsigned char dev_descriptor[18] = { 0 };

s32 write_regs16(u32 address, u16 val)
{
	*((volatile u16 *) (address)) = val;

	return 0;
}
u16 read_regs16(u32 address, u16 *val)
{
	volatile u16 tmp = 0;

	tmp = *((volatile u16 *) (address));

	if ((void*)0 != val) *val = tmp;

	return tmp;
}
void get_usb_bus(void)
{
	write_regs16(0x101e4008, read_regs16(0x101e4008, (void*)0) & ~0x08);
	write_regs16(0x101e4004, read_regs16(0x101e4004, (void*)0) |  0x08);
	write_regs16(0x101e4000, read_regs16(0x101e4000, (void*)0) & ~0x08);
	write_regs16(0x10171400, read_regs16(0x10171400, (void*)0) |  0x02);
}
void writeUU16(unsigned int ahb_addr, unsigned short write_value)
{
    volatile unsigned short *pAddr = (unsigned short *)ahb_addr;
    *pAddr = write_value;
    return;
};

unsigned char readU8(unsigned int regaddr)
{
	char data=*(volatile char *)regaddr;
		
	return(data);
}
unsigned short readU16(unsigned int addr)
{
	unsigned char dat1,dat2;
	unsigned short dat;
	dat1 = readU8(addr);
	dat2 = readU8(addr + 1);
	dat = (dat2 << 8) | dat1;
	return dat;
}

static void _sleep(unsigned int loopi)
{
	unsigned int loopk;	
	//    unsigned int loopm;
	unsigned int loopx=loopi*100;

	for(loopk=0;loopk<=loopx;loopk++)   
	{

		loopk=loopk+44323;
		loopk=loopk-44323;

		 for(loopk=0;loopk<=loopx;loopk++)   
		{
		   
		}
		if(loopk==loopx)
			printf("delay==0X%X  \n",loopx); 	
	}	  	
}
void usb_config(void)
{
	int addr=0;
	int temp;

	// config the GPIO3 register,0 output
	//config software mode
	addr= 0x101E4008;
	temp=readU16(addr);
	temp=temp & 0xFFF7;
	writeUU16(addr,temp);

	//config output direction
	addr= 0x101E4004;
	temp=readU16(addr);
	temp=temp | 0x0008;
	writeUU16(addr,temp);

	//config the output value '0'
	addr= 0x101E4000;
	temp=readU16(addr);
	temp=temp & 0xFFF7;
	writeUU16(addr,temp);

	// config the PIN MUX register
	addr=0x10171400;
	temp=readU16(addr);
	temp=temp | 0x0002;
	writeUU16(addr,temp);

	// software Reset USB
	addr=0x10171200;
	temp=readU16(addr);
	temp=temp & 0xFDFF;

	writeUU16(addr,temp);
	_sleep(20);
	temp=readU16(addr);
	temp=temp | 0x0200;
	writeUU16(addr,temp);

	_sleep(50000);     
}
/* macro to read the 32 bit timer */
#define READ_TIMER 		(orion_apb_read32(CFG_TIMERBASE+4))

static ulong timestamp;
static ulong lastdec;

static void reset_timer_masked(void);
static ulong get_timer_masked(void);

/* ORION: only support 8/16bit write to APB bus */
static void orion_apb_write32(ulong addr, ulong data)
{
	*(volatile ushort *) (addr + 0) = (ushort) (data);
	*(volatile ushort *) (addr + 2) = (ushort) (data >> 16);
}

static ulong orion_apb_read32(ulong addr)
{
	ulong data;
	data = *(volatile ushort *) (addr + 0);
	data |= (*(volatile ushort *) (addr + 2) << 16);
	return data;
}

/* 
 * nothing really to do with interrupts, 
 * just starts up a counter. 
 */
int timer_init(void)
{
	orion_apb_write32((CFG_TIMERBASE + 0), CFG_TIMER_RELOAD); /* TimerLoad */
	orion_apb_write32((CFG_TIMERBASE + 4), CFG_TIMER_RELOAD); /* TimerValue */
	orion_apb_write32((CFG_TIMERBASE + 8), 0x1);

	/* init the timestamp and lastdec value */
	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */

void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

static void set_timer(ulong t)
{
	timestamp = t;
}

/* 
 * delay x useconds AND perserve advance timstamp value.
 */
void udelay(unsigned long usec)
{
	ulong tmo, tmp;

	tmo = usec * (CFG_HZ / (1000 * 1000));

	tmp = get_timer(0);	/* get current timestamp */
	if ((tmo + tmp + 1) < tmp)	/* if setting this fordward will roll time stamp */
		reset_timer_masked();	/* reset "advancing" timestamp to 0, set lastdec value */
	else
		tmo += tmp;	/* else, set advancing stamp wake up time */

	while (get_timer_masked() < tmo)	/* loop till event */
		 /*NOP*/;
}

static void reset_timer_masked(void)
{
	/* reset time */
	lastdec = READ_TIMER;	/* capure current decrementer value time */
	timestamp = 0;		/* start "advancing" time stamp from 0 */
}

static ulong get_timer_masked(void)
{
	ulong now = READ_TIMER;	/* current tick value */

	if (lastdec >= now) {	/* normal mode (non roll) */
		/* normal mode */
		timestamp += lastdec - now;	/* move stamp fordward with absoulte diff ticks */
	}
	else {	/* we have overflow of the count down timer */
		/* nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"...it could also roll and cause problems.
		 */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}

/* waits specified delay value and resets timestamp */
static void udelay_masked(unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	tmo = usec * (CFG_HZ / (1000 * 1000));

	endtime = get_timer_masked() + tmo;

	do {
		ulong now = get_timer_masked();
		diff = endtime - now;
	} while (diff >= 0);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
static unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
static ulong get_tbclk(void)
{
	ulong tbclk;

	tbclk = CFG_HZ;
	return tbclk;
}
unsigned char MGC_HdrcReadUlpiReg(unsigned char* pBase, unsigned char bAddr, unsigned char* pbData) 
{
    unsigned char bCtl = 0;

    /* ensure not powered down */
    if(MGC_Read8(pBase, 0x01) & 0x01) {
		return -1;
    }

    /* polled */
    MGC_Write8(pBase, 0x75, bAddr);
    MGC_Write8(pBase, 0x76, 0x04 | 0x01);
    while(!(0x02 & bCtl)) {
		bCtl = MGC_Read8(pBase, 0x76);
    }
	
    *pbData = MGC_Read8(pBase, 0x74);
    MGC_Write8(pBase, 0x76, 0);

    return 0;
}

unsigned char MGC_HdrcWriteUlpiReg(unsigned char* pBase, unsigned char bAddr, unsigned char bData) 
{
    unsigned char bCtl = 0;

printf(" I'm here ...... %s, %d. \n", __FUNCTION__, __LINE__);
    /* ensure not powered down */
    if(MGC_Read8(pBase, 0x01) & 0x01) {
		return -1;
    }
printf(" I'm here ...... %s, %d. \n", __FUNCTION__, __LINE__);

    /* polled */
    MGC_Write8(pBase, 0x75, bAddr);
    MGC_Write8(pBase, 0x74, bData);
    MGC_Write8(pBase, 0x76, 0x01);
printf(" I'm here ...... %s, %d. \n", __FUNCTION__, __LINE__);

    while(!(0x02 & bCtl)) {
printf(" I'm here ...... %s, %d. \n", __FUNCTION__, __LINE__);
		bCtl = MGC_Read8(pBase, 0x76);
    }

    MGC_Write8(pBase, 0x76, 0);
printf(" I'm here ...... %s, %d. \n", __FUNCTION__, __LINE__);

    return -1;
}


static void MGC_Ulpi_sta_read(unsigned char* pThis)
{
	unsigned int tmp = 0, addr = 0;

	printf("Enter %s:%d ... ... \n", __FUNCTION__, __LINE__);
	MGC_Write8(pThis, 0x70, 0x03);
	printf(" DELME... ... \n");
    
       addr = ULPI_OTG_CTL;
       MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
       printf (" **0X%X read from ULPI ULPI_OTG_CTL reg(0X%X)\n", tmp,addr);
       
       addr = ULPI_FUN_CTL;
       MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
       printf (" **0X%X read from ULPI ULPI_FUN_CTL reg(0X%X)\n", tmp,addr);
       
       addr = ULPI_IF_CTL;
       MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
       printf (" **0X%X read from ULPI ULPI_IF_CTL reg(0X%X)\n", tmp,addr);
       
       addr = ULPI_DEBUG;
       MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
       printf (" **0X%X read from ULPI ULPI_DEBUG reg(0X%X)\n", tmp,addr);
        
	printf("%s() ok \n", __FUNCTION__);

	return;
}


static void MGC_Ulpi_setting(unsigned char* pThis)
{
	unsigned int tmp = 0, addr = 0;

	printf("Enter %s:%d ... ... \n", __FUNCTION__, __LINE__);
	MGC_Write8(pThis, 0x70, 0x03);
	printf(" DELME... ... \n");
    
	addr = ULPI_FUN_CTL+1; /* write the function RESET regs, or with the write data */
	tmp = 0x20;	
	MGC_HdrcWriteUlpiReg(pThis, addr, tmp);
	udelay(1000);

	tmp = MGC_Read8(pThis, REG_LPI_VBUS);

	/* set the DRV_VBUS_EXT = 1 */
	addr = ULPI_OTG_CTL;
	MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
	tmp |= 0x40;
	MGC_HdrcWriteUlpiReg(pThis, addr, tmp);

	/* set the USEExternalVbusIndicator = 1 */
	MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
	tmp |= 0x80;
	MGC_HdrcWriteUlpiReg(pThis, addr, tmp);

	/* set the DRV_VBUS = 1, Turn on the PHY led */
	MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
	tmp |= 0x20;
	MGC_HdrcWriteUlpiReg(pThis, addr, tmp);

	/* set the DPpulldown and Dmpulldown = 11 */
       addr = ULPI_OTG_CTL;
	MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
	tmp |= 0x06;
	MGC_HdrcWriteUlpiReg(pThis, addr, tmp);
//
//	/* set the TermSelect = 1 */
//	addr = ULPI_FUN_CTL;  
//	MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
//	tmp |= 0x04;
//	MGC_HdrcWriteUlpiReg(pThis, addr, tmp);
//
//	/* set the OP Mode = 0 */
//	MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
//	tmp |= 0xe7;
//	MGC_HdrcWriteUlpiReg(pThis, addr, tmp);
//
//	/* set the XcvrSelect[0] = 1 */
//	MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
//	tmp |= 0x01;
//	MGC_HdrcWriteUlpiReg(pThis, addr, tmp);
//
//	/* set the Indicator Pass Thru  = 1  and set Indicator Complement =0 */
	addr = ULPI_IF_CTL;
	MGC_HdrcReadUlpiReg(pThis, addr, &tmp);
	tmp |= 0x40;
	tmp &= 0xdf;
	MGC_HdrcWriteUlpiReg(pThis, addr, tmp);


	printf("%s() ok \n", __FUNCTION__);

	return;
}

/* Read a word over the MCU interface and check it matches the expected value */
void MGC_CheckReg(unsigned int addr,unsigned int exp, unsigned int be ,unsigned int temp)
{
	unsigned int ret;
	unsigned int readout;
	unsigned int readexp;

	unsigned char b0=0, b1=0, b2=0, b3=0;

	switch(be) {
		case 1: 
		{ 
			b0=1; 
			readout = MGC_Read8(BASE_ADDR, addr);
			break; 
		}
		case 2:	
		{ 
			b1=1; 
			readout = MGC_Read8(BASE_ADDR, addr) << 8;
			break; 
		}
		case 4:	
		{ 
			b2=1; 
			readout = MGC_Read8(BASE_ADDR, addr) << 16;
			break; 
		}
		case 8: 
		{ 
			b3=1; 
			readout = MGC_Read8(BASE_ADDR, addr) << 24;
			break; 
		}
		case 3: 
		{ 
			b0=1; b1=1; 
			readout = MGC_Read16(BASE_ADDR, addr);
			break; 
		}	
		case 12:
		{ 
			b2=1; b3=1; 
			readout = MGC_Read16(BASE_ADDR, addr) << 16;
			break; 
		}
		case 15:
		{ 
			b0=1; b1=1; b2=1; b3=1; 
			readout = MGC_Read32(BASE_ADDR, addr);
			break; 
		}
	}

	ret = readout % 256;
	readexp = exp % 256;

	if (b0==1 && (ret != readexp)) { 
		printf (" **ERROR the 0 byte: 0X%X read from reg (0X%X), (expected 0X%X) \n", ret,addr,exp);
	}

	ret = readout / 256;
	ret = ret % 256;
	readexp = exp / 256; 
	readexp = readexp % 256; 

	if (b1==1 && (ret != readexp)) { 
		printf (" **ERROR the 1 byte: 0X%X read from reg (0X%X), (expected 0X%X) \n", ret,addr,exp);
	}

	ret = readout / 65536;
	ret = ret % 256;
	readexp = exp / 65536; 
	readexp = readexp % 256; 

	if (b2==1 && (ret != readexp)) { 
		printf (" **ERROR the 2 byte: 0X%X read from reg(0X%X), (expected 0X%X) \n", ret,addr,exp);
	}

	ret = readout / 0x1000000;
	ret = ret % 256;
	readexp = exp / 0x1000000; 
	readexp = readexp % 256;   


	if (b3==1 && (ret != readexp)) { 
		printf (" **ERROR the 3 byte: 0X%X read from reg (0X%X), (expected 0X%X) \n", ret,addr,exp);
	}

	return;
}  

void sleep(unsigned int loopi)
{
	unsigned int loopk;	
	unsigned int loopx = loopi * 100;

	for (loopk = 0; loopk <= loopx; loopk++) {
		loopk = loopk + 44323;
		loopk = loopk - 44323;
		for (loopk = 0; loopk <= loopx; loopk++) {
			; /* nothing to do here */
		}
	}

	return;
}

void MGC_CheckIntr(int intr_addr, unsigned int intr_data,unsigned int be)
{
	int iloop = 0;
	unsigned int readout = 0; 
                 
	switch(intr_addr)
	{
		case REG_INTRTX:
		{
			do {
				readout = MGC_Read16(BASE_ADDR, intr_addr);
				printf("Read interrupt for (0x%x)\n", intr_addr); 
				sleep(1000000);
			} while(readout==0x0 && iloop++ < 0x5);

			if (readout == 0x0) {
				printf(" ERROR : No interrupt for (0x%x)\n", intr_addr); 
				return;
			}
			else {
				if(readout == intr_data) {
					readout = intr_data;
					printf(" Success :(0x%x)interrupt for (0x%x) detected \n",readout,intr_addr);
				}
				else {
					printf(" ERROR : interrupt data is wrong for (0x%x), Expected:0x%x ,Readout :0x%x \n", intr_addr, intr_data, readout); 
					return;
				}

				switch (readout & 0xffff) { 
					case INTR_EP0: printf ("  EP0 interrupt   \n");			break;
					case INTR_EP1: printf ( "  EP1 Tx interrupt   \n");     break;
					case INTR_EP2: printf ( "  EP2 Tx interrupt   \n");     break;
					case INTR_EP3: printf ( "  EP3 Tx interrupt   \n");     break;
					case INTR_EP4: printf ( "  EP4 Tx interrupt   \n");     break;
					case INTR_EP5: printf ( "  EP5 Tx interrupt   \n");     break;
					case INTR_EP6: printf ( "  EP6 Tx interrupt   \n");     break;
					case INTR_EP7: printf ( "  EP7 Tx interrupt   \n");     break;
					case INTR_EP8: printf ( "  EP8 Tx interrupt   \n");     break;
					case INTR_EP9: printf ( "  EP9 Tx interrupt   \n");     break;
					case INTR_EP10: printf ( "  EP10 Tx interrupt   \n");   break;
					case INTR_EP11: printf ( "  EP11 Tx interrupt   \n");   break;
					case INTR_EP12: printf ( "  EP12 Tx interrupt   \n");   break;
					case INTR_EP13: printf ( "  EP13 Tx interrupt   \n");   break;
					case INTR_EP14: printf ( "  EP14 Tx interrupt   \n");   break;
					case INTR_EP15: printf ( "  EP15 Tx interrupt   \n");   break;

					default: printf ( "  Multiple Tx interrupts   \n");  	break;
				}    
			}
			
			break;
		}
		
		case REG_INTRRX: 
		{
			do {
				readout = MGC_Read16(BASE_ADDR, intr_addr);
				printf("Read interrupt for (0x%x)\n", intr_addr); 
				sleep(1000000);
			} while(readout==0x0 && iloop++ < 0x5);

			if (readout == 0x0) {
				printf(" ERROR : No interrupt for (0x%x)\n", intr_addr); 
				return;
			}
			else {
				if(readout == intr_data) {
					readout=intr_data;
					printf(" Success :(0x%x)interrupt for (0x%x) detected \n",readout, intr_addr);
				}
				else 
				{
					printf(" ERROR : interrupt data is wrong for (0x%x), Expected:0x%x ,Readout :0x%x \n", intr_addr, intr_data, readout); 
					return;
				}

				switch(readout & 0xffff)
				{ 
					case INTR_EP1: printf ( "  EP1 Rx interrupt   \n");     break;
					case INTR_EP2: printf ( "  EP2 Rx interrupt   \n");     break;
					case INTR_EP3: printf ( "  EP3 Rx interrupt   \n");     break;
					case INTR_EP4: printf ( "  EP4 Rx interrupt   \n");     break;
					case INTR_EP5: printf ( "  EP5 Rx interrupt   \n");     break;
					case INTR_EP6: printf ( "  EP6 Rx interrupt   \n");     break;
					case INTR_EP7: printf ( "  EP7 Rx interrupt   \n");     break;
					case INTR_EP8: printf ( "  EP8 Rx interrupt   \n");     break;
					case INTR_EP9: printf ( "  EP9 Rx interrupt   \n");     break;
					case INTR_EP10: printf ( "  EP10 Rx interrupt   \n");   break;
					case INTR_EP11: printf ( "  EP11 Rx interrupt   \n");   break;
					case INTR_EP12: printf ( "  EP12 Rx interrupt   \n");   break;
					case INTR_EP13: printf ( "  EP13 Rx interrupt   \n");   break;
					case INTR_EP14: printf ( "  EP14 Rx interrupt   \n");   break;
					case INTR_EP15: printf ( "  EP15 Rx interrupt   \n");   break;
					default: printf ( "  Multiple Rx interrupts   \n");  	break;
				}
			}

			break;
		}
		case  REG_INTRUSB:
		{
			do {
				readout = MGC_Read8(BASE_ADDR, intr_addr);
				printf("Read interrupt for (0x%x)\n", intr_addr); 
				sleep(1000000);
			} while(readout==0x0 && iloop++ < 0x5000000);

			if (readout==0x0) {
				printf(" ERROR : No interrupt for (0x%x) \n",intr_addr); 
				return;
			}
			else {
				if(readout==intr_data)
				{
					readout=intr_data;
					printf(" Success :(0x%x)interrupt for (0x%x) detected \n",readout, intr_addr);
				}
				else {
					printf(" ERROR : interrupt data is wrong for (0x%x), Expected: 0x%x, Readout: 0x%x. \n", intr_addr, intr_data, readout); 
					return;
				}
				switch(readout & 0xff)
				{ 
					case INTR_SUS:   printf ( "  Suspend interrupt    \n"); 		break;
					case INTR_RES:   printf ( "  Resume interrupt   \n");   		break;
					case INTR_RST:   printf ( "  Reset interrupt   \n");    		break;
					case INTR_SOF:   printf ( "  SOF interrupt   \n");      		break;
					case INTR_CONN:  printf ( "  Connect interrupt   \n");  		break;
					case INTR_DISCON:printf ( "  Disconnect interrupt   \n");      	break;
					case INTR_SREQ:  printf ( "  Session request interrupt   \n");	break;
					case INTR_VBERR: printf ( "  VBus error interrupt   \n");      	break;
					default: 		 printf ( "  Multiple USB interrupts   \n");	break;
				}
			}

			break;
		}       
		case REG_DMA_INTR:
		{
			do {
				readout = MGC_Read16(BASE_ADDR, intr_addr);
				printf("Read interrupt for (0x%x)\n", intr_addr); 
				sleep(1000000);
			}while(readout==0x0 && iloop++ < 0x5);

			if(readout==0x0) {
				printf(" ERROR : No interrupt for (0x%x) \n", intr_addr);
				return;
			}
			else {
				if(readout==intr_data) {
					readout=intr_data;
					printf(" Success :(0x%x)interrupt for (0x%x) detected \n", readout,intr_addr);
				}
				else 
				{
					printf(" ERROR : interrupt data is wrong for (0x%x), Expected:0x%x, Readout: 0x%x \n", intr_addr, intr_data, readout); 
					return;
				}
				printf ( "  DMA interrupt (%x) \n", readout & 0xffff);      

				break;
			}

			break;
		}
	}	

	return;
}

/* Check the WriteCount register and check that the FIFO contents matches sent data */
unsigned char MGC_ReadFIFO_DMA(void)
{
	int i;
	unsigned char rbytes = MGC_Read8(BASE_ADDR, 0x18);
	
	MGC_Write32(BASE_ADDR, 0x208, dev_descriptor); /* setting DMA addr */
	MGC_Write32(BASE_ADDR, 0x20c, rbytes); /* setting byte numbers that will be received */
	MGC_Write16(BASE_ADDR, 0x204, DMACNTRL_ENAB | DMACNTRL_INTE); /* setting CTRL regs */
{
	/* check interrupt for judging whether the DMA was finished or not */
	MGC_CheckIntr (REG_DMA_INTR, 0x01, BE_8_0);

	/* display the device descriptor */
	unsigned int endAddr = MGC_Read32(BASE_ADDR, 0x208); 
	printf("\n(%08x - %08x = %d)[", endAddr, dev_descriptor, endAddr - (unsigned int)dev_descriptor);
	for (i = 0; i < rbytes; i++) printf(" %02x", dev_descriptor[i]); printf("]\n");
}	

	return rbytes;
}

/* Load data into the endpoint's FIFO over the MCU interface from MCUTxdata */
void MGC_Load_DMA(void)
{
	unsigned int dma_mem_addr = (unsigned int)get_desc_cmd;

	/* setting DMA addrress */
	MGC_Write32(BASE_ADDR,0x208, dma_mem_addr);

	/* setting byte numbers that will be sent */
	MGC_Write32(BASE_ADDR,0x20c, 8);
	MGC_Write16(BASE_ADDR,0x204, DMACNTRL_ENAB | DMACNTRL_INTE | DMACNTRL_DIR);

	/* setting CTRL register */
	MGC_Write16(BASE_ADDR,0x204, DMACNTRL_ENAB | DMACNTRL_INTE | DMACNTRL_DIR);

	return;
} 

/* Standard devicd requests: Get Descriptor */
void usb_get_descriptor2(void)
{
	MGC_Load_DMA();
	MGC_CheckIntr(REG_DMA_INTR, 0x01, BE_8_0);

	MGC_Write8(BASE_ADDR, REG_EP0_CSR, CSR0_SETUP | CSR0_TPR); /* send setup packet */
	printf(" send setup packet \n");  

	MGC_CheckIntr(REG_INTRTX, 0x01, BE_8_0);
	MGC_CheckReg(REG_EP0_CSR, CSR0_NULL, BE_8_0, 0);
	printf(" setup packet has been transmitted \n");  

	MGC_Write8(BASE_ADDR, REG_EP0_CSR, CSR0_REQP);
	printf(" send a IN packet \n");

	MGC_CheckIntr(REG_INTRTX, 0x01,BE_8_0);
	MGC_CheckReg(REG_EP0_CSR, CSR0_RPR, BE_8_0, 0);
	printf(" IN packet is sent (%04x)\n", MGC_Read16(BASE_ADDR, 0x18));

	MGC_ReadFIFO_DMA();
	MGC_Write8(BASE_ADDR, REG_EP0_CSR, CSR0_NULL);
	printf(" read packet is OK \n");
	MGC_Write8(BASE_ADDR, REG_EP0_CSR, CSR0_TPR | CSR0_STATP);

	MGC_CheckIntr(REG_INTRTX, 0x01,BE_8_0);
	MGC_Write8(BASE_ADDR, REG_EP0_CSR, CSR0_NULL);
	
	printf(" OVER \n");

	return;
}

void MGC_simpletest(void)
{
	int tmp = 0;
	
	printf(" [0x62] = %02x \n", MGC_Read8(BASE_ADDR, 0x62));
	printf(" [0x63] = %02x \n", MGC_Read8(BASE_ADDR, 0x63));
	printf(" [0x64] = %02x \n", MGC_Read8(BASE_ADDR, 0x64));
	printf(" [0x65] = %02x \n", MGC_Read8(BASE_ADDR, 0x65));
	printf(" [0x66] = %02x \n", MGC_Read8(BASE_ADDR, 0x66));
	printf(" [0x67] = %02x \n", MGC_Read8(BASE_ADDR, 0x67));

	
	MGC_Write8(BASE_ADDR, 0x0b, 0xff); /* enable all of intrs */
	//printf(" wait a session start \n");
	MGC_CheckIntr(0x0a, 0x40, BE_8_0); /* wait a session start */

	tmp = MGC_Read8(BASE_ADDR, 0x60);
	tmp |= 0x01;
	printf("Write value to address 0x60 value=%02x  \n",tmp);
	MGC_Write8(BASE_ADDR, 0x60, tmp); 
       
	printf(" start a session \n");
       sleep(5000);
	MGC_CheckIntr(0x0a, 0x10, BE_8_0); /* wait a connection */
	MGC_Ulpi_sta_read(BASE_ADDR);

	MGC_CheckReg(REG_DEVCTL, DVC_FSDEV | DVC_VBUS | DVC_HMODE | DVC_SESS, BE_8_0, 0);

	MGC_Write8(BASE_ADDR, REG_POWER, PWR_RST| PWR_HSEN | PWR_SUSE);
	printf(" start a reset \n");

	sleep(5000);

	MGC_Write8(BASE_ADDR, REG_POWER, 0x0);
	printf(" clear the Reset bit \n");

	MGC_CheckReg(REG_DEVCTL, DVC_FSDEV | DVC_VBUS | DVC_HMODE | DVC_SESS, BE_8_0, 0);

	MGC_CheckIntr(REG_INTRUSB, INTR_SOF, BE_8_0); /* wait a SOF intr */
	printf(" a SOF intr is OK \n");

	usb_get_descriptor2();

	return;
}

#define M_REG_INDEX        0x0E   /* 8 bit */
#define M_REG_TXFIFOSZ     0x62    /* 8 bit, TxFIFO size */
#define M_REG_RXFIFOSZ     0x63    /* 8 bit, RxFIFO size */
#define M_REG_TXFIFOADD    0x64    /* 16 bit, TxFIFO address */
#define M_REG_RXFIFOADD    0x66    /* 16 bit, RxFIFO address */
s32 write_regs8(u32 address, u8 val)
{
	*((volatile u8 *) (address)) = val;

	return 0;
}

u8 read_regs8(u32 address, u8 *val)
{
	volatile u8 tmp = 0;

	tmp = *((volatile u8 *) (address));

	if ((void*)0 != val) *val = tmp;

	return tmp;
}
#define MUSBFDRC_ADDR    	0x10200000

#define DRC_IN8(r)       	read_regs8(r, NULL)
#define DRC_OUT8(r,d)    	write_regs8(r, d)
#define DRC_IN16(r)      	read_regs16(r, NULL)
#define DRC_OUT16(r,d)   	write_regs16(r, d)

#define MOTG_REG_READ8(r)	(DRC_IN8(MUSBFDRC_ADDR + (r)))
#define MOTG_REG_WRITE8(r,v)	(DRC_OUT8(MUSBFDRC_ADDR + (r),(v)))
#define MOTG_REG_READ16(r)	(DRC_IN16(MUSBFDRC_ADDR + (r)))
#define MOTG_REG_WRITE16(r,v)	(DRC_OUT16(MUSBFDRC_ADDR + (r),(v)))
#define FIFO_ADDRESS(e)		(MUSBFDRC_ADDR + (e<<2) + M_FIFO_EP0)

#define FIFO_TX 	0           
#define FIFO_RX 	1 
#define	FIFO_DPB 	16

unsigned short MGC_DRC_Build_Table[] =
{
    0, 1, FIFO_TX, 128,  /* 1024-byte TX fifo, endpoint 1 */
    0, 1, FIFO_RX, 128,  /* 1024-byte RX fifo, endpoint 1 */
    0, 2, FIFO_TX, 128,  /* 1024-byte TX fifo, endpoint 2 */
    0, 2, FIFO_RX, 128,  /* 1024-byte RX fifo, endpoint 2 */
    255
};
s32 log2func(s32 x)
{
    s32 i;
    
    for(i = 0; x > 1; i++)
	x = x / 2;

    return i;
}
s32 MGC_Config_A_Fifo(u16 ep, u16 feature, 
		      u16 size, u16 address)
{
    u8  sz;

    /* 
     *  size and address are in the unit of 8 bytes
     *  max packet size, 4096 bytes / 8 = 512
     *  max address, 0xfff8 / 8 = 0x1fff
     */
    if ((size > 512) || (address > 0x1fff))
		return 0;

    MOTG_REG_WRITE8(M_REG_INDEX, ep);
    sz = (feature & FIFO_DPB) | log2func(size);
    
    if(feature & FIFO_RX)
    {
		MOTG_REG_WRITE8(M_REG_RXFIFOSZ, sz);
		MOTG_REG_WRITE16(M_REG_RXFIFOADD, address);
    }
    else
    {
		MOTG_REG_WRITE8(M_REG_TXFIFOSZ, sz);
		MOTG_REG_WRITE16(M_REG_TXFIFOADD, address);
    }

	return (1);
}

s32 MGC_Config_Dynamic_Fifos(void)
{
    s16 i;
    u16 address = 8;   /* in the unit of 8 bytes, 
			  first 64 bytes reserved for endpoint 0 */

    for(i = 0; MGC_DRC_Build_Table[i] != 225; i = i + 4)
    {
		if(MGC_Config_A_Fifo(MGC_DRC_Build_Table[i + 1], 
			  MGC_DRC_Build_Table[i + 2], 
			  MGC_DRC_Build_Table[i + 3], 
			  address) == 0)
	    	return 0;
	
		address += MGC_DRC_Build_Table[i + 3]; 
    }

    return 1;
}
int main(int argc, char *argv[])
{
	printf(" usbtest - test USB host controller of ORION 1.4 \n");
	//printf(" software reset USB and ULPI config\n");
	//MGC_EXTRA_INIT();	
	//MGC_Ulpi_setting(BASE_ADDR);
	//MGC_Ulpi_sta_read(BASE_ADDR);
	//printf(" wait a session start \n");
	//MGC_CheckIntr(0x0a, 0x40, BE_8_0); /* wait a session start */
	printf(" software reset USB and ULPI config\n");
#if 0
	MGC_EXTRA_INIT();	
#else
	get_usb_bus();
	usb_config();
#endif
	MGC_Ulpi_setting(BASE_ADDR);
	MGC_Ulpi_sta_read(BASE_ADDR);

	MGC_simpletest();  
      	return 0;
}













