//  ANALOGIX Company 
//  ANX9030 Demo Firmware on SST
//  Version 1.01	2006/11/06

#include <stdio.h>
#include <string.h>
#include "ANX9030_Sys9030.h"
#include "ANX9030_System_Config.h"
#include "hdmi_i2c.h"
#include "hdmi_gpio.h"
#include "cshdmi.h"
#include <pthread.h>

int restart_system;
pthread_t hdmi_tid;
int hdmi_terminate_flag=1;

void * hdmi_main (void *arg) 
{
    unsigned char read_buf;
	unsigned int hdmi_id;
	
 	arg = arg; /* avoid waring */
    debug_printf("OK in hdmi thread. Timing =%d\n",anx9030_video_timing_id);
	hdmi_i2c_open();
	hdmi_gpio_reset();
	
	
    // ANX9030 located

	hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_DEV_IDH_REG, &read_buf);
	hdmi_id = read_buf & 0xFF;
	hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_DEV_IDL_REG, &read_buf);
	hdmi_id = (hdmi_id<<8) | (read_buf & 0xFF);
	if(0x9030 != hdmi_id)
	{
		printf("the HDMI chip id is 0x%08x \n", hdmi_id);
		printf("please check the HDMI chip is ready, or check the I2C controller is ok. \n");
		return	((void *)0);
	}

    ANX9030_ShutDown(0);
    ANX9030_HDCP_ONorOFF(0); /*0:off, 1:on*/
	restart_system = 1;
    while (hdmi_terminate_flag) 
    {
	
        if (restart_system) 
        {
    
            restart_system = 0;
 	        ANX9030_Initial();
            anx9030_system_config_retry =1;
        }
        if(!anx9030_shutdown)
        {
	    	ANX9030_Interrupt_Process();
            

        	if(( anx9030_parse_edid_done == 1 
                &&   anx9030_system_config_done == 0))
        	{
        	    anx9030_system_config_retry = 0;
        		debug_printf("anx9030_parse_edid_done=%d\n",anx9030_parse_edid_done);
	        	ANX9030_Parse_EDID_Result_For_System();
	        	ANX9030_System_Config();
	        	anx9030_system_config_done = 1;
        	}
			ANX9030_Operation_Process ();
        }
		
     }

   return NULL;
  }

CSHDMI_ErrorCode CSHDMI_Init(void) 
{
	int err;
	
	printf("Timing mode =%d \n",HDMI_GetTimingMode());
    err= pthread_create(&hdmi_tid, NULL, hdmi_main, NULL);
    if (err !=0){
        printf("can't create hdmi thread: %s \n", strerror(err));
        return HDMI_ERROR_INIT;
    }
	hdmi_terminate_flag=1;
	return HDMI_SUCCESS;
}

void CSHDMI_Terminate(void)
{
	int err;
	void *ret;

	hdmi_terminate_flag =0;

	err= pthread_join(hdmi_tid, &ret);
    if (err !=0)
	{
        printf("can't join hdmi main thread: %s \n", strerror(err));
        return ;
    }

	return ;
}

