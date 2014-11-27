
#ifndef _EVENT_H_
#define _EVENT_H_

#include "typedef.h"
// #include "r_cg_userdefine.h"

//event type
#define M_TYPE_RTC 1
#define M_TYPE_KEY 2
#define M_TYPE_TIMER 3
#define M_TYPE_BLE 4
#define M_TYPE_TRANS 5
#define M_TYPE_SYS 6
#define M_TYPE_SYSRESET 15

//clock event content
#define M_C_CLOCK1HZ 1
#define M_C_CLOCK2HZ 2
#define M_C_CLOCK5HZ 3
#define M_C_CLOCK64HZ 4
#define M_C_CLOCK32HZ 5
#define M_C_CLOCK1MIN 6
#define M_C_CLOCK1HOUR 7

//timer event content
#define M_C_TIMER1 0x1
#define M_C_TIMER2 0x2
#define M_C_TIMER3 0x3
#define M_C_TIMER4 0x4
#define M_C_TIMER5 0x5
#define M_C_TIMER6 0x6
#define M_C_TIMER7 0x7

//ble event content
#define M_C_COMFIRM 0x1
#define M_C_TIMESYNC 0x2
#define M_C_ALARMSET 0x3
#define M_C_MOTOR 0x4
#define M_C_LED 0x5
#define M_C_FLASHCLEAR 0x6
#define M_C_GSENSOR 0x7
#define M_C_DATAREQ 0x8
#define M_C_BLESTATU 0x9

//trans event content
#define M_C_UARTSENDEND 0x1
#define M_C_UARTREVREQ 0x2
#define M_C_FLASHSTART 0x3
#define M_C_FLASHFINISH 0x4
#define M_C_ACCUPLOAD 0x5


//system event content
#define M_C_ADCEND 1
#define M_C_CHARGEINT 2

#define BUF_SIZE 0x10
#define BUF_SIZE_MASK 0xF

typedef struct
{
	uchar type;
	uchar content;
} sMSG;

typedef struct
{
	uchar remain_size;
	uchar write_pos;
	uchar read_pos;
	uchar buf_size_mask;
	sMSG *p_buf;
} sFIFO;

void fifoInit(sFIFO *p_fifo, sMSG *p_buf);
sMSG fifoGet(void);
int fifoPut4ISR(sMSG);
void fifoFlush(void);

extern sMSG msgQueue[];
extern sFIFO sMsgFifo;

#endif /* EVENT_H_ */

