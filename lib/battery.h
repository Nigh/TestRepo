#ifndef _BATTERY_H_
#define _BATTERY_H_
#include "r_cg_userdefine.h"

extern int batteryLevel;
extern uint adcValue[4];
extern uchar powerLevel;

void startAD(void);
void stopAD(void);
void setADTimer(uint time);


#define AD_POWERON()
#define AD_POWEROFF()
#define AD_START()
#define AD_STOP()

#endif
