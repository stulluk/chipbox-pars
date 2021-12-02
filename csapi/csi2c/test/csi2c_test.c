
#include <stdio.h>
#include <stdlib.h>
#include "csi2c.h"
#include <unistd.h> 
#define test_len 253
#define chipaddress 0x50


int
test_i2c_1(void)
{
	CSI2C_HANDLE handle;
	CSI2C_ErrCode error;
	int i;
	char one_byte[test_len];
	char buffer[test_len];

	printf("Test I2C API 1: Test read\n");
	handle = CSI2C_Open(chipaddress);
	if (handle == NULL) {
		printf("Open i2c  error\n");
		return -1;
	}

	for (i = 0; i < test_len; i++) {
		error = CSI2C_Read(handle, i, one_byte + i, 1);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}

	}
	error = CSI2C_Read(handle, 0, buffer, test_len);
	if (error != 0) {
		printf("read error \n");
		return -1;
	}
	for (i = 0; i < test_len; i++) {
		printf("Readed Byte: Register(0x%x)=0x%x----0x%x\n", i,
		       one_byte[i], buffer[i]);
	}

	error = CSI2C_Close(handle);
	if (error != 0) {
		printf("close error one byte \n");
		return -1;
	}

	return error;
}

int
test_i2c_2(void)
{
	CSI2C_HANDLE handle;
	CSI2C_ErrCode error;
	int i;
	char one_byte[test_len];
	char buffer[test_len];
	char write[test_len];
    //	hdmi_gpio_reset();

	printf("Test I2C API 1: Test read\n");
	handle = CSI2C_Open(chipaddress);
	if (handle == NULL) {
		printf("Open i2c  error\n");
		return -1;
	}

	for (i = 0; i < test_len; i++) {
		write[i] = i;
		error = CSI2C_Read(handle, i, one_byte + i, 1);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}

	}

	for (i = 0; i < test_len; i++) {
		error = CSI2C_Write(handle, i, (char *) write + i, 1);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}

	}

	error = CSI2C_Read(handle, 0, buffer, test_len);
	if (error != 0) {
		printf("read error \n");
		return -1;
	}
	for (i = 0; i < test_len; i++) {
		printf("Readed Byte: Register(0x%x)=0x%x--write:0x%x--0x%x\n",
		       i, one_byte[i], write[i], buffer[i]);
	}

	error = CSI2C_Close(handle);
	if (error != 0) {
		printf("close error one byte \n");
		return -1;
	}

	return error;
}

int
test_i2c_3(void)
{
	CSI2C_HANDLE handle;
	CSI2C_ErrCode error;
	int i = 0;
	char one_byte[test_len];
	char buffer[test_len];
	char write[test_len];
//    hdmi_gpio_reset();

	printf("short btye =%d\n", sizeof (short));
	printf("Test I2C API 3: Test read\n");
	handle = CSI2C_Open(chipaddress);

	//printf("handle = %d, handle->dev_fd = %d \n", (int) handle, handle->dev_fd);
	if (handle == NULL) {
		printf("Open i2c  error\n");
		return -1;
	}

	CSI2C_Attr i2c_attr;
	error = CSI2C_GetAttr(handle, &i2c_attr);
	if (error != 0) {
		printf("GetAttr %d , error no %d\n", i, error);
		return -1;
	}
	printf("Test I2C API: Get Attr\n");
	i2c_attr.write_delayus = 1000000;
	i2c_attr.subaddr_num = 2;
	error = CSI2C_SetAttr(handle, &i2c_attr);
	if (error != 0) {
		printf("SetAttr error one byte %d , error no %d\n", i, error);
		return -1;
	}
	printf("Test I2C API: Set Attr\n");

	printf("Test I2C API: Opened\n");
	for (i = 0; i < test_len; i++) {
		write[i] = i;
		error = CSI2C_Read(handle, i, one_byte + i, 1);
		printf("Test I2C API: read %d --readed=%d\n", i, one_byte[i]);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}

	}

//      for (i =0; i<test_len; i++){
//           error = CSI2C_Write(handle,i, write+i, 1 );
//           printf("\nTest I2C API: write %d --written=%d\n",i,write[i]);
//           if (error != 0){
//               printf("read error one byte %d , error no %d\n", i,error);
//               return -1;
//           }
//       }

//     for (i =0; i<test_len; i++){
//         error = CSI2C_Read(handle,i, one_byte+i, 1 );
//         printf("Test I2C API: read %d --readed=%d\n",i,one_byte[i]);
//         if (error != 0){
//             printf("read error one byte %d , error no %d\n", i,error);
//             return -1;
//         }
//     }

	error = CSI2C_Read(handle, 0, buffer, test_len);
	if (error != 0) {
		printf("read error \n");
		return -1;
	}
	for (i = 0; i < test_len; i++) {
		printf
		    ("Readed Byte: Register(0x%x)=0x%x-- write: 0x%x --0x%x\n",
		     i, one_byte[i], write[i], buffer[i]);
	}

	for (i = 0; i < test_len; i++) {
		error = CSI2C_Write(handle, i, write + i, 1);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
		write[i] = i;
	}

	error = CSI2C_Write(handle, 0, (char *) write, test_len);
	if (error != 0) {
		printf("read error one byte %d , error no %d\n", i, error);
		return -1;
	}
	printf("Test I2C API: writed\n");

	error = CSI2C_Read(handle, 0, buffer, test_len);
	if (error != 0) {
		printf("read error \n");
		return -1;
	}
	for (i = 0; i < test_len; i++) {
		printf
		    ("Readed Byte: Register(0x%x)=0x%x-- write: 0x%x --0x%x\n",
		     i, one_byte[i], write[i], buffer[i]);
	}

	//printf("handle = %d, handle->dev_fd = %d \n", (int) handle, handle->dev_fd);
	error = CSI2C_Close(handle);
	if (error != 0) {
		printf("close error one byte \n");
		return -1;
	}

	return error;
}

int
test_i2c_4(void)
{
	CSI2C_HANDLE handle;
	CSI2C_ErrCode error;
	CSI2C_Attr i2c_attr;
	int i = 0;
	int j = 0;
	char read_byte[test_len];
	char write_byte[test_len];

	printf("Test I2C API 4: Test loop\n");
	handle = CSI2C_Open(chipaddress);
	if (handle == NULL) {
		printf("Open i2c  error\n");
		return -1;
	}

	error = CSI2C_GetAttr(handle, &i2c_attr);
	if (error != 0) {
		printf("GetAttr %d , error no %d\n", i, error);
		return -1;
	}
	printf("Test I2C API: Get Attr\n");
	printf
	    ("i2c_attr->>>>>>>>>>>>>>>> loop = %d ,speed = %d,subaddr_num = %d,write_delayus = %d \n",
	     i2c_attr.loop, i2c_attr.speed, i2c_attr.subaddr_num,
	     i2c_attr.write_delayus);

	i2c_attr.speed = I2C_SPEED_100K;
	i2c_attr.write_delayus = 1000000;
	i2c_attr.subaddr_num = 2;
	i2c_attr.loop = 1;
	error = CSI2C_SetAttr(handle, &i2c_attr);
	if (error != 0) {
		printf("SetAttr error one byte %d , error no %d\n", i, error);
		return -1;
	}
	printf("Test I2C API: Set Attr\n");

	for (i = 0; i < test_len; i++) {
		error = CSI2C_Read(handle, i, read_byte + i, 1);
		printf("Readed %d --readed=%d\n", i, read_byte[i]);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
	}
	for (i = 0; i < test_len; i++) {
		read_byte[i] = 0;
		printf("%d\n", read_byte[i]);
	}
	error = CSI2C_Read(handle, 0, read_byte, test_len);
	if (error != 0) {
		printf("read error \n");
		return -1;
	}
	for (i = 0; i < test_len; i++) {
		printf("Readed Byte: Register(%d)= %d \n", i, read_byte[i]);
	}

	for (i = 0, j = test_len; i < test_len; i++, j--) {
		if (j > 255)
			write_byte[i] = 12;
		else
			write_byte[i] = j;
		printf("%d\n", write_byte[i]);
	}
	for (i = 0; i < test_len; i++) {
		error = CSI2C_Write(handle, i, write_byte + i, 1);
		printf("Write %d --Writed =%d\n", i, write_byte[i]);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
	}
/*111111111111111111111111111111111111*/
#if 0
	for (i = 0, j = test_len; i < test_len; i++, j--) {
		if (j > 255)
			write_byte[i] = 12;
		else
			write_byte[i] = j;
		printf("%d\n", write_byte[i]);
	}
	for (i = 0; i < test_len; i++) {
		error = CSI2C_Write(handle, i, write_byte + i, 1);
		printf("Write %d --Writed =%d\n", i, write_byte[i]);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
	}
	for (i = 0; i < test_len; i++) {
		error = CSI2C_Read(handle, i, read_byte + i, 1);
		printf("Readed %d --readed=%d\n", i, read_byte[i]);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
	}
	for (i = 0; i < test_len; i++) {
		read_byte[i] = 0;
		printf("%d\n", read_byte[i]);
	}
	error = CSI2C_Read(handle, 0, read_byte, test_len);
	if (error != 0) {
		printf("read error \n");
		return -1;
	}
	for (i = 0; i < test_len; i++) {
		printf("Readed Byte: Register(%d)= %d \n", i, read_byte[i]);
	}

/*2222222222222222222222222222222222222222*/
#if 1
	for (i = 0, j = test_len; i < test_len; i++, j--) {
		if (j > 255)
			write_byte[i] = 18;
		else
			write_byte[i] = j;
		printf("%d\n", write_byte[i]);
	}
	error = CSI2C_Write(handle, 0, write_byte, test_len);
	if (error != 0) {
		printf("read error one byte %d , error no %d\n", i, error);
		return -1;
	}
	for (i = 0; i < test_len; i++) {
		read_byte[i] = 0;
		printf("%d\n", read_byte[i]);
	}
	for (i = 0; i < test_len; i++) {
		error = CSI2C_Read(handle, i, read_byte + i, 1);
		printf("Readed %d --readed=%d\n", i, read_byte[i]);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
	}
#endif
#endif
	return error;
}

int
test_i2c_5(void)
{
	CSI2C_HANDLE handle;
	CSI2C_ErrCode error;
	CSI2C_Attr i2c_attr;
	int i = 0;
	int j = 0;
	char read_byte[test_len];
	char write_byte[test_len];
	//int tt;

	printf("Test I2C API 4: Test loop\n");
	handle = CSI2C_Open(chipaddress);
	if (handle == NULL) {
		printf("Open i2c  error\n");
		return -1;
	}

	error = CSI2C_GetAttr(handle, &i2c_attr);
	if (error != 0) {
		printf("GetAttr %d , error no %d\n", i, error);
		return -1;
	}
	printf("Test I2C API: Get Attr\n");
	printf("i2c_attr->>>>>>>>>>>>>>>> loop = %d ,speed = %d,subaddr_num = %d,write_delayus = %d \n",
	     i2c_attr.loop, i2c_attr.speed, i2c_attr.subaddr_num,
	     i2c_attr.write_delayus);

	i2c_attr.speed = I2C_SPEED_100K;
	i2c_attr.write_delayus = 1000000;
	i2c_attr.subaddr_num = 2;
	i2c_attr.loop = 0;
	error = CSI2C_SetAttr(handle, &i2c_attr);
	if (error != 0) {
		printf("SetAttr error one byte %d , error no %d\n", i, error);
		return -1;
	}
	printf("Test I2C API: Set Attr\n");

#if 0
	for (i = 0, j = test_len; i < test_len; i++, j--) {
		if (j > 255)
			write_byte[i] = 12;
		else
			write_byte[i] = j;
		printf("%d\n", write_byte[i]);
	}
	for (i = 0; i < test_len; i++) {
		error = CSI2C_Write(handle, i, write_byte + i, 1);
		if (error != 0) {
			printf("Write error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
		printf("Write %d --Writed =%d \n", i, write_byte[i]);

		// scanf("%d",&tt);

		error = CSI2C_GetAttr(handle, &i2c_attr);
		if (error != 0) {
			printf("GetAttr %d , error no %d\n", i, error);
			return -1;
		}
		j = i % 2;
		printf("%d \n", j);
#if 1
		if (j)
			i2c_attr.speed = I2C_SPEED_100K;
		else
			i2c_attr.speed = I2C_SPEED_400K;
		printf("i2c_attr = %d \n", i2c_attr.speed);
#endif
		error = CSI2C_SetAttr(handle, &i2c_attr);
		if (error != 0) {
			printf("SetAttr error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
		printf("i2c_attr = %d \n", i2c_attr.speed);
		error = CSI2C_Read(handle, i, read_byte + i, 1);
		printf("Readed %d --readed=%d\n", i, read_byte[i]);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
	}
#endif
/*111111111111111111111111111111111111*/
#if 0
	for (i = 0, j = test_len; i < test_len; i++, j--) {
		if (j > 255)
			write_byte[i] = 12;
		else
			write_byte[i] = j;
		printf("%d\n", write_byte[i]);
	}
	for (i = 0; i < test_len; i++) {
		error = CSI2C_Write(handle, i, write_byte + i, 1);
		printf("Write %d --Writed =%d\n", i, write_byte[i]);
		if (error != 0) {
			printf("Write error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
	}
	for (i = 0; i < test_len; i++) {
		error = CSI2C_Read(handle, i, read_byte + i, 1);
		printf("Readed %d --readed=%d\n", i, read_byte[i]);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
	}
	for (i = 0; i < test_len; i++) {
		read_byte[i] = 0;
		printf("%d", read_byte[i]);
	}
	error = CSI2C_Read(handle, 0, read_byte, test_len);
	if (error != 0) {
		printf("read error \n");
		return -1;
	}
	for (i = 0; i < test_len; i++) {
		printf("Readed Byte: Register(%d)= %d \n", i, read_byte[i]);
	}
#endif
/*2222222222222222222222222222222222222222*/
#if 1
	for (i = 0, j = test_len; i < test_len; i++, j--) {
		if (j > 255)
			write_byte[i] = 18;
		else
			write_byte[i] = j;
		printf("%d\n", write_byte[i]);
	}
	error = CSI2C_Write(handle, 0, write_byte, test_len);
	if (error != 0) {
		printf("write error one byte %d , error no %d\n", i, error);
		return -1;
	}
        sleep(1);
	for (i = 0; i < test_len; i++) {
		read_byte[i] = 0;
		printf("%d", read_byte[i]);
	}
	for (i = 0; i < test_len; i++) {
		error = CSI2C_Read(handle, i, read_byte + i, 1);
		printf("Readed %d --readed=%d\n", i, read_byte[i]);
		if (error != 0) {
			printf("read error one byte %d , error no %d\n", i,
			       error);
			return -1;
		}
	}
#endif
	return error;
}

int
main(void)
{
	int retvale;
	//retvale = test_i2c_1();
	//retvale = test_i2c_2();
	//tvale = test_i2c_3();
	//retvale = test_i2c_4();
        retvale = test_i2c_5();
	return retvale;
}
