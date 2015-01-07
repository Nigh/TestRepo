
// #pragma interrupt INTST1 intUartSend
// #pragma interrupt INTSR1 intUartRecev

#include "r_cg_macrodriver.h"
#include "r_cg_serial.h"
#include "r_cg_userdefine.h"
#include "spiapp.h"
#include "flashapp.h"
#include "flashfunc.h"

extern float absf(float);
uchar g_Statu=G_INACTIVE;

extern time_t time2(void);
extern void memcpyUser(uchar* src,uchar* dst,const size_t length);

extern const uchar data_accessConfirm[5];
void echo(void)
{
	if(BLE_Connect_Timeout>0){
		BLE_Connect_Timeout=0;
		ledSetMode(LED_M_OFF,1);
		setVibrate(&sV5);
		sVibrate.count=1;
		uartBufWrite(data_accessConfirm,5);
		calcSendBufSum();
		uartSend(5);
		return;
	}
	if(batteryStatu==BAT_CHARGE){
		ledSetMode(LED_M_MQ,1);
	}else{
		ledSetMode(LED_M_STATICPOWER,1);
	}
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

uint accDiff(int a,int b)
{
	if(a>b){
		return (a-b)>>6;
	}else{
		return (b-a)>>6;
	}
}


tNECK Neck={HEAD_DOWN,HEAD_DOWN,{0,0,0,0,0,0},{0,0,0},0};
sNECKLOGLONG currentNeckLog={0,0,0,0,0,1};
sNECKMOVESTATU sNeckMoveStatu={0};
uint currentStepLogSec=0;
extern sAPPTIMER stopVibrateTimer;
extern void set3DHEx(uchar addr,uchar value);
uchar dClick=0;
sGACC sGAcc;
extern const uchar data_axisDirect[3];
static const sMSG sAccUpload={M_TYPE_TRANS,M_C_ACCUPLOAD};
extern uint stepTarget;
extern unsigned long dayStep;
void _3DH5Hz(void)
{
	// static char oldG[3];
	static uint SMA=0,SMAOld=0,SMACount=0;	//1sec SMA (使用 1G=256)
	static sGACC oldAcc;
	static int staticCount=0,inactiveCount=0;
	static uchar stepHalf=0;
	int iTemp;
	uchar calcCount=0;
	char _;
	uchar* pBuf=spiRevBuf;
	char temp[3];
	tEULER* tEu;
	tNECK* tNeck=&Neck;

	read3DHCount();

	if(receiveMax!=0){
		read3DH();

		while(calcCount++<receiveMax){
			temp[0]=pBuf[1];
			temp[1]=pBuf[3];
			temp[2]=pBuf[5];
			if(dClick) dClick++;
			if(IsClick(temp)==1){
				if(dClick==0)
					dClick=1;
				else if(dClick>24)
					dClick=1;
				else{
					if(dClick>3){
						echo();
					}
					dClick=0;
				}
				staticCount=0;
			}
			if(stepHalf=!stepHalf && isTimeSync){

				#ifdef USE_TRADITIONAL_ALGORITHM
				if(g_Statu==G_INACTIVE){
					_=_calcStep(temp,1);neckUnhealthCount=0;
				}
				else
					_=_calcStep(temp,0);
				#else
				_=CalculateStep(temp);
				#endif

				if(_>0) {
					steps+=_;staticCount=0;g_Statu=G_ACTIVE;
					dayStep+=_;
					if(dayStep>stepTarget){
						dayStep=0;
						ledSetMode(LED_M_RANDOM,2);
						setVibrate(&sV1);
						sVibrate.count=2;
					}
				}
				// else if(_<0) {g_Statu=G_INACTIVE;}
			}

			pBuf+=6;
		}

		if(g_Statu==G_INACTIVE){
			sGAcc.x=spiRevBuf[0]/4+spiRevBuf[3]/4+spiRevBuf[6]/4+spiRevBuf[9]/4;
			sGAcc.y=spiRevBuf[1]/4+spiRevBuf[4]/4+spiRevBuf[7]/4+spiRevBuf[10]/4;
			sGAcc.z=spiRevBuf[2]/4+spiRevBuf[5]/4+spiRevBuf[8]/4+spiRevBuf[11]/4;
			// sGAcc=spiRevBuf;
			if(SMACount==0){
				SMAOld=SMA;
				if(SMAOld>G_SLEEPLIMIT)
					staticCount=0;
				SMA=0;
				SMACount=1;
			}else{
				SMA+=accDiff(oldAcc.x,sGAcc.x)+accDiff(oldAcc.y,sGAcc.y)+accDiff(oldAcc.z,sGAcc.z);
				SMACount++;
				if(SMACount>=10)
					SMACount=0;
			}

			memcpyUser(&sGAcc,&oldAcc,sizeof(sGACC));

			if(SMAOld<NECKMOVE_UPLIMIT && isTimeSync){
				tEu=calcRulerA(&sGAcc);
				if(absf(tEu->Pitch)>absf(tEu->Roll)){
					if(tEu->Pitch>NECK_PITCH_POSITIVE_DOWNLIMIT and tEu->Pitch<NECK_PITCH_POSITIVE_UPLIMIT)
						tNeck->PositionID=HEAD_UP;
					else if(tEu->Pitch<NECK_PITCH_NEGATIVE_UPLIMIT and tEu->Pitch>NECK_PITCH_NEGATIVE_DOWNLIMIT)
						tNeck->PositionID=HEAD_DOWN;
					else
						tNeck->PositionID=0x0;
				}else{
					if(tEu->Roll>NECK_ROLL_POSITIVE_DOWNLIMIT and tEu->Roll<NECK_ROLL_POSITIVE_UPLIMIT)
						tNeck->PositionID=HEAD_RIGHT;
					else if(tEu->Roll<NECK_ROLL_NEGATIVE_UPLIMIT and tEu->Roll>NECK_ROLL_NEGATIVE_DOWNLIMIT)
						tNeck->PositionID=HEAD_LEFT;
					else
						tNeck->PositionID=0x0;
				}
				if(tNeck->PositionID){
					tNeck->StartTime=time2();
					iTemp=NeckActivityAlgorithm(tEu,tNeck);
					if(stopVibrateTimer.en)
						iTemp=0;
					if(!sNeckMoveStatu.statu and iTemp){
						sNeckMoveStatu.statu=1;
						currentNeckLog.UTC=sUtcs.lTime;
					}
					if(sNeckMoveStatu.statu){
						if(iTemp){
							currentNeckLog.neckMove+=NeckData.Movement.Pitch>NeckData.Movement.Roll
							?NeckData.Movement.Pitch
							:NeckData.Movement.Roll;
						}else{
							switch(tNeck->PositionID){
							case HEAD_UP: currentNeckLog.upTime++; break;
							case HEAD_DOWN: currentNeckLog.downTime++; break;
							case HEAD_LEFT: currentNeckLog.leftTime++; break;
							case HEAD_RIGHT: currentNeckLog.rightTime++; break;
							}
						}
					}
				}
			}

		}
	}else{
		set3DHEx(0x20,0x47);
		set3DHEx(0x20,0x47);
	}

	if(directGEn)
	{
		DI();
		fifoPut4ISR(sAccUpload);
		EI();
		// uartBufWrite(data_axisDirect,3);
		// memcpy(sGAcc,&uartSendBuf[3],6);
		// calcSendBufSum();
		// uartSend(10);
	}

	if(++staticCount>G_ACTIVE_TIMEOUT){
		g_Statu=G_SLEEP;
		set3DHEx(0x20,0x1f);
		set3DHEx(0x20,0x1f);
		R_TAU0_Channel5_Stop();
		staticCount=0;
	}else if(staticCount>STEP_ACTIVE_TIMEOUT){
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

const sFLASHOP opFlashWait={FLASH_F_IDLEWAIT,0};
const sFLASHOP opFlashAddrErase={FLASH_F_BLOCKERASE,FLASH_S_ADDR};
const sFLASHOP opFlashAddrSave={FLASH_F_WRITE,FLASH_S_ADDR};
void addrCache(void)
{
	flashOpPut(opFlashWait);
	flashOpPut(opFlashAddrErase);
	flashOpPut(opFlashWait);
	flashOpPut(opFlashAddrSave);
	flashOpFin();
}

void addrClear(void)
{
	flashOpPut(opFlashWait);
	flashOpPut(opFlashAddrErase);
	flashOpPut(opFlashWait);
	flashOpFin();
}

// 缓存颈动量时清理longLog
// 触发颈动量时配置longLog.UTC
// 写入flash后，清Log.UTC
static const sFLASHOP opFlashNeckErase={FLASH_F_BLOCKERASE,FLASH_S_NECK};
static const sFLASHOP opFlashNeckSave={FLASH_F_WRITE,FLASH_S_NECK};
sNECKLOG neckLog[16]={0};
void neckLogCache(void)
{
	uchar i=0;
	while(neckLog[i++].UTC!=0 && i<15);
	i-=1;
	neckLog[i].UTC=currentNeckLog.UTC;
	neckLog[i].neckMove=currentNeckLog.neckMove;
	neckLog[i].leftTime=currentNeckLog.leftTime/10;
	neckLog[i].rightTime=currentNeckLog.rightTime/10;
	neckLog[i].upTime=currentNeckLog.upTime/10;
	neckLog[i].downTime=currentNeckLog.downTime/10;

	memsetUser(&currentNeckLog,0,sizeof(sNECKLOGLONG));
	// if(i>=1)	// debug
	if(i>=NECK_SAVE_PACKAGE-1)
	{
		flashOpPut(opFlashWait);
		if(needErase((i+1)*sizeof(sNECKLOG),neckFlash.endAddr)){
			flashOpPut(opFlashNeckErase);
			flashOpPut(opFlashWait);
		}
		flashOpPut(opFlashNeckSave);
		flashOpFin();
	}
}


static uchar isStepLogEmpty(void);
static const sFLASHOP opFlashStepErase={FLASH_F_BLOCKERASE,FLASH_S_STEP};
static const sFLASHOP opFlashStepSave={FLASH_F_WRITE,FLASH_S_STEP};
extern sSTEPLONGLOG currentStepLog;
extern unsigned long dayStep;
sSTEPLOG stepLog;
void stepLogCache(void)
{
	currentStepLog.steps[currentStepLog.logCount++]=steps;
	steps=0;
	if(currentStepLog.logCount>7)
	{
		memcpyUser(&currentStepLog,&stepLog,sizeof(sSTEPLOG));
		if(!isStepLogEmpty())
		{
			flashOpPut(opFlashWait);
			if(needErase(sizeof(sSTEPLOG),stepFlash.endAddr)){
				flashOpPut(opFlashStepErase);
				flashOpPut(opFlashWait);
			}
			flashOpPut(opFlashStepSave);
			flashOpFin();
		}
		memsetUser(&currentStepLog,0,sizeof(sSTEPLONGLOG));
		currentStepLog.UTC=sUtcs.lTime;
	}
}

uchar isStepLogEmpty(void)
{
	uchar i=0;
	while(i<=7){
		if(currentStepLog.steps[i]>0)
			return 0;
		i++;
	}
	return 1;
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

void dddebug(void)
{
	flashOpPut(opFlashWait);
	if(needErase(3*sizeof(sNECKLOG),neckFlash.endAddr)){
		flashOpPut(opFlashNeckErase);
		flashOpPut(opFlashWait);
	}
	flashOpPut(opFlashNeckSave);
	flashOpFin();
}
