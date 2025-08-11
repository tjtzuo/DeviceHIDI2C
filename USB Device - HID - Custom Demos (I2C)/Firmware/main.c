/********************************************************************
 FileName:		main.c
 Dependencies:	See INCLUDES section
 Processor:		PIC18 USB Microcontrollers
 Hardware:		The code is natively intended to be used on the following
				hardware platforms: PICDEM™ FS USB Demo Board.
				The firmware may be
				modified for use on other USB platforms by editing the
				HardwareProfile.h file.
 Complier:		Microchip C18 (for PIC18)
 Company:		Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the “Company”) for its PIC® Microcontroller is intended and
 supplied to you, the Company’s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
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
#include "usb_config.h"
#include "./USB/usb_device.h"
#include "./USB/usb.h"
#include "./USB/usb_function_hid.h"
#include "HardwareProfile.h"

#include "user.h"							// Modifiable


/** CONFIGURATION **************************************************/
#if defined(PICDEM_FS_USB)		// Configuration bits for PICDEM FS USB Demo Board (based on PIC18F4550)
		#pragma config PLLDIV	= 5			// (20 MHz crystal on PICDEM FS USB board)
		#if (USB_SPEED_OPTION == USB_FULL_SPEED)
			#pragma config CPUDIV	= OSC1_PLL2
		#else
			#pragma config CPUDIV	= OSC3_PLL4
		#endif
		#pragma config USBDIV	= 2			// Clock source from 96MHz PLL/2
		#pragma config FOSC		= HSPLL_HS
		#pragma config FCMEN	= OFF
		#pragma config IESO		= OFF
		#pragma config PWRT		= OFF
		#pragma config BOR		= ON
		#pragma config BORV		= 3
		#pragma config VREGEN	= ON	  //USB Voltage Regulator
		#pragma config WDT		= OFF
		#pragma config WDTPS	= 32768
		#pragma config MCLRE	= ON
		#pragma config LPT1OSC	= OFF
		#pragma config PBADEN	= OFF
//		#pragma config CCP2MX	= ON
		#pragma config STVREN	= ON
		#pragma config LVP		= OFF
//		#pragma config ICPRT	= OFF		// Dedicated In-Circuit Debug/Programming
		#pragma config XINST	= OFF		// Extended Instruction Set
		#pragma config CP0		= OFF
		#pragma config CP1		= OFF
//		#pragma config CP2		= OFF
//		#pragma config CP3		= OFF
		#pragma config CPB		= OFF
//		#pragma config CPD		= OFF
		#pragma config WRT0		= OFF
		#pragma config WRT1		= OFF
//		#pragma config WRT2		= OFF
//		#pragma config WRT3		= OFF
		#pragma config WRTB		= OFF		// Boot Block Write Protection
		#pragma config WRTC		= OFF
//		#pragma config WRTD		= OFF
		#pragma config EBTR0	= OFF
		#pragma config EBTR1	= OFF
//		#pragma config EBTR2	= OFF
//		#pragma config EBTR3	= OFF
		#pragma config EBTRB	= OFF

#elif defined(PICDEM_FS_USB_K50)
		#pragma config PLLSEL	= PLL3X		// 3X PLL multiplier selected
		#pragma config CFGPLLEN = OFF		// PLL turned on during execution
		#pragma config CPUDIV	= NOCLKDIV	// 1:1 mode (for 48MHz CPU)
		#pragma config LS48MHZ	= SYS48X8	// Clock div / 8 in Low Speed USB mode
		#pragma config FOSC		= INTOSCIO	// HFINTOSC selected at powerup, no clock out
		#pragma config PCLKEN	= OFF		// Primary oscillator driver
		#pragma config FCMEN	= OFF		// Fail safe clock monitor
		#pragma config IESO		= OFF		// Internal/external switchover (two speed startup)
		#pragma config nPWRTEN	= OFF		// Power up timer
		#pragma config BOREN	= SBORDIS	// BOR enabled
		#pragma config nLPBOR	= ON		// Low Power BOR
		#pragma config WDTEN	= SWON		// Watchdog Timer controlled by SWDTEN
		#pragma config WDTPS	= 32768		// WDT postscalar
		#pragma config PBADEN	= OFF		// Port B Digital/Analog Powerup Behavior
		#pragma config SDOMX	= RC7		// SDO function location
		#pragma config LVP		= OFF		// Low voltage programming
		#pragma config MCLRE	= ON		// MCLR function enabled (RE3 disabled)
		#pragma config STVREN	= ON		// Stack overflow reset
		//#pragma config ICPRT	= OFF		// Dedicated ICPORT program/debug pins enable
		#pragma config XINST	= OFF		// Extended instruction set

#elif defined(LOW_PIN_COUNT_USB_DEVELOPMENT_KIT)
		//Config bit settings for LPC Dev Kit Board when using PIC18F14K50
		#pragma config CPUDIV = NOCLKDIV
		#pragma config USBDIV = OFF
		#pragma config FOSC	  = HS
		#pragma config PLLEN  = ON
		#pragma config FCMEN  = OFF
		#pragma config IESO	  = OFF
		#pragma config PWRTEN = OFF
		#pragma config BOREN  = ON
		#pragma config BORV	  = 30
		#pragma config WDTEN  = OFF
		#pragma config WDTPS  = 32768
		#pragma config MCLRE  = OFF
		#pragma config HFOFST = OFF
		#pragma config STVREN = ON
		#pragma config LVP	  = OFF
		#pragma config XINST  = OFF
		#pragma config BBSIZ  = OFF
		#pragma config CP0	  = OFF
		#pragma config CP1	  = OFF
		#pragma config CPB	  = OFF
		#pragma config WRT0	  = OFF
		#pragma config WRT1	  = OFF
		#pragma config WRTB	  = OFF
		#pragma config WRTC	  = OFF
		#pragma config EBTR0  = OFF
		#pragma config EBTR1  = OFF
		#pragma config EBTRB  = OFF

#else
	#error No hardware board defined, see "HardwareProfile.h" and __FILE__
#endif


/** VARIABLES ******************************************************/
//#pragma udata

//The ReceivedDataBuffer[] and ToSendDataBuffer[] arrays are used as
//USB packet buffers in this firmware.	Therefore, they must be located in
//a USB module accessible portion of microcontroller RAM.
#if defined(__18F14K50) || defined(__18F13K50) || defined(__18LF14K50) || defined(__18LF13K50)
	#pragma udata usbram2
#elif defined(__18F2455) || defined(__18F2550) || defined(__18F4455) || defined(__18F4550)\
		|| defined(__18F2458) || defined(__18F2553) || defined(__18F4458) || defined(__18F4553)\
		|| defined(__18LF24K50) || defined(__18F24K50) || defined(__18LF25K50)\
		|| defined(__18F25K50) || defined(__18LF45K50) || defined(__18F45K50)
	#pragma udata USB_VARIABLES=0x500
#elif defined(__18F4450) || defined(__18F2450)
	#pragma udata USB_VARIABLES=0x480
#else
	#pragma udata
#endif

DATA_PACKET ReceivedDataBuffer;
DATA_PACKET ToSendDataBuffer;

#if defined(__18CXX)
	#pragma udata
#endif

USB_HANDLE USBOutHandle = 0;	//USB handle.  Must be initialized to 0 at startup.
USB_HANDLE USBInHandle = 0;		//USB handle.  Must be initialized to 0 at startup.

/** PRIVATE PROTOTYPES *********************************************/
static void InitializeSystem(void);
void YourHighPriorityISRCode(void);
void YourLowPriorityISRCode(void);

/** VECTOR REMAPPING ***********************************************/
#if defined(__18CXX)
	//On PIC18 devices, addresses 0x00, 0x08, and 0x18 are used for
	//the reset, high priority interrupt, and low priority interrupt
	//vectors.	However, the Microchip HID bootloader occupies the
	//0x00-0xFFF program memory region.	 Therefore, the bootloader code remaps
	//these vectors to new locations as indicated below.  This remapping is
	//only necessary if you wish to be able to (optionally) program the hex file
	//generated from this project with the USB bootloader.
	//If no bootloader is used, edit the
	//usb_config.h file and comment out the following defines:
	//#define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER
	//#define PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER

#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
	#define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
	#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
	#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018
#elif defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
	#define REMAPPED_RESET_VECTOR_ADDRESS			0x800
	#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
	#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818
#else
	#define REMAPPED_RESET_VECTOR_ADDRESS			0x00
	#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x08
	#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x18
#endif

#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
	//Note: If this project is built while one of the bootloaders has
	//been defined, but then the output hex file is not programmed with
	//the bootloader, addresses 0x08 and 0x18 would end up programmed with 0xFFFF.
	//As a result, if an actual interrupt was enabled and occured, the PC would jump
	//to 0x08 (or 0x18) and would begin executing "0xFFFF" (unprogrammed space).  This
	//executes as nop instructions, but the PC would eventually reach the REMAPPED_RESET_VECTOR_ADDRESS
	//(0x1000 or 0x800, depending upon bootloader), and would execute the "goto _startup".	This
	//would effective reset the application.

	//To fix this situation, we should always deliberately place a
	//"goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS" at address 0x08, and a
	//"goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS" at address 0x18.	 When the output
	//hex file of this project is programmed with the bootloader, these sections do not
	//get bootloaded (as they overlap the bootloader space).  If the output hex file is not
	//programmed using the bootloader, then the below goto instructions do get programmed,
	//and the hex file still works like normal.	 The below section is only required to fix this
	//scenario.
	#pragma code HIGH_INTERRUPT_VECTOR = 0x08
	void High_ISR (void)
	{
		_asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#pragma code LOW_INTERRUPT_VECTOR = 0x18
	void Low_ISR (void)
	{
		_asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
	}
#endif	//end of "#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER)"

#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
	extern void _startup (void);		// See c018i.c in your C18 compiler dir
	#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
	void _reset (void)
	{
		_asm goto _startup _endasm
	}
#endif

	#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
	void Remapped_High_ISR (void)
	{
		_asm goto YourHighPriorityISRCode _endasm
	}
	#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
	void Remapped_Low_ISR (void)
	{
		_asm goto YourLowPriorityISRCode _endasm
	}
	#pragma code


	//These are your actual interrupt handling routines.
	#pragma interrupt YourHighPriorityISRCode
	void YourHighPriorityISRCode()
	{
		//Check which interrupt flag caused the interrupt.
		//Service the interrupt
		//Clear the interrupt flag
		//Etc.
		#if defined(USB_INTERRUPT)
			USBDeviceTasks();
		#endif

	}	//This return will be a "retfie fast", since this is in a #pragma interrupt section
	#pragma interruptlow YourLowPriorityISRCode
	void YourLowPriorityISRCode()
	{
		//Check which interrupt flag caused the interrupt.
		//Service the interrupt
		//Clear the interrupt flag
		//Etc.

	}	//This return will be a "retfie", since this is in a #pragma interruptlow section
#endif


/** DECLARATIONS ***************************************************/
#if defined(__18CXX)
	#pragma code
#endif

/********************************************************************
 * Function:		void main(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		Main program entry point.
 *
 * Note:			None
 *******************************************************************/
#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{
	InitializeSystem();

#if defined(USB_INTERRUPT)
	USBDeviceAttach();
#endif

	while (1)
	{
#if defined(USB_POLLING)
		// Check bus status and service USB interrupts.
		USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
						// this function periodically.	This function will take care
						// of processing and responding to SETUP transactions
						// (such as during the enumeration process when you first
						// plug in).	USB hosts require that USB devices should accept
						// and process SETUP packets in a timely fashion.	 Therefore,
						// when using polling, this function should be called
						// frequently (such as once about every 100 microseconds) at any
						// time that a SETUP packet might reasonably be expected to
						// be sent by the host to your device.	In most cases, the
						// USBDeviceTasks() function does not take very long to
						// execute (~50 instruction cycles) before it returns.
#endif


		// Application-specific tasks.
		// Application related code may be added here, or in the ProcessIO() function.
		ProcessIO();
	}//end while
}//end main


/********************************************************************
 * Function:		static void InitializeSystem(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		InitializeSystem is a centralize initialization
 *					routine. All required USB initialization routines
 *					are called from here.
 *
 *					User application initialization routine should
 *					also be called from here.
 *
 * Note:			None
 *******************************************************************/
static void InitializeSystem(void)
{
#if defined(__18CXX)
	ADCON1 |= 0x0F;					// Default all pins to digital
#endif

// Enable all digital input buffer
#if defined(__18F14K50)
	ANSEL = 0;
	ANSELH = 0;
#endif

	#if defined(PIC18F45K50_FAMILY)
		//Configure oscillator settings for clock settings compatible with USB
		//operation.  Note: Proper settings depends on USB speed (full or low).
		#if(USB_SPEED_OPTION == USB_FULL_SPEED)
			OSCTUNE = 0x80; //3X PLL ratio mode selected
			OSCCON = 0x70;	//Switch to 16MHz HFINTOSC
			OSCCON2 = 0x10; //Enable PLL, SOSC, PRI OSC drivers turned off
			while(OSCCON2bits.PLLRDY != 1);	  //Wait for PLL lock
			*((unsigned char*)0xFB5) = 0x90;  //Enable active clock tuning for USB operation
		#endif
		//Configure all I/O pins for digital mode (except RA0/AN0 which has POT on demo board)
		ANSELA = 0x01;
		ANSELB = 0x00;
		ANSELC = 0x00;
		ANSELD = 0x00;
		ANSELE = 0x00;
	#endif


//	The USB specifications require that USB peripheral devices must never source
//	current onto the Vbus pin.	Additionally, USB peripherals should not source
//	current on D+ or D- when the host/hub is not actively powering the Vbus line.
//	When designing a self powered (as opposed to bus powered) USB peripheral
//	device, the firmware should make sure not to turn on the USB module and D+
//	or D- pull up resistor unless Vbus is actively powered.	 Therefore, the
//	firmware needs some means to detect when Vbus is being powered by the host.
//	A 5V tolerant I/O pin can be connected to Vbus (through a resistor), and
//	can be used to detect when Vbus is high (host actively powering), or low
//	(host is shut down or otherwise not supplying power).  The USB firmware
//	can then periodically poll this I/O pin to know when it is okay to turn on
//	the USB module/D+/D- pull up resistor.	When designing a purely bus powered
//	peripheral device, it is not possible to source current on D+ or D- when the
//	host is not actively providing power on Vbus. Therefore, implementing this
//	bus sense feature is optional.	This firmware can be made to use this bus
//	sense feature by making sure "USE_USB_BUS_SENSE_IO" has been defined in the
//	HardwareProfile.h file.
#if defined(USE_USB_BUS_SENSE_IO)
	tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
#endif

//	If the host PC sends a GetStatus (device) request, the firmware must respond
//	and let the host know if the USB peripheral device is currently bus powered
//	or self powered.  See chapter 9 in the official USB specifications for details
//	regarding this request.	 If the peripheral device is capable of being both
//	self and bus powered, it should not return a hard coded value for this request.
//	Instead, firmware should check if it is currently self or bus powered, and
//	respond accordingly.  If the hardware has been configured like demonstrated
//	on the PICDEM FS USB Demo Board, an I/O pin can be polled to determine the
//	currently selected power source.  On the PICDEM FS USB Demo Board, "RA2"
//	is used for	this purpose.  If using this feature, make sure "USE_SELF_POWER_SENSE_IO"
//	has been defined in HardwareProfile.h, and that an appropriate I/O pin has been mapped
//	to it in HardwareProfile.h.
#if defined(USE_SELF_POWER_SENSE_IO)
	tris_self_power = INPUT_PIN;	// See HardwareProfile.h
#endif

	UserInit();

	USBDeviceInit();	//usb_device.c. Initializes USB module SFRs and firmware
						//variables to known states.
}//end InitializeSystem

#if 0

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
/*
	old_sw2 = sw2;
	old_sw3 = sw3;

	//Initialize I/O pin and ADC settings to collect potentiometer measurements
	mInitPOT();
*/
	//initialize the variable holding the handle for the last
	// transmission
	USBOutHandle = 0;
	USBInHandle = 0;

	memset(ToSendDataBuffer, -1, sizeof(ToSendDataBuffer));

	blinkStatusValid = TRUE;
/*
	//Initialize all of the mouse data to 0,0,0 (no movement)
	buffer[0]=buffer[1]=buffer[2]=0;

	//enable emulation mode.  This means that the mouse data
	//will be send to the PC causing the mouse to move.	 If this is
	//set to FALSE then the demo board will send 0,0,0 resulting
	//in no mouse movement
	emulate_mode = TRUE;

	//initialize the variable holding the handle for the last
	// transmission
	lastTransmission = 0;
*/
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

	// Start Generic HID specific

	if (usb_state == 'r')
	{
		//Check if we have received an OUT data packet from the host
		if (!HIDRxHandleBusy(USBOutHandle))//Check if data was received from the host.
		{
			//if the handle is no longer busy then the last
			//transmission is complete

			// The CPU owns the endpoint. Check for received data.

			BYTE len = USBHandleGetLength(USBOutHandle);

			if (len > 1)	// at least 2 bytes
			{
				BYTE count = ReceivedDataBuffer[1];

				//We just received a packet of data from the USB host.
				//Check the first byte of the packet to see what command the host
				//application software wants us to fulfill.
				//Look at the data the host sent, to see what kind of application specific command it sent.
				//Echo back to the host PC the command we are fulfilling in the first byte.
				switch (ToSendDataBuffer[0] = ReceivedDataBuffer[0])
				{
				case READ_VERSION:
					// ToSendDataBuffer[0] = READ_VERSION;
					if (count == 2)
					{
						ToSendDataBuffer[1] = 2;
						ToSendDataBuffer[2] = MINOR_VERSION;
						ToSendDataBuffer[3] = MAJOR_VERSION;
					}
					else
						ToSendDataBuffer[1] = ToSendDataBuffer[2] = ToSendDataBuffer[3] = 0xFF;
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
					ToSendDataBuffer[1] = 0;
					ToSendDataBuffer[2] = ToSendDataBuffer[3] = 0xFF;
					break;

				case 0x81:	//Get Pushbutton State command
					ToSendDataBuffer[1] = 1;
					if (sw3 == 1)							//pushbutton not pressed, pull up resistor on circuit board is pulling the PORT pin high
					{
						ToSendDataBuffer[2] = 0x01;
					}
					else									//sw3 must be == 0, pushbutton is pressed and overpowering the pull up resistor
					{
						ToSendDataBuffer[2] = 0x00;
					}
					ToSendDataBuffer[3] = 0xFF;
					break;

				case RD_POT:	//Read POT (analog voltage) command. Uses ADC to measure an analog voltage on one of the ANxx I/O pins, and returns the result to the host
				{
					WORD_VAL w;

					w = ReadPOT();					//Use ADC to read the I/O pin voltage.	See the relevant HardwareProfile - xxxxx.h file for the I/O pin that it will measure.
													//Some demo boards, like the PIC18F87J50 FS USB Plug-In Module board, do not have a potentiometer (when used stand alone).
													//This function call will still measure the analog voltage on the I/O pin however.	To make the demo more interesting, it
													//is suggested that an external adjustable analog voltage should be applied to this pin.
					ToSendDataBuffer[1] = 2;
					ToSendDataBuffer[2] = w.v[0];	//Measured analog voltage LSB
					ToSendDataBuffer[3] = w.v[1];	//Measured analog voltage MSB
					break;
				}

				case SPI_Write:
					if (count == 2)
					{
						ToSendDataBuffer[3] = ReceivedDataBuffer[3];
						ToSendDataBuffer[2] = ReceivedDataBuffer[2];
						ToSendDataBuffer[1] = ReceivedDataBuffer[1];//ACK
//						ToSendDataBuffer[1] = 0xFF;//NACK
					}
					else
						ToSendDataBuffer[1] = ToSendDataBuffer[2] = ToSendDataBuffer[3] = 0xFF;
					break;

				case SPI_Read:
					if (count == 1)
					{
						ToSendDataBuffer[3] = ReceivedDataBuffer[3];
						ToSendDataBuffer[2] = ReceivedDataBuffer[2];
						ToSendDataBuffer[1] = 2;//ACK
//						ToSendDataBuffer[1] = 0xFF;//NACK
						break;
					}

				default://NACK
					ToSendDataBuffer[1] = ToSendDataBuffer[2] = ToSendDataBuffer[3] = 0xFF;
					break;
				}

				// Ready to transmit the received data back to the host.

				usb_state = 't';
			}
		}
	}

	if (usb_state == 't')
	{
		//Check to make sure the endpoint/buffer is free before we modify the contents
		if (!HIDTxHandleBusy(USBInHandle))
		{
			// The CPU owns the endpoint. Prepare to send data to the host.

			//Prepare the USB module to send the data packet to the host
			USBInHandle = HIDTxPacket(HID_EP, ToSendDataBuffer, sizeof(ToSendDataBuffer));

			//Re-arm the OUT endpoint, so we can receive the next OUT data packet
			//that the host may try to send us.
			USBOutHandle = HIDRxPacket(HID_EP, ReceivedDataBuffer, sizeof(ReceivedDataBuffer));

			// Return to checking for new received data.

			usb_state = 'r';
		}
	}

/*
	if (Switch3IsPressed())		//Note: Switch3IsPressed() implements only the
	{							//crudest of switch debounce code. As a result
								//some pusbbuttons will behave temperamentally.
								//Proper debounce code should be used which
								//implements delays millseconds long, and
								//checks/reckecks pushbutton state many times to
								//verify that the state is stable.	In order
								//to avoid using blocking functions, or
								//microcontroller timer resources this feature
								//is not implemented in this example.
		emulate_mode = !emulate_mode;
	}

	//Call the function that emulates the mouse
	Emulate_Mouse();
*/
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
WORD_VAL ReadPOT(void)
{
	WORD_VAL w;

#if defined(__18CXX)
	mInitPOT();

	ADCON0bits.GO = 1;				// Start AD conversion
	while (ADCON0bits.NOT_DONE);		// Wait for conversion

	w.v[0] = ADRESL;
	w.v[1] = ADRESH;
#endif

	return w;
}//end ReadPOT


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

	if (USBSuspendControl == 1)
	{
		if (led_count == 0)
		{
			mLED_1_Toggle();
			if (mGetLED_1())
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
		if (USBDeviceState == DETACHED_STATE)
		{
			mLED_Both_Off();
		}
		else if (USBDeviceState == ATTACHED_STATE)
		{
			mLED_Both_On();
		}
		else if (USBDeviceState == POWERED_STATE)
		{
			mLED_Only_1_On();
		}
		else if (USBDeviceState == DEFAULT_STATE)
		{
			mLED_Only_2_On();
		}
		else if (USBDeviceState == ADDRESS_STATE)
		{
			if (led_count == 0)
			{
				mLED_1_Toggle();
				mLED_2_Off();
			}//end if
		}
		else if (USBDeviceState == CONFIGURED_STATE)
		{
			if (led_count == 0)
			{
				mLED_1_Toggle();
				if (mGetLED_1())
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

#endif


// ******************************************************************************************************
// ************** USB Callback Functions ****************************************************************
// ******************************************************************************************************
// The USB firmware stack will call the callback functions USBCBxxx() in response to certain USB related
// events.	For example, if the host PC is powering down, it will stop sending out Start of Frame (SOF)
// packets to your device.	In response to this, all USB devices are supposed to decrease their power
// consumption from the USB Vbus to <2.5mA each.  The USB module detects this condition (which according
// to the USB specifications is 3+ms of no bus activity/SOF packets) and then calls the USBCBSuspend()
// function.  You should modify these callback functions to take appropriate actions for each of these
// conditions.	For example, in the USBCBSuspend(), you may wish to add code that will decrease power
// consumption from Vbus to <2.5mA (such as by clock switching, turning off LEDs, putting the
// microcontroller to sleep, etc.).	 Then, in the USBCBWakeFromSuspend() function, you may then wish to
// add code that undoes the power saving things done in the USBCBSuspend() function.

// The USBCBSendResume() function is special, in that the USB stack will not automatically call this
// function.  This function is meant to be called from the application firmware instead.  See the
// additional comments near the function.

/******************************************************************************
 * Function:		void USBCBSuspend(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		Call back that is invoked when a USB suspend is detected
 *
 * Note:			None
 *****************************************************************************/
void USBCBSuspend(void)
{
	//Example power saving code.  Insert appropriate code here for the desired
	//application behavior.	 If the microcontroller will be put to sleep, a
	//process similar to that shown below may be used:

	//ConfigureIOPinsForLowPower();
	//SaveStateOfAllInterruptEnableBits();
	//DisableAllInterruptEnableBits();
	//EnableOnlyTheInterruptsWhichWillBeUsedToWakeTheMicro();	//should enable at least USBActivityIF as a wake source
	//Sleep();
	//RestoreStateOfAllPreviouslySavedInterruptEnableBits();	//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.
	//RestoreIOPinsToNormal();									//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.

	//IMPORTANT NOTE: Do not clear the USBActivityIF (ACTVIF) bit here.	 This bit is
	//cleared inside the usb_device.c file.	 Clearing USBActivityIF here will cause
	//things to not work as intended.


}

/******************************************************************************
 * Function:		void _USB1Interrupt(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		This function is called when the USB interrupt bit is set
 *					In this example the interrupt is only used when the device
 *					goes to sleep when it receives a USB suspend command
 *
 * Note:			None
 *****************************************************************************/
/*
void __attribute__ ((interrupt)) _USB1Interrupt(void)
{
	#if !defined(self_powered)
		if(U1OTGIRbits.ACTVIF)
		{
			IEC5bits.USB1IE = 0;
			U1OTGIEbits.ACTVIE = 0;
			IFS5bits.USB1IF = 0;

			//USBClearInterruptFlag(USBActivityIFReg,USBActivityIFBitNum);
			USBClearInterruptFlag(USBIdleIFReg,USBIdleIFBitNum);
			//USBSuspendControl = 0;
		}
	#endif
}
*/

/******************************************************************************
 * Function:		void USBCBWakeFromSuspend(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		The host may put USB peripheral devices in low power
 *					suspend mode (by "sending" 3+ms of idle).  Once in suspend
 *					mode, the host may wake the device back up by sending non-
 *					idle state signalling.
 *
 *					This call back is invoked when a wakeup from USB suspend
 *					is detected.
 *
 * Note:			None
 *****************************************************************************/
void USBCBWakeFromSuspend(void)
{
	// If clock switching or other power savings measures were taken when
	// executing the USBCBSuspend() function, now would be a good time to
	// switch back to normal full power run mode conditions.  The host allows
	// a few milliseconds of wakeup time, after which the device must be
	// fully back to normal, and capable of receiving and processing USB
	// packets.	 In order to do this, the USB module must receive proper
	// clocking (IE: 48MHz clock must be available to SIE for full speed USB
	// operation).
	// Make sure the selected oscillator settings are consistent with USB
	// operation before returning from this function.
}

/********************************************************************
 * Function:		void USBCB_SOF_Handler(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		The USB host sends out a SOF packet to full-speed
 *					devices every 1 ms. This interrupt may be useful
 *					for isochronous pipes. End designers should
 *					implement callback routine as necessary.
 *
 * Note:			None
 *******************************************************************/
void USBCB_SOF_Handler(void)
{
	// No need to clear UIRbits.SOFIF to 0 here.
	// Callback caller is already doing that.
}

/*******************************************************************
 * Function:		void USBCBErrorHandler(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		The purpose of this callback is mainly for
 *					debugging during development. Check UEIR to see
 *					which error causes the interrupt.
 *
 * Note:			None
 *******************************************************************/
void USBCBErrorHandler(void)
{
	// No need to clear UEIR to 0 here.
	// Callback caller is already doing that.

	// Typically, user firmware does not need to do anything special
	// if a USB error occurs.  For example, if the host sends an OUT
	// packet to your device, but the packet gets corrupted (ex:
	// because of a bad connection, or the user unplugs the
	// USB cable during the transmission) this will typically set
	// one or more USB error interrupt flags.  Nothing specific
	// needs to be done however, since the SIE will automatically
	// send a "NAK" packet to the host.	 In response to this, the
	// host will normally retry to send the packet again, and no
	// data loss occurs.  The system will typically recover
	// automatically, without the need for application firmware
	// intervention.

	// Nevertheless, this callback function is provided, such as
	// for debugging purposes.
}


/*******************************************************************
 * Function:		void USBCBCheckOtherReq(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		When SETUP packets arrive from the host, some
 *					firmware must process the request and respond
 *					appropriately to fulfill the request.  Some of
 *					the SETUP packets will be for standard
 *					USB "chapter 9" (as in, fulfilling chapter 9 of
 *					the official USB specifications) requests, while
 *					others may be specific to the USB device class
 *					that is being implemented.	For example, a HID
 *					class device needs to be able to respond to
 *					"GET REPORT" type of requests.	This
 *					is not a standard USB chapter 9 request, and
 *					therefore not handled by usb_device.c.	Instead
 *					this request should be handled by class specific
 *					firmware, such as that contained in usb_function_hid.c.
 *
 * Note:			None
 *******************************************************************/
void USBCBCheckOtherReq(void)
{
	USBCheckHIDRequest();
}//end


/*******************************************************************
 * Function:		void USBCBStdSetDscHandler(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		The USBCBStdSetDscHandler() callback function is
 *					called when a SETUP, bRequest: SET_DESCRIPTOR request
 *					arrives.  Typically SET_DESCRIPTOR requests are
 *					not used in most applications, and it is
 *					optional to support this type of request.
 *
 * Note:			None
 *******************************************************************/
void USBCBStdSetDscHandler(void)
{
	// Must claim session ownership if supporting this request
}//end


/*******************************************************************
 * Function:		void USBCBInitEP(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		This function is called when the device becomes
 *					initialized, which occurs after the host sends a
 *					SET_CONFIGURATION (wValue not = 0) request.	 This
 *					callback function should initialize the endpoints
 *					for the device's usage according to the current
 *					configuration.
 *
 * Note:			None
 *******************************************************************/
void USBCBInitEP(void)
{
	//enable the HID endpoint
	USBEnableEndpoint(HID_EP, USB_IN_ENABLED | USB_OUT_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);
//	USBEnableEndpoint(MSD_IN_EP, USB_IN_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);

	//Re-arm the OUT endpoint for the next packet
	USBOutHandle = HIDRxPacket(HID_EP, (BYTE*)&ReceivedDataBuffer, sizeof(ReceivedDataBuffer));
}

/********************************************************************
 * Function:		void USBCBSendResume(void)
 *
 * PreCondition:	None
 *
 * Input:			None
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		The USB specifications allow some types of USB
 *					peripheral devices to wake up a host PC (such
 *					as if it is in a low power suspend to RAM state).
 *					This can be a very useful feature in some
 *					USB applications, such as an Infrared remote
 *					control	receiver.  If a user presses the "power"
 *					button on a remote control, it is nice that the
 *					IR receiver can detect this signalling, and then
 *					send a USB "command" to the PC to wake up.
 *
 *					The USBCBSendResume() "callback" function is used
 *					to send this special USB signalling which wakes
 *					up the PC.	This function may be called by
 *					application firmware to wake up the PC.	 This
 *					function should only be called when:
 *
 *					1.	The USB driver used on the host PC supports
 *						the remote wakeup capability.
 *					2.	The USB configuration descriptor indicates
 *						the device is remote wakeup capable in the
 *						bmAttributes field.
 *					3.	The USB host PC is currently sleeping,
 *						and has previously sent your device a SET
 *						FEATURE setup packet which "armed" the
 *						remote wakeup capability.
 *
 *					This callback should send a RESUME signal that
 *					has the period of 1-15ms.
 *
 * Note:			Interrupt vs. Polling
 *					-Primary clock
 *					-Secondary clock ***** MAKE NOTES ABOUT THIS *******
 *					 > Can switch to primary first by calling USBCBWakeFromSuspend()

 *					The modifiable section in this routine should be changed
 *					to meet the application needs. Current implementation
 *					temporary blocks other functions from executing for a
 *					period of 1-13 ms depending on the core frequency.
 *
 *					According to USB 2.0 specification section 7.1.7.7,
 *					"The remote wakeup device must hold the resume signaling
 *					for at least 1 ms but for no more than 15 ms."
 *					The idea here is to use a delay counter loop, using a
 *					common value that would work over a wide range of core
 *					frequencies.
 *					That value selected is 1800. See table below:
 *					==========================================================
 *					Core Freq(MHz)		MIP			RESUME Signal Period (ms)
 *					==========================================================
 *						48				12			1.05
 *						 4				1			12.6
 *					==========================================================
 *					* These timing could be incorrect when using code
 *					  optimization or extended instruction mode,
 *					  or when having other interrupts enabled.
 *					  Make sure to verify using the MPLAB SIM's Stopwatch
 *					  and verify the actual signal on an oscilloscope.
 *******************************************************************/
void USBCBSendResume(void)
{
	static WORD delay_count;

	USBResumeControl = 1;				// Start RESUME signaling

	delay_count = 1800U;				// Set RESUME line for 1-13 ms
	do
	{
		delay_count--;
	}
	while (delay_count);

	USBResumeControl = 0;
}


/*******************************************************************
 * Function:		BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *						  USB_EVENT event, void *pdata, WORD size)
 *
 * PreCondition:	None
 *
 * Input:			USB_EVENT event - the type of event
 *					void *pdata - pointer to the event data
 *					WORD size - size of the event data
 *
 * Output:			None
 *
 * Side Effects:	None
 *
 * Overview:		This function is called from the USB stack to
 *					notify a user application that a USB event
 *					occured.  This callback is in interrupt context
 *					when the USB_INTERRUPT option is selected.
 *
 * Note:			None
 *******************************************************************/
BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
	switch (event)
	{
	case EVENT_TRANSFER:
		Nop();//Add application specific callback task or callback function here if desired.
		break;
	case EVENT_SOF:
		USBCB_SOF_Handler();
		break;
	case EVENT_SUSPEND:
		USBCBSuspend();
		break;
	case EVENT_RESUME:
		USBCBWakeFromSuspend();
		break;
	case EVENT_CONFIGURED:
		USBCBInitEP();
		break;
	case EVENT_SET_DESCRIPTOR:
		USBCBStdSetDscHandler();
		break;
	case EVENT_EP0_REQUEST:
		USBCBCheckOtherReq();
		break;
	case EVENT_BUS_ERROR:
		USBCBErrorHandler();
		break;
/*
	case EVENT_TRANSFER_TERMINATED:
		//Add application specific callback task or callback function here if desired.
		//The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
		//FEATURE (endpoint halt) request on an application endpoint which was
		//previously armed (UOWN was = 1).	Here would be a good place to:
		//1.  Determine which endpoint the transaction that just got terminated was
		//		on, by checking the handle value in the *pdata.
		//2.  Re-arm the endpoint if desired (typically would be the case for OUT
		//		endpoints).
		break;
*/
	default:
		break;
	}
	return TRUE;
}

/** EOF main.c *************************************************/
