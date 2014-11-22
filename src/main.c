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

void afterBoot(void)
{
	P2.0=1;
	R_INTC0_Start();
	sUtcs.lTime=0;
	sAlarmTime.lTime=0xffffffff;
	R_UART1_Start();
	fifoInit(&sMsgFifo,msgQueue);
	setADTimer(10);
}

void set3DHEx(uchar addr,uchar value);
void init3DH(void);
void iMain(void)
{
	afterBoot();
	ledSetMode(LED_M_OFF,1);
	EI();
	HALT();
	NOP();
	HALT();
	fifoFlush();
	ledSetMode(LED_M_MQ,3);

	init3DH();
	R_TAU0_Channel5_Start();

	while(1){
		DI();
		gMsg=fifoGet();
		EI();
		NOP();
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
extern uint currentNeckLogSec;
extern sNECKLOGLONG currentNeckLog;
extern sAPPTIMER adTimer;
void fRtc2Hz(void)
{
	static uint count=0;
	static uchar* const pBuf=spiRevBuf;
	static uchar gOld[3]={0},flag=0;	//flag 用于标示是否已更新gOld

	count++;
	if((count&0x1)==0)
	{
		if(batteryStatu!=BAT_NORMAL){
			if(batteryStatu==BAT_CHARGE)
				ledSetMode(LED_M_POWER,2);
		}
		timer(&adTimer);
		// startAD();
		// if(!(count&0x7))
		// 	setVibrate(&sV1);
		currentNeckLogSec++;
		if(currentNeckLogSec>=300){
			currentNeckLogSec=0;
			neckLogCache();
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
		while(i<sTimerTask.maxIndex){if(sTimerTask.tArray[i]()==0) taskDelete(i); else i++; }
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
	timerHandler[gMsg.content]();
}


// ***************************************
// ***************************************
void fTimeSync(void)
{
	uchar* ptr1=&uartRevBuf[3];
	uchar* ptr2=&sUtcs;
	*ptr2++=*ptr1++;
	*ptr2++=*ptr1++;
	*ptr2++=*ptr1++;
	*ptr2=*ptr1;

	uartBufWrite(data_transSuccess,5);
	uartSend(5);
}

void fSetAlarm(void)
{
	uchar* ptr1=&uartRevBuf[3];
	uchar* ptr2=&sAlarmTime;
	*ptr2++=*ptr1++;
	*ptr2++=*ptr1++;
	*ptr2++=*ptr1++;
	*ptr2=*ptr1;

	uartBufWrite(data_transSuccess,5);
	uartSend(5);
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
	uartBufWrite(data_transSuccess,5);
	uartSend(5);
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

	uartBufWrite(data_transSuccess,5);
	uartSend(5);
}

void fFormatFlash(void)
{


	uartBufWrite(data_transSuccess,5);
	uartSend(5);
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

	uartBufWrite(data_transSuccess,5);
	uartSend(5);
}

void fDataReqest(void)
{

}


fFUNC const bleHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(_nop_Ex),				// 1
	VECTOR(fTimeSync),				// 2
	VECTOR(fSetAlarm),				// 3
	VECTOR(fMotorCtl),				// 4
	VECTOR(fLEDCtl),				// 5
	VECTOR(fFormatFlash),				// 6
	VECTOR(fGsensorAcc),				// 7
	VECTOR(fDataReqest),				// 8
};

void fBLEPro(void)
{
	if(gMsg.content<=M_C_BLESTATU)
		bleHandler[gMsg.content]();
}

// ***************************************
// ***************************************
void fUartSendEnd(void)
{
	NOP();
	sUart.statu=UART_IDLE;
}

void fUartRevReq(void)
{
	startHClk();
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

void fFlashOpFinish(void)
{
	sFLASHOP op=flashOpGet();
	if(op.opType==0){
		// flashSleep();
	}else{
		fFlashOp[op.opType]();
	}
}

fFUNC const transHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(fUartSendEnd),				// 1
	VECTOR(fUartRevReq),				// 2
	VECTOR(fFlashOpStart),				// 3
	VECTOR(fFlashOpFinish),				// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
};

void fTransPro(void)
{
	transHandler[gMsg.content]();
}
// ***************************************
// ***************************************
void fAdcEnd(void)
{
	uint temp;
	if(sVibrate.en)
	{
		setADTimer(10);
		return;
	}
	batteryLevel=((adcValue[0]>>6)+(adcValue[1]>>6)+(adcValue[2]>>6)+(adcValue[3]>>6))>>2;
	batteryLevel=batteryLevel-743;	//743~868  batteryLevel:0~125
	powerLevel=batteryLevel/27;

	uartBufWrite(data_batteryLevel,3);
	uartSendBuf[3]=powerLevel*25;
	calcSendBufSum();
	uartSend(5);
}


extern sAPPTIMER chargeScanTimer;
void chargeScan(void)
{
	if(P7.0==0)
		batteryStatu|=BAT_CHARGE;
	else
		batteryStatu&=0xff^BAT_CHARGE;
	if(P7.1==0)
		batteryStatu|=BAT_FULL;
	else
		batteryStatu&=0xff^BAT_FULL;
	setTimer64Hz(&chargeScanTimer,16);
}

sAPPTIMER chargeScanTimer={0,0,&chargeScan};

void fChargeInt(void)
{
	if(P7.0==0)
		batteryStatu|=BAT_CHARGE;
	else
		batteryStatu&=0xff^BAT_CHARGE;
	if(P7.1==0)
		batteryStatu|=BAT_FULL;
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
