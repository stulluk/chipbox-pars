#ifndef _CSMPR_COMMON_H_
#define _CSMPR_COMMON_H_


#ifdef __cplusplus
extern "C" {
#endif


#define DEBUG_DEFINE
#if defined(DEBUG_DEFINE)
#define CSMPR_DBG_TRACE         printf( "[%s]\n", __FUNCTION__ )
#define	CSMPR_DBG_DEBUG	        printf
#define	CSMPR_DBG_ERROR		    printf
#else
#define CSMPR_DBG_TRACE         
#define	CSMPR_DBG_DEBUG
#define	CSMPR_DBG_ERROR		   
#endif 

typedef enum
{
    CSMPR_SUCCESS = 0,
    CSMPR_ERROR   = -1
} CSMPR_RESULT;

typedef enum
{
	CS_PVR_REC			= 0,
	CS_PVR_STOP
} eCS_PVR_FILE_STATUS;

#ifdef __cplusplus
}
#endif

#endif
