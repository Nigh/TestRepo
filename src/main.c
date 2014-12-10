#include "flashbranch.h"
#include "main.h"
#include "r_cg_userdefine.h"
#include "NeckAlgorithm.h"

#define VECTOR(vec) vec

fFUNC const msgHandler[]={		// No
	VECTOR(_halt_Ex),				// 0
	VECTOR(fRtcPro),			// 1
	VECTOR(fKeyPro),			// 2
	VECTOR(fTimerPro),			// 3
	VECTOR(fBLEPro),			// 4
	VECTOR(fTransPro),				// 5
	VECTOR(fSysPro),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
	VECTOR(_nop_Ex),				// 9
	VECTOR(_nop_Ex),				// 10
	VECTOR(_nop_Ex),				// 11
	VECTOR(_nop_Ex),				// 12
	VECTOR(_nop_Ex),				// 13
	VECTOR(_nop_Ex),				// 14
	VECTOR(fReset),				// 15
};

sMSG gMsg={0,0};	//global message
uchar uartRevTimeout=0;

uint calcStepLogNum(void);
uint calcNeckLogNum(void);
void uartSendLogCount(void);

extern void dddebug(void);
extern void initFlash(void);
	//for debug
extern void flashErase(unsigned short dataLength, unsigned long flashAddr);
extern void flashWrite(unsigned char* ptr, unsigned short dataLength, unsigned long flashAddr);

void afterBoot(void)
{
	unsigned long x=0;
	// flashErase(1,0);	// for debug
	// NOP();	//wait 400ms by debugger
	while(x++<100000);
	// flashErase(1,131072);	// for debug
	// NOP();	//wait 400ms by debugger

	P2.0=1;
	R_INTC0_Start();
	R_KEY_Start();
	sUtcs.lTime=0;
	sAlarmTime.lTime=0xffffffff;
	currentStepLog.UTC=sUtcs.lTime;
	R_UART1_Start();
	fifoInit(&sMsgFifo,msgQueue);
	flashQueueInit(&sFlashQueue);
	setADTimer(10);
}

#include "bootmain.h"
extern int isFlashIdle(void); 
extern void R_PCLBUZ0_Start(void);
void set3DHEx(uchar addr,uchar value);
void init3DH(void);
extern sNECKLOG neckLog[16];
extern sSTEPLOG stepLog;
void iMain(void)
{
	afterBoot();

	// flashWrite(&neckLog, 20, 0);	//for debug
	// sUtcs.lTime=1417171128	//for debug
	// flashWrite(&stepLog, 20, 131072);	//for debug

	ledSetMode(LED_M_OFF,1);
	EI();
	HALT();
	NOP();
	HALT();
	fifoFlush();
	initFlash();
	init3DH();
	R_TAU0_Channel5_Start();
	// dddebug();

	ledSetMode(LED_M_MQ,3);
	// R_PCLBUZ0_Start();
	while(1){
		if(sUart.statu!=UART_IDLE)
			startHClk();
		DI();
		gMsg=fifoGet();
		EI();
		NOP();
		if(gMsg.type<=15)
			msgHandler[gMsg.type]();
	}
}

// ***************************************
// ***************************************
void fReset(void)
{
	
}

// ***************************************
// ***************************************
extern int spiRevBuf[48];
extern uchar receiveMax;
extern void read3DHCount(void);
extern sNECKMOVESTATU sNeckMoveStatu;
extern sNECKLOGLONG currentNeckLog;
extern sAPPTIMER adTimer;

extern sGACC sGAcc;
void neckHealthCheck(void)
{
	if(currentNeckLog.neckMove<HEALTHNECKMOVE)
		neckUnhealthCount++;
	else
		neckUnhealthCount=0;
	if(neckUnhealthCount>=5){
		if(neckUnhealthCount>50)
			neckUnhealthCount=50;
		if(g_Statu==G_INACTIVE and (sGAcc.x>30 or sGAcc.x<-30))
			setVibrate(&sV5);
		sVibrate.count=neckUnhealthCount/5;
	}
}


void fRtc2Hz(void)
{
	static uint count=0;
	static uchar* const pBuf=spiRevBuf;
	static uchar gOld[3]={0},flag=0;	//flag 用于标示是否已更新gOld
	static uint currentStepLogSec=0;
	count++;

	if(sSelf.mode!=SYS_ACTIVE)
		return;

	if(sUpload.statu!=UPLOAD_IDLE)
	{
		if(sUpload.timeOut++>1){
			sUpload.timeOut=0;
			sUpload.timeOutCount++;
			if(sUpload.timeOutCount<3)
				uartSendLogCount();
			else{
				sUpload.statu=UPLOAD_IDLE;
				sUpload.timeOutCount=0;
			}
		}
	}

	if(uartRevTimeout>0){
		uartRevTimeout++;
		if(uartRevTimeout>1){
			sUart.statu&=0xFF^UART_REV;
			uartRevTimeout=0;
		}
	}

	if((count&0x1)==0)
	{
		if(BLE_Connect_Timeout>0)
			BLE_Connect_Timeout--;
		if(batteryStatu!=BAT_NORMAL){
			if(batteryStatu==BAT_CHARGE && (sLed.ledMode==LED_M_OFF or sLed.ledMode==LED_M_POWER))
				ledSetMode(LED_M_POWER,2);
		}
		timer(&adTimer);

		if(sNeckMoveStatu.statu){
			sNeckMoveStatu.timeCount++;
			// if(sNeckMoveStatu.timeCount>=240){
			if(sNeckMoveStatu.timeCount>=60){	//debug
				sNeckMoveStatu.timeCount=0;
				sNeckMoveStatu.statu=0;
				neckHealthCheck();
				neckLogCache();
			}
		}

		currentStepLogSec++;
		// if(currentStepLogSec>=300){
		if(currentStepLogSec>=60){	//debug
			currentStepLogSec=0;
			stepLogCache();
		}

		sUtcs.lTime++;
		if(g_Statu==G_SLEEP){
			startHClk();
			// P2.3=0;
			read3DHCount();
			if(receiveMax>0){
				read3DH();
				if(flag){
					if(gOld[0]-pBuf[1]>10 and gOld[0]-pBuf[1]<240)
						g_Statu=G_INACTIVE;
					else if(gOld[1]-pBuf[3]>10 and gOld[1]-pBuf[3]<240)
						g_Statu=G_INACTIVE;
					else if(gOld[2]-pBuf[5]>10 and gOld[2]-pBuf[5]<240)
						g_Statu=G_INACTIVE;
				}else{
					gOld[0]=pBuf[1];
					gOld[1]=pBuf[3];
					gOld[2]=pBuf[5];
					flag=1;
				}

				if(g_Statu!=G_SLEEP){
					flag=0;
					R_TAU0_Channel5_Start();
					set3DHEx(0x20,0x47);
				}
			}
		}
	}
}

void fRtc5Hz(void)
{
	startHClk();
	_3DH5Hz();
}


extern sTIMERTASK sTimerTask;
extern void taskDelete(uint ptr);
void fRtc64Hz(void)
{
	uint i=0;
	if(sTimerTask.maxIndex>0)
		while(i<sTimerTask.maxIndex){if(sTimerTask.tArray[i]<0xff or sTimerTask.tArray[i]()==0) taskDelete(i); else i++; }
	else
		R_IT_Stop();
}

fFUNC const rtcHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(_nop_Ex),				// 1
	VECTOR(fRtc2Hz),				// 2
	VECTOR(fRtc5Hz),				// 3
	VECTOR(fRtc64Hz),				// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
};

void fRtcPro(void)
{
	if(gMsg.content<=8)
		rtcHandler[gMsg.content]();
}


// ***************************************
// ***************************************
fFUNC const keyHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(_nop_Ex),				// 1
	VECTOR(_nop_Ex),				// 2
	VECTOR(_nop_Ex),				// 3
	VECTOR(_nop_Ex),				// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
};

void fKeyPro(void)
{
	if(gMsg.content<=8)
		keyHandler[gMsg.content]();
}


// ***************************************
// ***************************************
fFUNC const timerHandler[]={	// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(_nop_Ex),				// 1
	VECTOR(_nop_Ex),				// 2
	VECTOR(_nop_Ex),				// 3
	VECTOR(_nop_Ex),				// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
};

void fTimerPro(void)
{
	if(gMsg.content<=8)
		timerHandler[gMsg.content]();
}


// ***************************************
// ***************************************
static const sFLASHOP opFlashNeckRead={FLASH_F_READ,FLASH_S_NECK};
static const sFLASHOP opFlashStepRead={FLASH_F_READ,FLASH_S_STEP};
extern const sFLASHOP opFlashWait;
void dataReadSend(void)
{
	flashOpPut(opFlashWait);
	if(sUpload.statu==UPLOAD_NECK)
		flashOpPut(opFlashNeckRead);
	if(sUpload.statu==UPLOAD_STEP)
		flashOpPut(opFlashStepRead);
	flashOpFin();
}

#include "flashfunc.h"
void flashReadSeek(void)
{
	if(sUpload.statu==UPLOAD_NECK){
		neckFlash.startAddr+=2*sizeof(sNECKLOG);
		if(neckFlash.startAddr>NECKRANGEND){
			neckFlash.startAddr-=NECKRANGEND;
		}
	} else if(sUpload.statu==UPLOAD_STEP){
		stepFlash.startAddr+=sizeof(sSTEPLOG);
		if(stepFlash.startAddr>STEPRANGEND){
			stepFlash.startAddr-=STEPRANGEND;
		}
	}
}

extern const uchar data_logCount[3];
void uartSendLogCount(void)
{
	union{uchar temp[4];uint iTemp[2];}uTemp;

	uTemp.iTemp[0]=calcNeckLogNum();
	uTemp.iTemp[1]=calcStepLogNum();
	uartBufWrite(data_logCount,3);
	uartSendBuf[3]=uTemp.temp[0];
	uartSendBuf[4]=uTemp.temp[1];
	uartSendBuf[5]=uTemp.temp[2];
	uartSendBuf[6]=uTemp.temp[3];
	calcSendBufSum();
	uartSend(4+4);
}

extern int uartTimeOutTask(void);
extern uchar uartTimeOutTaskStatu;
void fBLEConfirm(void)
{
	if((uartRevBuf[3]==0x09 or uartRevBuf[3]==0x0A or uartRevBuf[3]==0x0B) 
		and sUpload.statu!=UPLOAD_IDLE){
		if(sUpload.packageRemain>=0 and uartRevBuf[3]!=0x0B)
			flashReadSeek();
		if(sUpload.packageRemain<=0){
			sUpload.statu=UPLOAD_IDLE;
			uartSendLogCount();
			return;
		}
		dataReadSend();
		if(uartRevBuf[3]!=0x0B)
			sUpload.packageRemain--;
		sUpload.timeOut=0;
		if(!uartTimeOutTaskStatu){
			uartTimeOutTaskStatu=1;
			taskInsert(&uartTimeOutTask);
		}
	}
}


extern const uchar data_transSuccess[5];
void uartSuccess(uchar num)
{
	uartBufWrite(data_transSuccess,3);
	uartSendBuf[3]=num;
	calcSendBufSum();
	uartSendDirect(5);
}

void fTimeSync(void)
{
	uchar* ptr1=&uartRevBuf[3];
	uchar* ptr2=&sUtcs;
	*ptr2++=*ptr1++;
	*ptr2++=*ptr1++;
	*ptr2++=*ptr1++;
	*ptr2=*ptr1;

	currentStepLog.UTC=sUtcs.lTime;
	currentNeckLog.UTC=sUtcs.lTime;

	uartSuccess(0x03);
}

void fSetAlarm(void)
{
	uchar* ptr1=&uartRevBuf[3];
	uchar* ptr2=&sAlarmTime;
	*ptr2++=*ptr1++;
	*ptr2++=*ptr1++;
	*ptr2++=*ptr1++;
	*ptr2=*ptr1;

	uartSuccess(0x04);
}

void fMotorCtl(void)
{
	uint ledMode;
	switch(uartRevBuf[3]&0xf)
	{
		case 1: setVibrate(&sV1);break;
		case 2: setVibrate(&sV2);break;
		case 3: setVibrate(&sV3);break;
		case 4: setVibrate(&sV4);break;
		case 5: setVibrate(&sV5);break;
		case 6: setVibrate(&sV6);break;
		default: break;
	}
	sVibrate.count=uartRevBuf[3]>>4;
	uartSuccess(0x05);
}

void fLEDCtl(void)
{
	uint ledMode;
	switch(uartRevBuf[3]&0xf)
	{
		case 1: ledMode=LED_M_FLASHALL;break;
		case 2: ledMode=LED_M_MQ;break;
		case 3: ledMode=LED_M_BREATHE;break;
		case 4: ledMode=LED_M_SWING;break;
		case 5: ledMode=LED_M_HERATBEAT;break;
		case 6: powerLevel=1;ledMode=LED_M_POWER;break;
		case 7: powerLevel=2;ledMode=LED_M_POWER;break;
		case 8: powerLevel=3;ledMode=LED_M_POWER;break;
		case 9: powerLevel=4;ledMode=LED_M_POWER;break;
		case 10: ledMode=LED_M_RANDOM;break;
		default: ledMode=LED_M_OFF;break;
	}
	ledSetMode(ledMode,uartRevBuf[3]>>4);

	uartSuccess(0x06);
}


extern void memsetUser(uchar* ptr,const uchar ch,const size_t length);
void goSleep(void)
{
	startHClk();
	set3DHEx(0x20,0x07);	// power down
	if(sSelf.mode==SYS_ACTIVE)
	{
		memsetUser(&currentNeckLog,0,sizeof(sNECKLOGLONG));
		memsetUser(&currentStepLog,0,sizeof(sSTEPLONGLOG));
		g_Statu=G_SLEEP;
		R_TAU0_Channel5_Stop();
		// 此处应关闭蓝牙
	}
	sSelf.mode=SYS_SLEEP;
}

void goActive(void)
{
	if(sSelf.mode==SYS_SLEEP)
	{
		set3DHEx(0x20,0x47);
		R_TAU0_Channel5_Start();
		g_Statu=G_INACTIVE;
		// 此处应打开蓝牙
	}
	sSelf.mode=SYS_ACTIVE;
}

void fFormatFlash(void)
{
	if(uartRevBuf[3]==0xd9){
		goSleep();
	}
	uartSuccess(0x07);
}

void fGsensorAcc(void)
{
	if(uartRevBuf[3]){
		directGEn=1;
		if(g_Statu==G_SLEEP){
			R_TAU0_Channel5_Start();
			set3DHEx(0x20,0x47);
		}
	}else{
		directGEn=0;
	}

	uartSuccess(0x08);
}


uint calcStepLogNum(void)
{
	unsigned long temp;
	if(stepFlash.startAddr>stepFlash.endAddr){
		temp=(STEPRANGEND-STEPRANGSTART)-(stepFlash.startAddr-stepFlash.endAddr);
		return temp/sizeof(sSTEPLOG);
	}else if(stepFlash.startAddr<stepFlash.endAddr){
		temp=stepFlash.endAddr-stepFlash.startAddr;
		return temp/sizeof(sSTEPLOG);
	}else
		return 0;
}

uint calcNeckLogNum(void)
{
	unsigned long temp;
	if(neckFlash.startAddr>neckFlash.endAddr){
		temp=(NECKRANGEND-NECKRANGSTART)-(neckFlash.startAddr-neckFlash.endAddr);
		return temp/2/sizeof(sNECKLOG);
	}else if(neckFlash.startAddr<neckFlash.endAddr){
		temp=neckFlash.endAddr-neckFlash.startAddr;
		return temp/2/sizeof(sNECKLOG);
	}else
		return 0;
}


void fDataReqest(void)
{
	if(sUpload.statu!=UPLOAD_IDLE)
		return;
	sUpload.timeOut=0;
	sUpload.timeOutCount=0;		
	if(!uartTimeOutTaskStatu){
		uartTimeOutTaskStatu=1;
		taskInsert(&uartTimeOutTask);
	}
	if(uartRevBuf[3]==0x1){	//neck_log
		sUpload.statu=UPLOAD_NECK;
		sUpload.packageRemain=calcNeckLogNum();
	}else if(uartRevBuf[3]==0x2){	//step_log
		sUpload.statu=UPLOAD_STEP;
		sUpload.packageRemain=calcStepLogNum();
	}
	// uartSuccess(0x0d);
	uartSendLogCount();
	// dataReadSend();
}

void fDEBUG(void)
{
	TDR06=uartRevBuf[3]*4000;
	uartBufWrite(data_transSuccess,5);
	uartSendDirect(5);
}

void fConnectRequest(void)
{
	if(uartRevBuf[3]==0x01){
		BLE_Connect_Timeout=BLE_CONNECT_TIMEOUT_SET;
		// ledSetMode(LED_M_MQ,10);
	}
	uartSuccess(0x10);
}

fFUNC const bleHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(fBLEConfirm),				// 1
	VECTOR(fTimeSync),				// 2
	VECTOR(fSetAlarm),				// 3
	VECTOR(fMotorCtl),				// 4
	VECTOR(fLEDCtl),				// 5
	VECTOR(fFormatFlash),				// 6
	VECTOR(fGsensorAcc),				// 7
	VECTOR(fDataReqest),				// 8
	VECTOR(fDEBUG),				// 9
	VECTOR(fConnectRequest),				// 10
};

void fBLEPro(void)
{
	if(gMsg.content<=10)
		bleHandler[gMsg.content]();
}

// ***************************************
// ***************************************
void fUartSendEnd(void)
{
	NOP();
	// sUart.statu&=0xFF^UART_SEND;
}

void fUartRevReq(void)
{
	startHClk();
	uartRevTimeout=1;
}


void fFlashOpStart(void)
{
	static const sMSG sMsg={M_TYPE_TRANS,M_C_FLASHFINISH};
	// if(isFlashAwake())
	// 	return;
	// flashAwake();
	if(isFlashIdle()){
		DI();
		fifoPut4ISR(sMsg);
		EI();
	}
}

extern sFLASHOP gOP;
void fFlashOpFinish(void)
{
	gOP=flashOpGet();
	if(gOP.opType==0){
		// flashSleep();
	}else{
		if(gOP.opType<=5)
			fFlashOp[gOP.opType]();
	}
}



void memcpyUser(uchar* src,uchar* dst,const size_t length)
{
	size_t i=0;
	while(i++<length)
		*dst++=*src++;
}

extern const uchar data_axisDirect[3];
void fAccUpload(void)
{
	sGACC tempAcc;
	uartBufWrite(data_axisDirect,3);
	if(sGAcc.x>0)
		memcpyUser(&sGAcc,&uartSendBuf[3],6);
	else
	{
		tempAcc.x=-sGAcc.x;
		tempAcc.y=-sGAcc.y;
		tempAcc.z=sGAcc.z;
		memcpyUser(&tempAcc,&uartSendBuf[3],6);
	}
	calcSendBufSum();
	uartSend(10);
}

fFUNC const transHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(fUartSendEnd),				// 1
	VECTOR(fUartRevReq),				// 2
	VECTOR(fFlashOpStart),				// 3
	VECTOR(fFlashOpFinish),				// 4
	VECTOR(fAccUpload),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
};

void fTransPro(void)
{
	if(gMsg.content<=8)
		transHandler[gMsg.content]();
}
// ***************************************
// ***************************************

extern const uchar data_batteryLevel[3];
void fAdcEnd(void)
{
	static uint batteryLevelOld=100;
	uint temp;
	if(sVibrate.en)
	{
		setADTimer(10);
		return;
	}
	batteryLevel=((adcValue[0]>>6)+(adcValue[1]>>6)+(adcValue[2]>>6)+(adcValue[3]>>6))>>2;
	batteryLevel=batteryLevel-746;	//746~871  batteryLevel:0~125
	// 42~125 -> 3.6v~4.2v
	if(batteryLevel<0)
		batteryLevel=0;
	if(batteryLevelOld>58 && batteryLevel<=58){
		setVibrate(&sV5);
		sVibrate.count=1;
	}
	batteryLevelOld=batteryLevel;
	// batteryLevel=batteryLevel-810;	//810~919  batteryLevel:0~109
	// batteryLevel=batteryLevel-743;	//743~868  batteryLevel:0~125

	if(batteryLevel>42)
		powerLevel=(float)(batteryLevel-42)*1.2;	//0~100
	else
		powerLevel=0;
	if(powerLevel>100)
		powerLevel=100;

	uartBufWrite(data_batteryLevel,3);
	uartSendBuf[3]=powerLevel;
	calcSendBufSum();
	uartSend(5);
}


extern sAPPTIMER chargeScanTimer;
void chargeScan(void)
{
	if(P7.0==0){
			batteryStatu|=BAT_CHARGE;
			goActive();
	}else
		batteryStatu&=0xff^BAT_CHARGE;
	if(P7.1==0){
			batteryStatu|=BAT_FULL;
			goActive();
	}else
		batteryStatu&=0xff^BAT_FULL;
	setTimer64Hz(&chargeScanTimer,16);
}

sAPPTIMER chargeScanTimer={0,0,&chargeScan};

void fChargeInt(void)
{
	if(P7.0==0){
		batteryStatu|=BAT_CHARGE;
		goActive();
	}
	else
		batteryStatu&=0xff^BAT_CHARGE;
	if(P7.1==0){
		batteryStatu|=BAT_FULL;
		goActive();
	}
	else
		batteryStatu&=0xff^BAT_FULL;

	setTimer64Hz(&chargeScanTimer,16);

}


fFUNC const sysHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(fAdcEnd),				// 1
	VECTOR(fChargeInt),				// 2
	VECTOR(_nop_Ex),				// 3
	VECTOR(_nop_Ex),				// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
};

void fSysPro(void)
{
	if(gMsg.content<=8)
		sysHandler[gMsg.content]();
}
// ***************************************
// ***************************************

void set3DHEx(uchar addr,uchar value)
{
	enable_3dh();
	SIO00=addr;
	while(CSIIF00==0);CSIIF00=0;
	SIO00=value;
	while(CSIIF00==0);CSIIF00=0;
	disable_3dh();
}

void init3DH(void)
{
	startHClk();
	// set3DHEx(0x20,0x4f);	//50 Hz and Low power
	set3DHEx(0x20,0x47);	//50 Hz
	set3DHEx(0x23,0x80);	//Block update mode
	// set3DHEx(0x23,0x88);	//Block update and High resolution mode
	set3DHEx(0x2e,0x00);
	set3DHEx(0x24,0x40);
	set3DHEx(0x2e,0x8f);
}
