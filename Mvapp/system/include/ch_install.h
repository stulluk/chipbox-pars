#ifndef __CH_INSTALL_H
#define __CH_INSTALL_H

#include "database.h"
#include "sattp.h"

/* For Blind Scan By KB Kim 2011.02.26 */
#include "fe_mngr.h"

#define kCS_INSTALL_MAX_NO_OF_NOTIFY_CLIENTS              3

enum
{
    eCS_INSTALL_HP = 0x01,
    eCS_INSTALL_LP = 0x02	
};

typedef enum 
{
    eCS_INSTALL_SERVICEINFO = 0,
    eCS_INSTALL_TPINFO,
    eCS_INSTALL_MAX_SERVICE_NUMBER_REACHED,
    eCS_INSTALL_COMPLETE,
    eCS_INSTALL_NONE
}tCS_INSTALL_Notification;


typedef enum
{
    eCS_INSTALL_STATE_SCANNING = 0,
	/* For Blind Scan By KB Kim 2011.02.26 */
	eCS_INSTALL_STATE_BLIND_SCANNING,
    eCS_INSTALL_STATE_NONE
}tCS_INSTALL_State;


typedef enum
{
    eCS_INSTALL_EXACT_APPEND = 0,
    eCS_INSTALL_EXACT_PID_APPEND,
    eCS_INSTALL_NETWORK_APPEND
}tCS_INSTALL_ScanMode;

typedef enum
{
    eCS_INSTALL_ALL_SERVICE = 0,
    eCS_INSTALL_SCRAMBLED_SERVICE,
    eCS_INSTALL_FREE_SERVICE
}tCS_INSTALL_ServiceMode;

typedef struct
{
    tCS_INSTALL_ScanMode		ScanMode;
    tCS_INSTALL_ServiceMode		ServiceMode;
}tCS_INSTALL_Mode;

typedef struct 
{
    U16					u16TP_Index[MAX_TP_COUNT];
    U16					TP_Num;
}st_INSTALL_TPList;

typedef struct Install_ServiceData_Struct
{
	U16									u16TP_Index;
	U16									Service_ID ;
	U16									PMT_PID;
	U16									Video_PID;
	U16									Audio_PID;
	U16									PCR_PID;
#ifdef FOR_USA
	U16									LCN;
#endif
	tCS_DB_ServiceType					ServiceType;
	tCS_DB_AudioType					Audio_Type;
	tCS_DB_VideoType					Video_Type;
	U8									ServiceScramble;
	char								ServiceNames[MAX_SERVICE_NAME_LENGTH];
    
	struct Install_ServiceData_Struct 	*pNext_Service;
}	tCS_INSTALL_ServiceInfo;

typedef struct
{
	U8									u8ServiceType;
	U8									u8ServiceScramble;
	char								acServiceNames[MAX_SERVICE_NAME_LENGTH];
}	tMV_Display_ServiceInfo;

typedef struct 
{
    U16       ONID;
    U16       TSID;
    U16	Service_Num;
    tCS_INSTALL_ServiceInfo*  pServiceListHead;
    tCS_INSTALL_ServiceInfo*  pCurrent_Service;
}tCS_INSTALL_ServiceList;

typedef void (*tCS_INSTALL_NotificationFunction)( tCS_INSTALL_Notification notification );

BOOL CS_INSTALL_Register_Notify(U8 *ClientId, tCS_INSTALL_NotificationFunction NotifyFunction);
BOOL CS_INSTALL_Unregister_Notify(U8 ClientId);

BOOL CS_INSTALL_Init(void);

/* For Blind Scan By KB Kim 2011.02.26 */
BOOL MvGetLnbMode(U8 *u8Sat_Index, U8 u8Sat_count, U8 *blinfLnbUniversal);
void MvInstallSetBlind(U8 mode);
BOOL MvInstallStartBlind(MV_BlindScanParams blindParam);
BOOL MvInstallStopBlind(void);
BOOL MvBlindStartInstallation(void);
BOOL MvBlindStopInstallation(void);
void MvAddBlindScanTpData(MV_stTPInfo TPInfo);
void MV_INSTALL_Init_Blind_TPdata(U8 u8Sat_count);
BOOL MV_INSTALL_Add_Blind_TPData(U8 *u8Sat_Index, U8 u8Sat_count, U8 polH);
void INSTALL_TPUnlocked(void);

BOOL CS_INSTALL_StartInstallation(U8 mode);
BOOL CS_INSTALL_StopInstallation(void);
U16  CS_INSTALL_GetTPListNum(void);
U16  CS_INSTALL_GetNumOfTPInstalled(void);
U16 CS_INSTALL_GetCurrentTPIndex(void);
void CS_INSTALL_GetSignalInfo (U8 *Level, U8 * Quality, BOOL* lock);
tMV_Display_ServiceInfo * MV_INSTALL_GetServiceList(void);
U16 CS_INSTALL_GetFirstInstallService(void);
BOOL CS_INSTALL_Read_TPdata(U8 *u8Sat_Index, U8 u8Sat_count, U8 u8SelectTP);

/* For TP Data Sync By KB KIm 2011.03.03 */
// BOOL CS_INSTALL_Get_TPdata(MV_stTPInfo 	*Temp_TPDatas, U8 u8Sat_Index, U8 u8Start);
BOOL CS_INSTALL_Get_TPdata(MV_stTPInfo 	*Temp_TPDatas);

U16 CS_INSTALL_Get_Num_Of_TP_BySat(U8 *u8Search_Sat_Index, U16 u16num_installed);
U16  CS_INSTALL_GetIndexOfTPInstalled(void);
void CS_INSTALL_Get_AddTP_Data(U16 *u16Temp_TpIndex, U16 num_installed);
#endif 


