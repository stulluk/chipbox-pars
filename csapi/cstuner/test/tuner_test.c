

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "cs_tuner.h"


static inline int _gpio_open(char *devname)
{	
	int gpio_fd;
	gpio_fd = open(devname, O_RDWR);
	if(gpio_fd < 0) 
	{
		printf("Error: %s.\n",devname);
		
	}

	return gpio_fd;
}

static int _gpio_read(char *devname, char *buf)
{
	int gpio_fd;
	int retval;
	
	gpio_fd = open(devname,O_RDWR);
	if (gpio_fd <= 0)
	{
		printf("Error: Open %s.\n",devname);
		return gpio_fd;
	}
	retval = read(gpio_fd, buf, 1);
	if (retval != 1)
	{
		printf("Error: Read %s. \n",devname);
	}
	close(gpio_fd);
	return retval;
	
	
}

static int _gpio_write(char *devname, char* buf, int len )
{
	int gpio_fd;
	int retval;
	char cmd='O';
    puts(devname);
	gpio_fd = open(devname,O_RDWR);
	if (gpio_fd <= 0)
	{
		printf("Error: Open %s.\n",devname);
		return -1;
	}
	
	retval = write(gpio_fd, &cmd , 1);
	if (retval != 1)
	{
		printf("Error: Read %s. \n",devname);
		return -1;
	}

	retval = write(gpio_fd, buf , len);
	if (retval != len)
	{
		printf("Error: Read %s. \n",devname);
		return -1;
	}
/*
	cmd= 'o';
	retval = write(gpio_fd, &cmd , 1);
	if (retval != 1)
	{
		printf("Error: Read %s. \n",devname);
		return -1;
	}

	return len;
*/    
        retval = close(gpio_fd);
}

static int tuner_gpio_reset(void)
{
        int retval;
	char value = '0';
	retval = _gpio_write("/dev/gpio/6",&value,1);
	usleep(400000);
	value = '1';
	retval = _gpio_write("/dev/gpio/6", &value,1);
	usleep(400000);
 
	return 0;	
}

static int _gpio_write_onebit(char * devname, int value)
{
	int gpio_fd;
	int retval;
	char cmd ='o';
	struct gpio_cmd{
		int cmd;
		int value;
		} gpio_arg;
	
	gpio_fd = open(devname,O_RDWR);
	if (gpio_fd <= 0)
	{
		printf("Error: Open %s.\n",devname);
		return -1;
	}
	gpio_arg.cmd=0x02;
	gpio_arg.value = 1;
	ioctl(gpio_fd,0x02,&gpio_arg);

	
	retval = write(gpio_fd, &cmd , 1);
	if (retval != 1)
	{
		printf("Error: Read %s. \n",devname);
		return -1;
	}
        retval = close(gpio_fd);
	return 0;
}


int tuner_lock(unsigned int tuner_frequency, unsigned int tuner_symbol_rate, unsigned int  tuner_mod)
{
	int err_code = 0;
	int tuner_fd = 0;
	
	TUNER_PARAMS_S tuner_param;
	TUNER_STATUS_E tuner_status = 0;
	int  icount = 0;
	
        while(1)
	{
                tuner_gpio_reset();

	        err_code = cs_tuner_init();
	        if(err_code < 0)
	        {
	             printf("Error: cs_tuner_init.\n");
	             return err_code;
	        }
	        printf("1 success cs_tuner_init. \n");
	        

	        tuner_fd = cs_tuner_open(TUNER_ID_0);
	        if(tuner_fd < 0)
	        {
	             printf("Error: cs_tuner_init.\n");
	             return tuner_fd;
	        }	
	        printf("1 success cs_tuner_open. \n");

	
		memset(&tuner_param, 0, sizeof(TUNER_PARAMS_S));
		tuner_param.frequency = tuner_frequency * 1000;
		tuner_param.qam_params.symbol_rate = tuner_symbol_rate;        
                if(tuner_mod==0)
		    tuner_param.qam_params.modulation = QAM_32;
                else if(tuner_mod==1)
		    tuner_param.qam_params.modulation = QAM_64;
                else if(tuner_mod==2)
		    tuner_param.qam_params.modulation = QAM_128;
                else if(tuner_mod==3)
		    tuner_param.qam_params.modulation = QAM_256;
                else
		    tuner_param.qam_params.modulation = QAM_64;
                   
		tuner_param.inversion = INVERSION_NORMAL;
		
                err_code = cs_tuner_set_params(tuner_fd, &tuner_param);
		if(err_code < 0)
		{
		     printf("Error: cs_tuner_set_params.\n");
		     return err_code;
		}	

		for(icount = 0; icount < 8; icount++)
		{
		     tuner_status = 0;
		     err_code = cs_tuner_read_status(tuner_fd, &tuner_status);
		     if(err_code < 0)
		     {
		      	  printf("Error: cs_tuner_read_status.\n");
			  return err_code;
		     }
                     if(tuner_status==1)  return tuner_status;                     
		     sleep(2);
		}
	}

	return 0;
}

int main()
{
	int err_code = 0;
	int tuner_fd = 0;
	int tuner_status;


	while (1) {
		while(tuner_lock(698, 6875,QAM_64+1)<0) ;

		for(;;)
		{
			tuner_status = 0;
			err_code = cs_tuner_read_status(tuner_fd, &tuner_status);
			if(err_code < 0)
			{
				printf("Error: cs_tuner_read_status.\n");
				return err_code;
			}

			if(tuner_status == TUNER_STATUS_FE_LOCKED)
			{
				printf("tuner_status = %d\n", tuner_status);
				//return 0;
			}
			sleep(2);
		}
	}

	printf("out main. \n");

	return 0;
}

