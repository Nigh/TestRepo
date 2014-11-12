#ifndef _UARTAPP_H_
#define _UARTAPP_H_

#include "r_cg_userdefine.h"

#define UART_IDLE (0)
#define UART_SEND (1)
#define UART_REV (2)
#define UART_WAIT (3)

extern uchar uartSendBuf[32];
extern uchar uartRevBuf[32];

typedef struct
{
	uchar statu;
	uchar count;
	uint time;
} sUART;

extern sUART sUart;

extern void uartBufInit(uchar);
extern void uartBufWrite(uchar *wPtr,uchar len);
extern void uartSend(uchar len);

#endif
