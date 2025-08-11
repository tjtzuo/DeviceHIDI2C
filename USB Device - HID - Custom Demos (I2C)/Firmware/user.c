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
/*
#if defined(__18CXX)
	#include <spi.h>
#endif
*/

/** V A R I A B L E S ********************************************************/
#if defined(__18CXX)
	#pragma udata
#endif

BYTE old_sw2,old_sw3;
BYTE counter;
BYTE trf_state;
BYTE temp_mode;

BYTE pTemp;						// Pointer to current logging position, will
								// loop to zero once the max index is reached
BYTE valid_temp;				// Keeps count of the valid data points
WORD temp_data[30];				// 30 points of data

BOOL blinkStatusValid = TRUE;

// Begin Generic Hid specific
char usb_state = 'r';
// End Generic Hid specific


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
BOOL Switch2IsPressed(void);
BOOL Switch3IsPressed(void);
void ResetTempLog(void);
WORD_VAL ReadPOT(void);
void ServiceRequests(void);


/** D E C L A R A T I O N S **************************************************/
#if defined(__18CXX)
	#pragma code
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
	old_sw2 = sw2;
	old_sw3 = sw3;
    
//	InitTempSensor();

	//Initialize I/O pin and ADC settings to collect potentiometer measurements
	mInitPOT();

	ResetTempLog();
	temp_mode = TEMP_REAL_TIME;

	#if defined(__18CXX)
	/* Init Timer0 for data logging interval (every 1 second) */
	T0CON = 0b10010111;
	#endif
/*
	//initialize the variable holding the handle for the last
	// transmission
	USBOutHandle = 0;
	USBInHandle = 0;
*/
	memset(&ToSendDataBuffer, -1, sizeof(ToSendDataBuffer));

	blinkStatusValid = TRUE;
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
 * Function:		void ResetTempLog(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	pTemp = 0; valid_temp = 0;
 *
 * Overview:		This function resets the temperature logging variables
 *
 * Note:			None
 *****************************************************************************/
void ResetTempLog(void)
{
	pTemp = 0;
	valid_temp = 0;
}//end ResetLog

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
WORD_VAL ReadPOT(void)
{
	WORD_VAL w;

	w.Val = 0;

	#if defined(__18CXX) || defined(__XC8)
			ADCON0bits.GO = 1;				// Start AD conversion
			while(ADCON0bits.GO);	  // Wait for conversion
			w.v[0] = ADRESL;
			w.v[1] = ADRESH;
	#endif

	return w;
}//end ReadPOT

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
//	BYTE index;

	// Start Generic HID specific

	if (usb_state == 'r')
	{
		//Check if we have received an OUT data packet from the host
		if (!HIDRxHandleBusy(USBOutHandle))
		{
			//if the handle is no longer busy then the last
			//transmission is complete

			counter = 0;

			INPacket.CMD = OUTPacket.CMD;
			INPacket.len = OUTPacket.len;

			//We just received a packet of data from the USB host.
			//Check the first byte of the packet to see what command the host
			//application software wants us to fulfill.
			switch (OUTPacket.CMD)				//Look at the data the host sent, to see what kind of application specific command it sent.
			{
			case READ_VERSION:
				INPacket._byte[2] = MINOR_VERSION;
				INPacket._byte[3] = MAJOR_VERSION;
				counter = 4;
				break;
				/*
							case ID_BOARD:
								counter = 1;
								if(OUTPacket.ID == 0)
								{
									mLED_3_Off();mLED_4_Off();
								}
								else if(OUTPacket.ID == 1)
								{
									mLED_3_Off();mLED_4_On();
								}
								else if(OUTPacket.ID == 2)
								{
									mLED_3_On();mLED_4_Off();
								}
								else if(OUTPacket.ID == 3)
								{
									mLED_3_On();mLED_4_On();
								}
								else
									counter = 0;
								break;

							case UPDATE_LED:
								// LED1 & LED2 are used as USB event indicators.
								if(OUTPacket.led_num == 3)
								{
									if(OUTPacket.led_status)
									{
										mLED_3_On();
									}
									else
									{
										mLED_3_Off();
									}
									counter = 1;
								}//end if
								else if(OUTPacket.led_num == 4)
								{
									if(OUTPacket.led_status)
									{
										mLED_4_On();
									}
									else
									{
										mLED_4_Off();
									}
									counter = 1;
								}//end if else
								break;
				*/
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
				counter = 2;
				break;

			case 0x81:	//Get push button state
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

			case RD_POT:	//Read POT command.	 Uses ADC to measure an analog voltage on one of the ANxx I/O pins, and returns the result to the host
			{
				WORD_VAL w = ReadPOT();					//Use ADC to read the I/O pin voltage.	See the relevant HardwareProfile - xxxxx.h file for the I/O pin that it will measure.
														//Some demo boards, like the PIC18F87J50 FS USB Plug-In Module board, do not have a potentiometer (when used stand alone).
														//This function call will still measure the analog voltage on the I/O pin however.	To make the demo more interesting, it
														//is suggested that an external adjustable analog voltage should be applied to this pin.
//				ToSendDataBuffer[1] = 2;
//				ToSendDataBuffer[2] = w.v[0];	//Measured analog voltage LSB
//				ToSendDataBuffer[3] = w.v[1];	//Measured analog voltage MSB
				ToSendDataBuffer._word[1] = w.Val;

				counter = 4;
				break;
			}

			default:
				break;
			}

			INPacket._byte[USBGEN_EP_SIZE - 1] = counter;

			// Ready to transmit the received data back to the host.
			usb_state = 't';
		}
	}

	if (usb_state == 't')
	{
		//Check to make sure the endpoint/buffer is free before we modify the contents
		if (!HIDTxHandleBusy(USBInHandle))
		{
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
	static WORD led_count=0;

	if(led_count == 0)led_count = 10000U;
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

/******************************************************************************
 * Function:		BOOL Switch2IsPressed(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			BOOL - TRUE if the SW2 was pressed and FALSE otherwise
 *
 * Side Effects:	None
 *
 * Overview:		returns TRUE if the SW2 was pressed and FALSE otherwise
 *
 * Note:			None
 *****************************************************************************/
BOOL Switch2IsPressed(void)
{
	if(sw2 != old_sw2)
	{
		old_sw2 = sw2;					// Save new value
		if(sw2 == 0)					// If pressed
			return TRUE;				// Was pressed
	}//end if
	return FALSE;						// Was not pressed
}//end Switch2IsPressed

/******************************************************************************
 * Function:		BOOL Switch3IsPressed(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			BOOL - TRUE if the SW3 was pressed and FALSE otherwise
 *
 * Side Effects:	None
 *
 * Overview:		returns TRUE if the SW3 was pressed and FALSE otherwise
 *
 * Note:			None
 *****************************************************************************/
BOOL Switch3IsPressed(void)
{
	if(sw3 != old_sw3)
	{
		old_sw3 = sw3;					// Save new value
		if(sw3 == 0)					// If pressed
			return TRUE;				// Was pressed
	}//end if
	return FALSE;						// Was not pressed
}//end Switch3IsPressed

/** EOF user.c ***************************************************************/
