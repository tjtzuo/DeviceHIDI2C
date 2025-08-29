

/** I N C L U D E S **********************************************************/
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "protocol_io.h"
#include "user.h"
#include <i2c.h>
#include <EEP.h>

/** V A R I A B L E S ********************************************************/
#pragma udata
WORD HoldTime;
WORD_VAL utmp[2];
/** P R I V A T E  P R O T O T Y P E S ***************************************/

/** D E C L A R A T I O N S **************************************************/
#pragma code

void InitI2C(void)
{
	SSPSTAT &= 0x3F;				// power on state
	SSPCON1 = 0x00;					// power on state
	SSPCON2 = 0x00;					// power on state
	SSPCON1 |= MASTER;				// select serial mode
	SSPSTAT |= SLEW_OFF;			// slew rate on/off
	//SSPSTAT |= 0x40;		//CKE

	TRISBbits.TRISB4 = 1;	//SDA input
	TRISBbits.TRISB6 = 1;	//SCL input
	SSPCON1 |= SSPENB;				// enable synchronous serial port

	if(Read_b_eep(EEP_I2C_BR)!=0xFF)	//I2C Baud rate had in EEPRom
	{
		SSPADD = Read_b_eep(EEP_I2C_BR); //
	}
	else
	{
		SSPADD = C_I2C_BR;				//tune I2C rate
	}

	//Default I2C hold time
	HoldTime = 2;
}

void I2C_Idle(void)
{

	while ( ( SSPCON2 & 0x1F ) || ( SSPSTATbits.R_W ) );

}

BOOL I2C_Ack(void)
{
	WORD WC = 0;
	SSPCON2bits.ACKDT = 0;			 // set acknowledge bit state for ACK
	SSPCON2bits.ACKEN = 1;			 // initiate bus acknowledge sequence
	while(SSPCON2bits.ACKEN )
	{
		if(WC > i2cTimeout )
		{
			WC = 0;
			//i2cHang = 1;
			return 0;
		}
	}
	WC = 0;
	return 1;
}

BOOL I2C_NotAck(void)
{
	WORD WC = 0;
	SSPCON2bits.ACKDT = 1;			// set acknowledge bit for not ACK
	SSPCON2bits.ACKEN = 1;			// initiate bus acknowledge sequence
	while(SSPCON2bits.ACKEN )
	{
		if(WC > i2cTimeout )
		{
			WC = 0;
			//i2cHang = 1;
			return 0;
		}
	}
	WC = 0;
	return 1;
}
BOOL I2C_Start(void)
{
	WORD WC = 0;
	SSPCON2bits.SEN = 1;	//Start
	while ( SSPCON2bits.SEN )	// wait until start condition is over
	{
		if(WC > i2cTimeout )
		{
			WC = 0;
			//i2cHang = 1;
			return 0;
		}
		WC++;
	}
	WC = 0;
	return 1;
}

BOOL I2C_ReStart(void)
{
	WORD WC = 0;
	SSPCON2bits.RSEN = 1;	// initiate bus restart condition
	while ( SSPCON2bits.RSEN )	// wait until re-start condition is over
	{
		if(WC > i2cTimeout )
		{
			WC = 0;
			//i2cHang = 1;
			return 0;
		}
		WC++;
	}
	WC = 0;
	return 1;
}
BOOL I2C_Stop(void)
{
	WORD WC = 0;
	SSPCON2bits.PEN = 1;	//stop
	while ( SSPCON2bits.PEN )	// wait until stop condition is over
	{
		if(WC > i2cTimeout )
		{
			WC = 0;
			//i2cHang = 1;
			return 0;
		}
		WC++;
	}
	WC = 0;
	return 1;
}

void I2C_ProtocolErr(void)
{
	SSPCON1 &= (~SSPENB);	// disable synchronous serial port
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	SSPCON1 |= SSPENB;
}
//===============GPIO================================
#define SDA_i PORTCbits.RC1
#define SCL_i PORTCbits.RC2
#define SDA_o LATCbits.LATC1
#define SCL_o LATCbits.LATC2

//#define SDA_i PORTBbits.RB4
//#define SCL_i PORTBbits.RB6
//#define SDA_o LATBbits.LATB4
//#define SCL_o LATBbits.LATB6

void FL_SetDIR_SDA(BOOL input)
{
	TRISCbits.TRISC1 = input;
	//TRISBbits.TRISB4 = input;
}
void I2C_Enable(BOOL bEN)
{
	SSPCON1bits.SSPEN = bEN;
	if(!bEN)//enable GPIO mode
	{
		PORTC |= 0x06;//Set SDA & SCL to High Status

		TRISCbits.TRISC2 = 0; //SDA output
		TRISCbits.TRISC1 = 0; //SCL output
		//TRISBbits.TRISB4 = 0;
		//TRISBbits.TRISB6 = 0;
		I2C_Both_High();
	}
	else// enable I2C mode
	{
		//TRISBbits.TRISB4 = 1; //SDA input
		//TRISBbits.TRISB6 = 1; //SCL input
	}
}

void I2C_HoldTime(WORD limitTime)
{
	while(limitTime--);
}

void I2C_Both_Low(void)
{
	LATC &= ~0x06;
	I2C_HoldTime(8000);
}

void I2C_Both_High(void)
{
	LATC |= 0x06;
	I2C_HoldTime(8000);
}

void I2C_Start_IO(void)
{
	SDA_o = 1;
	SCL_o = 1;
	I2C_HoldTime(HoldTime);
	SDA_o = 0;
	I2C_HoldTime(HoldTime);
	SCL_o= 0;
	I2C_HoldTime(HoldTime);
}

void I2C_ReStart_IO(void)
{
	LATC |= 0x06;//Both high
	I2C_HoldTime(HoldTime);
	I2C_Start_IO();
}

void I2C_Stop_IO(void)
{
	SDA_o = 0;
	I2C_HoldTime(HoldTime);
	SCL_o = 0;
	I2C_HoldTime(HoldTime);
	SCL_o = 1;
	I2C_HoldTime(HoldTime);
	SDA_o = 1;
	I2C_HoldTime(HoldTime);
}

void I2C_TX_IO(BYTE txdata)
{
	//#define txcnt utmp[1].v[0]
	//#define shift utmp[1].v[1]
	BYTE txcnt;
	BYTE shift;
	for(txcnt=0;txcnt<8;txcnt++)
	{
		shift = txcnt ^ 0x07;//get bit shift cnt

		SDA_o = (txdata>>shift)&0x01;
		Nop();
		Nop();
		SCL_o = 1;
		I2C_HoldTime(HoldTime);
		SCL_o = 0;
		I2C_HoldTime(HoldTime);
	}
}

BOOL I2C_Rec_Ack(void)
{
	BYTE ack;
	//TRISBbits.TRISB4 = 1; //SDA input
	FL_SetDIR_SDA(1);// SDA input
	SCL_o = 1;
	I2C_HoldTime(HoldTime);
	while(SCL_i != 1)// Detect Clock Stretch
	{
		SCL_o = 1;
		I2C_HoldTime(HoldTime);
	}
	ack = SDA_i;
	I2C_HoldTime(HoldTime);
	SCL_o = 0;
	I2C_HoldTime(HoldTime);
	//TRISBbits.TRISB4 = 0; //SDA output
	FL_SetDIR_SDA(0);// SDA output
	return (ack&0x01);
}

BYTE I2C_RX_IO(void)
{
	//#define rxcnt utmp[1].v[0]
	//#define rxdata utmp[1].v[1]
	BYTE rxcnt;
	BYTE rxdata;
	rxdata = 0;
	//TRISBbits.TRISB4 = 1; //SDA input
	FL_SetDIR_SDA(1);// SDA input
	while(SCL_i != 1)// Detect Clock Stretch
	{
		SCL_o = 1;
	}
	for(rxcnt=0;rxcnt<8;rxcnt++)
	{
		SCL_o = 1;
		rxdata |= SDA_i;
		I2C_HoldTime(HoldTime);
		if(rxcnt<7)
			rxdata <<= 1;
		SCL_o = 0;
		I2C_HoldTime(HoldTime);
	}
	//TRISBbits.TRISB4 = 0; //SDA output
	FL_SetDIR_SDA(0);// SDA output
	return rxdata;
}

void I2C_Send_Ack(BOOL back)
{
	SDA_o = back;
	SCL_o = 1;
	I2C_HoldTime(HoldTime);
	SCL_o = 0;
	I2C_HoldTime(HoldTime);
}
#if 0
void I2C_Pattern(BYTE PatNo)
{
	I2C_Enable(0);//disable I2C mode , enable GPIO mode
	switch(PatNo)
	{
	case 0x00:// scl = 0 && sda = 0
		I2C_Both_Low();
		break;

	case 0x01:// Start -> scl = 0 && sda = 0
		I2C_Start_IO();//Start
		I2C_Both_Low();
		break;

	case 0x02:// Start -> scl = 1 && sda = 1
		I2C_Start_IO();//Start
		I2C_Both_High();
		break;

	case 0x03:// Start -> Adr = 0x16 -> scl = 0 && sda = 0
		I2C_Start_IO();//Start
		I2C_TX_IO(0x16);
		I2C_Both_Low();
		break;

	case 0x04:// Start -> Adr = 0x16 -> scl = 1 && sda = 1
		I2C_Start_IO();//Start
		I2C_TX_IO(0x16);
		I2C_Both_High();
		break;

	case 0x05://Start -> Adr = 0x16 -> Tx -> ReStart -> Both Low
		I2C_Start_IO();//Start
		I2C_TX_IO(0x16);
		I2C_Rec_Ack();
		I2C_TX_IO(0x09);
		I2C_Rec_Ack();
		I2C_ReStart_IO();//Restart
		I2C_Both_Low();
		break;

	case 0x06://Start -> Adr = 0x16 -> Tx -> ReStart -> Both High
		I2C_Start_IO();//Start
		I2C_TX_IO(0x16);
		I2C_Rec_Ack();
		I2C_TX_IO(0x09);
		I2C_Rec_Ack();
		I2C_ReStart_IO();//Restart
		I2C_Both_High();
		break;

	case 0x07://Start -> Adr = 0x16 -> Tx -> ReStart -> Adr = 0x17 -> Rx -> no send ack ->Both Low
		I2C_Start_IO();//Start
		I2C_TX_IO(0x16);
		I2C_Rec_Ack();
		I2C_TX_IO(0x09);
		I2C_Rec_Ack();
		I2C_ReStart_IO();//Restart
		I2C_TX_IO(0x17);
		I2C_Rec_Ack();
		I2C_RX_IO();
		I2C_Both_Low();
		break;

	case 0x08://Start -> Adr = 0x16 -> Tx -> ReStart -> Adr = 0x17 -> Rx -> no send ack -> Both high
		I2C_Start_IO();
		I2C_TX_IO(0x16);
		I2C_Rec_Ack();
		I2C_TX_IO(0x09);
		I2C_Rec_Ack();
		I2C_ReStart_IO();//Restart
		I2C_TX_IO(0x17);
		I2C_Rec_Ack();
		I2C_RX_IO();
		I2C_Both_High();
		break;

	case 0x09://Start -> Adr = 0x16 -> Tx -> ReStart -> Adr = 0x17 -> Rx -> send ack ->Both Low
		I2C_Start_IO();//Start
		I2C_TX_IO(0x16);
		I2C_Rec_Ack();
		I2C_TX_IO(0x09);
		I2C_Rec_Ack();
		I2C_ReStart_IO();//Restart
		I2C_TX_IO(0x17);
		I2C_Rec_Ack();
		I2C_RX_IO();
		I2C_Send_Ack(0);
		I2C_Both_Low();
		break;

	case 0x0A://Start -> Adr = 0x16 -> Tx -> ReStart -> Adr = 0x17 -> Rx -> send ack -> Both high
		I2C_Start_IO();//Start
		I2C_TX_IO(0x16);
		I2C_Rec_Ack();
		I2C_TX_IO(0x09);
		I2C_Rec_Ack();
		I2C_ReStart_IO();//Restart
		I2C_TX_IO(0x17);
		I2C_Rec_Ack();
		I2C_RX_IO();
		I2C_Send_Ack(0);
		I2C_Both_High();
		break;

	case 0x0B://Start -> Restart -> Both Low
		I2C_Start_IO();//Start
		I2C_ReStart_IO();//Restart
		break;

	case 0x0C://Start -> Adr = 0x20 -> Both Low
		I2C_Start_IO();//Start
		I2C_TX_IO(0x20);//Adr = 0x20
		I2C_Both_Low();
		break;

	case 0x0D://Start -> Adr = 0x20 -> Both High
		I2C_Start_IO();//Start
		I2C_TX_IO(0x20);//Adr = 0x20
		I2C_Both_High();
		break;

	case 0x0E://scl = 1 && sda = 0
		SDA_o = 0;
		I2C_HoldTime(HoldTime);
		break;

	case 0x0F://scl = 0 && sda = 1;
		SCL_o = 0;
		I2C_HoldTime(HoldTime);
		break;

	default:
		break;
	}
	I2C_Enable(1);//enable I2C mode , disable GPIO mode
}
#endif
