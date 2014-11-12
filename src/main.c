#include "main.h"
#include "r_cg_userdefine.h"

#define VECTOR(vec) vec

fFUNC const msgHandler[]={		// No
	VECTOR(_halt_Ex),				// 0
	VECTOR(fRtcPro),			// 1
	VECTOR(fKeyPro),			// 2
	VECTOR(fTimerPro),			// 3
	VECTOR(fBLEPro),			// 4
	VECTOR(fTransPro),				// 5
	VECTOR(_nop_Ex),				// 6
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

void init3DH(void);
void iMain(void)
{
	afterBoot();
	ledSetMode(LED_M_OFF);
	EI();
	HALT();
	NOP();
	HALT();
	fifoFlush();
	ledSetMode(LED_M_MQ);

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

void fRtc2Hz(void)
{
	static uint count=0;
	count++;

	uartBufWrite(test,5);
	uartSend(5);
	// switch(count)
	// {
	// 	case 4:
	// 	ledSetMode(LED_M_HERATBEAT);
	// 	break;

	// 	case 16:
	// 	ledSetMode(LED_M_BREATHE);
	// 	break;

	// 	case 32:
	// 	ledSetMode(LED_M_MQ);
	// 	break;

	// 	case 42:
	// 	ledSetMode(LED_M_POWER);
	// 	break;

	// 	case 52:
	// 	ledSetMode(LED_M_SWING);
	// 	break;

	// 	case 62:
	// 	ledSetMode(LED_M_FLASHALL);
	// 	break;

	// 	case 72:
	// 	ledSetMode(LED_M_RANDOM);
	// 	break;

	// 	case 82:
	// 	ledSetMode(LED_M_OFF);
	// 	break;

	// 	case 84:
	// 	count=0;
	// 	break;
	// }
}

void fRtc5Hz(void)
{
	startHClk();
	_3DH5Hz();
}


fFUNC const rtcHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(_nop_Ex),				// 1
	VECTOR(fRtc2Hz),				// 2
	VECTOR(fRtc5Hz),				// 3
	VECTOR(_nop_Ex),				// 4
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

void set3DHEx(uchar addr,uchar value)
{
	enable_3dh();
	SIO00=addr;
	while(CSIIF00==0);CSIIF00=0;
	// while(SSR00&0x0040);
	// while(SSR00&0x0040!=0);
	SIO00=value;
	while(CSIIF00==0);CSIIF00=0;
	// while(SSR00&0x0040);
	// while(SSR00&0x0040!=0);
	disable_3dh();
}

void init3DH(void)
{
	startHClk();
	// set3DHEx(0x20,0x4f);	//50 Hz and Low power
	set3DHEx(0x20,0x47);	//50 Hz
	// set3DHEx(0x23,0x80);	//Block update mode
	set3DHEx(0x23,0x88);	//Block update and High resolution mode
	set3DHEx(0x2e,0x00);
	set3DHEx(0x24,0x40);
	set3DHEx(0x2e,0x8f);
}
