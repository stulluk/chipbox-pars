/*
 * closed_stubs.c - EABI placeholders for the closed (OABI-only) libraries mvapp links.
 *
 * libsystem_merih.a, libsystem_sc.a, libci.a, libteletext.a, libsubtitle.a and
 * libsecurity.a exist only as OABI objects without source, so an EABI mvapp cannot
 * link them. These stubs let an EABI mvapp link and run with the features switched
 * off: no CAS/BISS/oscam, no CI CAM, no teletext/subtitles. libsystem_merih (tuner and
 * SI) is re-implemented in system/merih_eabi/.
 *
 * Return values follow the callers: TUNER_NO_ERROR (0) for tuner calls, FALSE for
 * CAS/SI "did something" BOOLs, 0 for int "init ok", empty data for getters.
 */
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>

#include "csapi.h"
#include "linuxos.h"
#include "dvbtuner.h"
#include "tableApi.h"
#include "casapi.h"
#include "ci_api.h"
#include "csttxdraw.h"
#include "subtitle.h"
#include "security.h"

/* libsystem_merih tuner part: real implementation in system/merih_eabi/. */

/* libsystem_merih SI part: real implementation in system/merih_eabi/si_*.c. */

/* ---- libsystem_sc: CAS / BISS / oscam --------------------------------------- */

int CasDrvInit(void) { return 0; }
U8 CasDrvGetCardStatus(U8 slot) { (void)slot; return 0; }
U16 CasGetCurrentCardId(U8 slot) { (void)slot; return 0; }
U8 StbSGetOscamStatus(void) { return 0; }
void CasDrvSaveKey2Flase(void) {}

void CasGetSystemInfo(U16 casId, U8 *casName)
{
  (void)casId;
  if (casName != NULL)
    casName[0] = '\0';
}

void CasDrvGetKeyDbInfo(CasDbInfo_t *casDbInfo)
{
  if (casDbInfo != NULL)
    memset(casDbInfo, 0, sizeof(*casDbInfo));
}

BOOL CasDrvNotifyPMT(U8 sourceId, U8 *PmtData)
{
  (void)sourceId; (void)PmtData;
  return FALSE;
}

BOOL CasDrvStartNewChannel(U8 sourceId, U16 dmxId, U16 channelId, U16 serviceId,
                           U16 vPid, U16 aPid)
{
  (void)sourceId; (void)dmxId; (void)channelId; (void)serviceId; (void)vPid; (void)aPid;
  return FALSE;
}

BOOL CasDrvStopChannel(U8 sourceId)
{
  (void)sourceId;
  return FALSE;
}

BOOL CasDrvDeleteKey(U8 *casId, U8 *providerId, unsigned char Key_Number,
                     unsigned char KeySkip)
{
  (void)casId; (void)providerId; (void)Key_Number; (void)KeySkip;
  return FALSE;
}

BOOL CasDrvDeleteProvider(U8 *casId, U8 *providerId)
{
  (void)casId; (void)providerId;
  return FALSE;
}

BOOL CasDrvUpdateKey(U8 *casId, U8 *providerId, U8 *providerName, U8 keyNumber,
                     U8 keyLength, U8 *keyData)
{
  (void)casId; (void)providerId; (void)providerName; (void)keyNumber;
  (void)keyLength; (void)keyData;
  return FALSE;
}

BOOL CasDrvUpdateProvider(U8 *casId, U8 *providerId, unsigned char *providerName)
{
  (void)casId; (void)providerId; (void)providerName;
  return FALSE;
}

BOOL CasDrvGetBissKey(CasBissInfo_t bissInfo, U8 *key, U8 *keyLength)
{
  (void)bissInfo; (void)key;
  if (keyLength != NULL)
    *keyLength = 0;
  return FALSE;
}

BOOL CasDrvUpdateBissKey(CasBissInfo_t bissInfo, U32 *provider, U8 *keyNumber, U8 *key,
                         U8 keyLength)
{
  (void)bissInfo; (void)provider; (void)keyNumber; (void)key; (void)keyLength;
  return FALSE;
}

int CasDrvReadOscamServerData(OscamServerInfo_t *ServerData)
{
  (void)ServerData;
  return 0; /* no servers */
}

int CasDrvWriteOscamServerData(OscamServerInfo_t *serverData, int number, int newNumber)
{
  (void)serverData; (void)number; (void)newNumber;
  return 0;
}

/* ---- libci: Common Interface ------------------------------------------------ */

static menu_t empty_menu; /* is_valid == 0; callers dereference without a NULL check */

int CS_CI_init(tCS_CI_InitParam param) { (void)param; return 0; }
void CS_CI_Register_CI_Notify(tCS_CI_NotifyFunctions funcs) { (void)funcs; }
BOOL Check_ca_system_id_valid_status(unsigned short ca_system_id)
{
  (void)ca_system_id;
  return FALSE;
}
int ai_enter_menu(unsigned char app_index) { (void)app_index; return -1; }
int ai_get_application_information(struct application_information_t **p)
{
  if (p != NULL)
    *p = NULL;
  return -1;
}
int cas_send_ca_pmt_list_to_all(unsigned char *p_ca_pmt, unsigned int length)
{
  (void)p_ca_pmt; (void)length;
  return -1;
}
int mmi_enter_choice(unsigned char num) { (void)num; return -1; }
int mmi_enter_input(unsigned char command, unsigned char *str, int length)
{
  (void)command; (void)str; (void)length;
  return -1;
}
menu_t *mmi_get_menu(void)
{
  memset(&empty_menu, 0, sizeof(empty_menu));
  return &empty_menu;
}
void mmi_free_menu(menu_t *p_menu) { (void)p_menu; }

/* ---- libteletext / libsubtitle / libsecurity -------------------------------- */

INT32 InitialTeletext(TTXInit_Params Params) { (void)Params; return 0; }
INT32 CreateTeletext(U16 Ttxpid, U16 PageHex, TTXMode Mode, U8 *Lang,
                     TTX_NotificationCallBack NotifyFunction)
{
  (void)Ttxpid; (void)PageHex; (void)Mode; (void)Lang; (void)NotifyFunction;
  return -1;
}
INT32 DestroyTeletext(void) { return 0; }

int InitialSubtitle(SUBInit_Params Initpara) { (void)Initpara; return 0; }
void OpenSubtitle(U16 Pid, Sub_NotificationCallBack NotifyFunction)
{
  (void)Pid; (void)NotifyFunction;
}
void CloseSubtitle(void) {}

int Sc41Init(void) { return 0; }

/* ---- DirectFB entry points (DirectFB is not used on the HDMI/MiniGUI path) ---- */

int DirectFBInit(int *argc, char *(*argv[]))
{
  (void)argc; (void)argv;
  return -1; /* DFB_FAILURE */
}

int DirectFBCreate(void **interface_ptr)
{
  if (interface_ptr != NULL)
    *interface_ptr = NULL;
  return -1; /* DFB_FAILURE */
}
