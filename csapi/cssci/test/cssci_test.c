
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <string.h>

#include "cssci.h"


#define  CSSCI_OBJ_TYPE	'p'
#define  CSSCI_DEV_FILE	"/dev/misc/smartcard"

#define  CSSCI_IOC_MAGIC        'p'

#define SCI_IOC_WARMRESET     _IOW(CSSCI_IOC_MAGIC, 0x01, int)
#define SCI_IOC_ISPRESENT     _IOR(CSSCI_IOC_MAGIC, 0x02, int)
#define SCI_IOC_ACTIVE        _IOW(CSSCI_IOC_MAGIC, 0x03, int)
#define SCI_IOC_DEACTIVE      _IOW(CSSCI_IOC_MAGIC, 0x04, int)
#define SCI_IOC_CLKSTART      _IOW(CSSCI_IOC_MAGIC, 0x05, int)
#define SCI_IOC_CLKSTOP       _IOW(CSSCI_IOC_MAGIC, 0x06, int)
#define SCI_IOC_SETPROTOCOL   _IOW(CSSCI_IOC_MAGIC, 0x07, int)
#define SCI_IOC_ISPRESENT     _IOR(CSSCI_IOC_MAGIC, 0x02, int)


/*static unsigned char *SCI_Events[20] = {"","","","","","","","","","",
				"CSSCI_CARDINS","CSSCI_CARDREM",
				"CSSCI_RXTIMEOUT","CSSCI_ATRRDY",
				"CSSCI_READRDY","CSSCI_WRITERDY",
				"CSCSI_READERROR"};*/


typedef struct CSSCI_SCINFO_ {
	CSSCI_SCTYPE type;

	/* for ATR */
	unsigned char init_char; 	/*Initial Character, TS */
	unsigned char format_char;	/*Format character (Y0), T0 */
	unsigned char current_protocol;
	unsigned char protocols[4];
	unsigned char TA1;		/*Codes F,D */
	unsigned char TB1;		/*Codes I,P */
	unsigned char TC1;		/*Codes N */
	unsigned char TD1;		/*Y2, T1 */
	unsigned char *hist_char;	/*historical characters */
	unsigned char TCheck;
	unsigned char nATR;
	unsigned char atr[33];
	unsigned char specific_mode;
	unsigned char	specific_type_changable;

	/* for T1 protocol */
	unsigned char IFSC;	/* Tx max bytes from device to card for T1 */
	unsigned char CWI;	/*  for CWT (character waitting time) */
	unsigned char BWI;	/*  for BWT (block waitting time) */
	unsigned char RC;		/* specifies the error detection code, 1:CRC,0:LRC */
	unsigned char TxMaxInfoSize;	/* max bytes send to card one transmittal from interface device */
	unsigned char OurSequence;		/* the sequence no. will send to card */
	unsigned char TheirSequence;	/* the last sequence no. used by card */
	unsigned char TxCount;
	unsigned char RxCount;
	unsigned char FirstBlock;	/* bool type */
	unsigned char Aborting;		/* bool type */
	unsigned int		State;		/* if chaining mode to transmit */

} CSSCI_SCINFO;

typedef struct tagCSSCI_OBJ {
	char obj_type;
	int dev_fd;
	int errcode;
	CSSCI_SCINFO sciinfo;
} CSSCI_OBJ;


static unsigned char check_bytes1[] = {0x00,0xA4,0x04,0x00,0x05,0xF9,0x5A,0x54,0x00,0x06};
static unsigned char check_bytes2[] = {0x80,0x46,0x00,0x00,0x04,0x01,0x00,0x00,0x04};
static unsigned char check_bytes3[] = {0x00,0xC0,0x00,0x00,0x04};
static unsigned char check_bytes_t14_1[] = {0x01,0x02,0x00,0x00,0x00,0x00,0x3c};
static unsigned char check_bytes_t14_2[] = {0x01,0x02,0x02,0x03,0x00,0x00,0x3D};
static unsigned char check_bytes_t14_3[] = {0x01,0x02,0x00,0x03,0x00,0x00,0x3F};
static unsigned char check_bytes_t14_4[] = {0x01,0x02,0x01,0x03,0x00,0x00,0x3E};
static unsigned char check_bytes_t14_5[] = {0x01,0x02,0x03,0x03,0x00,0x00,0x3c};
static unsigned char check_bytes_t14_6[] = {0x01,0x02,0x03,0x03,0x01,0x00,0x3d};
static unsigned char check_bytes_t14_7[] = {0x01,0x02,0x03,0x03,0x02,0x00,0x3e};
static unsigned char check_bytes_t14_8[] = {0x01,0x02,0x03,0x03,0x03,0x00,0x3f};

/*	cssci_test_3():
Test Write data 1...
<Readed data num =1 0xa4>
Readed data num =2
Num=0:[90]  Num=1:[00]  

Test Write data 2...
<Readed data num =1 0x46>
Readed data num =2
Num=0:[61]  Num=1:[04]  

Test Write data 3...
Readed data num =7
Num=0:[c0]  Num=1:[00]  Num=2:[98]  Num=3:[96]  Num=4:[a1]  Num=5:[90]  Num=6:[00] 
*/

CSSCI_HANDLE  sci_handle;

/*static void call_back_function(CSSCI_HANDLE scihandle, CSSCI_SCIEVENT *scievent)
{
	printf("Callback invoked for the event : %s\n",SCI_Events[*scievent]);
}*/


int Sci_cardinset(void)
{
	int ret=0;;
	CSSCI_Status sci_status;
	unsigned char *atr, buf[100];
	unsigned int  i,atr_len,written_len,readed_len;

	ret = CSSCI_Reset(sci_handle, &atr, &atr_len);
	if(ret == CSAPI_FAILED){
		printf("\nunknown card...\n");
		return	 -1;
	}
	CSSCI_OBJ  * sci_obj = (CSSCI_OBJ *)sci_handle;

	printf("\n[%d] bytes ATR infomation: \n",atr_len);
	for(i = 0; i < atr_len; i++)
	{
		printf("0x%x  ",*(atr+i));
	}
    //    printf("sci_obj=0x%x,sci_obj->sciinfo=0x%x,sci_obj->sciinfo.TD1=0x%x, sci_obj->sciinfo.TCheck=0x%x, sci_obj->sciinfo.nATR=0x%x, sci_handle=0x%x,  0x%x\n\n",sci_obj,&sci_obj->sciinfo,sci_obj->sciinfo.TD1, sci_obj->sciinfo.TCheck, sci_obj->sciinfo.nATR, sci_handle, sci_obj->sciinfo.hist_char);
    
	printf("History characters: %s\n\n",sci_obj->sciinfo.hist_char);
	
#ifdef P_T0

	ret = CSSCI_Transfer(sci_handle,check_bytes1,10,buf,2,&written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\n");


	ret = CSSCI_Transfer(sci_handle,check_bytes2,9,buf,4,&written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\n");

	ret = CSSCI_Transfer(sci_handle,check_bytes3,5,buf,6,&written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\n");
#endif
#ifdef IRDETO_T14 /* T14 */

    printf("Test 1:\n using test command: ");
    for (i=0; i< 7; i++){
        printf("0x%x ",check_bytes_t14_1[i]);
    }
    printf("\n");
	ret = CSSCI_Transfer(sci_handle,check_bytes_t14_1, 7, buf, 2, &written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

    printf("readed %d data\n",readed_len);
	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\n--------------------------------------------------------------------------------------------------------------\n");


    printf("Test 2:\n using test command: ");
    for (i=0; i< 7; i++){
        printf("0x%x ",check_bytes_t14_2[i]);
    }
    printf("\n");
	ret = CSSCI_Transfer(sci_handle,check_bytes_t14_2, 7, buf, 2, &written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

    printf("readed %d data\n",readed_len);
	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}

	printf("\n--------------------------------------------------------------------------------------------------------------\n");

    printf("Test 3:\n using test command: ");
    for (i=0; i< 7; i++){
        printf("0x%x ",check_bytes_t14_3[i]);
    }
    printf("\n");
	ret = CSSCI_Transfer(sci_handle,check_bytes_t14_3, 7, buf, 2, &written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

    printf("readed %d data\n",readed_len);
	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}

	printf("\n--------------------------------------------------------------------------------------------------------------\n");

    printf("Test 4:\n using test command: ");
    for (i=0; i< 7; i++){
        printf("0x%x ",check_bytes_t14_4[i]);
    }
    printf("\n");
	ret = CSSCI_Transfer(sci_handle,check_bytes_t14_4, 7, buf, 2, &written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

    printf("readed %d data\n",readed_len);
	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\n--------------------------------------------------------------------------------------------------------------\n");

	printf("\n");

    printf("Test 5:\n using test command: ");
    for (i=0; i< 7; i++){
        printf("0x%x ",check_bytes_t14_5[i]);
    }
    printf("\n");
	ret = CSSCI_Transfer(sci_handle,check_bytes_t14_5, 7, buf, 2, &written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

    printf("readed %d data\n",readed_len);
	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\n--------------------------------------------------------------------------------------------------------------\n");

	printf("\n");

    printf("Test 6:\n using test command: ");
    for (i=0; i< 7; i++){
        printf("0x%x ",check_bytes_t14_6[i]);
    }
    printf("\n");
	ret = CSSCI_Transfer(sci_handle,check_bytes_t14_6, 7, buf, 2, &written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

    printf("readed %d data\n",readed_len);
	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\n");
	printf("\n--------------------------------------------------------------------------------------------------------------\n");


    printf("Test 7:\n using test command: ");
    for (i=0; i< 7; i++){
        printf("0x%x ",check_bytes_t14_7[i]);
    }
    printf("\n");
	ret = CSSCI_Transfer(sci_handle,check_bytes_t14_7, 7, buf, 2, &written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

    printf("readed %d data\n",readed_len);
	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\n");
	printf("\n--------------------------------------------------------------------------------------------------------------\n");

    printf("Test 8:\n using test command: ");
    for (i=0; i< 7; i++){
        printf("0x%x ",check_bytes_t14_8[i]);
    }
    printf("\n");
	ret = CSSCI_Transfer(sci_handle,check_bytes_t14_8, 7, buf, 2, &written_len,&readed_len,&sci_status);
	if(ret == CSAPI_FAILED)
		return	 -1;

    printf("readed %d data\n",readed_len);
	for(i = 0;i < readed_len;i ++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\n");

#endif	

	return		0;

}


void Sci_CallBackFunction(CSSCI_HANDLE handle , CSSCI_SCIEVENT* scievent)
{

	//printf("scievent = %d\n",*scievent);

	CSSCI_OBJ  * sci_obj = (CSSCI_OBJ *)handle;
	if((NULL == sci_obj) || (sci_obj->obj_type != CSSCI_OBJ_TYPE)){
		printf("returning from object type mismatch\n");
		return;
	}

	switch(*scievent)
	{
		case CSSCI_CARDINS:
			  printf("*****************cardinset******************\n");	
			  Sci_cardinset();
			break;
		case CSSCI_CARDREM:
			  printf("*****************cardremove******************\n");		
			break;
			default:
				break;
	}
}



int cssci_test_1(void)
{
    int fdes;
    int retval;
    char is_cardin;
    char * r_buffer;
    int loop;

    fdes = open(CSSCI_DEV_FILE, O_RDWR);
    if(fdes < 0) 
	{
		printf(" Smartcard Open Error : %s\n", strerror(errno));
     	return 1;
	}
    
    do{
        retval = ioctl(fdes, SCI_IOC_ISPRESENT, &is_cardin);
        if(retval < 0) 
            {
                printf(" Smartcard  Error : %s\n", strerror(errno));
                return 1;
            }
    }while(is_cardin !=1);

    if (is_cardin == 1){
        retval = ioctl(fdes, SCI_IOC_ACTIVE);
        if(retval < 0) {
            printf(" Smartcard IOCTL Error : %s\n", strerror(errno));
            return 1;
        }
        
        while (1){
            puts("Will read");
            r_buffer = (char *)malloc(1);
            retval = read(fdes, (void *) r_buffer, 1);
            if(retval < 0) {
                printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
            }
            printf("Readed data num =%d\n",retval);
            for (loop =0; loop <retval; loop++){
                printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
            }
            puts("");
            free ((void *)r_buffer);

        }
#if 0
        puts("Will read");
        r_buffer = (char *)malloc(33);
        retval = read(fdes, (void *) r_buffer, 33);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
        }
        printf("Readed data num =%d\n",retval);
        for (loop =0; loop <retval; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }
        puts("");


        puts("Will read");
        r_buffer = (char *)malloc(33);
        retval = read(fdes, (void *) r_buffer, 33);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
        }
        printf("Readed data num =%d\n",retval);
        for (loop =0; loop <retval; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }
        puts("");


        puts("Will read");
        r_buffer = (char *)malloc(33);
        retval = read(fdes, (void *) r_buffer, 33);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
        }
        printf("Readed data num =%d\n",retval);
        for (loop =0; loop <retval; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }
        puts("");


        puts("Will read");
        r_buffer = (char *)malloc(33);
        retval = read(fdes, (void *) r_buffer, 33);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
        }
        printf("Readed data num =%d\n",retval);
        for (loop =0; loop <retval; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }
        puts("");

        retval = ioctl(fdes, SCI_IOC_DEACTIVE, O_RDWR);
        if(retval < 0) 
            {
            printf(" Smartcard IOCTL Error : %s\n", strerror(errno));
            return 1;
        }
#endif        
    } 
    else {
         printf("There is no card!\n");                       
     }
    
    close(fdes);
    

    return 0;
}


int cssci_test_2(void)
{
    int fdes;
    int retval;
    char is_cardin;
    char * r_buffer;
    int loop;
    /*static unsigned char check_bytes1[] = {0x00,0xA4,0x04,0x00,0x05,0xF9,0x5A,0x54,0x00,0x06};
    static unsigned char check_bytes2[] = {0x80,0x46,0x00,0x00,0x04,0x01,0x00,0x00,0x04};
    static unsigned char check_bytes3[] = {0x00,0xC0,0x00,0x00,0x04};*/

    fdes = open(CSSCI_DEV_FILE, O_RDWR);
    if(fdes < 0) 
	{
		printf(" Smartcard Open Error : %s\n", strerror(errno));
     	return 1;
	}
    
    do{
        retval = ioctl(fdes, SCI_IOC_ISPRESENT, &is_cardin);
        if(retval < 0) 
            {
                printf(" Smartcard  Error : %s\n", strerror(errno));
                return 1;
            }
    }while(is_cardin !=1);
		
	r_buffer = (char *)malloc(33);
	
    if (is_cardin == 1){
		 retval = ioctl(fdes, SCI_IOC_ACTIVE);
        retval = ioctl(fdes, SCI_IOC_WARMRESET);
        if(retval < 0) {
            printf(" Smartcard IOCTL Error : %s\n", strerror(errno));
            return 1;
        }
        
        retval = read(fdes, (void *) r_buffer, 33);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
        }
        printf("Readed data num =%d\n",retval);
        for (loop =0; loop <=retval; loop++){
             printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
         }
#if 0				
        usleep(500000);
				
        printf("\n\nTest Write data 1\n");
        for (loop =0; loop <10; loop++){
            printf("write[%02x]  ",check_bytes1[loop]);
        }
        puts("");
        retval = write(fdes, (void *)check_bytes1, 5);
        if(retval < 0) {
            printf(" Smartcard Write Error : %s\n", strerror(errno));
            return 1;
        }
		 usleep(500000);
		 retval = write(fdes, (void *)check_bytes1+5, 5);
        if(retval < 0) {
            printf(" Smartcard Write Error : %s\n", strerror(errno));
            return 1;
        }
        retval = read(fdes, (void *) r_buffer, 2);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
        } 
        sleep(10);
        printf("Readed data num =%d\n",retval);
        for (loop =0; loop <retval; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }
        
        usleep(500000);
				
        printf("\n\nTest Write data 2\n");
        retval = write(fdes, (void *)check_bytes2, 5);
        if(retval < 0) {
            printf(" Smartcard Write Error : %s\n", strerror(errno));
            return 1;
        }
		 usleep(500000);
		 retval = write(fdes, (void *)check_bytes2+5, 4);
        if(retval < 0) {
            printf(" Smartcard Write Error : %s\n", strerror(errno));
            return 1;
        }
        retval = read(fdes, (void *) r_buffer, 2);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
        } 
        
        printf("Readed data num =%d\n",retval);
        for (loop =0; loop <retval; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }
        // sleep(10);

        puts("\n\n\n");
        printf("\n\nTest Write data 2 command\n");
        for (loop =0; loop <5; loop++){
            printf("write[%02x]  ",check_bytes2[loop]);
        }
        puts("");
        retval = write(fdes, (void *)check_bytes2, 5);
        if(retval < 0) {
            printf(" Smartcard Write Error : %s\n", strerror(errno));
            return 1;
        }
        retval = read(fdes, (void *) r_buffer, 1);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
        }
        printf("Readed data num =%d\n",retval);
        for (loop =0; loop <retval; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }

        
        printf("\n\nTest Write data 2 data\n");
        retval = write(fdes, (void *)check_bytes2, 4);
        if(retval < 0) {
            printf(" Smartcard Write Error : %s\n", strerror(errno));
            return 1;
        }
        retval = read(fdes, (void *) r_buffer, 10);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
        }
        printf("Readed data num =%d\n",retval);
        for (loop =0; loop <retval; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }


        /* Test 3 */
 
        printf("\n\nTest Write data 3\n");
        retval = write(fdes, (void *)check_bytes3, 5);
        if(retval < 0) {
            printf(" Smartcard Write Error : %s\n", strerror(errno));
            return 1;
        }
        retval = read(fdes, (void *) r_buffer, 10);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", strerror(errno));
            return 1;
        }
        printf("Readed data num =%d\n",retval);
        for (loop =0; loop <retval; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }

#endif
        sleep(3);

        retval = ioctl(fdes, SCI_IOC_DEACTIVE, O_RDWR);
        if(retval < 0) 
            {
            printf(" Smartcard IOCTL Error : %s\n", strerror(errno));
            return 1;
        }
        
    } 
    else {
         printf("There is no card!\n");                       
     }
       

    return 0;
}




int cssci_test_3(void)
{
    CSSCI_HANDLE fdes;
	 int i,loop;
	 char * r_buffer;	 
    int retval,len;
		unsigned char *atr;
		unsigned int  atr_len;
    

    fdes = CSSCI_Open();
    if(fdes == NULL) 
	{
		printf(" Smartcard Open Error : %s\n", strerror(errno));
     	return 1;
	}
	CSSCI_Reset(fdes, &atr, &atr_len);
	
	CSSCI_OBJ *dev_obj = (CSSCI_OBJ *) fdes;
	
#if	1	
	printf("\nnATR[%d]: 0x ",dev_obj->sciinfo.nATR);
	
	for (i = 0; i < dev_obj->sciinfo.nATR; i++ )
		printf("%x  ",dev_obj->sciinfo.atr[i]);

	if(dev_obj->errcode == SCI_SUCCESS)
		printf("\nSmart Card Open All Right\n");
	else
		printf( "\nSmart Card Open have some problem: %s\n", CSSCI_GetErrString(fdes) );



/*	retval = CSSCI_Activate(fdes);
    if(retval == CSAPI_FAILED) {
            printf(" Smartcard Activate Error : %s\n", strerror(errno));
            return 1;
    }*/


	r_buffer = (char *)malloc(33);
    		
printf("\nTest Write data 1...\n");
        retval = CSSCI_Write(fdes, check_bytes1, 5, &len);
        if(len != 5) {
            printf(" Smartcard Write Error : %d  %s\n", retval,CSSCI_GetErrString(fdes));
            return 1;
        }
//		 usleep(500000);
		  retval = CSSCI_Read(fdes, r_buffer, 1, &len);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", CSSCI_GetErrString(fdes));
            return 1;
        } 
        printf("<Readed data num =%d 0x%x>\n",len, *r_buffer);
				
		 retval = CSSCI_Write(fdes, check_bytes1+5, 5, &len);
        if(len != 5) {
            printf(" Smartcard Write Error : %d  %s\n", retval,CSSCI_GetErrString(fdes));
            return 1;
        }
        retval = CSSCI_Read(fdes, r_buffer, 2, &len);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", CSSCI_GetErrString(fdes));
            return 1;
        } 
        printf("Readed data num =%d\n",len);
        for (loop =0; loop <len; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }        
        sleep(1);
				
printf("\n\nTest Write data 2...\n");
        retval = CSSCI_Write(fdes, check_bytes2, 5, &len);
        if(len != 5) {
            printf(" Smartcard Write Error : %s\n", CSSCI_GetErrString(fdes));
            return 1;
        }
//		 usleep(500000);
		 retval = CSSCI_Read(fdes, r_buffer, 1, &len);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", CSSCI_GetErrString(fdes));
            return 1;
        } 
        printf("<Readed data num =%d 0x%x>\n",len, *r_buffer);
				
		 retval = CSSCI_Write(fdes, check_bytes2+5, 4, &len);
        if(len != 4) {
            printf(" Smartcard Write Error : %s\n", CSSCI_GetErrString(fdes));
            return 1;
        }
        retval = CSSCI_Read(fdes, r_buffer, 2, &len);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", CSSCI_GetErrString(fdes));
            return 1;
        }        
        printf("Readed data num =%d\n",len);
        for (loop =0; loop <len; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        } 
		 sleep(1);
				
printf("\n\nTest Write data 3...\n");
        retval = CSSCI_Write(fdes, check_bytes3, 5, &len);
        if(len != 5) {
            printf(" Smartcard Write Error : %s\n", CSSCI_GetErrString(fdes));
            return 1;
        }
        retval = CSSCI_Read(fdes, r_buffer, 7, &len);
        if(retval < 0) {
            printf(" Smartcard Read Error : %s\n", CSSCI_GetErrString(fdes));
            return 1;
        }        
        printf("Readed data num =%d\n",len);
        for (loop =0; loop <len; loop++){
            printf("Num=%d:[%02x]  ",loop,r_buffer[loop]);
        }
#endif
/*
printf("\n\nTest smartcard in/out ...\n");
		retval = 0;
		retval |= CSSCI_SetNotify(fdes, call_back_function,CSSCI_CARDINS,1);
		retval |= CSSCI_SetNotify(fdes, call_back_function,CSSCI_CARDREM,1);
		if(retval < 0) {
            printf(" Smartcard SetNotify Error : %s\n", CSSCI_GetErrString(fdes));
            return 1;
        }

		while(1)
			sleep(5);
*/
		retval = CSSCI_Deactivate(fdes);
        if(retval == CSAPI_FAILED) {
            printf(" Smartcard Deactivate Error : %s\n", CSSCI_GetErrString(fdes));
            return 1;
        }        

    return 0;
}



int cssci_test_4(void)
{

	sci_handle = CSSCI_Open();
	CSSCI_SetNotify(sci_handle,Sci_CallBackFunction,CSSCI_CARDINS,1);
	CSSCI_SetNotify(sci_handle,Sci_CallBackFunction,CSSCI_CARDREM,1);

	while(1)
	{
		sleep(5);
	}

	return 0;

}


int main(void)
{
       if (cssci_test_4()){
           printf("\n\nTEST   FAILED!\n");
       }
       else{ 
           printf("\n\nTEST  SUCCESS!\n");
       }
			 
    return 0;
}
