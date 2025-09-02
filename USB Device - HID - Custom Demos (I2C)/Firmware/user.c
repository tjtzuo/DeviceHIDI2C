/********************************************************************
 FileName:		user.c
 Dependencies:	See INCLUDES section
 Processor:		PIC18 USB Microcontrollers
 Hardware:		This demo is natively intended to be used on Microchip USB demo
				boards supported by the MCHPFSUSB stack.  See release notes for
				support matrix.	 This demo can be modified for use on other hardware
				platforms.
 Complier:		Microchip C18 (for PIC18)
 Company:		Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC(R) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

********************************************************************
 File Description:

 Change History:
  Rev	Date		 Description
  1.0	11/19/2004	 Initial release
  2.1	02/26/2007	 Updated for simplicity and to use common
					 coding style
********************************************************************/

/** INCLUDES *******************************************************/
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "USB/usb.h"
#include "USB/usb_function_hid.h"
#include "HardwareProfile.h"
#include "user.h"
#include "protocol_io.h"
#include <EEP.h>
#include <flash.h>
/*
//#define SLAVE_ADDR	0x16
#define SLAVE_ADDR	0xAA

// bq27541
#define SBS_TEMP	0x06
#define SBS_VOLT	0x08
#define SBS_CURR	0x14
*/
#define RECORD_ADDRESS	0x3F00

#pragma romdata record = RECORD_ADDRESS
ROM char rec_data[256] = { 0x12, 0x34 };

/** V A R I A B L E S ********************************************************/
#pragma udata

BYTE counter;
BOOL blinkStatusValid = TRUE;

// Begin Generic Hid specific
char usb_state = 'r';
// End Generic Hid specific

IIC_PACKET iic;
/*
BYTE tranlength;
BYTE eeadr;
BYTE testFlow;
BYTE DownLoadFlow;
BYTE ErrorCode;
*/
BOOL i2cHang;
WORD i2cTimeout;
/*
BOOL bClkTrim;
BOOL bWriteBusy;
BOOL bTestFlow;
*/
extern USB_HANDLE USBOutHandle;
extern USB_HANDLE USBInHandle;

extern DATA_PACKET ReceivedDataBuffer;
extern DATA_PACKET ToSendDataBuffer;

#define INPacket	ToSendDataBuffer
#define OUTPacket	ReceivedDataBuffer


// Timer0 - 1 second interval setup.
// Fosc/4 = 12MHz
// Use /256 prescalar, this brings counter freq down to 46,875 Hz
// Timer0 should = 65536 - 46875 = 18661 or 0x48E5
#define TIMER0L_VAL			0xE5
#define TIMER0H_VAL			0x48


/** PRIVATE PROTOTYPES *********************************************/
void BlinkUSBStatus(void);
//WORD_VAL ReadPOT(void);
void ServiceRequests(void);


/** D E C L A R A T I O N S **************************************************/
#pragma code

#define M_TMR0_ON	T0CONbits.TMR0ON = 1; // enable timer 0
#define M_TMR0_OFF	T0CONbits.TMR0ON = 0; // disable timer 0

void SetTimer0(void)
{
	T0CONbits.T08BIT = 0; // 16 bit counter
	T0CONbits.T0CS = 0; // internal clock
	T0CONbits.T0SE = 0; // low-to-high edge
	T0CONbits.PSA = 1; // Prescale
	//T0CON |= 0x02; // 1:8
	//T0CON |= 0x01; // 1:4

	TMR0H = 0;
	TMR0L = 0;

	INTCONbits.T0IE = 0; // mask interrupt
	INTCONbits.T0IF = 0; // clear overflow flag
}

void OpenT1(void)
{
	T1CONbits.RD16 = 1; // 16 bit counter

	//T1CONbits.TMR1CS = 0; // internal clock, Focs/4
	//T1CONbits.T1OSCEN = 0; // Timer1 Oscillator shut off
	//T1CONbits.T1SYNC = 0; // Timer1 External Clock Input Synchronization Select bit
	//T1CON |= 0x30; // 1:8
	T1CON |= 0x20; // 1:4
	PIR1bits.TMR1IF = 0;
}

void OpenT3(void)
{
	T3CONbits.RD16 = 1; // 16 bit counter
	T3CON |= 0x20; // 1:4
	PIR2bits.TMR3IF = 0;
}

void SetT1(unsigned int time)
{
	unsigned int t1 = 0xFFFF;
	T1CONbits.TMR1ON = 0;
	t1 = t1 - time;
	TMR1H = t1/256;
	TMR1L = t1%256;
	PIR1bits.TMR1IF = 0;

	T1CONbits.TMR1ON = 1;
}

void SetT3(unsigned int time)
{
	unsigned int t3 = 0xFFFF;
	T3CONbits.TMR3ON = 0;
	t3 = t3 - time;
	TMR3H = t3/256;
	TMR3L = t3%256;
	PIR2bits.TMR3IF = 0;

	T3CONbits.TMR3ON = 1;
}


#ifndef __EEP_H
/****************************************************************************
			EEPROM
***************************************************************************/
unsigned char Read_b_eep( unsigned char badd )
{
	EEADR = badd;
	EECON1bits.CFGS = 0;// Access EEPROM
	EECON1bits.EEPGD = 0;// Access Flash Program or EEPROM
	EECON1bits.RD = 1;// Read
	Nop();							//Nop may be required for latency at high frequencies
	Nop();							//Nop may be required for latency at high frequencies
	return ( EEDATA );// return with read byte
}

void Write_b_eep( unsigned char badd,unsigned char bdata )
{
	unsigned char GIE_BIT_VAL = INTCONbits.GIE;

	EEADR = badd;
	EEDATA = bdata;
	EECON1bits.CFGS = 0;// Access EEPROM
	EECON1bits.EEPGD = 0;// Access Flash Program or EEPROM
	EECON1bits.WREN = 1;// Enable Write

	INTCONbits.GIE = 0;// Disable All interrupt
	EECON2 = 0x55;
	EECON2 = 0xAA;
	EECON1bits.WR = 1;
//	INTCONbits.GIE = 1;// Enable All interrupt

	while( EECON1bits.WR );	//Wait till the write completion

	INTCONbits.GIE = GIE_BIT_VAL;
	EECON1bits.WREN = 0;//Disable Write
}
#endif


/******************************************************************************
 * Function:		void UserInit(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		This routine should take care of all of the demo code
 *					initialization that is required.
 *
 * Note:
 *
 *****************************************************************************/
void UserInit(void)
{
	//Initialize all of the LED pins
	mInitAllLEDs();

	//Initialize all of the push buttons
	mInitAllSwitches();

	memset(&ToSendDataBuffer, -1, sizeof(ToSendDataBuffer));

	InitI2C();

	//=====Load smbus timeout=======
	if (Read_b_eep(EEP_I2C_timeout) != 0xFF)
	{
		i2cTimeout = ((WORD)Read_b_eep(EEP_I2C_timeout+1)<<8)|Read_b_eep(EEP_I2C_timeout);
	}
	else
	{
		i2cTimeout = C_I2C_timeout;
	}

	SetTimer0();
	I2C_Enable(1);
	FL_FlashLED(0);
/*
	bTestFlow = 0;
	testFlow = 0;//no test flow
*/
//	blinkStatusValid = FALSE;
}//end UserInit

/********************************************************************
 * Function:		void ProcessIO(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		This function is a place holder for other user
 *					routines. It is a mixture of both USB and
 *					non-USB tasks.
 *					Sends a received Input report back
 *					to the host in an Output report.
 *					Both directions use interrupt transfers.
 *
 * Note:			None
 *******************************************************************/
void ProcessIO(void)
{
	//Blink the LEDs according to the USB device status
	if (blinkStatusValid)
	{
		BlinkUSBStatus();
	}

	// User Application USB tasks
	if ((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1)) return;

	//respond to any USB commands that might have come over the bus
	ServiceRequests();
}//end ProcessIO

/******************************************************************************
 * Function:		WORD_VAL ReadPOT(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			WORD_VAL - the 10-bit right justified POT value
 *
 * Side Effects:	ADC buffer value updated
 *
 * Overview:		This function reads the POT and leaves the value in the
 *					ADC buffer register
 *
 * Note:			None
 *****************************************************************************/
/*
WORD_VAL ReadPOT(void)
{
	WORD_VAL w;

	w.Val = 0;

	ADCON0bits.GO = 1;			// Start AD conversion
	while(ADCON0bits.GO);		// Wait for conversion
	w.v[0] = ADRESL;
	w.v[1] = ADRESH;

	return w;
}//end ReadPOT
*/

/******************************************************************************
 * Function:		void ServiceRequests(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	USB traffic can be generated
 *
 * Overview:		This function takes in the commands from the PC from the
 *					application and executes the commands requested
 *
 * Note:			None
 *****************************************************************************/
void ServiceRequests(void)
{
	BYTE index;
    unsigned int addr;

	// Start Generic HID specific

	// Receive
	if (usb_state == 'r')
	{
		//Check if we have received an OUT data packet from the host
		if (!HIDRxHandleBusy(USBOutHandle))
		{
			//if the handle is no longer busy then the last
			//transmission is complete

			// The CPU owns the endpoint. Check for received data.

			BYTE len = USBHandleGetLength(USBOutHandle);

			if (len != USBGEN_EP_SIZE) return;

			counter = 0;

			//Echo back to the host PC the command we are fulfilling in the first byte.
			INPacket.CMD = OUTPacket.CMD;
			INPacket.len = OUTPacket.len;

			//We just received a packet of data from the USB host.
			//Check the first byte of the packet to see what command the host
			//application software wants us to fulfill.
			switch (OUTPacket.CMD)				//Look at the data the host sent, to see what kind of application specific command it sent.
			{
			case READ_VERSION:
				if (OUTPacket.len == 2)
				{
					INPacket._byte[2] = MINOR_VERSION;
					INPacket._byte[3] = MAJOR_VERSION;
					counter = 4;
				}
				else
					counter = 1;
				break;

			case 0x80:	//Toggle LEDs command
				blinkStatusValid = FALSE;			//Stop blinking the LEDs automatically, going to manually control them now.
#if defined(__18F14K50)
				LATBbits.LATB7 ^= 1;
#else
				if (mGetLED_1() == mGetLED_2())
				{
					mLED_1_Toggle();
					mLED_2_Toggle();
				}
				else
				{
					if (mGetLED_1())
					{
						mLED_2_On();
					}
					else
					{
						mLED_2_Off();
					}
				}
#endif
				ToSendDataBuffer._byte[1] = 0;
				counter = 2;
				break;

			case 0x81:	//Get push button state
				ToSendDataBuffer._byte[1] = 1;
				if (sw3 == 1)							//pushbutton not pressed, pull up resistor on circuit board is pulling the PORT pin high
				{
					ToSendDataBuffer._word[1] = 1;
				}
				else									//sw3 must be == 0, pushbutton is pressed and overpowering the pull up resistor
				{
					ToSendDataBuffer._word[1] = 0;
				}
				counter = 4;
				break;
/*
			case RD_POT:	//Read POT command.	 Uses ADC to measure an analog voltage on one of the ANxx I/O pins, and returns the result to the host
			{
				WORD_VAL w = ReadPOT();					//Use ADC to read the I/O pin voltage.	See the relevant HardwareProfile - xxxxx.h file for the I/O pin that it will measure.
														//Some demo boards, like the PIC18F87J50 FS USB Plug-In Module board, do not have a potentiometer (when used stand alone).
														//This function call will still measure the analog voltage on the I/O pin however.	To make the demo more interesting, it
														//is suggested that an external adjustable analog voltage should be applied to this pin.
				ToSendDataBuffer._byte[1] = 2;
//				ToSendDataBuffer[2] = w.v[0];	//Measured analog voltage LSB
//				ToSendDataBuffer[3] = w.v[1];	//Measured analog voltage MSB
				ToSendDataBuffer._word[1] = w.Val;

				counter = 4;
				break;
			}
*/
			case READ_FLASH:
				if (OUTPacket.len == 3)
				{
					counter = min(OUTPacket._byte[4], 60);
					memcpypgm2ram(&INPacket._byte[2], (ROM FAR void*)OUTPacket._word[1], counter);
					INPacket._byte[1] = counter;
					counter += 2;
				}
				else
				{
					INPacket._byte[1] = 0;
					counter = 2;
				}	
				break;

			case WRITE_FLASH:
				counter = OUTPacket._byte[3];
				if (counter <= 60 && counter == OUTPacket.len - 2)
				{
					WriteBytesFlash(RECORD_ADDRESS + OUTPacket._byte[2] * FLASH_ERASE_BLOCK, counter, &OUTPacket._byte[4]);
					INPacket._byte[1] = 1;
				}
				else
					INPacket._byte[1] = 0;
				counter = 2;
				break;

			case ERASE_FLASH:
				addr = OUTPacket._word[1];
				if ((OUTPacket.len == 2) && (addr >= RECORD_ADDRESS) && ((addr % FLASH_ERASE_BLOCK) == 0))
				{
					EECON1bits.EEPGD = 1;	// point to Flash program memory
					EECON1bits.CFGS = 0;	// access Flash program memory
					EraseFlash(addr, addr + FLASH_ERASE_BLOCK - 1);
					INPacket._byte[1] = 1;
				}
				else
					INPacket._byte[1] = 0;
				counter = 2;
				break;

			case READ_EEDATA:
				if (OUTPacket.len == 1)
				{
					INPacket._byte[3] = Read_b_eep(INPacket._byte[2] = OUTPacket._byte[2]);
					INPacket._byte[1] = 2;
					counter = 4;
				}
				else
					counter = 1;
				break;

			case WRITE_EEDATA:
				if (OUTPacket.len == 2)
				{
					Write_b_eep(INPacket._byte[2] = OUTPacket._byte[2], INPacket._byte[3] = OUTPacket._byte[3]);
					counter = 4;
				}
				else
					counter = 1;
				break;

			case EEROM_32W:
				addr = ((unsigned int)OUTPacket._byte[1]<<8)|OUTPacket._byte[2];
				INPacket._byte[1] = EE_Write(addr, &OUTPacket._byte[3], 32);
				counter = 2;
				break;

			case EEROM_32R:
				addr = ((unsigned int)OUTPacket._byte[1]<<8)|OUTPacket._byte[2];
				if (EE_Read(addr, &INPacket._byte[2], 32))
				{
					INPacket._byte[1] = 32;
					counter = 34;
				}
				else
				{
					INPacket._byte[1] = 0;
					counter = 2;
				}
				break;

			case EEROM_60W:
				addr = ((unsigned int)OUTPacket._byte[1] << 8) | OUTPacket._byte[2];
				INPacket._byte[1] = EE_Write(addr, &OUTPacket._byte[4], OUTPacket._byte[3]);
				counter = 2;
				break;

			case EEROM_60R:
				addr = ((unsigned int)OUTPacket._byte[1] << 8) | OUTPacket._byte[2];
				counter = OUTPacket._byte[3];
				if (EE_Read(addr, &INPacket._byte[2], counter))
				{
					INPacket._byte[1] = counter;
					counter += 2;
				}
				else
				{
					INPacket._byte[1] = 0;
					counter = 2;
				}
				break;

			case I2C_Write:
				INPacket._byte[1] = Transmit();
				counter = 2;
				break;

			case I2C_Read:
				iic.adr = OUTPacket._byte[1];
				iic.cmd = OUTPacket._byte[2];
				iic.len = min(OUTPacket._byte[3], sizeof(iic.data));
				if (IIC_Read(&iic))
				{
					INPacket._byte[1] = iic.len;
					for (index = 0; index < iic.len; index++)
					{
						INPacket._byte[2+index] = iic.data[index];
					}
					counter += iic.len;
				}
				else
				{
					// NACK
					INPacket._byte[1] = 0;
				}
				counter = 2;
				break;

			case I2C_WriteF4:
				iic.adr = 0x16;
				iic.cmd = 0xF4;
				iic.len = min(OUTPacket.len, sizeof(iic.data));
				for (index = 0; index < iic.len; index++)
				{
					iic.data[index] = OUTPacket._byte[2 + index];
				}
				INPacket._byte[1] = IIC_Write(&iic);
				counter = 2;
				break;

			case RESET:
				Reset();
				break;

			default:
//				ToSendDataBuffer._byte[1] = 0xFF;
				break;
			}

			for (index = counter; index < USBGEN_EP_SIZE - 1; index++)
				INPacket._byte[index] = 0xFF;

			INPacket._byte[index] = counter;

			// Ready to transmit the received data back to the host.
			usb_state = 't';
		}
	}

	// Transmit
	if (usb_state == 't')
	{
		//Check to make sure the endpoint/buffer is free before we modify the contents
		if (!HIDTxHandleBusy(USBInHandle))
		{
			// The CPU owns the endpoint. Prepare to send data to the host.

			//Prepare the USB module to send the data packet to the host
			USBInHandle = HIDTxPacket(HID_EP, (BYTE*)&ToSendDataBuffer, USBGEN_EP_SIZE);

			//Re-arm the OUT endpoint, so we can receive the next OUT data packet
			//that the host may try to send us.
			USBOutHandle = HIDRxPacket(HID_EP, (BYTE*)&ReceivedDataBuffer, USBGEN_EP_SIZE);

			// Return to checking for new received data.
			usb_state = 'r';
		}
	}
}//end ServiceRequests

/********************************************************************
 * Function:		void BlinkUSBStatus(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		BlinkUSBStatus turns on and off LEDs
 *					corresponding to the USB device state.
 *
 * Note:			mLED macros can be found in HardwareProfile.h
 *					USBDeviceState is declared and updated in
 *					usb_device.c.
 *******************************************************************/
void BlinkUSBStatus(void)
{
	// No need to clear UIRbits.SOFIF to 0 here.
	// Callback caller is already doing that.
	static WORD led_count = 0;

	if (led_count == 0)	led_count = 10000U;
	led_count--;

#ifdef __18F14K50
	if (led_count == 0)
	{
		mLED_1_Toggle();
	}
#else

	#define mLED_Both_Off()			{mLED_1_Off();mLED_2_Off();}
	#define mLED_Both_On()			{mLED_1_On();mLED_2_On();}
	#define mLED_Only_1_On()		{mLED_1_On();mLED_2_Off();}
	#define mLED_Only_2_On()		{mLED_1_Off();mLED_2_On();}

	if(USBSuspendControl == 1)
	{
		if(led_count==0)
		{
			mLED_1_Toggle();
			if(mGetLED_1())
			{
				mLED_2_On();
			}
			else
			{
				mLED_2_Off();
			}
		}//end if
	}
	else
	{
		if(USBDeviceState == DETACHED_STATE)
		{
			mLED_Both_Off();
		}
		else if(USBDeviceState == ATTACHED_STATE)
		{
			mLED_Both_On();
		}
		else if(USBDeviceState == POWERED_STATE)
		{
			mLED_Only_1_On();
		}
		else if(USBDeviceState == DEFAULT_STATE)
		{
			mLED_Only_2_On();
		}
		else if(USBDeviceState == ADDRESS_STATE)
		{
			if(led_count == 0)
			{
				mLED_1_Toggle();
				mLED_2_Off();
			}//end if
		}
		else if(USBDeviceState == CONFIGURED_STATE)
		{
			if(led_count==0)
			{
				mLED_1_Toggle();
				if(mGetLED_1())
				{
					mLED_2_Off();
				}
				else
				{
					mLED_2_On();
				}
			}//end if
		}//end if(...)
	}//end if(UCONbits.SUSPND...)
#endif
}//end BlinkUSBStatus


void FL_FlashLED(BOOL bON)
{
	LATBbits.LATB7 = ~bON;
}


/*****************************************************************************
			I2C
*****************************************************************************/

BOOL Read_BYTE_FROM_I2C(unsigned char* buf)
{
	WORD WC = 0;
	i2cHang = 0;

	I2C_Idle();
	SSPCON2bits.RCEN = 1;

	while(!SSPSTATbits.BF)
	{
		if(WC > i2cTimeout )
		{
			WC = 0;
			i2cHang = 1;
			return 0;
		}
		WC++;
	}
	WC = 0;

	buf[0] = SSPBUF;
	while(SSPCON2bits.RCEN);

	if ( PIR2bits.BCLIF )	// test for bus collision
	{
		PIR2bits.BCLIF = 0;	//clear flag
		return 0;			// return with Bus Collision error
	}

	return 1;
}

BOOL IIC_Write(IIC_PACKET* iic)
{
	BYTE i;
	PIR2bits.BCLIF = 0;	//clear flag
	I2C_Idle();
	//SSPCON2bits.SEN = 1;	//Start
	//while ( SSPCON2bits.SEN );	// wait until start condition is over
	if(I2C_Start() == 0)
	{
		I2C_Stop();
		return 0;
	}

	if ( PIR2bits.BCLIF )		// test for bus collision
	{
		PIR2bits.BCLIF = 0;	//clear flag
		I2C_Stop();
		return 0;			 // return with Bus Collision error
	}

	if(Write_BYTE_TO_I2C(iic->adr&0xFE) == 0) // Address
	{
		I2C_Stop();
		return 0;
	}

	if(Write_BYTE_TO_I2C(iic->cmd) == 0) // command
	{
		I2C_Stop();
		return 0;
	}

	for(i=0;i<iic->len;i++)//
	{
		if(Write_BYTE_TO_I2C(iic->data[i]) == 0)
		{
			I2C_Stop();
			return 0;
		}
	}

	I2C_Stop();

	return 1;
}

BOOL IIC_Read(IIC_PACKET* iic)
{
	BYTE i;
	PIR2bits.BCLIF = 0;	//clear flag
	I2C_Idle();
	//SSPCON2bits.SEN = 1;	//Start
	//while ( SSPCON2bits.SEN );	// wait until start condition is over
	if (I2C_Start() == 0)
	{
		I2C_Stop();
		return 0;
	}

	if ( PIR2bits.BCLIF )		// test for bus collision
	{
		//LATCbits.LATC2 = 0;
		PIR2bits.BCLIF = 0;	//clear flag
		I2C_Stop();
		return 0;			// return with Bus Collision error
	}

	if (Write_BYTE_TO_I2C(iic->adr&0xFE) == 0)
	{
		I2C_Stop();
		return 0;
	}

	if (Write_BYTE_TO_I2C(iic->cmd) == 0)
	{
		I2C_Stop();
		return 0;
	}
	//=================================
	//SSPCON2bits.RSEN = 1;			// initiate bus restart condition
	//while ( SSPCON2bits.RSEN );	// wait until re-start condition is over
	if (I2C_ReStart() == 0)
	{
		I2C_Stop();
		return 0;
	}

	if ( PIR2bits.BCLIF )		// test for bus collision
	{
		PIR2bits.BCLIF = 0;	//clear flag
		//LATCbits.LATC3 = 0;
		I2C_Stop();
		return 0;			// return with Bus Collision error
	}

	if (Write_BYTE_TO_I2C(iic->adr|0x01) == 0)
	{
		I2C_Stop();
		return 0;
	}

	// recevie data
	for (i = 0; i < iic->len; i++)
	{
		if (Read_BYTE_FROM_I2C(&iic->data[i]) == 0)
		{
			if (i2cHang)
			{
				i2cHang = 0;
				I2C_ProtocolErr();
			}
			I2C_Stop();
			return 0;
		}

		if (i < (iic->len-1))
		{
			// return ack
			if (I2C_Ack() == 0)
			{
				I2C_Stop();
				return 0;
			}
			//while(SSPCON2bits.ACKEN );
		}
	}

	// last byte need to return nack
	I2C_NotAck();
	//while ( SSPCON2bits.ACKEN ); // wait until ACK sequence is over

	I2C_Stop();
	return 1;
}


BOOL Transmit(void)
{
	BYTE i;
//	WORD WC = 0;
//	i2cHang = 0;

	PIR2bits.BCLIF = 0;	//clear flag
	I2C_Idle();

	if (I2C_Start() == 0)
	{
		I2C_Stop();
		return 0;
	}
/*
	SSPCON2bits.SEN = 1;	//Start
	while ( SSPCON2bits.SEN )	   // wait until start condition is over
	{
		if (WC > i2cTimeout )
		{
			WC = 0;
			i2cHang = 1;
			return 0;
		}
		WC++;
	}
	WC = 0;
*/
	if ( PIR2bits.BCLIF )		// test for bus collision
	{
		PIR2bits.BCLIF = 0;	//clear flag
		I2C_Stop();
		return 0;			 // return with Bus Collision error
	}

	if (Write_BYTE_TO_I2C((OUTPacket._byte[1]&0xFE)) == 0)
	{
		I2C_Stop();
		return 0;
	}

	if (Write_BYTE_TO_I2C(OUTPacket._byte[2]) == 0)
	{
		I2C_Stop();
		return 0;
	}

	for (i = 0; i < min(OUTPacket._byte[3], USBGEN_EP_SIZE-4); i++)//tranlength
	{
		if (Write_BYTE_TO_I2C(OUTPacket._byte[4+i]) == 0)
		{
			I2C_Stop();
			return 0;
		}
	}

	I2C_Stop();
	return 1;
}

BOOL Write_BYTE_TO_I2C(unsigned char data_out)
{
	WORD WC = 0;
	i2cHang = 0;

	SSPBUF = data_out;

	if (SSPCON1bits.WCOL)//write collision detect
	{
		SSPCON1bits.WCOL = 0;
		return 0;
	}

	while (SSPSTATbits.BF)
	{
		if (WC > i2cTimeout )
		{
			WC = 0;
			i2cHang = 1;
			return 0;
		}
		WC++;
	}
	WC = 0;

	while ( ( SSPCON2 & 0x1F ) || ( SSPSTATbits.R_W ) )
	{
		if (WC > i2cTimeout )
		{
			WC = 0;
			i2cHang = 1;
			return 0;
		}
		WC++;
	}
	WC = 0;

	if (SSPCON2bits.ACKSTAT)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/*****************************************************************************

/*****************************************************************************
			EEPROM
*****************************************************************************/
BOOL EE_Write(unsigned int address,unsigned char* buf,BYTE len)
{
#if 1
	BYTE i;
	I2C_Start_IO();//Start
	I2C_TX_IO(0xA0);
	if(I2C_Rec_Ack())
	{
		I2C_Stop_IO();
		return 0;
	}
	I2C_TX_IO((address&0xFF00)>>8);
	if(I2C_Rec_Ack())
	{
		I2C_Stop_IO();
		return 0;
	}
	I2C_TX_IO(address&0xFF);
	if(I2C_Rec_Ack())
	{
		I2C_Stop_IO();
		return 0;
	}
	for(i=0;i<len;i++)
	{
		I2C_TX_IO(buf[i]);
		if(I2C_Rec_Ack())
		{
			I2C_Stop_IO();
			return 0;
		}
	}

	I2C_Stop_IO();
#else
	BYTE i;
	PIR2bits.BCLIF = 0;	//clear flag
	I2C_Idle();
	SSPCON2bits.SEN = 1;	//Start
	while ( SSPCON2bits.SEN );      // wait until start condition is over

	if ( PIR2bits.BCLIF )       // test for bus collision
	{
		PIR2bits.BCLIF = 0;	//clear flag
		I2C_Stop();
		return 0;            // return with Bus Collision error
	}
	// write ID
	if(Write_BYTE_TO_I2C(0xA0) == 0)
	{
		I2C_Stop();
		return 0;
	}
	// write address
	if(Write_BYTE_TO_I2C((address&0xFF00)>>8) == 0)
	{
		I2C_Stop();
		return 0;
	}

	if(Write_BYTE_TO_I2C(address&0xFF) == 0)
	{
		I2C_Stop();
		return 0;
	}
	// page data write
	for(i=0;i<len;i++)//
	{
		if(Write_BYTE_TO_I2C(buf[i]) == 0)
		{
			I2C_Stop();
			return 0;
		}
	}

	I2C_Stop();
#endif
	return 1;
}

BOOL EE_Read(unsigned int address,unsigned char* buf,BYTE len)
{
#if 1
	BYTE i;
	I2C_Start_IO();//Start
	I2C_TX_IO(0xA0);// device adress
	if(I2C_Rec_Ack())
	{
		I2C_Stop_IO();
		return 0;
	}
	I2C_TX_IO((address&0xFF00)>>8);// high address
	if(I2C_Rec_Ack())
	{
		I2C_Stop_IO();
		return 0;
	}
	I2C_TX_IO(address&0xFF);// low address
	if(I2C_Rec_Ack())
	{
		I2C_Stop_IO();
		return 0;
	}

	I2C_ReStart_IO();//Restart
	I2C_TX_IO(0xA1);// device address read
	if(I2C_Rec_Ack())
	{
		I2C_Stop_IO();
		return 0;
	}

	// recevie data
	for(i=0;i<len;i++)
	{
		buf[i] = I2C_RX_IO();
		if(i < len-1)
		{
			// return ack
			I2C_Send_Ack(0);// ACK
		}
	}
	I2C_Send_Ack(1);// NACK
	I2C_Stop_IO();
#else
	BYTE i;
	PIR2bits.BCLIF = 0;	//clear flag
	I2C_Idle();
	SSPCON2bits.SEN = 1;	//Start
	while ( SSPCON2bits.SEN );      // wait until start condition is over

	if ( PIR2bits.BCLIF )       // test for bus collision
	{
		//LATCbits.LATC2 = 0;
		PIR2bits.BCLIF = 0;	//clear flag
		I2C_Stop();
		return 0;            // return with Bus Collision error
	}

	if(Write_BYTE_TO_I2C(0xA0) == 0)
	{
		I2C_Stop();
		return 0;
	}

	// write address
	if(Write_BYTE_TO_I2C((address&0xFF00)>>8) == 0)
	{
		I2C_Stop();
		return 0;
	}

	if(Write_BYTE_TO_I2C(address&0xFF) == 0)
	{
		I2C_Stop();
		return 0;
	}
	//=================================
	SSPCON2bits.RSEN = 1;           // initiate bus restart condition
	while ( SSPCON2bits.RSEN );  // wait until re-start condition is over

	if ( PIR2bits.BCLIF )       // test for bus collision
	{
		PIR2bits.BCLIF = 0;	//clear flag
		//LATCbits.LATC3 = 0;
		I2C_Stop();
		return 0;            // return with Bus Collision error
	}

	if(Write_BYTE_TO_I2C(0xA1) == 0)
	{
		I2C_Stop();
		return 0;
	}

	// recevie data
	for(i=0;i<len;i++)
	{
		//FL_DelayCnt(50);
		if(Read_BYTE_FROM_I2C(&buf[i]) == 0)
		{
			if(i2cHang)
			{
				i2cHang = 0;
				I2C_ProtocolErr();
			}
			I2C_Stop();
			return 0;
		}

		if(i < len-1)
		{
			// return ack
			I2C_Ack();
			while(SSPCON2bits.ACKEN );
		}
	}
	// last byte need to return nack
	I2C_NotAck();
	while ( SSPCON2bits.ACKEN ); // wait until ACK sequence is over

	I2C_Stop();
#endif
	return 1;
}


/** EOF user.c ***************************************************************/
