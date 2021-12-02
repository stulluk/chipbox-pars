#ifndef __CSEVT_H__
#define __CSEVT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

typedef void *CSEVT_HANDLE;

typedef enum 
{
   EVT_READ    = 0x01,    /**< Set when a read will not block */
   EVT_WRITE   = 0x02,    /**< Set when a write will not block */
   EVT_EXCEPT  = 0x04,    /**< Set when out-of-band data is available */
   EVT_USER    = 0x08,    /**< Set when out-of-band data is available */
   EVT_INVALID = 0x10     /**< Set when the file descriptor is invalid */
} CSEVT_TYPE;

typedef void (* event_proc)(void *puser, int fd, int events);

CSEVT_HANDLE CSEVT_Init(void);
CSAPI_RESULT CSEVT_Term(CSEVT_HANDLE handle);

CSAPI_RESULT CSEVT_Register(CSEVT_HANDLE handle, int fd, event_proc func, void *puser, int events);
CSAPI_RESULT CSEVT_UnRegister(CSEVT_HANDLE handle, int fd);

CSAPI_RESULT CSEVT_ReportEvent(CSEVT_HANDLE handle, int fd, int events, void *context);

#ifdef __cplusplus
}
#endif

#endif 
