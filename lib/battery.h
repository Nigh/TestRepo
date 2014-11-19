#ifndef _BATTERY_H_
#define _BATTERY_H_
#include "r_cg_userdefine.h"
extern uint batteryLevel;

void startAD(void);
void stopAD(void);

#define AD_POWERON()
#define AD_POWEROFF()
#define AD_START()
#define AD_STOP()

#endif
