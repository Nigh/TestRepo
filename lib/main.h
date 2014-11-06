#ifndef _MAIN_H_
#define _MAIN_H_
#include "gvar.h"
#include "typedef.h"
#include "event2.0.h"
// #include "rtcapp.h"
// #include "bleapp.h"
// #include "timerapp.h"
// #include "keyapp.h"

extern sMSG sMsg;
extern void iMain(void);
extern void fRtcPro(void);
extern void fKeyPro(void);
extern void fTimerPro(void);
extern void fBLEPro(void);
extern void fReset(void);
#endif
