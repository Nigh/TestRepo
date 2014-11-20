#ifndef _UARTAPP_H_
#define _UARTAPP_H_

#include "r_cg_userdefine.h"

#define UART_IDLE (0)
#define UART_SEND (1)
#define UART_REV (2)
#define UART_WAIT (3)

extern const uchar data_transSuccess[5];
extern const uchar data_transFail[5];

extern const uchar data_neckLog[3];
extern const uchar data_stepLog[3];
extern const uchar data_logCount[3];
extern const uchar data_axisDirect[3];
extern const uchar data_batteryLevel[3];
extern const uchar data_bleCtrl[3];

extern uchar uartSendBuf[32];
extern uchar uartRevBuf[32];

typedef struct
{
	uchar statu;
	uchar count;
	uint time;
} sUART;

extern sUART sUart;

extern void calcSendBufSum(void);
extern void uartBufWrite(uchar *wPtr,uchar len);
extern void uartSend(uchar len);

#endif
