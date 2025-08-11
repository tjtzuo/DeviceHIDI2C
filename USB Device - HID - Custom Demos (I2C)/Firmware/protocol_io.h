#ifndef PROTOCOL_IO_H
#define PROTOCOL_IO_H

#define Debug_Port	0
extern WORD i2cTimeout;
void InitI2C(void);
void I2C_Idle(void);
BOOL I2C_Ack(void);
BOOL I2C_NotAck(void);
BOOL I2C_Start(void);
BOOL I2C_ReStart(void);
BOOL I2C_Stop(void);
void I2C_ProtocolErr(void);
void I2C_Pattern(BYTE PatNo);

void I2C_Enable(BOOL bEN);
void I2C_Start_IO(void);
void I2C_Stop_IO(void);
void I2C_ReStart_IO(void);
void I2C_TX_IO(BYTE txdata);
BYTE I2C_RX_IO(void);
void I2C_Send_Ack(BOOL back);
BOOL I2C_Rec_Ack(void);
void I2C_Both_High(void);
void I2C_Both_Low(void);
#endif