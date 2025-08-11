/*********************************************************************
 *
 *				  Microchip USB C18 Firmware Version 1.0
 *
 *********************************************************************
 * FileName:		user.h
 * Dependencies:	See INCLUDES section below
 * Processor:		PIC18
 * Compiler:		C18 2.30.01+
 * Company:			Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PIC® Microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PIC Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ********************************************************************/

#ifndef PICDEM_FS_DEMO_H
#define PICDEM_FS_DEMO_H

/** I N C L U D E S **********************************************************/
#include "GenericTypeDefs.h"

/** D E F I N I T I O N S ****************************************************/
/* PICDEM FS USB Demo Version */
#define MINOR_VERSION	0x00	//Demo Version 1.00
#define MAJOR_VERSION	0x01

#define C_I2C_timeout	4000
#define C_I2C_BR		0xA0 //0x80

/*          EEPROM Address          */
#define EEP_I2C_timeout	0x08
#define EEP_I2C_BR		0x0A


typedef enum
{
	READ_VERSION	= 0x00,

	RD_POT			= 0x37,

    I2C_Write		= 0x40,
    I2C_Read		= 0x41,    
    I2C_Test		= 0x42,
    I2C_BlockWrite	= 0x43,
    I2C_BlockRead	= 0x44,
    I2C_WriteRead	= 0x45,
    I2C_Read2		= 0x46,    

	IO_CTL			= 0xF1,
    IO_Test			= 0xF2,
	RESET			= 0xFF
}TYPE_CMD;

typedef enum
{
	I2C_NAK				= 0,
	I2C_ACK				= 1,
	I2C_TIMEOUT			= 2
}IIC_STATE;

#define USBGEN_EP_SIZE          64

/** S T R U C T U R E S ******************************************************/
typedef union DATA_PACKET
{
	BYTE _byte[USBGEN_EP_SIZE];	 //For byte access
	WORD _word[USBGEN_EP_SIZE/2];//For word access(USBGEN_EP_SIZE msut be even)
	struct
	{
		BYTE CMD;
		BYTE len;
	};
/*
	struct
	{
		unsigned :8;
		BYTE ID;
	};
	struct
	{
		unsigned :8;
		BYTE led_num;
		BYTE led_status;
	};
	struct
	{
		unsigned :8;
		WORD word_data;
	};
*/
} DATA_PACKET;

typedef struct IIC_PACKET
{
	BYTE adr;
	BYTE cmd;
	BYTE sts;
	BYTE len;
	BYTE data[34];
}IIC_PACKET;


/** P U B L I C	 P R O T O T Y P E S *****************************************/
void UserInit(void);
void ProcessIO(void);

unsigned char Read_b_eep( unsigned int badd );
void Write_b_eep( unsigned int badd,unsigned char bdata );

BOOL Transmit(void);
BOOL Receive(void);
BOOL Write_BYTE_TO_I2C(unsigned char data_out);
BOOL Read_BYTE_FROM_I2C(unsigned char* buf);
BOOL Read_PAGE_FROM_I2C(void);
BOOL EE_Write(unsigned int address,unsigned char* buf);
BOOL EE_Read(unsigned int address,unsigned char* buf);
void FL_FlashLED(BOOL bON);
BOOL Get_BlockCMD(IIC_PACKET* iic,BYTE cmd,BYTE len);
BOOL Set_BlockCMD(IIC_PACKET* iic,BYTE cmd,BYTE len);
void Set_InterCMD(IIC_PACKET* iic,BYTE cmd,unsigned int value);
void Test_I2CFlow(void);
BOOL IIC_GPIO_Read(IIC_PACKET* iic);
BOOL IIC_Read(IIC_PACKET* iic);
BOOL IIC_Read2(IIC_PACKET* iic);


#endif //PICDEM_FS_DEMO_H
