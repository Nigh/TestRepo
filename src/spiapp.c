
// #pragma interrupt INTST1 intUartSend
// #pragma interrupt INTSR1 intUartRecev

#include "r_cg_macrodriver.h"
#include "r_cg_serial.h"
#include "spiapp.h"


uchar g_Statu=G_INACTIVE;


void echo(void)
{
	P2.3=0;
	sLed.ledMode++;
	if(sLed.ledMode>LED_M_RANDOM)
		sLed.ledMode=LED_M_OFF;
	ledSetMode(sLed.ledMode,0xFFFF);
	P2.3=1;
}

int spiRevBuf[48]={0};
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
	while(1){
		SIO00=0xFF;
		while(CSIIF00==0);CSIIF00=0;
		*pBuf++=SIO00;
		SIO00=0xFF;
		while(CSIIF00==0);CSIIF00=0;
		*pBuf++=SIO00;
		if(receiveCount++>=receiveMax3Local)
			break;
	}
	disable_3dh();
}

void read3DHCount(void)
{
	enable_3dh();
	SIO00=(0x2f|0x80);
	while(CSIIF00==0);CSIIF00=0;
	SIO00=0xff;
	while(CSIIF00==0);CSIIF00=0;
	receiveMax=(SIO00&0x1f);
	receiveMax3=receiveMax*3;
	disable_3dh();
}

tNECK Neck={HEAD_DOWN,HEAD_DOWN,{0,0,0,0,0,0},{0,0,0},0};
extern void set3DHEx(uchar addr,uchar value);
uchar dClick=0;
void _3DH5Hz(void)
{
	// static char oldG[3];
	static uchar staticCount=0,inactiveCount=0;
	uchar calcCount=0,_;
	uchar* pBuf=spiRevBuf;
	char temp[3];
	sGACC* sGAcc;
	tEULER* tEu;
	tNECK* tNeck=&Neck;

	read3DHCount();
	if(receiveMax==0)
		return;
	read3DH();

	while(calcCount++<receiveMax){
		temp[0]=pBuf[1];
		temp[1]=pBuf[3];
		temp[2]=pBuf[5];
		if(dClick) dClick++;
		if(IsClick(temp)==1){
			if(dClick==0)
				dClick=1;
			else if(dClick>55)
				dClick=1;
			else{
				if(dClick>6){
					echo();
				}
				dClick=0;
			}
			staticCount=0;
		}
		_=CalculateStep(temp);
		if(_) {staticCount=0;g_Statu=G_ACTIVE;}
		steps+=_;
		pBuf+=6;
	}

	if(g_Statu==G_INACTIVE){
		sGAcc=spiRevBuf;
		tEu=calcRulerA(sGAcc);
		tNeck->PositionID=HEAD_DOWN;
		tNeck->StartTime=time2();
		NeckActivityAlgorithm(tEu,tNeck);
	}

	if(++staticCount>300){
		g_Statu=G_SLEEP;
		set3DHEx(0x20,0x1f);
		R_TAU0_Channel5_Stop();
		staticCount=0;
	}else if(staticCount>50){
		g_Statu=G_INACTIVE;
		if(directGEn)
			staticCount=0;
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
