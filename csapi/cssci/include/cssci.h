#ifndef __CSAPI_SCI_H__
#define __CSAPI_SCI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

#define T0_CMD_LENGTH           5
#define T0_CLA_OFFSET           0
#define T0_INS_OFFSET           1
#define T0_P1_OFFSET            2
#define T0_P2_OFFSET            3
#define T0_P3_OFFSET            4
#define T0_RETRIES_DEFAULT      27			/*11011	5-3:2-0	Read  = 3  Write = 3*/

typedef void *CSSCI_HANDLE;

typedef enum {
	SCI_SUCCESS = 0,
	SCI_ERROR_OPEN,
	SCI_ERROR_CLOSE,
	SCI_ERROR_IOCTL,
	SCI_ERROR_BAD_PARAMATER,
	SCI_ERROR_INVALID_HANDLE,
	SCI_ERROR_UNKNOWN_DEVICE_NAME,
	SCI_ERROR_DEVICE_BUSY,
	SCI_ERROR_READ,
	SCI_ERROR_WRITE,
	SCI_ERROR_ACTIVE,
	SCI_ERROR_DEACTIVE,
	SCI_ERROR_SET_BAUDRATE,
	SCI_ERROR_SET_BLOCKGUARD,
	SCI_ERROR_GET_BLOCKGUARD,
	SCI_ERROR_SET_CONVENTION,
	SCI_ERROR_SET_PARITY_ERROR_DETECT,
	SCI_ERROR_SET_PARITY,
	SCI_ERROR_SET_TXRETRY,
	SCI_ERROR_SET_RXRETRY,
	SCI_ERROR_SET_RXTIMEOUT,
	SCI_ERROR_SET_CHGUARD,
	SCI_ERROR_SET_PROTOCOL,			/* Added */
	SCI_ERROR_RESET,
	SCI_ERROR_GET_BAUDRATE,
	SCI_ERROR_GET_TXRETRY,
	SCI_ERROR_GET_RXRETRY,
	SCI_ERROR_GET_RXTIMEOUT,
	SCI_ERROR_GET_CHGUARD,
	SCI_ERROR_GET_PROTOCOL,
	SCI_ERROR_INVALID_PARAMETERS,
	SCI_ERROR_GET_WRSIZE,
	SCI_ERROR_GET_RDSIZE,
	SCI_ERROR_REGISTER_CALLBACK,
	SCI_ERROR_NOTOPEN_DEVICE,
	SCI_ERROR_SET_RXTHRESHOLD,
	SCI_ERROR_SET_TXTHRESHOLD,
	SCI_ERROR_GET_RXTHRESHOLD,
	SCI_ERROR_GET_TXTHRESHOLD,
	SCI_ERROR_RXFIFO_REPORT,
	SCI_ERROR_TXFIFO_REPORT,
	SCI_ERROR_SET_CLOCK,
	SCI_ERROR_GET_CLOCK,
	SCI_ERROR_RX_TIMEOUT,
	SCI_ERROR_TX_TIMEOUT,
	SCI_ERROR_T1_WRITEBLOCK,
	SCI_ERROR_BLOCK_RETRIES,
	SCI_ERROR_NO_CARD,
	SCI_ERROR_NOATRRECEIVED
} CSSCI_ErrCode;

typedef struct{
    unsigned char SW1;
    unsigned char SW2;
} CSSCI_Status;	/* For T1: Status->SW1 use as SAD, Status->SW2 use as DAD */

typedef enum {
	IRDETO_V41L = 0,	/*Irdeto smart card version <=4.1 */
	IRDETO_V41_V50 = 1,	/*Irdeto smart card, 4.1 < version < 5.0 */
	IRDETO_V50G = 2,	/*Irdeto smart card, version >= 5.0 */
	ISO_COMPLAINT = 3	/* IEC / ISO 7816-3 complaint */
} CSSCI_SCTYPE;


/*Added */
typedef enum CSSCI_SCIEVENT_ {
	CSSCI_CARDINS = 10,
	CSSCI_CARDREM = 11,
	CSSCI_RXTIMEOUT = 12,
	CSSCI_ATRRDY = 13,
	CSSCI_READRDY = 14,
	CSSCI_WRITERDY = 15,
	CSSCI_READERROR = 16, /* need to add events for read and write size notifications */
} CSSCI_SCIEVENT;

#define CSSCI_DEFAULT_SC_TYPE ISO_COMPLAINT


CSSCI_HANDLE CSSCI_Open(void);
CSAPI_RESULT CSSCI_Close(CSSCI_HANDLE handle);

CSAPI_RESULT CSSCI_Activate(CSSCI_HANDLE handle);
CSAPI_RESULT CSSCI_Deactivate(CSSCI_HANDLE handle);
CSAPI_RESULT CSSCI_Reset(CSSCI_HANDLE handle, unsigned char **atr, unsigned int *atr_len);
CSAPI_RESULT CSSCI_GetATR(CSSCI_HANDLE handle, unsigned char **atr, unsigned int *atr_len);

CSAPI_RESULT CSSCI_SetRxTimeout(CSSCI_HANDLE handle, unsigned short rx_timeout);
CSAPI_RESULT CSSCI_GetRxTimeout(CSSCI_HANDLE handle, unsigned short *rx_timeout);

CSAPI_RESULT CSSCI_SetCHGuardDelay(CSSCI_HANDLE handle, unsigned int guard_delay);
CSAPI_RESULT CSSCI_GetCHGuardDelay(CSSCI_HANDLE handle, unsigned char *guard_delay);

/* T0: guard_delay >= 16 (etu) T1: guard_delay >= 22 (etu) */
CSAPI_RESULT CSSCI_SetBlockGuardDelay(CSSCI_HANDLE handle, unsigned int guard_delay);
CSAPI_RESULT CSSCI_GetBlockGuardDelay(CSSCI_HANDLE handle, unsigned char *guard_delay);

/*
T0: retry times <= 3;
T1: is_error_detect must set 0; 
parity_value: 0 even parity, 1 odd parity;
is_error_detect: 0 disable, 1 enable 
*/
CSAPI_RESULT CSSCI_Set_Parity_Manage(	CSSCI_HANDLE 	handle, 
																						unsigned char 	parity_value,
																						unsigned char 	is_error_detect,
																						unsigned char 	rx_retry_times,
																						unsigned char 	tx_retry_times	);
CSAPI_RESULT CSSCI_Get_Parity_Manage(	CSSCI_HANDLE 	handle, 
																						unsigned char 	*parity_value,
																						unsigned char 	*is_error_detect,
																						unsigned char 	*rx_retry_times,
																						unsigned char 	*tx_retry_times	);

CSAPI_RESULT CSSCI_Read(CSSCI_HANDLE handle, unsigned char *buffer, unsigned int len, unsigned int *readedlen);
CSAPI_RESULT CSSCI_Write(CSSCI_HANDLE handle, unsigned char *buffer, unsigned int len, unsigned int *writtedlen);
/* For T1: Status->SW1 use as SAD, Status->SW2 use as DAD */
CSAPI_RESULT CSSCI_Transfer( CSSCI_HANDLE handle,
                             								unsigned char *cmd,
                             								unsigned int  cmd_len,
                            								unsigned char *response,
                             								unsigned int response_len,
                              								unsigned int *written_len,
                              								unsigned int *responsed_len,
                             								CSSCI_Status *Status	);

CSAPI_RESULT CSSCI_SetSCType(CSSCI_HANDLE handle, CSSCI_SCTYPE type);

CSAPI_RESULT CSSCI_SetNotify(CSSCI_HANDLE handle, void (*call_back_function)(CSSCI_HANDLE, CSSCI_SCIEVENT *), 
																CSSCI_SCIEVENT sci_event, unsigned char event_enable);

/* how many bytes we could read from the rxfifo */
CSAPI_RESULT CSSCI_GetReadSize(CSSCI_HANDLE handle, unsigned char *rd_size);
/* how many bytes wo could write into the txfifo */
CSAPI_RESULT CSSCI_GetWriteSize(CSSCI_HANDLE handle, unsigned char *wr_size);

CSAPI_RESULT CSSCI_SetReadReadyNotifySize(CSSCI_HANDLE handle, unsigned int Rx_Threshold);
CSAPI_RESULT CSSCI_GetReadReadyNotifySize(CSSCI_HANDLE handle, unsigned int *Rx_Threshold);
CSAPI_RESULT CSSCI_SetWriteReadyNotifySize(CSSCI_HANDLE handle, unsigned int Tx_Threshold);
CSAPI_RESULT CSSCI_GetWriteReadyNotifySize(CSSCI_HANDLE handle, unsigned int *Tx_Threshold);
CSAPI_RESULT CSSCI_RxFIFO_Report(CSSCI_HANDLE handle, unsigned char report);
CSAPI_RESULT CSSCI_TxFIFO_Report(CSSCI_HANDLE handle, unsigned char report);

CSAPI_RESULT CSSCI_Set_SC_CLK(CSSCI_HANDLE handle, unsigned int freq);
CSAPI_RESULT CSSCI_Get_SC_CLK(CSSCI_HANDLE handle, unsigned int *freq);

CSSCI_ErrCode CSSCI_GetErrCode(CSSCI_HANDLE handle);
char *CSSCI_GetErrString(CSSCI_HANDLE handle);



#ifdef __cplusplus
}
#endif

#endif				/* __CSAPI_SCI_H */
