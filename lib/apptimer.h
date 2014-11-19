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

extern const sVIBRATE sV1;

#endif
