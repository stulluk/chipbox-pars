/*
 * The usb lowlevel function for orion 1.4 board
 */

#include <types.h>
#include <usb.h>
#include "mentor_usb.h"

#define CFG_HZ	55000000
#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})

void readFifo(int ep, unsigned char *buf, unsigned int len)
{
	volatile u32 fifo = FIFO_ADDRESS(ep);

	if((len > 0)&& ((u32) buf & 3))
    {
		while (len > 0)
		{
		    *buf++ = *((u8 *)fifo);
		    len--;
		}
    }
    else
    {
		/* word access if aligned */
		while ((len > 3)&& !((u32) buf & 3))
		{
		    *((u32 *)buf) = *((u32 *)fifo);
		    buf = buf + 4;
		    len = len - 4;
		}	

		/* byte access for unaligned */
		while (len > 0)
		{
		    *buf++ = *((u8 *)fifo);
		    len--;
		}
    }
}

void writeFifo(int ep, unsigned char *buf, unsigned int len)
{
	volatile u32 fifo = FIFO_ADDRESS(ep);
	int flag;

	flag = disable_interrupts();
	 /* byte access for unaligned */
    if((len > 0) && ((u32) buf & 3))
    {
		while(len)
		{
		    *((u8 *)fifo) = *buf++;
		    len--;
		}
    }
    else     /* word access if aligned */
    {
		while ((len > 3) && !((u32) buf & 3) )
		{
		    *((u32 *)fifo) = *((u32 *)buf); 
		    buf = buf + 4;
		    len = len - 4;
		}	

		while (len > 0)
		{
		    *((u8 *)fifo) = *buf++;
		    len--;
		}
    }
	if (flag)
    	enable_interrupts();
}

static void _sleep(unsigned int loopi)
{
	unsigned int loopk;	
	//    unsigned int loopm;
	unsigned int loopx=loopi*100;

	for(loopk=0;loopk<=loopx;loopk++)   
	{

		loopk=loopk+44323;
		loopk=loopk-44323;

		 for(loopk=0;loopk<=loopx;loopk++)   
		{
		   
		}
		if(loopk==loopx)
			printf("delay==0X%X  \n",loopx); 	
	}	  	
}

void writeU8(unsigned int regaddr, unsigned char data)
{
	volatile char  *addr=(char *)regaddr;
	*addr= data;
	return;
}

void writeU16(unsigned int addr, unsigned short val)
{
		writeU8(addr,(unsigned char)val);
		writeU8(addr+1,(unsigned char)(val>>8));
}

void writeUU16(unsigned int ahb_addr, unsigned short write_value)
{
    volatile unsigned short *pAddr = (unsigned short *)ahb_addr;
    *pAddr = write_value;
    return;
};

void writeU32(unsigned int addr, unsigned int val)
{
		writeU8(addr,(unsigned char)val);
		writeU8(addr+1, (unsigned char)((val & 0x0000ff00) >>8));
		writeU8(addr+2, (unsigned char)((val & 0x00ff0000) >>16));
		writeU8(addr+3, (unsigned char)((val & 0xff000000) >>24));
}

void writeUU32(unsigned int ahb_addr, unsigned int write_value) 
{
	volatile unsigned int *pAddr = (unsigned int *)ahb_addr;
    *pAddr = write_value;
    return;
}

unsigned char readU8(unsigned int regaddr)
{
	char data=*(volatile char *)regaddr;
		
	return(data);
}

unsigned short readU16(unsigned int addr)
{
	unsigned char dat1,dat2;
	unsigned short dat;
	dat1 = readU8(addr);
	dat2 = readU8(addr + 1);
	dat = (dat2 << 8) | dat1;
	return dat;
}

unsigned int readU32(unsigned int addr)
{
	unsigned char dat1,dat2,dat3,dat4;
	unsigned int dat;
	dat1 = readU8(addr);
	dat2 = readU8(addr + 1);
	dat3 = readU8(addr + 2);
	dat4 = readU8(addr + 3);
	dat = (dat4 << 24) | (dat3 << 16) | (dat2 << 8) | dat1;
	return dat;
}

//READ ULPI Register  USB
//byte read only
unsigned int  ULPI_ReadReg(unsigned int addr)
{
	unsigned int temp1=0;
	unsigned int readout=0;
	int loopi=0; 

	//Address of selected register written to ULPIRegAddr. 
	MCU_Write (REG_LPI_ADDR, addr, BE_8_0);
	//ULPIRdnWr and ULPIRegReq bits of ULPIRegControl register set to 1. 
	temp1=USB_ReadReg(REG_LPI_CTRL, BE_8_0);
	temp1=temp1 | 0X5;
	MCU_Write (REG_LPI_CTRL, temp1, BE_8_0);
	//Poll ULPIRegCmplt bit of ULPIRegControl: when becomes set, data in ULPIRegData register is valid.
	temp1=USB_ReadReg(REG_LPI_CTRL, BE_8_0);
	temp1=temp1/2;
	temp1=temp1%2;
	while(temp1!=1 && loopi<=0x01FF)
	{
		temp1=USB_ReadReg(REG_LPI_CTRL, BE_8_0);
		temp1=temp1/2;
		temp1=temp1%2;
		loopi=loopi+1;
		_sleep(10);
		if(loopi>=0x01f0)
		{
			return(0);
		} 
	}
	//read data out  from data register
	readout=USB_ReadReg(REG_LPI_DATA, BE_8_0);  
	MCU_Write (REG_LPI_CTRL, 0x0, BE_8_0);      
	temp1=USB_ReadReg(REG_LPI_CTRL, BE_8_0);

	return readout;
}


//READ ULPI Register  USB
//byte read only
void ULPI_WriteReg(unsigned int addr,unsigned int data)
{
	unsigned int temp1=0;
	int loopi=0;

	//Address of selected register written to ULPIRegAddr. 
	MCU_Write (REG_LPI_ADDR, addr, BE_8_0);

	temp1=USB_ReadReg(REG_LPI_ADDR, BE_8_0);

	// Data written to ULPIRegData register.    
	MCU_Write (REG_LPI_DATA, data, BE_8_0);

	temp1=USB_ReadReg(REG_LPI_DATA, BE_8_0);

	//ULPIRegReq bit of ULPIRegControl register set to 1. (ULPIRdnWr bit must be 0.) 
	temp1=USB_ReadReg(REG_LPI_CTRL, BE_8_0);
	temp1=temp1 | 0X1;
	temp1=temp1 & 0XFB;
	MCU_Write (REG_LPI_CTRL, temp1, BE_8_0);

	//Poll ULPIRegCmplt bit of ULPIRegControl: when becomes set, data in ULPIRegData register is valid.
	temp1=USB_ReadReg(REG_LPI_CTRL, BE_8_0);
	temp1=temp1/2;
	temp1=temp1%2;
	while(temp1!=1  && loopi<=0x01FF)
	{
		temp1=USB_ReadReg(REG_LPI_CTRL, BE_8_0);
		temp1=temp1/2;
		temp1=temp1%2;
		_sleep(10);
		loopi=loopi+1;
		if(loopi>=0x01f0)
		{
			return;
		} 
	}

	MCU_Write (REG_LPI_CTRL, 0, BE_8_0);
	temp1=USB_ReadReg(REG_LPI_ADDR, BE_8_0);
	temp1=USB_ReadReg(REG_LPI_CTRL, BE_8_0);
 }


//READ registe USB  can read 8,16,32;
unsigned int  USB_ReadReg(unsigned int addr, unsigned int be)
{
	unsigned int reg_addr;
	unsigned int readout=0; 
	reg_addr= MUSBFDRC_ADDR +addr;

	switch(be)
	{
		case 1 :  
		{   	
			readout=(unsigned int)readU8(reg_addr); 
		//	printf("First Byte Read:0X%X read from reg  (0X%X) \n", readout,reg_addr);
			break;
		}
		case 2:	{
			readout=(unsigned int)readU8(reg_addr);    	     
			readout=readout << 8;
			break;
		}
		case 4:		
		{
			readout=(unsigned int)readU8(reg_addr);
			readout=readout << 16;
			break;
		}
		case 8:	
		{
			readout=(unsigned int)readU8(reg_addr);
			readout=readout << 24;
			break;
		}
		case 3:
		{
			readout=(unsigned int)readU16(reg_addr);
			break;
		}
		case 12: 	
		{
			readout=(unsigned int)readU16(reg_addr);
			readout=readout << 16;
			break;
		}
		case 15:
		{ 	
			readout=(unsigned int)readU32(reg_addr);
			break;
		}
	}
	
	return readout;
} // ReadReg

//READ registe USB  can read 8,16,32;
void  MCU_Write(unsigned int addr,unsigned int ret, unsigned int be)
{
	unsigned int reg_addr;
	unsigned int writedata=ret; 
	reg_addr= MUSBFDRC_ADDR +addr;

	switch(be)
	{
		case 1 :  
		{   	
			writeU8(reg_addr,(unsigned char)writedata); 
			break;
		}   	    
		case 2:	
		{
			writedata=writedata >> 8;
			writeU8(reg_addr,(unsigned char)writedata); 
			break;
		}
		case 4:		
		{
			writedata=writedata >> 16;
			writeU8(reg_addr,(unsigned char)writedata); 
			break;
		}
		case 8:	
		{
			writedata=writedata >> 24;
			writeU8(reg_addr,(unsigned char)writedata); 
			break;
		}
		case 3:
		{
			writeU16(reg_addr,(unsigned short)writedata); 
			break;
		}
		case 12: 	
		{
			writedata=writedata >> 16;
			writeU16(reg_addr,(unsigned short)writedata); 
			break;
		}
		case 15:
		{ 	
			writeU32(reg_addr,writedata); 
			break;
		}
	}
} // MCU_Write

void enable_fsm(void)
{
	unsigned int temp=0;

	ulpi_set();
	_sleep(8000);

	//set ULPIVbusControl ==0x01;
	MCU_Write (REG_LPI_VBUS, 0x03, BE_8_0);

	temp=USB_ReadReg(M_REG_POWER,BE_8_0);
	temp=temp | M_POWER_ENSUSPEND;
	MCU_Write (M_REG_POWER, temp, BE_8_0);

	MCU_Write (M_REG_INTRUSBE, 0x30, BE_8_0);

	temp=USB_ReadReg(M_REG_DEVCTL,BE_8_0);
	temp=temp | 0x01;
	MCU_Write (M_REG_DEVCTL, temp, BE_8_0);
}

void ulpi_set(void)
{
	unsigned int temp1=0;
	unsigned int addr=0; 
	//write the function RESET register
	//or with the write data
	addr=ULPI_FUN_CTL+1;
	temp1=0x20;

	ULPI_WriteReg(addr,temp1);
	_sleep(5000);    

	temp1=USB_ReadReg(REG_LPI_CARCTRL, BE_8_0);
	temp1=USB_ReadReg(REG_LPI_VBUS, BE_8_0); 

	//let the LED on
	//set the DRV_VBUS_EXT =1 ;
	addr=ULPI_OTG_CTL;
	temp1=ULPI_ReadReg(addr);
	temp1=temp1 | 0x40;
	ULPI_WriteReg(addr,temp1);

	addr=ULPI_OTG_CTL;
	temp1=ULPI_ReadReg(addr);
	temp1=temp1 | 0x80;
	ULPI_WriteReg(addr,temp1);

	//set the DRV_VBUS=1;
	addr=ULPI_OTG_CTL;
	temp1=ULPI_ReadReg(addr);
	temp1=temp1 | 0x20;
	ULPI_WriteReg(addr,temp1);
	// turn on the PHY LED  


	//set the DPpulldown and Dmpulldown = 11  
	temp1=ULPI_ReadReg(addr);
	temp1=temp1 | 0x06;
	ULPI_WriteReg(addr,temp1);

	addr=ULPI_FUN_CTL;  
	//set the TermSelect = 1 ;
	temp1=ULPI_ReadReg(addr);
	temp1=temp1 | 0x04;
	ULPI_WriteReg(addr,temp1);

	//set the OP Mode  = 00  ;
	addr=ULPI_FUN_CTL;  
	temp1=ULPI_ReadReg(addr);
	temp1=temp1 & 0xE7;
	ULPI_WriteReg(addr,temp1); 

	addr=ULPI_FUN_CTL;  
	//set the XcvrSelect[0]  = 1  ;
	temp1=ULPI_ReadReg(addr);
	temp1=temp1 |  0x01;
	ULPI_WriteReg(addr,temp1);   

	//set the Indicator Pass Thru  = 1  and set Indicator Complement ;
	addr=ULPI_IF_CTL;
	temp1=ULPI_ReadReg(addr);
	temp1=temp1 |  0x40;
	temp1=temp1  & 0xDF;
	ULPI_WriteReg(addr,temp1); 
}

void usb_config(void)
{
	int addr=0;
	int temp;

#if defined(CONFIG_ARCH_ORION_13)
	// config the GPIO3 register,0 output
	//config software mode
	addr= 0x101E4008;
	temp=readU16(addr);
	temp=temp & 0xFFF7;
	writeUU16(addr,temp);

	//config output direction
	addr= 0x101E4004;
	temp=readU16(addr);
	temp=temp | 0x0008;
	writeUU16(addr,temp);

	//config the output value '0'
	addr= 0x101E4000;
	temp=readU16(addr);
	temp=temp & 0xFFF7;
	writeUU16(addr,temp);

	// config the PIN MUX register
	addr=0x10171400;
	temp=readU16(addr);
	temp=temp | 0x0002;
	writeUU16(addr,temp);
#endif

	// software Reset USB
	addr=0x10171200;
	temp=readU16(addr);
	temp=temp & 0xFDFF;

	writeUU16(addr,temp);
	_sleep(20);
	temp=readU16(addr);
	temp=temp | 0x0200;
	writeUU16(addr,temp);

	_sleep(50000);     
}

#if defined(CONFIG_ARCH_ORION_13)
void usb_release_bus(void)
{
	int addr=0;
	int temp;

	// config the GPIO3 register,0 output

	//config software mode
	addr= 0x101E4008;
	temp=readU16(addr);
	temp=temp & 0xFFF7;
	writeUU16(addr,temp);

	//config output direction
	addr= 0x101E4004;
	temp=readU16(addr);
	temp=temp | 0x0008;
	writeUU16(addr,temp);

	//config the output value '0'
	addr= 0x101E4000;
	temp=readU16(addr);
	temp=temp | 0x0008;
	writeUU16(addr,temp);

	// config the PIN MUX register
	addr=0x10171400;
	temp=readU16(addr);
	temp=temp & 0x0009;
	writeUU16(addr,temp);
}
#endif

s32 log2func(s32 x)
{
    s32 i;
    
    for(i = 0; x > 1; i++)
	x = x / 2;

    return i;
}

s32 MGC_Config_A_Fifo(u16 ep, u16 feature, 
		      u16 size, u16 address)
{
    u8  sz;

    /* 
     *  size and address are in the unit of 8 bytes
     *  max packet size, 4096 bytes / 8 = 512
     *  max address, 0xfff8 / 8 = 0x1fff
     */
    if ((size > 512) || (address > 0x1fff))
		return 0;

    MOTG_REG_WRITE8(M_REG_INDEX, ep);
    sz = (feature & FIFO_DPB) | log2func(size);
    
    if(feature & FIFO_RX)
    {
		MOTG_REG_WRITE8(M_REG_RXFIFOSZ, sz);
		MOTG_REG_WRITE16(M_REG_RXFIFOADD, address);
    }
    else
    {
		MOTG_REG_WRITE8(M_REG_TXFIFOSZ, sz);
		MOTG_REG_WRITE16(M_REG_TXFIFOADD, address);
    }

	return (1);
}

s32 MGC_Config_Dynamic_Fifos(void)
{
    s16 i;
    u16 address = 8;   /* in the unit of 8 bytes, 
			  first 64 bytes reserved for endpoint 0 */

    for(i = 0; MGC_DRC_Build_Table[i] != 225; i = i + 4)
    {
		if(MGC_Config_A_Fifo(MGC_DRC_Build_Table[i + 1], 
			  MGC_DRC_Build_Table[i + 2], 
			  MGC_DRC_Build_Table[i + 3], 
			  address) == 0)
	    	return 0;
	
		address += MGC_DRC_Build_Table[i + 3]; 
    }

    return 1;
}

int usb_lowlevel_init(void)
{
	char reg = 0, count = 0;
	
	do_usbtest_md();
// FIXME 	usb_config();
// FIXME 	enable_fsm();
	MGC_Config_Dynamic_Fifos();
// FIXME 	reg = MOTG_REG_READ8(M_REG_INTRUSB);
// FIXME 	while (!(reg & 0x10))
// FIXME 	{
// FIXME 		if (count > 5)
// FIXME 			return -1;
// FIXME 		mdelay(100);
// FIXME 		reg = MOTG_REG_READ8(M_REG_INTRUSB);
// FIXME 		count++;
// FIXME 	}
// FIXME 	
// FIXME 	MOTG_REG_WRITE8(M_REG_POWER, M_POWER_RESET | M_POWER_HSENAB | M_POWER_ENSUSPEND);
// FIXME 	mdelay(20);
// FIXME 	MOTG_REG_WRITE8(M_REG_POWER, 0);
// FIXME  	_sleep(10);
// FIXME  	MOTG_REG_WRITE8(M_REG_POWER, M_POWER_ENSUSPEND);

	return 0;
}

int usb_lowlevel_stop(void)
{
	return 0;
}

static int mentor_send_packet(struct usb_device *dev, unsigned long pipe, 
							u8 *buffer, int len, unsigned char step)
{
	int ep = usb_pipeendpoint(pipe);
	u32 reg;
	int time_start = get_timer(0);
	unsigned char ep_info = 0;

	MOTG_REG_WRITE8(M_REG_INDEX, ep);
	
	if (usb_pipecontrol(pipe)) {
		if (step == 0) { /* setup phase */
			writeFifo(ep, buffer, len);
			MOTG_REG_WRITE8(M_REG_CSR0, M_CSR0_TXPKTRDY | M_CSR0_H_SETUPPKT);
		}
		else if (step == 2) { /* status phase */
			if (usb_pipeout(pipe))
				MOTG_REG_WRITE8(M_REG_CSR0, M_CSR0_TXPKTRDY | M_CSR0_H_STATUSPKT);
			else
				MOTG_REG_WRITE8(M_REG_CSR0, M_CSR0_H_REQPKT | M_CSR0_H_STATUSPKT);
		}
		else if (step == 1) { /* data phase */
			if (usb_pipeout(pipe) && len) {
				writeFifo(ep, buffer, len);
				MOTG_REG_WRITE8(M_REG_CSR0, M_CSR0_TXPKTRDY);
				while (!(MOTG_REG_READ8(M_REG_INTRTX) & 0x0001)) {
					if (2*CFG_HZ < get_timer(time_start)) {
						printf("USB transmit timed out\n");
						return -USB_ST_CRC_ERR;
					}
				}
				reg = MOTG_REG_READ8(M_REG_CSR0);
				if ((reg & M_CSR0_H_ERROR) || (reg & M_CSR0_H_RXSTALL))
					return -USB_ST_STALLED;
				return 0;
			}	

			if (usb_pipein(pipe) && len) {
				MOTG_REG_WRITE8(M_REG_CSR0, M_CSR0_H_REQPKT);
				while (!(MOTG_REG_READ8(M_REG_INTRTX) & 0x0001)) {
					if (2*CFG_HZ < get_timer(time_start)) {
						printf("USB transmit timed out\n");
						return -USB_ST_CRC_ERR;
					}
				}
				reg = MOTG_REG_READ8(M_REG_CSR0);
				if ((reg & M_CSR0_H_ERROR) || (reg & M_CSR0_H_RXSTALL) ||
					!(reg & M_CSR0_RXPKTRDY))
					return -USB_ST_STALLED;
				len = MOTG_REG_READ16(M_REG_RXCOUNT);
				readFifo(ep, buffer, len);
				MOTG_REG_WRITE8(M_REG_CSR0, 0);
				return len;
			}
		}
		else {
			printf("Not valid step parameter!\n");
			return -USB_ST_BUF_ERR;
		}
		while (!(USB_ReadReg(M_REG_INTRTX, BE_8_0) & 0x0001)) {
			if (2*CFG_HZ < get_timer(time_start)) {
				printf("USB transmit timed out\n");
				return -USB_ST_CRC_ERR;
			}
		}
		reg = MOTG_REG_READ8(M_REG_CSR0);
		if ((reg & M_CSR0_H_ERROR) || (reg & M_CSR0_H_RXSTALL))
			return -USB_ST_STALLED;
		MCU_Write(M_REG_CSR0, 0, BE_8_0);
		return len;
	}

	if (usb_pipebulk(pipe)) {
		if (usb_pipeout(pipe) && len) {
			MOTG_REG_WRITE16(M_REG_TXMAXP, usb_maxpacket(dev, pipe));
			ep_info = (0x02 << 4) | usb_pipeendpoint(pipe);
			MOTG_REG_WRITE8(M_REG_TXTYPE, ep_info);
			writeFifo(ep, buffer, len);
			MOTG_REG_WRITE8(M_REG_TXCSR, M_TXCSR_TXPKTRDY);
			while (!(MOTG_REG_READ8(M_REG_INTRTX) & (0x0001 << ep))) {
				if (2*CFG_HZ < get_timer(time_start)) {
					printf("USB transmit timed out\n");
					return -USB_ST_CRC_ERR;
				}
			}
			reg = MOTG_REG_READ8(M_REG_TXCSR);
			if ((reg & M_TXCSR_H_ERROR) || (reg & M_TXCSR_H_RXSTALL))
				return -USB_ST_STALLED;
			return len;
		}	

		if (usb_pipein(pipe) && len) {
			MOTG_REG_WRITE16(M_REG_RXMAXP, usb_maxpacket(dev, pipe));
			ep_info = (0x02 << 4) | usb_pipeendpoint(pipe);
			MOTG_REG_WRITE8(M_REG_RXTYPE, ep_info);
			MOTG_REG_WRITE8(M_REG_RXCSR, M_RXCSR_H_REQPKT);
			while (!(MOTG_REG_READ8(M_REG_INTRRX) & (0x0001 << ep))) {
				if (2*CFG_HZ < get_timer(time_start)) {
					printf("USB transmit timed out\n");
					return -USB_ST_CRC_ERR;
				}
			}
			reg = MOTG_REG_READ8(M_REG_RXCSR);
			if ((reg & M_RXCSR_H_ERROR) || (reg & M_RXCSR_H_RXSTALL) ||
				!(reg & M_RXCSR_RXPKTRDY))
				return -USB_ST_STALLED;
			len = MOTG_REG_READ16(M_REG_RXCOUNT);
			readFifo(ep, buffer, len);
			MOTG_REG_WRITE8(M_REG_RXCSR, 0);
			return len;
		}
	}

	printf("Error: the pipe is neither control nor bulk!\n");
	return -USB_ST_BUF_ERR;
}

int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		    int len)
{
 	int dir_out = usb_pipeout(pipe);
	int max = usb_maxpacket(dev, pipe);
	int done = 0;
	
	dev->status = 0;

	while (done < len) {
		int res = mentor_send_packet(dev, pipe, (u8*)buffer+done,
					    	max > len - done ? len - done : max, 1);
		if (res < 0) {
			dev->status = -res;
			return res;
		}

		if (!dir_out && res < max) /* short packet */
			break;

		done += res;
	}

	dev->act_len = done;

	return 0;
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		       int len,struct devrequest *setup)
{
	int done = 0;
	int ep = usb_pipeendpoint(pipe);
	int i;

	dev->status = 0;

	/* setup phase */
	if (mentor_send_packet(dev, usb_sndctrlpipe(dev, ep),
			      (u8*)setup, sizeof(*setup), 0) == sizeof(*setup)) {
		int dir_in = usb_pipein(pipe);
		int max = usb_maxpacket(dev, pipe);

		/* data phase */
		while (done < len) {
			int res = mentor_send_packet(dev, pipe, (u8*)buffer+done,
						    max > len - done ? len - done : max, 1);
			if (res < 0) {
				// printf("status data failed!\n");
				dev->status = -res;
				return 0;
			}
			done += res;
			usb_dotoggle(dev, ep, usb_pipeout(pipe));
			if (dir_in && res < max) /* short packet */
				break;
		}

		/* status phase */
		if (mentor_send_packet(dev,
				      !dir_in ? usb_rcvctrlpipe(dev, ep) :
				      usb_sndctrlpipe(dev, ep),
				      0, 0, 2) < 0) {
			// printf("status phase failed!\n");
			dev->status = -1;
		}
	} else {
		// printf("setup phase failed!\n");
		dev->status = -1;
	}

	dev->act_len = done;
	if (setup->request == 5)  // set address
		for (i=0; i < 31; i++)
            MOTG_REG_WRITE8(M_REG_TARGET_FUNCTION_BASE + i*4, setup->value);

	return done;
}

int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int len, int interval)
{
	return -1;
}




