#ifndef		_HDMI_I2C_H_
#define		_HDMI_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif



int hdmi_i2c_open(void);
int hdmi_i2c_read(unsigned char address, int subaddr, char *buffer, unsigned int num);
int hdmi_i2c_write(unsigned char address, unsigned int subaddr, unsigned char *buffer, unsigned int num);
int hdmi_write_mask(int addr, int subaddr, int mask, int data);
int hdmi_i2c_read_byte(unsigned char address, int subaddr, char *buffer);
int hdmi_i2c_write_byte(unsigned char address, unsigned int subaddr, unsigned char byte);

#ifdef __cplusplus
}
#endif

#endif
