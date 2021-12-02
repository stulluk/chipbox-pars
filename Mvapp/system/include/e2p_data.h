#ifndef __E2P_DATA_H
#define __E2P_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "database.h"

#define MV_E2P_POWER_STATUS_ADDRESS             0

#define kCS_E2P_ADDRESS_BASE					1000

#define kCS_E2P_USER_SETTING_DATA_BASE			kCS_E2P_ADDRESS_BASE
#define kCS_E2P_USER_SETTING_DATA_SIZE			sizeof(tCS_DBU_UserSetting_Save)

#define kCS_E2P_VOLUME_BASE						(kCS_E2P_USER_SETTING_DATA_BASE + kCS_E2P_USER_SETTING_DATA_SIZE)
#define kCS_E2P_VOLUME_SIZE						4

#define kCS_E2P_MUTE_STATUS_BASE				(kCS_E2P_VOLUME_BASE + kCS_E2P_VOLUME_SIZE)
#define kCS_E2P_MUTE_STATUS_SIZE				4

#define kCS_E2P_CURRENT_SERVICE_BASE			(kCS_E2P_MUTE_STATUS_BASE + kCS_E2P_MUTE_STATUS_SIZE)
#define kCS_E2P_CURRENT_SERVICE_SIZE			sizeof(tCS_DBU_Service)

#define kCS_E2P_TIMER_BASE						(kCS_E2P_CURRENT_SERVICE_BASE + kCS_E2P_CURRENT_SERVICE_SIZE)
#define kCS_E2P_TIMER_SIZE						360

#define kCS_E2P_FULL_SYS_VER					(kCS_E2P_TIMER_BASE + kCS_E2P_TIMER_SIZE)
#define kCS_E2P_FULL_SYS_VER_SIZE				1
#define kCS_E2P_OS_VER							(kCS_E2P_FULL_SYS_VER + kCS_E2P_FULL_SYS_VER_SIZE)
#define kCS_E2P_OS_VER_SIZE						1
#define kCS_E2P_FILE_SYS_VER					(kCS_E2P_OS_VER + kCS_E2P_OS_VER_SIZE)
#define kCS_E2P_FILE_SYS_VER_SIZE				1
#define kCS_E2P_DB_VER							(kCS_E2P_FILE_SYS_VER + kCS_E2P_FILE_SYS_VER_SIZE)
#define kCS_E2P_DB_VER_SIZE						1
#define kCS_E2P_DEF_DB_VER						(kCS_E2P_DB_VER + kCS_E2P_DB_VER_SIZE)
#define kCS_E2P_DEF_DB_VER_SIZE					1

//#define kCS_E2P_TOTAL_SIZE						(kCS_E2P_TIMER_BASE + kCS_E2P_TIMER_SIZE)
#define kCS_E2P_TOTAL_SIZE						(kCS_E2P_DEF_DB_VER + kCS_E2P_DEF_DB_VER_SIZE)


#ifdef __cplusplus
}
#endif

#endif 

