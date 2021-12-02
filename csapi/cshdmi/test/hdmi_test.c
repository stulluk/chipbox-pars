/* Test Program for HDMI
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "cshdmi.h"
#include "ANX9030_System_Config.h"



pthread_t ntid;

int main()
{
    int err;
    void *ret;
    unsigned char buf;
    unsigned int rd_data;
 	int state=1,Timing=0;
	int sel,current_timing;
    
    CSHDMI_Init();

    while(1){
        sleep(1);
    }

#if 0

	while(state)
	{
		puts("Please set HDMI Timing Mode");
		puts("anx9030_V640x480p_60Hz    ------- 1");
	    puts("anx9030_V720x480p_60Hz_4x3------- 2");
		puts("anx9030_V720x480p_60Hz_16x9------ 3");
		puts("anx9030_V1280x720p_60Hz---------- 4");
		puts("anx9030_V1280x720p_50Hz---------- 19");
		puts("anx9030_V1920x1080i_60Hz--------- 5");
		puts("anx9030_V1920x1080p_60Hz--------- 16");
		puts("anx9030_V1920x1080i_50Hz--------- 20");
		puts("anx9030_V1920x1080p_50Hz--------- 31");
		puts("anx9030_V720x480i_60Hz_4x3------- 6");
		puts("anx9030_V720x480i_60Hz_16x9------ 7");
		puts("anx9030_V720x576i_50Hz_4x3------- 21");
		puts("anx9030_V720x576i_50Hz_16x9------ 22");
		puts("anx9030_V720x576p_50Hz_4x3------- 17");
		puts("anx9030_V720x576p_50Hz_16x9------ 18");
		printf("Your Selection=");
		scanf("%d",&Timing);
		if (((Timing>=1) && (Timing<=7)) || ((Timing>=16) && (Timing <=22)) || Timing==31)
			state =0;
	}	
	
#endif 
//	hdmi_init();

//	sleep(6);
//	hdmi_terminate();
	
#if 0
	

	while(state)
	{
		puts("Please set HDMI Timing Mode");
		puts("anx9030_V640x480p_60Hz    ------- 1");
	    puts("anx9030_V720x480p_60Hz_4x3------- 2");
		puts("anx9030_V720x480p_60Hz_16x9------ 3");
		puts("anx9030_V1280x720p_60Hz---------- 4");
		puts("anx9030_V1280x720p_50Hz---------- 19");
		puts("anx9030_V1920x1080i_60Hz--------- 5");
		puts("anx9030_V1920x1080p_60Hz--------- 16");
		puts("anx9030_V1920x1080i_50Hz--------- 20");
		puts("anx9030_V1920x1080p_50Hz--------- 31");
		puts("anx9030_V720x480i_60Hz_4x3------- 6");
		puts("anx9030_V720x480i_60Hz_16x9------ 7");
		puts("anx9030_V720x576i_50Hz_4x3------- 21");
		puts("anx9030_V720x576i_50Hz_16x9------ 22");
		puts("anx9030_V720x576p_50Hz_4x3------- 17");
		puts("anx9030_V720x576p_50Hz_16x9------ 18");
		printf("Your Selection=");
		scanf("%d",&Timing);
		if (((Timing>=1) && (Timing<=7)) || ((Timing>=16) && (Timing <=22)) || Timing==31)
			state =0;
	}	
    HDMI_SetTimingMode(Timing);

    

    err= pthread_create(&ntid, NULL, hdmi_main, NULL);
    if (err !=0){
        printf("can't create thread: %s \n", strerror(err));
        return 0;
    }
	
	
	getchar();
	state =1;
	while(state)
	{
		
		puts("Do you want to set Timing Mode or End HDMI Driver(1:Set Timing Mode.  2:End HDMI Driver)");
		printf("Your Selection(1/2)=");
		fflush(stdout);
		scanf("%d",&sel);
		if (sel==1 || sel==2)
			sleep(5);
		if (sel == 1)
		{
			current_timing=HDMI_GetTimingMode();
			printf("Current Timing is =%d \n",current_timing);
			puts("Please set HDMI Timing Mode");
			puts("anx9030_V640x480p_60Hz    ------- 1");
		    puts("anx9030_V720x480p_60Hz_4x3------- 2");
			puts("anx9030_V720x480p_60Hz_16x9------ 3");
			puts("anx9030_V1280x720p_60Hz---------- 4");
			puts("anx9030_V1280x720p_50Hz---------- 19");
			puts("anx9030_V1920x1080i_60Hz--------- 5");
			puts("anx9030_V1920x1080p_60Hz--------- 16");
			puts("anx9030_V1920x1080i_50Hz--------- 20");
			puts("anx9030_V1920x1080p_50Hz--------- 31");
			puts("anx9030_V720x480i_60Hz_4x3------- 6");
			puts("anx9030_V720x480i_60Hz_16x9------ 7");
			puts("anx9030_V720x576i_50Hz_4x3------- 21");
			puts("anx9030_V720x576i_50Hz_16x9------ 22");
			puts("anx9030_V720x576p_50Hz_4x3------- 17");
			puts("anx9030_V720x576p_50Hz_16x9------ 18");
			printf("Your Selection=");
			fflush(stdout);
			scanf("%2d",&Timing);
			if (((Timing>=1) && (Timing<=7)) || ((Timing>=16) && (Timing <=22)) || Timing==31)
			{
				HDMI_SetTimingMode(Timing);
				
			}
		
	  }
		else if (sel==2)
			state=0;
	}	
    

	
    err= pthread_join(ntid, &ret);
    if (err !=0){
        printf("can't join hdmi main thread: %s \n", strerror(err));
        return 0;
    }
#endif

#if 0
/*Test I2C */
	int i,j;
    hdmi_i2c_open();
	hdmi_gpio_reset();

    i=0;
    j=0;
   // for (i=0;i<256;i++){
        puts("-----------------------");
        //      for (j=0;j<256;j++){
            printf("Test Register 0x%x\n",i);
            hdmi_i2c_read_byte(0x39,i,&buf);
            printf("11  readed value(0x%x)=0x%x\n",i,buf);

            hdmi_i2c_read_byte(0x3D,i,&buf);
            printf("22  readed value(0x%x)=0x%x\n",i,buf);
//             hdmi_i2c_write_byte(0x39,0x05,j);
//             hdmi_i2c_read_byte(0x39,i,&buf);
//             printf("readed (0x%2x)  value(0x%x)=0x%x\n\n",j,i,buf);

            //  buf = 0xff;
//         hdmi_i2c_write_byte(0x39,0x05,buf);
//         hdmi_i2c_read(0x39,0x05,&buf,1);
//         printf("readed value(0x05)=0x%x\n",buf);
//         buf = 0x0;
//         hdmi_i2c_write(0x39,0x05,&buf,1);
            //       }
        //  }
    
#endif


#if 0
/* Test gpio*/

     while(1)
     {
         hdmi_gpio_reset();
     }	

#endif

}
