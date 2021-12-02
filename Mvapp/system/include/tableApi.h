/****************************************************************
*
* FILENAME
*	tableApi.h
*
* PURPOSE 
*	Table Process API Function Define
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2010.08.01           KB
*
****************************************************************/
#ifndef __SI_TABLE_API_H
#define __SI_TABLE_API_H

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include "table_def.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#define TABLE_TDT_TOT_PID         0x14
#define TABLE_EIT_PID             0x12

#define SI_TDT_TABLE_ID			  0x70
#define SI_TOT_TABLE_ID			  0x73
#define SI_EIT_PF_ACTUAL_TABLE_ID          0x4e
#define SI_EIT_PF_OTHER_TABLE_ID           0x4f
#define SI_EIT_SC_ACTUAL_FIRST_TABLE_ID    0x50
#define SI_EIT_SC_ACTUAL_LAST_TABLE_ID     0x5F

/****************************************************************
 *                       Type define                            *
 ****************************************************************/

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
extern U32 SiCodeConverter(U8 *dest, U8 *src, U32 len, U16 *code);
#ifdef USA_SI_TABLE_SUPPORT
extern void SiStartMultiSatSearch(void);
extern void SiStopMultiSatSearch(void);
extern void SiStartSatSearch(void);
extern void SiStopSatSearch(void);
extern void SiSetScanMode(U8 ScanMode, U8 usaLongNameMode);
#endif /* #ifdef USA_SI_TABLE_SUPPORT */

extern BOOL SiGetTableInfo(U8 tableInfoId, U8 *tableId, U8 *tunerId, U8 *channelMode, U16 *channelId);

extern void SiUnRegisterNitCallBack(void);
extern void SiUnRegisterLiveSearchCallBack(void);
extern void SiUnRegisterSearchCallBack(void);
extern void SiRegisterNitCallBack(NitResult_f callback);
extern void SiRegisterLiveSearchCallBack(LiveSearchResult_f callback);
extern void SiRegisterSearchCallBack(SearchResult_f callback);
extern BOOL SiStopLiveSection(U8 tableInfoId);
extern BOOL SiPauseLiveSection(U8 tableInfoId);
extern BOOL SiStartLiveSection(U8 *infoId,
						   U16 pid,
						   U16 tableId,
						   U8  tunerId,
						   U8  channelMode,
						   U8  numberOfFilter,
						   U8  crcEnable,
						   U8  *matchData,
						   U8  *matchMask,
						   U8  *notMask,
						   DemuxCallback_f callBack);
extern U32 SiGetLivePmtData(U8 *buffer);
extern void SiStopLiveSearch(void);
extern void SiStartLiveSearch(U8 tunerId, U8 channelMode, U16 serviceId);
extern void SiStopSearchChannel(void);

/* For TP NIT Search by KB Kim 11 Jan 2011 */
extern void SiStartSearchChannel(U8 tunerId, U8 nitMode, U8 tpModeOn, U8 channelMode);

extern BOOL SiInitTable(void);
extern void SiTermTable(void);

#ifdef __cplusplus
}
#endif

#endif /* __SI_TABLE_API_H */
