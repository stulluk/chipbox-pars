#ifndef ISDB_ALPUS_I2C_H_
#define ISDB_ALPUS_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ALPS_I2C_ERR_INDEX					1
#define ALPS_I2C_ERR_NON					0


#define TC90507_DEVICE_ADDR					0x20		// 0x30, Slave Address

#define I2C_SET _IOW('I', 0, unsigned long)

int	ALPS_i2c_init(void);
int TC90507_I2C_OpenPort(void);
void TC90507_I2C_ClosePort(void);
void TC90507_Set_SlaveAddress(U8 ucAddr);
void TC90507_Get_SlaveAddress(U8 *pucAddr);
int TC90507_RegWrite8(U8 ucRegAddr8, U8 ucData8);
int TC90507_RegWrite8Array(U8 ucRegAddr8, U8 ucDataLen, U8 *pucData8);
int TC90507_RegRead8(U8 ucRegAddr8, U8 *pucData8);
int  TC90507_RegRead8Array(U8 ucRegAddr8, U8 ucDataLen, U8 *pucData8);
int TC90507_RegWriteMaskBit8(U8 ucRegAddr8, U8 ucMask8, U8 ucData8);
int TC90507_Bypass_WriteArray(U8 ucDataLen, U8 *pucData8);
int TC90507_Bypass_ReadArray(U8 ucSubAddr, U8 ucDataLen, U8 *pucData8);
int TC90507_gpio_reset(void);

#ifdef __cplusplus
}
#endif

#endif

