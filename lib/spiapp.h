#ifndef _SPIAPP_H_
#define _SPIAPP_H_

#include "r_cg_userdefine.h"

#define G_SLEEP (1)
#define G_INACTIVE (2)
#define G_ACTIVE (3)

extern int spiRevBuf[];
extern uchar spiSendBuf[];
extern uchar g_Statu;
extern uint steps;

#define enable_3dh() P2.2=0;spiStart()
#define disable_3dh() P2.2=1;spiStop()

void read3DH(void);
void spiStart(void);
void spiStop(void);
void _3DH5Hz(void);
void neckLogCache(void);

#endif
