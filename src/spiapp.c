
// #pragma interrupt INTST1 intUartSend
// #pragma interrupt INTSR1 intUartRecev

#include "r_cg_macrodriver.h"
#include "r_cg_serial.h"
#include "spiapp.h"

extern float absf(float);
uchar g_Statu=G_INACTIVE;


void echo(void)
{
	// P2.3=0;
	// sLed.ledMode++;
	// if(sLed.ledMode>LED_M_RANDOM)
		// sLed.ledMode=LED_M_OFF;
	ledSetMode(LED_M_POWER,3);
	// P2.3=1;
}

// int spiRevBuf[48]={0};	// 5/16 used
int spiRevBuf[96]={0};		// 32 for safe
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
sNECKLOGLONG currentNeckLog={0,0,0,0,0,1};
uint currentNeckLogSec=0;
extern void set3DHEx(uchar addr,uchar value);
uchar dClick=0;
void _3DH5Hz(void)
{
	// static char oldG[3];
	static int staticCount=0,inactiveCount=0;
	int iTemp;
	uchar calcCount=0,_;
	uchar* pBuf=spiRevBuf;
	char temp[3];
	sGACC sGAcc;
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
				if(dClick>3){
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
		sGAcc.x=spiRevBuf[0]/4+spiRevBuf[3]/4+spiRevBuf[6]/4+spiRevBuf[9]/4;
		sGAcc.y=spiRevBuf[1]/4+spiRevBuf[4]/4+spiRevBuf[7]/4+spiRevBuf[10]/4;
		sGAcc.z=spiRevBuf[2]/4+spiRevBuf[5]/4+spiRevBuf[8]/4+spiRevBuf[11]/4;
		// sGAcc=spiRevBuf;
		tEu=calcRulerA(&sGAcc);
		if(absf(tEu->Pitch)>absf(tEu->Roll)){
			if(tEu->Pitch>5 and tEu->Pitch<50)
				tNeck->PositionID=HEAD_UP;
			else if(tEu->Pitch<-5 and tEu->Pitch>-50)
				tNeck->PositionID=HEAD_DOWN;
			else
				tNeck->PositionID=0x0;
		}else{
			if(tEu->Roll>5 and tEu->Roll<50)
				tNeck->PositionID=HEAD_LEFT;
			else if(tEu->Roll<-5 and tEu->Roll>-50)
				tNeck->PositionID=HEAD_RIGHT;
			else
				tNeck->PositionID=0x0;
		}
		if(tNeck->PositionID){
			tNeck->StartTime=time2();
			iTemp=NeckActivityAlgorithm(tEu,tNeck);
			if(iTemp){
				currentNeckLog.neckMove+=iTemp;
			}else{
				switch(tNeck->PositionID){
				case HEAD_UP:
					currentNeckLog.upTime++;
					break;
				case HEAD_DOWN:
					currentNeckLog.downTime++;
					break;
				case HEAD_LEFT:
					currentNeckLog.leftTime++;
					break;
				case HEAD_RIGHT:
					currentNeckLog.rightTime++;
					break;
				}
			}
		}
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


void memsetUser(uchar* ptr,const uchar ch,const size_t length)
{
	size_t i=0;
	while(i++<length)
		*ptr++=ch;
}

// 缓存颈动量时清理longLog
// 触发颈动量时配置longLog.UTC
static sNECKLOG neckLog[16];
void neckLogCache(void)
{
	uchar i=0;
	while(neckLog[i++].UTC!=0);
	i-=1;
	neckLog[i].UTC=currentNeckLog.UTC;
	neckLog[i].neckMove=currentNeckLog.neckMove;

	neckLog[i].leftTime=currentNeckLog.leftTime/10;
	neckLog[i].rightTime=currentNeckLog.rightTime/10;
	neckLog[i].upTime=currentNeckLog.upTime/10;
	neckLog[i].downTime=currentNeckLog.downTime/10;

	memsetUser(&currentNeckLog,0,sizeof(sNECKLOGLONG));

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
