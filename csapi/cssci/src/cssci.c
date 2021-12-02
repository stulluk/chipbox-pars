#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "cssci.h"
#include "../csevt/include/csevt.h"


#define  CSSCI_OBJ_TYPE	'p'
#define  CSSCI_DEV_FILE	"/dev/misc/smartcard"

#define SCI_IOC_MAGIC 'p'
#define SCI_IOC_WARMRESET			_IOW(SCI_IOC_MAGIC, 0x01, int)
#define SCI_IOC_ISPRESENT			_IOR(SCI_IOC_MAGIC, 0x02, int)
#define SCI_IOC_ACTIVE				_IOW(SCI_IOC_MAGIC, 0x03, int)
#define SCI_IOC_DEACTIVE			_IOW(SCI_IOC_MAGIC, 0x04, int)
#define SCI_IOC_CLKSTART			_IOW(SCI_IOC_MAGIC, 0x05, int)
#define SCI_IOC_CLKSTOP				_IOW(SCI_IOC_MAGIC, 0x06, int)
#define SCI_IOC_SET_PROTOCOL			_IOW(SCI_IOC_MAGIC, 0x07, int)
#define SCI_IOC_SET_RXINTTIGGER			_IOW(SCI_IOC_MAGIC, 0x08, int)
#define SCI_IOC_SET_TXINTTIGGER			_IOW(SCI_IOC_MAGIC, 0x09, int)
#define SCI_IOC_SET_ETU				_IOW(SCI_IOC_MAGIC, 0x0a, int)
#define SCI_IOC_SET_SC_CLOCK			_IOW(SCI_IOC_MAGIC, 0x0b, int)
#define SCI_IOC_SET_TXRETRY			_IOW(SCI_IOC_MAGIC, 0x0c, int)
#define SCI_IOC_SET_RXRETRY			_IOW(SCI_IOC_MAGIC, 0x0d, int)
#define SCI_IOC_SET_CHGUARD			_IOW(SCI_IOC_MAGIC, 0x0e, int)
#define SCI_IOC_SET_BLOCKGUARD			_IOW(SCI_IOC_MAGIC, 0x0f, int)
#define SCI_IOC_SET_RXTIMEOUT			_IOW(SCI_IOC_MAGIC, 0x10, int)
#define SCI_IOC_SET_CONVENTION			_IOW(SCI_IOC_MAGIC, 0x11, int)
#define SCI_IOC_SET_PARITY			_IOW(SCI_IOC_MAGIC, 0x12, int)
#define SCI_IOC_GET_RXRDYSIZE			_IOR(SCI_IOC_MAGIC, 0x13, int)
#define SCI_IOC_GET_TXRDYSIZE			_IOR(SCI_IOC_MAGIC, 0x14, int)
#define SCI_IOC_SET_RXFIFO_THRESHOLD		_IOW(SCI_IOC_MAGIC, 0x15, int)
#define SCI_IOC_SET_TXFIFO_THRESHOLD		_IOW(SCI_IOC_MAGIC, 0x16, int)
#define SCI_IOC_GET_RXFIFO_THRESHOLD		_IOR(SCI_IOC_MAGIC, 0x17, int)
#define SCI_IOC_GET_TXFIFO_THRESHOLD		_IOR(SCI_IOC_MAGIC, 0x18, int)
#define SCI_IOC_SET_RX_READREADY_REPORT		_IOW(SCI_IOC_MAGIC, 0x19, int)
#define SCI_IOC_SET_TX_WRITEREADY_REPORT	_IOW(SCI_IOC_MAGIC, 0x1a, int)
#define SCI_IOC_GET_BLOCKGUARD			_IOW(SCI_IOC_MAGIC, 0x1b, int)
#define SCI_IOC_GET_SC_CLOCK			_IOR(SCI_IOC_MAGIC, 0x1c, int)

#define SCI_IOC_RESET				_IOW(SCI_IOC_MAGIC, 0x1d, int)
#define SCI_IOC_GET_TXRETRY			_IOR(SCI_IOC_MAGIC, 0x1e, int)
#define SCI_IOC_GET_CHGUARD			_IOR(SCI_IOC_MAGIC, 0x1f, int)
#define SCI_IOC_GET_RXTIMEOUT			_IOR(SCI_IOC_MAGIC, 0x20, int)
#define SCI_IOC_GET_RXRETRY			_IOR(SCI_IOC_MAGIC, 0x22, int)
#define SCI_IOC_SET_TXRXNAK			_IOR(SCI_IOC_MAGIC, 0x23, int)

#define CALLBACK_FLAG_CARDINS			1
#define CALLBACK_FLAG_CARDREM			2
#define CALLBACK_FLAG_RXTIMEOUT		4
#define CALLBACK_FLAG_ATRRDY			8
#define CALLBACK_FLAG_READRDY			16
#define CALLBACK_FLAG_WRITERDY		32
#define CALLBACK_FLAG_READERROR		64

#define	MHZ		1000000
#define	SCI_F_IREDO_DEFAULT	620
#define	SCI_F_DEFAULT	372
#define	SCI_D_DEFAULT	1
#define	DEFAULT_T			0

#define CSSCI_PB

#ifdef CSSCI_DEBUG
#define debug_printf printf
#else
#define debug_printf(fmt, args...)
#endif


static CSAPI_RESULT CSSCI_T0_ProcessProcedureBytes(CSSCI_HANDLE  Smart_p,
		                                               unsigned char INS,
		                                               unsigned int WriteSize,
		                                               unsigned int ReadSize,
		                                               unsigned char *Buf_p,
		                                               unsigned char *Size_p,
		                                               unsigned int *NextWriteSize_p,
		                                               unsigned int *NextReadSize_p
   											          );
static CSAPI_RESULT CSSCI_T0_Transfer( CSSCI_HANDLE Smart_p,
                                  unsigned char *WriteBuffer_p,
                                  unsigned int NumberToWrite,
                                  unsigned char *ReadBuffer_p,
                                  unsigned int NumberToRead,
                                  unsigned int *NumberWritten_p,
                                  unsigned int *NumberRead_p,
                                  CSSCI_Status *Status
                                );
static CSAPI_RESULT CSSCI_T14_Transfer( CSSCI_HANDLE Smart_p,
                                  unsigned char *WriteBuffer_p,
                                  unsigned int NumberToWrite,
                                  unsigned char *ReadBuffer_p,
                                  unsigned int NumberToRead,
                                  unsigned int *NumberWritten_p,
                                  unsigned int *NumberRead_p
                                );
static CSAPI_RESULT CSSCI_T1_Transfer(CSSCI_HANDLE Smart_p,
                                 											unsigned char *Command_p,
                                 											unsigned int NumberToWrite,
                                 											unsigned int *NumberWritten_p,
                                 											unsigned char *Response_p,
                                 											unsigned int NumberToRead,
                                 											unsigned int *NumberRead_p,
                                 											unsigned char SAD,
                                 											unsigned char DAD );
static void CSSCI_Notify(void *inparam, int fd,int events);

typedef struct 
{
	CSSCI_HANDLE sci_handle;
	void (*callfun)(CSSCI_HANDLE, CSSCI_SCIEVENT *);
	CSSCI_SCIEVENT evt;
}CSSCI_CALLBAK_PARAM;

static CSSCI_CALLBAK_PARAM sci_callback[7];
static unsigned char      Size;
static unsigned char      PB[2];

CSEVT_HANDLE sci_evt;
static int 		register_flag = 0;
static char 	callback_flag = 0;
static char 	rxtimeout_flag = 0;
static char 	atr_readyflag = 0;
static char 	readrdy_flag = 0;
static char 	writerdy_flag = 0;
static char 	readerror_flag = 0;


/* ------ for T1 protocol definitions ------> Start */
#define MAXBLOCKLEN             3 + 254 + 2
#define NextSeq(i)      (i == 1)?0:1

/* Our internal state markers */
#define SMART_T1_OURTX          			0x0001
#define SMART_T1_CHAINING_US    	0x0002
#define SMART_T1_CHAINING_THEM  0x0004
#define SMART_T1_TXWAITING      		0x0008
#define SMART_T1_SRESPEXPECT    	0x0010
#define SMART_T1_VPPHIGHTILNAD  0x0020
#define SMART_T1_GOTNAK         		0x0040
#define SMART_T1_WTX            			0x0080

/* Block types */
#define BLOCK_TYPE_BIT		0xC0
#define S_REQUEST_BLOCK  	0xC0
#define R_BLOCK                 	0x80
#define I_BLOCK_1               	0x00
#define I_BLOCK_2               	0x40

/* In S-block */
#define S_RESYNCH_REQUEST       	0x00
#define S_IFS_REQUEST           		0x01
#define S_ABORT_REQUEST         		0x02
#define S_WTX_REQUEST          		0x03
#define S_VPP_ERROR_RESPONSE	0x24
#define S_RESPONSE_BIT          		0x20
#define S_RESYNCH_RESPONSE      	0xE0

/* I-block */
#define I_CHAINING_BIT	0x20
#define I_SEQUENCE_BIT	0x40

/* R-block */
#define R_EDC_ERROR			0x01
#define R_OTHER_ERROR	0x02

typedef enum {
    T1_R_BLOCK = 0,
    T1_S_REQUEST,
    T1_S_RESPONSE,
    T1_I_BLOCK,
    T1_CORRUPT_BLOCK
} SMART_BlockType_t;

typedef struct {
    unsigned char NAD;
    unsigned char PCB;
    unsigned char LEN;
} SMART_T1Header_t;

typedef struct {
    SMART_T1Header_t Header;
    unsigned char *Buffer;
    union {
        unsigned char LRC;
        unsigned short CRC;
    } EDC_u;
} SMART_T1Block_t;
/* ------ for T1 protocol definitions ------> End */


typedef struct CSSCI_SCINFO_ {
	CSSCI_SCTYPE type;

	/* for ATR */
	unsigned char init_char; 	/*Initial Character, TS */
	unsigned char format_char;	/*Format character (Y0), T0 */
	unsigned char current_protocol;
	unsigned char protocols[4];
	unsigned char TA1;		/*Codes F,D */
	unsigned char TB1;		/*Codes I,P */
	unsigned char TC1;		/*Codes N */
	unsigned char TD1;		/*Y2, T1 */
	unsigned char *hist_char;	/*historical characters */
	unsigned char TCheck;
	unsigned char nATR;
	unsigned char atr[33];
	unsigned char specific_mode;
	unsigned char	specific_type_changable;

	/* for T1 protocol */
	unsigned char IFSC;	/*  Tx max bytes from device to card for T1 */
	unsigned char CWI;	/*  for CWT (character waitting time) */
	unsigned char BWI;	/*  for BWT (block waitting time) */
	unsigned char RC;		/* specifies the error detection code, 1:CRC,0:LRC */
	unsigned char TxMaxInfoSize;	/* max bytes send to card one transmittal from interface device */
	unsigned char OurSequence;		/* the sequence no. will send to card */
	unsigned char TheirSequence;	/* the last sequence no. used by card */
	unsigned char TxCount;
	unsigned char RxCount;
	unsigned char FirstBlock;	/* bool type */
	unsigned char Aborting;		/* bool type */
	unsigned int  State;		/* if chaining mode to transmit */

} CSSCI_SCINFO;

typedef struct tagCSSCI_OBJ {
	char obj_type;
	int dev_fd;
	int errcode;
	CSSCI_SCINFO sciinfo;
} CSSCI_OBJ;

static CSSCI_OBJ cssci_obj = { 0, -1, 0, {0, 0, 0, 0, {DEFAULT_T}, 0, 0, 0, 0, NULL, 0, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }};
static unsigned char Parity_Value = 0, Is_Error_Detect = 0;

static char *sciout_errstr[] = {
	"SMARTCARD: no error",
	"SMARTCARD: open failed",
	"SMARTCARD: close failed",
	"SMARTCARD: ioctl failed",
	"SMARTCARD: invalid arguments",
	"SMARTCARD: invalid handle",
	"SMARTCARD: unknown device name",
	"SMARTCARD: device busy",
	"SMARTCARD: read failed",
	"SMARTCARD: write failed",
	"SMARTCARD: card active error",
	"SMARTCARD: card deactive error",
	"SMARTCARD: set baudrate error",
	"SMARTCARD: set block guard delay error",
	"SMARTCARD: get block guard delay error",
	"SMARTCARD: set convention error",
	"SMARTCARD: set parity error detect error",
	"SMARTCARD: set parity value error",
	"SMARTCARD: set transmit retry times error",
	"SMARTCARD: set receive retry timeout error",
	"SMARTCARD: set receive timeout error",
	"SMARTCARD: set guard delay error",
	"SMARTCARD: set protocol error",		/* Added */
	"SMARTCARD: card reset error",
	"SMARTCARD: get baudrate error",
	"SMARTCARD: get transmit retry times error",
	"SMARTCARD: get receive retry timeout error",
	"SMARTCARD: get receive timeout error",
	"SMARTCARD: get guard delay error",
	"SMARTCARD: get protocol error",
	"SMARTCARD: invalid parameters",
	"SMARTCARD: get write size error",
	"SMARTCARD: get read size error",
	"SMARTCARD: register callback error",
	"SMARTCARD: device not opened error",
	"SMARTCARD: set read threshold error",
	"SMARTCARD: set write threshold error",
	"SMARTCARD: get read threshold error",
	"SMARTCARD: get write threshold error",
	"SMARTCARD: read fifo report error",
	"SMARTCARD: write fifo report error",
	"SMARTCARD: set clock error",
	"SMARTCARD: get clock error",
	"SMARTCARD: read data time out",
	"SMARTCARD: write data time out",
	"SMARTCARD: write T1 block data time out",
	"SMARTCARD: T1 resyn retry error",
	"SMARTCARD: no card",
	"SMARTCARD: not ATR information received"
};


static int _cssci_read(CSSCI_HANDLE handle, unsigned char *buffer, unsigned int len)
{
	int retval = 0;
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;

	retval = read(dev_obj->dev_fd, buffer, len);
	if (retval < 0){
		readerror_flag = 1;
		if( callback_flag & CALLBACK_FLAG_READERROR)
			CSSCI_Notify(&sci_callback, dev_obj->dev_fd, CSSCI_READERROR);
		dev_obj->errcode = SCI_ERROR_READ;
		return -1;
	}
	return retval;
}

static int _cssci_write(CSSCI_HANDLE handle, unsigned char *buffer, unsigned int len)
{
	int retval = 0;
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;

	retval = write(dev_obj->dev_fd, buffer, len);
	if (retval < 0){
		dev_obj->errcode = SCI_ERROR_WRITE;
		return -1;
	}
	return retval;
}

/* Return: 1: error; 0: get right 
 * is_cardin: 0: no card; 1: card in
*/
static int _cssci_is_cardin(int fdes, char *is_cardin) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_ISPRESENT, is_cardin);
	if(retval < 0) {
		printf(" Smartcard  Error(I/O Get Card in/out status) : %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_card_active(int fdes)
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_ACTIVE);
	if (retval < 0) {
		debug_printf(" SCI ACTIVATE Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_card_reset(int fdes)
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_WARMRESET);
	if (retval < 0) {
		debug_printf(" SCI Reset Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_card_deactive(int fdes)
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_DEACTIVE);
	if (retval < 0) {
		debug_printf(" SCI ACTIVATE Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}
  
static int _cssci_set_txretry(int fdes, unsigned char retrytimes)
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_SET_TXRETRY,retrytimes);
	if (retval < 0) {
		debug_printf(" SCI Set Tx retry Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_get_txretry(int fdes, unsigned char *retrytimes)
{

	int retval;
	retval = ioctl(fdes, SCI_IOC_GET_TXRETRY,retrytimes);
	if (retval < 0) {
		debug_printf(" SCI Get Tx retry Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_set_rxretry(int fdes, unsigned char retrytimes)
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_SET_RXRETRY,retrytimes);
	if (retval < 0) {
		debug_printf(" SCI Set Rx retry Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_get_rxretry(int fdes, unsigned char *retrytimes)
{

	int retval;
	retval = ioctl(fdes, SCI_IOC_GET_RXRETRY,retrytimes);
	if (retval < 0) {
		debug_printf(" SCI Get Rx retry Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_set_chguard(int fdes, unsigned char chguard)
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_SET_CHGUARD, chguard);
	if (retval < 0) {
		debug_printf(" SCI Set Character Guard Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_get_chguard(int fdes, unsigned char *chguard)
{

	int retval;
	retval = ioctl(fdes, SCI_IOC_GET_CHGUARD, chguard);
	if (retval < 0) {
		debug_printf(" SCI Get Character Guard Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_set_blockguard(int fdes, unsigned char blockguard)
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_SET_BLOCKGUARD, blockguard);
	if (retval < 0) {
		debug_printf(" SCI Set Block Guard Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_get_blockguard(int fdes, unsigned char *blockguard)
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_GET_BLOCKGUARD, blockguard);
	if (retval < 0) {
		debug_printf(" SCI Get Block Guard Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_set_rxtimeout(int fdes, unsigned short timeout)
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_SET_RXTIMEOUT, timeout);
	if (retval < 0) {
		debug_printf(" SCI SETRXTIMEOUT Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_get_rxtimeout(int fdes, unsigned short *timeout)
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_GET_RXTIMEOUT, timeout);
	if (retval < 0) {
		debug_printf(" SCI GETRXTIMEOUT Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

/*0: direct convention 1: inverse convention */
static int _cssci_set_convention(int fdes, unsigned char convention) 
{
	int retval;
	
	retval = ioctl(fdes, SCI_IOC_SET_CONVENTION, convention);
	if (retval < 0) {
		debug_printf(" SCI Set Convention Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}


/*0: OFF(disable) 1: ON(enable) enable/disable report rxfifo reached threshold */
static int _cssci_rxfifo_report(int fdes, unsigned char ONorOFF) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_SET_RX_READREADY_REPORT, ONorOFF);
	if (retval < 0) {
		debug_printf(" SCI Set RX READREADY REPORT Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

/*0: OFF(disable) 1: ON(enable) enable/disable report txfifo reached threshold */
static int _cssci_txfifo_report(int fdes, unsigned char ONorOFF) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_SET_TX_WRITEREADY_REPORT, ONorOFF);
	if (retval < 0) {
		debug_printf(" SCI Set TX write ready report Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_set_rxfifo_threshold(int fdes, unsigned int rxbuffer_threshold) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_SET_RXFIFO_THRESHOLD, rxbuffer_threshold);
	if (retval < 0) {
		debug_printf(" SCI Set RXFIFO threshold Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_set_txfifo_threshold(int fdes, unsigned int txbuffer_threshold) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_SET_TXFIFO_THRESHOLD, txbuffer_threshold);
	if (retval < 0) {
		debug_printf(" SCI Set TXFIFO threshold Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_get_rxfifo_threshold(int fdes, unsigned int *rxbuffer_threshold_p) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_GET_RXFIFO_THRESHOLD, rxbuffer_threshold_p);
	if (retval < 0) {
		debug_printf(" SCI Get RXFIFO threshold Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_get_txfifo_threshold(int fdes, unsigned int *txbuffer_threshold_p) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_GET_TXFIFO_THRESHOLD, txbuffer_threshold_p);
	if (retval < 0) {
		debug_printf(" SCI Get TXFIFO threshold Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_get_txrdy_size(int fdes, unsigned char *txbuffer_size) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_GET_TXRDYSIZE, txbuffer_size);
	if (retval < 0) {
		debug_printf(" SCI Get TXFIFO Size Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_get_rxrdy_size(int fdes, unsigned char *rxbuffer_size) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_GET_RXRDYSIZE, rxbuffer_size);
	if (retval < 0) {
		debug_printf(" SCI Get RXFIFO Size Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_set_parity(int fdes, unsigned char parity) 
{
	int retval;
	
	retval = ioctl(fdes, SCI_IOC_SET_PARITY, parity);
	if (retval < 0) {
		debug_printf(" SCI Set Parity Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_set_parity_error_detect(int fdes, unsigned char value) /*value: 0 disable, 1 enable */
{
	int retval;
	
	retval = ioctl(fdes, SCI_IOC_SET_TXRXNAK, value);
	if (retval < 0) {
		debug_printf(" SCI Set Parity Error Detect(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

static int _cssci_set_etu(int fdes, unsigned int F, unsigned int D)
{
	int retval;
	struct CSSCI_BAUD_t{
		unsigned int F;
		unsigned int D;
	}sci_etu;
	sci_etu.F = F;
	sci_etu.D = D;
	retval = ioctl(fdes, SCI_IOC_SET_ETU, &sci_etu);
	if (retval < 0) {
		debug_printf(" SCI Set ETU Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

/*f is smartcard clock frequency (Hz)*/
static int _cssci_set_sc_clock(int fdes, unsigned int f) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_SET_SC_CLOCK, f);
	if (retval < 0) {
		debug_printf(" SCI Set Smart Card Clock Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

/*f is a pointer of smartcard clock frequency value(Hz)*/
static int _cssci_get_sc_clock(int fdes, unsigned int *f) 
{
	int retval;
	retval = ioctl(fdes, SCI_IOC_GET_SC_CLOCK, f);
	if (retval < 0) {
		debug_printf(" SCI Get Smart Card Clock Error(ioctl): %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

/* --------------- these functions will be used by T1 transfer --------------> Start */

static void generate_crc(unsigned char *Buffer_p, unsigned int Count, unsigned short *CRC_p)
{
    unsigned int i;

    /* Initialise the CRC */
    *CRC_p = 0;

    /* Then work through the buffer, updating the CRC accordingly as we go */
    for (i = 0; i < Count; i++)
    {
        *CRC_p ^= Buffer_p[i];
        *CRC_p ^= (unsigned char)(*CRC_p & 0xff) >> 4;
        *CRC_p ^= (*CRC_p << 8) << 4;
        *CRC_p ^= ((*CRC_p & 0xff) << 4) << 1;
    }
}

static void generate_lrc(unsigned char *Buffer_p, unsigned int Count, unsigned char *LRC_p)
{
    unsigned int i;

    *LRC_p = 0;
    for (i = 0; i < Count; i++)
    {
        *LRC_p = *LRC_p ^ Buffer_p[i];
    }
}

static void make_u8_from_block(SMART_T1Block_t *Block_p, unsigned char CRC, unsigned char *Buffer, unsigned int *Length)
{
    Buffer[0] = Block_p->Header.NAD;
    Buffer[1] = Block_p->Header.PCB;
    Buffer[2] = Block_p->Header.LEN;

    if (Block_p->Buffer != NULL)
        memcpy(&Buffer[3], Block_p->Buffer, Buffer[2]);

    /* Might not be set yet, but copy it anyway */
    if (CRC)
    {
        /* MSB-first */
        Buffer[3 + Buffer[2]] = (unsigned char)((Block_p->EDC_u.CRC & 0xff00) >> 8);
        Buffer[4 + Buffer[2]] = (unsigned char)(Block_p->EDC_u.CRC & 0x00ff);
        *Length = 3 + Buffer[2] + 2;
    }
    else
    {
        Buffer[3 + Buffer[2]] = Block_p->EDC_u.LRC;
        *Length = 3 + Buffer[2] + 1;
    }
}

static void calculate_edc(CSSCI_HANDLE Smart_p, SMART_T1Block_t *Block_p)
{
    unsigned char BlockBuffer[MAXBLOCKLEN];
    unsigned int Length;

	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) Smart_p;
	CSSCI_SCINFO *T1Details_p = &(dev_obj->sciinfo);		/* Shortcut for accessing details in Smart_p */

    /* Calculate EDC */
    if (T1Details_p->RC == 1)
    {
        make_u8_from_block(Block_p, 1, BlockBuffer, &Length);
        generate_crc(BlockBuffer, Length - 2, &Block_p->EDC_u.CRC);
    }
    else
    {
        make_u8_from_block(Block_p, 0, BlockBuffer, &Length);
        generate_lrc(BlockBuffer, Length - 1, &Block_p->EDC_u.LRC);
    }
}

static CSAPI_RESULT smart_t1_write_block(CSSCI_HANDLE Smart_p, SMART_T1Block_t *Block_p)
{
    CSAPI_RESULT error = CSAPI_SUCCEED;
    unsigned int Written, Length;
    unsigned char BlockBuffer[MAXBLOCKLEN];

	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) Smart_p;
	CSSCI_SCINFO *T1Details_p = &(dev_obj->sciinfo);		/* Shortcut for accessing details in Smart_p */

    make_u8_from_block(Block_p, (T1Details_p->RC == 1)?1:0, BlockBuffer, &Length);

    error = CSSCI_Write( Smart_p, BlockBuffer, Length, &Written );

    return error;
}

static CSAPI_RESULT smart_t1_read_block(CSSCI_HANDLE Smart_p, SMART_T1Block_t *Block_p)
{
    CSAPI_RESULT error = CSAPI_SUCCEED;
    unsigned int Read;

	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) Smart_p;
	CSSCI_SCINFO *T1Details_p = &(dev_obj->sciinfo);		/* Shortcut for accessing details in Smart_p */

    /* Wait for (and read) the header, starting with NAD */
    error = CSSCI_Read( Smart_p, &Block_p->Header.NAD, 1, &Read );

    if (error == CSAPI_SUCCEED)
    {
        /* Read PCB */
		 error = CSSCI_Read( Smart_p, &Block_p->Header.PCB, 1, &Read );

        /* Read LEN */
        if (error == CSAPI_SUCCEED)
        {
            error = CSSCI_Read(Smart_p, &Block_p->Header.LEN, 1, &Read );
        }

        /* Read any info field */
        if (error == CSAPI_SUCCEED)
        {
            error = CSSCI_Read(Smart_p, Block_p->Buffer, Block_p->Header.LEN, &Read );
        }

        /* Read epilogue field */
        if (error == CSAPI_SUCCEED)
        {
            if (T1Details_p->RC == 1)
            {
                error = CSSCI_Read(Smart_p, (unsigned char *)&Block_p->EDC_u.CRC, 2, &Read);
            }
            else
            {
                error = CSSCI_Read(Smart_p, &Block_p->EDC_u.LRC, 1, &Read);
            }
        }

    }
    else
    {
        error = SCI_ERROR_RX_TIMEOUT;
    }
    
    return error;
}

static SMART_BlockType_t smart_get_block_type(unsigned char PCB)
{
    SMART_BlockType_t ThisBlockType;

    /* See what we got back */
    switch (PCB & BLOCK_TYPE_BIT)
    {
        case S_REQUEST_BLOCK:   /* S-block */
                                ThisBlockType = T1_S_REQUEST;
                                break;
        case R_BLOCK:           /* R-block */
                                ThisBlockType = T1_R_BLOCK;
                                break;
        case I_BLOCK_1:         /* Two possible forms of i-block */
        case I_BLOCK_2:         ThisBlockType = T1_I_BLOCK;
                                break;
        default:                ThisBlockType = T1_CORRUPT_BLOCK;
                                break;
    }

    return ThisBlockType;
}

static CSAPI_RESULT smart_t1_resync(CSSCI_HANDLE *Smart_p, unsigned char SAD, unsigned char DAD)
{
    CSAPI_RESULT error = CSAPI_SUCCEED;
    SMART_T1Block_t TxBlock, RxBlock;
    unsigned char Valid;
    unsigned char LRC = 0, Count = 0;
    unsigned short CRC = 0;

    unsigned char BlockBuffer[MAXBLOCKLEN];
    unsigned int Length;

	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) Smart_p;
	CSSCI_SCINFO *T1Details_p = &(dev_obj->sciinfo);		/* Shortcut for accessing details in Smart_p */

    /* Send sync s.block */
    TxBlock.Header.PCB = S_REQUEST_BLOCK | S_RESYNCH_REQUEST;
    TxBlock.Header.NAD = SAD | (DAD << 4);
    TxBlock.Header.LEN = 0;
    TxBlock.Buffer = NULL;
    calculate_edc(Smart_p, &TxBlock);

    /* Send until we get a valid response, or until we've tried 3 times.
     * No need to assign RxBlock a buffer, since resync-responses don't
     * have an info field.
     */
    Count = 0;
    do
    {
        error = smart_t1_write_block(Smart_p, &TxBlock);

        error = smart_t1_read_block(Smart_p, &RxBlock);

        /* Calculate EDC */
        Valid = 0;
        if (error == CSAPI_SUCCEED)
        {
            if (T1Details_p->RC == 1)
            {
                make_u8_from_block(&RxBlock, 1, BlockBuffer, &Length);
                generate_crc(BlockBuffer, Length - 2, &CRC);
                if (CRC == RxBlock.EDC_u.CRC)
                    Valid = 1;
            }
            else
            {
                make_u8_from_block(&RxBlock, 0, BlockBuffer, &Length);
                generate_lrc(BlockBuffer, Length - 1, &LRC);
                if (LRC == RxBlock.EDC_u.LRC)
                    Valid = 1;
            }
        }

        Count++;
    } while ((Count < 3) &&
             ((RxBlock.Header.PCB != S_RESYNCH_RESPONSE) || (Valid == 0))
            );

    if ((Valid == 0) || (RxBlock.Header.PCB != S_RESYNCH_RESPONSE))
    {
        /* stsmart_no_answer would maybe also fit, apart from possibility
         * card is transmitting but EDC incorrect
         */
        error = SCI_ERROR_BLOCK_RETRIES;
    }

    /* Reset sequence numbers */
    T1Details_p->TheirSequence = 1;  /* So we're expecting 0 next block...   */
    T1Details_p->OurSequence = 0;    /* And sending 0 as our first S(N)      */
    T1Details_p->FirstBlock = 1;
    T1Details_p->TxCount = 0;

    return error;
}

static unsigned char smart_process_s_request(CSSCI_HANDLE Smart_p, SMART_T1Block_t *RxBlock_p)
{
    SMART_T1Block_t TxBlock;
    unsigned char ReceiveDone = 0, AbortError = 0;

	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) Smart_p;
	CSSCI_SCINFO *T1Details_p = &(dev_obj->sciinfo);		/* Shortcut for accessing details in Smart_p */

    if ((RxBlock_p->Header.PCB & S_VPP_ERROR_RESPONSE) == S_VPP_ERROR_RESPONSE)
    {
        ReceiveDone = 1;
    }
    else if ((RxBlock_p->Header.PCB & S_WTX_REQUEST) == S_WTX_REQUEST)
    {
        /* Multiply BWT by buffer[0], for this block only */
        T1Details_p->BWI = RxBlock_p->Buffer[0];
        ReceiveDone = 0;
    }
    else if ((RxBlock_p->Header.PCB & S_ABORT_REQUEST) == S_ABORT_REQUEST)
    {
        /* Abort any pending IO operations with the card. */
        usleep(500000);	/* FIX Me !!! */
		 AbortError = 1;
        ReceiveDone = 1;
    }
    else if ((RxBlock_p->Header.PCB & S_IFS_REQUEST) == S_IFS_REQUEST)
    {
        /* Change Txmaxinfosize */
        T1Details_p->TxMaxInfoSize = RxBlock_p->Buffer[0];
    }

    /* Those S-responses with an info field are supposed to match the
     * request. Which is handy.
     */
    TxBlock = *RxBlock_p;
    TxBlock.Header.PCB |= S_RESPONSE_BIT;
    calculate_edc(Smart_p, &TxBlock);
    smart_t1_write_block(Smart_p, &TxBlock);

    /* If we're acking an abort request, we need to read (and possibly discard)
     * the block the card sends in return.
     */
    if ( AbortError == 1 )
    {
        unsigned char BlockBuffer[MAXBLOCKLEN];
        SMART_T1Block_t RxBlock;
        unsigned char Done = 0;
        CSAPI_RESULT error = CSAPI_SUCCEED;

        T1Details_p->TxCount = 0;
        RxBlock.Buffer = &BlockBuffer[0];
        do
        {
            unsigned char Sblock;

            /* Read the block tranferring right to send */
            error = smart_t1_read_block(Smart_p, &RxBlock);

            /* Check type */
            Sblock = ((RxBlock.Header.PCB & BLOCK_TYPE_BIT) == S_REQUEST_BLOCK)?1:0;

            /* S-response can be considered acknowledged (in this case) if the
             * card does *not* send another abort request.
             */
            if ((Sblock == 0) ||
                ((Sblock == 1) && ((RxBlock.Header.PCB & S_ABORT_REQUEST) == 0)))
            {
                Done = 1;
            }
            else
            {
                /* Else, retransmit 3 times, before doing resync */
                T1Details_p->TxCount++;
                if (T1Details_p->TxCount >= 3)
                {
                    smart_t1_resync(Smart_p,
                                   RxBlock_p->Header.NAD & 0x7,
                                   (RxBlock_p->Header.NAD & 0x70) >> 4);
                    Done = 1;
                }
                else
                {
                    smart_t1_write_block(Smart_p, &TxBlock);
                }
            }
        } while (Done == 0);
    }

    return ReceiveDone;
}

/* ------------------- upper functions will be used by T1 transfer ------------------> End */

//////////////////////////////////////////////////////////////////////////////////////

/* Open Smart Card Interface device */
CSSCI_HANDLE CSSCI_Open(void)
{	
	if (cssci_obj.obj_type != CSSCI_OBJ_TYPE) {
		cssci_obj.dev_fd = open(CSSCI_DEV_FILE, O_RDWR);

		if (cssci_obj.dev_fd < 0) {
			cssci_obj.errcode = SCI_ERROR_OPEN;
			return NULL;
		}
	}

	cssci_obj.obj_type = CSSCI_OBJ_TYPE;
	cssci_obj.sciinfo.type = CSSCI_DEFAULT_SC_TYPE; /* TBD */

	sci_evt = CSEVT_Init();
	register_flag = 0;
	return (CSSCI_HANDLE) &cssci_obj;
}

CSAPI_RESULT CSSCI_GetATR(CSSCI_HANDLE handle, unsigned char **atr, unsigned int *atr_len)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	int retval;
	char is_cardin, tc1_flag = 0;
	unsigned char chguard_value;
	char *buf = NULL;
	static char *k_buffer=NULL;
	static char k_hist=0;
	char Y[4]= {0}, TA[8] = {0}, TB[8] = {0}, TC[8] = {0}, TD[8] = {0};/* TCK;*/ /* change these values for optimization */
	int i=0, count= 0;
	unsigned int FI =0, DI=0, F=0, D=0, f=0;
	unsigned char old_parity_value=0, old_is_error_detect=0, old_rx_retry_times=0, old_tx_retry_times=0;

	if (_cssci_is_cardin(dev_obj->dev_fd, &is_cardin)) {
			dev_obj->errcode = SCI_ERROR_IOCTL;
			debug_printf(" Smartcard IOCTL Error : %s\n", strerror(errno));
			return CSAPI_FAILED;
	}		
	
	if (is_cardin == 0){
		dev_obj->errcode = SCI_ERROR_NO_CARD;
		debug_printf(" No Card: %s\n", strerror(errno));
		return CSAPI_FAILED;
	}

	/* The error signal and character repetition is mandatory for the cards offering T=0 */
	#ifdef	P_T0
	retval = CSSCI_Get_Parity_Manage( handle, &old_parity_value, &old_is_error_detect, &old_rx_retry_times, &old_tx_retry_times );
	/* enable parity error detect, CS feature, let CS chip know T0 working, so the default guard time is 12 etu, not 11 etu */
	retval |= CSSCI_Set_Parity_Manage( handle, 0, 1, 3, 3 ); /* even parity, enable detect, retry times 3 */
	if( retval != CSAPI_SUCCEED ){
		debug_printf("Parity_Manage Failed!!\n");
		return CSAPI_FAILED;
	}
	#endif
	
	if (is_cardin == 1){
		debug_printf("Activating Smartcard\n");

		#ifdef	IRDETO_T14
		if (_cssci_set_etu(dev_obj->dev_fd, SCI_F_IREDO_DEFAULT, SCI_D_DEFAULT)) {
			dev_obj->errcode = SCI_ERROR_SET_BAUDRATE;
			debug_printf(" Smartcard _cssci_set_etu Error : %s\n", strerror(errno));
			return CSAPI_FAILED;
		}
		#endif

		buf = (char *) malloc(1);
		retval = read(dev_obj->dev_fd, (void *)buf, 1);    		     	/* initial read one byte (TS) -- to start parsing ATR*/
		if(retval < 0) {
			debug_printf("Smartcard Read Error : %s\n", strerror(errno));
			free(buf);
			return CSAPI_FAILED;
		}
		if (  *buf == 0x3B ){				/*parse the buffer data after reading initial byte */
			dev_obj->sciinfo.init_char = *buf;			/* Update initial char TS */	
			dev_obj->sciinfo.atr[count] = *buf;
			dev_obj->sciinfo.nATR = count + 1;
			count++;		
		}
		else	if( *buf == 0x3 ){

			if( _cssci_set_convention(dev_obj->dev_fd, 1) ){
				dev_obj->errcode = SCI_ERROR_SET_CONVENTION;
				debug_printf(" _cssci_set_convention error: %s\n", strerror(errno));
				return CSAPI_FAILED;
			}
			if (_cssci_card_reset(dev_obj->dev_fd)) {
        		dev_obj->errcode = SCI_ERROR_RESET;
		 		debug_printf(" SCI   _cssci_card_reset   Error(close): %s\n", strerror(errno));
        		return CSAPI_FAILED;
    		}
			if (_cssci_card_active(dev_obj->dev_fd)) {
       		dev_obj->errcode = SCI_ERROR_ACTIVE;
		 		debug_printf(" SCI   _cssci_card_active   Error(close): %s\n", strerror(errno));
        		return CSAPI_FAILED;
    		}
			
			retval = read(dev_obj->dev_fd, (void *)buf, 1);    		     	/* initial read one byte (TS) -- to start parsing ATR*/
			if(retval < 0) {
				debug_printf("Smartcard Read Error : %s\n", strerror(errno));
				free(buf);
				return CSAPI_FAILED;
			}
			if (  *buf == 0x3F ){				/*parse the buffer data after reading initial byte */
				dev_obj->sciinfo.init_char = *buf;			/* Update initial char TS */	
				dev_obj->sciinfo.atr[count] = *buf;
				dev_obj->sciinfo.nATR = count + 1;
				count++;		
			}
			else{
				debug_printf("read initial byte error: %x \n",*buf);
				return CSAPI_FAILED;
			}
			
		}
		else{
				debug_printf("read initial byte error: %x \n",*buf);
				return CSAPI_FAILED;
        }

		retval = read(dev_obj->dev_fd, (void *)buf, 1);				/* read second byte (T0) */
		if(retval < 0) {
			debug_printf("Smartcard Read Error : %s\n", strerror(errno));
			free(buf);
			return CSAPI_FAILED;
		}
		Y[0] = *buf;

       
		if( (k_buffer == NULL) || ((Y[0] & 0x0F) > k_hist) ){
			if(k_buffer){
				free(k_buffer);
				k_buffer = NULL;
			}
			k_buffer = (char *) malloc(Y[0] & 0x0F);
            debug_printf("k_buffer =0x%x\n",k_buffer);
		}
		
		k_hist = (Y[0] & 0x0F);						/* get total number of historical characters present */
		Y[1] = (Y[0] & 0xF0);						/* get Y1 indicator */
		//debug_printf("Y1 indicator present in the T0 = 0x%x\n", Y[1]);
		dev_obj->sciinfo.format_char = Y[0];				/* Update format char T0 */
		dev_obj->sciinfo.atr[count] = *buf;
		dev_obj->sciinfo.nATR = count + 1;
		count++;
		
		for(i=1;;){
			if ((Y[i] & 0x10) == 0x10){
				retval = read(dev_obj->dev_fd, (void *)buf, 1);		/* read next character TA if present */
				if(retval < 0) {
					debug_printf("Smartcard Read Error : %s\n", strerror(errno));
					free(buf);
					return CSAPI_FAILED;
				}
				TA[i] = *buf;
				dev_obj->sciinfo.atr[count] = *buf;
				dev_obj->sciinfo.nATR = count + 1;
				count++;

				if( i == 1 ){
					FI = (TA[1] & 0xF0); /* FI are coded */
					DI = (TA[1] & 0x0F); /* DI are coded */
				}
				if( (i > 2) && (dev_obj->sciinfo.protocols[i-2] == 1) ){
					dev_obj->sciinfo.IFSC = TA[i];	/* Tx max bytes from device to card for T1 */
				}
			}
			if ((Y[i] & 0x20) == 0x20){
				retval = read(dev_obj->dev_fd, (void *)buf, 1);		/* read next character TB if present */
				if(retval < 0) {
					debug_printf("Smartcard Read Error : %s\n", strerror(errno));
					free(buf);
					return CSAPI_FAILED;
				}
				TB[i] = *buf;
				dev_obj->sciinfo.atr[count] = *buf;
				dev_obj->sciinfo.nATR = count + 1;
				count++;
				//debug_printf("I = 0x%x\n", (TB[i] & 0xC0));
				//debug_printf("P = 0x%x\n", (TB[i] & 0x3F));
				if( (i > 2) && (dev_obj->sciinfo.protocols[i-2] == 1) ){
					dev_obj->sciinfo.CWI = TB[i] & 0x0F;	/*  for CWT (character waitting time) */
					dev_obj->sciinfo.BWI = (TB[i] & 0xF0) >> 4;	/*  for BWT (block waitting time) */
				}
			}
			if ((Y[i] & 0x40) == 0x40){
				retval = read(dev_obj->dev_fd, (void *)buf, 1);		/* read next character TC if present */
				if(retval < 0) {
					debug_printf("Smartcard Read Error : %s\n", strerror(errno));
					free(buf);
					return CSAPI_FAILED;
				}
				TC[i] = *buf;
				dev_obj->sciinfo.atr[count] = *buf;
				dev_obj->sciinfo.nATR = count + 1;
				count++;
				if( i == 1 )
					tc1_flag = 1;
				//debug_printf("TC%d = 0x%x\n", i, TC[i]);
				//debug_printf("N = 0x%x\n", (TC[i] & 0xFF));
				if( (i == 1) && (tc1_flag == 1) )
				{
					if( TC[1] == 255 )
						chguard_value = 0;
					else
					{
						#ifdef	P_T0
						chguard_value = TC[1];
						#endif
						#ifdef	P_T1
						chguard_value = TC[1] + 1;
						#endif
					}
			
					if (_cssci_set_chguard(dev_obj->dev_fd, chguard_value)) {
       				 dev_obj->errcode = SCI_ERROR_SET_CHGUARD;
       				 return CSAPI_FAILED;
    				}
				}
				if( (i > 2) && (dev_obj->sciinfo.protocols[i-2] == 1) ){
					dev_obj->sciinfo.RC = TC[i] & 0x01;	/* specifies the error detection code for T1, 1:CRC,0:LRC */
				}
			}
			if ((Y[i] & 0x80) == 0x80){
				retval = read(dev_obj->dev_fd, (void *)buf, 1);		/* read next character TD if present */
				if(retval < 0) {
					debug_printf("Smartcard Read Error : %s\n", strerror(errno));
					free(buf);
					return CSAPI_FAILED;
				}
				TD[i] = *buf;
				dev_obj->sciinfo.atr[count] = *buf;
				dev_obj->sciinfo.nATR = count + 1;
				count++;
				//debug_printf("TD%d = 0x%x\n", i, TD[i]);
				//debug_printf("Y%d  = 0x%x\n", i+1, (TD[i] & 0xF0));
				//debug_printf("T=0x%x\n", (TD[i] & 0xF0));
				i++;
				Y[i] = (TD[i-1] & 0xF0);
				dev_obj->sciinfo.protocols[i-2] = (TD[i-1] & 0x0F);
			}
			else{
				break;						/* Break out if no more TDi's are following */
			}

		}								/* end of for loop */

		dev_obj->sciinfo.TA1 = TA[1];		/* update interface byte values */
		dev_obj->sciinfo.TB1 = TB[1];
		dev_obj->sciinfo.TC1 = TC[1];
		dev_obj->sciinfo.TD1 = TD[1];
		//dev_obj->sciinfo.current_protocol = TD[1] & 0x0f;		/* Default value is T0 */

		retval = read(dev_obj->dev_fd, (void *)k_buffer, k_hist);		/* Read out all historical characters */
		if(retval < 0) {
			debug_printf("Smartcard Read Error : %s\n", strerror(errno));
			free(buf);
			free(k_buffer);
			k_buffer = NULL;
			return CSAPI_FAILED;
		}

		dev_obj->sciinfo.hist_char = k_buffer;		/* update historical values */
        //        debug_printf("dev_obj=0x%x, dev_obj->sciinfo=0x%x, dev_obj->sciinfo.TD1=0x%x, dev_obj->sciinfo.TCheck=0x%x, dev_obj->sciinfo.nATR =0x%x, dev_obj->sciinfo.hist_char = 0x%x\n",dev_obj, &dev_obj->sciinfo, dev_obj->sciinfo.TD1, dev_obj->sciinfo.TCheck, dev_obj->sciinfo.nATR, dev_obj->sciinfo.hist_char);
		for (i=0; i < k_hist; i++){			
		    debug_printf("T%d = 0x%x \t %c\n", i+1, *(dev_obj->sciinfo.hist_char + i), *(dev_obj->sciinfo.hist_char + i));
			dev_obj->sciinfo.atr[count] = *(dev_obj->sciinfo.hist_char + i);
			dev_obj->sciinfo.nATR = count + 1;
			count++;
		}
		
/****************************************************************************/
#ifdef	IRDETO_T14
		char TCK;
		retval = read(dev_obj->dev_fd, (void *)buf, 1);				/* Read checksum character */
		if(retval < 0) {
			debug_printf("Smartcard Read Error : %s\n", strerror(errno));
			return CSAPI_FAILED;
		}
		TCK = *buf;
		dev_obj->sciinfo.TCheck = TCK;		/* Update checksum value */
		dev_obj->sciinfo.atr[count] = *buf;
		dev_obj->sciinfo.nATR = count + 1;
		count++;
		//debug_printf("TCK checksum value is 0x%x\n", dev_obj->sciinfo.TCheck);
#endif
/*****************  if need TCK, open the upper read  *******************/

		if ( TA[1] != 0x00 )
		{
			
			/*Calculate the frequency f and F from coded FI's*/
			f = 1*MHZ; 
			F = 372; 		/* default value */
			
			if ( FI == 0x00 ){
				f = 4*MHZ; F = 372; }	
			if ( FI == 0x10 ){
				f = 5*MHZ; F = 372; }
				
			#ifdef	IRDETO_T14
					if ( FI == 0x20 ){
						f = 6*MHZ; F = 620; }
			#else
					if ( FI == 0x20 ){
						f = 6*MHZ; F = 558; }
			#endif
			
			if ( FI == 0x30 ){
				f = 8*MHZ; F = 744; }
			if ( FI == 0x40 ){
				f = 12*MHZ; F = 1116; }
			if ( FI == 0x50 ){
				f = 16*MHZ; F = 1488; }
			if ( FI == 0x60 ){
				f = 20*MHZ; F = 1860; }
			if ( FI == 0x90 ){
				f = 5*MHZ; F = 512; }
			if ( FI == 0xa0 ){
				f = 7.5*MHZ; F = 768; }
			if ( FI == 0xb0 ){
				f = 10*MHZ; F = 1024; }
			if ( FI == 0xc0 ){
				f = 15*MHZ; F = 1536; }
			if ( FI == 0xd0 ){
				f = 20*MHZ; F = 2048; }				
					

			/*Calculate the D from coded DI's*/
			D = 1;		/* default value */
			if ( DI == 0x01 )
				D = 1;
			if ( DI == 0x02 )
				D = 2;
			if ( DI == 0x03 )
				D = 4;
			if ( DI == 0x04 )
				D = 8;
			if ( DI == 0x05 )
				D = 16;
			if ( DI == 0x06 )
				D = 32;
			if ( DI == 0x07 )
				D = 64;
			if ( DI == 0x08 )
				D = 12;
			if ( DI == 0x09 )
				D = 20;

			if( TA[2] != 0x00 )
			{
				/* TA2 indicates the specific mode of operation */
				dev_obj->sciinfo.specific_mode = 1;
				dev_obj->sciinfo.current_protocol = TA[2] & 0x0F;
				dev_obj->sciinfo.specific_type_changable = ( (TA[2] & 0x80) ? 0 : 1);
			
				if( !(TA[2]&0x10) )
				{
					if (_cssci_set_etu(dev_obj->dev_fd, F, D)) {	/*Set F and D values obtained from ATR information */
						dev_obj->errcode = SCI_ERROR_IOCTL;
						debug_printf(" Smartcard IOCTL Error : %s\n", strerror(errno));
						return CSAPI_FAILED;
					}
				}
			}
			else		/* If TA2 is not present, negotiable mode is denoted */
			{
				dev_obj->sciinfo.specific_mode = 0;
				dev_obj->sciinfo.current_protocol = dev_obj->sciinfo.protocols[0];	/* first offered protocol */
				#ifdef	IRDETO_T14
				dev_obj->sciinfo.current_protocol = 14;
				#endif
			}
		
		}

		dev_obj->sciinfo.atr[count] = '\0';
		atr_readyflag = 1;

	} 
	else {
		debug_printf("There is no card!\n");                       
	}

	/*set ATR notification */
	if(atr_readyflag == 1){
		
		if (callback_flag & CALLBACK_FLAG_ATRRDY)
		{
			CSSCI_Notify(&sci_callback, dev_obj->dev_fd, CSSCI_ATRRDY);
		}

		if( dev_obj->sciinfo.current_protocol == 1 )	/* T1 protocol */
		{
 			retval = CSSCI_Set_Parity_Manage( handle, 0, 0, 0, 0 );	 /* disable parity error detect, let CS chip know T1 working */
			if( retval != CSAPI_SUCCEED ){
				debug_printf("CSSCI_Set_Parity_Manage Failed!!\n");
				return CSAPI_FAILED;
			}

			retval = CSSCI_SetBlockGuardDelay( handle, 22 );	 /* For T1, BGT=22 etu */
			if( retval != CSAPI_SUCCEED ){
				debug_printf("CSSCI_SetBlockGuardDelay Failed!!\n");
				return CSAPI_FAILED;
			}
		}
		
		*atr = dev_obj->sciinfo.atr;
		*atr_len = (unsigned int)dev_obj->sciinfo.nATR;
	}
	else{
		*atr = NULL;
		*atr_len = 0;
	}
	
	free(buf);
	
	return CSAPI_SUCCEED;
}

/* Close Smart Card Interface device */
CSAPI_RESULT CSSCI_Close(CSSCI_HANDLE handle)
{
	int retval;
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	dev_obj->obj_type = 0;
	
	CSEVT_Term(sci_evt);
	
	retval = close(dev_obj->dev_fd); 
	if (retval < 0) {
		dev_obj->errcode = SCI_ERROR_CLOSE;
		debug_printf(" SCI Close Error(close): %s\n", strerror(errno));
		return CSAPI_FAILED;
	}
	
	dev_obj->dev_fd = -1;			/*reset values back to default */
	dev_obj->errcode = 0;
	dev_obj->sciinfo.type = 0;
	dev_obj->sciinfo.init_char = 0;
	dev_obj->sciinfo.format_char = 0;
	dev_obj->sciinfo.current_protocol = 0;
	dev_obj->sciinfo.TA1 = 0;
	dev_obj->sciinfo.TB1 = 0;
	dev_obj->sciinfo.TC1 = 0;
	dev_obj->sciinfo.TD1 = 0;
	dev_obj->sciinfo.hist_char = NULL;
	dev_obj->sciinfo.TCheck = 0;
	for (retval=0; retval < 33; retval ++)
		dev_obj->sciinfo.atr[retval]=0;
	dev_obj->sciinfo.nATR = 0;
	register_flag = 0;
	callback_flag = 0;
	rxtimeout_flag = 0;
	atr_readyflag = 0;
	readrdy_flag = 0;
	writerdy_flag = 0;
	readerror_flag = 0;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_Activate(CSSCI_HANDLE handle)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if (_cssci_card_active(dev_obj->dev_fd)) {
        dev_obj->errcode = SCI_ERROR_ACTIVE;
		debug_printf(" SCI Active Card Error(close): %s\n", strerror(errno));
        return CSAPI_FAILED;
    }
	
	dev_obj->sciinfo.type = 0;
	dev_obj->sciinfo.init_char = 0;
	dev_obj->sciinfo.format_char = 0;
	dev_obj->sciinfo.current_protocol = 0;
	dev_obj->sciinfo.TA1 = 0;
	dev_obj->sciinfo.TB1 = 0;
	dev_obj->sciinfo.TC1 = 0;
	dev_obj->sciinfo.TD1 = 0;
	dev_obj->sciinfo.hist_char = NULL;
	dev_obj->sciinfo.TCheck = 0;
	dev_obj->sciinfo.nATR = 0;
	memset(dev_obj->sciinfo.atr, 0, sizeof(dev_obj->sciinfo.atr));
	
//	register_flag = 0;
//	callback_flag = 0;
	rxtimeout_flag = 0;
	atr_readyflag = 0;
	readrdy_flag = 0;
	writerdy_flag = 0;
	readerror_flag = 0;
	
	return CSAPI_SUCCEED;
}


CSAPI_RESULT CSSCI_Deactivate(CSSCI_HANDLE handle)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if (_cssci_card_deactive(dev_obj->dev_fd)) {
        dev_obj->errcode = SCI_ERROR_DEACTIVE;
		debug_printf(" SCI Deactive Card Error(close): %s\n", strerror(errno));
        return CSAPI_FAILED;
    }

	dev_obj->sciinfo.type = 0;
	dev_obj->sciinfo.init_char = 0;
	dev_obj->sciinfo.format_char = 0;
	dev_obj->sciinfo.current_protocol = 0;
	dev_obj->sciinfo.TA1 = 0;
	dev_obj->sciinfo.TB1 = 0;
	dev_obj->sciinfo.TC1 = 0;
	dev_obj->sciinfo.TD1 = 0;
	dev_obj->sciinfo.hist_char = NULL;
	dev_obj->sciinfo.TCheck = 0;
	dev_obj->sciinfo.nATR = 0;
	memset(dev_obj->sciinfo.atr, 0, sizeof(dev_obj->sciinfo.atr));
	
	//register_flag = 0;
	//callback_flag = 0;
	rxtimeout_flag = 0;
	atr_readyflag = 0;
	readrdy_flag = 0;
	writerdy_flag = 0;
	readerror_flag = 0;
	
	return CSAPI_SUCCEED;    
}

CSAPI_RESULT CSSCI_Reset(CSSCI_HANDLE handle, unsigned char **atr, unsigned int *atr_len)	/* modified Resetcard to Reset -- Added */
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if( _cssci_set_convention(dev_obj->dev_fd, 0) ){
				dev_obj->errcode = SCI_ERROR_SET_CONVENTION;
				debug_printf(" _cssci_set_convention error: %s\n", strerror(errno));
				return CSAPI_FAILED;
	}
	
	if (_cssci_card_reset(dev_obj->dev_fd)) {
        dev_obj->errcode = SCI_ERROR_RESET;
		 debug_printf(" SCI   _cssci_card_reset   Error(close): %s\n", strerror(errno));
        return CSAPI_FAILED;
    }
	if (_cssci_card_active(dev_obj->dev_fd)) {
        dev_obj->errcode = SCI_ERROR_ACTIVE;
		 debug_printf(" SCI   _cssci_card_active   Error(close): %s\n", strerror(errno));
        return CSAPI_FAILED;
    }

/*reset values back to default */	
	dev_obj->sciinfo.type = 0;
	dev_obj->sciinfo.init_char = 0;
	dev_obj->sciinfo.format_char = 0;
	dev_obj->sciinfo.current_protocol = 0;
	memset(dev_obj->sciinfo.protocols, 0, sizeof(dev_obj->sciinfo.protocols));
	dev_obj->sciinfo.TA1 = 0;
	dev_obj->sciinfo.TB1 = 0;
	dev_obj->sciinfo.TC1 = 0;
	dev_obj->sciinfo.TD1 = 0;
	dev_obj->sciinfo.hist_char = NULL;
	dev_obj->sciinfo.TCheck = 0;
	dev_obj->sciinfo.nATR = 0;
	memset(dev_obj->sciinfo.atr, 0, sizeof(dev_obj->sciinfo.atr));
	dev_obj->sciinfo.specific_mode = 0;
	dev_obj->sciinfo.specific_type_changable = 0;
	dev_obj->sciinfo.IFSC = 0;
	dev_obj->sciinfo.CWI = 0;
	dev_obj->sciinfo.BWI = 0;
	dev_obj->sciinfo.RC = 0;
	dev_obj->sciinfo.TxMaxInfoSize = 32;
	dev_obj->sciinfo.OurSequence = 0;
	dev_obj->sciinfo.TheirSequence = 1;
	dev_obj->sciinfo.TxCount = 0;
	dev_obj->sciinfo.RxCount = 0;
	dev_obj->sciinfo.FirstBlock = 0;
	dev_obj->sciinfo.Aborting = 0;

//	register_flag = 0;
//	callback_flag = 0;
	rxtimeout_flag = 0;
	atr_readyflag = 0;
	readrdy_flag = 0;
	writerdy_flag = 0;
	readerror_flag = 0;
	
	if(CSSCI_GetATR((CSSCI_HANDLE)&cssci_obj, atr, atr_len) ==  CSAPI_FAILED)	/* get the updated values for sciinfo by calling atr fn*/
	{
		if(!dev_obj->errcode)
			dev_obj->errcode = SCI_ERROR_UNKNOWN_DEVICE_NAME;
		 debug_printf("Error getting ATR information \n");
        return CSAPI_FAILED;
	}
	else
	{
		debug_printf("Got ATR info \n");
	}
	
    return CSAPI_SUCCEED;
} 

CSAPI_RESULT CSSCI_Set_SC_CLK(CSSCI_HANDLE handle, unsigned int freq)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	
	if(_cssci_set_sc_clock(dev_obj->dev_fd, freq)){
		dev_obj->errcode = SCI_ERROR_SET_CLOCK;
		return CSAPI_FAILED;
	}

	#ifdef	IRDETO_T14
	if (_cssci_set_etu(dev_obj->dev_fd, SCI_F_IREDO_DEFAULT, SCI_D_DEFAULT)) {
	#else
	if (_cssci_set_etu(dev_obj->dev_fd, SCI_F_DEFAULT, SCI_D_DEFAULT)) {	/* Adjust SCIBAUD value according to the new SCICLKICC */
	#endif
		dev_obj->errcode = SCI_ERROR_SET_BAUDRATE;
		debug_printf(" Smartcard _cssci_set_etu Error : %s\n", strerror(errno));
		return CSAPI_FAILED;
	}
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_Get_SC_CLK(CSSCI_HANDLE handle, unsigned int *freq)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	
	if(_cssci_get_sc_clock(dev_obj->dev_fd, freq)){
		dev_obj->errcode = SCI_ERROR_GET_CLOCK;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_SetRxTimeout(CSSCI_HANDLE handle, unsigned short rx_timeout)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if (_cssci_set_rxtimeout(dev_obj->dev_fd, rx_timeout)) {
        dev_obj->errcode = SCI_ERROR_SET_RXTIMEOUT;
        return CSAPI_FAILED;
    }
	
    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_GetRxTimeout(CSSCI_HANDLE handle, unsigned short *rx_timeout)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if (_cssci_get_rxtimeout(dev_obj->dev_fd, rx_timeout)) {
        dev_obj->errcode = SCI_ERROR_GET_RXTIMEOUT;
        return CSAPI_FAILED;
    }
    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_SetCHGuardDelay(CSSCI_HANDLE handle, unsigned int guard_delay)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if (_cssci_set_chguard(dev_obj->dev_fd, guard_delay)) {
        dev_obj->errcode = SCI_ERROR_SET_CHGUARD;
        return CSAPI_FAILED;
    }
    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_GetCHGuardDelay(CSSCI_HANDLE handle, unsigned char *guard_delay)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if (_cssci_get_chguard(dev_obj->dev_fd, guard_delay)) {
        dev_obj->errcode = SCI_ERROR_GET_CHGUARD;
        return CSAPI_FAILED;
    }
    return CSAPI_SUCCEED;
}


/*******
T0: guard_delay >= 16 (etu)
T1: guard_delay >= 22 (etu)
*******/
CSAPI_RESULT CSSCI_SetBlockGuardDelay(CSSCI_HANDLE handle, unsigned int guard_delay)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	#ifdef	P_T0
	if( guard_delay < 16 ){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
	guard_delay -= 12;
	#endif
	#ifdef	P_T1
	if( guard_delay < 22 ){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
	guard_delay -= 11;
	#endif	

	if (_cssci_set_blockguard(dev_obj->dev_fd, guard_delay)) {
        dev_obj->errcode = SCI_ERROR_SET_BLOCKGUARD;
        return CSAPI_FAILED;
    }
    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_GetBlockGuardDelay(CSSCI_HANDLE handle, unsigned char *guard_delay)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if (_cssci_get_blockguard(dev_obj->dev_fd, guard_delay)) {
        dev_obj->errcode = SCI_ERROR_GET_BLOCKGUARD;
        return CSAPI_FAILED;
    }

	#ifdef	P_T0
	*guard_delay = *guard_delay + 12;
	#endif
	#ifdef	P_T1
	*guard_delay = *guard_delay + 11;
	#endif
	
    return CSAPI_SUCCEED;
}


/******
T0: retry times <= 3, 
T1: is_error_detect must set 0; 
parity_value: 0 even parity, 1 odd parity
is_error_detect: 0 disable, 1 enable 
******/
CSAPI_RESULT CSSCI_Set_Parity_Manage(	CSSCI_HANDLE 	handle, 
																						unsigned char 	parity_value,
																						unsigned char 	is_error_detect,
																						unsigned char 	rx_retry_times,
																						unsigned char 	tx_retry_times	)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if( ((parity_value!=0)&&(parity_value!=1)) || ((is_error_detect!=0)&&(is_error_detect!=1)) ){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
	
	#ifdef	P_T0
	if( (rx_retry_times > 3) || (tx_retry_times > 3) ){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
	#endif
	#ifdef	P_T1
	if( (is_error_detect == 1) || (is_error_detect == 1) ){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
	#endif

	if (_cssci_set_parity(dev_obj->dev_fd, parity_value)) {
        dev_obj->errcode = SCI_ERROR_SET_PARITY;
        return CSAPI_FAILED;
    }
	Parity_Value = parity_value;

	if (_cssci_set_parity_error_detect(dev_obj->dev_fd, is_error_detect)) {
        dev_obj->errcode = SCI_ERROR_SET_PARITY_ERROR_DETECT;
        return CSAPI_FAILED;
    }
	Is_Error_Detect = is_error_detect;

	if (_cssci_set_rxretry(dev_obj->dev_fd, rx_retry_times)) {
        dev_obj->errcode = SCI_ERROR_SET_RXRETRY;
        return CSAPI_FAILED;
    }

	if (_cssci_set_txretry(dev_obj->dev_fd, tx_retry_times)) {
        dev_obj->errcode = SCI_ERROR_SET_TXRETRY;
        return CSAPI_FAILED;
    }
	
    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_Get_Parity_Manage(	CSSCI_HANDLE 	handle, 
																						unsigned char 	*parity_value,
																						unsigned char 	*is_error_detect,
																						unsigned char 	*rx_retry_times,
																						unsigned char		*tx_retry_times	)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if((parity_value == NULL)||(is_error_detect == NULL)||(rx_retry_times == NULL)||(tx_retry_times == NULL)){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
	
	*parity_value = Parity_Value;
	*is_error_detect = Is_Error_Detect;

	if (_cssci_get_rxretry(dev_obj->dev_fd, rx_retry_times)) {
        dev_obj->errcode = SCI_ERROR_SET_RXRETRY;
        return CSAPI_FAILED;
    }

	if (_cssci_get_txretry(dev_obj->dev_fd, tx_retry_times)) {
        dev_obj->errcode = SCI_ERROR_SET_TXRETRY;
        return CSAPI_FAILED;
    }
	
    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_Read(CSSCI_HANDLE handle, unsigned char *buffer, unsigned int len, unsigned int *readedlen)
{
	int retval = 0;
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);

	if ( (buffer == NULL) || (readedlen == NULL) ){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
	retval = _cssci_read(handle, buffer, len);
	if (retval < 0){
		*readedlen= 0;
		return CSAPI_FAILED;
	}
	if (retval < (int)len){
		rxtimeout_flag = 1;
		if(callback_flag & CALLBACK_FLAG_RXTIMEOUT)
			CSSCI_Notify(&sci_callback, dev_obj->dev_fd, CSSCI_RXTIMEOUT);
		
		*readedlen= retval;
		dev_obj->errcode = SCI_ERROR_RX_TIMEOUT;
		return CSAPI_FAILED;
	}
	
	*readedlen= retval;

	return CSAPI_SUCCEED;
}


CSAPI_RESULT CSSCI_Write(CSSCI_HANDLE handle, unsigned char *buffer, unsigned int len, unsigned int *writtedlen)
{
	int retval = 0;
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	
	if ( (buffer == NULL) || (writtedlen == NULL) ){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}

	#ifdef	C_ABV_card

		unsigned int	i,j;
		*writtedlen = 0;
		for(i=0; i< len; i++)
		{
			retval = _cssci_write(handle, buffer, 1);
			if (retval < 0){
				dev_obj->errcode = SCI_ERROR_TX_TIMEOUT;
				return CSAPI_FAILED;
			}
			*writtedlen += retval;
			buffer++;
			for(j=0;j<0xf;j++);	/* instead of usleep(10000), modified by sunbin 20081204 */
		}
	
	#else
	
		retval = _cssci_write(handle, buffer, len);
		if (retval < 0){
			*writtedlen= 0;
			return CSAPI_FAILED;
		}
		if (retval < (int)len){
			*writtedlen= retval;
			dev_obj->errcode = SCI_ERROR_TX_TIMEOUT;
			return CSAPI_FAILED;
		}

		*writtedlen = retval;
	
	#endif
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_Transfer( CSSCI_HANDLE handle,
                             unsigned char *cmd,
                             unsigned int  cmd_len,
                             unsigned char *response,
                             unsigned int response_len,
                              unsigned int *written_len,
                              unsigned int *responsed_len,
                             CSSCI_Status *Status
                             )
{
	char is_cardin;
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);


	if (_cssci_is_cardin(dev_obj->dev_fd, &is_cardin)) {
			dev_obj->errcode = SCI_ERROR_IOCTL;
			debug_printf(" Smartcard IOCTL Error : %s\n", strerror(errno));
			return CSAPI_FAILED;
	}
	
	if (cmd == NULL || cmd_len == 0 || response == NULL || response_len == 0 || Status == NULL){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
		
	if (is_cardin == 0){
		dev_obj->errcode = SCI_ERROR_NO_CARD;
		debug_printf(" No Card: %s\n", strerror(errno));
		return CSAPI_FAILED;
	}

	if (atr_readyflag !=1 ){
		dev_obj->errcode = SCI_ERROR_NOATRRECEIVED;
		debug_printf(" No Card: %s\n", strerror(errno));
		return CSAPI_FAILED;
		
	}	
	else {
		switch (dev_obj->sciinfo.current_protocol) {
			case 0: /*T0 protocol*/
				#ifdef	C_ABV_card
					usleep(50000);	/* strange card needs it, modified by sunbin 20081204 */
				#endif
				
				if( CSSCI_T0_Transfer(handle,cmd,cmd_len,response,response_len,written_len,responsed_len,Status) == CSAPI_SUCCEED)
						break;
				else
					return		CSAPI_FAILED;
				
			case 14: /*T14 protocol */
				if( CSSCI_T14_Transfer(handle,cmd,cmd_len,response,response_len,written_len,responsed_len) == CSAPI_SUCCEED)
					break;
				else
					return		CSAPI_FAILED;
				
			case 1: /*T1 protocol*/
				if( CSSCI_T1_Transfer(handle,cmd,cmd_len,written_len,response,response_len,responsed_len,Status->SW1,Status->SW2) == CSAPI_SUCCEED)
					break;
				else
					return		CSAPI_FAILED;
				
			default:
				break;
		}

	}
	return CSAPI_SUCCEED;
}

CSSCI_ErrCode CSSCI_GetErrCode(CSSCI_HANDLE handle)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;

	if ((NULL == dev_obj) || (dev_obj->obj_type != CSSCI_OBJ_TYPE))
		return SCI_ERROR_INVALID_PARAMETERS;

	return dev_obj->errcode;
}

char *CSSCI_GetErrString(CSSCI_HANDLE handle)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;

	if ((NULL == dev_obj) || (dev_obj->obj_type != CSSCI_OBJ_TYPE))
		return sciout_errstr[SCI_ERROR_INVALID_PARAMETERS];

	return sciout_errstr[dev_obj->errcode];
}

CSAPI_RESULT CSSCI_SetSCType(CSSCI_HANDLE handle, CSSCI_SCTYPE type)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	
	dev_obj->sciinfo.type = type; 		/*set manufacturer's type here */
	
	return CSAPI_SUCCEED;
}

static void CSSCI_Notify(void *inparam, int fd,int events)
{
	CSSCI_SCIEVENT eventtype;
	char val;
	CSSCI_OBJ *handle_obj[2];

	if( (inparam == NULL) || (fd == 0) ){
		printf("returning from NULL inparam in thread fun\n");
		return;
	}

	if ( EVT_EXCEPT == events)
	{
		if(  callback_flag&CALLBACK_FLAG_CARDINS ){
			
			handle_obj[0] = sci_callback[0].sci_handle;
			eventtype = CSSCI_CARDINS;
			_cssci_is_cardin(handle_obj[0]->dev_fd, &val);
			if(val == 1)
				sci_callback[0].callfun(sci_callback[0].sci_handle,&eventtype);
								
		}
		if(  callback_flag&CALLBACK_FLAG_CARDREM){
			
			handle_obj[1] = sci_callback[1].sci_handle;
			eventtype = CSSCI_CARDREM;
			_cssci_is_cardin(handle_obj[1]->dev_fd, &val);
			if(val == 0)
				sci_callback[1].callfun(sci_callback[1].sci_handle,&eventtype);
		}			
	}
	
	if ( (CSSCI_RXTIMEOUT == events) && (callback_flag&CALLBACK_FLAG_RXTIMEOUT) )
	{
		eventtype = CSSCI_RXTIMEOUT;
		if(rxtimeout_flag == 1){
			sci_callback[2].callfun(sci_callback[2].sci_handle, &eventtype);
			rxtimeout_flag = 0;
		}
	}
	if ( (CSSCI_ATRRDY == events) && (callback_flag&CALLBACK_FLAG_ATRRDY) )
	{
		eventtype = CSSCI_ATRRDY;
		if(atr_readyflag == 1){
			sci_callback[3].callfun(sci_callback[3].sci_handle, &eventtype);
			 atr_readyflag = 0;
		}
	}
	
	if ( (EVT_READ == events) && (callback_flag&CALLBACK_FLAG_READRDY) )
	{
		eventtype = CSSCI_READRDY;
		if(readrdy_flag == 1){
			sci_callback[4].callfun(sci_callback[4].sci_handle, &eventtype);
			readrdy_flag  = 0;
		}
	}
	if ( (EVT_WRITE == events) && (callback_flag&CALLBACK_FLAG_WRITERDY) )
	{
		eventtype = CSSCI_WRITERDY;
		if(writerdy_flag == 1){
			sci_callback[5].callfun(sci_callback[5].sci_handle, &eventtype);
			writerdy_flag = 0;
		}
	}
	
	if ( (CSSCI_READERROR == events) && (callback_flag&CALLBACK_FLAG_READERROR) )
	{
		eventtype = CSSCI_READERROR;
		if(readerror_flag == 1){
			sci_callback[6].callfun(sci_callback[6].sci_handle, &eventtype);
			readerror_flag = 0;
		}
	}
	
}

CSAPI_RESULT CSSCI_SetNotify(CSSCI_HANDLE handle, void (*call_back_function)(CSSCI_HANDLE, CSSCI_SCIEVENT *), 
								CSSCI_SCIEVENT sci_event, unsigned char event_enable)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	unsigned char i,is_cardin=0;
	CSSCI_SCIEVENT	scievent;

	if((NULL == dev_obj) || (dev_obj->obj_type != CSSCI_OBJ_TYPE)){
		printf("returning from object type mismatch\n");
		return CSAPI_FAILED;
	}

	if(call_back_function == NULL){
		dev_obj->errcode = SCI_ERROR_INVALID_PARAMETERS;
		printf("Returning bcz of NULL Callback function\n");
		return CSAPI_FAILED;
	}

	if(dev_obj->dev_fd == -1){
		dev_obj->errcode = SCI_ERROR_NOTOPEN_DEVICE;
		printf("Returning from Error in File discriptor\n");
		return CSAPI_FAILED;
	}

	if ( register_flag == 0 ){
		callback_flag = 0;
		for( i = 0; i < 7; i++ )
			sci_callback[i].evt = -1;
	}
	
	switch(sci_event)
	{
		case CSSCI_CARDINS:
			callback_flag |= CALLBACK_FLAG_CARDINS;
			sci_callback[0].sci_handle = handle;
			sci_callback[0].callfun = call_back_function;
			sci_callback[0].evt = CSSCI_CARDINS;

			if (_cssci_is_cardin(dev_obj->dev_fd, &is_cardin)) {
				dev_obj->errcode = SCI_ERROR_IOCTL;
				debug_printf(" Smartcard IOCTL Error : %s\n", strerror(errno));
				return CSAPI_FAILED;
			}
			if(is_cardin == 1){
				scievent = CSSCI_CARDINS;
				call_back_function(handle, &scievent );
			}
			
			break;
				
		case CSSCI_CARDREM:
			callback_flag |= CALLBACK_FLAG_CARDREM;
			sci_callback[1].sci_handle = handle;
			sci_callback[1].callfun = call_back_function;
			sci_callback[1].evt = CSSCI_CARDREM;

			if (_cssci_is_cardin(dev_obj->dev_fd, &is_cardin)) {
				dev_obj->errcode = SCI_ERROR_IOCTL;
				debug_printf(" Smartcard IOCTL Error : %s\n", strerror(errno));
				return CSAPI_FAILED;
			}
			if(is_cardin == 0){
				scievent = CSSCI_CARDREM;
				call_back_function( handle, &scievent );
			}
			
			break;
				
		case CSSCI_RXTIMEOUT:
			callback_flag |= CALLBACK_FLAG_RXTIMEOUT;
			sci_callback[2].sci_handle = handle;
			sci_callback[2].callfun = call_back_function;
			sci_callback[2].evt = CSSCI_RXTIMEOUT;		

			break;
				
		case CSSCI_ATRRDY:
			callback_flag |= CALLBACK_FLAG_ATRRDY;
			sci_callback[3].sci_handle = handle;
			sci_callback[3].callfun = call_back_function;
			sci_callback[3].evt = CSSCI_ATRRDY;

			break;
			
		case CSSCI_READRDY:
			callback_flag |= CALLBACK_FLAG_READRDY;
			sci_callback[4].sci_handle = handle;
			sci_callback[4].callfun = call_back_function;
			sci_callback[4].evt = CSSCI_READRDY;

			break;
			
		case CSSCI_WRITERDY:
			callback_flag |= CALLBACK_FLAG_WRITERDY;
			sci_callback[5].sci_handle = handle;
			sci_callback[5].callfun = call_back_function;
			sci_callback[5].evt = CSSCI_WRITERDY;

			break;
			
		case CSSCI_READERROR:
			callback_flag |= CALLBACK_FLAG_READERROR;
			sci_callback[6].sci_handle = handle;
			sci_callback[6].callfun = call_back_function;
			sci_callback[6].evt = CSSCI_READERROR;

			break;
	}

   if ( register_flag == 0 )
	{
		register_flag =1;
		printf("SCI  Callback registering in event module\n");
		if (CSAPI_FAILED == CSEVT_Register(sci_evt,  
	             				   dev_obj->dev_fd, 
						   CSSCI_Notify, 
						   &sci_callback[0], 
 						   event_enable ? EVT_EXCEPT : EVT_INVALID))
		{
			dev_obj->errcode = SCI_ERROR_REGISTER_CALLBACK;	
			printf("Returning from CSEVT_Register() Error\n");
			return CSAPI_FAILED;					
		}
	}	

	return CSAPI_SUCCEED;

}

/* how many bytes we could read from the rxfifo */
CSAPI_RESULT CSSCI_GetReadSize(CSSCI_HANDLE handle, unsigned char *rd_size)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	if(_cssci_get_rxrdy_size(dev_obj->dev_fd, rd_size)){
		dev_obj->errcode = SCI_ERROR_GET_RDSIZE;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
}

/* how many bytes wo could write into the txfifo */
CSAPI_RESULT CSSCI_GetWriteSize(CSSCI_HANDLE handle, unsigned char *wr_size)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	if(_cssci_get_txrdy_size(dev_obj->dev_fd, wr_size)){
		dev_obj->errcode = SCI_ERROR_GET_WRSIZE;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_SetReadReadyNotifySize(CSSCI_HANDLE handle, unsigned int Rx_Threshold)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	
	if(_cssci_set_rxfifo_threshold(dev_obj->dev_fd, Rx_Threshold)){
		dev_obj->errcode = SCI_ERROR_SET_RXTHRESHOLD;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_GetReadReadyNotifySize(CSSCI_HANDLE handle, unsigned int *Rx_Threshold)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	
	if(_cssci_get_rxfifo_threshold(dev_obj->dev_fd, Rx_Threshold)){
		dev_obj->errcode = SCI_ERROR_GET_RXTHRESHOLD;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_SetWriteReadyNotifySize(CSSCI_HANDLE handle, unsigned int Tx_Threshold)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	
	if(_cssci_set_txfifo_threshold(dev_obj->dev_fd, Tx_Threshold)){
		dev_obj->errcode = SCI_ERROR_SET_TXTHRESHOLD;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_GetWriteReadyNotifySize(CSSCI_HANDLE handle, unsigned int *Tx_Threshold)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	
	if(_cssci_get_txfifo_threshold(dev_obj->dev_fd, Tx_Threshold)){
		dev_obj->errcode = SCI_ERROR_GET_TXTHRESHOLD;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_RxFIFO_Report(CSSCI_HANDLE handle, unsigned char report)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	if(_cssci_rxfifo_report(dev_obj->dev_fd, report)){
		dev_obj->errcode = SCI_ERROR_RXFIFO_REPORT;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSCI_TxFIFO_Report(CSSCI_HANDLE handle, unsigned char report)
{
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSSCI_OBJ_TYPE);
	if(_cssci_txfifo_report(dev_obj->dev_fd, report)){
		dev_obj->errcode = SCI_ERROR_TXFIFO_REPORT;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
}


static CSAPI_RESULT CSSCI_T0_Transfer( CSSCI_HANDLE Smart_p,
                                  											unsigned char *WriteBuffer_p,
                                 					 						unsigned int NumberToWrite,
                                  											unsigned char *ReadBuffer_p,
                                  											unsigned int NumberToRead,
                                  											unsigned int *NumberWritten_p,
                                  											unsigned int *NumberRead_p,
                                  											CSSCI_Status *Status )
{
	CSAPI_RESULT Error = CSAPI_SUCCEED;
	unsigned int NumberWritten, NextWriteSize, NextReadSize, TotalRead;
	unsigned char Ins;

	/* Calculate number of bytes expected to be received */
	if (NumberToWrite == T0_CMD_LENGTH)
	{
		/* Extract number to read from P3 */
		NumberToRead = WriteBuffer_p[T0_P3_OFFSET];
	}
	else
	{
		/* Data is being sent, so no data response will be permitted */
		NumberToRead = 0;
	}	
    
	/* Set status INS byte */
	Ins = WriteBuffer_p[T0_INS_OFFSET];

	/* Initially we want to write the whole command; this then gets set by
	* the procedure bytes after the first write.
	*/
	NextWriteSize = T0_CMD_LENGTH;

	/* Should be updated by procedure bytes */
	NextReadSize = 0;
	
	/* Transmit data under the control of procedure bytes */
	while (NumberToWrite > 0 && Error == CSAPI_SUCCEED)
	{
	
		/* Transmit command */
		Error = CSSCI_Write( Smart_p, WriteBuffer_p,	NextWriteSize, &NumberWritten );		

		if (Error !=CSAPI_SUCCEED)
		{
			//printf("SCI_T0_Transfer1 timeout\n");
			return Error;		
		}

		/* Update pointers */
		WriteBuffer_p += NumberWritten;
		NumberToWrite -= NumberWritten;
		*NumberWritten_p += NumberWritten;
	           
		if (Error == CSAPI_SUCCEED)
		{
			/* Read procedure bytes */
			Error = CSSCI_T0_ProcessProcedureBytes(
			    Smart_p,
			    Ins,
			    NumberToWrite,
			    NumberToRead,
			    PB,
			    &Size,
			    &NextWriteSize,
			    &NextReadSize);
		}
	}

	TotalRead = 0;

	while (Error == CSAPI_SUCCEED && NextReadSize > 0 && NumberToRead > 0)
	{
		/* Read data response */

		Error = CSSCI_Read(Smart_p,
		                    &ReadBuffer_p[TotalRead],
		                    NextReadSize,
		                    NumberRead_p	);

	/* Handle procedure bytes */
		if (Error== CSAPI_SUCCEED)
		{
			NumberToRead -= *NumberRead_p;
			TotalRead += *NumberRead_p;
			NextReadSize = NumberToRead;
			Error = CSSCI_T0_ProcessProcedureBytes(
						Smart_p,
						Ins,
						0,
						NumberToRead,
					    PB,
			    		&Size,
						&NextWriteSize,
						&NextReadSize
						);
		}
		else
			return Error;
	}
	
	*NumberRead_p = TotalRead;

#ifdef CSSCI_PB
	Status->SW1 = PB[0];
	Status->SW2 = PB[1];
#endif	

	return Error;
} 


static CSAPI_RESULT CSSCI_T0_ProcessProcedureBytes(CSSCI_HANDLE  Smart_p,
		                                               unsigned char INS,
		                                               unsigned int WriteSize,
		                                               unsigned int ReadSize,
		                                               unsigned char *Buf_p,
		                                               unsigned char *Size_p,
		                                               unsigned int *NextWriteSize_p,
		                                               unsigned int *NextReadSize_p
   											          )
{
	CSAPI_RESULT Error = CSAPI_SUCCEED;
	CSAPI_RESULT CallError = CSAPI_SUCCEED;
	int MoreProcedureBytes;
	unsigned char P[500], i;
	unsigned int Sz, NextWriteSize;

	/* Assume no more data to be sent by IFD */
	NextWriteSize = 0;

	/* Assume that the next read is all available bytes */
	*NextReadSize_p = ReadSize;

	i = 0;                              /* Procedure byte count */
	do                                  /* Process each procedure byte */
	{
		/* Assume no more procedure bytes to come */
		MoreProcedureBytes = 0;

	/* Try to read an available procedure byte */
		#ifdef	C_ABV_card
			usleep(50000);	/* strange card needs it, modified by sunbin 20081204 */
		#endif
		CallError = CSSCI_Read(Smart_p, P, 1, &Sz);

	/* Check procedure byte */
		if (CallError == CSAPI_SUCCEED)       /* Do we have a procedure byte? */
		{
			
			/* Set the procedure byte in the status structure */
			Buf_p[i] = P[0];

			if (i == 0)                 /* First procedure byte? */
			{
			    /* Check for ACK byte */
				if ((INS ^ P[0]) == 0x00) /* ACK == INS */
				{

					/* Vpp should be set idle - all bytes can go */
					//CSSCI_DisableVpp(Smart_p);

					if (WriteSize > 0)
					{
						/* Send all remaining bytes */
						NextWriteSize = WriteSize;
					}
					else if (ReadSize == 0)
					{
						/* Await further procedure bytes */
						MoreProcedureBytes = 1;
						i = 0;
						continue;
					}
				}
				else if ((INS ^ P[0]) == 0xFF) /* ACK == ~INS */
				{

					/* Vpp should be set idle - one byte can go */
					//CSSCI_DisableVpp(Smart_p);

					if (WriteSize > 0)
					{
						/* Send the next byte only */
						NextWriteSize = 1;
					}
					else if (ReadSize == 0)
					{
						/* No more bytes available - await further bytes */
						MoreProcedureBytes = 1;
						i = 0;
						continue;
					}
					else
					{
						*NextReadSize_p = 1;
					}
				}
				else if ((INS ^ P[0]) == 0x01) /* ACK == INS+1 */
				{

					/* Vpp should be set active - all bytes can go */
					//CSSCI_EnableVpp(Smart_p);

					if (WriteSize > 0)
					{
						/* Send all remaining bytes */
						NextWriteSize = WriteSize;
					}
					else if (ReadSize == 0)
					{
						/* No more bytes available - await further bytes */
						MoreProcedureBytes = 1;
						i = 0;
						continue;
					}
				}
				else if ((INS ^ P[0]) == 0xFE) /* ACK == ~INS+1 */
				{

					/* Vpp should be set active - next byte can go   */
					//CSSCI_EnableVpp(Smart_p);

					if (WriteSize > 0)
					{
					/* Send the next byte only */
					NextWriteSize = 1;
					}
					else if (ReadSize == 0)
					{
						/* No more bytes available - await further bytes */
						MoreProcedureBytes = 1;
						i = 0;
						continue;
					}
					else
					{
						*NextReadSize_p = 1;
					}
				}
				else if ((P[0] & 0xF0) == 0x60) /* SW1 or NULL */
				{

					/* Get next procedure byte too */
					MoreProcedureBytes = 1;
					/* 0x60 is a 'null' byte, which indicates "please
					* wait". Therefore we shouldn't modify the bytes
					* expected when we get that. The others are errors.
					*/
					if (P[0] != 0x60)
					{
				//		*NextReadSize_p = 0;
					}

					switch (P[0])
					{
						case 0x60:      
						    i = 0;      /* New sequence of bytes to come */
						    continue;
						case 0x6E:      /* SW1 byte */
						    Error = CSAPI_FAILED;
						    break;
						case 0x61:      /* SW1 byte */
						    Error = CSAPI_SUCCEED;
						    break;
						case 0x6C:      /* SW1 byte */	/* added for ABC card by sunbin 20081122 */
						    Error = CSAPI_SUCCEED;
						    break;
						case 0x6A:      /* SW1 byte */	/* added for ABC card by sunbin 20081204 */
						    Error = CSAPI_SUCCEED;
						    break;
						case 0x63:      /* SW1 byte */	/* added for ABC card by sunbin 20081212 */
						    Error = CSAPI_SUCCEED;
						    break;
						case 0x69:      /* SW1 byte */	/* added for ABC card by sunbin 20081212 */
						    Error = CSAPI_SUCCEED;
						    break;
						case 0x6D:      /* SW1 byte */
						    Error = CSAPI_FAILED;
						    break;
						case 0x68:      /* SW1 byte */
						    Error = CSAPI_FAILED;
						    break;
						case 0x67:      /* SW1 byte */
						    Error = CSAPI_FAILED;
						    break;
						default:
						case 0x6F:      /* SW1 byte */
						    Error = CSAPI_FAILED;
						    break;
					}
				}
				else if ((P[0] & 0xF0) == 0x90) /* SW1 byte */
				{
					/* Next procedure byte is the final one */
					MoreProcedureBytes = 1;
   				//	 printf("SCI----------------------SW1 = 0x%x \n",P);
					*NextReadSize_p = 0;
				}
				else if(0 == P[0])
				{
					/* Unrecognised status byte */			
					Error = CSAPI_FAILED;
				}

				else
				{
					MoreProcedureBytes = 1;
					/* Unrecognised status byte */
					Error = CSAPI_FAILED;
				}
			}
			else
			{
			    /* This was SW2, so we should set the error status now */
				Buf_p[i] = P[0];
			}
		}
		else
		{
			/* No answer from the card */
			Error = CSAPI_FAILED;
			break;
		}
		
		i++;                        /* Next procedure byte */

	} while (MoreProcedureBytes);

    /* Set number of procedure bytes received */
    *Size_p = i;

    /* Set number of bytes to write in next IO transfer */
    *NextWriteSize_p = NextWriteSize;

    /* Make sure that any errors from any functions we called
     * get passed back up. 
     */
    if (Error == CSAPI_SUCCEED) 
    {
        Error = CallError;
    }
    /* Return error code */
    return Error;
		
} 


static CSAPI_RESULT CSSCI_T14_Transfer( CSSCI_HANDLE Smart_p,
                                  												unsigned char *WriteBuffer_p,
                                  												unsigned int NumberToWrite,
                                  												unsigned char *ReadBuffer_p,
                                  												unsigned int NumberToRead,
                                  												unsigned int *NumberWritten_p,
                                  												unsigned int *NumberRead_p )
{
	CSAPI_RESULT Error = CSAPI_SUCCEED;
	unsigned int payload_txlen, payload_rxlen, NumberRead_1=0, NumberRead_2=0;

	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) Smart_p;
	NumberToRead = NumberToRead;		/* don't care */
	
	if( WriteBuffer_p[0] != 0x01 ){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}

	payload_txlen = WriteBuffer_p[5];
	if( payload_txlen != (NumberToWrite - 7) ){
		dev_obj->errcode = SCI_ERROR_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
		
	/* Transmit command */
	Error = CSSCI_Write( Smart_p, WriteBuffer_p, NumberToWrite, NumberWritten_p );
	if (Error != CSAPI_SUCCEED){
			printf("CSSCI_T14_Transfer: CSSCI_Write error\n");
			return Error;		
	}

    if( NumberToWrite == *NumberWritten_p )
    {
		/* Read data response */
		
		Error = CSSCI_Read( Smart_p, ReadBuffer_p, 8, &NumberRead_1 );
		if ( (Error != CSAPI_SUCCEED) || (NumberRead_1 != 8) ){
			printf("CSSCI_T14_Transfer: CSSCI_Read error\n");
			*NumberRead_p = NumberRead_1;
			return Error;		
		}

		if( ReadBuffer_p[0] != 0x01 )
			printf("WARNING: The first rxdata is not Sync byte[0x01] in CSSCI_T14_Transfer -> CSSCI_Read\n");
		
		payload_rxlen = ReadBuffer_p[7];
		if( payload_rxlen >= 0 ){
			Error = CSSCI_Read( Smart_p, ReadBuffer_p+8, payload_rxlen+1, &NumberRead_2 );
			if ( (Error != CSAPI_SUCCEED) || (NumberRead_2 == 0) ){
				printf("CSSCI_T14_Transfer: CSSCI_Read error\n");
				*NumberRead_p += NumberRead_2;
				return Error;		
			}
		}

		*NumberRead_p = NumberRead_1 + NumberRead_2;

    }
		
	return Error;
} 


static CSAPI_RESULT CSSCI_T1_Transfer(CSSCI_HANDLE Smart_p,
                                 											unsigned char *Command_p,
                                 											unsigned int NumberToWrite,
                                 											unsigned int *NumberWritten_p,
                                 											unsigned char *Response_p,
                                 											unsigned int NumberToRead,
                                 											unsigned int *NumberRead_p,
                                 											unsigned char SAD,
                                 											unsigned char DAD )
{
	CSAPI_RESULT error = CSAPI_SUCCEED;
	
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) Smart_p;
	CSSCI_SCINFO *T1Details_p = &(dev_obj->sciinfo);		/* Shortcut for accessing details in Smart_p */
	
	SMART_BlockType_t ThisBlockType;    /* Type of current block */
	
    unsigned char TransmitDone = 0;      		/* Flags to control operations */
    unsigned char ReceiveDone = 0;
    unsigned char LastTxAcked = 0;
    unsigned char DoReceive = 0;

    SMART_T1Block_t TxBlock, RxBlock;   /* Transmission and reception blocks */
    SMART_T1Block_t RxTxBlock;      			/* used for transmitting blocks in receive
                                       								routine, so last tx'ed block doesn't get overwritten.  */    
    unsigned char LRC;                         /* EDC values */
    unsigned short CRC;
    unsigned char *WritePointer_p;    /* Pointer into Command_p */
    
    unsigned char BlockBuffer[MAXBLOCKLEN];	/* Used for make_u8_from_block (EDC checking). */    
    unsigned char TmpBuffer[MAXBLOCKLEN];		/* Used to be safe when receiving blocks while transmitting */
		
    unsigned int NumberLeft = NumberToWrite; /* Number left to write */

    /* Value in the struct might change between tx and r-block ack, meaning
     * we lose data. That's a Bad Thing. rxinfosize not used, so no need to
     * store it.
     */
    unsigned short TxMaxInfoStored = 0;

	*NumberWritten_p = 0;
	*NumberRead_p = 0;

    /* Variable setup */
    WritePointer_p = Command_p;
    T1Details_p->TxCount = 0;
    T1Details_p->Aborting = 0;

    /* Other than S(IFS), we shouldn't really receive anything in here */
    RxBlock.Buffer = TmpBuffer;

    /* Command -> response style, so we start off by dealing with
       sending the command */
    do {
        /* Build up the header - NAD, PCB, LEN, etc. */
        TxBlock.Header.NAD = SAD | (DAD << 4);
        TxBlock.Header.PCB = (T1Details_p->OurSequence << 6);

        /* See how long the remaining payload is, and therefore if we
         * have to chain */
        if (NumberLeft > T1Details_p->TxMaxInfoSize)
        {
            T1Details_p->State |= SMART_T1_CHAINING_US;
            TxBlock.Header.PCB |= I_CHAINING_BIT;
            TxBlock.Header.LEN = T1Details_p->TxMaxInfoSize;
        }
        else
        {
            T1Details_p->State &= ~SMART_T1_CHAINING_US;
            TxBlock.Header.LEN = NumberLeft;
        }

        /* Set up info section */
        TxBlock.Buffer = WritePointer_p;
        calculate_edc(Smart_p, &TxBlock);

        /* Transmit block */
        error = smart_t1_write_block(Smart_p, &TxBlock);
		 if( error != CSAPI_SUCCEED ){
				dev_obj->errcode = SCI_ERROR_T1_WRITEBLOCK;
				return CSAPI_FAILED;
		 }
		 
        TxMaxInfoStored = T1Details_p->TxMaxInfoSize;

        /* If chaining, wait for r-block */
        if ((T1Details_p->State & SMART_T1_CHAINING_US) && (error == CSAPI_SUCCEED))
        {
            DoReceive = 1;
			 *NumberWritten_p += TxMaxInfoStored;
        }
        else
        {
            DoReceive = 0;
            TransmitDone = 1;
			 *NumberWritten_p = NumberLeft;
        }

        while (DoReceive == 1)
        {
            unsigned char Valid = 0;
            unsigned int Length = 0;

            /* Default to one pass through the receive section */
            DoReceive = 0;
            error = smart_t1_read_block(Smart_p, &RxBlock);

            if (error == CSAPI_SUCCEED)
            {
                /* Calculate EDC */
                Valid = 0;
                if (T1Details_p->RC == 1)
                {
                    make_u8_from_block(&RxBlock, 1, BlockBuffer, &Length);
                    generate_crc(BlockBuffer, Length - 2, &CRC);
                    if (CRC == RxBlock.EDC_u.CRC)
                        Valid = 1;
                }
                else
                {
                    make_u8_from_block(&RxBlock, 0, BlockBuffer, &Length);
                    generate_lrc(BlockBuffer, Length - 1, &LRC);
                    if (LRC == RxBlock.EDC_u.LRC)
                        Valid = 1;
                }

                if (Valid == 1)
                {
                    /* See what we got back */
                    ThisBlockType = smart_get_block_type(RxBlock.Header.PCB);
                    if (ThisBlockType == T1_S_REQUEST)
                    {
                        TransmitDone = smart_process_s_request(Smart_p, &RxBlock);
                        /* Read another block, since processsreq. transmits a
                         * block
                         */
                        if (TransmitDone == 0)
                        {
                            error = smart_t1_read_block(Smart_p, &RxBlock);
                            if (error == CSAPI_SUCCEED)
                            {
                                ThisBlockType = smart_get_block_type(RxBlock.Header.PCB);
                            }
                            else
                            {
                                ThisBlockType = T1_CORRUPT_BLOCK;
                            }
                        }
                    }
                }
                else
                {
                    ThisBlockType = T1_CORRUPT_BLOCK;
                }
            }
            else
            {
                ThisBlockType = T1_CORRUPT_BLOCK;
            }

            /* If r-block received and ack okay, flip our seq. number;
             */
            if ((ThisBlockType == T1_R_BLOCK) &&
                (((RxBlock.Header.PCB & 0x10) >> 4) != T1Details_p->OurSequence) &&
                ((RxBlock.Header.PCB & R_EDC_ERROR) == 0) &&
                ((RxBlock.Header.PCB & R_OTHER_ERROR) == 0)
               )
            {
                NumberLeft -= TxMaxInfoStored;
                T1Details_p->OurSequence = NextSeq(T1Details_p->OurSequence);
                WritePointer_p += TxMaxInfoStored;
                T1Details_p->TxCount = 0;
            }
            else
            {
                /* Note retransmission */
                T1Details_p->TxCount++;

                /* How many attempts have we made? */
                if (T1Details_p->TxCount >= 3)
                {
                    error = smart_t1_resync(Smart_p, SAD, DAD);
                }
                else if (ThisBlockType == T1_CORRUPT_BLOCK)
                {
                    /* if corrupt, send an appropriate r-block */
                    RxTxBlock.Header.NAD = SAD | (DAD << 4);
                    if (T1Details_p->FirstBlock)
                        RxTxBlock.Header.PCB = 1;
                    else
                        RxTxBlock.Header.PCB = 1 | (T1Details_p->TheirSequence << 4);
                    RxTxBlock.Header.PCB |= R_BLOCK;
                    RxTxBlock.Header.LEN = 0;
                    RxTxBlock.Buffer = NULL;
                    calculate_edc(Smart_p, &RxTxBlock);

                    /* Transmit */
                    error = smart_t1_write_block(Smart_p, &RxTxBlock);
                    DoReceive = 1;
                }

            }
        }

    } while ((!TransmitDone) && (error == CSAPI_SUCCEED));

    T1Details_p->FirstBlock = 1;
    do {
        unsigned int Length; /* Amount copied to U8 buffer */

        /* The only way this could occur is an error left by the transmit section
         * above. Putting a break here avoids a goto, to skip this section.
         */
        if (error != CSAPI_SUCCEED)
            break;
        /* Check whether the user was expecting to read anything */
        if ((NULL == Response_p) || (NULL == NumberRead_p))
            break;

        /* Set the buffer to be within the user's buffer */
        RxBlock.Buffer = &Response_p[*NumberRead_p];

        error = smart_t1_read_block(Smart_p, &RxBlock);

        /* Calculate EDC */
        if (error == CSAPI_SUCCEED)
        {
            if (T1Details_p->RC == 1)
            {
                make_u8_from_block(&RxBlock, 1, BlockBuffer, &Length);
                generate_crc(BlockBuffer, Length - 2, &CRC);
            }
            else
            {
                make_u8_from_block(&RxBlock, 0, BlockBuffer, &Length);
                generate_lrc(BlockBuffer, Length - 1, &LRC);
            }
        }

        /* If EDC fails, transmit R-block requesting retransmission */
        if ((((T1Details_p->RC & 0x01) == 1) && (CRC != RxBlock.EDC_u.CRC)) ||
            (((T1Details_p->RC & 0x01) == 0) && (LRC != RxBlock.EDC_u.LRC)) ||
            (error != CSAPI_SUCCEED)
           )
        {
            /* Check how many times we've transmitted so far -
             * if it's less than 3, try it again, else start resync */
            if (T1Details_p->TxCount < 3)
            {
                /* Increase count, and retransmit */
                T1Details_p->TxCount++;

                /* Build r-block */
                RxTxBlock.Header.NAD = SAD | (DAD << 4);
                if ((error == SCI_ERROR_RX_TIMEOUT) && (T1Details_p->FirstBlock))
                    RxTxBlock.Header.PCB = 0;
                else
                    RxTxBlock.Header.PCB = (NextSeq(T1Details_p->TheirSequence) << 4);
                RxTxBlock.Header.PCB |= R_BLOCK;

                if (error == CSAPI_SUCCEED)
                    RxTxBlock.Header.PCB |= R_EDC_ERROR;
                else
                    RxTxBlock.Header.PCB |= R_OTHER_ERROR;

                RxTxBlock.Header.LEN = 0;
                RxTxBlock.Buffer = NULL;
                calculate_edc(Smart_p, &RxTxBlock);

                /* On some errors, should retransmit the last block we sent */
                if (error == SCI_ERROR_RX_TIMEOUT )
                {
                    /* The interface should send out an R Block instead of an I Block
                       if there is a receiving error such as frame or over-run. */
                    error = smart_t1_write_block(Smart_p, &TxBlock); /* R block */ 

                }
                else
                    /* Else request they retransmit theirs */
                    error = smart_t1_write_block(Smart_p, &RxTxBlock);
            }
            else
            {
                /* Resync */
                error = smart_t1_resync(Smart_p, SAD, DAD);
            }
        }
        else
        {
            /* See what kind of block this is */
            ThisBlockType = smart_get_block_type(RxBlock.Header.PCB);

            /* Process accordingly (expected types - s-request, i-block) */
            if ((ThisBlockType != T1_S_REQUEST) && (ThisBlockType != T1_I_BLOCK))
            {
                /* Retransmit */
                T1Details_p->TxCount++;

                /* How many times have we done this already? */
                if (T1Details_p->TxCount < 3)
                {
                    if (ThisBlockType == T1_R_BLOCK)
                    {
                        /* See if the last block we sent out is what they're
                         * requesting. */
                        if (T1Details_p->OurSequence != ((RxBlock.Header.PCB >> 4) & 1))
                        {
                            error = smart_t1_write_block(Smart_p, &RxTxBlock);
                        }
                        else
                        {
                            if (LastTxAcked)
                                error = smart_t1_write_block(Smart_p, &RxTxBlock);
                            else
                                error = smart_t1_write_block(Smart_p, &TxBlock);
                        }
                    }
                    else
                    {
                        /* It's all gone horribly wrong. Panic. */
                        if (LastTxAcked)
                            error = smart_t1_write_block(Smart_p, &RxTxBlock);
                        else
                            error = smart_t1_write_block(Smart_p, &TxBlock);
                    }
                }
                else
                {
                    smart_t1_resync(Smart_p, SAD, DAD);
                }
            }
            else
            {
                /* Valid block type, continue... */

                /* If s-request, process */
                if (ThisBlockType == T1_S_REQUEST)
                {
                    /* This function also transmits the response */
                    ReceiveDone = smart_process_s_request(Smart_p, &RxBlock);
                }
                else
                {
                    /* Check if this block has the expected seq. number */
                    if (((RxBlock.Header.PCB & I_SEQUENCE_BIT) >> 6) != T1Details_p->TheirSequence)
                    {
                        unsigned int Number = 0;

                        /* Seems valid ... copy data into the buffer,
                         * increase bytesread
                         */
                        if ((RxBlock.Header.LEN < (NumberToRead - *NumberRead_p)) ||
                            (NumberToRead == 0)
                           )
                            Number = RxBlock.Header.LEN;
                        else
                            Number = (NumberToRead - *NumberRead_p);

                        *NumberRead_p += Number;

                        T1Details_p->TxCount = 0;

                        /* Set their last received seq. number */
                        T1Details_p->TheirSequence =
                            ((RxBlock.Header.PCB & I_SEQUENCE_BIT) >> 6);

                        if (LastTxAcked == 0)
                        {
                            LastTxAcked = 1;
                            T1Details_p->OurSequence = NextSeq(T1Details_p->OurSequence);
                        }

                        if (T1Details_p->FirstBlock == 1)
                            T1Details_p->FirstBlock = 0;

                        /* Is this block chaining? */
                        if ((RxBlock.Header.PCB & I_CHAINING_BIT) != 0)
                        {
                            /* yes; send an r-block with their next
                             * expected no.  (block must have been
                             * received properly to get this far)
                             */
                            RxTxBlock.Header.PCB = R_BLOCK |
                                    (NextSeq(T1Details_p->TheirSequence) << 4);
                            RxTxBlock.Header.NAD = SAD | (DAD << 4);
                            RxTxBlock.Header.LEN = 0;
                            RxTxBlock.Buffer = NULL;
                            calculate_edc(Smart_p, &RxTxBlock);

                            error = smart_t1_write_block(Smart_p, &RxTxBlock);
                        }
                        else
                        {
                            /* Not chaining (now) */
                            ReceiveDone = 1;
                        }
                    }
                    else
                    {
                        /* Case - we received valid i-block with the
                         * same seq. number we got last; therefore they
                         * didn't get our block. Request retransmission
                         * of the missing block.
                         */
                        RxTxBlock.Header.NAD = SAD | (DAD << 4);
                        RxTxBlock.Header.PCB = R_BLOCK;
                        RxTxBlock.Header.PCB |= NextSeq(T1Details_p->TheirSequence);
                        RxTxBlock.Header.LEN = 0;
                        RxTxBlock.Buffer = NULL;
                        calculate_edc(Smart_p, &RxTxBlock);
                        error = smart_t1_write_block(Smart_p, &RxTxBlock);
                    }
                }
            }
        }

    } while ((!ReceiveDone) &&
             ((NumberToRead == 0) || (*NumberRead_p < NumberToRead)) &&
             (error == CSAPI_SUCCEED));

    return error;
}

/*End of File */
