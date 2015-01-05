#include "flashbranch.h"
#include "main.h"
#include "r_cg_userdefine.h"
#include "NeckAlgorithm.h"

#include "flashfunc.h"

#define VECTOR(vec) vec

fFUNC const msgHandler[]={			// No
	VECTOR(_halt_Ex),				// 0
	VECTOR(fRtcPro),				// 1
	VECTOR(fKeyPro),				// 2
	VECTOR(fTimerPro),				// 3
	VECTOR(fBLEPro),				// 4
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
	VECTOR(fReset),					// 15
};

sMSG gMsg={0,0};	//global message
uchar uartRevTimeout=0;
uchar OADTimeout=0,OADTimeoutCount=0;
uchar SNCode[17]="A22AL1B1SUV000";

uchar BLEResetCount=0;
uchar SNReSendCount=0;
static uint OADcount=0;

uchar get3DHEx(uchar addr);
void set3DHEx(uchar addr,uchar value);

int calcStepLogNum(void);
int calcNeckLogNum(void);
void uartSendLogCount(void);
void OADRequest(uint num);
void memcpyUser(uchar* src,uchar* dst,const size_t length);

extern void dddebug(void);
extern void initFlash(void);
	//for debug
extern void flashErase(unsigned short dataLength, unsigned long flashAddr);
extern void flashWrite(unsigned char* ptr, unsigned short dataLength, unsigned long flashAddr);

extern int isFlashIdle(void);

extern void readFromFlashBytes(unsigned char *rxbufPointer, unsigned short dataLength, unsigned long flashAddr);

void waitFlashIdle(void)
{
	unsigned int x=0;
	while(1)
	{
		if(isFlashIdle())
			return;
		x=0;
		while(x++<1000);
	}
}

uchar waitFlashIdleEx(void)
{
	unsigned int x=0,y=0;
	while(1)
	{
		if(isFlashIdle())
			return 0;	//idle
		x=0;
		while(x++<1000);
		y++;
		if(y>600)
			return 1;	//time out
	}
}

extern uint* const _PWM[4];
void selfCheck_sensor(void)
{
	set3DHEx(0x23,0x00);
	if(get3DHEx(0x23)!=0x00)
		return;
	set3DHEx(0x23,0xAA);
	if(get3DHEx(0x23)!=0xAA)
		return;
	set3DHEx(0x23,0x44);
	if(get3DHEx(0x23)!=0x44)	// the least bit is SPI mode ctrl, DO NOT CHANGE IT!!
		return;
	set3DHEx(0x23,0x00);
	led1Off();
}

uchar selfCheck_flash_module(uchar data,unsigned long addr)
{
	uchar _;
	flashErase(1,addr);
	if(waitFlashIdleEx())
		return 1;	//1 for error
	readFromFlashBytes(&_,1,addr);
	if(_!=0xff)
		return 1;

	_=data;
	flashWrite(&_, 1, addr);
	if(waitFlashIdleEx())
		return 1;
	readFromFlashBytes(&_,1,addr);
	if(_!=data)
		return 1;
	return 0;	//0 for success
}

void selfCheck_flash(void)
{
	if(selfCheck_flash_module(FLASH_TEST_DATA1,FLASH_TEST_ADDR1))
		return;
	if(selfCheck_flash_module(FLASH_TEST_DATA2,FLASH_TEST_ADDR2))
		return;
	if(selfCheck_flash_module(FLASH_TEST_DATA3,FLASH_TEST_ADDR3))
		return;
	led2Off();
}

void selfCheck(void)
{
	unsigned long i=0;
	ledSetMode(LED_M_STATICPOWER,0x1FF);
	led1On();
	led2On();
	led3On();
	led4On();
	VibrateOn();
	while(i++<500000);
	selfCheck_sensor();
	selfCheck_flash();
	HALT();
	NOP();
	HALT();
	VibrateOff();
	led3Off();
	// led4Off();
}

void recoverData(void)
{
	uchar i=0;
	uchar temp[20];
	union{
		uchar addr[16];
		unsigned long addrLong[4];
	}uAddr;
	waitFlashIdle();
	readFromFlashBytes(temp,16,SN_SAVE_START);
	for(i=0;i<16;i++){
		if(temp[i]!=0xFF)
			break;
	}
	if(i!=16){
		for(i=0;i<16;i++){
			SNCode[i]=temp[i];
		}
	}
	readFromFlashBytes(temp,2,ADDR_SAVE_START+4*sizeof(unsigned long));
	if(temp[0]==0xAA && temp[1]==0x55){
		readFromFlashBytes(uAddr.addr,4*sizeof(unsigned long),ADDR_SAVE_START);
		if(uAddr.addrLong[0]<=STEPRANGEND
			&& uAddr.addrLong[1]<=STEPRANGEND
			&& uAddr.addrLong[0]>=STEPRANGSTART
			&& uAddr.addrLong[1]>=STEPRANGSTART
			&& uAddr.addrLong[2]<=NECKRANGEND
			&& uAddr.addrLong[3]<=NECKRANGEND){
			stepFlash.startAddr=uAddr.addrLong[0];
			stepFlash.endAddr=uAddr.addrLong[1];
			neckFlash.startAddr=uAddr.addrLong[2];
			neckFlash.endAddr=uAddr.addrLong[3];
		}
	}else{
		return;
	}
	// addr seek
	// end addr在块尾，或当前无记录则截止
	// 否则，end addr + 20
	while(1)
	{
		readFromFlashBytes(temp,20,stepFlash.endAddr);
		for(i=0;i<20;i++)
		{
			if(temp[i]!=0xff)
				break;
		}
		if(i<20)
			stepFlash.endAddr+=20;
		else
			break;
		if(stepFlash.endAddr%4096==0)
			break;
		if(stepFlash.endAddr>=STEPRANGEND)
			break;
	}

	while(1)
	{
		readFromFlashBytes(temp,20,neckFlash.endAddr);
		for(i=0;i<20;i++)
		{
			if(temp[i]!=0xff)
				break;
		}
		if(i<20)
			neckFlash.endAddr+=20;
		else
			break;
		if(neckFlash.endAddr%4096==0)
			break;
		if(neckFlash.endAddr>=NECKRANGEND)
			break;
	}
}

void statuSelect(void)
{
	uchar temp[2];
	waitFlashIdle();
	readFromFlashBytes(&temp,2,PROGRAMFLAGADDR);
	if(temp[0]==0x00)
		sSelf.mode=SYS_ACTIVE;
	else
		sSelf.mode=SYS_SLEEP;

	if(temp[1]==0x55)
		ledSetMode(LED_M_MQ,1);
	else if(temp[1]==0x44)
		ledSetMode(LED_M_FLASHALL,5);
}

void afterBoot(void)
{
	unsigned long x=0;
	initFlash();
	P2.5=0;
	while(x++<100000);
	P2.0=1;
	P5.1=1;
	R_INTC0_Start();
	R_KEY_Start();
	sUtcs.lTime=0;
	sAlarmTime.lTime=0xffffffff;
	currentStepLog.UTC=sUtcs.lTime;
	R_UART1_Start();
	fifoInit(&sMsgFifo,msgQueue);
	flashQueueInit(&sFlashQueue);

	SetMinDeltaNeckMovement(MIN_DELTA_NECKMOVEMENT);
	SetMinZeroPoint(MIN_ZEROPOINT);
}

#include "bootmain.h"
extern int isFlashIdle(void); 
extern void R_PCLBUZ0_Start(void);
void init3DH(void);
extern sNECKLOG neckLog[16];
extern sSTEPLOG stepLog;
void iMain(void)
{
	afterBoot();

	ledSetMode(LED_M_OFF,1);
	EI();
	HALT();
	NOP();
	HALT();
	fifoFlush();
	P2.5=1;

	if(P3.0==0){
		startHClk();
		selfCheck();
		sSelf.mode=SYS_TEST;
		while(1){
			DI();
			gMsg=fifoGet();
			EI();
			if(gMsg.type<=15)
				msgHandler[gMsg.type]();
			if(P3.0==1)
				break;
		}
		sSelf.mode=SYS_SLEEP;
	}

	recoverData();
	statuSelect();
	if(sSelf.mode==SYS_ACTIVE){
		waitFlashIdle();
		flashErase(2,PROGRAMFLAGADDR);
		init3DH();
		init3DH();
		R_TAU0_Channel5_Start();
		// ledSetMode(LED_M_MQ,3);
		P5.1=0;
	}else{
		ledSetMode(LED_M_OFF,1);
		set3DHEx(0x20,0x07);	// power down
		set3DHEx(0x20,0x07);	// power down
	}

	setADTimer(3);
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

uchar resetHEX[]={0xff,0xff,0xff,0xff};
fFUNC resetFunc=&resetHEX;
void fReset(void)
{
	resetFunc();
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
	static int layDownCount=0;
	if(currentNeckLog.neckMove<HEALTHNECKMOVE)
		neckUnhealthCount++;
	else
		neckUnhealthCount=0;
	if(sGAcc.x>30 or sGAcc.x<-30){
		if(layDownCount<LAYDOWNTIMELIMIT){
			layDownCount++;
		}else{
			if(neckUnhealthCount>=5 and neckUnhealthCount%5==0){
				if(neckUnhealthCount>50)
					neckUnhealthCount=50;
				if(g_Statu==G_INACTIVE)
					setVibrate(&sV5);
				else
					neckUnhealthCount=0;
				// sVibrate.count=neckUnhealthCount/5;
				sVibrate.count=1;
			}
		}
	}else{
		layDownCount=0;
		neckUnhealthCount=0;
	}
}


extern void addrCache(void);
extern tNECK Neck;
extern sAPPTIMER stopVibrateTimer;
extern const uchar data_SNCode[3];
// Neck.PositionID
unsigned long daySec=0;
unsigned long dayStep=0;
uchar SNReSendTimeOutCount=0;
void fRtc2Hz(void)
{
	static uint count=0;
	static uint recoverCount=0,sleepCount=0;
	static uchar* const pBuf=spiRevBuf;
	static uchar gOld[3]={0},flag=0;	//flag 用于标示是否已更新gOld
	static uint currentStepLogSec=0;
	count++;

	// if(count==6 && P5.1==1)
	// 	P5.1==0;

	if(BLEResetCount>0)
	{
		BLEResetCount++;
		if(BLEResetCount>3){
			P2.5=1;
			BLEResetCount=0;
		}else if(BLEResetCount>2){
			P2.5=0;
		}
	}

	if(SNReSendCount>0){
		SNReSendCount++;
		if(SNReSendCount>3){
			uartBufWrite(data_SNCode,3);
			memcpyUser(&SNCode,&uartSendBuf[3],14);
			calcSendBufSum();
			uartSend(14+4);
			SNReSendTimeOutCount++;
			if(SNReSendTimeOutCount<4)
				SNReSendCount=1;
			else{
				SNReSendCount=0;
				SNReSendTimeOutCount=0;
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

	if(sSelf.mode==SYS_OAD){
		OADTimeout++;
		if(OADTimeout>2){
			OADRequest(OADcount+1);
			OADTimeout=0;
			OADTimeoutCount++;
			if(OADTimeoutCount>2){
				fReset();
			}
		}
	}

	if((count&0x1)==0)
		timer(&adTimer);

	if(sSelf.mode!=SYS_ACTIVE)
		return;

	if(isTimeSync && recoverCount>60000){
		addrCache();
		recoverCount=0;
	}

	if((count&0x1)==0)
	{
		recoverCount++;
		if(BLE_Connect_Timeout>0)
			BLE_Connect_Timeout--;
		if(batteryStatu!=BAT_NORMAL){
			if(batteryStatu==BAT_CHARGE && (sLed.ledMode==LED_M_OFF or sLed.ledMode==LED_M_POWER))
				ledSetMode(LED_M_POWER,2);
		}

		if(sNeckMoveStatu.statu && isTimeSync){
			if(g_Statu==G_SLEEP){
				switch(Neck.PositionID){
				case HEAD_UP: currentNeckLog.upTime+=10; break;
				case HEAD_DOWN: currentNeckLog.downTime+=10; break;
				case HEAD_LEFT: currentNeckLog.leftTime+=10; break;
				case HEAD_RIGHT: currentNeckLog.rightTime+=10; break;
				}
			}
			sNeckMoveStatu.timeCount++;
			if(sNeckMoveStatu.timeCount>=240){
			// if(sNeckMoveStatu.timeCount>=60){	//debug
				sNeckMoveStatu.timeCount=0;
				sNeckMoveStatu.statu=0;
				neckHealthCheck();
				neckLogCache();
				sleepCount=0;
			}
		}else{
			sleepCount++;
			if(sleepCount>=3600){
				sleepCount=0;
				neckUnhealthCount=0;
			}
		}

		currentStepLogSec++;
		if(currentStepLogSec>=300){
		// if(currentStepLogSec>=60){	//debug
			currentStepLogSec=0;
			if(isTimeSync)
				stepLogCache();
		}
		daySec++;
		if(daySec>=86400){
			daySec=0;
			dayStep=0;
		}
		sUtcs.lTime++;
		if(g_Statu==G_SLEEP){
			startHClk();
			// P2.3=0;
			read3DHCount();
			if(receiveMax>0){
				read3DH();
				if(!stopVibrateTimer.en){
					if(flag){
						if(gOld[0]-pBuf[1]>20 and gOld[0]-pBuf[1]<240)
							g_Statu=G_INACTIVE;
						else if(gOld[1]-pBuf[3]>20 and gOld[1]-pBuf[3]<240)
							g_Statu=G_INACTIVE;
						else if(gOld[2]-pBuf[5]>20 and gOld[2]-pBuf[5]<240)
							g_Statu=G_INACTIVE;
					}else{
						gOld[0]=pBuf[1];
						gOld[1]=pBuf[3];
						gOld[2]=pBuf[5];
						flag=1;
					}
				}

				if(g_Statu!=G_SLEEP){
					flag=0;
					R_TAU0_Channel5_Start();
					set3DHEx(0x20,0x47);
					set3DHEx(0x20,0x47);
				}
			}else{
				set3DHEx(0x20,0x1f);
				set3DHEx(0x20,0x1f);
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

extern const uchar data_batteryLevel[3];
extern const uchar data_version[3];
extern int uartTimeOutTask(void);
extern uchar uartTimeOutTaskStatu;
void fBLEConfirm(void)
{
	BLEResetCount=0;
	if((uartRevBuf[3]==0x09 or uartRevBuf[3]==0x0A or uartRevBuf[3]==0x0B)
		and sUpload.statu!=UPLOAD_IDLE){
		if(sUpload.packageRemain>0 and uartRevBuf[3]!=0x0B){
			flashReadSeek();
			sUpload.packageRemain--;
		}

		if(sUpload.packageRemain<=0){
			if(sUpload.statu==UPLOAD_NECK){
				neckFlash.startAddr=NECKRANGSTART;
				neckFlash.endAddr=NECKRANGSTART;
			}else if(sUpload.statu==UPLOAD_STEP){
				stepFlash.startAddr=STEPRANGSTART;
				stepFlash.endAddr=STEPRANGSTART;
			}
			uartSendLogCount();
			sUpload.statu=UPLOAD_IDLE;
			addrCache();
			return;
		}
		dataReadSend();
		sUpload.timeOut=0;
		sUpload.timeOutCount=0;
		if(!uartTimeOutTaskStatu){
			uartTimeOutTaskStatu=1;
			taskInsert(&uartTimeOutTask);
		}
	}else if(uartRevBuf[3]==0x11){
		uartBufWrite(data_version,3);
		memcpyUser(&version,&uartSendBuf[3],16);
		calcSendBufSum();
		uartSend(20);	// send version num 
	}else if(uartRevBuf[3]==0x14){
		SNReSendCount=0;
		uartBufWrite(data_batteryLevel,3);
		uartSendBuf[3]=powerLevel;
		calcSendBufSum();
		uartSend(5);	// send battery level
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
	daySec=sUtcs.lTime%86400;
	isTimeSync=1;
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
	if(sVibrate.count==0)
		sVibrate.count=0xff;
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
		case 6: powerLevel=BAT20+1;ledMode=LED_M_POWER;break;
		case 7: powerLevel=BAT40+1;ledMode=LED_M_POWER;break;
		case 8: powerLevel=BAT60+1;ledMode=LED_M_POWER;break;
		case 9: powerLevel=BAT80+1;ledMode=LED_M_POWER;break;
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
	set3DHEx(0x20,0x07);	// power down
	if(sSelf.mode==SYS_ACTIVE)
	{
		memsetUser(&currentNeckLog,0,sizeof(sNECKLOGLONG));
		memsetUser(&currentStepLog,0,sizeof(sSTEPLONGLOG));
		g_Statu=G_SLEEP;
		R_TAU0_Channel5_Stop();
		directGEn=0;
		P5.1=1;
	}
	sSelf.mode=SYS_SLEEP;
}

void goActive(void)
{
	if(sSelf.mode==SYS_SLEEP)
	{
		// set3DHEx(0x20,0x47);
		// set3DHEx(0x20,0x47);
		init3DH();
		init3DH();
		R_TAU0_Channel5_Start();
		g_Statu=G_INACTIVE;
		P5.1=0;
		sSelf.mode=SYS_ACTIVE;
	}
}

extern const uchar data_UTC[3];
extern const sFLASHOP opFlashAddrErase;
void fFormatFlash(void)
{
	if(uartRevBuf[3]==0xd9){
		goSleep();
	}else if(uartRevBuf[3]==0xEA){
		initFlash();
		flashOpPut(opFlashWait);
		flashOpPut(opFlashAddrErase);
		flashOpFin();
	}else if(uartRevBuf[3]==0xE1){
		uartBufWrite(data_UTC,3);
		memcpyUser(&sUtcs.lTime,&uartSendBuf[3],4);
		calcSendBufSum();
		uartSend(8);	// send version num 
		return;
	}
	uartSuccess(0x07);
}

void fGsensorAcc(void)
{
	if(uartRevBuf[3]){
		directGEn=1;
		set3DHEx(0x20,0x47);
		set3DHEx(0x20,0x47);
		if(g_Statu==G_SLEEP){
			R_TAU0_Channel5_Start();
		}
	}else{
		directGEn=0;
	}

	uartSuccess(0x08);
}


int calcStepLogNum(void)
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

int calcNeckLogNum(void)
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


uchar OADLog[18]={0};
extern const uchar data_OADRequest[];
static const sFLASHOP opFlashOADErase={FLASH_F_BLOCKERASE,FLASH_S_OAD};
static const sFLASHOP opFlashOADSave={FLASH_F_WRITE,FLASH_S_OAD};

void OADRequest(uint num)
{
	uartBufWrite(data_OADRequest,3);
	uartSendBuf[3]=num&0xFF;
	uartSendBuf[4]=num>>8;
	calcSendBufSum();
	uartSendDirect(6);
}

const sFLASHOP opFlashSNErase={FLASH_F_BLOCKERASE,FLASH_S_SN};
const sFLASHOP opFlashSNSave={FLASH_F_WRITE,FLASH_S_SN};
const sFLASHOP opFlashSNRead={FLASH_F_READ,FLASH_S_SN};
void fOAD(void)
{
	uint i=0,checkSum=0;
	uint *ptr=OADLog;
	static uchar _t=0;

	if(uartRevBuf[1]==0x12)	// SN 烧录(复用了OADlog)
	{
		i=0;
		while(i<16){OADLog[i++]=uartRevBuf[i+3];}
		waitFlashIdle();
		flashOpPut(opFlashWait);
		flashOpPut(opFlashSNErase);
		flashOpPut(opFlashWait);
		flashOpPut(opFlashSNSave);
		flashOpPut(opFlashWait);
		flashOpPut(opFlashSNRead);
		flashOpFin();
	}

	if(uartRevBuf[1]!=0x14)
		return;
	OADcount=uartRevBuf[4];
	OADcount=(OADcount<<8)|uartRevBuf[3];
	OADTimeout=0;
	OADTimeoutCount=0;
	if(OADcount<3 or sSelf.mode!=SYS_OAD){
		ledSetMode(LED_M_STATICPOWER,0x1FF);
		led1On();
		led2On();
		led3On();
		led4On();
		sSelf.mode=SYS_OAD;
		directGEn=0;
		if(OADcount>=0x0c04)
			fReset();
		OADRequest(OADcount+1);
		// flashOpPut(opFlashWait);
		// flashOpPut(opFlashOADErase);
		// flashOpPut(opFlashWait);
		// flashOpFin();
	}else{
		i=0;
		while(i<16){OADLog[i++]=uartRevBuf[i+5];}
		i=0;
		while(i<8){checkSum+=*ptr++; i++; }
		*ptr=checkSum;

		if((OADcount&0x1F)==0x0){
			_t++;
			ledAllOff();
			switch(_t&0x3){
				case 0: led1On();break;
				case 1: led2On();break;
				case 2: led3On();break;
				case 3: led4On();break;
			}
		}

		// flashOpPut(opFlashWait);
		waitFlashIdle();
		if(needErase(18,programFlash.endAddr)){
			flashOpPut(opFlashWait);
			flashOpPut(opFlashOADErase);
			flashOpPut(opFlashWait);
		}
		flashOpPut(opFlashOADSave);
		flashOpFin();
	}
}

void fConnectRequest(void)
{
	if(uartRevBuf[3]==0x01){
		BLE_Connect_Timeout=BLE_CONNECT_TIMEOUT_SET;
		// ledSetMode(LED_M_MQ,10);
	}
	uartSuccess(0x10);
}

void fSNRequest(void)
{
	uchar i;
	if(uartRevBuf[3]==0x01){
		uartBufWrite(data_SNCode,3);
		for(i=0;i<14;i++)
			uartSendBuf[i+3]=SNCode[i];
		calcSendBufSum();
		uartSend(14+4);
		SNReSendCount=1;
		SNReSendTimeOutCount=0;
	}
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
	VECTOR(fOAD),				// 9	(OAD+SN program)
	VECTOR(fConnectRequest),				// 10
	VECTOR(fSNRequest),				// 11
};

void fBLEPro(void)
{
	if(gMsg.content<=11)
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

extern void flashWrite(unsigned char* ptr, unsigned short dataLength, unsigned long flashAddr);
extern sFLASHOP gOP;
static uchar AA=0xAA;
void fFlashOpFinish(void)
{
	gOP=flashOpGet();
	if(gOP.opType==0){
		if(sSelf.mode==SYS_OAD){
			if(OADcount<2)
				OADcount=2;
			if(OADcount>=0x0c04){
				ledAllOff();
				waitFlashIdle();
				flashErase(1,PROGRAMFLAGADDR);
				waitFlashIdle();
				flashWrite(&AA,1,PROGRAMFLAGADDR);
				waitFlashIdle();
				// readFromFlashBytes(&AA,1,PROGRAMFLAGADDR);
				// readFromFlashBytes(&AA,1,PROGRAMERRANGSTART);
				NOP();
				fReset();
			}
			OADRequest(OADcount+1);
		}
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
void fChargeInt(void);
static uchar localPowerLevel=0xff;
void fAdcEnd(void)
{
	static uint batteryLevelOld=0x100;
	uint temp;
	if(sVibrate.en)
	{
		setADTimer(10);
		return;
	}
	batteryLevel=((adcValue[0]>>6)+(adcValue[1]>>6)+(adcValue[2]>>6)+(adcValue[3]>>6))>>2;
	// batteryLevel=batteryLevel-746;	//746~871  batteryLevel:0~125
	// batteryLevel=batteryLevel-745;	//745~920  batteryLevel:0~175
	batteryLevel=batteryLevel-745;	//745~915  batteryLevel:0~170
	// 42~125 -> 3.6v~4.2v
	// 42~170 -> 3.6v~4.2v
	if(batteryLevel<0)
		batteryLevel=0;
	if(batteryLevelOld>50 && batteryLevel<=50){
		setVibrate(&sV5);
		sVibrate.count=1;
	}

	if(batteryLevelOld<0x100){
		if(batteryStatu==BAT_NORMAL){
			if(batteryLevel>batteryLevelOld)
				batteryLevel=batteryLevelOld;
		}else{
			if(batteryLevel<batteryLevelOld)
				batteryLevel=batteryLevelOld;
		}
	}

	batteryLevelOld=batteryLevel;
	// batteryLevel=batteryLevel-810;	//810~919  batteryLevel:0~109
	// batteryLevel=batteryLevel-743;	//743~868  batteryLevel:0~125

	if(batteryLevel>42)
		powerLevel=(float)(batteryLevel-42)*0.783;	//0~100
	else
		powerLevel=0;
	if(powerLevel>100)
		powerLevel=100;

	if(batteryStatu!=BAT_NORMAL){
		if(powerLevel>95)
			powerLevel=95;
	}

	if(localPowerLevel<0xff){
		if(localPowerLevel>powerLevel){
			if(localPowerLevel==100){
				if(powerLevel<localPowerLevel and (batteryStatu&BAT_FULL)==0)
					localPowerLevel=99;
			}else{
				if(localPowerLevel-powerLevel>7)
					localPowerLevel-=7;
				else
					localPowerLevel=powerLevel;
			}
		}else{
			if(powerLevel-localPowerLevel>2)
				localPowerLevel+=2;
			else
				localPowerLevel=powerLevel;
		}
	}else{
		localPowerLevel=powerLevel;
		fChargeInt();
	}
	powerLevel=localPowerLevel;

	if(batteryStatu==BAT_NORMAL and batteryLevel<48){
		goSleep();
		return;
	}

	uartBufWrite(data_batteryLevel,3);
	uartSendBuf[3]=powerLevel;
	calcSendBufSum();
	if(sSelf.mode!=SYS_OAD){
		uartSend(5);
		BLEResetCount=1;
	}
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
		localPowerLevel=100;
		powerLevel=localPowerLevel;
		goActive();
	}else if(P7.0==0){
		batteryStatu&=0xff^BAT_FULL;
	}
	if(batteryStatu!=BAT_NORMAL)
		setTimer64Hz(&chargeScanTimer,16);
}

sAPPTIMER chargeScanTimer={0,0,&chargeScan};

void fChargeInt(void)
{
	if(P7.0==0){
		batteryStatu|=BAT_CHARGE;
		setADTimer(10);
		goActive();
	}
	else
		batteryStatu&=0xff^BAT_CHARGE;
	if(P7.1==0){
		batteryStatu|=BAT_FULL;
		localPowerLevel=100;
		powerLevel=localPowerLevel;
		goActive();
	}
	else if(P7.0==0){
		batteryStatu&=0xff^BAT_FULL;
	}

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

uchar get3DHEx(uchar addr)
{
	uchar reg=0;
	enable_3dh();
	SIO00=(addr|0xc0);
	while(CSIIF00==0);CSIIF00=0;
	SIO00=0xFF;
	while(CSIIF00==0);CSIIF00=0;
	reg=SIO00;
	disable_3dh();
	return reg;
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
