#ifndef _SPIAPP_H_
#define _SPIAPP_H_

#include "r_cg_userdefine.h"

extern char spiRevBuf[];
extern uchar spiSendBuf[];

#define enable_3dh() P2.2=0;spiStart()
#define disable_3dh() P2.2=1;spiStop()

void read3DH(void);
void spiStart(void);
void spiStop(void);
void _3DH5Hz(void);

#endif
