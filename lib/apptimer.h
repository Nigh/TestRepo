#ifndef _APPTIMER_H_
#define _APPTIMER_H_

#include "r_cg_userdefine.h"
#include "typedef.h"

typedef struct
{
	unsigned char en;
	unsigned int count;
	fFUNC func;
} sAPPTIMER;

typedef struct
{
	uint maxIndex;
	iFUNC* tArray[16];
} sTIMERTASK;

extern sVIBRATE sVibrate;
extern sTIMERTASK sTimerTask;

extern uchar timer(sAPPTIMER* apptimer);

// extern sAPPTIMER stopBuzzerTimer;
// extern sAPPTIMER stopVibrateTimer;
// extern sAPPTIMER BLEResetTimer;
void stopVibrate(void);
void setVibrate(sVIBRATE* sV);
extern int uartTimeOutTask(void);

extern void setTimer64Hz(sAPPTIMER* apptimer,uint period);
extern void taskInsert(iFUNC* task);
extern void taskDelete(uint ptr);

extern const sVIBRATE sV1;
extern const sVIBRATE sV2;
extern const sVIBRATE sV3;
extern const sVIBRATE sV4;
extern const sVIBRATE sV5;
extern const sVIBRATE sV6;

#endif
