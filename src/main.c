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
	sUtcs.lTime=0;
	R_UART1_Start();
	fifoInit(&sMsgFifo,msgQueue);
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
uchar test[9]={'#',0x03,0x06,0xea,0xaa,4,3,2,1};

extern int spiRevBuf[48];
extern uchar receiveMax;
extern void read3DHCount(void);
void fRtc2Hz(void)
{
	static uint count=0;
	static uchar* const pBuf=spiRevBuf;
	static uchar gOld[3]={0};

	count++;
	if((count&0x1)==0)
	{
		sUtcs.lTime++;
		if(g_Statu==G_SLEEP){
			startHClk();
			// P2.3=0;
			read3DHCount();
			if(receiveMax>0){
				read3DH();

				if(gOld[0]-pBuf[1]>10 and gOld[0]-pBuf[1]<240)
					g_Statu=G_INACTIVE;
				else if(gOld[1]-pBuf[3]>10 and gOld[1]-pBuf[3]<240)
					g_Statu=G_INACTIVE;
				else if(gOld[2]-pBuf[5]>10 and gOld[2]-pBuf[5]<240)
					g_Statu=G_INACTIVE;
				if(g_Statu!=G_SLEEP){
					R_TAU0_Channel5_Start();
					set3DHEx(0x20,0x47);
				}
			}
			// P2.3=1;
		}
	}
	// uartBufWrite(test,5);
	// uartSend(5);

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
	if(sTimerTask.maxIndex>0){
		while(i<sTimerTask.maxIndex){
			if(sTimerTask.tArray[i]()==0){
				taskDelete(i);
			}else{
				i++;
			}
		}
	}else{
		R_TAU0_Channel6_Stop();
	}
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

}

fFUNC const bleHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(fTimeSync),				// 1
	VECTOR(_nop_Ex),				// 2
	VECTOR(_nop_Ex),				// 3
	VECTOR(_nop_Ex),				// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
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

fFUNC const transHandler[]={		// No
	VECTOR(fUartSendEnd),				// 0
	VECTOR(_nop_Ex),				// 1
	VECTOR(_nop_Ex),				// 2
	VECTOR(_nop_Ex),				// 3
	VECTOR(_nop_Ex),				// 4
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
	NOP();
}

fFUNC const sysHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(fAdcEnd),				// 1
	VECTOR(_nop_Ex),				// 2
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
