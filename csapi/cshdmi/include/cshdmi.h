#ifndef   __CSAPI_HDMI_H__	
#define	  __CSAPI_HDMI_H__	

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HDMI_SUCCESS=0,
    HDMI_ERROR_INIT
}CSHDMI_ErrorCode;


CSHDMI_ErrorCode   CSHDMI_Init(void);
void   CSHDMI_Terminate(void);

#ifdef __cplusplus
}
#endif

#endif   /* __CSAPI_HDMI_H__ */

