#ifndef _MAIN_H_
#define _MAIN_H_
#include "r_cg_userdefine.h"
#include "typedef.h"
#include "event2.0.h"


extern sMSG sMsg;
extern void iMain(void);
extern void fRtcPro(void);
extern void fKeyPro(void);
extern void fTimerPro(void);
extern void fBLEPro(void);
extern void fTransPro(void);

extern void fReset(void);
#endif
