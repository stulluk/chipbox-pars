/****************************************************************
*
* FILENAME
*	mvmisc.h
*
* PURPOSE 
*	Header for Miscellaneous Functions
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2010.01.03           KB
*
****************************************************************/
#ifndef MV_MISC_H
#define MV_MISC_H

/****************************************************************
 *                       Include files                          *
 ****************************************************************/

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#define MAX_NUMBER_OF_I2C_DEVICE   1
#define MAX_NUMBER_OF_GPIO         2

#define GPIO_READ_MODE   0
#define GPIO_WRITE_MODE  1
#define GPIO_IO_MODE     2

/****************************************************************
 *                       Type define                            *
 ****************************************************************/
typedef struct
{
	U8       DevideNo; 
	U8       Address;
	U8       SubAddrLen;
	U8       Speed;
}I2cOpenParam_t;

typedef struct
{
	U8 PortNumber; // value : port number, 0xFF : No Port assigned
	U8 BitNumber;  // Number of bit in Port
	U8 Mode;       // 0 : read, 1 : Write.  2 : I/O
	U8 Value;      // Initial Value
}GpioOpenParam_t;

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/

#endif // #ifndef MV_MISC_H

