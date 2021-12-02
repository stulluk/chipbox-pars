#include <errno.h>

#include "linuxos.h"
#include "e2p.h"
#include "mvosapi.h"  /* For E2P Error problem By KB Kim 2011.09.07 */

#if 1

CSI2C_HANDLE h_e2p = NULL;
// static char* errstr = NULL;


#if 1

int dev_fd;

/* For EEPROM access problem by KB Kim 2011.09.13 */
U32 E2PAccessSem;

/* For EEPROM access problem by KB Kim 2011.09.13 */
int E2P_Init(void)
{
	OsCreateSemaphore(&E2PAccessSem, 1);
	return 0;
}

/* For EEPROM access problem by KB Kim 2011.09.13 */
int E2P_Open(unsigned int chipaddr)
{
	U32 temp;

	temp = (U32)chipaddr;
	OsSemaphoreWait(E2PAccessSem, TIMEOUT_FOREVER);

	dev_fd = open("/dev/misc/eeprom", O_RDWR);
	if (dev_fd < 0) {
		printf("E2P_Open : open eeprom device error : %s\n", strerror(errno));
		return (-1);
	}

	// printf("E2P_Open : open eeprom success\n");

	return 0;
}

void E2P_Close(void)
{
	close(dev_fd);
	OsSemaphoreSignal(E2PAccessSem); /* For EEPROM access problem by KB Kim 2011.09.13 */
}

CS_ErrorCode_t E2P_Write(E2P_Handle_t Handle, U16 SubAddr, U8 *Data, U32 Size)
{
	U32 wSize;
	U32 temp;

	temp = (U32)Handle;
	
	E2P_Open(0xa0); /* For EEPROM access problem by KB Kim 2011.09.13 */

	lseek(dev_fd, SubAddr, SEEK_SET);
	wSize = write(dev_fd, Data, Size);
	if (Size == wSize) {

		// printf("E2P_Write : successfully write length is %d\n", Size);
		// printf("%s succeed\n", __FUNCTION__);
		E2P_Close();
		return CSAPI_SUCCEED;
	}
	else {
		printf("E2P_Write : write eeprom error %s\n", strerror(errno));
		E2P_Close();
		return CSAPI_FAILED;
	}
	E2P_Close();
	return CSAPI_SUCCEED;
}

CS_ErrorCode_t E2P_Read(E2P_Handle_t Handle, U16 SubAddr, U8 *Data, U32 Size)
{
	U32 wSize;
	U32 temp;

	temp = (U32)Handle;

	// printf("E2P_Read : Read Addr [0x%X] Size [%d]\n", SubAddr, Size);
	E2P_Open(0xa0); /* For EEPROM access problem by KB Kim 2011.09.13 */
	lseek(dev_fd, SubAddr, SEEK_SET);
	wSize = read(dev_fd, Data, Size);
	if (Size == wSize) {

		// printf("E2P_Read : successfully read length is %d\n", Size);
		// printf("%s succeed\n", __FUNCTION__);
		E2P_Close();
		return CSAPI_SUCCEED;
	}
	else {
		// printf("E2P_Read : Read eeprom error Addr[0x%X - %d]%s[%d/%d]\n", SubAddr, Size, strerror(errno), wSize, Size);
		E2P_Close();
		return CSAPI_FAILED;
	}

	E2P_Close();
	return CSAPI_SUCCEED;
}
#else
int E2P_Init(unsigned int chipaddr)
{
        CSI2C_Attr i2c_attr;
        
        if (NULL == (h_e2p = CSI2C_Open(chipaddr))) {
		errstr = CSI2C_GetErrString(h_e2p);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

        if (CSI2C_GetAttr(h_e2p, &i2c_attr)) {
		errstr = CSI2C_GetErrString(h_e2p);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	i2c_attr.speed = I2C_SPEED_100K;
	i2c_attr.write_delayus = 5000;
	i2c_attr.subaddr_num = 2;
	i2c_attr.loop = 0;

	if (CSI2C_SetAttr(h_e2p, &i2c_attr)) {
		errstr = CSI2C_GetErrString(h_e2p);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}
}

CS_ErrorCode_t E2P_Write(E2P_Handle_t Handle, U16 SubAddr, U8 *Data, U32 Size)
{
        if (CSI2C_Write(h_e2p, SubAddr, Data, Size)) {
		errstr = CSI2C_GetErrString(h_e2p);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return CSAPI_FAILED;
	}

        return CSAPI_SUCCEED;
}

CS_ErrorCode_t E2P_Read(E2P_Handle_t Handle, U16 SubAddr, U8 *Data, U32 Size)
{
        if (CSI2C_Read(h_e2p, SubAddr, Data, Size)) {
		errstr = CSI2C_GetErrString(h_e2p);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return CSAPI_FAILED;
	}

        return CSAPI_SUCCEED;
}

void API_e2p_close(void)
{
        if (CSI2C_Close(h_e2p)) {
		errstr = CSI2C_GetErrString(h_e2p);
		printf("%s  %s\n", __FUNCTION__, errstr);
	}
}
#endif
#else
int E2P_Init(unsigned int chipaddr)
{
      ;
}

CS_ErrorCode_t E2P_Write(E2P_Handle_t Handle, U16 SubAddr, U8 *Data, U32 Size)
{
    int     fp_eeprom;
    int     retval;
    char    filename[100];

	if( Data == NULL ) return(FALSE);

    sprintf( filename, "/home/eeprom%d", SubAddr );
    fp_eeprom = fopen( filename, "wb");
	if( fp_eeprom <= 0 )
	{
		return(FALSE);
	}

    printf( "E2P_Write %s %d\n", filename, *Data );
	retval = fwrite( Data , Size, 1,  fp_eeprom );
    //printf("fwrite actual size = %d\n", retval);
    
	if( retval != 1 )
	{
		printf("Error: write eeprom\n");
		return(FALSE);
	}

    fclose(fp_eeprom);
	return(TRUE);


}

CS_ErrorCode_t E2P_Read(E2P_Handle_t Handle, U16 SubAddr, U8 *Data, U32 Size)
{

    int     fp_eeprom;
    int     retval;
    char    filename[100];

    if( Data == NULL ) return(FALSE);

    sprintf( filename, "/home/eeprom%d", SubAddr );
    fp_eeprom = fopen( filename, "r" );

    if( fp_eeprom <= 0 )
    {
    	return(FALSE);
    }

    retval = fread( Data, Size, 1, fp_eeprom );
    if( retval != 1 )
    {
    	printf("Error: Read EEPROM\n");
        fclose(fp_eeprom);
        return(FALSE);
    }

    printf( "E2P_Read %s %d\n", filename, *Data );
    fclose(fp_eeprom);
    return(TRUE);

}
#endif

