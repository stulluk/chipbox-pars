
#include <stdio.h>
#include <stdlib.h>
#include "csgpio.h"
#include <unistd.h> 


int gpio_test_write(unsigned int gpio)
{
    CSGPIO_HANDLE gpio_handle;
    CSAPI_RESULT retval;
    CSGPIO_DIRECTION  gpio_direction;
    unsigned char bit =0;
    char * errstr;
    int i;
    gpio_handle =CSGPIO_Open(gpio);
    if (gpio_handle == NULL) {
        errstr =CSGPIO_GetErrString(gpio_handle);
        fprintf(stderr,"OPEN %s\n",errstr);
        return 1;
    }

    gpio_direction = GPIO_DIRECTION_WRITE;
    retval = CSGPIO_SetDirection(gpio_handle, gpio_direction);
    if (retval != CSAPI_SUCCEED) {
        errstr =CSGPIO_GetErrString(gpio_handle);
        fprintf(stderr,"SetDir %s\n",errstr);
        return 1;
    }

    retval = CSGPIO_Write(gpio_handle, 1);
    if (retval != CSAPI_SUCCEED) {
        errstr =CSGPIO_GetErrString(gpio_handle);
        fprintf(stderr,"WRITE %s\n",errstr);
        return 1;
    }
    sleep(3);
    

    retval = CSGPIO_Write(gpio_handle, 0);
    if (retval != CSAPI_SUCCEED) {
        errstr =CSGPIO_GetErrString(gpio_handle);
        fprintf(stderr,"WRITE2 %s\n",errstr);
        return 1;
    }
    sleep(3);

    for (i=0; i< 100; i++){
        bit=bit?0:1;
        retval = CSGPIO_Write(gpio_handle, bit);
        if (retval != CSAPI_SUCCEED) {
            errstr =CSGPIO_GetErrString(gpio_handle);
            fprintf(stderr,"WRITE3 %s\n",errstr);
            return 1;
        }
        usleep(10000);
      
    }

    retval = CSGPIO_Close(gpio_handle);
    if (retval != CSAPI_SUCCEED) {
        errstr =CSGPIO_GetErrString(gpio_handle);
        fprintf(stderr,"%s\n",errstr);
        return 1;
    }


    return 0;
}

int gpio_test_read(unsigned int gpio)
{
    CSGPIO_HANDLE gpio_handle;
    CSAPI_RESULT retval;
    unsigned char bit=0;
    unsigned char before_bit=0;
    char * errstr;
   
    gpio_handle =CSGPIO_Open(gpio);
    if (gpio_handle == NULL) {
        errstr =CSGPIO_GetErrString(gpio_handle);
        fprintf(stderr,"%s\n",errstr);
        return 1;
    }
/* Default Direction is READ */    
    while(1)
    {
        retval = CSGPIO_Read(gpio_handle, &bit);
        if (retval != CSAPI_SUCCEED) {
            errstr =CSGPIO_GetErrString(gpio_handle);
            fprintf(stderr,"%s\n",errstr);
            return 1;
        }
       
        if (bit != before_bit){
            printf("Read Value  %d\n",bit);
            before_bit = bit;
        }
        usleep(10000);
        
    }
    return 0;
    
}


int main(void)
{
    int gpio;
 
    do{
        printf("Test GPIO Num(0~15) =?");
        scanf("%d",&gpio);
    }while(gpio>15 || gpio<0);
    
    if (gpio_test_write(gpio)){
        printf("TEST WRITE  FAILED!\n");
    }
    else{ 
        printf("TEST WRITE SUCCESS!\n");
    }

    
    if (gpio_test_read(gpio)){
        printf("TEST READ  FAILED!\n");
    }
    else{ 
        printf("TEST READ SUCCESS!\n");
    }
    return 0;
}
