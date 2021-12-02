#include "linuxos.h"
//#include "i2c.h"
#include "hdmitx.h"

static CSI2C_HANDLE	CAT6611_I2CHandle = NULL;

void DelayMS(USHORT ms)
{
    usleep( ms * 1000 );
}

SYS_STATUS CAT6611_I2CInit(U16 Addr)
{
    CSI2C_Attr   hdmi_handle_attr;
    CSAPI_RESULT err = CSAPI_SUCCEED;
  
    if( CAT6611_I2CHandle == NULL )
    {
        CAT6611_I2CHandle = CSI2C_Open( Addr >> 1);
    	if( CAT6611_I2CHandle == NULL )
        {
            printf("CAT6611_I2CInit CSI2C_Open err = %d\n", err);
            return ER_FAIL;
        }

    	hdmi_handle_attr.speed = 1;
    	hdmi_handle_attr.loop = 0;
    	hdmi_handle_attr.subaddr_num = 1;
    	hdmi_handle_attr.write_delayus = 0;
    	err = CSI2C_SetAttr( CAT6611_I2CHandle, &hdmi_handle_attr );
        if( err != CSAPI_SUCCEED )
        {
            printf("CAT6611_I2CInit CSI2C_SetAttr err = %d\n", err);
            return ER_FAIL;
        }
    }

    return ER_SUCCESS;

}

BYTE HDMITX_ReadI2C_Byte(BYTE RegAddr)
{
	 BYTE 		p_data = 0;
	 rt_error_t error  = CS_NO_ERROR;

	 error = CSI2C_Read( CAT6611_I2CHandle, RegAddr, &p_data, 1 );

	 if( error != CS_NO_ERROR )  printf("CAT6611 Read error   %x  RegAddr=0x%x\n",error,RegAddr);
	 //else printf("CAT6611 Read OK! RegAddr[0x%x]=0x%x\n", RegAddr, p_data );

     //usleep(10000);
	 return p_data;
}

SYS_STATUS HDMITX_WriteI2C_Byte(BYTE RegAddr, BYTE d)
{
	rt_error_t 	error = CS_NO_ERROR;

	error = CSI2C_Write( CAT6611_I2CHandle, RegAddr, &d, 1 );

	if( error != CS_NO_ERROR )  printf("CAT6611 Write error  %x\n",error);

    //usleep(10000);
	return ER_SUCCESS;
}

SYS_STATUS HDMITX_ReadI2C_ByteN(BYTE RegAddr, BYTE *pData, int N)
{

#if 0
	 BYTE 		p_data = 0;
	 rt_error_t error  = CS_NO_ERROR;

	 error = CSI2C_Read( CAT6611_I2CHandle, RegAddr, &p_data, N );

	 if( error != CS_NO_ERROR )  printf("CAT6611 Read error   %x  RegAddr=0x%x\n",error,RegAddr);
	 //else printf("CAT6611 Read OK! RegAddr[0x%x]=0x%x\n", RegAddr, p_data );

     usleep(10000);
#else
	BYTE i;

	for(i=0;i<N;i++)
	{
		pData[i]=HDMITX_ReadI2C_Byte(RegAddr+i);
	}
#endif

	 return ER_SUCCESS;
}

SYS_STATUS HDMITX_WriteI2C_ByteN(SHORT RegAddr, BYTE *pData, int N)
{
#if 0
 	rt_error_t 	error = CS_NO_ERROR;
	
	error = CSI2C_Write( CAT6611_I2CHandle, RegAddr, &pData, N );

	if( error != CS_NO_ERROR )  printf("CAT6611 Write error  %x\n",error);

    usleep(10000);
#else
	BYTE i;

	for(i=0;i<N;i++)
	{
		HDMITX_WriteI2C_Byte(RegAddr+i,pData[i]);
	}
#endif
	return ER_SUCCESS;
}

