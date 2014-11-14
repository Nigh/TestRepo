#ifndef _APPTIMER_H_
#define _APPTIMER_H_

#include "r_cg_userdefine.h"

typedef struct
{
	unsigned char en;
	unsigned int count;
	fFUNC func;
} sAPPTIMER;

// extern uchar timer(sAPPTIMER* apptimer);

// extern sAPPTIMER stopBuzzerTimer;
// extern sAPPTIMER stopVibrateTimer;
// extern sAPPTIMER BLEResetTimer;

#endif
