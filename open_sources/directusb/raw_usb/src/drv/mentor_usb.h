#define M_REG_FADDR        0x00   /* 8 bit */
#define M_REG_POWER        0x01   /* 8 bit */
#define M_REG_INTRTX       0x02  
#define M_REG_INTRRX       0x04
#define M_REG_INTRTXE      0x06  
#define M_REG_INTRRXE      0x08  
#define M_REG_INTRUSB      0x0A   /* 8 bit */
#define M_REG_INTRUSBE     0x0B   /* 8 bit */
#define M_REG_FRAME        0x0C  
#define M_REG_INDEX        0x0E   /* 8 bit */
#define M_REG_TESTMODE     0x0F   /* 8 bit */
#define M_REG_TARGET_FUNCTION_BASE     0x80   /* 8 bit */

/* Indexed registers */

#define M_REG_TXMAXP       0x10
#define M_REG_CSR0         0x12
#define M_REG_TXCSR        0x12
#define M_REG_RXMAXP       0x14
#define M_REG_RXCSR        0x16
#define M_REG_COUNT0       0x18
#define M_REG_RXCOUNT      0x18
#define M_REG_TXTYPE       0x1A    /* 8 bit, only valid in Host mode */
#define M_REG_TYPE0        0x1A    /* 2 bit, only valid in MDRC Host mode */
#define	M_REG_NAKLIMIT0	   0x1B    /* 8 bit, only valid in Host mode */
#define M_REG_TXINTERVAL   0x1B    /* 8 bit, only valid in Host mode */
#define M_REG_RXTYPE       0x1C    /* 8 bit, only valid in Host mode */
#define M_REG_RXINTERVAL   0x1D    /* 8 bit, only valid in Host mode */
#define M_REG_CONFIGDATA   0x1F    /* 8 bit */
#define M_REG_FIFOSIZE     0x1F    /* 8 bit */

/* FIFOs for Endpoints 0 - 15, 32 bit word boundaries */

#define M_FIFO_EP0         0x20

/* Additional Control Registers */

#define	M_REG_DEVCTL	   0x60	   /* 8 bit */    

/* Dynamic FIFO sizing */

#define M_REG_TXFIFOSZ     0x62    /* 8 bit, TxFIFO size */
#define M_REG_RXFIFOSZ     0x63    /* 8 bit, RxFIFO size */
#define M_REG_TXFIFOADD    0x64    /* 16 bit, TxFIFO address */
#define M_REG_RXFIFOADD    0x66    /* 16 bit, RxFIFO address */

/* POWER */
#define M_POWER_ISOUPDATE   0x80 
#define	M_POWER_SOFTCONN    0x40
#define	M_POWER_HSENAB	    0x20
#define	M_POWER_HSMODE	    0x10
#define M_POWER_RESET       0x08
#define M_POWER_RESUME      0x04
#define M_POWER_SUSPENDM    0x02
#define M_POWER_ENSUSPEND   0x01

/* CSR0 in Host mode */
#define	M_CSR0_FLUSHFIFO      0x0100
#define	M_CSR0_H_NAKTIMEOUT   0x0080
#define M_CSR0_H_STATUSPKT    0x0040
#define M_CSR0_H_REQPKT       0x0020
#define M_CSR0_H_ERROR        0x0010
#define M_CSR0_H_SETUPPKT     0x0008
#define M_CSR0_H_RXSTALL      0x0004
#define M_CSR0_TXPKTRDY       0x0002
#define M_CSR0_RXPKTRDY       0x0001


/* TXCSR in Host mode */
#define M_TXCSR_AUTOSET       0x8000
#define M_TXCSR_ISO           0x4000
#define M_TXCSR_MODE          0x2000
#define M_TXCSR_DMAENAB       0x1000
#define M_TXCSR_FRCDATATOG    0x0800
#define M_TXCSR_DMAMODE       0x0400
#define M_TXCSR_H_NAKTIMEOUT  0x0080
#define M_TXCSR_CLRDATATOG    0x0040
#define M_TXCSR_H_RXSTALL     0x0020
#define M_TXCSR_FLUSHFIFO     0x0008
#define M_TXCSR_H_ERROR       0x0004
#define M_TXCSR_FIFONOTEMPTY  0x0002
#define M_TXCSR_TXPKTRDY      0x0001

/* RXCSR in Host mode */
#define M_RXCSR_AUTOCLEAR     0x8000
#define M_RXCSR_DMAENAB       0x2000
#define M_RXCSR_H_AUTOREQ     0x4000
#define M_RXCSR_DISNYET       0x1000
#define M_RXCSR_DMAMODE       0x0800
#define M_RXCSR_INCOMPRX      0x0100
#define M_RXCSR_CLRDATATOG    0x0080
#define M_RXCSR_H_RXSTALL     0x0040
#define M_RXCSR_H_REQPKT      0x0020
#define M_RXCSR_FLUSHFIFO     0x0010
#define M_RXCSR_DATAERROR     0x0008
#define M_RXCSR_H_ERROR       0x0004
#define M_RXCSR_FIFOFULL      0x0002
#define M_RXCSR_RXPKTRDY      0x0001

#define MUSBFDRC_ADDR    			0x10200000
#define DRC_IN8(r)       			*((volatile unsigned char *)(r))
#define DRC_OUT8(r,d)    			(*((volatile unsigned char *)(r)) = d)
#define DRC_IN16(r)       			*((volatile unsigned short *)(r))
#define DRC_OUT16(r,d)    			(*((volatile unsigned short *)(r)) = d)
#define MOTG_REG_READ8(r)			(DRC_IN8(MUSBFDRC_ADDR + (r)))
#define MOTG_REG_WRITE8(r,v)		(DRC_OUT8(MUSBFDRC_ADDR + (r),(v)))
#define MOTG_REG_READ16(r)			(DRC_IN16(MUSBFDRC_ADDR + (r)))
#define MOTG_REG_WRITE16(r,v)		(DRC_OUT16(MUSBFDRC_ADDR + (r),(v)))
#define FIFO_ADDRESS(e)				(MUSBFDRC_ADDR + (e<<2) + M_FIFO_EP0)

#define     FIFO_TX             0           
#define     FIFO_RX             1 
#define		FIFO_DPB			16
u16  MGC_DRC_Build_Table[] =
{
    0, 1, FIFO_TX, 128,  /* 1024-byte TX fifo, endpoint 1 */
    0, 1, FIFO_RX, 128,  /* 1024-byte RX fifo, endpoint 1 */
    0, 2, FIFO_TX, 128,  /* 1024-byte TX fifo, endpoint 2 */
    0, 2, FIFO_RX, 128,  /* 1024-byte RX fifo, endpoint 2 */
    255
};

#define REG_LPI_VBUS     0x70
#define REG_LPI_CARCTRL  0x71
#define REG_LPI_IMASK    0x72
#define REG_LPI_ISTAT    0x73
#define REG_LPI_DATA     0x74
#define REG_LPI_ADDR     0x75
#define REG_LPI_CTRL     0x76
#define REG_LPI_RXDATA   0x77

#define ULPI_FUN_CTL      0x04
#define ULPI_IF_CTL       0x07
#define ULPI_OTG_CTL      0x0A

#define BE_8_0			1

unsigned int  USB_ReadReg(unsigned int addr, unsigned int be);
void  MCU_Write(unsigned int addr,unsigned int ret, unsigned int be);
void enable_fsm(void);
void ulpi_set(void);

