/****************************************************************
*
* FILENAME
*	casdrv_def.h
*
* PURPOSE 
*	Cas Driver Define Header file
*	
*
* AUTHOR
*	Jacob
*
* HISTORY
*  Status                            Date              Author
*  Create                         23.04.2008           Jacob
*
****************************************************************/
#ifndef __CASDRVDEF_H
#define __CASDRVDEF_H

/****************************************************************
*	Include Values                                              *
*****************************************************************/
#include "linuxdefs.h"

/****************************************************************
 *                          define                              *
 ****************************************************************/
#define SHARE_CAM_ENABLE

#define DebugPrint(x)                    printf x
#define DebugPrt(x)                      printf x

#define CAS_TABLE_CAT                   0x01
#define CAS_TABLE_ECM                   0x80
#define CAS_TABLE_EMM                   0x82

/* For Smart Card */
#define MAX_CARD_SLOT                   1
#define SMART_CARD_MODE_T0              0
#define SMART_CARD_MODE_T1              1
#define SMART_CARD_MODE_T14             14

#define  CAS_ID_LENGTH                  2
#define  CAS_NAME_LENGTH                20
#define  CAS_PROVIDER_ID_LENGTH         4
#define  CAS_PROVIDER_NAME_LENGTH       20
#define  CAS_KEY_LENGTH                 8

#define SMART_CARD_REMOVED              0
#define SMART_CARD_INSERTED             1
#define SMART_CARD_REINIT               2


/* 9F Data move to Key Db Jacob 26 Oct. 2008 */
#define N2_9F_DATA_SIZE                  0x400

#define CAS_ID_SECA                      0x01
#define CAS_ID_VIA                       0x05  
#define CAS_ID_IRDETO                    0x06
#define CAS_ID_CONAX                     0x0B
#define CAS_ID_CRYPTO                    0x0D
#define CAS_ID_BETA                      0x17
#define CAS_ID_NAGRA                     0x18
#define CAS_ID_BISS                      0x26
#define CAS_ID_TPS                       0xF0 /* Dummy Id */
#define CAS_ID_DCW                       0xF2 /* Dummy Id */

#define INVALID_CAS_ID                  0x0000
#define SECA_CAS_ID                     0x0100
#define VIACCESS_CAS_ID                 0x0500
#define VIACCESS2_CAS_ID                0x0501
#define IRDETO_CAS_ID                   0x0600
#define NDS_CAS_ID                      0x0900
#define CONAX_CAS_ID                    0x0B00
#define POWERVU_CAS_ID                  0x0E00
#define BETA_CAS_ID                     0x1700
#define NAGRA_CAS_ID                    0x1800
#define NAGRA_UNLOCK_ID                 0x18FF
#define NAGRA2_CAS_ID                   0x1801
/* For New DN By Jacob 25 Oct. 2007 */
#define BEV_N2_CAS_ID                   0x1234
#define CRYPTOWORKS_CAS_ID              0x0D00
#define BISS_CAS_ID                     0x2600
#define XCRYPT_CAS_ID                   0x4AD1
#define FIRECRYPT_CAS_ID                0x4A70 // for FireCrypt by Jacob 15 Aug. 2006
#define ALL_SECA_CAS_ID                 0xBEEF
#define FUN_IRDETO_CAS_ID               0xEFFF
#define TPS_CAS_ID                      0xF000
#define DCW_CAS_ID                      0xF200
#define UNKNOWN_CAS_ID                  0xFFFF

#define DEMUX_INVALID_PID                0x1FFF

/* CW Stream Type */
#define	CAS_DRV_VIDEO                    0
#define CAS_DRV_AUDIO                    1
#define CAS_DRV_TELETEXT                 2
#define CAS_DRV_DAUDIO                   3

#define CAS_CW_EVEN_VALID                0x10
#define CAS_CW_ODD_VALID                 0x20

/* Cas Key type */
#define CAS_KEY_TYPE_DES                 0x00
#define CAS_KEY_TYPE_PK0                 0x10  /* For N1 PK0 Key */
#define CAS_KEYTYPE_ECM_MOD              0x20  /* For N2, CW & Conax */
#define CAS_KEYTYPE_ECM_EXP              0x30  /* For N2, CW & Conax */
#define CAS_KEYTYPE_ECM_06               0x40  /* 6 Bytes Key for Crypto Works CW */
#define CAS_KEYTYPE_EMM_MOD              0x50
#define CAS_KEYTYPE_EMM_EXP              0x60
#define CAS_KEYTYPE_EMM_I_KEY            0x70  /* For N2             */
#define CAS_KEYTYPE_EMM_S_KEY            0x80  /* For N2             */
#define CAS_KEYTYPE_EMM_V_KEY            0x90  /* For N2             */
#define CAS_KEYTYPE_N1_MOD1              0xA0  /* For N1             */
#define CAS_KEYTYPE_N1_MOD2              0xB0  /* For N1             */
#define CAS_KEYTYPE_N1_EXP               0xC0  /* For N1             */
#define CAS_KEYTYPE_INVALID              0xFF  /* Invalid Key number */

#define MAX_NUMBER_OF_SOURCE             4

#define KEY_NO_SKIP                      0

/* For share data By Jacob 07 Nov. 2008 */
#define PLUS_SERIAL_KEY                  0
#define PLUS_REGIST_CODE                 1

/* For BISS Key By Jacob 31 Jan. 2011 */
#define BISS_PROVIDER_TP_HOR             0x80000000
#define BISS_PROVIDER_TP_VER             0x00000000
#define BISS_KEY_LENGTH                  20

#define MAX_RX_TIMEOUT                   65535
#define MAX_TX_TIMEOUT                   65535

#define boolean                          BOOL
#define false                            FALSE
#define true                             TRUE

/* For OSCAM Menu by KB Kim 2011.04.22 */
#define MAX_SERVER_DATA                 20
#define SERVER_LABEL_LENGTH             64
#define SERVER_DEVICE_LENGTH            128
#define SERVER_DEVICE_LENGTH            128
#define SERVER_USER_ID_LENGTH           64
#define SERVER_PASSWORD_LENGTH          SERVER_USER_ID_LENGTH
#define SERVER_DESKEY_LENGTH            32
#define SERVER_PROTOCOL_LENGTH          16
#define SERVER_ETC_LENGTH               16
#define SERVER_NANO_LENGTH              1024
#define OSCAM_SERVER_READ_BUFFER        4096
#define OSCAM_CONFIG_FOLDER             "/usr/work1/"
#define OSCAM_SERVER_FILE               "oscam.server"
#define OSCAM_TEMP_FILE                 "/tmp/tmposcam"
#define OSCAM_DEFAULT_DES_KEY           "0102030405060708091011121314"
#define OSCAM_DEFAULT_LABEL             "my_server_"
#define SERVER_LABEL_VALID              0x01
#define SERVER_DEVICE_VALID             0x02
#define SERVER_PROTOCOL_VALID           0x04
#define SERVER_USERID_VALID             0x08
#define SERVER_PASSWORD_VALID           0x10
#define SERVER_ENABLE_VALID             0x20
#define SERVER_DESKEY_VALID             0x40
#define SERVER_ALL_DATA_VALID           0x7F
#define SERVER_DATA_ERROR               0x80000000

/****************************************************************
 *	Type Define                                                 *
 ****************************************************************/
/* For OSCAM Menu by KB Kim 2011.04.22 */
typedef struct OscamServerInfo_s
{
	U8 Label[SERVER_LABEL_LENGTH];
	U8 Protocol[SERVER_PROTOCOL_LENGTH];
	U8 Url[SERVER_DEVICE_LENGTH];
	U8 R_Port[6];
	U8 L_Port[6];
	U8 UserId[SERVER_USER_ID_LENGTH];
	U8 PassWord[SERVER_PASSWORD_LENGTH];
	U8 DesKey[SERVER_DESKEY_LENGTH];
	U16 R_PortVal;
	U16 L_PortVal;
	U8  Enable;
	U8  Connection;
	U8  ProtocolValue;
	U8  Delete;
} OscamServerInfo_t;

typedef enum
{
	TABLE_NULL,
	TABLE_CAT,
	TABLE_ECM,
	TABLE_EMM
} TableType_t;

typedef struct {
	int SatPosition;
	U16 Ts_Id;
	U16 Network_Id;
	U16 Service_Id;
	U16 Pmt_id;
	U16 V_id;
	U16 A_Id;
} ChannelInfo_t;

typedef struct CasBissInfo_s
{
	U32                     TpFreq;
	U32                     Symbol;
	U16                     ServiceId;
	U16                     VideoPid;
	U8                      Pol;
	char                    ChannelName[CAS_PROVIDER_NAME_LENGTH];
} CasBissInfo_t;

typedef enum 
{
	SMART_EVENTNONE = 0,
	SMART_CARDINS   = 10,
	SMART_CARDREM   = 11,
	SMART_RXTIMEOUT = 12,
	SMART_ATRRDY    = 13,
	SMART_READRDY   = 14,
	SMART_WRITERDY  = 15,
	SMART_READERROR = 16, /* need to add events for read and write size notifications */
} SMARTEvent_t;

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *	Function Prototypes                                         *
 ****************************************************************/
#endif /* #ifndef __CASDRVDEF_H */
