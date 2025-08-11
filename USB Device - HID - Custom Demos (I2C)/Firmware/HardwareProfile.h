/********************************************************************
 FileName:		HardwareProfile.h
 Dependencies:	See INCLUDES section
 Processor:		PIC18 USB Microcontrollers
 Hardware:		This demo is natively intended to be used on Microchip USB demo
				boards supported by the MCHPFSUSB stack.  See release notes for
				support matrix.	 The firmware may be modified for use on
				other USB platforms by editing this file (HardwareProfile.h)
				and adding a new hardware specific
				HardwareProfile - [platform name].h file.
 Compiler:		Microchip C18 (for PIC18)
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
  2.3	09/15/2008	 Broke out each hardware platform into its own
					 "HardwareProfile - xxx.h" file
********************************************************************/

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

//#define DEMO_BOARD USER_DEFINED_BOARD

#if !defined(DEMO_BOARD)

	#if defined(__18CXX)
		#if defined(__18F4550)
			#include "HardwareProfile - PICDEM FSUSB.h"
		#elif defined(__18F45K50)
			#include "HardwareProfile - PICDEM FSUSB K50.h"
		#elif defined(__18F14K50)
			#include "HardwareProfile - LPC USB Development Kit - PIC18F14K50.h"
		#endif
	#endif

#endif

#if !defined(DEMO_BOARD)
	#error "Demo board not defined.	 Either define DEMO_BOARD for a custom board or select the correct processor for the demo board."
#endif

#endif	//HARDWARE_PROFILE_H
