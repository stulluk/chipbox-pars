/****************************************************************
*
* FILENAME
*	casapi.h
*
* PURPOSE 
*	Cas Driver Key Data Header
*	
*
* AUTHOR
*	Jacob
*
* HISTORY
*  Status                            Date              Author
*  Create                        14 July 2005          Jacob
*
****************************************************************/
#ifndef  _CAS_API_H_INCLUDED
#define  _CAS_API_H_INCLUDED
/****************************************************************
 *                       include files                          *
 ****************************************************************/
#include "casdrv_def.h"
#include "caskey.h"

/****************************************************************
 *                          define                              *
 ****************************************************************/
#define SC_USED
  // #define SC_NAGRA
  // #define USA_NAGRA2
#define SC_CARD_USED

#ifdef SHARE_CAM_ENABLE
	#define SC_SHARE_USED
#endif

#include "cascard.h"

#define boolean BOOL
/****************************************************************
 *	                    Type Define                             *
 ****************************************************************/

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
#ifdef SHARE_CAM_ENABLE
/* For S_CAM Menu By Jacob 14 May 2011 */
extern void StbSetOscamStatus(U8 status);
extern U8 StbSGetOscamStatus(void);

extern int CasDrvSendMessageToOscam(int length, unsigned char *data);
extern int CasDrvRecvMessageFromOscam(unsigned char *data);
extern BOOL CasDrvInitOscam(void);

/* For OSCAM Menu by KB Kim 2011.04.22 */
extern int CasDrvReadOscamServerData(OscamServerInfo_t *ServerData);
/* For OSCAM Menu by KB Kim 2011.04.22 */
extern int CasDrvWriteOscamServerData(OscamServerInfo_t *serverData, int number, int newNumber);
#endif

extern U8 StbCheckEmmTimeOut(U32 time);
extern void StbSetEmmTime(void);
extern U8  StbFtaStatus(void);
extern U8  StbCardStatus(void);
extern void StbSmartCartAccessIn(void);
extern void StbSmartCartAccessOut(void);

extern BOOL StbSmartWrite (U8   cardSlot,
                           U8  *buffer,
                           U32  numberToWrite,
                           U32 *numberActualWritten,
                           U32  timeout);
extern BOOL StbSmartRead (U8   cardSlot,
                          U8  *buffer,
                          U32  numberToRead,
                          U32 *numberActualRead,
                          U32  timeout);
extern BOOL StbSmartIO(U8   cardSlot,
				       U8   cardProtocol,
				       U8  *writeData,
				       U32  numberToWrite,
				       U32 *numberActualWritten,
				       U8  *response,
				       U32  numberToRead,
				       U32 *numberActualRead,
				       U16 *result);
extern void StbSetCW (U16 sourceId, U8 Stream_Type, U8 *EvenKey, U8 *OddKey);
extern BOOL CasDrvPauseTable (U16 sourceId, U8 tableInfoId);
extern BOOL CasDrvStopTable (U16 sourceId, U8 tableInfoId);
extern BOOL CasdrvStartTable (U8             sourceId,
					          TableType_t    type,
					          U16            pid,
					          U8             numberOfFilter,
					          U8            *filterData,
					          U8            *filterMask,
					          U8            *tableSlot);
extern BOOL CasDrvGetChanneInfo(U16 channelId, CasBissInfo_t *chInfo);
extern BOOL CasDrvStopChannel (U8 sourceId);
extern BOOL CasDrvStartNewChannel  (U8 sourceId, U16 dmxId, U16 channelId, U16 serviceId, U16 vPid, U16 aPid);
extern BOOL CasDrvNotifyPMT(U8 sourceId, U8 *PmtData);
extern BOOL CasDrvStopChannel (U8 sourceId);
extern void CasDrvEngineControl(BOOL ftaOn, BOOL cardOn);

extern void CasGetSystemInfo (U16 casId, U8 *casName);
extern BOOL CasDrvDeleteAllCasKey(void);
extern BOOL CasDrvDeleteKey (U8 *casId, U8 *providerId, unsigned char Key_Number, unsigned char KeySkip);
extern BOOL CasDrvDeleteProvider (U8 *casId, U8 *providerId);
extern BOOL CasDrvDeleteCas (U8 *casId);
extern void CasDrvSaveKey2Flase(void);
extern BOOL CasDrvUpdateProvider (U8 *casId, U8 *providerId, unsigned char *providerName);
extern BOOL CasDrvUpdateKey (U8 *casId, U8 *providerId, U8 *providerName, U8 keyNumber, U8 keyLength, U8 *keyData);
extern BOOL CasDrvUpdateBissKey (CasBissInfo_t bissInfo, U32 *provider, U8 *keyNumber, U8 *key, U8 keyLength);
extern U8 CasDrvGetKey (U8 *casId, U8 *providerId, U8 Key_Number, U8 KeySkip, U8 *Key);
extern BOOL CasDrvGetBissKey (CasBissInfo_t bissInfo, U8 *key, U8 *keyLength);
extern void CasDrvDelete9FData(void);
extern void CasDrvSet9FData(U8 *data, U32 size);
extern U8 *CasDrvGetUsa9fData(void);
extern BOOL CasDrvResetKeyDb(void);
extern BOOL CasDrvInitKeyDb (void);
extern void CasDrvGetKeyDbInfo(CasDbInfo_t *casDbInfo);
extern int CasDrvInit(void);

extern U16 CasGetCurrentCardId(U8 slot);
extern U8	CasDrvGetCardStatus(U8 slot);
extern BOOL CasDrvSmartGetAtr(U8 cardSlot, U8 *atr, U32 *length);
extern BOOL CasDrvSmartGetHistory(U8 cardSlot, U8 *history, U32 *length);
extern BOOL CasDrvSmartFlush (U8 cardSlot);

#endif // #ifndef  _CAS_API_H_INCLUDED

