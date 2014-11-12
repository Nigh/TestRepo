
// #pragma interrupt INTST1 intUartSend
// #pragma interrupt INTSR1 intUartRecev

#include "r_cg_macrodriver.h"
#include "r_cg_serial.h"
#include "spiapp.h"

// __interrupt static void intUartSend(void) {}

// __interrupt static void intUartRecev(void) {}

void echo(void)
{
	sLed.ledMode++;
	if(sLed.ledMode>LED_M_RANDOM)
		sLed.ledMode=LED_M_OFF;
	ledSetMode(sLed.ledMode);
}


char spiRevBuf[48]={0};
uchar spiSendBuf[16]={0};
uchar nullvar=0;
uchar receiveMax=0,receiveMax3=0;
void read3DH(void)
{
	register uchar receiveCount=1;
	register uchar *pBuf=spiRevBuf;
	register uchar receiveMax3Local=receiveMax3;
	enable_3dh();
	SIO00=(0x28|0xc0);
	while(CSIIF00==0);CSIIF00=0;
	// while(SSR00&0x0040);
	// while(SSR00&0x0040!=0);
	while(1){
		SIO00=0xFF;
		while(CSIIF00==0);CSIIF00=0;
		// while(SSR00&0x0040);
		// while(SSR00&0x0040!=0);
		SIO00=0xFF;
		while(CSIIF00==0);CSIIF00=0;
		// while(SSR00&0x0040);
		// while(SSR00&0x0040!=0);
		*pBuf++=SIO00;
		if(receiveCount++>=receiveMax3Local)
			break;
	}
	disable_3dh();
}

void _3DH5Hz(void)
{
	uchar calcCount=0;
	uchar *pBuf=spiRevBuf;
	char temp[3];

	enable_3dh();
	SIO00=(0x2f|0x80);
	while(CSIIF00==0);CSIIF00=0;
	// while(SSR00&0x0040);
	// while(SSR00&0x0040!=0);
	SIO00=0xff;
	while(CSIIF00==0);CSIIF00=0;
	// while(SSR00&0x0040);
	// while(SSR00&0x0040!=0);
	receiveMax=(SIO00&0x1f);
	receiveMax3=receiveMax*3;
	disable_3dh();

	if(receiveMax==0)
		return;

	read3DH();

	// read3DH();

	while(calcCount++<receiveMax){
		temp[0]=pBuf[0];
		temp[1]=pBuf[1];
		temp[2]=pBuf[2];
		if(IsClick(temp)==1){
			echo();
		}
		pBuf+=3;
	}
}

void spiStart(void)
{
	// R_CSI00_Start();
	SO0 |= _0100_SAU_CH0_CLOCK_OUTPUT_1;    /* CSI00 clock initial level */
	SO0 &= ~_0001_SAU_CH0_DATA_OUTPUT_1;           /* CSI00 SO initial level */
	SOE0 |= _0001_SAU_CH0_OUTPUT_ENABLE;           /* enable CSI00 output */
	SS0 |= _0001_SAU_CH0_START_TRG_ON;             /* enable CSI00 */
	CSIIF00 = 0U;    /* clear INTCSI00 interrupt flag */

}

void spiStop(void)
{
	// R_CSI00_Stop();
	ST0 |= _0001_SAU_CH0_STOP_TRG_ON;        /* disable CSI00 */
	SOE0 &= ~_0001_SAU_CH0_OUTPUT_ENABLE;    /* disable CSI00 output */
	CSIIF00 = 0U;    /* clear INTCSI00 interrupt flag */
}
