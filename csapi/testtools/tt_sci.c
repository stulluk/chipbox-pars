
#include "global.h"
#include "cssci.h"
 
CSSCI_HANDLE  sci_handle;
static unsigned int in_counter=0,out_counter=0,quit=0;


static int    Sci_cardinset(void)
{
	int ret=0;
	unsigned char *atr;
	unsigned int  i,freq,atr_len;

	ret = CSSCI_Reset(sci_handle, &atr, &atr_len);
	if(ret == CSAPI_FAILED){
		ret = CSSCI_Get_SC_CLK(sci_handle, &freq);
		if(ret == CSAPI_FAILED)
			printf("\nCSSCI_Get_SC_CLK error\n");
		printf("\nunknown card...\nsci_clock: %d Hz\n",freq);
		return	 -1;
	}

	printf("\n[%d] bytes ATR infomation: ",atr_len);
	for(i = 0; i < atr_len; i++)
	{
		printf("0x%x ",atr[i]);
	}

	return		0;
}

static void Sci_CallBackFunction(CSSCI_HANDLE handle , CSSCI_SCIEVENT* scievent)
{
	switch(*scievent)
	{
		case CSSCI_CARDINS:

			in_counter++;

			if(in_counter == 2){
				quit = 1;
				break;
			}

			printf("*****************cardinset******************\n");	
			Sci_cardinset();
			printf("\n");

			if( in_counter == 1)
				printf("Please remove card to test cardout:\n");

			break;

		case CSSCI_CARDREM:

			printf("*****************cardremove******************\n");
			out_counter++;

			if( (out_counter == 1) && (in_counter == 0) )
				printf("Please insert card to test cardin:\n");

			if( (out_counter == 2) || (in_counter == 1) )
				quit = 1;

			break;

		default:
			break;
	}
}

static void Dummy_CallBackFunction(CSSCI_HANDLE handle , CSSCI_SCIEVENT* scievent)
{
	;
}

static void sci_test(int argc, char *argv[]) 
{
	int ret=0;;
	unsigned int  freq;
	int time_count;
	sci_handle = CSSCI_Open();

	ret = CSSCI_Get_SC_CLK(sci_handle, &freq);
	if(ret == CSAPI_FAILED)
		printf("\nCSSCI_Get_SC_CLK error\n");
	else
		printf("sci_clock: %d Hz\n",freq);

	freq = 3670000;
	ret = CSSCI_Set_SC_CLK(sci_handle, freq);
	if(ret == CSAPI_FAILED)
		printf("\nCSSCI_Set_SC_CLK error\n");
	else
		printf("CSSCI_Set_SC_CLK  OK\n");

	ret = CSSCI_Get_SC_CLK(sci_handle, &freq);
	if(ret == CSAPI_FAILED)
		printf("\nCSSCI_Get_SC_CLK error\n");
	else
		printf("sci_clock: %d Hz\n",freq);


	CSSCI_SetNotify(sci_handle,Sci_CallBackFunction,CSSCI_CARDINS,1);
	CSSCI_SetNotify(sci_handle,Sci_CallBackFunction,CSSCI_CARDREM,1);
	time_count =0;
	while(1)
	{
		usleep(500000);
		if( quit == 1 ){
			in_counter = 0;
			out_counter = 0;
			quit = 0;
			CSSCI_SetNotify(sci_handle,Dummy_CallBackFunction,CSSCI_CARDINS,1);
			CSSCI_SetNotify(sci_handle,Dummy_CallBackFunction,CSSCI_CARDREM,1);
			break;
		}
		time_count ++;
		if (time_count > 20){
			printf("Smart Card Testing Time Out....\n");
			break;
		}	
	}
	//CSSCI_Close(sci_handle);
	printf("SmartCard test over, byebye...\n");

	return;
}

static void sci_transfer(int argc, char **argv)
{
	CSSCI_HANDLE fdes;
	unsigned char *atr;
	unsigned int  atr_len;
	CSAPI_RESULT result;
	char * buffer;	 
	int i;
	int nbyte, len; 
	unsigned char cmd_bytes[15];
	int readed_len, written_len;
	CSSCI_Status sci_status;

	if (argc < 8) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	fdes = CSSCI_Open();
	if(!fdes) {
		printf(" Smartcard Open Error : %s\n", strerror(errno));
		return;
	}

	result = CSSCI_Reset(fdes, &atr, &atr_len);
	if (result == CSAPI_FAILED) {
		printf(" Smartcard Reset Error : %s\n", strerror(errno));
		return;
	}

	for (i = 0; i < atr_len; i++ )
		printf("%x  ", atr[i]);
	printf("\n");

	nbyte = atoi(argv[2]);
	if (nbyte >= 33) {
		printf(" invalid parameters, see help! \n");
		return;
	}
	buffer = (char *)malloc(nbyte*sizeof(char));

	for (i = 0; i < argc-3; i++)
		cmd_bytes[i] = atoi(argv[i+3]);

	result = CSSCI_Transfer(fdes,cmd_bytes,argc-3,buffer,2,
				&written_len,&readed_len,&sci_status);

/*        Test */
/*unsigned char check_bytes1[] = {0x00,0xA4,0x04,0x00,0x05,0xF9,0x5A,0x54,0x00,0x06};*/
/*        buffer = (char *)malloc(33);*/
/*        result = CSSCI_Transfer(fdes,check_bytes1,10,buffer,2,*/
/*                                &written_len,&readed_len,&sci_status);*/

	if(result == CSAPI_FAILED) {
		printf("ERROR: CSSCI_Transfer!\n ");
		return;
	}

	printf("readed_len: %d sci_status( SW1 : %d, SW2: %d)\n", 
	       readed_len, sci_status.SW1, sci_status.SW2);
	for(i = 0;i < readed_len;i ++) {
		printf("0x%x ",buffer[i]);
	}
	printf("\n");

	free(buffer);
	CSSCI_Close(fdes);
}

static void sci_getATR(int argc, char **argv)
{
	CSSCI_HANDLE fdes;
	unsigned char *atr;
	unsigned int  atr_len = 0;
	CSAPI_RESULT result;
	int i;

	fdes = CSSCI_Open();
	if(!fdes) {
		printf(" Smartcard Open Error : %s\n", strerror(errno));
		return;
	}
	//sleep(5);

	result = CSSCI_Reset(fdes, &atr, &atr_len);
	if (result == CSAPI_FAILED) {
		printf(" Smartcard Reset Error : %s\n", strerror(errno));
		return;
	}

	for (i = 0; i < atr_len; i++ )
		printf("%x  ", atr[i]);

	printf("\n\nSmartCard getATR success!\n");

	CSSCI_Close(fdes);
}

static struct cmd_t cssci_tt[] = { 	
 	{
 		"sci_test",
 		NULL,
 		sci_test,
 		 " test smartcard, eg: in/out/get ATR \
 		\n usage: sci_test \
 		\n eg:sci_test \n"},
 	{
 		"sci_transfer",
 		NULL,
 		sci_transfer,
 		 " sci_transfer  - to finish a transfer operation. \
 		\n usage: sci_transfer <read bytes> <write data0 data1 … dataN> \
 		\n eg:sci_transfer  \n"},
 	{
 		"sci_getatr",
 		NULL,
 		sci_getATR,
 		 " sci_getatr - to get a ATR from smart card. \
 		\n usage: sci_getatr \
 		\n eg:sci_getatr \n"},

};
